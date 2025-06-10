#include "opening_book.h"
#include "heuristic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#ifdef _OPENMP
#include <omp.h>
#endif

// === ZMIENNE GLOBALNE ===
OpeningEntry* openingBook = NULL;
int bookSize = 0;
int bookCapacity = 0;
int moveHistory[25];
int historyLength = 0;

// === ZARZĄDZANIE HISTORIĄ RUCHÓW ===

void clearMoveHistory(void) {
    historyLength = 0;
    memset(moveHistory, 0, sizeof(moveHistory));
}

void addMoveToHistory(int move) {
    if (historyLength < 25) {
        moveHistory[historyLength++] = move;
    }
}

char* buildMoveSequence(void) {
    static char sequence[MAX_SEQUENCE_LENGTH];
    sequence[0] = '\0';
    
    for (int i = 0; i < historyLength; i++) {
        if (i > 0) strcat(sequence, ",");
        char moveStr[10];
        sprintf(moveStr, "%d", moveHistory[i]);
        strcat(sequence, moveStr);
    }
    return sequence;
}

int getMoveCount(void) {
    return historyLength;
}

// === THREAD-SAFE FUNCTIONS ===

// Mutex do synchronizacji dostępu do książki otwarć
#ifdef _OPENMP
static omp_lock_t book_lock;
static bool lock_initialized = false;

void init_book_lock() {
    if (!lock_initialized) {
        omp_init_lock(&book_lock);
        lock_initialized = true;
    }
}

void cleanup_book_lock() {
    if (lock_initialized) {
        omp_destroy_lock(&book_lock);
        lock_initialized = false;
    }
}
#endif

// Thread-safe dodawanie wpisu do książki
void addOpeningEntryThreadSafe(const char* sequence, int move, int score, int depth) {
#ifdef _OPENMP
    omp_set_lock(&book_lock);
#endif
    addOpeningEntry(sequence, move, score, depth);
#ifdef _OPENMP
    omp_unset_lock(&book_lock);
#endif
}

// === ZARZĄDZANIE KSIĄŻKĄ ===

void initOpeningBook(void) {
    if (openingBook == NULL) {
        bookCapacity = 1000;
        openingBook = malloc(bookCapacity * sizeof(OpeningEntry));
        bookSize = 0;
    }
}

void addOpeningEntry(const char* sequence, int move, int score, int depth) {
    initOpeningBook();
    
    // Sprawdź czy wpis już istnieje - jeśli tak, aktualizuj jeśli lepsza analiza
    for (int i = 0; i < bookSize; i++) {
        if (strcmp(openingBook[i].sequence, sequence) == 0) {
            if (depth > openingBook[i].depth_analyzed) {
                openingBook[i].best_move = move;
                openingBook[i].score = score;
                openingBook[i].depth_analyzed = depth;
            }
            return;
        }
    }
    
    // Dodaj nowy wpis
    if (bookSize >= bookCapacity) {
        bookCapacity *= 2;
        openingBook = realloc(openingBook, bookCapacity * sizeof(OpeningEntry));
        if (!openingBook) {
            printf("Error: Cannot allocate memory for opening book!\n");
            exit(1);
        }
    }
    
    strcpy(openingBook[bookSize].sequence, sequence);
    openingBook[bookSize].best_move = move;
    openingBook[bookSize].score = score;
    openingBook[bookSize].depth_analyzed = depth;
    bookSize++;
}

void freeOpeningBook(void) {
    if (openingBook) {
        free(openingBook);
        openingBook = NULL;
        bookSize = 0;
        bookCapacity = 0;
    }
}

// === UŻYCIE KSIĄŻKI W GRZE ===

bool isInOpeningPhase(int moveCount) {
    return moveCount <= MAX_OPENING_MOVES;
}

int getOpeningMove(const char* moveSequence, int moveCount) {
    // Sprawdź czy jesteśmy w fazie otwarcia
    if (!isInOpeningPhase(moveCount)) {
        return 0; // Poza fazą otwarcia
    }
    
    if (openingBook == NULL || bookSize == 0) {
        return 0; // Brak książki
    }
    
    // Znajdź sekwencję w książce
    for (int i = 0; i < bookSize; i++) {
        if (strcmp(openingBook[i].sequence, moveSequence) == 0) {
            printf("[OPENING] Using book move %d for sequence: %s\n", 
                   openingBook[i].best_move, moveSequence);
            return openingBook[i].best_move;
        }
    }
    
    return 0; // Nie znaleziono w książce
}

// === ŁADOWANIE/ZAPISYWANIE ===

bool loadOpeningBook(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("[OPENING] Book file %s not found. Starting with empty book.\n", filename);
        return false;
    }
    
    initOpeningBook();
    bookSize = 0;
    
    char line[200];
    int loaded = 0;
    
    while (fgets(line, sizeof(line), file)) {
        // Pomiń komentarze i puste linie
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        
        char sequence[MAX_SEQUENCE_LENGTH];
        int move, score, depth;
        
        // Format: "sequence -> move (score) [depth]"
        if (sscanf(line, "%s -> %d (%d) [%d]", sequence, &move, &score, &depth) == 4) {
            addOpeningEntry(sequence, move, score, depth);
            loaded++;
        }
    }
    
    fclose(file);
    printf("[OPENING] Loaded %d entries from %s\n", loaded, filename);
    return true;
}

void saveOpeningBook(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("[OPENING] Error: Cannot save book to %s\n", filename);
        return;
    }
    
    fprintf(file, "# Auto-generated Opening Book\n");
    fprintf(file, "# Max opening moves: %d\n", MAX_OPENING_MOVES);
    fprintf(file, "# Format: sequence -> move (score) [depth]\n");
    fprintf(file, "# Generated entries: %d\n\n", bookSize);
    
    for (int i = 0; i < bookSize; i++) {
        fprintf(file, "%s -> %d (%d) [%d]\n", 
                openingBook[i].sequence,
                openingBook[i].best_move,
                openingBook[i].score,
                openingBook[i].depth_analyzed);
    }
    
    fclose(file);
    printf("[OPENING] Saved %d entries to %s\n", bookSize, filename);
}

// === AUTO-UCZENIE KSIĄŻKI ===

// Deklaracja zewnętrznych funkcji z innych plików
extern int minimax(int depth, int alpha, int beta, int maximizingPlayer, bool isRoot, int originalPlayer);
extern bool winCheck(int who);
extern bool loseCheck(int who);
extern int board[5][5];

// Funkcja rekurencyjna eksploracji pozycji
void explorePosition(const char* currentSequence, int currentPlayer, int depth, int maxDepth, int searchDepth) {
    if (depth > maxDepth) return;
    
    // Sprawdź czy pozycja już jest w książce
    for (int i = 0; i < bookSize; i++) {
        if (strcmp(openingBook[i].sequence, currentSequence) == 0) {
            return; // Już mamy tę pozycję
        }
    }
    
    printf("[LEARN] Depth %d/%d: %s\n", depth, maxDepth, 
           strlen(currentSequence) == 0 ? "(start)" : currentSequence);
    
    int bestMove = 0;
    int bestScore = -100000;
    bool foundWinningMove = false;
    
    // Przeszukaj wszystkie możliwe ruchy
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (board[i][j] == 0) {
                int move = (i + 1) * 10 + (j + 1);
                
                // Wykonaj ruch
                board[i][j] = currentPlayer;
                
                // Sprawdź czy to natychmiastowa wygrana
                if (winCheck(currentPlayer)) {
                    board[i][j] = 0;
                    bestMove = move;
                    bestScore = 10000;
                    foundWinningMove = true;
                    break;
                }
                
                // Sprawdź czy to samobójczy ruch (3 w rzędzie)
                if (loseCheck(currentPlayer)) {
                    board[i][j] = 0;
                    continue; // Pomiń ten ruch
                }
                
                // Oceń pozycję za pomocą minimax
                int score = minimax(searchDepth - 1, -100000, 100000, 3 - currentPlayer, false, currentPlayer);
                
                if (score > bestScore) {
                    bestScore = score;
                    bestMove = move;
                }
                
                board[i][j] = 0; // Cofnij ruch
            }
        }
        if (foundWinningMove) break;
    }
    
    // Dodaj do książki jeśli znaleziono ruch
    if (bestMove != 0) {
        addOpeningEntry(currentSequence, bestMove, bestScore, searchDepth);
        
        // KLUCZOWA ZMIANA: Eksploruj WSZYSTKIE możliwe ruchy przeciwnika
        if (depth < maxDepth) {
            // Przeszukaj wszystkie możliwe odpowiedzi przeciwnika
            for (int i = 0; i < 5; i++) {
                for (int j = 0; j < 5; j++) {
                    if (board[i][j] == 0) {
                        int responseMove = (i + 1) * 10 + (j + 1);
                        
                        // Wykonaj ruch przeciwnika
                        board[i][j] = 3 - currentPlayer;
                        
                        // Sprawdź czy przeciwnik nie wygrał
                        if (winCheck(3 - currentPlayer)) {
                            board[i][j] = 0; // Cofnij ruch
                            continue; // Pomiń - przeciwnik wygrał
                        }
                        
                        // Sprawdź czy przeciwnik nie popełnił samobójstwa
                        if (loseCheck(3 - currentPlayer)) {
                            board[i][j] = 0; // Cofnij ruch
                            continue; // Pomiń - błędny ruch przeciwnika
                        }
                        
                        // Zbuduj nową sekwencję dla ruchu przeciwnika
                        char newSequence[MAX_SEQUENCE_LENGTH];
                        if (strlen(currentSequence) == 0) {
                            sprintf(newSequence, "%d", responseMove);
                        } else {
                            sprintf(newSequence, "%s,%d", currentSequence, responseMove);
                        }
                        
                        // Rekurencyjnie eksploruj naszą odpowiedź na ruch przeciwnika
                        explorePosition(newSequence, currentPlayer, depth + 1, maxDepth, searchDepth);
                        
                        board[i][j] = 0; // Cofnij ruch przeciwnika
                    }
                }
            }
        }
    }
}

void learnOpenings(int maxDepth, int searchDepth, const char* filename) {
    printf("\n=== OPENING BOOK LEARNING ===\n");
    printf("Max depth: %d (limited to %d moves)\n", maxDepth, MAX_OPENING_MOVES);
    printf("Search depth: %d\n", searchDepth);
    printf("Output file: %s\n", filename);
    
#ifdef _OPENMP
    printf("OpenMP threads: %d\n", omp_get_max_threads());
    init_book_lock();
#else
    printf("Single-threaded mode\n");
#endif
    printf("This may take several minutes...\n\n");
    
    // Ograniczenie do MAX_OPENING_MOVES
    if (maxDepth > MAX_OPENING_MOVES) {
        maxDepth = MAX_OPENING_MOVES;
        printf("[LEARN] Limited max depth to %d (MAX_OPENING_MOVES)\n", maxDepth);
    }
    
    initOpeningBook();
    clearMoveHistory();
    
    // KROK 1: Równoległa analiza pierwszych ruchów (najdroższe obliczenia)
    if (maxDepth >= 1) {
        exploreFirstLevelParallel(maxDepth, searchDepth);
    }
    
    // KROK 2: Równoległa analiza dalszych poziomów
    if (maxDepth >= 2) {
        printf("\n[PARALLEL] Analyzing deeper positions with %d threads...\n", 
#ifdef _OPENMP
               omp_get_max_threads()
#else
               1
#endif
        );
        
        // Lista wszystkich pierwszych ruchów do równoległej analizy
        int firstMoves[25];
        int moveCount = 0;
        for (int i = 1; i <= 5; i++) {
            for (int j = 1; j <= 5; j++) {
                firstMoves[moveCount++] = i * 10 + j;
            }
        }
        
        // Równoległa analiza: każdy wątek bierze jeden pierwszy ruch
        // i analizuje wszystkie możliwe odpowiedzi przeciwnika
#ifdef _OPENMP
        #pragma omp parallel for schedule(dynamic, 1)
#endif
        for (int m = 0; m < moveCount; m++) {
            exploreFromFirstMove(firstMoves[m], maxDepth, searchDepth);
        }
    }
    
    // Zapisz książkę
    saveOpeningBook(filename);
    
#ifdef _OPENMP
    cleanup_book_lock();
#endif
    
    printf("\n=== LEARNING COMPLETE ===\n");
    printf("Generated %d opening positions\n", bookSize);
    printf("Book saved to: %s\n", filename);
}

// Równoległa eksploracja pierwszego poziomu (głównych ruchów otwarcia)
void exploreFirstLevelParallel(int maxDepth, int searchDepth) {
    printf("[PARALLEL] Analyzing first moves with %d threads...\n", 
#ifdef _OPENMP
           omp_get_max_threads()
#else
           1
#endif
    );
    
    // Lista wszystkich możliwych pierwszych ruchów
    int firstMoves[25];
    int moveScores[25];
    int moveCount = 0;
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            firstMoves[moveCount] = (i + 1) * 10 + (j + 1);
            moveScores[moveCount] = -100000;
            moveCount++;
        }
    }
    
#ifdef _OPENMP
    #pragma omp parallel for schedule(dynamic, 1)
#endif
    for (int m = 0; m < moveCount; m++) {
        int move = firstMoves[m];
        int row = (move / 10) - 1;
        int col = (move % 10) - 1;
        
        // Każdy wątek ma własną kopię planszy
        int local_board[5][5];
        memcpy(local_board, board, sizeof(board));
        
        // Wykonaj pierwszy ruch
        local_board[row][col] = 1;
        
        // Skopiuj planszę do globalnej (thread-safe)
#ifdef _OPENMP
        omp_set_lock(&book_lock);
#endif
        memcpy(board, local_board, sizeof(board));
        
        // Oceń pozycję
        int score = minimax(searchDepth - 1, -100000, 100000, 2, false, 1);
        
        // Przywróć pustą planszę
        board[row][col] = 0;
#ifdef _OPENMP
        omp_unset_lock(&book_lock);
#endif
        
        // Zapisz wynik w lokalnej tablicy
        moveScores[m] = score;
        
        printf("[PARALLEL] First move %d analyzed: score=%d\n", move, score);
    }
    
    // Po zakończeniu równoległej analizy - znajdź najlepszy ruch
    int bestScore = -100000;
    int bestMove = 0;
    
    for (int i = 0; i < moveCount; i++) {
        if (moveScores[i] > bestScore) {
            bestScore = moveScores[i];
            bestMove = firstMoves[i];
        }
    }
    
    // Dodaj TYLKO JEDEN wpis dla pustej sekwencji - najlepszy pierwszy ruch
    addOpeningEntryThreadSafe("", bestMove, bestScore, searchDepth);
    
    printf("[PARALLEL] Best first move: %d (score: %d)\n", bestMove, bestScore);
}

// Funkcja do eksploracji wszystkich pozycji zaczynających się od danego pierwszego ruchu
void exploreFromFirstMove(int firstMove, int maxDepth, int searchDepth) {
    if (maxDepth < 2) return;
    
    int row = (firstMove / 10) - 1;
    int col = (firstMove % 10) - 1;
    
    // Wykonaj pierwszy ruch na lokalnej kopii planszy
    int localBoard[5][5];
    memcpy(localBoard, board, sizeof(board));
    localBoard[row][col] = 1;
    
    char firstSequence[10];
    sprintf(firstSequence, "%d", firstMove);
    
    // Thread-safe: każdy wątek używa swojej kopii planszy
    // Synchronizacja tylko przy zapisie do książki
    
    // Analizuj wszystkie możliwe odpowiedzi przeciwnika (gracz 2)
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (localBoard[i][j] == 0) {
                int secondMove = (i + 1) * 10 + (j + 1);
                
                // Wykonaj drugi ruch
                localBoard[i][j] = 2;
                
                // Sprawdź czy to nie natychmiastowa wygrana/przegrana
                bool skipMove = false;
                
                // Skopiuj lokalną planszę do globalnej dla sprawdzenia win/lose
#ifdef _OPENMP
                omp_set_lock(&book_lock);
#endif
                memcpy(board, localBoard, sizeof(board));
                
                if (winCheck(2)) {
                    // Przeciwnik wygrał - pomiń tę linię
                    skipMove = true;
                } else if (loseCheck(2)) {
                    // Przeciwnik popełnił samobójczy ruch - pomiń
                    skipMove = true;
                }
                
                if (!skipMove) {
                    // Znajdź najlepszy ruch dla gracza 1
                    int bestMove = 0;
                    int bestScore = -100000;
                    
                    // Przeszukaj wszystkie możliwe trzecie ruchy
                    for (int ii = 0; ii < 5; ii++) {
                        for (int jj = 0; jj < 5; jj++) {
                            if (board[ii][jj] == 0) {
                                int thirdMove = (ii + 1) * 10 + (jj + 1);
                                board[ii][jj] = 1;
                                
                                if (winCheck(1)) {
                                    board[ii][jj] = 0;
                                    bestMove = thirdMove;
                                    bestScore = 10000;
                                    break;
                                } else if (!loseCheck(1)) {
                                    int score = minimax(searchDepth - 1, -100000, 100000, 2, false, 1);
                                    if (score > bestScore) {
                                        bestScore = score;
                                        bestMove = thirdMove;
                                    }
                                }
                                board[ii][jj] = 0;
                            }
                        }
                        if (bestScore == 10000) break;
                    }
                    
                    // Dodaj do książki jeśli znaleziono dobry ruch
                    if (bestMove != 0) {
                        char sequence[50];
                        sprintf(sequence, "%d,%d", firstMove, secondMove);
                        addOpeningEntryThreadSafe(sequence, bestMove, bestScore, searchDepth);
                    }
                }
                
#ifdef _OPENMP
                omp_unset_lock(&book_lock);
#endif
                
                // Cofnij drugi ruch
                localBoard[i][j] = 0;
                
                // UWAGA: Rekurencja dla głębszych poziomów poza lockiem!
                if (maxDepth >= 3 && !skipMove) {
                    char sequence[50];
                    sprintf(sequence, "%d,%d", firstMove, secondMove);
                    
                    // Każdy wątek używa swojej kopii planszy dla rekurencji
                    memcpy(board, localBoard, sizeof(board));
                    board[i-1][j-1] = 2;  // Przywróć drugi ruch
                    explorePosition(sequence, 1, 3, maxDepth, searchDepth);
                    board[i-1][j-1] = 0;  // Cofnij
                }
            }
        }
    }
}

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

// Hash table dla szybkiego wyszukiwania
HashNode* hashTable[HASH_TABLE_SIZE];

// Zewnętrzne definicje z board.h
extern const int win[28][4][2];
extern const int lose[48][3][2];

// === FUNKCJE HASH TABLE ===

// Funkcja hash - djb2 algorithm
unsigned int hash(const char* str) {
    unsigned int hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash % HASH_TABLE_SIZE;
}

// Inicjalizacja hash table
void initHashTable(void) {
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        hashTable[i] = NULL;
    }
}

// Dodaj wpis do hash table
void addToHashTable(const char* sequence, int move, int score, int depth) {
    unsigned int index = hash(sequence);
    
    HashNode* newNode = malloc(sizeof(HashNode));
    if (!newNode) {
        printf("Error: Cannot allocate memory for hash node!\n");
        return;
    }
    
    strcpy(newNode->sequence, sequence);
    newNode->best_move = move;
    newNode->score = score;
    newNode->depth_analyzed = depth;
    newNode->next = hashTable[index];  // Dodaj na początku łańcucha
    hashTable[index] = newNode;
}

// Znajdź wpis w hash table
HashNode* findInHashTable(const char* sequence) {
    unsigned int index = hash(sequence);
    HashNode* current = hashTable[index];
    
    while (current) {
        if (strcmp(current->sequence, sequence) == 0) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;  // Nie znaleziono
}

// Zwolnij pamięć hash table
void freeHashTable(void) {
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        HashNode* current = hashTable[i];
        while (current) {
            HashNode* temp = current;
            current = current->next;
            free(temp);
        }
        hashTable[i] = NULL;
    }
}

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
    initHashTable();  // Inicjalizuj hash table
}

void addOpeningEntry(const char* sequence, int move, int score, int depth) {
    initOpeningBook();
    
    // Konwertuj do kanonicznej formy (uwzględniając symetrie)
    char canonicalSeq[MAX_SEQUENCE_LENGTH];
    int transform;
    getCanonicalSequence(sequence, canonicalSeq, &transform);
    
    // Transformuj też ruch do odpowiedniej formy kanonicznej
    int canonicalMove = transformMove(move, transform);
    
    // Sprawdź czy wpis już istnieje w formie kanonicznej
    for (int i = 0; i < bookSize; i++) {
        if (strcmp(openingBook[i].sequence, canonicalSeq) == 0) {
            if (depth > openingBook[i].depth_analyzed) {
                openingBook[i].best_move = canonicalMove;
                openingBook[i].score = score;
                openingBook[i].depth_analyzed = depth;
                
                // Aktualizuj też hash table
                HashNode* node = findInHashTable(canonicalSeq);
                if (node) {
                    node->best_move = canonicalMove;
                    node->score = score;
                    node->depth_analyzed = depth;
                }
            }
            return;
        }
    }
    
    // Dodaj nowy wpis w formie kanonicznej
    if (bookSize >= bookCapacity) {
        bookCapacity *= 2;
        openingBook = realloc(openingBook, bookCapacity * sizeof(OpeningEntry));
        if (!openingBook) {
            printf("Error: Cannot allocate memory for opening book!\n");
            exit(1);
        }
    }
    
    strcpy(openingBook[bookSize].sequence, canonicalSeq);
    openingBook[bookSize].best_move = canonicalMove;
    openingBook[bookSize].score = score;
    openingBook[bookSize].depth_analyzed = depth;
    bookSize++;
    
    // Dodaj też do hash table dla szybkiego wyszukiwania
    addToHashTable(canonicalSeq, canonicalMove, score, depth);
}

void freeOpeningBook(void) {
    if (openingBook) {
        free(openingBook);
        openingBook = NULL;
        bookSize = 0;
        bookCapacity = 0;
    }
    freeHashTable();  // Zwolnij hash table
}

// === UŻYCIE KSIĄŻKI W GRZE ===

bool isInOpeningPhase(int moveCount) {
    return moveCount <= MAX_OPENING_MOVES;
}

int getOpeningMove(const char* moveSequence, int moveCount) {
    printf("[DEBUG GET] Called with sequence='%s', moveCount=%d\n", moveSequence, moveCount);
    
    // Sprawdź czy jesteśmy w fazie otwarcia
    if (!isInOpeningPhase(moveCount)) {
        printf("[DEBUG GET] Not in opening phase\n");
        return 0; // Poza fazą otwarcia
    }
    
    if (openingBook == NULL || bookSize == 0) {
        printf("[DEBUG GET] No opening book loaded\n");
        return 0; // Brak książki
    }
    
    printf("[DEBUG GET] Opening book has %d entries\n", bookSize);
    
    // Konwertuj sekwencję do formy kanonicznej
    char canonicalSeq[MAX_SEQUENCE_LENGTH];
    int transform;
    getCanonicalSequence(moveSequence, canonicalSeq, &transform);
    
    printf("[DEBUG GET] Canonical sequence: '%s', transform: %d\n", canonicalSeq, transform);
    
    // Szybkie wyszukiwanie w hash table - O(1) zamiast O(n)
    HashNode* node = findInHashTable(canonicalSeq);
    if (node) {
        printf("[DEBUG GET] Found in hash table: move=%d\n", node->best_move);
        // Ruch jest w formie kanonicznej - musimy go odwrócić do oryginalnej orientacji
        int canonicalMove = node->best_move;
        int originalMove = canonicalMove;
        
        // Znajdź odwrotną transformację
        if (transform != 0) {
            // Testuj wszystkie transformacje, żeby znaleźć tę która odwraca kanoniczną
            for (int t = 0; t < 8; t++) {
                int testMove = transformMove(canonicalMove, t);
                char testSeq[MAX_SEQUENCE_LENGTH];
                sprintf(testSeq, "%s,%d", moveSequence, testMove);
                
                char testCanonical[MAX_SEQUENCE_LENGTH];
                int testTransform;
                getCanonicalSequence(testSeq, testCanonical, &testTransform);
                
                // Jeśli po dodaniu ruchu i transformacji dostajemy spójną kanoniczną formę
                if (testTransform == transform) {
                    originalMove = testMove;
                    break;
                }
            }
        }
        
        printf("[OPENING] Using book move %d for sequence: %s (canonical: %s, move: %d)\n", 
               originalMove, moveSequence, canonicalSeq, canonicalMove);
        return originalMove;
    } else {
        printf("[DEBUG GET] Not found in hash table for '%s'\n", canonicalSeq);
        
        // Fallback - szukaj w tablicy liniowo
        for (int i = 0; i < bookSize; i++) {
            if (strcmp(openingBook[i].sequence, canonicalSeq) == 0) {
                printf("[DEBUG GET] Found in linear search: move=%d\n", openingBook[i].best_move);
                return openingBook[i].best_move;
            }
        }
        printf("[DEBUG GET] Not found anywhere\n");
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
        
        printf("[DEBUG LOAD] Processing line: '%s'\n", line);
        
        char sequence[MAX_SEQUENCE_LENGTH];
        int move, score, depth;
        
        // Specjalne parsowanie dla pustej sekwencji
        if (sscanf(line, " -> %d (%d) [%d]", &move, &score, &depth) == 3) {
            // Pusta sekwencja
            addOpeningEntry("", move, score, depth);
            loaded++;
            printf("[DEBUG LOAD] Empty sequence -> %d (score=%d, depth=%d)\n", move, score, depth);
        }
        // Format: "sequence -> move (score) [depth]"
        else if (sscanf(line, "%s -> %d (%d) [%d]", sequence, &move, &score, &depth) == 4) {
            addOpeningEntry(sequence, move, score, depth);
            loaded++;
            printf("[DEBUG LOAD] %s -> %d (score=%d, depth=%d)\n", sequence, move, score, depth);
        } else {
            printf("[DEBUG LOAD] Failed to parse line: '%s'\n", line);
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
        
        // Równoległa analiza: analizuj reprezentatywne sekwencje dla różnych przypadków
        // Pokrywamy główne sytuacje strategiczne:
        int predefinedSequences[][2] = {
            // 1. Gracz1 gra środek, Gracz2 różne odpowiedzi
            {33, 12},  // środek -> lewy górny róg
            {33, 11},  // środek -> drugi róg
            {33, 13},  // środek -> trzeci róg
            
            {33, 21},  // środek -> czwarty róg
            {33, 22},  // środek -> bok górny
            {33, 23},  // środek -> bok lewy
            
            // 2. Gracz1 gra róg, Gracz2 odpowiada środkiem (strategia obronna)
            {11, 33},  // lewy górny róg -> środek (my jako gracz2)
            {12, 33},  // lewy dolny róg -> środek
            {13, 33},  // środek -> środek

            {21, 33},  // prawy górny róg -> środek
            {22, 33},  // prawy dolny 
            {23, 33},  // środek -> środek
        };
        int numSequences = sizeof(predefinedSequences) / sizeof(predefinedSequences[0]);
        
        printf("[PARALLEL] Will analyze %d representative sequences with %d threads...\n", numSequences, 
#ifdef _OPENMP
               omp_get_max_threads()
#else
               1
#endif
        );
        
        // Dodaj licznik postępu
        int completed = 0;

#ifdef _OPENMP
        #pragma omp parallel for schedule(dynamic, 1)
#endif
        for (int s = 0; s < numSequences; s++) {
            printf("[PROGRESS] Starting sequence %d/%d: %d,%d\n", s+1, numSequences, 
                   predefinedSequences[s][0], predefinedSequences[s][1]);
            
            exploreFromPredefinedSequence(predefinedSequences[s][0], predefinedSequences[s][1], maxDepth, searchDepth);
            
#ifdef _OPENMP
            #pragma omp atomic
#endif
            completed++;
            
            printf("[PROGRESS] Completed %d/%d sequences (%.1f%%)\n", 
                   completed, numSequences, (completed * 100.0) / numSequences);
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
    printf("[FIRST LEVEL] Using predefined opening moves (skipping heavy analysis)...\n");
    
    // ZAKODOWANE NAJLEPSZE RUCHY OTWARCIA:
    // 1. Gracz 1 zawsze gra 33 (środek)
    // 2. Gracz 2: jeśli 33 wolne to 33, jeśli zajęte to 22 (lewy górny róg)
    
    // Dodaj podstawowe ruchy do książki bez ciężkiej analizy
    addOpeningEntryThreadSafe("", 33, 5000, 1);  // Pusty -> gracz 1 gra 33
    
    // Odpowiedzi gracza 2 na 33
    addOpeningEntryThreadSafe("33", 14, 4000, 1);  // Jeśli gracz 1 zagrał 33, gracz 2 gra 14 

    // Gdyby gracz 1 nie zagrał 33 (rzadkie), gracz 2 gra środek
    for (int i = 1; i <= 5; i++) {
        for (int j = 1; j <= 5; j++) {
            int move = i * 10 + j;
            if (move != 33) {  // Wszystkie pierwsze ruchy oprócz 33
                char seq[10];
                sprintf(seq, "%d", move);
                addOpeningEntryThreadSafe(seq, 33, 4500, 1);  // Odpowiedź: środek
            }
        }
    }
    
    printf("[FIRST LEVEL] Added %d predefined opening moves\n", bookSize);
}

// === LOKALNE WERSJE FUNKCJI (BEZ MUTEX) ===

bool winCheckLocal(int localBoard[5][5], int player) {
    for (int i = 0; i < 28; i++) {
        if ((localBoard[win[i][0][0]][win[i][0][1]] == player) &&
            (localBoard[win[i][1][0]][win[i][1][1]] == player) &&
            (localBoard[win[i][2][0]][win[i][2][1]] == player) &&
            (localBoard[win[i][3][0]][win[i][3][1]] == player)) {
            return true;
        }
    }
    return false;
}

bool loseCheckLocal(int localBoard[5][5], int player) {
    for (int i = 0; i < 48; i++) {
        if ((localBoard[lose[i][0][0]][lose[i][0][1]] == player) &&
            (localBoard[lose[i][1][0]][lose[i][1][1]] == player) &&
            (localBoard[lose[i][2][0]][lose[i][2][1]] == player)) {
            return true;
        }
    }
    return false;
}

int evaluateBoardLocal(int localBoard[5][5], int player) {
    // Skopiuj logikę z heuristic.c ale używaj localBoard
    int score = 0;
    
    // Preferuj środek planszy
    if (localBoard[2][2] == player) score += 10;
    
    // Oceń pozycję na podstawie kontroli nad polami
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (localBoard[i][j] == player) {
                // Punkty za kontrolę nad polami blisko środka
                int distFromCenter = abs(i - 2) + abs(j - 2);
                score += (5 - distFromCenter);
            } else if (localBoard[i][j] == (3 - player)) {
                // Ujemne punkty za kontrolę przeciwnika
                int distFromCenter = abs(i - 2) + abs(j - 2);
                score -= (5 - distFromCenter);
            }
        }
    }
    
    return score;
}

int minimaxLocal(int localBoard[5][5], int depth, int alpha, int beta, int currentPlayer, bool maximizing, int player) {
    // Sprawdź stany końcowe przed sprawdzaniem głębokości
    if (winCheckLocal(localBoard, player)) return 10000;
    if (winCheckLocal(localBoard, 3 - player)) return -10000;
    if (loseCheckLocal(localBoard, player)) return -10000;
    if (loseCheckLocal(localBoard, 3 - player)) return 10000;
    
    // Sprawdź głębokość
    if (depth == 0) {
        return evaluateBoardLocal(localBoard, player);
    }
    
    int best;
    if (maximizing) {
        best = -100000;
        bool hasLegalMove = false;
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 5; j++) {
                if (localBoard[i][j] == 0) {
                    localBoard[i][j] = currentPlayer;
                    
                    // Sprawdź czy ruch jest legalny
                    bool isLegal = !(loseCheckLocal(localBoard, currentPlayer) && !winCheckLocal(localBoard, currentPlayer));
                    
                    if (isLegal) {
                        hasLegalMove = true;
                        int val = minimaxLocal(localBoard, depth - 1, alpha, beta, 3 - currentPlayer, false, player);
                        localBoard[i][j] = 0;
                        if (val > best) best = val;
                        if (best > alpha) alpha = best;
                        if (beta <= alpha) {
                            return best; // Przycinanie alfa-beta
                        }
                    } else {
                        localBoard[i][j] = 0; // Cofnij nielegalny ruch
                    }
                }
            }
        }
        if (!hasLegalMove) return -10000;
        return best;
    } else {
        best = 100000;
        bool hasLegalMove = false;
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 5; j++) {
                if (localBoard[i][j] == 0) {
                    localBoard[i][j] = currentPlayer;
                    
                    bool isLegal = !(loseCheckLocal(localBoard, currentPlayer) && !winCheckLocal(localBoard, currentPlayer));
                    
                    if (isLegal) {
                        hasLegalMove = true;
                        int val = minimaxLocal(localBoard, depth - 1, alpha, beta, 3 - currentPlayer, true, player);
                        localBoard[i][j] = 0;
                        if (val < best) best = val;
                        if (best < beta) beta = best;
                        if (beta <= alpha) {
                            return best;
                        }
                    } else {
                        localBoard[i][j] = 0;
                    }
                }
            }
        }
        if (!hasLegalMove) return 10000;
        return best;
    }
}

// === FUNKCJE SYMETRII I ROTACJI ===

// Konwersja ruch (row,col) -> int i odwrotnie
typedef struct {
    int row, col;
} Position;

Position moveToPosition(int move) {
    Position pos;
    pos.row = (move / 10) - 1;  // 0-4
    pos.col = (move % 10) - 1;  // 0-4
    return pos;
}

int positionToMove(Position pos) {
    return (pos.row + 1) * 10 + (pos.col + 1);
}

// Transformacje symetrii dla pozycji 5x5
Position rotate90(Position pos) {
    return (Position){pos.col, 4 - pos.row};
}

Position rotate180(Position pos) {
    return (Position){4 - pos.row, 4 - pos.col};
}

Position rotate270(Position pos) {
    return (Position){4 - pos.col, pos.row};
}

Position flipHorizontal(Position pos) {
    return (Position){pos.row, 4 - pos.col};
}

Position flipVertical(Position pos) {
    return (Position){4 - pos.row, pos.col};
}

Position flipDiagonal1(Position pos) {  // główna przekątna
    return (Position){pos.col, pos.row};
}

Position flipDiagonal2(Position pos) {  // anty-przekątna
    return (Position){4 - pos.col, 4 - pos.row};
}

// Tablica wszystkich transformacji
typedef Position (*TransformFunc)(Position);
TransformFunc transforms[8] = {
    NULL,           // identność (brak transformacji)
    rotate90,
    rotate180, 
    rotate270,
    flipHorizontal,
    flipVertical,
    flipDiagonal1,
    flipDiagonal2
};

// Transformuj sekwencję ruchów
void transformSequence(const char* input, char* output, int transformIndex) {
    if (transformIndex == 0 || transforms[transformIndex] == NULL) {
        strcpy(output, input);
        return;
    }
    
    output[0] = '\0';
    
    if (strlen(input) == 0) {
        return;  // Pusta sekwencja
    }
    
    char tempInput[MAX_SEQUENCE_LENGTH];
    strcpy(tempInput, input);
    
    char* token = strtok(tempInput, ",");
    bool first = true;
    
    while (token != NULL) {
        int move = atoi(token);
        Position pos = moveToPosition(move);
        Position newPos = transforms[transformIndex](pos);
        int newMove = positionToMove(newPos);
        
        if (!first) strcat(output, ",");
        char moveStr[10];
        sprintf(moveStr, "%d", newMove);
        strcat(output, moveStr);
        first = false;
        
        token = strtok(NULL, ",");
    }
}

// Transformuj pojedynczy ruch
int transformMove(int move, int transformIndex) {
    if (transformIndex == 0 || transforms[transformIndex] == NULL) {
        return move;
    }
    
    Position pos = moveToPosition(move);
    Position newPos = transforms[transformIndex](pos);
    return positionToMove(newPos);
}

// Znajdź kanoniczną (najmniejszą leksykograficznie) reprezentację sekwencji
void getCanonicalSequence(const char* input, char* canonical, int* bestTransform) {
    strcpy(canonical, input);
    *bestTransform = 0;
    
    for (int t = 1; t < 8; t++) {
        char transformed[MAX_SEQUENCE_LENGTH];
        transformSequence(input, transformed, t);
        
        if (strcmp(transformed, canonical) < 0) {
            strcpy(canonical, transformed);
            *bestTransform = t;
        }
    }
}

// Funkcja do eksploracji wszystkich pozycji zaczynających się od danego pierwszego ruchu
// Funkcja rekurencyjna dla eksploracji głębszych poziomów (thread-safe)
void exploreRecursive(int localBoard[5][5], const char* currentSequence, int currentPlayer, 
                     int depth, int maxDepth, int searchDepth) {
    if (depth > maxDepth) return;
    
    // Progress tracking - pokaż aktualną analizę
    static int depth1_completed = 0;
    static int depth2_completed = 0;
    
    if (depth == 1) {
#ifdef _OPENMP
        #pragma omp atomic
#endif
        depth1_completed++;
        
        if (depth1_completed % 10 == 0) {
            printf("[PROGRESS DEEP] Level 1: Completed %d positions, analyzing: %s\n", 
                   depth1_completed, currentSequence);
        }
    }
    
    if (depth == 2) {
#ifdef _OPENMP
        #pragma omp atomic
#endif
        depth2_completed++;
        
        if (depth2_completed % 50 == 0) {
            printf("[PROGRESS DEEP] Level 2: Completed %d positions, current: %s\n", 
                   depth2_completed, currentSequence);
        }
    }
    
    // Znajdź najlepszy ruch dla aktualnego gracza
    int bestMove = 0;
    int bestScore = -100000;
    bool foundWinningMove = false;
    
    // Przeszukaj wszystkie możliwe ruchy
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (localBoard[i][j] == 0) {
                int move = (i + 1) * 10 + (j + 1);
                
                // Wykonaj ruch
                localBoard[i][j] = currentPlayer;
                
                // Sprawdź czy to natychmiastowa wygrana
                if (winCheckLocal(localBoard, currentPlayer)) {
                    localBoard[i][j] = 0;
                    bestMove = move;
                    bestScore = 10000;
                    foundWinningMove = true;
                    break;
                }
                
                // Sprawdź czy to samobójczy ruch (3 w rzędzie)
                if (loseCheckLocal(localBoard, currentPlayer)) {
                    localBoard[i][j] = 0;
                    continue; // Pomiń ten ruch
                }
                
                // Oceń pozycję za pomocą minimax z pełną głębokością
                int score = minimaxLocal(localBoard, searchDepth - 1, -100000, 100000, 
                                        3 - currentPlayer, false, currentPlayer);
                
                if (score > bestScore) {
                    bestScore = score;
                    bestMove = move;
                }
                
                localBoard[i][j] = 0; // Cofnij ruch
            }
        }
        if (foundWinningMove) break;
    }
    
    // Dodaj do książki jeśli znaleziono ruch
    if (bestMove != 0) {
        addOpeningEntryThreadSafe(currentSequence, bestMove, bestScore, searchDepth);
        
        // OPTYMALIZACJA: Kontynuuj rekurencję tylko dla NAJLEPSZYCH 3-5 odpowiedzi przeciwnika
        if (depth < maxDepth) {
            // Wykonaj najlepszy ruch
            int bestRow = (bestMove / 10) - 1;
            int bestCol = (bestMove % 10) - 1;
            localBoard[bestRow][bestCol] = currentPlayer;
            
            // Znajdź najlepsze odpowiedzi przeciwnika (max 5 zamiast wszystkich 20+)
            typedef struct {
                int move;
                int score;
            } MoveScore;
            
            MoveScore topMoves[8]; // Top 8 ruchów przeciwnika
            int topCount = 0;
            int candidatesEvaluated = 0;
            
            // Oceń wszystkie możliwe odpowiedzi
            for (int i = 0; i < 5; i++) {
                for (int j = 0; j < 5; j++) {
                    if (localBoard[i][j] == 0) {
                        int responseMove = (i + 1) * 10 + (j + 1);
                        candidatesEvaluated++;
                        
                        // Progress dla preselekcji (która jest najwolniejsza)
                        if (depth <= 2) {
                            printf("[MINIMAX EVAL] Depth %d: Evaluating candidate move %d (%d) for sequence: %s\n", 
                                   depth, candidatesEvaluated, responseMove, currentSequence);
                        }
                        
                        // Wykonaj ruch przeciwnika
                        localBoard[i][j] = 3 - currentPlayer;
                        
                        // Sprawdź czy przeciwnik nie wygrał lub popełnił błąd
                        if (winCheckLocal(localBoard, 3 - currentPlayer) || 
                            loseCheckLocal(localBoard, 3 - currentPlayer)) {
                            localBoard[i][j] = 0; // Cofnij ruch
                            continue; // Pomiń ten ruch
                        }
                        
                        // Oceń pozycję z większą głębokością (połowa pełnej głębokości)
                        int preselectDepth = searchDepth / 2;
                        if (preselectDepth < 3) preselectDepth = 3;
                         int score = minimaxLocal(localBoard, preselectDepth, -100000, 100000, 
                                                currentPlayer, true, 3 - currentPlayer);

                        // Dodaj do top 8 jeśli warto
                        if (topCount < 8) {
                            topMoves[topCount].move = responseMove;
                            topMoves[topCount].score = score;
                            topCount++;
                        } else {
                            // Znajdź najgorszy z top 8 i zastąp jeśli lepszy
                            int worstIdx = 0;
                            for (int k = 1; k < 8; k++) {
                                if (topMoves[k].score < topMoves[worstIdx].score) {
                                    worstIdx = k;
                                }
                            }
                            if (score > topMoves[worstIdx].score) {
                                topMoves[worstIdx].move = responseMove;
                                topMoves[worstIdx].score = score;
                            }
                        }
                        
                        localBoard[i][j] = 0; // Cofnij ruch przeciwnika
                    }
                }
            }
            
            // Rekurencyjnie eksploruj tylko top ruchy
            for (int t = 0; t < topCount; t++) {
                int responseMove = topMoves[t].move;
                int row = (responseMove / 10) - 1;
                int col = (responseMove % 10) - 1;
                
                localBoard[row][col] = 3 - currentPlayer;
                
                char newSequence[MAX_SEQUENCE_LENGTH];
                sprintf(newSequence, "%s,%d", currentSequence, responseMove);
                
                exploreRecursive(localBoard, newSequence, currentPlayer, 
                               depth + 1, maxDepth, searchDepth);
                
                localBoard[row][col] = 0;
            }
            
            // Cofnij najlepszy ruch
            localBoard[bestRow][bestCol] = 0;
        }
    }
}

void exploreFromFirstMove(int firstMove, int maxDepth, int searchDepth) {
    if (maxDepth < 2) return;
    
    int row = (firstMove / 10) - 1;
    int col = (firstMove % 10) - 1;
    
    // Każdy wątek ma własną kopię planszy
    int localBoard[5][5];
    memcpy(localBoard, board, sizeof(board));
    localBoard[row][col] = 1;  // Pierwszy ruch
    
    char firstSequence[5];
    sprintf(firstSequence, "%d", firstMove);
    
    // Analizuj wszystkie możliwe odpowiedzi przeciwnika (gracz 2)
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (localBoard[i][j] == 0) {
                int secondMove = (i + 1) * 10 + (j + 1);
                
                // Wykonaj drugi ruch na lokalnej planszy
                localBoard[i][j] = 2;
                
                // Sprawdź czy to nie natychmiastowa wygrana/przegrana
                bool skipMove = false;
                
                if (winCheckLocal(localBoard, 2)) {
                    skipMove = true;  // Przeciwnik wygrał
                } else if (loseCheckLocal(localBoard, 2)) {
                    skipMove = true;  // Przeciwnik popełnił samobójczy ruch
                }
                
                if (!skipMove) {
                    // Zbuduj sekwencję dwóch ruchów
                    char sequence[50];
                    sprintf(sequence, "%d,%d", firstMove, secondMove);
                    
                    // Użyj funkcji rekurencyjnej dla dalszej eksploracji
                    exploreRecursive(localBoard, sequence, 1, 2, maxDepth, searchDepth);
                }
                
                // Cofnij drugi ruch
                localBoard[i][j] = 0;
            }
        }
    }
}

// Eksploruje dalsze ruchy dla predefiniowanej sekwencji 2 ruchów
void exploreFromPredefinedSequence(int firstMove, int secondMove, int maxDepth, int searchDepth) {
    if (maxDepth < 3) return;  // Potrzebujemy przynajmniej 3 ruchy
    
    printf("[DEEP ANALYSIS] Exploring from sequence %d,%d to depth %d\n", firstMove, secondMove, maxDepth);
    
    // Przygotuj planszę z dwoma pierwszymi ruchami
    int localBoard[5][5];
    memcpy(localBoard, board, sizeof(board));
    
    // Wykonaj pierwszy ruch (gracz 1)
    int row1 = (firstMove / 10) - 1;
    int col1 = (firstMove % 10) - 1;
    localBoard[row1][col1] = 1;
    
    // Wykonaj drugi ruch (gracz 2)  
    int row2 = (secondMove / 10) - 1;
    int col2 = (secondMove % 10) - 1;
    localBoard[row2][col2] = 2;
    
    // Zbuduj sekwencję startową
    char startSequence[50];
    sprintf(startSequence, "%d,%d", firstMove, secondMove);
    
    // Rozpocznij rekurencyjną eksplorację od trzeciego ruchu (gracz 1)
    exploreRecursive(localBoard, startSequence, 1, 2, maxDepth, searchDepth);
}

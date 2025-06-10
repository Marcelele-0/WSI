#ifndef OPENING_BOOK_H
#define OPENING_BOOK_H

#include <stdbool.h>

#define MAX_OPENING_MOVES 10
#define MAX_SEQUENCE_LENGTH 100
#define MAX_BOOK_ENTRIES 10000

// Struktura wpisu w książce otwarć
typedef struct {
    char sequence[MAX_SEQUENCE_LENGTH];  // "33,22,43" - sekwencja ruchów
    int best_move;                       // Najlepszy ruch dla tej sekwencji
    int score;                          // Ocena minimax dla tego ruchu
    int depth_analyzed;                 // Głębokość analizy użyta
} OpeningEntry;

// === GŁÓWNE FUNKCJE ===

// Ładowanie/zapisywanie książki
bool loadOpeningBook(const char* filename);
void saveOpeningBook(const char* filename);

// Użycie książki w grze
int getOpeningMove(const char* moveSequence, int moveCount);
bool isInOpeningPhase(int moveCount);

// Auto-uczenie książki
void learnOpenings(int maxDepth, int searchDepth, const char* filename);
void exploreFirstLevelParallel(int maxDepth, int searchDepth);  // Parallel learning

// === FUNKCJE POMOCNICZE ===

// Zarządzanie historią ruchów
void clearMoveHistory(void);
void addMoveToHistory(int move);
char* buildMoveSequence(void);
int getMoveCount(void);

// Zarządzanie książką
void initOpeningBook(void);
void addOpeningEntry(const char* sequence, int move, int score, int depth);
void freeOpeningBook(void);

// === ZMIENNE GLOBALNE ===
extern OpeningEntry* openingBook;
extern int bookSize;
extern int bookCapacity;
extern int moveHistory[25];  // Historia ruchów (max 25 w grze 5x5)
extern int historyLength;

#endif // OPENING_BOOK_H

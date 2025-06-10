#ifndef OPENING_BOOK_H
#define OPENING_BOOK_H

#include <stdbool.h>

#define MAX_OPENING_MOVES 10
#define MAX_SEQUENCE_LENGTH 100
#define MAX_BOOK_ENTRIES 10000
#define HASH_TABLE_SIZE 100003  // Liczba pierwsza dla lepszego hash'owania

// Struktura wpisu w książce otwarć
typedef struct {
    char sequence[MAX_SEQUENCE_LENGTH];  // "33,22,43" - sekwencja ruchów
    int best_move;                       // Najlepszy ruch dla tej sekwencji
    int score;                          // Ocena minimax dla tego ruchu
    int depth_analyzed;                 // Głębokość analizy użyta
} OpeningEntry;

// Struktura węzła hash table
typedef struct HashNode {
    char sequence[MAX_SEQUENCE_LENGTH];
    int best_move;
    int score;
    int depth_analyzed;
    struct HashNode* next;  // Dla obsługi kolizji (chaining)
} HashNode;

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
void exploreFromFirstMove(int firstMove, int maxDepth, int searchDepth);  // Parallel deeper analysis
void exploreFromPredefinedSequence(int firstMove, int secondMove, int maxDepth, int searchDepth);  // Deep analysis from 2-move sequence
void exploreRecursive(int localBoard[5][5], const char* currentSequence, int currentPlayer, 
                     int depth, int maxDepth, int searchDepth);  // Thread-safe recursive exploration

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

// Hash table functions (wewnętrzne - dla optymalizacji)
void initHashTable(void);
void addToHashTable(const char* sequence, int move, int score, int depth);
HashNode* findInHashTable(const char* sequence);
void freeHashTable(void);
unsigned int hash(const char* str);

// Funkcje symetrii i rotacji (optymalizacja przez redukcję pozycji)
void getCanonicalSequence(const char* input, char* canonical, int* bestTransform);
void transformSequence(const char* input, char* output, int transformIndex);
int transformMove(int move, int transformIndex);

// === ZMIENNE GLOBALNE ===
extern OpeningEntry* openingBook;
extern int bookSize;
extern int bookCapacity;
extern int moveHistory[25];  // Historia ruchów (max 25 w grze 5x5)
extern int historyLength;

#endif // OPENING_BOOK_H

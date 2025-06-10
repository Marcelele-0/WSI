#ifndef OPENING_BOOK_H
#define OPENING_BOOK_H

#include <stdbool.h>

#define MAX_MOVES 10
#define MAX_POSITIONS 1000

typedef struct {
    int moves[MAX_MOVES];  // Sekwencja ruchów
    int move_count;        // Ile ruchów w sekwencji
    int best_response;     // Najlepszy następny ruch
    int score;            // Ocena pozycji
} BookEntry;

typedef struct {
    BookEntry entries[MAX_POSITIONS];
    int entry_count;
} OpeningBook;

// Funkcje opening book
void generateOpeningBook(int max_depth, int max_moves);
bool loadOpeningBook(const char* filename);
int getBookMove(int moves[], int move_count);
void saveOpeningBook(const char* filename);
int getBestFirstMove();

// Globalna książka otwarć
extern OpeningBook opening_book;

#endif // OPENING_BOOK_H

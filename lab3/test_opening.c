#include "opening_book.h"
#include "board.h"
#include <stdio.h>

int main() {
    printf("=== TESTING OPENING BOOK LOADING ===\n");
    
    setBoard();  // Inicjalizuj planszę
    
    bool result = loadOpeningBook("opening_book_2.txt");
    printf("Load result: %s\n", result ? "SUCCESS" : "FAILED");
    
    // Test dla pustej sekwencji
    printf("\n=== TESTING EMPTY SEQUENCE ===\n");
    int move = getOpeningMove("", 0);
    printf("Move for empty sequence: %d\n", move);
    
    freeOpeningBook();
    return 0;
}

#ifndef HEURISTIC_H
#define HEURISTIC_H

#include <stdbool.h>

// Deklaracje funkcji heurystycznych
int evaluateBoard(int who);
int minimax(int depth, int alpha, int beta, int currentPlayer, bool maximizing, int player);

#endif // HEURISTIC_H

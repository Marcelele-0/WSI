#include "heuristic.h"
#include <stdbool.h>

// Deklaracje zewnętrzne - zmienne są zdefiniowane w board.h
extern int board[5][5];
extern const int win[28][4][2];
extern const int lose[48][3][2];

// Deklaracje funkcji z board.h
extern bool winCheck(int who);
extern bool loseCheck(int who);

// Funkcja oceny planszy dla gracza 'who'
int evaluateBoard(int who) {
    // TODO: Nowa heurystyka
    return 0;
}

// Algorytm minimax z przycinaniem alfa-beta - POPRAWIONY
int minimax(int depth, int alpha, int beta, int currentPlayer, bool maximizing, int player) {
    // Sprawdź stany końcowe przed sprawdzaniem głębokości
    if (winCheck(player)) return 10000;        // Wygrana gracza
    if (winCheck(3 - player)) return -10000;   // Wygrana przeciwnika
    if (loseCheck(player)) return -10000;      // Przegrana gracza (3 w rzędzie)
    if (loseCheck(3 - player)) return 10000;   // Przegrana przeciwnika (3 w rzędzie)
    
    // Sprawdź głębokość
    if (depth == 0) {
        return evaluateBoard(player);
    }
    
    int best;
    if (maximizing) {
        best = -100000;
        bool hasLegalMove = false;
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 5; j++) {
                if (board[i][j] == 0) {
                    board[i][j] = currentPlayer;
                    
                    // Sprawdź czy ruch jest legalny
                    bool isLegal = !(loseCheck(currentPlayer) && !winCheck(currentPlayer));
                    
                    if (isLegal) {
                        hasLegalMove = true;
                        int val = minimax(depth - 1, alpha, beta, 3 - currentPlayer, false, player);
                        board[i][j] = 0;
                        if (val > best) best = val;
                        if (best > alpha) alpha = best;
                        if (beta <= alpha) {
                            return best; // Przycinanie alfa-beta
                        }
                    } else {
                        board[i][j] = 0; // Cofnij nielegalny ruch
                    }
                }
            }
        }
        // Jeśli nie ma legalnych ruchów, to przegrana
        if (!hasLegalMove) return -10000;
        return best;
    } else {
        best = 100000;
        bool hasLegalMove = false;
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 5; j++) {
                if (board[i][j] == 0) {
                    board[i][j] = currentPlayer;
                    
                    // Sprawdź czy ruch jest legalny
                    bool isLegal = !(loseCheck(currentPlayer) && !winCheck(currentPlayer));
                    
                    if (isLegal) {
                        hasLegalMove = true;
                        int val = minimax(depth - 1, alpha, beta, 3 - currentPlayer, true, player);
                        board[i][j] = 0;
                        if (val < best) best = val;
                        if (best < beta) beta = best;
                        if (beta <= alpha) {
                            return best; // Przycinanie alfa-beta
                        }
                    } else {
                        board[i][j] = 0; // Cofnij nielegalny ruch
                    }
                }
            }
        }
        // Jeśli nie ma legalnych ruchów, to przegrana
        if (!hasLegalMove) return 10000; // Przeciwnik przegrywa = my wygrywamy
        return best;
    }
}

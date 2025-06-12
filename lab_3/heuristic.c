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
    int score = 0;
    int opponent = 3 - who;
    
    // 0. ANALIZA STANU GRY: Policz puste pola (końcówka)
    int empty_fields = 0;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (board[i][j] == 0) empty_fields++;
        }
    }
    bool is_endgame = (empty_fields <= 8);  // Końcówka gdy ≤8 pustych pól
    
    // 1. NAJWYŻSZA WAGA: Natychmiastowa wygrana (+10000)
    if (winCheck(who)) return 10000;
    if (winCheck(opponent)) return -10000;
    
    // 2. NAJWYŻSZA WAGA: Natychmiastowa przegrana (-10000)
    if (loseCheck(who)) return -10000;
    if (loseCheck(opponent)) return 10000;
    
    // 3. BARDZO WYSOKA WAGA: Sprawdź możliwości wygranej w następnym ruchu
    int my_win_threats = 0;
    int opp_win_threats = 0;
    int my_forced_losses = 0;  // Sytuacje gdzie mogę być zmuszony do 3 w rzędzie
    int opp_forced_losses = 0; // Sytuacje gdzie przeciwnik może być zmuszony
    
    // Sprawdź wszystkie wzorce 4 w rzędzie
    for (int i = 0; i < 28; i++) {
        int my_count = 0, opp_count = 0, empty_count = 0;
        for (int j = 0; j < 4; j++) {
            int row = win[i][j][0];
            int col = win[i][j][1];
            if (board[row][col] == who) my_count++;
            else if (board[row][col] == opponent) opp_count++;
            else empty_count++;
        }
        
        // Sprawdź zagrożenia wygranej (3 moje + 1 puste)
        if (my_count == 3 && empty_count == 1 && opp_count == 0) {
            my_win_threats++;
            score += 5000;  // Mogę wygrać w następnym ruchu
        }
        
        // Sprawdź zagrożenia przeciwnika (3 jego + 1 puste)
        if (opp_count == 3 && empty_count == 1 && my_count == 0) {
            opp_win_threats++;
            score -= 3000;  // Muszę blokować
        }
        
        // ANALIZA PRZYMUSU: Sprawdź czy ktoś będzie zmuszony do ruchu
        if (my_count == 2 && empty_count == 1 && opp_count == 1) {
            // Mam 2, przeciwnik zablokował 1 pozycję, zostaje mi 1 opcja
            if (is_endgame) {
                my_forced_losses++; // W końcówce mogę być zmuszony do złego ruchu
                score -= 400;
            } else {
                score -= 100; // W mid-game mniej groźne
            }
        }
        
        if (opp_count == 2 && empty_count == 1 && my_count == 1) {
            // Przeciwnik ma 2, ja zablokowałem 1, zostaje mu 1 opcja
            if (is_endgame) {
                opp_forced_losses++; // Przeciwnik może być zmuszony
                score += 300;
            } else {
                score += 80;
            }
        }
        
        // Sprawdź budowanie pozycji (2 + 2 puste)
        if (my_count == 2 && empty_count == 2 && opp_count == 0) {
            if (is_endgame) {
                score += 150;  // W końcówce mniej agresywnie
            } else {
                score += 200;  // W mid-game buduj pozycję
            }
        }
        if (opp_count == 2 && empty_count == 2 && my_count == 0) {
            score -= 100;   // Przeciwnik buduje pozycję
        }
    }
    
    // 4. BARDZO WYSOKA WAGA: Wykryj widełki (2+ zagrożenia jednocześnie)
    if (my_win_threats >= 2) {
        score += 9999;  // WIDEŁKI! Nie do obrony
    }
    if (opp_win_threats >= 2) {
        score -= 9998;  // Przeciwnik ma widełki - prawdopodobnie przegraliśmy
    }
    
    // 5. KOŃCÓWKA: Analiza parity (kto gra ostatni)
    if (is_endgame) {
        bool i_play_last = (empty_fields % 2 == 1); // Jeśli nieparzysta liczba pól = ja gram ostatni
        
        if (my_forced_losses > opp_forced_losses) {
            if (i_play_last) {
                score -= 500; // Ja mam więcej pułapek I gram ostatni = źle
            } else {
                score -= 200; // Ja mam więcej pułapek ale przeciwnik gra ostatni
            }
        } else if (opp_forced_losses > my_forced_losses) {
            if (!i_play_last) {
                score += 400; // Przeciwnik ma więcej pułapek I gra ostatni = dobrze
            } else {
                score += 150; // Przeciwnik ma więcej pułapek ale ja gram ostatni
            }
        }
    }
    
    // 5. ŚREDNIA WAGA: Sprawdź niebezpieczne wzorce 3 w rzędzie
    for (int i = 0; i < 48; i++) {
        int my_count = 0, opp_count = 0;
        for (int j = 0; j < 3; j++) {
            int row = lose[i][j][0];
            int col = lose[i][j][1];
            if (board[row][col] == who) my_count++;
            else if (board[row][col] == opponent) opp_count++;
        }
        
        // Sprawdź ryzyko 3 w rzędzie
        if (my_count == 2) score -= 150;    // Ryzyko stworzenia 3 w rzędzie
        if (opp_count == 2) score += 100;   // Przeciwnik w ryzyku
    }
    
    // 6. NISKA WAGA: Analiza wzorców rozwoju (kontrola opcji)
    for (int i = 0; i < 28; i++) {
        int my_count = 0, opp_count = 0, empty_count = 0;
        for (int j = 0; j < 4; j++) {
            int row = win[i][j][0];
            int col = win[i][j][1];
            if (board[row][col] == who) my_count++;
            else if (board[row][col] == opponent) opp_count++;
            else empty_count++;
        }
        
        // Analiza moich wzorców rozwoju
        if (my_count == 2 && opp_count == 0) {
            if (empty_count >= 2) {
                score += 50;  // Bezpieczny wzorzec - mam wybór opcji
            } else if (empty_count == 1) {
                score -= 30;  // Niebezpieczny - tylko jedna opcja (ryzyko XXX)
            }
        }
        
        if (my_count == 1 && opp_count == 0 && empty_count == 3) {
            score += 15;  // Dobry start - dużo opcji rozwoju
        }
        
        // Analiza wzorców przeciwnika (blokowanie jego opcji)
        if (opp_count == 2 && my_count == 0) {
            if (empty_count >= 2) {
                score -= 40;  // Przeciwnik ma bezpieczny wzorzec
            } else if (empty_count == 1) {
                score += 20;  // Przeciwnik w pułapce - tylko jedna opcja
            }
        }
        
        if (opp_count == 1 && my_count == 0 && empty_count == 3) {
            score -= 10;  // Przeciwnik ma dobre opcje rozwoju
        }
        
        // Premiuj dzielenie linii przeciwnika (blokowanie)
        if (opp_count >= 1 && my_count >= 1) {
            score += 25;  // Podzieliłem jego linię
        }
    }
    
    // 7. BARDZO NISKA WAGA: Kontrola centrum planszy
    if (board[2][2] == who) score += 25;
    if (board[2][2] == opponent) score -= 20;
    
    // Sprawdź pola obok centrum
    int center_positions[][2] = {{1,1}, {1,2}, {1,3}, {2,1}, {2,3}, {3,1}, {3,2}, {3,3}};
    for (int i = 0; i < 8; i++) {
        int row = center_positions[i][0];
        int col = center_positions[i][1];
        if (board[row][col] == who) score += 5;
        if (board[row][col] == opponent) score -= 3;
    }
    
    return score;
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

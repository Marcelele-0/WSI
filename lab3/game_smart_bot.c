#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "board.h"   // Deklaruje: board[5][5], setBoard(), setMove(), winCheck(), loseCheck()

#define INF 1000000  // Wielka wartość reprezentująca "nieskończoność" w algorytmie

int playerId, oppId, searchDepth;
char playerName[10];

/* Funkcja oceny heurystycznej stanu planszy */
int evaluateBoard() {
    int score = 0;
    // Sprawdzamy wszystkie możliwe segmenty długości 4: kierunki (0,1), (1,0), (1,1), (1,-1)
    int dr[4] = {0, 1, 1, 1};
    int dc[4] = {1, 0, 1,-1};
    for(int r = 0; r < 5; r++){
        for(int c = 0; c < 5; c++){
            for(int d = 0; d < 4; d++){
                int endR = r + dr[d]*3;
                int endC = c + dc[d]*3;
                // Sprawdź, czy segment nie wychodzi poza planszę
                if(endR < 0 || endR > 4 || endC < 0 || endC > 4) continue;
                int countMy = 0, countOpp = 0;
                for(int i = 0; i < 4; i++){
                    int nr = r + dr[d]*i;
                    int nc = c + dc[d]*i;
                    if(board[nr][nc] == playerId) countMy++;
                    else if(board[nr][nc] == oppId) countOpp++;
                }
                // Jeśli nie ma symboli przeciwnika w tej linii:
                if(countOpp == 0) {
                    if(countMy == 3) {
                        // Dokładnie 3 nasze symbole w linii: kara (przegrana)
                        score -= 1000;
                    } else {
                        // Dodajemy punkty = 50 * liczba naszych symboli (mierzy rozwój ciągu)
                        score += 50 * countMy;
                    }
                }
                // Jeśli nie ma naszych symboli w tej linii:
                if(countMy == 0) {
                    if(countOpp == 3) {
                        // Przeciwnik ma 3 w linii: dobry stan dla nas (przeciwnik przegrał)
                        score += 1000;
                    } else {
                        // Kara = 50 * liczba symboli przeciwnika (potencjalne zagrożenie)
                        score -= 50 * countOpp;
                    }
                }
            }
        }
    }
    return score;
}

/* Rekurencyjny algorytm minimax z przycinaniem alfa-beta */
int minimax(int depth, int alpha, int beta, bool isMaximizing) {
    // Terminalne sprawdzenie końca gry:
    // Jeśli wygrywa nasz gracz lub przeciwnik uzyskał 3 (czyli przegrał):
    if(winCheck(playerId) || loseCheck(oppId)) {
        return +INF;  // Nasze zwycięstwo
    }
    if(loseCheck(playerId) || winCheck(oppId)) {
        return -INF;  // Nasza porażka
    }
    // Jeśli osiągnięta głębokość lub nie ma już ruchów, zwracamy ocenę heurystyczną:
    if(depth == 0) {
        return evaluateBoard();
    }
    if(isMaximizing) {
        int bestVal = -INF;
        // Maksymalizujemy nasz ruch:
        for(int i = 0; i < 5; i++){
            for(int j = 0; j < 5; j++){
                if(board[i][j] == 0) {
                    setMove(i, j, playerId);
                    int val = minimax(depth - 1, alpha, beta, false);
                    setMove(i, j, 0);  // Cofnięcie ruchu (pusty)
                    if(val > bestVal) bestVal = val;
                    if(bestVal > alpha) alpha = bestVal;
                    if(beta <= alpha) {
                        // Pruning: dalsze dzieci nie muszą być brane pod uwagę
                        i = 5; j = 5;  // wyjście z obu pętli
                        break;
                    }
                }
            }
        }
        return bestVal;
    } else {
        int bestVal = INF;
        // Minimalizujemy dla ruchu przeciwnika:
        for(int i = 0; i < 5; i++){
            for(int j = 0; j < 5; j++){
                if(board[i][j] == 0) {
                    setMove(i, j, oppId);
                    int val = minimax(depth - 1, alpha, beta, true);
                    setMove(i, j, 0);
                    if(val < bestVal) bestVal = val;
                    if(bestVal < beta) beta = bestVal;
                    if(beta <= alpha) {
                        i = 5; j = 5; 
                        break;
                    }
                }
            }
        }
        return bestVal;
    }
}

/* Znajdź najlepszy ruch (root minimax + losowe rozstrzyganie remisów) */
void findBestMove(int *outRow, int *outCol) {
    int bestVal = -INF;
    int bestMovesCount = 0;
    int bestMoves[25][2];
    // Przeszukujemy wszystkie legalne ruchy:
    for(int i = 0; i < 5; i++){
        for(int j = 0; j < 5; j++){
            if(board[i][j] == 0) {
                setMove(i, j, playerId);
                int moveVal = minimax(searchDepth - 1, -INF, INF, false);
                setMove(i, j, 0);
                if(moveVal > bestVal) {
                    bestVal = moveVal;
                    bestMovesCount = 0;
                    bestMoves[bestMovesCount][0] = i;
                    bestMoves[bestMovesCount][1] = j;
                    bestMovesCount++;
                } else if(moveVal == bestVal) {
                    // remis: kolejny dobry ruch
                    bestMoves[bestMovesCount][0] = i;
                    bestMoves[bestMovesCount][1] = j;
                    bestMovesCount++;
                }
            }
        }
    }
    // Losowe rozstrzygnięcie między najlepszymi
    if(bestMovesCount > 0) {
        int choice = rand() % bestMovesCount;
        *outRow = bestMoves[choice][0];
        *outCol = bestMoves[choice][1];
    } else {
        // Brak ruchu (powinno być zablokowane wcześniej)
        *outRow = -1;
        *outCol = -1;
    }
}

int main(int argc, char *argv[]) {
    if(argc < 6) {
        printf("Użycie: %s <IP> <port> <player(1/2)> <name> <depth>\n", argv[0]);
        return 1;
    }
    // Parsowanie argumentów
    char *ip = argv[1];
    int port = atoi(argv[2]);
    playerId = atoi(argv[3]);
    strncpy(playerName, argv[4], 9);
    searchDepth = atoi(argv[5]);
    oppId = (playerId == 1 ? 2 : 1);

    srand(time(NULL));
    // Inicjalizacja połączenia TCP
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &serv_addr.sin_addr);
    if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
        perror("Błąd połączenia");
        return 1;
    }
    // (Opcjonalnie można wysłać nazwę gracza, jeśli wymaga protokół)

    // Jeżeli jesteśmy graczem 1, wykonujemy pierwszy ruch
    if(playerId == 1) {
        // Liczymy dotychczasowe ruchy (brak ruchów)
        int row = 2, col = 2;
        // Zasada książki otwarć: pierwszy ruch - środek
        setMove(row, col, playerId);
        char msg[32];
        sprintf(msg, "MOVE %d %d\n", row, col);
        send(sock, msg, strlen(msg), 0);
    }

    // Pętla gry: czytamy ruchy przeciwnika, wykonujemy swoje
    while(true) {
        char buffer[64];
        memset(buffer, 0, sizeof(buffer));
        int n = recv(sock, buffer, 63, 0);
        if(n <= 0) break;  // brak danych lub rozłączenie
        buffer[n] = '\0';

        if(strncmp(buffer, "MOVE", 4) == 0) {
            int r, c;
            sscanf(buffer, "MOVE %d %d", &r, &c);
            // Wprowadzamy ruch przeciwnika
            setMove(r, c, oppId);
            // Sprawdzamy książkę otwarć (gdy to nasz pierwszy ruch jako gracz 2)
            int moveR = -1, moveC = -1;
            int movesCount = 0;
            for(int i = 0; i < 5; i++)
                for(int j = 0; j < 5; j++)
                    if(board[i][j] != 0) movesCount++;
            // Jeśli przeciwnik wykonał pierwszy ruch i jesteśmy drugim graczem:
            if(movesCount == 1 && playerId == 2) {
                // Jeśli środek wolny, zajmujemy go, inaczej randomowy róg
                if(board[2][2] == 0) {
                    moveR = 2; moveC = 2;
                } else {
                    int corners[4][2] = {{0,0},{0,4},{4,0},{4,4}};
                    int available[4], cnt = 0;
                    for(int k = 0; k < 4; k++){
                        int rr = corners[k][0], cc = corners[k][1];
                        if(board[rr][cc] == 0) {
                            available[cnt++] = k;
                        }
                    }
                    if(cnt > 0) {
                        int choice = rand() % cnt;
                        moveR = corners[available[choice]][0];
                        moveC = corners[available[choice]][1];
                    }
                }
            }
            // Jeśli nie zastosowaliśmy książki, używamy minimax
            if(moveR == -1) {
                findBestMove(&moveR, &moveC);
            }
            // Wykonujemy nasz ruch
            if(moveR != -1) {
                setMove(moveR, moveC, playerId);
                char msg[32];
                sprintf(msg, "MOVE %d %d\n", moveR, moveC);
                send(sock, msg, strlen(msg), 0);
            }
        }
        // (Można też obsłużyć komunikaty typu WIN/LOSE, ale zwykle serwer kończy po grze)
    }

    close(sock);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "board.h"   // Deklaruje: board[5][5], setBoard(), setMove(), winCheck(), loseCheck()

#define INF 1000000

extern int  board[5][5];      // 0 = puste, 1/2 = pionki graczy
extern int  playerId, oppId;  
extern int  searchDepth;
extern void setMove(int r, int c, int pid);

// -----------------------------------------------------------------------------
// lineCount: policz w segmencie 4 pól ile jest pid i ile przeciwnika
// -----------------------------------------------------------------------------
static void lineCount(int pid, int *outMy, int *outOpp,
                      int sr, int sc, int dr, int dc)
{
    int cm = 0, co = 0;
    for (int i = 0; i < 4; i++) {
        int r = sr + dr*i, c = sc + dc*i;
        if      (board[r][c] == pid)    cm++;
        else if (board[r][c] != 0)      co++;
    }
    *outMy  = cm;
    *outOpp = co;
}

// -----------------------------------------------------------------------------
// hasFour: czy istnieje gdziekolwiek na planszy 4 w linii dla pid?
// -----------------------------------------------------------------------------
static bool hasFour(int pid)
{
    int dr[4] = {0,1,1,1}, dc[4] = {1,0,1,-1};
    for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 5; c++) {
            for (int d = 0; d < 4; d++) {
                int endR = r + dr[d]*3, endC = c + dc[d]*3;
                if (endR<0||endR>4||endC<0||endC>4) continue;
                int cm, co;
                lineCount(pid, &cm, &co, r, c, dr[d], dc[d]);
                if (cm == 4 && co == 0) return true;
            }
        }
    }
    return false;
}

// -----------------------------------------------------------------------------
// hasOnlyThree: czy jest gdziekolwiek na planszy dokładnie 3 w linii dla pid?
// -----------------------------------------------------------------------------
static bool hasOnlyThree(int pid)
{
    int dr[4] = {0,1,1,1}, dc[4] = {1,0,1,-1};
    for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 5; c++) {
            for (int d = 0; d < 4; d++) {
                int endR = r + dr[d]*3, endC = c + dc[d]*3;
                if (endR<0||endR>4||endC<0||endC>4) continue;
                int cm, co;
                lineCount(pid, &cm, &co, r, c, dr[d], dc[d]);
                if (cm == 3 && co == 0) return true;
            }
        }
    }
    return false;
}

// -----------------------------------------------------------------------------
// evaluateBoard: heurystyka z otwartymi trójkami i widełkami
// -----------------------------------------------------------------------------
int evaluateBoard() {
    int score = 0;
    int dr[4] = {0,1,1,1}, dc[4] = {1,0,1,-1};
    int openThrees = 0;

    for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 5; c++) {
            for (int d = 0; d < 4; d++) {
                int endR = r + dr[d]*3, endC = c + dc[d]*3;
                if (endR<0||endR>4||endC<0||endC>4) continue;

                int countMy = 0, countOpp = 0;
                for (int i = 0; i < 4; i++) {
                    int nr = r + dr[d]*i, nc = c + dc[d]*i;
                    if      (board[nr][nc] == playerId) countMy++;
                    else if (board[nr][nc] == oppId)    countOpp++;
                }

                // moje linie
                if (countOpp == 0) {
                    if      (countMy == 4) score += 100000;
                    else if (countMy == 3) {
                        // czy konczymy w trójkę?
                        bool consec = false;
                        for (int i = 0; i < 2; i++) {
                            int nr1 = r + dr[d]*i,    nc1 = c + dc[d]*i;
                            int nr2 = r + dr[d]*(i+1),nc2 = c + dc[d]*(i+1);
                            int nr3 = r + dr[d]*(i+2),nc3 = c + dc[d]*(i+2);
                            if (board[nr1][nc1]==playerId &&
                                board[nr2][nc2]==playerId &&
                                board[nr3][nc3]==playerId) {
                                consec = true; break;
                            }
                        }
                        if (consec) {
                            score -= 10000;
                        } else {
                            score += 5000;
                            openThrees++;
                        }
                    }
                    else if (countMy == 2) score += 20;
                    else if (countMy == 1) score += 10;
                }

                // linie przeciwnika
                if (countMy == 0) {
                    if      (countOpp == 4) score -= 100000;
                    else if (countOpp == 3) score += 10000;
                    else if (countOpp == 2) score -= 20;
                    else if (countOpp == 1) score -= 10;
                }
            }
        }
    }

    // bonus za widełki
    if (openThrees >= 2) {
        score += 20000;
    }

    return score;
}

// -----------------------------------------------------------------------------
// minimax z alfa-beta oraz terminalami 4 i 3
// -----------------------------------------------------------------------------
int minimax(int depth, int alpha, int beta, bool isMaximizing) {
    // terminalne zwycięstwa/przegrane
    if (hasFour(playerId))     return +INF;
    if (hasOnlyThree(playerId))return -INF;
    if (hasFour(oppId))        return -INF;
    if (hasOnlyThree(oppId))   return +INF;

    if (depth == 0) {
        return evaluateBoard();
    }

    if (isMaximizing) {
        int bestVal = -INF;
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 5; j++) {
                if (board[i][j] != 0) continue;
                setMove(i, j, playerId);
                int val = minimax(depth-1, alpha, beta, false);
                setMove(i, j, 0);
                if (val > bestVal) bestVal = val;
                if (bestVal > alpha) alpha = bestVal;
                if (beta <= alpha) break;
            }
        }
        return bestVal;
    } else {
        int bestVal = INF;
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 5; j++) {
                if (board[i][j] != 0) continue;
                setMove(i, j, oppId);
                int val = minimax(depth-1, alpha, beta, true);
                setMove(i, j, 0);
                if (val < bestVal) bestVal = val;
                if (bestVal < beta) beta = bestVal;
                if (beta <= alpha) break;
            }
        }
        return bestVal;
    }
}

// -----------------------------------------------------------------------------
// findBestMove zgodnie z regułami gry
// -----------------------------------------------------------------------------
void findBestMove(int *outR, int *outC) {
    int bestVal = -INF, count = 0;
    int cand[25][2];

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (board[i][j] != 0) continue;

            setMove(i, j, playerId);

            // 1) natychmiast wygrana?
            if (hasFour(playerId)) {
                *outR = i; *outC = j;
                setMove(i, j, 0);
                return;
            }
            // 2) tworzy przegrywającą trójkę?
            if (hasOnlyThree(playerId)) {
                setMove(i, j, 0);
                continue;
            }

            // 3) minimax
            int val = minimax(searchDepth-1, -INF, INF, false);
            setMove(i, j, 0);

            if (val > bestVal) {
                bestVal = val;
                count = 0;
                cand[count][0] = i;
                cand[count][1] = j;
                count++;
            } else if (val == bestVal) {
                cand[count][0] = i;
                cand[count][1] = j;
                count++;
            }
        }
    }

    if (count > 0) {
        int idx = rand() % count;
        *outR = cand[idx][0];
        *outC = cand[idx][1];
    } else {
        *outR = -1;
        *outC = -1;
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

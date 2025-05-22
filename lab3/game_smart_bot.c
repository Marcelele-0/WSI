#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "board.h"  

#define INF 1000000

extern int  board[5][5];   // 0 = puste, 1/2 = gracze
extern int  playerId, oppId;
extern int  searchDepth;

/**
 * Ustawia ruch zakodowany jako dwucyfrowa liczba:
 * dziesiątki = wiersz+1 (1–5), jedności = kolumna+1 (1–5).
 * Zwraca false, jeśli poza planszą lub pole zajęte.
 */
bool setMove(int move, int player) {
    int i = (move / 10) - 1;
    int j = (move % 10) - 1;
    if (i < 0 || i > 4 || j < 0 || j > 4) return false;
    if (board[i][j] != 0)                return false;
    board[i][j] = player;
    return true;
}

/**
 * Zdejmuje pionek z planszy, używane do cofania ruchu.
 */
static void undoMove(int r, int c) {
    board[r][c] = 0;
}

// -----------------------------------------------------------------------------
// Zlicza w 4-pólkowym segmencie, ile Twoich i ile przeciwnika.
// -----------------------------------------------------------------------------
static void lineCount(int pid, int *outMy, int *outOpp,
                      int sr, int sc, int dr, int dc)
{
    int cm = 0, co = 0;
    for (int k = 0; k < 4; k++) {
        int r = sr + dr*k, c = sc + dc*k;
        if      (board[r][c] == pid) cm++;
        else if (board[r][c] != 0)   co++;
    }
    *outMy  = cm;
    *outOpp = co;
}

// -----------------------------------------------------------------------------
// Czy gdziekolwiek jest 4 w linii (czysta czwórka) dla pid?
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
// Czy gdziekolwiek jest dokładnie 3 w linii (czysta trójka) dla pid?
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
// Heurystyka: premie/kar za 1–4 w linii, otwarte trójki, widełki…
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
                for (int k = 0; k < 4; k++) {
                    int nr = r + dr[d]*k, nc = c + dc[d]*k;
                    if      (board[nr][nc] == playerId) countMy++;
                    else if (board[nr][nc] == oppId)    countOpp++;
                }

                // Analiza moich segmentów
                if (countOpp == 0) {
                    if      (countMy == 4) score += 100000;
                    else if (countMy == 3) {
                        // Czy są 3 obok siebie?
                        bool consec = false;
                        for (int k = 0; k < 2; k++) {
                            int r1=r+dr[d]*k,    c1=c+dc[d]*k;
                            int r2=r+dr[d]*(k+1),c2=c+dc[d]*(k+1);
                            int r3=r+dr[d]*(k+2),c3=c+dc[d]*(k+2);
                            if (board[r1][c1]==playerId &&
                                board[r2][c2]==playerId &&
                                board[r3][c3]==playerId) {
                                consec = true;
                                break;
                            }
                        }
                        if (consec) {
                            score -= 10000;  // przegrana trójka
                        } else {
                            score += 5000;   // otwarta trójka
                            openThrees++;
                        }
                    }
                    else if (countMy == 2) score += 20;
                    else if (countMy == 1) score += 10;
                }

                // Analiza segmentów przeciwnika
                if (countMy == 0) {
                    if      (countOpp == 4) score -= 100000;
                    else if (countOpp == 3) score += 10000;
                    else if (countOpp == 2) score -= 20;
                    else if (countOpp == 1) score -= 10;
                }
            }
        }
    }

    // Widełki: dwa lub więcej otwartych trójek
    if (openThrees >= 2) {
        score += 20000;
    }

    return score;
}

// -----------------------------------------------------------------------------
// Minimax z alfa-beta i terminalami na 4/3
// -----------------------------------------------------------------------------
int minimax(int depth, int alpha, int beta, bool isMaximizing) {
    if (hasFour(playerId))      return +INF;
    if (hasOnlyThree(playerId)) return -INF;
    if (hasFour(oppId))         return -INF;
    if (hasOnlyThree(oppId))    return +INF;

    if (depth == 0) {
        return evaluateBoard();
    }

    if (isMaximizing) {
        int best = -INF;
        for (int r = 0; r < 5; r++) {
            for (int c = 0; c < 5; c++) if (board[r][c] == 0) {
                int move = (r+1)*10 + (c+1);
                setMove(move, playerId);
                int val = minimax(depth-1, alpha, beta, false);
                undoMove(r, c);
                if (val > best) best = val;
                if (best > alpha) alpha = best;
                if (beta <= alpha) return best;
            }
        }
        return best;
    } else {
        int best = INF;
        for (int r = 0; r < 5; r++) {
            for (int c = 0; c < 5; c++) if (board[r][c] == 0) {
                int move = (r+1)*10 + (c+1);
                setMove(move, oppId);
                int val = minimax(depth-1, alpha, beta, true);
                undoMove(r, c);
                if (val < best) best = val;
                if (best < beta) beta = best;
                if (beta <= alpha) return best;
            }
        }
        return best;
    }
}

// -----------------------------------------------------------------------------
// findBestMove zgodnie z regułami gry i nową heurystyką
// -----------------------------------------------------------------------------
void findBestMove(int *outR, int *outC) {
    int bestVal = -INF, count = 0;
    int cand[25][2];

    for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 5; c++) if (board[r][c] == 0) {
            int move = (r+1)*10 + (c+1);

            setMove(move, playerId);

            // 1) wygrana czwórką?
            if (hasFour(playerId)) {
                *outR = r; *outC = c;
                undoMove(r, c);
                return;
            }
            // 2) przegrywająca trójka?
            if (hasOnlyThree(playerId)) {
                undoMove(r, c);
                continue;
            }

            // 3) oceniaj minimax
            int val = minimax(searchDepth-1, -INF, INF, false);
            undoMove(r, c);

            if (val > bestVal) {
                bestVal = val;
                count = 0;
                cand[count][0] = r;
                cand[count][1] = c;
                count++;
            } else if (val == bestVal) {
                cand[count][0] = r;
                cand[count][1] = c;
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
    int server_socket;
    struct sockaddr_in server_addr;
    char server_message[16], player_message[16];

    bool end_game;
    int player, msg, move;

    if (argc != 6) {
        printf("Usage: %s <IP> <PORT> <PLAYER_ID> <NAME> <DEPTH>\n", argv[0]);
        return -1;
    }

    // Create socket
  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if ( server_socket < 0 ) {
    printf("Unable to create socket\n");
    return -1;
  }
  printf("Socket created successfully\n");

  // Set port and IP the same as server-side
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(atoi(argv[2]));
  server_addr.sin_addr.s_addr = inet_addr(argv[1]);

  // Send connection request to server
  if ( connect(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0 ) {
    printf("Unable to connect\n");
    return -1;
  }
  printf("Connected with server successfully\n");

  // Receive the server message
  memset(server_message, '\0', sizeof(server_message));
  if ( recv(server_socket, server_message, sizeof(server_message), 0) < 0 ) {
    printf("Error while receiving server's message\n");
    return -1;
  }

  memset(player_message, '\0', sizeof(player_message));
  snprintf(player_message, sizeof(player_message), "%s %s", argv[3], argv[4]);
  // Send the message to server
  if ( send(server_socket, player_message, strlen(player_message), 0) < 0 ) {
    printf("Unable to send message\n");
    return -1;
  }

  setBoard();
  end_game = false;
  sscanf(argv[3], "%d", &player);

  while ( !end_game ) {
    memset(server_message, '\0', sizeof(server_message));
    if ( recv(server_socket, server_message, sizeof(server_message), 0) < 0 ) {
      printf("Error while receiving server's message\n");
      return -1;
    }
    sscanf(server_message, "%d", &msg);
    move = msg%100;
    msg = msg/100;
    if ( move != 0 ) {
      setMove(move, 3-player);
    }
    if ( (msg == 0) || (msg == 6) ) {
      move = bestMove();
      setMove(move, player);
      memset(player_message, '\0', sizeof(player_message));
      snprintf(player_message, sizeof(player_message), "%d", move);
      if ( send(server_socket, player_message, strlen(player_message), 0) < 0 ) {
        printf("Unable to send message\n");
        return -1;
      }
     } else {
       end_game = true;
       switch ( msg ) {
         case 1 : printf("You won.\n"); break;
         case 2 : printf("You lost.\n"); break;
         case 3 : printf("Draw.\n"); break;
         case 4 : printf("You won. Opponent error.\n"); break;
         case 5 : printf("You lost. Your error.\n"); break;
      }
    }
  }

  // Close socket
  close(server_socket);

  return 0;
}
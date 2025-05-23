#include "board.h"

// Sprawdza czy obecny stan planszy zawiera "otwartą trójkę" gracza who
bool creates_open_three(int who) {
    for (int l = 0; l < 48; l++) {
        int count = 0;
        for (int m = 0; m < 3; m++) {
            int x = lose[l][m][0];
            int y = lose[l][m][1];
            if (board[x][y] == who) count++;
        }
        if (count == 3) {
            // Wyznacz kierunek linii
            int x1 = lose[l][0][0], y1 = lose[l][0][1];
            int x2 = lose[l][1][0], y2 = lose[l][1][1];
            int x3 = lose[l][2][0], y3 = lose[l][2][1];
            int dx = x2 - x1, dy = y2 - y1;
            // Sprawdź pole przed pierwszym i za ostatnim
            int before_x = x1 - dx, before_y = y1 - dy;
            int after_x = x3 + dx, after_y = y3 + dy;
            if (before_x >= 0 && before_x < 5 && before_y >= 0 && before_y < 5 && board[before_x][before_y] == 0 &&
                after_x >= 0 && after_x < 5 && after_y >= 0 && after_y < 5 && board[after_x][after_y] == 0) {
                return true;
            }
        }
    }
    return false;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include "board.h"  // Include the board.h header file for board-related functions

int player, opponent, searchDepth;

// Nowa heurystyka: faworyzuje linie z wieloma swoimi pionkami i blokuje linie przeciwnika
#include "board.h"
int evaluateBoard(int who) {
    int score = 0;
    for (int w = 0; w < 28; w++) { // 28 możliwych czwórek
        int my_count = 0, opp_count = 0;
        for (int k = 0; k < 4; k++) {
            int i = win[w][k][0];
            int j = win[w][k][1];
            if (board[i][j] == who) my_count++;
            else if (board[i][j] == 3 - who) opp_count++;
        }
        if (my_count > 0 && opp_count == 0) {
            if (my_count == 1) score += 1;
            else if (my_count == 2) score += 2;
            else if (my_count == 3) score += 4;
            else if (my_count == 4) score += 1000; // wygrana
        }
        if (opp_count > 0 && my_count == 0) {
            if (opp_count == 1) score -= 1;
            else if (opp_count == 2) score -= 2;
            else if (opp_count == 3) score -= 4;
            else if (opp_count == 4) score -= 1000; // przegrana
        }
    }
    return score;
}

int minimax(int depth, int alpha, int beta, int currentPlayer, bool maximizing) {
    if (depth == 0) {
        int eval = evaluateBoard(player);
        return eval;
    }
    int best;
    if (maximizing) {
        best = -100000;
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 5; j++) {
                if (board[i][j] == 0) {
                    board[i][j] = currentPlayer;
                    int val = minimax(depth - 1, alpha, beta, 3 - currentPlayer, false);
                    board[i][j] = 0;
                    if (val > best) best = val;
                    if (best > alpha) alpha = best;
                    if (beta <= alpha) {
                        return best;
                    }
                }
            }
        }
        return best;
    } else {
        best = 100000;
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 5; j++) {
                if (board[i][j] == 0) {
                    board[i][j] = currentPlayer;
                    int val = minimax(depth - 1, alpha, beta, 3 - currentPlayer, true);
                    board[i][j] = 0;
                    if (val < best) best = val;
                    if (best < beta) beta = best;
                    if (beta <= alpha) {
                        return best;
                    }
                }
            }
        }
        return best;
    }
}

// Funkcja bestMove: wybiera najlepszy ruch na podstawie heurystyki i minimax
int bestMove() {
    int bestScore = -100000;
    int move = 0;
    int safeMove = 0;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (board[i][j] == 0) {
                board[i][j] = player;
                // Jeśli ten ruch daje natychmiastową wygraną, wybierz go od razu
                if (winCheck(player)) {
                    board[i][j] = 0;
                    return (i + 1) * 10 + (j + 1);
                }
                // Jeśli ten ruch blokuje natychmiastową wygraną przeciwnika, zapamiętaj go
                board[i][j] = 3 - player;
                if (winCheck(3 - player)) {
                    board[i][j] = 0;
                    // wróć do swojego ruchu
                    board[i][j] = player;
                    move = (i + 1) * 10 + (j + 1);
                    board[i][j] = 0;
                    return move;
                }
                board[i][j] = player;
                // Odrzuć ruch, jeśli tworzy otwartą trójkę
                if (creates_open_three(player)) {
                    board[i][j] = 0;
                    if (safeMove == 0) safeMove = (i + 1) * 10 + (j + 1);
                    continue;
                }
                // Odrzuć ruch, jeśli natychmiast przegrywasz
                if (loseCheck(player)) {
                    board[i][j] = 0;
                    if (safeMove == 0) safeMove = (i + 1) * 10 + (j + 1);
                    continue;
                }
                int score = minimax(searchDepth - 1, -100000, 100000, 3 - player, false);
                board[i][j] = 0;
                if (score > bestScore) {
                    bestScore = score;
                    move = (i + 1) * 10 + (j + 1);
                }
            }
        }
    }
    // Jeśli nie znaleziono żadnego "bezpiecznego" ruchu, wybierz pierwszy niebezpieczny
    if (move == 0 && safeMove != 0) {
        return safeMove;
    }
    // Jeśli nie ma lepszego ruchu, wybierz wolne pole najbliżej środka planszy
    if (move == 0) {
        int bestDist = 100;
        int bestI = -1, bestJ = -1;
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 5; j++) {
                if (board[i][j] == 0) {
                    int dist = (i - 2) * (i - 2) + (j - 2) * (j - 2);
                    if (dist < bestDist) {
                        bestDist = dist;
                        bestI = i;
                        bestJ = j;
                    }
                }
            }
        }
        if (bestI != -1 && bestJ != -1) {
            return (bestI + 1) * 10 + (bestJ + 1);
        }
    }
    return move;
}

int main(int argc, char *argv[]) {
  int server_socket;
  struct sockaddr_in server_addr;
  char server_message[16], player_message[16];

  bool end_game;
  int msg, move;

  if (argc != 6) {
    printf("Usage: %s <IP> <PORT> <PLAYER_ID> <NAME> <DEPTH>\n", argv[0]);
    return -1;
  }
  
  searchDepth = atoi(argv[5]);

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
  if (send(server_socket, player_message, strlen(player_message), 0) < 0) {
    printf("Unable to send message\n");
    return -1;
  }

  setBoard();
  end_game = false;
  sscanf(argv[3], "%d", &player);

  while (!end_game) {
    memset(server_message, '\0', sizeof(server_message));
    if (recv(server_socket, server_message, sizeof(server_message), 0) < 0) {
      printf("Error while receiving server's message\n");
      return -1;
    }
    sscanf(server_message, "%d", &msg);
    move = msg % 100;
    msg = msg / 100;
    if (move != 0) {
      setMove(move, 3 - player);
    }
    if ((msg == 0) || (msg == 6)) {
      move = bestMove();
      printf("[SMART BOT] setMove called with move=%d, player=%d (my move)\n", move, player);
      setMove(move, player);
      memset(player_message, '\0', sizeof(player_message));
      snprintf(player_message, sizeof(player_message), "%d", move);
      if (send(server_socket, player_message, strlen(player_message), 0) < 0) {
        printf("Unable to send message\n");
        return -1;
      }
    } else {
      end_game = true;
      switch (msg) {
        case 1: printf("You won.\n"); break;
        case 2: printf("You lost.\n"); break;
        case 3: printf("Draw.\n"); break;
        case 4: printf("You won. Opponent error.\n"); break;
        case 5: printf("You lost. Your error.\n"); break;
      }
    }
  }

  // Close socket
  close(server_socket);

  return 0;
}
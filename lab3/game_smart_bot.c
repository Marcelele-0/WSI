<<<<<<< Updated upstream
=======
#include "board.h"
#include "heuristic.h"
#include "opening_book.h"
>>>>>>> Stashed changes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <stdbool.h>

<<<<<<< Updated upstream
#include "board.h"  // Assuming board.h contains the necessary board functions like setBoard, setMove, winCheck, loseCheck

#define INF 1000000

int player, opponent, searchDepth = 3;

int evaluateBoard() {
    int score = 0;
    int dr[4] = {0, 1, 1, -1};  // kierunki: →, ↓, ↘, ↖
    int dc[4] = {1, 0, 1, 1};

    for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 5; c++) {
            for (int d = 0; d < 4; d++) {
                int countMy = 0, countOpp = 0;
                bool valid = true;

                for (int i = 0; i < 4; i++) {
                    int nr = r + dr[d] * i;
                    int nc = c + dc[d] * i;
                    if (nr < 0 || nr >= 5 || nc < 0 || nc >= 5) {
                        valid = false;
                        break;
                    }
                    if (board[nr][nc] == player) countMy++;
                    else if (board[nr][nc] == opponent) countOpp++;
                }

                if (!valid) continue;

                // **4 w linii** -> Automatyczna wygrana
                if (countMy == 4) return INF;
                if (countOpp == 4) return -INF;

                // **Trójka w linii** bez czwartej -> Przegrana
                if (countMy == 3 && countOpp == 0) score -= 100000;  // kara za trójkę
                else if (countOpp == 3 && countMy == 0) score += 500; // przeciwnik popełnia błąd

                // **Trójka z przerwą** - Zagrożenie wygranej
                else if (countMy == 3 && countOpp == 1) score += 200;  // duża nagroda za zagrożenie
                else if (countOpp == 3 && countMy == 1) score -= 200; // duża kara za zagrożenie przeciwnika

                // **Dwie trójki w jednym ruchu (Win by Force)**
                else if (countMy == 3 && countOpp == 1 && canFormTwoThreats(r, c, dr[d], dc[d], player)) {
                    score += 1000;  // bardzo wysoka nagroda za "win by force"
                }

                // **2 w linii** (nie pełna linia) z pustymi miejscami, ale nie takie bliskie – nagradzamy ostrożnie
                else if (countMy == 2 && countOpp == 0) score += 20;
                else if (countOpp == 2 && countMy == 0) score -= 20;
            }
        }
    }

    return score;
}

// Funkcja do sprawdzania czy istnieje możliwość stworzenia dwóch zagrożeń
bool canFormTwoThreats(int r, int c, int dr, int dc, int player) {
    int count = 0;
    for (int i = -1; i <= 1; i++) {
        int nr = r + dr * i;
        int nc = c + dc * i;
        if (nr >= 0 && nr < 5 && nc >= 0 && nc < 5 && board[nr][nc] == player) {
            count++;
        }
    }
    return count == 2;
}


int minimax(int depth, int alpha, int beta, bool maximizing) {
    if (winCheck(player)) return INF;
    if (winCheck(opponent)) return -INF;
    if (loseCheck(player)) return -INF;
    if (loseCheck(opponent)) return INF;
    if (depth == 0) return evaluateBoard();

    if (maximizing) {
        int maxEval = -INF;
        for (int i = 0; i < 5; i++) for (int j = 0; j < 5; j++) {
=======

int player, opponent, searchDepth;
bool learningMode = false;  // Tryb uczenia książki otwarć
int gameMovesCount = 0;     // Licznik ruchów w grze

// Funkcja bestMove: wybiera najlepszy ruch na podstawie heurystyki i minimax
int bestMove() {
    // KROK 1: Sprawdź książkę otwarć (tylko w pierwszych 10 ruchach)
    char* currentSequence = buildMoveSequence();
    
    if (isInOpeningPhase(gameMovesCount)) {
        int openingMove = getOpeningMove(currentSequence, gameMovesCount);
        if (openingMove != 0) {
            printf("[OPENING BOOK] Using move %d from book for sequence: %s\n", openingMove, currentSequence);
            return openingMove;  // Użyj ruchu z książki
        }
    }
    
    // KROK 2: Standardowy minimax jeśli brak w książce
    int bestScore = -100000;
    int move = 0;
    int safeMove = 0;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
>>>>>>> Stashed changes
            if (board[i][j] == 0) {
                board[i][j] = player;
                int eval = minimax(depth - 1, alpha, beta, false);
                board[i][j] = 0;
                if (eval > maxEval) maxEval = eval;
                if (maxEval > alpha) alpha = maxEval;
                if (beta <= alpha) break;
            }
        }
        return maxEval;
    } else {
        int minEval = INF;
        for (int i = 0; i < 5; i++) for (int j = 0; j < 5; j++) {
            if (board[i][j] == 0) {
                board[i][j] = opponent;
                int eval = minimax(depth - 1, alpha, beta, true);
                board[i][j] = 0;
                if (eval < minEval) minEval = eval;
                if (minEval < beta) beta = minEval;
                if (beta <= alpha) break;
            }
        }
        return minEval;
    }
}

int bestMove() {
    int bestScore = -INF;
    int move = 0;
    for (int i = 0; i < 5; i++) for (int j = 0; j < 5; j++) {
        if (board[i][j] == 0) {
            board[i][j] = player;
            int score = minimax(searchDepth - 1, -INF, INF, false);
            board[i][j] = 0;
            if (score > bestScore) {
                bestScore = score;
                move = (i + 1) * 10 + (j + 1);
            }
        }
    }
    return move;
}

int main(int argc, char *argv[]) {
    int server_socket;
    struct sockaddr_in server_addr;
    char server_message[16], player_message[16];

    bool end_game;
    int player, msg, move;

<<<<<<< Updated upstream
    if (argc != 6) {
        printf("Usage: %s <IP> <PORT> <PLAYER_ID> <NAME> <DEPTH>\n", argv[0]);
        return -1;
    }
=======
  // OBSŁUGA TRYBU UCZENIA
  if (argc >= 2 && strstr(argv[1], "--learn") != NULL) {
    printf("=== OPENING BOOK LEARNING MODE ===\n");
    
    // Parsuj argumenty uczenia: --learn-depth=X-search=Y
    int learnDepth = 6;   // domyślnie
    int searchDepth = 6;  // domyślnie
    
    for (int i = 1; i < argc; i++) {
      if (strstr(argv[i], "--learn-depth=") != NULL) {
        sscanf(argv[i], "--learn-depth=%d", &learnDepth);
      }
      if (strstr(argv[i], "--search-depth=") != NULL) {
        sscanf(argv[i], "--search-depth=%d", &searchDepth);
      }
    }
    
    printf("Learning parameters: depth=%d, search=%d\n", learnDepth, searchDepth);
    
    // Inicjuj planszę
    setBoard();
    clearMoveHistory();
    
    // Rozpocznij uczenie
    learnOpenings(learnDepth, searchDepth, "opening_book.txt");
    
    // Zwolnij pamięć i zakończ
    freeOpeningBook();
    return 0;
  }

  // NORMALNY TRYB GRY
  if (argc != 6) {
    printf("Usage: %s <IP> <PORT> <PLAYER_ID> <n> <DEPTH>\n", argv[0]);
    printf("   or: %s --learn-depth=X --search-depth=Y\n", argv[0]);
    return -1;
  }
  
  searchDepth = atoi(argv[5]);
>>>>>>> Stashed changes

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
  
  // Inicjalizacja książki otwarć
  clearMoveHistory();
  gameMovesCount = 0;
  loadOpeningBook("opening_book.txt");  // Załaduj książkę jeśli istnieje

  while ( !end_game ) {
    memset(server_message, '\0', sizeof(server_message));
    if ( recv(server_socket, server_message, sizeof(server_message), 0) < 0 ) {
      printf("Error while receiving server's message\n");
      return -1;
    }
    sscanf(server_message, "%d", &msg);
<<<<<<< Updated upstream
    move = msg%100;
    msg = msg/100;
    if ( move != 0 ) {
      setMove(move, 3-player);
=======
    move = msg % 100;
    msg = msg / 100;
    if (move != 0) {
      setMove(move, 3 - player);
      addMoveToHistory(move);  // Dodaj ruch przeciwnika do historii
      gameMovesCount++;
>>>>>>> Stashed changes
    }
    if ( (msg == 0) || (msg == 6) ) {
      move = bestMove();
      setMove(move, player);
      addMoveToHistory(move);  // Dodaj swój ruch do historii  
      gameMovesCount++;
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
  
  // Zwolnij pamięć książki otwarć
  freeOpeningBook();

  return 0;
}
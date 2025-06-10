#include "board.h"
#include "heuristic.h"
#include "opening_book.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <stdbool.h>


int player, opponent, searchDepth;
bool learningMode = false;  // Tryb uczenia książki otwarć
int gameMovesCount = 0;     // Licznik ruchów w grze

// Funkcja bestMove: wybiera najlepszy ruch na podstawie heurystyki i minimax
int bestMove() {
    // KROK 1: Sprawdź książkę otwarć (tylko w pierwszych 10 ruchach)
    char* currentSequence = buildMoveSequence();
    
    printf("[DEBUG] bestMove called, gameMovesCount=%d, sequence='%s'\n", gameMovesCount, currentSequence);
    
    if (isInOpeningPhase(gameMovesCount)) {
        int openingMove = getOpeningMove(currentSequence, gameMovesCount);
        printf("[DEBUG] Opening phase - checked book for '%s', got move: %d\n", currentSequence, openingMove);
        if (openingMove != 0) {
            printf("[OPENING BOOK] Using move %d from book for sequence: %s\n", openingMove, currentSequence);
            return openingMove;  // Użyj ruchu z książki
        }
    } else {
        printf("[DEBUG] Not in opening phase (move count: %d > %d)\n", gameMovesCount, MAX_OPENING_MOVES);
    }
    
    // KROK 2: Standardowy minimax jeśli brak w książce
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
                // Odrzuć ruch, jeśli natychmiast przegrywasz (3 w rzędzie)
                if (loseCheck(player)) {
                    board[i][j] = 0;
                    continue; // Całkowicie odrzuć ten ruch - nie jest bezpieczny
                }
                // Ten ruch jest bezpieczny (nie powoduje natychmiastowej przegranej)
                if (safeMove == 0) safeMove = (i + 1) * 10 + (j + 1);
                int score = minimax(searchDepth - 1, -100000, 100000, 3 - player, false, player);
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

  // OBSŁUGA TRYBU UCZENIA
  if (argc >= 2 && (strstr(argv[1], "--learn") != NULL || 
                    (argc > 1 && strstr(argv[1], "--learn-depth") != NULL))) {
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
    learnOpenings(learnDepth, searchDepth, "opening_book_2.txt");
    
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
  
  // Inicjalizacja książki otwarć
  clearMoveHistory();
  gameMovesCount = 0;
  loadOpeningBook("opening_book_2.txt");  // Załaduj książkę jeśli istnieje

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
      addMoveToHistory(move);  // Dodaj ruch przeciwnika do historii
      gameMovesCount++;
    }
    if ((msg == 0) || (msg == 6)) {
      move = bestMove();
      printf("[SMART BOT] setMove called with move=%d, player=%d (my move)\n", move, player);
      setMove(move, player);
      addMoveToHistory(move);  // Dodaj swój ruch do historii  
      gameMovesCount++;
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
  
  // Zwolnij pamięć książki otwarć
  freeOpeningBook();

  return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include "board.h"  // Include the board.h header file for board-related functions

int player, opponent, searchDepth = 3;

int evaluateBoard() {
  // Basic stub: always return 0
  return 0;
}

int minimax(int depth, int alpha, int beta, bool maximizing) {
  // Basic stub: always return 0
  return 0;
}

int bestMove() {
  // Basic stub: always return the first available move
  for (int i = 0; i < 5; i++) {
      for (int j = 0; j < 5; j++) {
          if (board[i][j] == 0) {
              return (i + 1) * 10 + (j + 1);
          }
      }
  }
  return 0;
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
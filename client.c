#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define BUF_SIZE 1024

void communicate_with_server(int socket_desc, int client_type) {
    char message[BUF_SIZE];
    char server_reply[BUF_SIZE];

    while (1) {
        if (client_type == 1) {
            // Section 1: generate new pin
            snprintf(message, BUF_SIZE, "Pin from section 1");
        } else {
            // Receive pin from previous section
            if (recv(socket_desc, server_reply, BUF_SIZE, 0) < 0) {
                perror("Recv failed");
                return;
            }
            printf("Received from server: %s\n", server_reply);

            if (client_type == 2) {
                snprintf(message, BUF_SIZE, "Pin from section 2");
            } else {
                snprintf(message, BUF_SIZE, "Pin from section 3");
            }
        }

        if (send(socket_desc, message, strlen(message), 0) < 0) {
            perror("Send failed");
            return;
        }

        sleep(rand() % 3 + 1); // Random delay to simulate work time
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <port> <client_type>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int PORT = atoi(argv[1]);
    int client_type = atoi(argv[2]);
    int socket_desc;
    struct sockaddr_in server_addr;

    srand(time(NULL));

    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    if (connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connect failed");
        close(socket_desc);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server\n");
    communicate_with_server(socket_desc, client_type);

    close(socket_desc);
    return 0;
}

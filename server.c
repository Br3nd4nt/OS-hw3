#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

// #define PORT 8080
#define BUF_SIZE 1024
#define MAX_CLIENTS 10

typedef struct {
    int socket;
    int type; // 1 - участок 1, 2 - участок 2, 3 - участок 3
} client_info;

client_info clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;

void *handle_client(void *arg) {
    int client_socket = ((client_info*)arg)->socket;
    int client_type = ((client_info*)arg)->type;
    char buffer[BUF_SIZE];
    int read_size;

    while ((read_size = recv(client_socket, buffer, BUF_SIZE, 0)) > 0) {
        buffer[read_size] = '\0';
        printf("Received from client %d: %s\n", client_type, buffer);
        pthread_mutex_lock(&client_mutex);
        // Process and forward to next client
        if (client_type == 1) {
            // Send to a random worker on section 2
            int next_client_socket = clients[rand() % client_count].socket;
            send(next_client_socket, buffer, strlen(buffer), 0);
        } else if (client_type == 2) {
            // Send to a random worker on section 3
            int next_client_socket = clients[rand() % client_count].socket;
            send(next_client_socket, buffer, strlen(buffer), 0);
        } else {
            // Final processing on section 3
            printf("Quality control done for: %s\n", buffer);
        }
        pthread_mutex_unlock(&client_mutex);
    }

    close(client_socket);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int PORT = atoi(argv[1]);
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_size = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 3) < 0) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while ((client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_size)) >= 0) {
        printf("Client connected\n");

        // Determine client type (for simplicity, rotate between types)
        int client_type = (client_count % 3) + 1;

        pthread_mutex_lock(&client_mutex);
        clients[client_count].socket = client_socket;
        clients[client_count].type = client_type;
        client_count++;
        pthread_mutex_unlock(&client_mutex);

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_client, (void*)&clients[client_count - 1]);
    }

    close(server_socket);
    return 0;
}

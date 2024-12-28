#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include "request-handle.h"
#include "threadpool.h"

#define PORT 8080
#define BACKLOG 10
#define BUF_SIZE 1024

int server_fd = -1; // global server socket

void cleanup_and_exit(int signum) {
    if (server_fd != -1) {
        printf("\nRecebido sinal %d, fechando o servidor...\n", signum);
        close(server_fd);
    }
    exit(0);
}

void setup_signal_handler() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = cleanup_and_exit;
    sa.sa_flags = 0;

    // Mascara de sinais vazia
    sigemptyset(&sa.sa_mask);

    // Configurar para SIGINT (Ctrl+C) e SIGTERM (kill)
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Erro ao configurar SIGINT");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("Erro ao configurar SIGTERM");
        exit(EXIT_FAILURE);
    }
}

int main() {
    struct sockaddr_in server_addr;

    setup_signal_handler();

    // 1. Criação do socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    // 2. Configurar endereço do servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // 3. Associar o socket ao endereço
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Erro ao associar socket");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 4. Colocar o socket em modo de escuta
    if (listen(server_fd, BACKLOG) == -1) {
        perror("Erro ao escutar no socket");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("simple server listening on port %d...\n", PORT);

    // 5. Loop principal para aceitar conexões
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);

        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd == -1) {
            perror("Erro ao aceitar conexão");
            continue;
        }

        printf("Conexão aceita de %s:%d\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));


        char buffer[BUF_SIZE];
        int bytes_read = read(client_fd, buffer, BUF_SIZE - 1);

        if (bytes_read < 0) {
            perror("Erro ao ler do cliente");
            close(client_fd);
            continue;
        }

        buffer[bytes_read] = '\0';

        TaskArgs *args = malloc(sizeof(TaskArgs));
        args->client_fd = client_fd;
        args->buffer = malloc(BUF_SIZE * sizeof(char));
        strncpy(args->buffer, buffer, BUF_SIZE - 1);
        args->buffer[BUF_SIZE - 1] = '\0';

        handle_request(args);
    }

    close(server_fd);
    return 0;
}

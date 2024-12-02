#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "request-handle.h"

#define PORT 8080
#define MAX_EVENTS 10
#define BUFFER_SIZE 1024

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

void set_nonblocking(int sock) {
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
}

int main() {
    int epoll_fd;
    struct sockaddr_in server_addr;
    struct epoll_event event, events[MAX_EVENTS];

    setup_signal_handler();

    // Criar socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    // Configurar endereço e bind
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erro ao associar socket");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Colocar o socket em escuta
    if (listen(server_fd, MAX_EVENTS) < 0) {
        perror("Erro ao escutar socket");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Criar epoll
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("Erro ao criar epoll");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Adicionar o socket do servidor ao epoll
    event.events = EPOLLIN; // Monitorar para leitura
    event.data.fd = server_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) < 0) {
        perror("Erro ao adicionar ao epoll");
        close(server_fd);
        close(epoll_fd);
        exit(EXIT_FAILURE);
    }

    printf("Servidor ouvindo na porta %d\n", PORT);

    while (1) {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for (int i = 0; i < n; i++) {
            if (events[i].data.fd == server_fd) {
                // Aceitar nova conexão
                int client_fd = accept(server_fd, NULL, NULL);
                set_nonblocking(client_fd);
                event.events = EPOLLIN | EPOLLET;
                event.data.fd = client_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event);
                printf("Nova conexão aceita\n");
            } else {
                // process request
                handle_request(events[i].data.fd);
            }
        }
    }

    close(server_fd);
    return 0;
}

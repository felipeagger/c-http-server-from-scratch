#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include "cpu-bound.h"
#include "request-handle.h"

#define BUF_SIZE 1024


char* getParamQueryString(char buffer[BUF_SIZE], const char *param_name) {
    char *query_string = strstr(buffer, "?");

    if (query_string != NULL) {
        query_string++; // Move o ponteiro para depois do '?'
        char *param_value = NULL;

        size_t param_name_length = strlen(param_name);
        char *param_start = strstr(query_string, param_name);
        
        if (param_start != NULL && *(param_start + param_name_length) == '=') {
            param_start += param_name_length + 1; // Move o ponteiro para depois de 'param_name='
            char *end_of_param = strchr(param_start, '&'); // Encontra o próximo '&' ou fim da string
            char *end_of_param_fallback = strchr(param_start, ' '); // Encontra o próximo ' ' ou fim da string

            if (end_of_param != NULL) {
                size_t param_length = end_of_param - param_start; // Calcula o tamanho do valor do parâmetro
                param_value = malloc(param_length + 1); // Aloca memória para o valor do parâmetro
                strncpy(param_value, param_start, param_length);
                param_value[param_length] = '\0'; // Adiciona o terminador nulo
            } else if (end_of_param_fallback != NULL) {
                size_t param_length = end_of_param_fallback - param_start; // Calcula o tamanho do valor do parâmetro
                param_value = malloc(param_length + 1); // Aloca memória para o valor do parâmetro
                strncpy(param_value, param_start, param_length);
                param_value[param_length] = '\0';
            } else {
                // Se não houver '&', pega até o final da string
                param_value = strdup(param_start); // Duplica a string até o final
            }
        }

        return param_value; // Retorna o valor do parâmetro ou NULL se não encontrado
    }
    
    return NULL;
}

void handle_request(int client_fd) {
    char buffer[BUF_SIZE];
    int bytes_read = read(client_fd, buffer, BUF_SIZE - 1);

    if (bytes_read < 0) {
        perror("Erro ao ler do cliente");
        close(client_fd);
        return;
    }

    buffer[bytes_read] = '\0'; // Garante que o buffer seja uma string
    printf("Requisição recebida:\n%s\n", buffer);

    //params
    char *timeout = getParamQueryString(buffer, "timeout");
    if (timeout == NULL) {
        timeout = "10"; // ms
    }

    const char *response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: 32\r\n"
            "\r\n"
            "{\"message\": \"request completed\"}";

    // Routing
    if (strstr(buffer, "GET /api/cpu-bound") == buffer) {
        
        simulateCPU(atoi(timeout));

        write(client_fd, response, strlen(response));

    } else if (strstr(buffer, "GET /api/io-bound") == buffer) {
        
        usleep(atoi(timeout) * 1000);

        write(client_fd, response, strlen(response));

    } else if (strstr(buffer, "GET /health") == buffer) {
        
        // response 200
        char *resp = "HTTP/1.1 200 OK\r\nContent-Length: 8\r\n\r\nHealthy!";
        write(client_fd, resp, strlen(resp));

    } else {
        
        // response 404
        char *resp = "HTTP/1.1 404 Not Found\r\nContent-Length: 16\r\n\r\nRoute not found!";
        write(client_fd, resp, strlen(resp));
    }

    //free(timeout);
    //free(resp_size);
    close(client_fd);
}
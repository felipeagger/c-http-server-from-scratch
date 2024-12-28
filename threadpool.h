#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>

typedef struct {
    int client_fd;
    char *buffer;
} TaskArgs;

typedef void (*FunctionPointer)(TaskArgs*);

typedef struct {
    FunctionPointer function;  // Ponteiro para a função
    void* argument;           // Argumento da função
} Task;

// Estrutura para o pool de threads
typedef struct {
    Task* tasks;              // Fila de tarefas
    int task_count;           // Número de tarefas na fila
    int head, tail;           // Ponteiros para início e fim da fila
    int queue_size;           // Tamanho da fila

    pthread_mutex_t lock;     // Mutex para sincronização
    pthread_cond_t notify;    // Condição para notificação
    int shutdown;             // Flag para finalizar o pool
} ThreadPool;


//int thread_pool_add(ThreadPool* pool, void (*function)(void*), void* argument);
int thread_pool_add(ThreadPool* pool, FunctionPointer TaskFunc, void* argument);
void thread_pool_init(ThreadPool* pool, int thread_count, int queue_size);
void thread_pool_destroy(ThreadPool* pool);

#endif
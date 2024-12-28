#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


typedef struct {
    int client_fd;
    char *buffer;
} TaskArgs;

typedef void (*FunctionPointer)(TaskArgs*);

typedef struct {
    FunctionPointer function;  // Ponteiro para a função
    void* argument;           // Argumento da função
} Task;

typedef struct {
    Task* tasks;              // Fila de tarefas
    int task_count;           // Número de tarefas na fila
    int head, tail;           // Ponteiros para início e fim da fila
    int queue_size;           // Tamanho da fila

    pthread_mutex_t lock;     // Mutex para sincronização
    pthread_cond_t notify;    // Condição para notificação
    int shutdown;             // Flag para finalizar o pool
} ThreadPool;


int thread_pool_add(ThreadPool* pool, FunctionPointer TaskFunc, void* argument) {
    pthread_mutex_lock(&pool->lock);

    if (pool->task_count == pool->queue_size) {
        pthread_mutex_unlock(&pool->lock);
        return -1;  // Fila cheia
    }

    Task task = { TaskFunc, argument };
    pool->tasks[pool->tail] = task;
    pool->tail = (pool->tail + 1) % pool->queue_size;
    pool->task_count++;

    pthread_cond_signal(&pool->notify);
    pthread_mutex_unlock(&pool->lock);
    return 0;
}

void* thread_loop(void* arg) {
    ThreadPool* pool = (ThreadPool*)arg;

    while (1) {
        pthread_mutex_lock(&pool->lock);

        while (pool->task_count == 0 && !pool->shutdown) {
            pthread_cond_wait(&pool->notify, &pool->lock);
        }

        if (pool->shutdown) {
            pthread_mutex_unlock(&pool->lock);
            pthread_exit(NULL);
        }

        Task task = pool->tasks[pool->head];
        pool->head = (pool->head + 1) % pool->queue_size;
        pool->task_count--;

        pthread_mutex_unlock(&pool->lock);

        // Executa a tarefa
        task.function(task.argument);
    }
}

void thread_pool_init(ThreadPool* pool, int thread_count, int queue_size) {
    pool->tasks = (Task*)malloc(sizeof(Task) * queue_size);
    pool->task_count = 0;
    pool->queue_size = queue_size;
    pool->head = pool->tail = 0;
    pool->shutdown = 0;

    pthread_mutex_init(&pool->lock, NULL);
    pthread_cond_init(&pool->notify, NULL);

    for (int i = 0; i < thread_count; i++) {
        pthread_t thread;
        pthread_create(&thread, NULL, thread_loop, (void*)pool);
        pthread_detach(thread);
    }
}

void thread_pool_destroy(ThreadPool* pool) {
    pthread_mutex_lock(&pool->lock);
    pool->shutdown = 1;
    pthread_cond_broadcast(&pool->notify);
    pthread_mutex_unlock(&pool->lock);

    free(pool->tasks);
    pthread_mutex_destroy(&pool->lock);
    pthread_cond_destroy(&pool->notify);
}

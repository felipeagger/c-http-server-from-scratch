#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include "cpu-bound.h"

//int timeout = 5; seconds
//simulateCPU(timeout);

void simulateCPU(int timeout) {
    clock_t start = clock();
    int i = 2;

    while (1) {
        // Verifica se o tempo limite foi atingido
        if (((double)(clock() - start) / CLOCKS_PER_SEC) * 1000 > timeout) { // to ms
            return;
        }
        isPrime(i); // Execute CPU-bound work
        i++;
    }
}

bool isPrime(int n) {
    if (n < 2) {
        return false;
    }
    for (int i = 2; i <= (int)sqrt((double)n); i++) {
        if (n % i == 0) {
            return false;
        }
    }
    return true;
}


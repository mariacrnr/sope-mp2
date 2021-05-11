#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Queue Queue;

struct Queue {
    int front, back, size, capacity;
    int* array;
};

struct Queue* startQueue(int size);

int push(struct Queue* queue, int value);

int pop(struct Queue* queue);

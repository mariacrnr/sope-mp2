#include "queue.h"

struct Queue* startQueue(int size){
    struct Queue* queue = (struct Queue*) malloc(sizeof(struct Queue));

    queue->front = 0;
    queue->size = size;
    queue->back = size - 1;

    queue->capacity = 0;

    queue->array = (int*) malloc(queue->size * sizeof(int));

    return queue;
}

int push(struct Queue* queue, int value){
    if(queue->size == queue->capacity) return 1;
}

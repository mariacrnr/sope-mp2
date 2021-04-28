#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

typedef struct pthreadLinked pthreadLinked;

struct pthreadLinked {
   pthread_t thread;
   pthreadLinked* next;
};


void startLinkedList(pthread_t thread, pthreadLinked* start);

void insertThread(pthread_t thread, pthreadLinked* current);

void freeLinkedList(pthreadLinked* start);
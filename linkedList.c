#include "linkedList.h"

void startLinkedList(pthread_t thread, pthreadLinked* start) {

    start = (pthreadLinked*) malloc(sizeof(pthreadLinked));
    start->thread = thread;

}


void insertThread(pthread_t thread, pthreadLinked* current) {
    
    pthreadLinked* newThread = (pthreadLinked*) malloc(sizeof(pthreadLinked));
    newThread->thread = thread;
    current->next = newThread;
    current = newThread;

}


void freeLinkedList(pthreadLinked* start) {
    while (start != NULL) {
        pthreadLinked* tempLink = start;
        start = start->next;
        free(tempLink);
    }
}

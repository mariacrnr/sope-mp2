#include "linkedList.h"

void startLinkedList(pthread_t thread, pthreadLinked** start) {

    *start = (pthreadLinked*) malloc(sizeof(pthreadLinked));

    if ((*start) != NULL) {
        (*start)->thread = thread;
        (*start)->next = NULL;
    }
    
}


void insertThread (pthread_t thread, pthreadLinked** current) {

    pthreadLinked* newThread = (pthreadLinked*) malloc(sizeof(pthreadLinked));

    if (newThread != NULL) {
        newThread->thread = thread;
        newThread->next = NULL;
        (*current)->next = newThread;
        (*current) = newThread; 
    }
    
}


void freeLinkedList(pthreadLinked** start) {

    while (*start != NULL) {
        pthreadLinked* tempLink = *start;
        *start = (*start)->next;
        free(tempLink);
    }

}

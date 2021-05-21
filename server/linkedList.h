#ifndef SERVER_LINKEDLIST_H_
#define SERVER_LINKEDLIST_H_ 1

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct pthreadLinked pthreadLinked;

//Struct that stores a linked list object
struct pthreadLinked {
   pthread_t thread; //Thread to store
   pthreadLinked* next; //Next element in the linked list
};

/**
 * @brief Function that starts the linked list with the first thread
 * 
 * @param thread First thread to be added to the linked list
 * @param start First linked list object
 * 
 */
void startLinkedList(pthread_t thread, pthreadLinked** start);

/**
 * @brief Function that inserts a new thread in the linked list
 * 
 * @param thread Thread to be added to the linked list
 * @param current Current linked list object
 * 
 */
void insertThread(pthread_t thread, pthreadLinked** current);

/**
 * @brief Function that frees the linked list and all it's objects
 * 
 * @param start First linked list object
 * 
 */
void freeLinkedList(pthreadLinked**  start);

#endif // SERVER_LINKEDLIST_H_
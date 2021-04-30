#ifndef INCLUDE_CLIENT_AUX_H_
#define INCLUDE_CLIENT_AUX_H_ 1

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>

#include "../include/common.h"

typedef struct {
    int requestId;
    int fifoID;
    unsigned int seed;
} routineArgs;

/**
 * @brief Function that prints the operations executed by the program
 * 
 * @param message information associated with the operation.
 * @param oper operation that occurred. 
 * 
 */
void registOperation(Message message, const char* oper);

/**
 * @brief Function that handles all the functions that remove or free objects in the routine function
 * 
 * @param privateFifo char * that holds the name of the private fifo
 * @param params struct thats stores each thread's arguments
 * 
 */
void cleanup (char* privateFifo, routineArgs* params);

/**
 * @brief Function that completes the "message" variable.
 * 
 * @param message variable to be completed
 * @param requestID ID of the client request. 
 * 
 */
void parseMessage(Message* message, int requestID, unsigned int seed);

#endif // INCLUDE_CLIENT_AUX_H_ 
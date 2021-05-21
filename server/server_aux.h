#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>
#include <semaphore.h>
#include <pthread.h>
#include <signal.h>

#include "common.h"
#include "lib.h"

/**
 * @brief Function that prints the operations executed by the program
 * 
 * @param message information associated with the operation.
 * @param oper operation that occurred. 
 * 
 */
void registOperation(Message message, const char* oper);

/**
 * @brief Function that completes the "message" variable.
 * 
 * @param message variable to be completed
 * @param requestID ID of the client request. 
 * 
 */
void parseMessage(Message* message);

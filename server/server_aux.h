#ifndef SERVER_SERVER_AUX_H_
#define SERVER_SERVER_AUX_H_ 1

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

#include "../server/common.h"
#include "../server/lib.h"


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
 * 
 */
void parseMessage(Message* message);

/**
 * @brief Function that handles the SIGPIPE signal
 * 
 * @param signo variable to that holds the signal
 * 
 */
void pipeHandler(int signo);

#endif // SERVER_SERVER_AUX_H_
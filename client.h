#ifndef _CLIENT_H
#define _CLIENT_H 1

#include "macros.h"
#include "client_aux.h"
#include "delay.h"
#include "linkedList.h"

//Global variable that turns to 1 if the Server times out
int cancel = 0;

//Global variable that turns to 1 if the Client times out
int timedOut = 0;

//Mutex that manages the access to the Cancel variable
pthread_mutex_t threadCancelMutex;

//Mutex that manages the access to the TimedOut variable
pthread_mutex_t TimedOutMutex;


/**
 * @brief Main function of the Client program that parses the command line arguments and opens the public FIFO and invokes ClientTaskManager;
 * 
 * @param argc Number of command line arguments. 
 * @param argv Array of strings with the command line arguments.
 *
 * @return Returns 0 if no errors occurred, 1 otherwise.
 */
int main(int argc, char* argv[]);

/**
 * @brief Function of the Client program that creates new threads, that resemble Client requests, until the Server or the Clients times out; 
 * 
 * @param publicFifoFD File Descriptor of the public FIFO
 * @param t time in seconds until client shuts down
 *
 * @return Returns 0 if no errors occurred, 1 otherwise.
 */
int clientTaskManager(int publicFifoFD, int t);


#endif // _CLIENT_H    
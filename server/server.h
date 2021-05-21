#ifndef SERVER_SERVER_H_
#define SERVER_SERVER_H_ 1

#include "../server/macros.h"
#include "../server/server_aux.h"
#include "../server/linkedList.h"

//Semaphore that handles race conditions when modifying bufferEmpty variable
sem_t semBufferEmpty;

//Semaphore that handles race conditions when modifying bufferFull variable
sem_t semBufferFull;

//Message array of size bufsz where requests are written to by producer threads and read from by the consumer thread
Message* buffer;



//Global variable that keeps track of next available index when writing to the buffer
int producerIndex;

//Global variable that keeps track of next available index when reading from the buffer
int consumerIndex;



//Global variable that stores buffer size
int bufsz;

//Global variable that turns to 1 if the Server times out
int timeOut;

//Global variable that stores number of unreadMessages written on buffer.
//Increased at every producer thread creation, and decreased at every consumer thread while loop iteration
int unreadMessages;

//Mutex that manages the access to the buffer variable
pthread_mutex_t bufferMutex;

//Mutex that manages the access to the unreadMessages variable
pthread_mutex_t unreadMessagesMutex;

/**
 * @brief Function that is called by every producer thread when it is created.
 * Writes to the buffer the message sent as argument.
 * 
 * @param arg A void pointer pointing to a Message variable containing information to be used in the function
 *
 * @return A NULL void pointer since the return value of the threads is not meaningful
 */
void* routineProducer(void* arg);

/**
 * @brief Function that is called by the consumer thread when it is created. 
 * Repeatedly tries to read from buffer until server times out and buffer is empty.
 * 
 * @param arg A void pointer pointing to a Message variable containing information to be used in the function
 *
 * @return A NULL void pointer since the return value of the threads is not meaningful
 */
void* routineConsumer(void* arg);

/**
 * @brief Main loop. Creates one consumer thread, then keeps on creating producer threads until server times out.
 *
 * @param t Time in seconds until server times out
 * @param publicFD File Descriptor of the public FIFO
 * @param publicFIFO 
 * 
 * @return Returns 0 if no errors occured, 1 otherwise.
 */
int requestReceiver(int t, int publicFD, char* publicFIFO);

/**
 * @brief Main function of the Server program that parses the command line arguments, 
 * initializes global variables, opens the public FIFO and invokes requestReceiver;
 * 
 * @param argc Number of command line arguments. 
 * @param argv Array of strings with the command line arguments.
 *
 * @return Returns 0 if no errors occurred, 1 otherwise.
 */
int main(int argc, char* argv[]);
#endif // SERVER_SERVER_H_

#include "../include/client.h"

void* routine(void* arg) {

    int privateFD, ret;

    routineArgs* params = arg;

    Message message;
    parseMessage(&message, params->requestId, params->seed); // Initializes the various attributes of this Message instance
    
    char* privateFifo = malloc(MAX_BUF); 
    snprintf(privateFifo, MAX_BUF, "/tmp/%d.%lu", message.pid, message.tid); // Names a FIFO based on the process id and thread id

    if (mkfifo(privateFifo, 0666) == -1) { // Creates FIFO for communication between this thread and the server 
        perror("Error creating private FIFO");
        cleanup(privateFifo,params);
        pthread_exit(NULL);
    }

    if (write(params->fifoID, &message, sizeof(message)) == -1) { 
        perror("Error writing message to FIFO"); 
        cleanup(privateFifo,params);  
        pthread_exit(NULL);
	}

    registOperation(message, "IWANT");

    if ((privateFD = open(privateFifo, O_RDONLY | O_NONBLOCK, 0666)) < 0) {
        perror("Erro opening private file descriptor");
        cleanup(privateFifo,params);
        pthread_exit(NULL);
    }


    while ((ret = read(privateFD, &message, sizeof(message))) != sizeof(message)) { // Loops until reading successfully from private file descriptor or if time out occurs
        if (timedOut == 1) {
            registOperation(message, "GAVUP");
            close(privateFD);
            cleanup(privateFifo,params);
            pthread_exit(NULL);
        }
    }

    if (message.tskres == -1) { // If server timed out and, consequently, sent to the thread a -1 task result
        registOperation(message, "CLOSD");

        pthread_mutex_lock(&threadCancelMutex);
        cancel = 1; // Stops thread creation loop
        pthread_mutex_unlock(&threadCancelMutex);
        
    } else { // If no problem occurred
        registOperation(message, "GOTRS");
    }

    close(privateFD);
    cleanup(privateFifo,params);
    pthread_exit(NULL);
}

int clientTaskManager(int publicFifoFD, int t){

    struct timespec sleepTime;      // Struct used to store time durations
    sleepTime.tv_sec = 0;           
    sleepTime.tv_nsec = SLEEP_TIME;    

    int nthreads = 0; // Thread counter

    pthreadLinked* start = NULL;    // First pthread at linked list. Is used as a pseudo constant variable, since the first element rarely changes
    pthreadLinked* current = NULL;  // Current pthread at linked list

    pthread_t thread; // Declares a pthread_t variable, where a thread will be created at every while loop iteration

    time_t initT = time(NULL);      // Stores the while's first iteration's exact time 
    time_t nowT;                    // Will store each while iteration's exact time

    unsigned int seed = time(NULL); //Starts the random seed variable for the rand_r() function

    while ( (time(&nowT) - initT < t)  && (cancel == 0)) {  // Verifies if time elapsed since first iteration exceeds maximum time,
                                                            // or if cancel flag is activated
        nthreads++;

        routineArgs* args = malloc(sizeof(*args)); // Will store each thread's arguments
        
        args->requestId = nthreads;  // The current thread counter is used as the thread's Id, since it'll be unique for each iteration and, therefore, thread 
        args->fifoID = publicFifoFD; // Stores 
        args->seed = seed + nthreads;

        if (pthread_create(&thread, NULL, routine, args) != 0) {  
            perror("Error creating new thread");
            return 1;
        }

        if (nthreads == 1) { // If it is the first thread, startLinkedList is called and the linked list is initialized
            startLinkedList(thread, &start); // Initializes the first element of the linked list, which will be pointed to by "start" pointer
            current = start; // At this stage, the starting element is the current element. This will be changed with further insertions.
        }

        else insertThread(thread, &current); // Adds new element to the linked list, which will be pointed to by the "current" pointer

        nanosleep(&sleepTime, NULL); // Pauses thread's creation loop for sleepTime time
    }

    if (cancel == 0) // If the Server didn't time out, then it means it was the Client who timed out
        timedOut = 1; // Client's timeout flag is activated 
    
    
    current = start; // Starting at the first thread
    
    while (current != NULL) { // Until the thread pointed to by the last thread is null
        if (pthread_join(current->thread, NULL) != 0) { // Join thread
            perror("Error joining threads");
            return 1;
        }
        current = current->next; // Jumps to next thread
    }

    close(publicFifoFD); // Ceases communication between server and client
    freeLinkedList(&start); // Frees all memory allocated by the thread linked list

    return 0;
}

int main(int argc, char *argv[]) {

    int publicFifoID;

    pthread_mutex_init(&threadCancelMutex, NULL);
    pthread_mutex_init(&TimedOutMutex, NULL);

    if (argc !=  4) {
        perror("Error at number of arguments");
        return 1;
    }
    
    while((publicFifoID = open(argv[3], O_WRONLY)) == -1); // Waits until public FIFO is opened by the server

    if (clientTaskManager(publicFifoID, atoi(argv[2])) == 1) { // Calls clientTaskManager and handles errors
        perror("Error at clientTaskManager");
        return 1;
    }

    pthread_mutex_destroy(&threadCancelMutex);
    pthread_mutex_destroy(&TimedOutMutex);
    
    return 0;
}

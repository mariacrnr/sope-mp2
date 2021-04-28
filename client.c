#include "client.h"

void* routine(void* arg) {
    int privateFD, ret;

    routineArgs* params = arg;

    Message message;
    parseMessage(&message, params->requestId);
    
    char* privateFifo = malloc(MAX_BUF);
    snprintf(privateFifo, MAX_BUF, "/tmp/%d.%lu", message.pid, message.tid);

    if (mkfifo(privateFifo, 0666) == -1) {
        perror("Error creating privateFifo\n");
        cleanup(privateFifo,params);
        pthread_exit(NULL);
    }

    if (write(params->fifoID, &message, sizeof(message)) == -1) { 
        cleanup(privateFifo,params);  
        pthread_exit(NULL);
	}

    registOperation(message, "IWANT");


    if ((privateFD = open(privateFifo, O_RDONLY)) == -1) {
        cleanup(privateFifo,params);
        pthread_exit(NULL);
    }
    
    if ((ret = read(privateFD, &message, sizeof(message))) != sizeof(message)) {

        if (timedOut == 1) 
            registOperation(message, "GAVUP");
        else 
            perror("Error in read\n");

    } else if (message.tskres == -1) {
        registOperation(message, "CLOSD"); // Ao detetar um CLOSD, parar a produção de threads

        pthread_mutex_lock(&threadCancelMutex);
        cancel = 1;
        pthread_mutex_unlock(&threadCancelMutex);
        
    } else {
        registOperation(message, "GOTRS");
    }

    close(privateFD);
    cleanup(privateFifo,params);
    pthread_exit(NULL);
}

int clientTaskManager(int publicFifoFD, int t){

    struct timespec sleepTime;      // Struct used to store time durations
    sleepTime.tv_sec = 0;           // Stores 0 full seconds, plus...
    sleepTime.tv_nsec = 3000000;    // 3000000 nano seconds, which equals to 3ms

    int nthreads = 0; // Initializes a thread counter with 0

    pthreadLinked* start = NULL;    // First pthread at linked list. Is used as a pseudo constant variable, since the first element rarely changes
    pthreadLinked* current = NULL;  // Current pthread at linked list

    pthread_t thread; // Declares a pthread_t variable, where a thread will be created at every while loop iteration

    time_t initT = time(NULL);      // Stores the while's first iteration's exact time 
    time_t nowT;                    // Will store each while iteration's exact time

    while ( (time(&nowT) - initT < t)  && (cancel == 0)) {  // Verifies if time elapsed since first iteration exceeds maximum time,
                                                            // or if cancel flag is activated
        nthreads++; // Increments thread counter

        routineArgs* args = malloc(sizeof(*args)); // Will store each thread's arguments
        
        args->requestId = nthreads;  // The current thread counter is used as the thread's Id, since it'll be unique for each iteration and, therefore, thread 
        args->fifoID = publicFifoFD; // Stores 

        if (pthread_create(&thread, NULL, routine, args) != 0) {  // Creates the thread passed as reference, also calling the routine function with args passed as parameter ( Funtion that each thread is gonna execute).
            perror("Error creating new thread");
            return 1;
        }

        if (nthreads == 1) {     // If it is the first thread, startLinkedList is called and the linked list is initialized
            startLinkedList(thread, &start);
            current = start;
        }
        else insertThread(thread, &current);

        nanosleep(&sleepTime, NULL);
    }
    
    if (cancel == 0) { 
        timedOut = 1; 
        for (int c = 0; c <= nthreads; c++) {
            close(c + 3);
        }  
    }
    
    current = start;
    while (current != NULL) {
        if (pthread_join(current->thread, NULL) != 0) return 1;
        current = current->next;
    }

    close(publicFifoFD);
    freeLinkedList(&start);

    return 0;
}

int main(int argc, char *argv[]) {

    srand(time(NULL));  //Starts the random seed variable for the rand() function

    int publicFifoID;

    pthread_mutex_init(&threadCancelMutex, NULL);
    pthread_mutex_init(&TimedOutMutex, NULL);

    if (argc !=  4) {
        perror("Error at number of arguments\n");
        return 1;
    }
    
    while((publicFifoID = open(argv[3], O_WRONLY)) == -1); // Waits for public FIFO to be opened by the server

    if (clientTaskManager(publicFifoID, atoi(argv[2])) == 1) { // Calls clientTaskManager and handles errors
        perror("Error at clientTaskManager\n");
        return 1;
    }

    pthread_mutex_destroy(&threadCancelMutex);
    pthread_mutex_destroy(&TimedOutMutex);
    
    return 0;
}

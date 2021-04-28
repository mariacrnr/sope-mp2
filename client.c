#include "client.h"

void registOperation(Message message, char* oper) {
    printf("%ld ; %d ; %d ; %d ; %ld ; %d ; %s\n",
            time(NULL), message.rid, message.tskload, message.pid, message.tid, message.tskres, oper);
}

void* routine(void* arg) {
    int privateFD, ret;

    routineArgs* params = arg;
    Message message;
    
    srand(time(NULL));   // Initialization, should only be called once.
    int r = rand() % 10;

    message.rid = params->requestId;
    message.tskload = r;
    message.tid = pthread_self();
    message.tskres = -1;
    
    char* privateFifo = malloc(MAX_BUF);
    
    snprintf(privateFifo, MAX_BUF, "/tmp/%d.%lu", message.pid, message.tid);

    if (mkfifo(privateFifo, 0666) == -1) {
        perror("Error creating privateFifo\n");
        remove(privateFifo);
        free(privateFifo);
        free(params);
        pthread_exit(NULL);
    }
    
    
    //pthread_mutex_lock(&clientMutex);

    if (write(params->fifoID, &message, sizeof(message)) == -1) { 
       
        remove(privateFifo);
        free(privateFifo);
        free(params);
		pthread_exit(NULL); 
	}

    //pthread_mutex_unlock(&clientMutex);

    registOperation(message, "IWANT");


    if ((privateFD = open(privateFifo, O_RDONLY)) == -1) {
    
        remove(privateFifo);
        free(privateFifo);
        free(params);
        pthread_exit(NULL);
    }
    //printf("------------ PrivateFD Writer: %d\n", privateFD);
    /*printf("PRINT READ %d\n", ret);
    printf("------------ PrivateFD Receiver: %d\n", privateFD);*/
    
    if ((ret = read(privateFD, &message, sizeof(message))) <= 0) {
        //printf("===========Read failed!\n");
        //pthread_mutex_lock(&TimedOutMutex);
        if (timedOut == 1) 
            registOperation(message, "GAVUP");
        else 
            printf("Error in read\n");
        //pthread_mutex_unlock(&TimedOutMutex);

    } else if (message.tskres == -1) {
        registOperation(message, "CLOSD"); //mal detetar um CLOSD, parar a produção de threads!!!!

        pthread_mutex_lock(&threadCancelMutex);
        cancel = 1;
        pthread_mutex_unlock(&threadCancelMutex);
        
        
        printf("CHANGED CANCEL\n");
    } else {
        registOperation(message, "GOTRS");
    }

    close(privateFD);
    remove(privateFifo);
    free(privateFifo);
    free(params);
    
    
    // reads the response and closes the private fifo
    //printf("------------------------%li DIED---------------------\n", pthread_self());
    pthread_exit(NULL);
}

int clientTaskManager(int publicFifoFD, int t){

    struct timespec sleepTime;
    sleepTime.tv_sec = 0;
    sleepTime.tv_nsec = 3000000;

    pthread_t thread;
    int nthreads = 0;

    pthreadLinked* start = NULL;
    pthreadLinked* current = NULL;

    time_t initT = time(NULL);
    time_t nowT;

    while ( (time(&nowT) - initT < t)  && (cancel == 0)) { 
        nthreads++;

        routineArgs* args = malloc(sizeof(*args));
        
        args->requestId = nthreads;
        args->fifoID = publicFifoFD;

        if (pthread_create(&thread, NULL, routine, args) != 0) {
            perror("Error creating new thread");
            return 1;
        }

        if (nthreads == 1) startLinkedList(thread, start);
        else insertThread(thread, current);
        
        
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
    freeLinkedList(start);

    return 0;
}

int main(int argc, char *argv[]) {
    int publicFifoID;

    pthread_mutex_init(&threadCancelMutex, NULL);
    pthread_mutex_init(&TimedOutMutex, NULL);
    //pthread_mutex_init(&clientMutex, NULL);

    if (argc !=  4) {
        perror("Error at number of arguments\n");
        return 1;
    }
    
    while((publicFifoID = open(argv[3], O_WRONLY)) == -1);

    if (clientTaskManager(publicFifoID, atoi(argv[2])) == 1) {
        perror("Error at clientTaskManager\n");
        return 1;
    }


    //pthread_mutex_destroy(&clientMutex);

    //printf("FIM\n");
    
    return 0;
}

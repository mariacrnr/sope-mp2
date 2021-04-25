#include "client.h"


void registOperation(Message message, char* oper) {
    printf("%ld ; %d ; %d ; %d ; %lu ; %d ; %s\n",
            time(NULL), message.rid, message.tskload, message.pid, message.tid, message.tskres, oper);
}

void cleanupHandler(void *arg) {

    routineArgs *params = arg;

    close(params->privateFifoID);
    remove(params->privateFifoName);
    free(params);

    printf("In the cleanup handler\n");
}


void* routine(void* arg) {
    int privateFD, ret;

    routineArgs *params = arg;
    Message message;
    
    srand(time(NULL));   // Initialization, should only be called once.
    int r = rand() % 10;

    message.rid = params->requestId;
    message.tskload = r;
    message.pid = getpid();
    message.tid = gettid();
    message.tskres = -1;
    
    char* privateFifo = malloc(MAX_BUF);
    
    snprintf(privateFifo, MAX_BUF, "/tmp/%d.%lu", message.pid, message.tid);

    if (mkfifo(privateFifo, O_WRONLY) == -1) {
        perror("Error creating privateFifo\n");
        pthread_exit(NULL);
    }
    
    
    pthread_mutex_lock(&clientMutex);

    if (write(params->fifoID, &message, sizeof(message)) == -1) { 
		pthread_exit(NULL);
	}

    pthread_mutex_unlock(&clientMutex);

    registOperation(message, "IWANT");


    if ((privateFD = open(privateFifo, O_RDONLY)) == -1) {
        pthread_exit(NULL);
    }

    
    params->privateFifoID = privateFD;
    params->privateFifoName = privateFifo;

    pthread_cleanup_push(cleanupHandler, (void *) params); // limpa e dá free a cenas
    pthread_cleanup_pop(0);

    if ((ret = read(privateFD, &message, sizeof(message))) == -1) { // deve ficar aqui parado à espera de resposta do Service;
        printf("Failed to read FIFO\n");
        pthread_exit(arg); //nao conseguiu fazer o read
    }
    else {
        if (message.tskres == -1) {
            registOperation(message, "CLOSD"); //mal detetar um CLOSD, parar a produção de threads!!!!

            pthread_mutex_lock(&threadCancelMutex);
            cancel = 1;
            pthread_mutex_unlock(&threadCancelMutex);
                
        } else {
            registOperation(message, "GOTRS");
        }
    }


    pthread_mutex_lock(&threadCounterMutex);

    ThreadsFinished++;

    pthread_mutex_unlock(&threadCounterMutex);

    // reads the response and closes the private fifo
    pthread_exit(NULL);
}

int clientTaskManager(char* fifoname, int t){

    int fd;
    if ((fd = open(fifoname, O_WRONLY)) == -1) {
        perror("Error opening fifo");
        return 1;
    }

    struct timespec sleepTime;
    sleepTime.tv_sec = SLEEP_TIME;     

    int nthreads = 0;
    pthread_t* threads;

    time_t initT = time(NULL);
    time_t nowT;


    while ( (time(&nowT) - initT < t)  && (cancel == 0)) { // e o fecho do server
        nthreads++;

        threads = (pthread_t*) realloc(threads, sizeof(pthread_t) * nthreads);

        routineArgs* args = malloc(sizeof(*args));
        
        args->requestId = nthreads;
        args->fifoID = fd;
        //args->
        
        if (pthread_create(&threads[nthreads], NULL, routine, args) != 0) {
            perror("Error creating new thread");
            break;
        }

        //usleep(delay*1000);

        nanosleep(&sleepTime, NULL);
    }

    close(fd);
    
    for(int c = ThreadsFinished ; c < nthreads; c++){
        
        pthread_cancel(threads[c]);
        
    }

    for (int i = 0; i < ThreadsFinished; i++) {
        void *res;
        pthread_join(threads[i], &res);	// Note: threads give no termination code
        printf("%i\n", *((int*)res));
        printf("\nTermination of thread %d: %lu.\n", i, (unsigned long)threads[i]);
    }


    free(threads);
    

    return 0;
}

int main(int argc, char *argv[]) {

    pthread_mutex_init(&clientMutex, NULL);
    pthread_mutex_init(&threadCounterMutex, NULL);
    pthread_mutex_init(&threadCancelMutex, NULL);

    if (argc !=  4) {
        perror("Error at number of arguments\n");
        return 1;
    }

    if (clientTaskManager(argv[3], atoi(argv[2])) == 1) {
        perror("Error at clientTaskManager\n");
        return 1;
    }

    pthread_mutex_destroy(&clientMutex);
    pthread_mutex_destroy(&threadCounterMutex);
    pthread_mutex_destroy(&threadCancelMutex);

    return 0;
}

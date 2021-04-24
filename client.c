#include "client.h"

pthread_mutex_t clientMutex;

void registOperation(Message message, char* oper) {
    printf("%ld ; %d ; %d ; %d ; %lu ; %d ; %s\n",
            time(NULL), message.rid, message.tskload, message.pid, message.tid, message.tskres, oper);
}


void* routine(void* arg) {
    int privateFD;

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
    char* clientMessage = malloc(MAX_BUF);
    
    snprintf(privateFifo, MAX_BUF, "/tmp/%d.%lu", message.pid, message.tid);
    snprintf(clientMessage, MAX_BUF, "%d ; %d ; %d ; %lu ; %d\n", message.rid, message.tskload, message.pid, message.tid, message.tskres);

    if (mkfifo(privateFifo, O_WRONLY) == -1) {
        perror("Error creating privateFifo\n");
        pthread_exit(arg);
    }
    
    
    pthread_mutex_lock(&clientMutex);

    if (write(params->fifoID, &clientMessage, sizeof(clientMessage) == -1)) {
		pthread_exit(arg);
	}

    pthread_mutex_unlock(&clientMutex);

    registOperation(message, "completar com mensagem correta");


    if ( (privateFD = open(privateFifo, O_RDONLY)) == -1){
        pthread_exit(arg);
    }

    //while(time > 0){ //ta mal, mas é necessário verificar tempo

        if(read(privateFD, &clientMessage, sizeof(clientMessage)) == -1){
            pthread_exit(arg);
        }
    

    close(privateFD);

    


    // reads the response and closes the private fifo
    pthread_exit(NULL);
}

int clientTaskManager(char* fifoname, int t){
    
    routineArgs* args = malloc(sizeof(*args));

    int fd;
    if ((fd = open(fifoname, O_WRONLY)) == -1){
        perror("Error opening fifo");
        pthread_exit(args);
    }

    args->fifoID = fd;

    struct timespec sleepTime;
    sleepTime.tv_sec = SLEEP_TIME;     

    int nthreads = 0;
    pthread_t* threads;

    time_t initT = time(NULL);
    time_t nowT;

    while (time(&nowT) - initT < t ) {
        nthreads++;

        threads = (pthread_t*) realloc(threads, sizeof(pthread_t) * nthreads);

        args->requestId = nthreads;
        
        if (pthread_create(&threads[nthreads], NULL, routine, args) != 0) {
            perror("Error creating new thread");
            break;
        }

        nanosleep(&sleepTime, NULL);
    }

    close(fd);

    for (int i = 0; i < nthreads + 1; i++) {
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
    
    if (argc !=  4) {
        perror("Error at number of arguments\n");
        return 1;
    }

    if (clientTaskManager(argv[3], atoi(argv[2])) == 1) {
        perror("Error at clientTaskManager\n");
        return 1;
    }

    pthread_mutex_destroy(&clientMutex);

    return 0;
}

#include "client.h"


void registOperation(Message message, char* oper) {
    printf("%ld ; %d ; %d ; %d ; %ld ; %d ; %s\n",
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

    routineArgs* params = arg;
    Message message;
    
    srand(time(NULL));   // Initialization, should only be called once.
    int r = rand() % 10;

    message.rid = params->requestId;
    message.tskload = r;
    message.pid = getpid();
    message.tid = pthread_self();
    message.tskres = -1;
    
    char* privateFifo = malloc(MAX_BUF);
    
    snprintf(privateFifo, MAX_BUF, "/tmp/%d.%lu", message.pid, message.tid);

    if (mkfifo(privateFifo, 0666) == -1) {
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

    pthread_mutex_lock(&threadCounterMutex);

    nprivateFDS++;
    
    memcpy(&privateFDS[nprivateFDS], &privateFD, sizeof(int));

    pthread_mutex_unlock(&threadCounterMutex);


    
    params->privateFifoID = privateFD;
    params->privateFifoName = privateFifo;

    

    // pthread_cleanup_push(cleanupHandler, (void *) params); // limpa e dá free a cenas
    // pthread_cleanup_pop(0);

    //while(ret == 0)

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

        //coidog usado no metodo pthread_cancel (subtituido pelo metodo do "file discriptor mass closing")
    // pthread_mutex_lock(&threadCounterMutex);

    // ThreadsFinished++;

    // pthread_mutex_unlock(&threadCounterMutex);

    // close(params->privateFifoID);
    // remove(params->privateFifoName);
    free(params);

    // reads the response and closes the private fifo
    pthread_exit(NULL);
}

int clientTaskManager(char* fifoname, int t){

    int fd;
    if ((fd = open(fifoname, O_WRONLY)) == -1) {
        perror("Error opening fifo");
        return 1;
    }

    // struct timespec sleepTime;
    // sleepTime.tv_sec = SLEEP_TIME;
    
    pthread_t* threads;
    int nthreads = 0;

    time_t initT = time(NULL);
    time_t nowT;


    while ( (time(&nowT) - initT < t)  && (cancel == 0)) { // e o fecho do server
        nthreads++;
        nprivateFDS++;

        if (nthreads == 1) {
            threads = (pthread_t*) malloc(sizeof(pthread_t));
            privateFDS = (int *) malloc(sizeof(int));
        } else{
            threads = (pthread_t*) realloc(threads, sizeof(pthread_t) * nthreads);
            privateFDS = (int *) realloc(privateFDS, sizeof(int) * nprivateFDS);
        }

        routineArgs* args = malloc(sizeof(*args));
        
        args->requestId = nthreads;
        args->fifoID = fd;
        
        if (pthread_create(&threads[nthreads-1], NULL, routine, args) != 0) {
            perror("Error creating new thread");
            break;
        }


        sleep(1);
        //nanosleep(&sleepTime, NULL);
    }
    
    for(int c = 0; c < nprivateFDS; c++){
        close(privateFDS[c]);
        remove(privateFDS[c]);
    }
    
    // for(int c = ThreadsFinished ; c < nthreads; c++){
        
    //     pthread_cancel(threads[c]);
        
    // }

    for (int i = 0; i < nthreads; i++) {
        void *res;
        pthread_join(threads[i], &res);	// Note: threads give no termination code
        printf("%i\n", *((int*)res));
        printf("\nTermination of thread %d: %lu.\n", i, (unsigned long)threads[i]);
    }

    free(threads);
    free(privateFDS);
    close(fd);

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

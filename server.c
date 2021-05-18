#include "server.h"

void sigHandler(int signo){
    closeConsumer = 1;
}

void* routineProducer(void* arg) {
    Message* params = arg;

    //pthread_mutex_lock(&timeOutMutex);
    params->tskres = (timeOut) ?  TSKTIMEOUT : task(params->tskload);
    //pthread_mutex_unlock(&timeOutMutex);
    
    //printf("ESTOU PRESTES A ENTRAR NO SEMAFORO DO PRODUCER\n");
    sem_wait(&semBufferEmpty); // Fica preso no timeout do server
    pthread_mutex_lock(&bufferMutex);

    //printf("ESTOU DENTRO DO SEMAFORO DO PRODUCER!\n");
    buffer[producerIndex] = *params;
    producerIndex++;
    producerIndex %= bufsz;

    //printf("ESTOU PRESTES A SAIR DO SEMAFORO DO PRODUCER!\n");
    pthread_mutex_unlock(&bufferMutex);
    sem_post(&semBufferFull);

    //Message serverMessage = *params;
    parseMessage(&(*params));
    registOperation(&(*params), "TSKEX");

    free(params);
    pthread_exit(NULL);
}   

void* routineConsumer(void* arg) {
    
    Message message;
    char privateFifo[MAX_BUF];
    int privateID;
    int value = 0;

    
    while(!closeConsumer || value != 0) {  // !closeconsumer == 1 && value == 0)
        sem_getvalue(&semBufferFull, &value);

        //if (value == 0 && closeConsumer) break;
        //printf("zzzzz STUCK zzzzz value: %d\n", value);
        //printf("================ESTOU PRESTES A ENTRAR NO SEMAFORO DO CONSUMER================\n");
        if (sem_trywait(&semBufferFull) != 0) continue; // Fica preso no timout do Cliente
        
        pthread_mutex_lock(&bufferMutex);
        //printf("ESTOU DENTRO DO SEMAFORO CONSUMER!\n");
        message = buffer[consumerIndex];
        consumerIndex++;
        consumerIndex %= bufsz;

        pthread_mutex_unlock(&bufferMutex);
        sem_post(&semBufferEmpty);
        //envia para respetivo private fifo
        
        //printf("==================ESTOU FORA DO SEMAFORO DO CONSUMER!=============\n");
        
        snprintf(privateFifo, MAX_BUF, "/tmp/%d.%lu", message.pid, message.tid);

        parseMessage(&message);

        //printf("PRESTES A DAR OPEN DA PRIV FIFO\n");

        if ((privateID = open(privateFifo, O_WRONLY)) < 0){
            registOperation(&message, "FAILD"); 
            clientTimeOut = 1;     
        }else{
            //printf("PRESTES A DAR WRITE NA PRIV FIFO\n");


            if (write(privateID, &message, sizeof(Message)) == -1) { 
                registOperation(&message, "FAILD");
                clientTimeOut = 1;
            }
            else if(timeOut) registOperation(&message, "2LATE");
            else registOperation(&message, "TSKDN");
            

        }

        //close(privateID);
        
        //printf("<<<<<<<<<<<<<<<<DONE>>>>>>>>>>>>>>>\n");
    }

    //free(privateFifo);

    pthread_exit(NULL);

}

int requestReceiver(int t, int publicFD, char * publicFIFO, int bufferSize){
    int nthreads = 0;
    int ret;
    Message message;
    pthread_t thread, consumerThread;

    pthreadLinked* start = NULL;    // First pthread at linked list. Is used as a pseudo constant variable, since the first element rarely changes
    pthreadLinked* current = NULL;  // Current pthread at linked list


    time_t initT = time(NULL);      // Stores the while's first iteration's exact time 
    time_t nowT;                    // Will store each while iteration's exact time

    /*struct sigaction new, old;
	sigset_t smask;	// signals to mask during signal handler
	sigemptyset(&smask);
	new.sa_handler = sigHandler;
	new.sa_mask = smask;
	new.sa_flags = 0;	// usually enough

	if(sigaction(SIGKILL, &new, &old) == -1) {
		perror ("sigaction (SIGUSR1)");
        return 1;
	}*/

    //alarm(t);

    buffer = malloc(sizeof(Message) * bufferSize);


    if (pthread_create(&consumerThread, NULL, routineConsumer, NULL) != 0) {  
        free(buffer);
        perror("Error creating new thread");
        return 1;
    }
    //(time(&nowT) - initT < t)
    while (time(&nowT) - initT < t) { // condição mal, ainda tem q ser feito
        if(time(&nowT) - initT >= t) timeOut = 1;

        if ((ret = read(publicFD, &message, sizeof(Message))) == sizeof(Message)){ //6
            nthreads++;
            Message* args = malloc(sizeof(*args));

            *args = message;

            registOperation(&message, "RECVD");

            if (pthread_create(&thread, NULL, routineProducer, args) != 0) {  
                free(args);
                perror("Error creating new thread");
                return 1;
            }

            if (nthreads == 1) { // If it is the first thread, startLinkedList is called and the linked list is initialized
                startLinkedList(thread, &start); // Initializes the first element of the linked list, which will be pointed to by "start" pointer
                current = start; // At this stage, the starting element is the current element. This will be changed with further insertions.
            } else {
                insertThread(thread, &current); // Adds new element to the linked list, which will be pointed to by the "current" pointer
            }

        }

    }
    
    remove(publicFIFO);

    current = start; // Starting at the first thread

    while (current != NULL) { // Until the thread pointed to by the last thread is null
        if (pthread_join(current->thread, NULL) != 0) { // Join ProducerThreads
            free(buffer);
            freeLinkedList(&start);
            perror("Error joining threads");
            return 1;
        }
        current = current->next; // Jumps to next thread
    }

    closeConsumer = 1;
    
    /*int value;
    do{
        sem_getvalue(&semBufferFull, &value);
    }while(value != 0);
    
    kill(getpid(),SIGKILL);*/

    if (pthread_join(consumerThread, NULL) != 0) { //Joins Consumer thread
        free(buffer);
        freeLinkedList(&start);
        perror("Error joining threads");
        return 1;
    }

    free(buffer);
    freeLinkedList(&start);
    //close(publicFD); // Ceases communication between server and client
    return 0;
}


int main(int argc, char* argv[]) {
    char* publicFIFO;
    int nsecs, publicFD;

    pthread_mutex_init(&bufferMutex, NULL);
    pthread_mutex_init(&timeOutMutex, NULL);
    
    producerIndex = 0;
    consumerIndex = 0;

    timeOut = 0;
    closeConsumer = 0;
    clientTimeOut = 0;
    
    switch(argc){
        case UNDEF_BUFSZ:
           bufsz = DEFAULT_BUFSZ;   
           publicFIFO = argv[3];
           break;

        case DEF_BUFSZ:
            bufsz = (atoi(argv[4]) > 0) ? atoi(argv[4]) : DEFAULT_BUFSZ;
            publicFIFO = argv[5];
            break;

        default: 
            perror("Error at number of arguments");
            return 1;
    }

    sem_init(&semBufferEmpty, 0, bufsz);
    sem_init(&semBufferFull, 0, 0);
        
    nsecs = atoi(argv[2]);

    if (mkfifo(publicFIFO, PERM) == -1) { // Creates FIFO for communication between this thread and the server 
        perror("Error creating private FIFO");
        return 1;
    }

    if ((publicFD = open(publicFIFO, O_RDONLY | O_NONBLOCK, 0666)) == -1) {
        perror("Error opening public file descriptor");
        return 1;
    }

    if (requestReceiver(nsecs, publicFD, publicFIFO, bufsz) == 1){
        perror("Error in requestReceiver\n");
    }
    
    
    sem_destroy(&semBufferEmpty);
    sem_destroy(&semBufferFull);
    
    pthread_mutex_destroy(&bufferMutex);
    pthread_mutex_destroy(&timeOutMutex);

    return 0;
}
#include "server.h"

void* routineProducer(void* arg) {
    Message* params = arg;
    params->tskres = task(params->tskload);
    
    sem_wait(&semBufferEmpty);
    pthread_mutex_lock(&bufferMutex);
    buffer[counter] = *params;
    counter++;
    pthread_mutex_unlock(&bufferMutex);
    sem_post(&semBufferFull);

    pthread_exit(NULL);
}   

void* routineConsumer(void* arg) {
    
    Message message;
    char* privateFifo = malloc(MAX_BUF);
    int privateID;


    sem_wait(&semBufferFull);
    pthread_mutex_lock(&bufferMutex);
    message = buffer[counter - 1];
    counter--;
    pthread_mutex_unlock(&bufferMutex);
    sem_post(&semBufferEmpty);
    //envia para respetivo private fifo
    
    snprintf(privateFifo, MAX_BUF, "/tmp/%d.%lu", message.pid, message.tid);

    if ((privateID = open(privateFifo, O_WRONLY)) < 0){
        perror("Error opening private FIFO\n");
        pthread_exit(NULL);
    }  

    message.tid = pthread_self();
    message.pid = getpid();

    if (write(privateID, &message, sizeof(message)) == -1) { 
        perror("Error writing to private fifo");
        pthread_exit(NULL);
	}
    
    free(privateFifo);

    pthread_exit(NULL);

}

int requestReceiver(int t, int publicFD, int bufferSize){
    int nthreads = 0;
    int ret;
    Message message;
    pthread_t thread, consumerThread;

    pthreadLinked* start = NULL;    // First pthread at linked list. Is used as a pseudo constant variable, since the first element rarely changes
    pthreadLinked* current = NULL;  // Current pthread at linked list


    time_t initT = time(NULL);      // Stores the while's first iteration's exact time 
    time_t nowT;                    // Will store each while iteration's exact time

    buffer = malloc(sizeof(Message) * bufferSize);


    if (pthread_create(&consumerThread, NULL, routineConsumer, NULL) != 0) {  
        free(buffer);
        perror("Error creating new thread");
        return 1;
    }
    
    while (time(&nowT) - initT < t) {
        
        if ((ret = read(publicFD, &message, sizeof(message))) == sizeof(message)){
            nthreads++;
            Message* args = malloc(sizeof(*args));

            *args = message;

            if (pthread_create(&thread, NULL, routineProducer, NULL) != 0) {  
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

            free(args);
        }

    }

    if (pthread_join(consumerThread, NULL) != 0) { //Joins Consumer thread
        free(buffer);
        freeLinkedList(&start);
        perror("Error joining threads");
        return 1;
    }

    current = start; // Starting at the first thread
    
    while (current != NULL) { // Until the thread pointed to by the last thread is null
        if (pthread_join(current->thread, NULL) != 0) { // Join thread
            perror("Error joining threads");
            return 1;
        }
        current = current->next; // Jumps to next thread
    }

    free(buffer);
    freeLinkedList(&start);
    close(publicFD); // Ceases communication between server and client
    return 0;
}


int main(int argc, char* argv[]) {
    char* publicFIFO;
    int nsecs, bufsz, publicFD;

    pthread_mutex_init(&bufferMutex, NULL);

    counter = 0;
    
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

    // if ((semBufferEmpty = sem_open("semBufferEmpty", O_CREAT, 0666, bufsz)) == SEM_FAILED) {
    //     perror("Error inicializing the semaphore");
    //     return 1;
    // }

    // if ((semBufferFull = sem_open("semBufferFull", O_CREAT, 0666, 0)) == SEM_FAILED) {
    //     perror("Error inicializing the semaphore");
    //     return 1;
    // }
        

    nsecs = atoi(argv[2]);

    if (mkfifo(publicFIFO, PERM) == -1) { // Creates FIFO for communication between this thread and the server 
        perror("Error creating private FIFO");
        return 1;
    }

    if ((publicFD = open(publicFIFO, O_RDONLY | O_NONBLOCK, 0666)) == -1) {
        perror("Error opening public file descriptor");
        return 1;
    }

    if (requestReceiver(nsecs, publicFD, bufsz) == 1){
        perror("Error in requestReceiver\n");
    }
    
    // sem_unlink("semBufferEmpty");
    // sem_unlink("semBufferFull");
    sem_destroy(&semBufferEmpty);
    sem_destroy(&semBufferFull);
    pthread_mutex_destroy(&bufferMutex);

    return 0;
}
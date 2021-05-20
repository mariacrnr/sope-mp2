#include "server.h"

void* routineProducer(void* arg) {
    usleep(50000 + (rand() % 50000)); 
    Message* params = arg;
    if(!timeOut){ 
        params->tskres = task(params->tskload); //Lib function that gets the result of the task

        Message serverMessage = *params;
        parseMessage(&serverMessage); // Initializes the various attributes of this Message instance
        registOperation(&serverMessage, "TSKEX"); 

    } else{
        params->tskres = TSKTIMEOUT;
    }

    sem_wait(&semBufferEmpty); //Used to decrement the number of empty slots available at the buffer
    pthread_mutex_lock(&bufferMutex);

    buffer[producerIndex] = *params; //Array of messages corresponding to each request
    producerIndex++;
    producerIndex %= bufsz;

    pthread_mutex_unlock(&bufferMutex);
    sem_post(&semBufferFull);  //Used to decrement the number of occupied slots at the buffer

    free(params);
    pthread_exit(NULL);
}   

void* routineConsumer(void* arg) {
    
    Message message;
    char privateFifo[MAX_BUF];
    int privateID;
    while(1) {  //Checks the buffer continually for results to the requests until the server times out or all the requests have been answered
        if(timeOut && !running) break;

        if (sem_trywait(&semBufferFull) != 0) continue;

        pthread_mutex_lock(&bufferMutex); 

        message = buffer[consumerIndex];  
        consumerIndex++;
        consumerIndex %= bufsz;

        pthread_mutex_unlock(&bufferMutex);
        sem_post(&semBufferEmpty);
        
        snprintf(privateFifo, MAX_BUF, "/tmp/%d.%lu", message.pid, message.tid);// Names a FIFO based on the process id and thread id

        parseMessage(&message);

        if ((privateID = open(privateFifo, O_WRONLY)) < 0){ 
            registOperation(&message, "FAILD");    
        }else{

            if (write(privateID, &message, sizeof(Message)) < 0) { 
                registOperation(&message, "FAILD");
            }
            else{

                if(message.tskres == TSKTIMEOUT) registOperation(&message, "2LATE");
                else registOperation(&message, "TSKDN");

            }        
            close(privateID);
        }

        pthread_mutex_lock(&runningMutex);
        running--;
        pthread_mutex_unlock(&runningMutex);
        
    }
    
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


    buffer = malloc(sizeof(Message) * bufferSize);


    if (pthread_create(&consumerThread, NULL, routineConsumer, NULL) != 0) {  
        free(buffer);
        perror("Error creating new thread");
        return 1;
    }

    while (time(&nowT) - initT < t) { 
        if(time(&nowT) - initT >= t)  timeOut = 1;

        if ((ret = read(publicFD, &message, sizeof(Message))) == sizeof(Message)){
            nthreads++;
            Message* args = malloc(sizeof(*args));

            *args = message;

            registOperation(&message, "RECVD");

            pthread_mutex_lock(&runningMutex);
            running++;
            pthread_mutex_unlock(&runningMutex);

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


    timeOut = 1;
    
    remove(publicFIFO);

    current = start; // Starting at the first thread

    while (current != NULL) { // Until the thread pointed to by the last thread is null
        if (pthread_join(current->thread, NULL) != 0) { // Joins ProducerThreads
            free(buffer);
            freeLinkedList(&start);
            perror("Error joining threads");
            return 1;
        }
        current = current->next; // Jumps to next thread
    }
    
    closeConsumer = 1;

    if (pthread_join(consumerThread, NULL) != 0) { //Joins Consumer thread
        free(buffer);
        freeLinkedList(&start);
        perror("Error joining threads");
        return 1;
    }

    free(buffer);
    freeLinkedList(&start);
    return 0;
}


int main(int argc, char* argv[]) {
    char* publicFIFO;
    int nsecs, publicFD;

    srand(time(NULL));

    pthread_mutex_init(&bufferMutex, NULL);
    pthread_mutex_init(&timeOutMutex, NULL);
    pthread_mutex_init(&runningMutex, NULL);
    //pthread_mutex_init(&closeConsumerMutex, NULL);
    
    producerIndex = 0;
    consumerIndex = 0;

    timeOut = 0;
    closeConsumer = 0;

    running = 0;
    
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
    pthread_mutex_destroy(&runningMutex);

    return 0;
}
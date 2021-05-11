#include "server.h"

void* routineProducer(void* arg) {
    
    //taskres
    //enviar para buffer

}   

void* routineConsumer(void* arg) {
    
    //lê o buffer
    //envia para respetivo private fifo
        

}

void requestReceiver(int t, int publicFD, int bufferSize){
    int nthreads = 0;
    int ret;
    Message message;
    pthread_t thread, consumerThread;

    pthreadLinked* start = NULL;    // First pthread at linked list. Is used as a pseudo constant variable, since the first element rarely changes
    pthreadLinked* current = NULL;  // Current pthread at linked list

    time_t initT = time(NULL);      // Stores the while's first iteration's exact time 
    time_t nowT;                    // Will store each while iteration's exact time

    if (pthread_create(&consumerThread, NULL, routineConsumer, "nao sei o q por aqui :>") != 0) {  
            perror("Error creating new thread");
            return 1;
    }
    
    while (time(&nowT) - initT < t) {
        
        if ((ret = read(publicFD, &message, sizeof(message))) == sizeof(message)){
            nthreads++;
            Message* args = malloc(sizeof(*args));

            *args = message;

            if (pthread_create(&thread, NULL, routineProducer, args) != 0) {  
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

    if (pthread_join(&consumerThread, NULL) != 0) { //Joins Consumer thread
            perror("Error joining threads");
            return 1;
    }

    while (current != NULL) { // Until the thread pointed to by the last thread is null
        if (pthread_join(current->thread, NULL) != 0) { // Join thread
            perror("Error joining threads");
            return 1;
        }
        current = current->next; // Jumps to next thread
    }

    close(publicFD); // Ceases communication between server and client
    freeLinkedList(&start); // Frees all memory allocated by the thread linked list
  
}


int main(int argc, char *argv[]) {

    char* publicFIFO;
    int nsecs, bufsz, publicFD;

    switch(argc){
        case UNDEF_BUFSZ:
           bufsz = DEFAULT_BUFSZ;   
           publicFIFO = argv[3];

        case DEF_BUFSZ:
            bufsz = (atoi(argv[4]) > 0) ? atoi(argv[4]) : DEFAULT_BUFSZ;
            publicFIFO = argv[5];

        default: 
            perror("Error at number of arguments");
            return 1;
    }

    nsecs = atoi(argv[2]);

    if (mkfifo(publicFIFO, PERM) == -1) { // Creates FIFO for communication between this thread and the server 
        perror("Error creating private FIFO");
        return 1;
    }

    if ((publicFD = open(publicFIFO, O_RDONLY | O_NONBLOCK, 0666)) == -1){
        perror("Error opening public file descriptor");
        return 1;
    }
    
    return 0;
}
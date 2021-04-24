#include "client.h"

int main(int argc, char *argv[]) {
    
    if (argc <  4) {
        perror("Number of arguments invalid\n");
        return 1;
    }

    if (clientTaskManager(argv[2], atoi(argv[1])) == 1){
        return 1;
    }

    return 0;
}


void * routine(void *a){
    
}


int clientTaskManager(char* fifoname, int t){
    
    // if (mkfifo(fifoname,/*???*/) == -1){
	// 	perror("Error creating fifo file\n");
	// 	return 1;
	// }  
    
    int fd = open(fifoname, O_WRONLY); //Write only
    
    if (fd == -1) {
        perror("Error opening fifo");
        return 1;
    }

    int nthreads = -1;
    pthread_t* threads;

    time_t initT = time(NULL);
    time_t nowT;

    while (time(&nowT) - initT < t) {
        nthreads++;

        threads = (pthread_t*) realloc(threads, sizeof(pthread_t) * nthreads);
        
        if (pthread_create(&threads[nthreads], NULL, routine, ) != 0) {
            perror("Error creating new thread");
            break;
        }

        //nsleep();
    }

    for (int i = 0; i < nthreads + 1; i++) {
        void *res;
        pthread_join(threads[i], &res);	// Note: threads give no termination code
        printf("%i\n", *((int*)res));
        printf("\nTermination of thread %d: %lu.\n", i, (unsigned long)threads[i]);
    }

    free(threads);

    return 0;
}







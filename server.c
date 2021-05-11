#include "server.h"

int main(int argc, char *argv[]) {

    
    char * publicFIFO;
    int nsecs, bufsz;

    switch(argc){
        case UNDEFINED_BUF:
           bufsz = DEFAULT_BUFSZ;
           publicFIFO = argv[3];

        case DEFINED_BUF:
            bufsz = atoi(argv[4]);
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
    return 0;
}
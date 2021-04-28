#include "client_aux.h"

void registOperation(Message message, char* oper) {
    printf("%ld ; %d ; %d ; %d ; %ld ; %d ; %s\n",
            time(NULL), message.rid, message.tskload, message.pid, message.tid, message.tskres, oper);
}


void cleanup (char* privateFifo,routineArgs* params){
    remove(privateFifo);
    free(privateFifo);
    free(params);
}

void parseMessage(Message* message, int requestID ) {
    message->rid = requestID;
    message->pid = getpid();
    message->tskload = rand() % 10;
    message->tid = pthread_self();
    message->tskres = -1;
}
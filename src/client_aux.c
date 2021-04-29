#include "../include/client_aux.h"

void registOperation(Message message, const char* oper) {
    printf("%ld ; %d ; %d ; %d ; %ld ; %d ; %s\n",
            time(NULL), message.rid, message.tskload, message.pid, message.tid, message.tskres, oper);
}


void cleanup (char* privateFifo,routineArgs* params){
    if (privateFifo != NULL) remove(privateFifo);
    free(privateFifo);
    free(params);
}

void parseMessage(Message* message, int requestID, unsigned int seed) {
    message->rid = requestID;
    message->pid = getpid();
    message->tskload = rand_r(&seed) % 9 + 1;
    message->tid = pthread_self();
    message->tskres = -1;
}
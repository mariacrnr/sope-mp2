#include "server_aux.h"

void registOperation(Message message, const char* oper) {
    printf("%ld ; %d ; %d ; %d ; %ld ; %d ; %s\n",
            time(NULL), message.rid, message.tskload, message.pid, (unsigned long) message.tid, message.tskres, oper);
}

void parseMessage(Message* message, int requestID, unsigned int seed) {
    message->rid = requestID;
    message->pid = getpid();
    message->tskload = rand_r(&seed) % 9 + 1; // Random number between 1 and 9
    message->tid = pthread_self();
    message->tskres = -1;
}

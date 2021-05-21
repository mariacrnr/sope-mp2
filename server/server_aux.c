#include "../server/server_aux.h"

void registOperation(Message message, const char* oper) {
    printf("%ld ; %d ; %d ; %d ; %ld ; %d ; %s\n",
            time(NULL), message.rid, message.tskload, message.pid, message.tid, message.tskres, oper);
}

void parseMessage(Message* message) {
    message->pid = getpid();
    message->tid = pthread_self();
}

void pipeHandler(int signo){}

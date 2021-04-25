#ifndef _CLIENT_H
#define _CLIENT_H 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <sys/syscall.h>

#define gettid() syscall(SYS_gettid)

#include "macros.h"
#include "common.h"
#include "delay.h"

typedef struct {
    int requestId;
    int fifoID;
    int privateFifoID;
    char * privateFifoName;
} routineArgs;

int cancel = 0;
int ThreadsFinished = 0;

pthread_mutex_t clientMutex, threadCounterMutex, threadCancelMutex;

int main(int argc, char *argv[]);

int clientTaskManager(char* fifoname, int time);

void registOperation(Message message, char* oper);

#endif // _CLIENT_H
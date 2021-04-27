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

#include "macros.h"
#include "common.h"
#include "delay.h"

typedef struct {
    int requestId;
    int fifoID;
} routineArgs;

int cancel = 0;
int timedOut = 0;

pthread_mutex_t clientMutex, threadCancelMutex, TimedOutMutex;


int main(int argc, char* argv[]);

int clientTaskManager(int publicFifoID, int time);

void registOperation(Message message, char* oper);

#endif // _CLIENT_H    
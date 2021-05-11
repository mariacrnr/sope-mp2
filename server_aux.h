#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>

#include "common.h"

void registOperation(Message message, const char* oper);

void parseMessage(Message* message, int requestID, unsigned int seed);
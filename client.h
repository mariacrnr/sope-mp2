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

#include "common.h"


int main(int argc, char *argv[]);

int clientTaskManager(char* fifoname, int time);
#include "macros.h"
#include "server_aux.h"
#include "linkedList.h"

sem_t semBufferEmpty;
sem_t semBufferFull;
Message* buffer;
int counter;

pthread_mutex_t bufferMutex;

void* routineProducer(void* arg);

void* routineConsumer(void* arg);

int requestReceiver(int t, int publicFD, int bufferSize);


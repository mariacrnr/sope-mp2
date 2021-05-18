#include "macros.h"
#include "server_aux.h"
#include "linkedList.h"

sem_t semBufferEmpty;
sem_t semBufferFull;
Message* buffer;

int producerIndex;
int consumerIndex;

int bufsz;
int timeOut;
int closeConsumer;
int clientTimeOut;

pthread_mutex_t bufferMutex;
pthread_mutex_t timeOutMutex;
pthread_mutex_t closeConsumerMutex;

void* routineProducer(void* arg);

void* routineConsumer(void* arg);

int requestReceiver(int t, int publicFD, char* publicFIFO, int bufferSize);

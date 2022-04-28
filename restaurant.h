#include <stdlib.h>
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#include <pthread.h>
#include <csignal>

using namespace std;

#define forLoop(x) for (int i = 0; i < x; i++)

#define MAX_BUFFER_SIZE 10

//Prototypes for both source files
void signalHandler(int signum);
int countDish(int isVegan);

//Prototypes for restuarantThreads.cpp
void *donatelloProducer(void * arg);
void *portecelliProducer(void * arg);
void *veganConsumer(void * arg);
void *nonVeganConsumer(void * arg);
void *hybridConsumer(void * arg);
void *counter(void * arg);



/*
Purpose: Make use of interprocess communication techniques through threads. Experiment with the bounded buffer problem.
Known bugs: Very rarely, the counter can be off by 1 due to a consumer executing the same time that the counter prints. (Tried fixing through the use of semaphores but it creates an issue where the consumers and produce have to wait for the 
            counter to print and therefore the trays never converge to 10)
Limitations: Due to the specs wanting to reduce the amount of shared memory, there was a limited amount of use of global variables. Did not create an array of strings to hold the dish names. Because of this, all the print statements were 
             hardcoded.
Usage: To make and run code
            Case 1
            make all or make restaurantThread
            ./restaurantThread

            Case 2
            make runThreads
            
       To clean files
            make clean
*/

#include <restaurant.h>

int* vegan;
int* nonVegan;

sem_t vMutex;
sem_t vFull; 
sem_t vEmpty; 

sem_t nMutex;
sem_t nFull;
sem_t nEmpty;

int veganIn = 0;
int veganOut = 0;
int nonVeganIn = 0;
int nonVeganOut = 0;

//Declared as a global variable because we need to be able to call pthread_cancel() with each thread outside of main and in the signal handler (allows ctrl-c to send a cancellation request to the thread)
pthread_t donatelloProducer_id, portecelliProducer_id, veganConsumer_id, 
          nonVeganConsumer_id, hybridConsumer_id, counter_id;

int main()
{
    signal(SIGINT, signalHandler);

    printf("\n====================================================================\nRunning with threads\n====================================================================\n\n");

    vegan = (int*)malloc(MAX_BUFFER_SIZE*sizeof(int));
    nonVegan = (int*)malloc(MAX_BUFFER_SIZE*sizeof(int));

    sem_init(&vMutex, 1, 1);
    sem_init(&vEmpty, 1, MAX_BUFFER_SIZE);
    sem_init(&vFull, 1, 0);

    sem_init(&nMutex, 1, 1);
    sem_init(&nEmpty, 1, MAX_BUFFER_SIZE);
    sem_init(&nFull, 1, 0);

    forLoop(MAX_BUFFER_SIZE)
    {
        vegan[i] = 0;
        nonVegan[i] = 0;
    }

    int result = 0;
    
    //Create the producer and consumer threads as well as a thread to represent the counter
    result = pthread_create(&donatelloProducer_id, NULL, donatelloProducer, NULL);
    if (result != 0)
    {
        printf("Error creating donatelloProducer thread\n");
    }

    result = pthread_create(&portecelliProducer_id, NULL, portecelliProducer, NULL);
    if (result != 0)
    {
        printf("Error creating portecelliProducer thread\n");
    }

    result = pthread_create(&veganConsumer_id, NULL, veganConsumer, NULL);
    if (result != 0)
    {
        printf("Error creating veganConsumer thread\n");
    }

    result = pthread_create(&nonVeganConsumer_id, NULL, nonVeganConsumer, NULL);
    if (result != 0)
    {
        printf("Error creating nonVeganConsumer thread\n");
    }

    result = pthread_create(&hybridConsumer_id, NULL, hybridConsumer, NULL);
    if (result != 0)
    {
        printf("Error creating hybridConsumer thread\n");
    }

    result = pthread_create(&counter_id, NULL, counter, NULL);
    if (result != 0)
    {
        printf("Error creating counter thread\n");
    }

    //Joins and waits for each thread to finish (each thread is in an infinite loop so it will never finish unless ctrl-c signal is captured)
    pthread_join(donatelloProducer_id, NULL);
    pthread_join(portecelliProducer_id, NULL);
    pthread_join(veganConsumer_id, NULL);
    pthread_join(nonVeganConsumer_id, NULL);
    pthread_join(hybridConsumer_id, NULL);
    pthread_join(counter_id, NULL);


    free(vegan);
    free(nonVegan);
    pthread_exit(NULL);
    return 0;
}

/*
Function: donatelloProducer
Purpose: Donatello producer function
Method: While loop that prints and produces a dish in the tray for a non vegan consumer. The function will wait for the non vegan buffers to be empty
        and then produces the next dish at the in pointer for non vegan consumers.
Input: void pointer
Output: N/A
*/
void *donatelloProducer(void * arg)
{
    while(1)
    {
        sem_wait(&nEmpty);
        sem_wait(&nMutex);

        int dish = 1 + (rand() % 2);
        nonVegan[nonVeganIn] = dish;
        nonVeganIn = (nonVeganIn + 1) % MAX_BUFFER_SIZE;

        if(dish == 1)
        {
            printf("Donatello produced non vegan dish: Fettuccine Chicken Alfredo\n");
        }
        else if(dish == 2)
        {
            printf("Donatello produced non vegan dish: Garlic Sirloin Steak\n");
        }
        else
        {
            printf("Donatello had an error producing a dish\n");  
        }

        sem_post(&nFull);
        sem_post(&nMutex);

        srand(time(NULL) ^ (pthread_self()));
        sleep(rand() % 5 + 1);
    }
    pthread_exit(NULL);
}

/*
Function: portecelliProducer
Purpose: Portecelli producer function
Method: While loop that prints and produces a dish in the tray for a vegan consumer. The function will wait for the vegan buffers to be empty
        and then produces the next dish at the in pointer for vegan consumers.
Input: void pointer
Output: N/A
*/
void *portecelliProducer(void * arg)
{
    while(1)
    {
        sem_wait(&vEmpty);
        sem_wait(&vMutex);

        int dish = 1 + (rand() % 2);
        vegan[veganIn] = dish;
        veganIn = (veganIn + 1) % MAX_BUFFER_SIZE;

        if(dish == 1)
        {
            printf("Portecelli produced vegan dish: Pistachio Pesto Pasta\n");
        }
        else if(dish == 2)
        {
            printf("Portecelli produced vegan dish: Avocado Fruit Salad\n");
        }
        else
        {
            printf("Portecelli had an error producing a dish\n");  
        }

        sem_post(&vFull);
        sem_post(&vMutex);
        
        srand(time(NULL) ^ (pthread_self()));
        sleep(rand() % 5 + 1);
    }
    pthread_exit(NULL);
}

/*
Function: veganConsumer
Purpose: Vegan consumer function
Method: While loop that prints and consumes a dish in the tray from the vegan producer. The function will wait for the vegan buffers to be full
        and then consumes the next dish at the out pointer for vegan.
Input: void pointer
Output: N/A
*/
void *veganConsumer(void * arg)
{
    while(1)
    {
        sem_wait(&vFull);
        sem_wait(&vMutex);
        
        int dish = vegan[veganOut];
        vegan[veganOut] = 0;
        veganOut = (veganOut + 1) % MAX_BUFFER_SIZE;

        if(dish == 1)
        {
            printf("Vegan consumer took vegan dish: Pistachio Pesto Pasta\n");
        }
        else if(dish == 2)
        {
            printf("Vegan consumer took vegan dish: Avocado Fruit Salad\n");
        }
        else
        {
            printf("Error taking vegan dish\n");  
        }
        

        sem_post(&vMutex);
        sem_post(&vEmpty);

        srand(time(NULL) ^ (pthread_self()));
        sleep(rand() % 5 + 10);
    }
    pthread_exit(NULL);
}

/*
Function: nonVeganConsumer
Purpose: Non vegan consumer function
Method: While loop that prints and consumes a dish in the tray from the non vegan producer. The function will wait for the non vegan buffers to be full
        and then consumes the next dish at the out pointer for non vegan.
Input: void pointer
Output: N/A
*/
void *nonVeganConsumer(void * arg)
{
    while(1)
    {
        sem_wait(&nFull);
        sem_wait(&nMutex);
        
        int dish = nonVegan[nonVeganOut];
        nonVegan[nonVeganOut] = 0;
        nonVeganOut = (nonVeganOut + 1) % MAX_BUFFER_SIZE;

        if(dish == 1)
        {
            printf("Non vegan consumer took non vegan dish: Fettuccine Chicken Alfredo\n");
        }
        else if(dish == 2)
        {
            printf("Non vegan consumer took non vegan dish: Garlic Sirloin Steak\n");
        }
        else
        {
            printf("Error taking non vegan dish\n");  
        }

        sem_post(&nMutex);
        sem_post(&nEmpty);

        srand(time(NULL) ^ (pthread_self()));
        sleep(rand() % 5 + 10);
    }
    pthread_exit(NULL);
}

/*
Function: hybridConsumer
Purpose: Hybrid consumer function
Method: While loop that prints and consumes a dish in each tray from both producers. The function will wait for the vegan and non vegan buffers to be full
        and then consumes the next dish the out pointer is at for both vegan and non vegan.
Input: void pointer
Output: N/A
*/
void *hybridConsumer(void * arg)
{
    while(1)
    {
        sem_wait(&nFull);
        sem_wait(&nMutex);
        sem_wait(&vFull);
        sem_wait(&vMutex);
        
        int nonVeganDish = nonVegan[nonVeganOut];
        nonVegan[nonVeganOut] = 0;
        nonVeganOut = (nonVeganOut + 1) % MAX_BUFFER_SIZE;

        int veganDish = vegan[veganOut];
        vegan[veganOut] = 0;
        veganOut = (veganOut + 1) % MAX_BUFFER_SIZE;

        if(nonVeganDish == 1)
        {
            if(veganDish == 1)
            {
                printf("Hybrid consumer took non vegan dish: Fettuccine Chicken Alfredo, and vegan dish Pistachio Pesto Pasta\n");
            }
            else
            {
                printf("Hybrid consumer took non vegan dish: Fettuccine Chicken Alfredo, and vegan dish Avocado Fruit Salad\n");
            }
        }
        else if(nonVeganDish == 2)
        {
            if(veganDish == 1)
            {
                printf("Hybrid consumer took non vegan dish: Garlic Sirloin Steak, and vegan dish Pistachio Pesto Pasta\n");
            }
            else
            {
                printf("Hybrid consumer took non vegan dish: Garlic Sirloin Steak, and vegan dish Avocado Fruit Salad\n");
            }
        }
        else
        {
            printf("Hybrid consumer had an error taking the dishes\n");  
        }
        
        sem_post(&nMutex);
        sem_post(&nEmpty);
        sem_post(&vMutex);
        sem_post(&vEmpty);

        srand(time(NULL) ^ (pthread_self()));
        sleep(rand() % 5 + 10);
    }
    pthread_exit(NULL);
}

/*
Function: counter
Purpose: Prints the trays
Method: While loop that prints the number of items in the two trays every ten seconds 
Input: void pointer
Output: N/A
*/
void *counter(void * arg)
{
    while(1)
    {
        sleep(10);
        printf("\nItems in non vegan tray: %i/10, Items in vegan tray: %i/10\n\n", countDish(0), countDish(1));
    }
    pthread_exit(NULL);
}

/*
Function: countDish
Purpose: Counts the trays
Method: Checks to see which tray needs to be counted and loops until the max buffer size is reached
Input: Int representing if it is counting the vegan or non vegan tray
Output: Int representing the count
*/
int countDish(int isVegan)
{
    int count = 0;
    forLoop(MAX_BUFFER_SIZE)
    {
        if(isVegan == 1)
        {
            if(vegan[i] != 0)
            {
                count++;
            }
        }
        else
        {
            if(nonVegan[i] != 0)
            {
                count++;
            }
        }
    }
    return count;
}

/*
Function: signalHandler
Purpose: Handles signals
Method: Prints a message signifying a signal has been caught. Will also release the memory from the buffers and close the threads
Input: int representing the signal
Output: N/A
*/
void signalHandler(int signum) 
{
    //Sends a cancellation request to the threadS
    pthread_cancel(donatelloProducer_id);
    pthread_cancel(portecelliProducer_id);
    pthread_cancel(veganConsumer_id);
    pthread_cancel(nonVeganConsumer_id);
    pthread_cancel(hybridConsumer_id);
    pthread_cancel(counter_id);

    //Frees the buffer
    free(vegan);
    free(nonVegan);
    
    printf("\n\nCTRL C recieved\nThreads closed and memory from buffer released\n\n");
 
    exit(signum);  
}
/*
Purpose: Make use of interprocess communication techniques through processes. Experiment with the bounded buffer problem.
Method: The program will fork five child processes in which 2 are producers and 3 are consumers. Each producer will wait until the buffer is empty in order to produce a "dish" denoted 1 or 2 for the consumer. Each consumer will wait for the
        buffers to be full in order to consumer a "dish" denoted 1 or 2. Each of the buffers utilizes and in and out pointer to create a first in first out scenario where producers will not produce if the buffer is full and the consumer will
        not consumer if the buffer is empty. The producers will wait 1-5 seconds before producing the next dish and the consumers will wait 10-15 seconds before consuming the next dish. 
Known bugs: Very rarely, the counter can be off by 1 due to a consumer executing the same time that the counter prints. (Tried fixing through the use of semaphores but it creates an issue where the consumers and produce have to wait for the 
            counter to print and therefore the trays never converge to 10)
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

int main()
{
    printf("\n====================================================================\nRunning with processes\n====================================================================\n\n");

    srand(time(NULL));

    vegan = (int*)mmap(NULL, sizeof(int*)*MAX_BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    nonVegan = (int*)mmap(NULL, sizeof(int*)*MAX_BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    //Create shared semaphores
    sem_t *vMutex = (sem_t*)mmap(NULL, sizeof(sem_t*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_t *vFull = (sem_t*)mmap(NULL, sizeof(sem_t*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_t *vEmpty = (sem_t*)mmap(NULL, sizeof(sem_t*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    sem_init(vMutex, 1, 1);
    sem_init(vEmpty, 1, MAX_BUFFER_SIZE);
    sem_init(vFull, 1, 0);

    //Create shared semaphores
    sem_t *nMutex = (sem_t*)mmap(NULL, sizeof(sem_t*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_t *nFull = (sem_t*)mmap(NULL, sizeof(sem_t*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_t *nEmpty = (sem_t*)mmap(NULL, sizeof(sem_t*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    sem_init(nMutex, 1, 1);
    sem_init(nEmpty, 1, MAX_BUFFER_SIZE);
    sem_init(nFull, 1, 0);

    //Create shared in/out pointers
    int* veganIn = (int*)mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int* veganOut = (int*)mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int* nonVeganIn = (int*)mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int* nonVeganOut = (int*)mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    *veganIn = 0;
    *veganOut = 0;
    *nonVeganIn = 0;
    *nonVeganOut = 0;

    forLoop(MAX_BUFFER_SIZE)
    {
        vegan[i] = 0;
        nonVegan[i] = 0;
    }

    pid_t donatelloProducer;
    pid_t portecelliProducer;
    pid_t veganConsumer;
    pid_t nonVeganConsumer;
    pid_t hybridConsumer;

    //vegan producer 
    portecelliProducer = fork();    
    if(portecelliProducer == 0)
    {
        while(1)
        {
            sem_wait(vEmpty);
            sem_wait(vMutex);

            int dish = 1 + (rand() % 2);
            vegan[*veganIn] = dish;
            *veganIn = (*veganIn + 1) % MAX_BUFFER_SIZE;

            if(0 < dish < 3)
            {
                printf("Portecelli produced vegan dish: Pistachio Pesto Pasta\n");
            }
            else
            {
                printf("Portecelli had an error producing a dish\n");  
            }

            sem_post(vFull);
            sem_post(vMutex);
            
            srand(time(NULL) ^ (getpid()));
            sleep(rand() % 5 + 1);
        }
    }

    //non vegan producer
    donatelloProducer = fork();
    if(donatelloProducer == 0)
    {
        while(1)
        {
            sem_wait(nEmpty);
            sem_wait(nMutex);

            int dish = 1 + (rand() % 2);
            nonVegan[*nonVeganIn] = dish;
            *nonVeganIn = (*nonVeganIn + 1) % MAX_BUFFER_SIZE;

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

            sem_post(nFull);
            sem_post(nMutex);

            srand(time(NULL) ^ (getpid()));
            sleep(rand() % 5 + 1);
        }
    }

    //vegan consumer
    veganConsumer = fork();
    if(veganConsumer == 0)
    {
        while(1)
        {
            sem_wait(vFull);
            sem_wait(vMutex);
            
            int dish = vegan[*veganOut];
            vegan[*veganOut] = 0;
            *veganOut = (*veganOut + 1) % MAX_BUFFER_SIZE;

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

            sem_post(vMutex);
            sem_post(vEmpty);

            srand(time(NULL) ^ (getpid()));
            sleep(rand() % 5 + 10);
        }
    }

    //non vegan consumer
    nonVeganConsumer = fork();
    if(nonVeganConsumer == 0)
    {
        while(1)
        {
            sem_wait(nFull);
            sem_wait(nMutex);
            
            int dish = nonVegan[*nonVeganOut];
            nonVegan[*nonVeganOut] = 0;
            *nonVeganOut = (*nonVeganOut + 1) % MAX_BUFFER_SIZE;

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

            sem_post(nMutex);
            sem_post(nEmpty);

            srand(time(NULL) ^ (getpid()));
            sleep(rand() % 5 + 10);
        } 
    }

    //hybrid consumer
    hybridConsumer = fork();
    if(hybridConsumer == 0)
    {
        while(1)
        {
            sem_wait(nFull);
            sem_wait(nMutex);
            sem_wait(vFull);
            sem_wait(vMutex);
            
            int nonVeganDish = nonVegan[*nonVeganOut];
            nonVegan[*nonVeganOut] = 0;
            *nonVeganOut = (*nonVeganOut + 1) % MAX_BUFFER_SIZE;

            int veganDish = vegan[*veganOut];
            vegan[*veganOut] = 0;
            *veganOut = (*veganOut + 1) % MAX_BUFFER_SIZE;

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
                
            sem_post(nMutex);
            sem_post(nEmpty);
            sem_post(vMutex);
            sem_post(vEmpty);

            srand(time(NULL) ^ (getpid()));
            sleep(rand() % 5 + 10);
        }
    }
    else
    {
        //Never needs to wait for child to finish before parent executes because the child forks are in an infinite loop
        signal(SIGINT, signalHandler);
        while(1)
        {
            sleep(10);
            printf("\nItems in non vegan tray: %i/10, Items in vegan tray: %i/10\n\n", countDish(0), countDish(1));
        }
    }
    return 0;
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
Method: Prints a message signifying a signal has been caught
Input: int representing the signal
Output: N/A
*/
void signalHandler(int signum) 
{
   printf("\nCTRL C recieved\n");

   exit(signum);  
}
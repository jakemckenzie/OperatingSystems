 /* 
  * Jake McKenzie
  * Operating Systems
  * Project 2 Produce-Consumer problem in linux
  * 
  * EXAMPLE INSTRUCTIONS: 
  * 
  * gcc ./project2.c -pthread
  * 
  * ./a.out 5 2 3
  * 
  * In problem we will solve the bounded-buffer problem
  * with a synchronization solution. The producer produces
  * items for the bufffer and the consumer takes items from
  * the buffer consuming them. 
  */

#include "buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

// semphore full state
sem_t full; 
// semphore empty state
sem_t empty;
// bounded buffer
buffer_item buffer[BUFFER_SIZE];
// pthreaded mutex lock
pthread_mutex_t mutex;
// amount of produced items
volatile int produce; 
// amount of consumed items
volatile int consume; 
// total inventory
volatile int inventory; 
// the amount the user specifies to sleep
volatile int slp;
// the amount the user specifies to produce
volatile int prdc;
// the amount the user specifies to consume
volatile int cnsm;

void random_sleep(void);
void *producer(void *param); 
void *consumer(void *param);
int produce_item(buffer_item item);
int consume_item(buffer_item *item);

int main(int argc, char *argv[]) {
    if (argc != 4) {    
        fprintf(stderr,"ERROR: ./project2 Please provide a sleep time, producer number and consumer number greater than 1!\n");
        return -1;
    } else{
        if(atoi(argv[1]) < 2 || atoi(argv[2]) < 2 || atoi(argv[3]) < 2){
            fprintf(stderr,"ERROR: ./project2 Please provide a sleep time, producer number and consumer number greater than 1!\n");
            return -1;
        } else{
            // user defined sleep time
            slp = atoi(argv[1]);
            // user defined producer number
            prdc = atoi(argv[2]);
	        // user defined consumer number
            cnsm = atoi(argv[3]);
            
            srand(time(NULL));
            // c does not have a monitor so we accomplish this 
            // by locking by associating a condition variable with 
            // a mutex lock. This generates and initiallizes a 
            // conditional variable and the associated 
            // mutex lock
            pthread_mutex_init(&mutex, NULL);
            sem_init(&empty, 0, BUFFER_SIZE);
            sem_init(&full, 0, 0);
            
            /* initiallize counters to zero */
            inventory = 0;
            produce   = 0;
            consume   = 0;

            // produces the correct number of producer threads
            pthread_t prdc_t[prdc];
            int i = 0; 
            while ( i != prdc) {
                pthread_create(&prdc_t[i], NULL, producer, NULL);
                i++;
            }
            // produces the correct number of consumer threads.
            pthread_t cnsm_t[cnsm];
            int j = 0;
            
            while (j != cnsm) {
                pthread_create(&cnsm_t[j], NULL, consumer, NULL);
                j++;
            }
            // go to sleep
            sleep(slp);
            return 0;
        }
    } 
}
/*
 * This is where producers are spawned based on user definition
 */
void *producer(void *param) {  
    buffer_item item;
    while(1){
        // sleep for random amounts of time based on user definition
        random_sleep();
        item = rand(); 
        if(produce_item(item)){
            printf("Error...\n");
        } else{  
            printf("Producer id: %u produced %d items\n",(unsigned int)pthread_self(), prdc);
        }
    }

}
/*
 * This is where consumers are spawned based on user definition
 */
void *consumer(void *param) {    
    buffer_item item;
    while(1){
        // sleep for random amounts of time based on user definition
        random_sleep();
        if(consume_item(&item)){
            printf("Error...\n");
        } else{  
            printf("Consumer id: %u consumed %d items\n",(unsigned int)pthread_self(), cnsm);
        }    
    }
}
/*
 * The is where items in the buffer are produced
 */
int produce_item(buffer_item item) {

    int flag = 0;
    sem_wait(&empty);
    pthread_mutex_lock(&mutex);
    if(inventory != BUFFER_SIZE) {
        buffer[produce] = item;
        produce++;
        inventory++;
        flag = 0;
    } else {
        flag = -1;
    }
    
    pthread_mutex_unlock(&mutex);
    sem_post(&full);

    return flag;
}
/*
 * This is where items in the buffer are consumed
 */
int consume_item(buffer_item *item) {
    int flag;
    sem_wait(&full);
    pthread_mutex_lock(&mutex);
    if(inventory != 0) {
        *item = buffer[consume];
        consume++;
        inventory--;
        flag = 0;
    } else {
        flag = -1;
    }
    pthread_mutex_unlock(&mutex);
    sem_post(&empty);

    return flag;
}
/*
 * A random amount of sleep based on how much the user specifies
 */
void random_sleep(void) {
    sleep(rand() % slp + 1);
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <mQueue.h>

#define NUM_THREADS 4

typedef struct _counter_data_t
{
    int number;
    pthread_mutex_t numberLock;
}counter_data_t;

/* create thread struct for testing */
typedef struct _thread_data_t {
  int tid;
  int numbersToAdd;
  queue_t* queue;

  counter_data_t* counterData;
} thread_data_t;

void freeString(void *s)
{
    free(s);
}

int getNextValue(counter_data_t * counterData)
{
    int value;
    pthread_mutex_lock(&counterData->numberLock);
    value = counterData->number--;
    pthread_mutex_unlock(&counterData->numberLock);
    return value;
}

void *addTest(void *arg)
{
    thread_data_t *td = (thread_data_t*)arg;
    int i,p;
    char* s;

    printf("add TID %d created:\n", td->tid, s);
    for (i=0; i<td->numbersToAdd; i++)
    {
        p = rand()%100;//getNextValue(td->counterData);
        s = malloc(sizeof(char) * 3);
        sprintf(s, "%d", p);
        printf("TID %d: add %s\n", td->tid, s);
        addToQueue(s, p, td->queue, td->tid);
    }
    printf("add thread %d exiting\n",td->tid);
    pthread_exit(NULL);
}

void *popTest(void *arg)
{
    thread_data_t *td = (thread_data_t*)arg;
    int i;
    char* s;

    printf("pop TID %d created:\n", td->tid, s);
    for (i=0; i<4; i++)
    {
        if((s = pop(td->queue)))
        {
            printf("TID %d: popped %s\n", td->tid, s);
            free(s);
        }
        else
        {
                printf("No Item returned\n");
        }

    }

    pthread_exit(NULL);
}

int main()
{
    int i, p, j, rc, lastNum, currNum, sorted, count, duplications;
    char *s;
    counter_data_t *counterData;
    int addsPerThread = 300;


    srand(time(NULL));

    counterData = malloc(sizeof(counter_data_t));       //used to guarentee exactly one of each priority between threads
    counterData->number = NUM_THREADS * addsPerThread;
    pthread_mutex_init(&counterData->numberLock, NULL);

    printf("Queue Test:\n\n");
    queue_t *queue = createPriorityQueue(&freeString, 100000);

    pthread_t add_thr[NUM_THREADS];
    pthread_t pop_thr[NUM_THREADS];

    /* create a thread_data_t argument array */
    thread_data_t thr_add_data[NUM_THREADS];
    thread_data_t thr_pop_data[NUM_THREADS];

    /* create add threads */
    printf("creating add test threads\n");
    for (i = 0; i < NUM_THREADS; ++i) {

        //set data for threads
        thr_add_data[i].tid = i;
        thr_add_data[i].queue = queue;
        thr_add_data[i].numbersToAdd = addsPerThread;
        thr_add_data[i].counterData = counterData;

        //make threads
        if ((rc = pthread_create(&add_thr[i], NULL, addTest, &thr_add_data[i]))) {
            fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
            return EXIT_FAILURE;
        }
    }


    /* block until all threads complete */
    for (i = 0; i < NUM_THREADS; ++i) {
        pthread_join(add_thr[i], NULL);
    }


    printf("\nheap = ");
    printHeap(queue);

    printf("\nsorted = ");

    lastNum = 0;
    currNum = 0;
    count = 0;
    sorted = 0; //true if 0
    duplications = 0; //true if 0
    while ((s = pop(queue)))
    {
        currNum = atoi(s);

        if (currNum < lastNum)
        {
            printf(">");
            sorted++;
        }
        else if(currNum == lastNum)
        {
            //printf("=");
            duplications++;
        }
        count++;
        lastNum = currNum;
        printf("%s ", s);
        free(s);
    }
    printf("\nThis queue is ");
    if (sorted != 0 )//|| duplications != 0)
    {
        printf("not ");
    }
    printf("sorted.\n%d/%d values, %d sort errors\n", count, NUM_THREADS*addsPerThread, sorted);
    destroyQueue(queue);
    free(counterData);
}

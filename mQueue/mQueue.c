#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <mQueue.h>


inline void swap(queue_item_t * heap, int a, int b)
{
    temp_v temp;

    temp.integer = heap[a].priority;
    heap[a].priority = heap[b].priority;
    heap[b].priority = temp.integer;

    temp.voidPointer = heap[a].data;
    heap[a].data = heap[b].data;
    heap[b].data = temp.voidPointer;
}

void mathDelay( int value )
{
    int i,j;
    for(i = 0; i<value; i++)
    {
        for(j = 0; j < value; j++);
    }
}

//get higest priority item
//Due to threading, pop must wait for all adding functions to end to ensure that the lowest priority item will always be removed
//NOTE: It is probably true that if you didn't care so much about always having the highest priority then you could allow both add and pop operations to run concurrently for increased performance
void *pop(queue_t* queue)
{
    int child;
    int parent = 0;
    queue_item_t* heap;
    void *toRet = NULL;

    //check to see if adding is allowed yet,

    //while not in the proccess of RESIZING
    pthread_mutex_lock(&queue->resizeLock);
    while(queue->needsResizing)
    {
        printf("pop: waiting to for resizing to finish\n");
        pthread_cond_wait(&queue->resizeCond, &queue->resizeLock);
    }
    pthread_mutex_unlock(&queue->resizeLock);

    //add can only add when accessors is negative or 0
    pthread_mutex_lock(&queue->accessorLock);
    while(queue->accessors > 0)
    {
        printf("pop: waiting for adders to leave\n");
        pthread_cond_wait(&queue->accessorCond, &queue->accessorLock);
    }
    queue->accessors--;
    pthread_mutex_unlock(&queue->accessorLock);

    heap = queue->heap;
    //begin popping
    if(queue->itemCount > 0)
    {
        toRet = heap[0].data;

        heap[0].data = heap[--queue->itemCount].data;
        heap[0].priority = heap[queue->itemCount].priority;

        child = parent * 2;

        while(child + 1 < queue->itemCount)
        {
            if (child + 2 < queue->itemCount)
            {
                if(heap[child+1].priority > heap[child+2].priority)
                {
                    if (heap[parent].priority > heap[child+2].priority)
                    {
                        swap(heap, parent, child + 2);
                        parent = child + 2;
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    if (heap[parent].priority > heap[child+1].priority)
                    {
                        swap(heap, parent, child + 1);
                        parent = child + 1;
                    }
                    else
                    {
                        break;
                    }
                }
            }
            else //child + 1 < queue->itemCount
            {
                if (heap[parent].priority > heap[child+1].priority)
                {
                    swap(heap, parent, child + 1);
                    parent = child + 1;
                }
                else
                {
                    break;
                }
            }

            child = parent * 2;
        }
    }
    //remove itself from the accessor Count
    queue->accessors++;
    pthread_cond_signal(&queue->accessorCond);
    return toRet;
}

//TODO: replace mutexes with *mutexs to avoid using the &

//add to priority queue. Due to threading, it must wait for all popping functions to end to ensure that the lowest priority item will always be removed
//NOTE: It is probably true that if you didn't care so much about always having the highest priority then you could allow both add and pop operations to run concurrently for increased performance
queue_t *addToQueue(void* data, int priority, queue_t *queue, int tid)
{
    int child;
    int parent;
    int i;
    //pthread_mutex_t* parentLock;
    //pthread_mutex_t* childLock;
    queue_item_t* heap;

    //check if the item about to be added will fit
    pthread_mutex_lock(&queue->itemCountLock);

    //check for popper exclusion
    pthread_mutex_lock(&queue->accessorLock);
    while(queue->accessors < 0)
    {
        pthread_cond_wait(&queue->accessorCond, &queue->accessorLock);
    }
    queue->accessors++;
    pthread_mutex_unlock(&queue->accessorLock);


    //assign child location and give lock
    child = queue->itemCount++;
    //childLock = &queue->heap[child].itemLock;
    pthread_mutex_lock(&queue->heap[child].itemLock);                      //because adding and popping is mutually exclusive, locking the itemCount and giving the child it's lock before unlocking ensures no children get created before this child is locke


    if (queue->itemCount >= queue->heapSize)
    {
        //this process waits for accessors to = 0 (all other threads leave)
        pthread_mutex_lock(&queue->accessorLock);
        while(queue->accessors != 1)
        {
            pthread_cond_wait(&queue->accessorCond, &queue->accessorLock);
        }
        pthread_mutex_unlock(&queue->accessorLock);

        if((heap = realloc(queue->heap, sizeof(queue_item_t) * (queue->heapSize * 2 + 1))))        //could do this math faster
        {   //success
            queue->heap = heap;

            //initialize mutexes
            for (i = queue->heapSize, queue->heapSize = queue->heapSize * 2 + 1; i < queue->heapSize; i++)
            {
                queue->heap[i].itemLock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
            }

            pthread_cond_broadcast(&queue->resizeCond);
        }
        else
        {   //failure

            //remove itself from the accessor Count
            pthread_mutex_lock(&queue->accessorLock);
            queue->accessors--;
            pthread_mutex_unlock(&queue->accessorLock);

            pthread_cond_signal(&queue->accessorCond);
            pthread_mutex_unlock(&heap[child].itemLock);
            return NULL;
        }
    }
    else
    {
        heap = queue->heap;
    }
    pthread_mutex_unlock(&queue->itemCountLock);

    //create child
    heap[child].data = data;
    heap[child].priority = priority;

    while (child > 0)
    {
        parent = (child + child%2 - 2)/2;
        //parentLock = &heap[parent].itemLock;

        pthread_mutex_lock(&heap[parent].itemLock);

        if(heap[parent].priority > heap[child].priority)
        {
            swap(heap, parent, child);
            pthread_mutex_unlock(&heap[child].itemLock);
            child = parent;

        }
        else
        {
            pthread_mutex_unlock(&heap[parent].itemLock);
            break;
        }
    }
    pthread_mutex_unlock(&queue->heap[child].itemLock);

    //remove itself from the accessor Count
    pthread_mutex_lock(&queue->accessorLock);
    queue->accessors--;
    pthread_mutex_unlock(&queue->accessorLock);

    //done signal waiting threads. NOTE: MIGHT NEED TO BE BROADCASTED
    pthread_cond_signal(&queue->accessorCond);
    return queue;
}

//Takes the data freeing function and a suggested heap size
queue_t *createPriorityQueue(void (*destroyData)(void *data), unsigned int suggestedSize)
{
    queue_t *queue;
    int i;
    int shiftCount = 5;

    if (!(queue = malloc(sizeof(queue_t))))
    {
        return NULL;                        //failure
    }

    //assign destroy function
    queue->destroyData = destroyData;

    //threading
    pthread_mutex_init(&queue->itemCountLock, NULL);

    queue->accessors = 0;
    pthread_mutex_init(&queue->accessorLock, NULL);
    pthread_cond_init(&queue->accessorCond, NULL);

    queue->needsResizing = 0;
    pthread_mutex_init(&queue->resizeLock, NULL);
    pthread_cond_init(&queue->resizeCond, NULL);

    queue->itemCount = 0;

    //determine heapSize
    if (suggestedSize < 15)
    {
        queue->heapSize = 15;
    }
    else
    {
        suggestedSize = suggestedSize >> 4;

        while (suggestedSize > 1)
        {
            suggestedSize = suggestedSize >> 1;
            shiftCount++;
        }

        queue->heapSize = (suggestedSize << shiftCount) - 1;
    }

    //create heap block
    if (!(queue->heap = malloc(sizeof(queue_item_t) * queue->heapSize)))
    {
        return NULL;                       //could not build heap
    }

    //make mutexes -- might be able to put this in addToQueue
    for (i = 0; i < queue->heapSize; i++)
    {
        pthread_mutex_init(&(queue->heap[i].itemLock),NULL);
    }

    return queue;
}

//Destroys the queue
void destroyQueue(queue_t *queue)
{
    int i;

    for (i = 0; i < queue->itemCount; i++)
    {
        queue->destroyData(queue->heap[i].data);
    }

    free(queue->heap);
    free(queue);
}

void printHeap(queue_t *queue)
{
    int i;

    for (i = 0; i<queue->itemCount; i++)
    {
        printf("%d ", queue->heap[i].priority);
    }
    printf("\n");
}

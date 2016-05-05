#include <stdio.h>
#include <stdlib.h>

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

//get higest priority item
//Due to threading, pop must wait for all adding functions to end to ensure that the lowest priority item will always be removed
//NOTE: It is probably true that if you didn't care so much about always having the highest priority then you could allow both add and pop operations to run concurrently for increased performance
void *pop(queue_t* queue)
{
    int child;
    int parent = 0;
    queue_item_t* heap = queue->heap;
    void *toRet = NULL;

    //check to see if adding is allowed yet,
    //add can only add when accessors is negative or 0
    pthread_mutex_lock(queue->accessorLock);
    while(queue->accessors > 0)
    {
        pthread_cond_wait(queue->accessorCond, queue->accessorLock);
    }
    queue->accessors--;
    pthread_mutex_unlock(queue->accessorLock);

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
    return toRet;
}

//add to priority queue. Due to threading, it must wait for all popping functions to end to ensure that the lowest priority item will always be removed
//NOTE: It is probably true that if you didn't care so much about always having the highest priority then you could allow both add and pop operations to run concurrently for increased performance
queue_t *addToQueue(void* data, int priority, queue_t *queue)
{
    int child;
    int parent;
    queue_item_t* heap;

    //check to see if adding is allowed yet,
    //add can only add when accessors is positive or 0
    pthread_mutex_lock(queue->accessorLock);
    while(queue->accessors < 0)
    {
        pthread_cond_wait(queue->accessorCond, queue->accessorLock);
    }
    queue->accessors++;
    pthread_mutex_unlock(queue->accessorLock);


    //TODO: MAKE SURE RESIZING THE HEAP DOESN'T BREAK THE CODE (OH GOD IT'S SO DANGEROUS)
    if (queue->itemCount + 1 > queue->heapSize)
    {
        if((heap = realloc(queue->heap, sizeof(queue_item_t) * (queue->heapSize * 2 + 1))))        //could do this math faster
        {   //success
            queue->heap = heap;
            queue->heapSize = queue->heapSize * 2 + 1;
        }
        else
        {   //failure
            printf("No memory?\n");
            //remove itself from the accessor Count
            queue->accessors--;
            return NULL;
        }
    }
    else
    {
        heap = queue->heap;
    }

    child = queue->itemCount++;

    heap[child].data = data;
    heap[child].priority = priority;

    while (child > 0)
    {
        parent = (child + child%2 - 2)/2;

        if(heap[parent].priority > heap[child].priority)
        {
            swap(heap, parent, child);
            child = parent;
        }
        else
        {
            //remove itself from the accessor Count
            queue->accessors--;
            return queue;
        }
    }

    //remove itself from the accessor Count
    queue->accessors--;

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
    queue->accessors = 0;
    queue->accessorLock = &((pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER);
    queue->accessorCond = &((pthread_cond_t)PTHREAD_COND_INITIALIZER);

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
        queue->heap[i].heapLock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
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

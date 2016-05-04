#include <stdio.h>
#include <stdlib.h>

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
void *pop(queue_t* queue)
{
    int child;
    int parent = 0;
    queue_item_t* heap = queue->heap;
    void *toRet = NULL;

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

    return toRet;
}

//add to priority queue
queue_t *addToQueue(void* data, int priority, queue_t *queue)
{
    int child;
    int parent;
    queue_item_t* heap;// = queue->heap;

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
            return queue;
        }
    }
    return queue;
}

//Takes the data freeing function and a suggested heap size
queue_t *createPriorityQueue(void (*destroyData)(void *data), unsigned int suggestedSize)
{
    queue_t *queue;
    int shiftCount = 4;

    if (!(queue = malloc(sizeof(queue_t))))
    {
        return NULL;                        //failure
    }

    //assign destroy function
    queue->destroyData = destroyData;

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

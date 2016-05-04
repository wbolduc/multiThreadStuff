#ifndef _MQUEUE_H
#define _MQUEUE_H

#include <stdlib.h>
#include <stdio.h>

//obvious limitation is that the user needs to use integers for priorities
typedef struct _queue_item_t{
    void *data;
    int priority;
}queue_item_t;

typedef struct _queue_t{
    void (*destroyData)(void *data);    //If I moved this into queue item then you could store different types of data

    queue_item_t *heap;

    int itemCount;
    int heapSize;
}queue_t;

typedef union _temp_v{
    void* voidPointer;
    int   integer;
}temp_v;

//get higest priority item
void *pop(queue_t* queue);

//Utility for swapping items in an array of queue_item_t (heap)
inline void swap(queue_item_t * heap, int a, int b);

//add to priority queue, returns the same queue on success
queue_t *addToQueue(void* data, int priority, queue_t* queue);

//Takes the data freeing function and a suggested heap size, also floors suggestedSize to the smallest number of the form n^2 - 1 as that makes doubling the tree easier
queue_t *createPriorityQueue(void (*destroyData)(void *data), unsigned int suggestedSize);

//Destroys the queue
void destroyQueue(queue_t *queue);

//Testing function
void printHeap(queue_t *queue);

#endif

#ifndef _MQUEUE_H
#define _MQUEUE_H

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

//builds a lock on every array index, will try to make another version in the future that uses less locks, might be faster?

//A multithreadable version of my queue library


//obvious limitation is that the user needs to use integers for priorities
typedef struct _queue_item_t{
    void *data;
    pthread_mutex_t itemLock;
    int priority;
}queue_item_t;

typedef struct _queue_t{
    void (*destroyData)(void *data);    //If I moved this into queue item then you could store different types of data

    queue_item_t *heap;

    int itemCount;
    int heapSize;

    pthread_mutex_t itemCountLock;

    int needsResizing;
    pthread_mutex_t resizeLock;     //cant I store pointers instead of referencing them each time?
    pthread_cond_t resizeCond;

    int accessors;
    pthread_mutex_t accessorLock;
    pthread_cond_t accessorCond;

}queue_t;

typedef union _temp_v{
    void* voidPointer;
    int   integer;
}temp_v;

//get higest priority item
void *pop(queue_t* queue);

//delay utility
void mathDelay( int value );

//Utility for swapping items in an array of queue_item_t (heap)
inline void swap(queue_item_t * heap, int a, int b);

//add to priority queue, returns the same queue on success
queue_t *addToQueue(void* data, int priority, queue_t* queue, int tid);

//Takes the data freeing function and a suggested heap size, also floors suggestedSize to the smallest number of the form n^2 - 1 as that makes doubling the tree easier
queue_t *createPriorityQueue(void (*destroyData)(void *data), unsigned int suggestedSize);

//Destroys the queue
void destroyQueue(queue_t *queue);

//Testing function
void printHeap(queue_t *queue);

#endif

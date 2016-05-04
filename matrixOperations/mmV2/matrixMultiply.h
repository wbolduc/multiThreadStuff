#ifndef _MATRIX_MULTIPLY_H
#define _MATRIX_MULTIPLY_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>

typedef struct _matrix_2d_t {
    int **matrix;   //matrix

    int xSize;      //matrixdimensions
    int ySize;

    int x;          //position of the last item to be operated on
    int y;
    pthread_mutex_t lock;
} matrix_2d_t;

typedef struct _thread_data_t {
  int tid;
  matrix_2d_t *pos;
  double scalar;
} thread_data_t;

void delay( int secs );

void mathDelay( int value );

void printMatrix(int** matrix, int width, int length);

//returns the next item for a thread to operate on. If there are no longer any items return 0
int* getNextItem( matrix_2d_t * pos);

matrix_2d_t *makePos(int **matrix, int xSize, int ySize);

void *scalarWorker( void *arg );

//Boss class, creates all the threads and then waits for them all to join
int scalarMultiply2d(int** matrix, int width, int length, double scalar, int NUM_THREADS);

#endif /*_MATRIX_MULTIPLY_H*/

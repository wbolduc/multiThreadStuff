#ifndef MATRIX_H
#define MATRIX_H

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

//Clock based delay, not actually useful for testing threads because threads aren't really active
void delay( int secs );

//Performs arbitrary math, used to test threading
void mathDelay( int value );

//Prints the matrix
void printMatrix(int** matrix, int width, int length);

#endif /*MATRIX_H*/

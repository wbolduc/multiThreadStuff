#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>

#include "matrixMultiply.h"

int main(int argc, char **argv)
{
    int **matrix;
    int width = 30;
    int length = 30;
    int i, j;
    int NUM_THREADS = 4;


    if (argc >= 3)
    {
        width = atoi(argv[1]);
        length = atoi(argv[2]);
    }
    if (argc >= 4)
    {
        NUM_THREADS = atoi(argv[3]);
    }

    matrix = malloc(sizeof(int**) * length);
    for(i = 0; i < length; i++)
    {
        matrix[i] = malloc(sizeof(int*) * width);
        for(j = 0; j < width; j++)
        {
            matrix[i][j] = 0;//i*width + j;
        }
    }

    printMatrix(matrix, width, length);
    scalarMultiply2d(matrix, width, length, 3, NUM_THREADS);
    printMatrix(matrix, width, length);

    for(i = 0; i < length; i++)
    {
        free(matrix[i]);
    }
    free(matrix);
}

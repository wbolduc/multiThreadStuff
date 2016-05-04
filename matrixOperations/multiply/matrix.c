#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>

#include "matrix.h"

void delay( int secs )
{
    secs += time(NULL);
    while (time(NULL) < secs);
}

void mathDelay( int value )
{
    int i,j;
    for(i = 0; i<value; i++)
    {
        for(j = 0; j < value; j++);
    }
}

void printMatrix(int** matrix, int width, int length)
{
    int i, j;
    for(i = 0; i < length; i++)
    {
        for(j = 0; j < width; j++)
        {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
    printf("-------------------------------------------------------\n");
}

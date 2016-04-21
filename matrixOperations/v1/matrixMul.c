#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>



//matrix scalar mutiplication

typedef struct _matrixItem {
    int *item;
} matrixIndex;

typedef struct _thread_data_t {
  int tid;
  double scalar;
} thread_data_t;

int NUM_THREADS;

pthread_mutex_t     valLock  = PTHREAD_MUTEX_INITIALIZER;
int*                nextVal = NULL;

int                 valueReady = 0;
pthread_cond_t      checkSignal = PTHREAD_COND_INITIALIZER;
pthread_mutex_t     signalLock  = PTHREAD_MUTEX_INITIALIZER;

int                 valPickedUp = 0;

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

void *scalarMultiply( void *arg )
{
    thread_data_t *data = (thread_data_t *)arg;

    while(1)
    {
        pthread_mutex_lock(&signalLock);

        if (!valueReady)
        {
            pthread_cond_wait(&checkSignal, &signalLock);
        }
        valueReady = 0;     //Very dangerous


        while (pthread_mutex_trylock(&valLock))
        {
            if (!valueReady)
            {
                pthread_cond_wait(&checkSignal, &signalLock);
            }
            valueReady = 0;     //Very dangerous
        }

        pthread_mutex_unlock(&signalLock);
        valPickedUp = 1;

        *nextVal += data->tid;//data->scalar;
        pthread_mutex_unlock(&valLock);
        mathDelay(1050);
    }
}

int scalarMultiply2d(int** matrix, int width, int length, double scalar)
{
    int i, j, rc;

    //threadList
    pthread_t thr[NUM_THREADS];
    thread_data_t thr_data[NUM_THREADS];


    //claim nextVal before creating threads
    pthread_mutex_lock(&valLock);

    /* create threads */
    for (i = 0; i < NUM_THREADS; ++i)
    {
        thr_data[i].tid = 1 + i;
        thr_data[i].scalar = scalar;
        if ((rc = pthread_create(&thr[i], NULL, scalarMultiply, &thr_data[i])))
        {
            fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
            return EXIT_FAILURE;
        }
    }

    for(i = 0; i < length; i++)
    {
        for(j = 0; j < width; j++)
        {
            nextVal = &matrix[i][j];
            pthread_mutex_unlock(&valLock);

            valueReady = 1;
            pthread_cond_signal(&checkSignal);
            while(!valPickedUp);
            valPickedUp = 0;

            pthread_mutex_lock(&valLock);

            //printMatrix(matrix,width,length);
        }
    }

    for (i = 0; i < NUM_THREADS; i++)
    {
        pthread_cancel(thr[i]);
    }
}


int main(int argc, char **argv)
{
    int **matrix;
    int width = 30;
    int length = 30;
    int i, j;
    NUM_THREADS = 4;


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
    scalarMultiply2d(matrix, width, length, 3);
    printMatrix(matrix, width, length);

    for(i = 0; i < length; i++)
    {
        free(matrix[i]);
    }
    free(matrix);
}

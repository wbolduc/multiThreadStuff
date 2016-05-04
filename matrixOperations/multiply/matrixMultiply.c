#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include "matrix.c"

typedef struct _thread_data_t {
  int tid;
  matrix_2d_t *pos;
  double scalar;
} thread_data_t;


int NUM_THREADS;

//returns the next item for a thread to operate on. If there are no longer any items return 0
int* getNextItem( matrix_2d_t * pos)
{
    int *p = NULL;
    pthread_mutex_lock(&pos->lock);
    if(pos->y < pos->ySize)
    {
        if (pos->x >= pos->xSize)
        {
            pos->x = 0;
            pos->y++;
        }

        p = &pos->matrix[pos->y][pos->x];
        pos->x++;
    }
    printMatrix(pos->matrix, pos->xSize, pos->ySize);
    pthread_mutex_unlock(&pos->lock);

    return p;
}

matrix_2d_t *makeMatrix2d(int **matrix, int xSize, int ySize)
{
    matrix_2d_t *newPos = malloc(sizeof(matrix_2d_t));
    newPos->matrix = matrix;
    newPos->xSize = xSize;
    newPos->ySize = ySize;

    newPos->x = 0;
    newPos->y = 0;

    pthread_mutex_init(&newPos->lock,NULL);
    return newPos;
}


void *scalarWorker( void *arg )
{
    thread_data_t *data = (thread_data_t *)arg;
    int *item;
    while((item = getNextItem(data->pos)))
    {
        mathDelay(20000);
        //printf("Thread %d got address  %p\n", data->tid, item);
        *item += data->tid;      //in reality it would do math and not just assign it's own thread value
    }

    pthread_exit(NULL);
}

//Boss class, creates all the threads and then waits for them all to join
int scalarMultiply2d(int** matrix, int width, int length, double scalar)
{
    int i, j, rc;

    //threadList
    pthread_t thr[NUM_THREADS];
    thread_data_t thr_data[NUM_THREADS];

    /* create pos structure */
    matrix_2d_t *pos = makeMatrix2d(matrix, width, length);

    /* create threads */
    for (i = 0; i < NUM_THREADS; ++i)
    {
        thr_data[i].tid = 1 + i;
        thr_data[i].scalar = scalar;
        thr_data[i].pos = pos;
        if ((rc = pthread_create(&thr[i], NULL, scalarWorker, &thr_data[i])))
        {
            fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
            return EXIT_FAILURE;
        }
    }


    /* wait for threads to finish computing */
    for (i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(thr[i], NULL);
    }
    free(pos);
    return 1;
}

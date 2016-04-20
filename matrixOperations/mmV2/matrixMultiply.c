#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>

typedef struct _matrix_pos_t {
    int **matrix;   //matrix

    int xSize;      //matrixdimensions
    int ySize;

    int x;          //position of the last item to be operated on
    int y;
    pthread_mutex_t lock;
} matrix_pos_t;

typedef struct _thread_data_t {
  int tid;
  matrix_pos_t *pos;
  double scalar;
} thread_data_t;


int NUM_THREADS;

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

//returns the next item for a thread to operate on. If there are no longer any items return 0
int* getNextItem( matrix_pos_t * pos)
{
    int *p = NULL;

    pthread_mutex_lock(&pos->lock);
    //printMatrix(pos->matrix,pos->x, pos->y);
    if (pos->y < pos->ySize)
    {
        if (pos->x < pos->xSize)
        {
            p = &pos->matrix[pos->y][pos->x++]; //get current location address then increment x
        }
        else
        {
            if (pos->y < pos->ySize)
            {
                pos->x = 0;
                pos->y++;
                p = &pos->matrix[pos->y][pos->x];
            }
        }
    }
    pthread_mutex_unlock(&pos->lock);

    return p;
}

matrix_pos_t *makePos(int **matrix, int xSize, int ySize)
{
    matrix_pos_t *newPos = malloc(sizeof(matrix_pos_t));
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
        mathDelay(1500);
        *item = data->tid;      //in reality it would do math and not just assign it's own thread value
    }

    printf("Thread %d got no more numbers, terminate\n",data->tid);
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
    matrix_pos_t *pos = makePos(matrix, width, length);

    /* create threads */
    for (i = 0; i < NUM_THREADS; ++i)
    {
        thr_data[i].tid = 1 + i;
        thr_data[i].scalar = scalar;
        thr_data[i].pos = pos;
        printf("made thread %d\n",i+1);
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

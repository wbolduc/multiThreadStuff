#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <mQueue.h>

void freeString(void *s)
{
    free(s);
}


int main()
{
    int i,p,j;
    char *s;
    srand(time(NULL));


    printf("Queue Test:\n\n");
    queue_t *queue = createPriorityQueue(&freeString, 17);

    for (j=0; j<10; j++)
    {
        printf("%d) Adding:\n",j);

        for (i=0; i<4; i++)
        {
            p = rand()%100;
            s = malloc(sizeof(char) * 3);
            sprintf(s, "%d", p);
            addToQueue(s, p, queue);
            printf("%d: ", i+1);
            printHeap(queue);
        }

        printf("\n\n%d) Popping: ",j);
        for (i=0; i<2; i++)
        {
            s = pop(queue);
            printf("%s ", s);
            free(s);
        }
        printf("\n\n");
    }
    printf("\nWhat's left: ");
    printHeap(queue);

    destroyQueue(queue);
}

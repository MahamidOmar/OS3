//
// Created by Ayman Barham on 01/01/2023.
//
#include "queue.h"
#include <stdlib.h>

struct QueueNode_t
{
    Element data;
    struct QueueNode_t *next;
};
typedef struct QueueNode_t *QueueNode;

struct queue
{
    int currentSize;
    int maxSize;
    QueueNode head;
    QueueNode tail;
    copyElement copyFunc;
    freeElement freeFunc;
};

Queue createQueue(unsigned int maxSize, copyElement copyFunc, freeElement freeFunc)
{
    if (!copyFunc || !freeFunc)
    {
        return NULL;
    }
    Queue q = malloc(sizeof(*q));
    if (!q)
    {
        return NULL;
    }

    q->maxSize = maxSize;
    q->currentSize = 0;
    q->head = NULL;
    q->tail = NULL;
    q->copyFunc = copyFunc;
    q->freeFunc = freeFunc;

    return q;
}

void freeQueue(Queue q)
{
    if (!q)
    {
        return;
    }

    QueueNode tmp = q->head;
    QueueNode toDelete = q->head;
    while (tmp)
    {
        tmp = tmp->next;
        q->freeFunc(toDelete->data);
        free(toDelete);
        toDelete = tmp;
    }
    free(q);
}

bool isEmptyQueue(Queue q)
{
    // q is not NULL
    if (q->currentSize != 0)
    {
        return false;
    }
    return true;
}

bool isFullQueue(Queue q)
{
    // q is not NULL
    if (q->currentSize == q->maxSize)
    {
        return true;
    }
    return false;
}

unsigned int getSizeQueue(Queue q)
{
    // q is not NULL
    return q->currentSize;
}

// before adding we must check if full
void addElement(Queue q, Element value)
{
    // q is not NULL
    QueueNode toAdd = malloc(sizeof(*toAdd));
    Element data = q->copyFunc(value);

    if (!toAdd || !data)
    { // bad alloc
        return;
    }
    toAdd->data = data;
    toAdd->next = NULL;

    // if empty
    if (isEmptyQueue(q))
    {
        q->head = toAdd;
        q->tail = toAdd;
    } else
    {
        q->tail->next = toAdd;
        q->tail = toAdd;
    }

    q->currentSize++;
}

void dequeElement(Queue q)
{
    // q is not NULL
    if (isEmptyQueue(q))
    {
        return;
    }

    QueueNode toRemove = q->head;
    // if only 1
    if (getSizeQueue(q) == 1)
    {
        q->tail = NULL;
    }
    //sss

    q->head = q->head->next;
    q->currentSize--;

    q->freeFunc(toRemove->data);
    free(toRemove);

}

Element topElement(Queue q)
{
    // q is not NULL
    if (isEmptyQueue(q))
    {
        return NULL;
    }
    return q->copyFunc(q->head->data);
}

Queue removeHalfElementsRandomly(Queue q, Queue deletedNodes)
{
    // q is not NULL
    if (isEmptyQueue(q)) {
        return q;
    }

    unsigned int numToDelete = q->currentSize / 2;
    unsigned int randomIndex, j;

    numToDelete = q->currentSize - numToDelete;

    unsigned int* indexArray = malloc(sizeof(*indexArray) * q->currentSize);
    if (!indexArray) {
        exit(1);
    }
    // init to zeroes
    for (int i = 0; i < q->currentSize; ++i)
    {
        indexArray[i] = 0;
    }

    srand(time(NULL));

    j = numToDelete;
    while (j != 0)
    {
        randomIndex = rand() % q->currentSize;
        if (indexArray[randomIndex] == 1)
        {
            continue;
        }
        indexArray[randomIndex] = 1;
        j--;
    }

    Queue remainingQueue = createQueue(q->maxSize, q->copyFunc, q->freeFunc);
    QueueNode temp = q->head;
    unsigned int index = 0;

    while (temp)
    {
        if (indexArray[index] == 1)
        {
            addElement(deletedNodes, temp->data);
        } else {
            addElement(remainingQueue, temp->data);
        }
        ++index;
        temp = temp->next;
    }

    free(indexArray);

    return remainingQueue;
}

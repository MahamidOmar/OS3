//
// Created by Ayman Barham on 01/01/2023.
//

#ifndef OS3_QUEUE_H
#define OS3_QUEUE_H

#include "stdbool.h"
#include "segel.h"

typedef struct queue* Queue;
typedef void* Element;
typedef Element (*copyElement)(Element);
typedef void (*freeElement)(Element);

Queue createQueue(unsigned int maxSize, copyElement copyFunc, freeElement freeFunc);
void freeQueue(Queue q);
bool isEmptyQueue(Queue q);
bool isFullQueue(Queue);
int getSizeQueue(Queue q);
void addElement(Queue q, Element value);
void dequeElement(Queue q);
Element topElement(Queue q);
Queue removeHalfElementsRandomly(Queue q, Queue deletedNodes);


#endif //OS3_QUEUE_H

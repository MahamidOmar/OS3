#include <stdlib.h>
#include <assert.h>
#include "queue.h"


struct Node{
    Element value;
    struct Node* next;
};

struct queue{
    int size;
    int max_size;
    struct Node* start;
    struct Node* end;
    copyElement copyElement;
    freeElement freeElement;
};

Queue queueCreate(int capacity, copyElement copyFunction, freeElement freeFunction){
    if(!copyFunction || !freeFunction){
        return NULL;
    }
    Queue new_queue = malloc(sizeof(*new_queue));
    if(new_queue == NULL){
        return new_queue;
    }

    new_queue -> size = 0;
    new_queue -> max_size = capacity;
    new_queue -> start = NULL;
    new_queue -> end = NULL;
    new_queue -> copyElement = copyFunction;
    new_queue -> freeElement = freeFunction;
    return new_queue;
}

void queueFree(Queue q){
    if(q == NULL){
        return;
    }

    while(!isEmptyQueue(q)){
        dequeElement(q);
    }

    free(q);
}

bool isEmptyQueue(Queue q){
    assert(q);
    if(q -> size == 0){
        return true;
    }
    return false;
}

bool isFullQueue(Queue q){
    assert(q);
    if(q -> size == q -> max_size){
        return true;
    }
    return false;
}

int getSizeQueue(Queue q){
    assert(q);
    return q -> size;
}

//int queueGetMaxSize(Queue q){
//    assert(q);
//    return q -> max_size;
//}

void addElement(Queue q, Element value){
//    if(q == NULL){
//        return QUEUE_BAD_ARGUMENT;
//    }
//    if(q -> size + 1 > q -> max_size){
//        return QUEUE_FULL;
//    }

    struct Node* new_node = malloc(sizeof(struct Node));
    Element new_element = q -> copyElement(value);

//    if(new_node == NULL || new_element == NULL){
//        return QUEUE_OPERATION_FAIL;
//    }

    new_node -> value = new_element;
    new_node -> next = NULL;

    if(q -> start == NULL){
        q -> start = new_node;
        q -> end = new_node;
        q -> size = 1;
        return;
    }

    q -> end -> next = new_node;
    q -> end = new_node;
    q -> size += 1;

}

void dequeElement(Queue q){
//    if(q == NULL){
//        return QUEUE_BAD_ARGUMENT;
//    }
//    if(q -> size == 0){
//        return QUEUE_EMPTY;
//    }

    struct Node* tmp = NULL;

    tmp = q -> start;
    q -> start = q -> start -> next;
    q -> size -= 1;
    q -> freeElement(tmp -> value);
    free(tmp);
//    return QUEUE_SUCCESS;
}

Element topElement(Queue q){
//    if(q == NULL || value == NULL){
//        return QUEUE_BAD_ARGUMENT;
//    }
//    if(q -> size == 0){
//        return QUEUE_EMPTY;
//    }
    return q -> copyElement(q -> start -> value);

}

Queue removeHalfElementsRandomly(Queue q, Queue removed){
    if(q == NULL){
        return NULL;
    }
    if(q -> size == 0){
        return NULL;
    }

    int i, r;
    int *indexes_to_remove = (int *)malloc(sizeof(int) * (q->size));
    for(int i = 0; i < q->size; i++){
        indexes_to_remove[i] = 0;
    }

    // Calculate the ceiling of 30% of the size 
    int required_reduction = (q -> size) * 0.5;
    required_reduction = (q -> size) - required_reduction;

    srand(time(0));
    for(i = 0; i < required_reduction; i++){
        r = rand() % (q -> size);
        if(indexes_to_remove[r] == 1){
            i--;
        }
        else{
            indexes_to_remove[r] = 1;
        }
    }
    
    Queue modified_queue = queueCreate(q->max_size, q->copyElement, q->freeElement);
    struct Node *runner = q -> start;
    for(i = 0; i < q->size; i++){
        if(indexes_to_remove[i] == 0){
            addElement(modified_queue, runner->value);
        }
        else{
            addElement(removed, runner->value);
        }
        runner = runner -> next;
    }

    free(indexes_to_remove);
    return modified_queue;
}
#include "segel.h"
#include "request.h"
#include "queue.h"
#include <pthread.h>

// The maximum length of sched alg param string
#define MAXALGNAME 7

// Global Variables to manage the thread pool
pthread_mutex_t mutex;
pthread_cond_t cond;
pthread_cond_t full_cond;
int working_threads_counter;
Queue pending_queue;

// 
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

// HW3: Parse the new arguments too
void getargs(int *port, int *num_of_thread, int *q_size, char* sched_alg, int argc, char *argv[])
{
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <port> <threads> <queue_size> <schedalg>\n", argv[0]);
        exit(1);
    }
    *port = atoi(argv[1]);
    *num_of_thread = atoi(argv[2]);
    *q_size = atoi(argv[3]);
    strcpy(sched_alg, argv[4]);
}

void *thread_work_function(void* arg){
    struct timeval pickup;

    Request request;

    while(1){
        pthread_mutex_lock(&mutex);

        while(isEmptyQueue(pending_queue))
            pthread_cond_wait(&cond, &mutex);

        // get the pickup time
        gettimeofday(&pickup, NULL);

        // get request from queue
        request = topElement(pending_queue);
        dequeElement(pending_queue);

        // update dispatch time and stat_thread in request
        increaseStaticCount((StatThread)arg);
        setDispatchRequest(request, &pickup);
        requestSetThread(request, (StatThread)arg);

        // increase working count
        working_threads_counter += 1;

        pthread_mutex_unlock(&mutex);

        // Check if the pulled work from queue isn't NULL.
        // Then add to the working queue, and handle the job.
        if(request != NULL){
            requestHandle(request);
            Close(getFdRequest(request));
            destroyRequest(request);
        }

        // lock and decrease the counter then unlock
        // And tell the main thread that a thread is free again
        pthread_mutex_lock(&mutex);
        working_threads_counter -= 1;
        pthread_cond_signal(&full_cond);
        pthread_mutex_unlock(&mutex);
        
    }
    return NULL;
}


int main(int argc, char *argv[])
{
    Queue tmp_queue, removed_vals_queue;
    int listenfd, connfd, port, clientlen, num_of_threads, q_size;
    char sched_alg[MAXALGNAME];
    Request request, tmp;
    struct sockaddr_in clientaddr;

    getargs(&port, &num_of_threads, &q_size, sched_alg, argc, argv);

    // Initializing global variables
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    pthread_cond_init(&full_cond, NULL);
    working_threads_counter = 0;
    pending_queue = createQueue(q_size, copyRequest, destroyRequest);

    pthread_t *threads = malloc(sizeof(pthread_t) * num_of_threads);
    StatThread *stat_threads = malloc(sizeof(*stat_threads) * num_of_threads);
    for(int i = 0; i < num_of_threads; i++){
        stat_threads[i] = createStatThread(i);
        pthread_create(&threads[i], NULL, thread_work_function, stat_threads[i]);
    }

    listenfd = Open_listenfd(port);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
        struct timeval *arrival_time = malloc(sizeof(*arrival_time));
        gettimeofday(arrival_time, NULL);
       
        request = createRequest(connfd, arrival_time);
    
        pthread_mutex_lock(&mutex);
        if(working_threads_counter + getSizeQueue(pending_queue) >= q_size){
            if(!strcmp(sched_alg, "block")){
                pthread_cond_wait(&full_cond, &mutex);
                addElement(pending_queue, request);
            }
            else if(!strcmp(sched_alg, "dt")){
                destroyRequest(request);
                Close(connfd);
            }
            else if(!strcmp(sched_alg, "dh")){
                if(isEmptyQueue(pending_queue)){
                    destroyRequest(request);
                    Close(connfd);
                }
                else{
                    tmp = topElement(pending_queue);
                    Close(getFdRequest(tmp));
                    dequeElement(pending_queue);
                    addElement(pending_queue, request);
                    destroyRequest(tmp);
                }
            }
            else if(strcmp(sched_alg, "random") == 0){
                if(!isEmptyQueue(pending_queue)){
                    removed_vals_queue = createQueue(q_size, copyRequest, destroyRequest);
                    tmp_queue = pending_queue;
                    pending_queue = removeHalfElementsRandomly(pending_queue, removed_vals_queue);
                    freeQueue(tmp_queue);
                    fprintf(stderr, "Queue Size after random: %d", getSizeQueue(pending_queue));
                    addElement(pending_queue, request);

                    // remove all not needed values
                    while(!isEmptyQueue(removed_vals_queue)){
                        tmp = topElement(removed_vals_queue);
                        Close(getFdRequest(tmp));
                        dequeElement(removed_vals_queue);
                        destroyRequest(tmp);
                    }
                    freeQueue(removed_vals_queue);
                }
                else{
                    destroyRequest(request);
                    Close(connfd);
                }
            }
        }
        else{
            addElement(pending_queue, request);
        }
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }
    free(threads);
    free(stat_threads);
}


    


 

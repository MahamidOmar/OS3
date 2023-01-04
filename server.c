#include "segel.h"
#include "request.h"
#include <pthread.h>
#include "queue.h"
#include "stat_thread.h"

// 
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

// helper func
void scheduleNextRequest(int pending_requests_size, int connfd, char* sched_name, Request request);
//

pthread_mutex_t queue_lock;
pthread_cond_t normal_cond;
pthread_cond_t block_cond;
int current_working_num_threads;
Queue request_queue;

void *workThread(void *stat_thread)
{
    StatThread st = (StatThread) stat_thread;
    Time received_time = malloc(sizeof(struct timeval)); // maybe put this in while
    Request request;
    while(1)
    {
        pthread_mutex_lock(&queue_lock);
        for (; isEmptyQueue(request_queue);)
        {
            pthread_cond_wait(&normal_cond, &queue_lock);
        }

        // get the time of day.
        gettimeofday(received_time, NULL);
        // get the request
        request = topElement(request_queue);
        dequeElement(request_queue);

        // update statistics
        increaseThreadCount(st);
        setDispatchRequest(request, received_time);
        requestSetThread(request, st);

        // count workers
        ++current_working_num_threads;

        pthread_mutex_unlock(&queue_lock);

        if (request)
        {
            requestHandle(request);
            Close(getFdRequest(request));
            destroyRequest(request);
        }

        pthread_mutex_lock(&queue_lock);
        --current_working_num_threads;
        pthread_cond_signal(&block_cond);
        pthread_mutex_unlock(&queue_lock);
    }
    return NULL;
}

// HW3: Parse the new arguments too
void getargs(int *port, int argc, char *argv[], int *total_threads, char *sched_name, int *queue_size)
{
    if (argc != 5)
    {
        fprintf(stderr, "Usage: %s <port> <threads> <queue_size> <schedalg>\n", argv[0]);
        exit(1);
    }
    *port = atoi(argv[1]);
    *total_threads = atoi(argv[2]);
    *queue_size = atoi(argv[3]);
    strcpy(sched_name, argv[4]);
}


int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen;
    int total_thread_num, queue_size;
    struct sockaddr_in clientaddr;
    char sched_name[7]; // 7 because random is biggest string;
    Request req;

    getargs(&port, argc, argv, &total_thread_num, sched_name, &queue_size);

    // init the locks and queues
    pthread_mutex_init(&queue_lock, NULL);
    pthread_cond_init(&normal_cond, NULL);
    pthread_cond_init(&block_cond, NULL);
    current_working_num_threads = 0;
    request_queue = createQueue(queue_size, copyRequest, destroyRequest);

    // init threads and their statistics
    pthread_t *threads = malloc(sizeof(pthread_t) * total_thread_num);
    StatThread *sts = malloc(sizeof(pthread_t) * total_thread_num);
    int i = 0;
    while (i < total_thread_num)
    {
        sts[i] = createStatThread(i);
        pthread_create(&threads[i], NULL, workThread, sts[i]);
        ++i;
    }

    // start the work
    listenfd = Open_listenfd(port);
    while (1)
    {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *) &clientaddr, (socklen_t *) &clientlen);
        Time arrive_time = malloc(sizeof(*arrive_time));
        gettimeofday(arrive_time, NULL);

        req = createRequest(connfd, arrive_time);

        pthread_mutex_lock(&queue_lock);

        scheduleNextRequest(queue_size, connfd, sched_name, req);

        pthread_cond_signal(&normal_cond);
        pthread_mutex_unlock(&queue_lock);
    }

    free(threads);
    free(sts);
}

void scheduleNextRequest(int queue_size, int connfd, char* sched_name, Request request)
{
    Request tmp_request;
    if (current_working_num_threads + getSizeQueue(request_queue) < queue_size)
    {
        addElement(request_queue,request);
        return;
    }
    if (!strcmp(sched_name, "block"))
    {
        pthread_cond_wait(&block_cond, &queue_lock);
        addElement(request_queue, request);
        return;
    }
    if (!strcmp(sched_name, "dt"))
    {
        destroyRequest(request);
        Close(connfd);
        return;
    }
    if (!strcmp(sched_name, "dh"))
    {
        if (isEmptyQueue(request_queue))
        {
            destroyRequest(request);
            Close(connfd);
            return;
        }

        // handle oldest request
        tmp_request = topElement(request_queue);
        Close(getFdRequest(tmp_request));
        dequeElement(request_queue);
        destroyRequest(tmp_request);

        // insert new request
        addElement(request_queue, request);
        return;
    }

    if (!strcmp(sched_name, "random"))
    {
        if (isEmptyQueue(request_queue))
        {
            destroyRequest(request);
            Close(connfd);
            return;
        }

        Queue deleted_vals, old_queue;
        deleted_vals = createQueue(queue_size, copyRequest, destroyRequest);
        old_queue = request_queue;

        request_queue = removeHalfElementsRandomly(request_queue, deleted_vals);

        freeQueue(old_queue);

        fprintf(stderr, "Queue Size after random: %d", getSizeQueue(request_queue));

        addElement(request_queue, request);

        while (!isEmptyQueue(deleted_vals))
        {
            tmp_request = topElement(deleted_vals);
            Close(getFdRequest(tmp_request));
            dequeElement(deleted_vals);
            destroyRequest(tmp_request);
        }

        freeQueue(deleted_vals);
        return;
    }
}
//
// Created by Ayman Barham on 01/01/2023.
//

#ifndef OS3_STAT_THREAD_H
#define OS3_STAT_THREAD_H
#include "stdlib.h"

typedef struct statThread_t* StatThread;
StatThread createStatThread(int threadId);
void increaseThreadCount(StatThread st);
void increaseDynamicCount(StatThread st);
void increaseStaticCount(StatThread st);

int getThreadId(StatThread st);
int getThreadCount(StatThread st);
int getThreadStaticCount(StatThread st);
int getThreadDynamicCount(StatThread st);

#endif //OS3_STAT_THREAD_H

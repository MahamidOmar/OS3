//
// Created by Ayman Barham on 01/01/2023.
//

#include "stat_thread.h"


StatThread createStatThread(int threadId)
{
    StatThread st = malloc(sizeof(*st));
    if (!st)
    {
        return NULL;
    }

    st->id = threadId;
    st->count = 0;
    st->dynamicCount = 0;
    st->staticCount = 0;

    return st;
}

void increaseThreadCount(StatThread st)
{
    ++st->count;
}

void increaseDynamicCount(StatThread st)
{
    ++st->dynamicCount;
}

void increaseStaticCount(StatThread st)
{
    ++st->staticCount;
}

int getThreadId(StatThread st)
{
    return st->id;
}

int getThreadCount(StatThread st)
{
    return st->count;
}

int getThreadStaticCount(StatThread st)
{
    return st->staticCount;
}

int getThreadDynamicCount(StatThread st)
{
    return st->dynamicCount;
}
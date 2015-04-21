#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>
#include <pthread.h>

#define PTHREAD_USING_MUTEX(mutex) \
    for(bool pthread_using_mutex_flag_ = true; \
        pthread_using_mutex_flag_ && pthread_mutex_lock(mutex) == 0; \
        pthread_mutex_unlock(mutex), pthread_using_mutex_flag_ = 0)

#endif // UTIL_H

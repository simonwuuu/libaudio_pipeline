/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2021 <ESPRESSIF SYSTEMS (SHANGHAI) CO., LTD>
 *
 * Permission is hereby granted for use on all ESPRESSIF SYSTEMS products, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sched.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include "portmacro.h"
#include "audio_mutex.h"
#include "audio_idf_version.h"
#include "esp_log.h"
#include "audio_mem.h"

pthread_mutex_t *mutex_create(void)
{
    pthread_mutex_t *mutex = NULL;
    mutex = (pthread_mutex_t *)audio_malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(mutex, NULL);
    return mutex;
}

int mutex_destroy(pthread_mutex_t *mutex)
{
        pthread_mutex_destroy(mutex);
        audio_free(mutex);
    return 0;
}

int mutex_lock(pthread_mutex_t *mutex)
{
    struct timespec ts;
    int status;
    status = clock_gettime(CLOCK_REALTIME, &ts);
    if (status < 0)
    {
        int errcode = errno;
        fprintf(stderr, "rb_block: clock_gettime() failed: %d\n", errcode);
        abort();
    }
    ts.tv_sec += portMAX_DELAY;

    return pthread_mutex_timedlock(mutex, &ts);
}

int mutex_unlock(pthread_mutex_t *mutex)
{
    return pthread_mutex_unlock(mutex);
}

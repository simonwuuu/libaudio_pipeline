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

#include "audio_queue.h"
#include <time.h>

#  define CONFIG_AUDIO_MSG_PRIO  1

mqd_t audio_queue_create(const char *mq_name, int oflags, uint32_t queue_len, uint32_t item_size)
{
    struct mq_attr attr;
    /* Fill in attributes for message queue */

    attr.mq_maxmsg  = queue_len;
    attr.mq_msgsize = item_size;
    // attr.mq_flags   = 0;

    return mq_open(mq_name, oflags, 0666, &attr);
}

int audio_queue_delete(mqd_t queue, const char *mq_name)
{
    int ret = 0;
    ret = mq_close(queue);
    if (ret < 0)
    {
        int errcode = errno;
        assert(errcode > 0);
        printf("ERROR: mq_close failed: %d\n", errcode);
        ret = -errcode;
        return ret;
    }
    ret = mq_unlink(mq_name);
    if (ret < 0)
    {
        int errcode = errno;
        assert(errcode > 0);
        printf("ERROR: mq_unlink failed: %d\n", errcode);
        ret = -errcode;
        return ret;
    }
    return ret;
}

int audio_queue_send(mqd_t queue, char *item, size_t msglen, TickType_t wait_time)
{
    int ret = 0;
    struct timespec ts;
    if(wait_time != 0) {
        int status = clock_gettime(CLOCK_REALTIME, &ts);
        if (status != 0)
        {
            printf("clock_gettime: ERROR clock_gettime failed\n");
            return -1;
        }
        ts.tv_sec += wait_time;
        ret = mq_timedsend((mqd_t)queue, item, msglen, CONFIG_AUDIO_MSG_PRIO, &ts);
    } else {
        ret = mq_send((mqd_t)queue, item, msglen, CONFIG_AUDIO_MSG_PRIO);
    }

    return ret;
}

int audio_queue_recv(mqd_t queue, char *item, size_t msglen, TickType_t wait_time)
{
    int ret = 0;
    struct timespec ts;
    if(wait_time != 0) {
        int status = clock_gettime(CLOCK_REALTIME, &ts);
        if (status != 0)
        {
            printf("clock_gettime: ERROR clock_gettime failed\n");
            return -1;
        }
        ts.tv_sec += wait_time;
        ret = mq_timedreceive((mqd_t)queue, item, msglen, 0, &ts);
    } else {
        ret = mq_receive((mqd_t)queue, item, msglen, 0);
    }

    return ret;
}

int audio_queue_message_available(const mqd_t queue)
{
    struct mq_attr mq_stat;
    mq_getattr(queue, &mq_stat);
    return mq_stat.mq_curmsgs;
}
/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2020 <ESPRESSIF SYSTEMS (SHANGHAI) CO., LTD>
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

#include "esp_log.h"
#include "audio_mem.h"
#include "audio_error.h"
#include "audio_thread.h"

esp_err_t audio_thread_create(audio_thread_t *p_handle, const char *name, void*(*main_func)(void*) , void *arg,
                              uint32_t stack, int prio, bool stack_in_ext, int core_id)
{
    pthread_attr_t attr;
    int res = pthread_attr_init(&attr);
    assert(res == 0);
    res = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    assert(res == 0);
    // res = pthread_attr_setstacksize(&attr, stack);
    // assert(res == 0);
    res = pthread_create(p_handle, &attr, main_func, arg);
    assert(res == 0);
    return ESP_OK;
}

esp_err_t audio_thread_cleanup(audio_thread_t *p_handle)
{
    return ESP_OK;
}

esp_err_t audio_thread_delete_task(audio_thread_t *p_handle)
{
    int res = pthread_cancel(*p_handle);
    assert(res == 0);
    return ESP_OK; /* Control never reach here if this is self delete */
}

/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2018 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
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

#ifndef _HTTP_STREAM_H_
#define _HTTP_STREAM_H_

#include "audio_error.h"
#include "audio_element.h"
#include "audio_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief      HTTP Stream configurations
 *             Default value will be used if any entry is zero
 */
typedef struct {
    audio_stream_type_t         type;                   /*!< Type of stream */
    int                         out_rb_size;            /*!< Size of output ringbuffer */
    int                         task_stack;             /*!< Task stack size */
    int                         task_core;              /*!< Task running in core (0 or 1) */
    int                         task_prio;              /*!< Task priority (based on freeRTOS priority) */
    bool                        stack_in_ext;           /*!< Try to allocate stack in external memory */
} http_stream_cfg_t;


#define HTTP_STREAM_TASK_STACK          (6 * 1024)
#define HTTP_STREAM_TASK_CORE           (0)
#define HTTP_STREAM_TASK_PRIO           (4)
#define HTTP_STREAM_RINGBUFFER_SIZE     (20 * 1024)

#define HTTP_STREAM_CFG_DEFAULT() {              \
    .type = AUDIO_STREAM_READER,                 \
    .out_rb_size = HTTP_STREAM_RINGBUFFER_SIZE,  \
    .task_stack = HTTP_STREAM_TASK_STACK,        \
    .task_core = HTTP_STREAM_TASK_CORE,          \
    .task_prio = HTTP_STREAM_TASK_PRIO,          \
    .stack_in_ext = true,                        \
}

/**
 * @brief      Create a handle to an Audio Element to stream data from HTTP to another Element
 *             or get data from other elements sent to HTTP, depending on the configuration
 *             the stream type, either AUDIO_STREAM_READER or AUDIO_STREAM_WRITER.
 *
 * @param      config  The configuration
 *
 * @return     The Audio Element handle
 */
audio_element_handle_t http_stream_init(http_stream_cfg_t *config);

#ifdef __cplusplus
}
#endif

#endif

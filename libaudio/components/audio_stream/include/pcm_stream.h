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

#ifndef _PCM_STREAM_H_
#define _PCM_STREAM_H_

#include "audio_error.h"
#include "audio_element.h"
#include "audio_common.h"
#include <tinyalsa/asoundlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PCM_ERROR_MAX 128

/** A PCM handle.
 * @ingroup libtinyalsa-pcm
 */
struct pcm {
    /** The PCM's file descriptor */
    int fd;
    /** Flags that were passed to @ref pcm_open */
    unsigned int flags;
    /** The number of (under/over)runs that have occured */
    int xruns;
    /** Size of the buffer */
    unsigned int buffer_size;
    /** The boundary for ring buffer pointers */
    unsigned int boundary;
    /** Description of the last error that occured */
    char error[PCM_ERROR_MAX];
    /** Configuration that was passed to @ref pcm_open */
    struct pcm_config config;
    struct snd_pcm_mmap_status *mmap_status;
    struct snd_pcm_mmap_control *mmap_control;
    struct snd_pcm_sync_ptr *sync_ptr;
    void *mmap_buffer;
    unsigned int noirq_frames_per_msec;
    /** The delay of the PCM, in terms of frames */
    long pcm_delay;
    /** The subdevice corresponding to the PCM */
    unsigned int subdevice;
    /** Pointer to the pcm ops, either hw or plugin */
    const struct pcm_ops *ops;
    /** Private data for pcm_hw or pcm_plugin */
    void *data;
    /** Pointer to the pcm node from snd card definition */
    struct snd_node *snd_node;
};

/**
 * @brief   PCM Stream configurations.
 */
typedef struct {
    audio_stream_type_t     type;               /*!< Type of stream */
    int                     buf_sz;             /*!< Audio Element Buffer size */
    int                     out_rb_size;        /*!< Size of output ringbuffer */
    int                     task_stack;         /*!< Task stack size */
    int                     task_core;          /*!< Task running in core (0 or 1) */
    int                     task_prio;          /*!< Task priority (based on freeRTOS priority) */
    struct   pcm            pcm;               /*!< pcm struct */
    unsigned int            card;               /* The card that the pcm belongs to.The default card is zero. */
    unsigned int            device;             /* The device that the pcm belongs to. The default device is zero. */
} pcm_stream_cfg_t;

#define PCM_STREAM_BUF_SIZE            (4 * 2048)
#define PCM_STREAM_TASK_STACK          (3072)
#define PCM_STREAM_TASK_CORE           (0)
#define PCM_STREAM_TASK_PRIO           (4)
#define PCM_STREAM_RINGBUFFER_SIZE     (32 * 1024)


#define PCM_STREAM_CFG_DEFAULT() {                   \
    .type = AUDIO_STREAM_NONE,                       \
    .buf_sz = PCM_STREAM_BUF_SIZE,                   \
    .out_rb_size = PCM_STREAM_RINGBUFFER_SIZE,       \
    .task_stack = PCM_STREAM_TASK_STACK,             \
    .task_core = PCM_STREAM_TASK_CORE,               \
    .task_prio = PCM_STREAM_TASK_PRIO,               \
    .pcm = {                                         \
        .fd = 0,                                     \
        .flags = 0,                                  \
        .xruns = 0,                                  \
        .buffer_size = 0,                            \
        .boundary = 0,                               \
        .error = NULL,                               \
        .config = {                                  \
            .channels = 2,                           \
            .rate = 48000,                           \
            .period_size = 1024,                     \
            .period_count = 2,                       \
            .format = PCM_FORMAT_S16_LE,             \
            .start_threshold = 2048,                 \
            .stop_threshold = 2048,                  \
            .silence_threshold = 0,                  \
            .silence_size = 0,                       \
            .avail_min = 1024,                       \
        },                                           \
        .mmap_status = NULL,                         \
        .mmap_control = NULL,                        \
        .sync_ptr = NULL,                            \
        .mmap_buffer = NULL,                         \
        .noirq_frames_per_msec = 0,                  \
        .pcm_delay = 0,                              \
        .subdevice = 0,                              \
        .ops = NULL,                                 \
        .data = NULL,                                \
        .snd_node = NULL,                            \
    },                                               \
    .card = 0,                                       \
    .device = 0,                                     \
}

/**
 * @brief      Create a handle to an Audio Element to stream data from PCM to another Element
 *             or get data from other elements written to PCM, depending on the configuration
 *             the stream type, either AUDIO_STREAM_READER or AUDIO_STREAM_WRITER.
 *
 * @param      config  The configuration
 *
 * @return     The Audio Element handle
 */
audio_element_handle_t pcm_stream_init(pcm_stream_cfg_t *config);

/**
 * @brief      Get volume of stream
 *
 * @param[in]  pcm_stream   The pcm element handle
 * @param[in]  volume       The volume of stream
 *
 * @return
 *     - ESP_OK  
 *     - ESP_FAIL  
 */
esp_err_t pcm_volume_get(audio_element_handle_t pcm_stream, int* volume);

/**
 * @brief      Setup volume of stream.
 *
 * @param[in]  pcm_stream   The pcm element handle.
 * @param[in]  volume       The volume of stream will be set.
 * 
 * @return
 *     - ESP_OK
 *     - ESP_FAIL
 */
esp_err_t pcm_volume_set(audio_element_handle_t pcm_stream, int volume);

#ifdef __cplusplus
}
#endif

#endif

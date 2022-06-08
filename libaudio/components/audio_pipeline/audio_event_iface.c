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
#include "audio_event_iface.h"

static const char *TAG = "AUDIO_EVT";


typedef struct audio_event_iface_item {
    STAILQ_ENTRY(audio_event_iface_item)    next;
    mqd_t                           queue;
    int                                     queue_size;
    int                                     mark_to_remove;
} audio_event_iface_item_t;

typedef STAILQ_HEAD(audio_event_iface_list, audio_event_iface_item) audio_event_iface_list_t;

/**
 * Audio event structure
 */
struct audio_event_iface {
    mqd_t                internal_queue;
    mqd_t                external_queue;
    int                        oflags;
    char                    internal_mqname[16];
    int                        internal_queue_size;
    int                        external_queue_size;
    audio_event_iface_list_t    listening_queues;
    void                        *context;
    on_event_iface_func         on_cmd;
    int                         wait_time;
    int                         type;
};

audio_event_iface_handle_t audio_event_iface_init(audio_event_iface_cfg_t *config)
{
    int ret;
    audio_event_iface_handle_t evt = audio_calloc(1, sizeof(struct audio_event_iface));
    AUDIO_MEM_CHECK(TAG, evt, return NULL);
    evt->external_queue =  config->external_queue;
    evt->oflags = config->oflags;
    evt->internal_queue_size = config->internal_queue_size;
    evt->external_queue_size = config->external_queue_size;
    evt->context = config->context;
    evt->on_cmd = config->on_cmd;
    evt->type = config->type;
    snprintf(evt->internal_mqname, sizeof(evt->internal_mqname), "/%0lx",  (unsigned long)((uintptr_t)evt));
    if (evt->internal_queue_size) {
        evt->internal_queue = audio_queue_create(evt->internal_mqname, evt->oflags, evt->internal_queue_size, sizeof(audio_event_iface_msg_t));
        AUDIO_MEM_CHECK(TAG, evt->internal_queue, goto _event_iface_init_failed);
        if (evt->internal_queue == (mqd_t) -1)
        {
            /* Unable to open message queue! */
            ret = -errno;
            printf("ERROR: mq_open failed: %d\n", ret);
            goto _event_iface_init_failed;
        }
    }

    return evt;
_event_iface_init_failed:
    return NULL;
}

esp_err_t audio_event_iface_read(audio_event_iface_handle_t evt, audio_event_iface_msg_t *msg, TickType_t wait_time)
{
    if (evt->internal_queue) {
            if (audio_queue_recv(evt->internal_queue, (char*)msg, sizeof(audio_event_iface_msg_t), wait_time) == sizeof(audio_event_iface_msg_t)) {
                return ESP_OK;
            }
    }
    return ESP_FAIL;
}

esp_err_t audio_event_iface_destroy(audio_event_iface_handle_t evt)
{
    audio_event_iface_discard(evt);

    if (evt->internal_queue) {
        audio_event_iface_set_cmd_waiting_timeout(evt, 0);
        audio_queue_delete(evt->internal_queue, evt->internal_mqname);
    }

    audio_free(evt);
    return ESP_OK;
}

esp_err_t audio_event_iface_set_listener(audio_event_iface_handle_t evt, audio_event_iface_handle_t listener)
{
    if ((0 == listener->internal_queue)
        || (0 == listener->internal_queue_size)) {
        return ESP_ERR_INVALID_ARG;
    }

    if (audio_event_iface_discard(listener) != ESP_OK) {
        AUDIO_ERROR(TAG, "Error cleanup listener");
        return ESP_FAIL;
    }

    evt->external_queue = listener->internal_queue;

    return ESP_OK;
}

esp_err_t audio_event_iface_set_msg_listener(audio_event_iface_handle_t evt, audio_event_iface_handle_t listener)
{

    // if ((NULL == evt->internal_queue)
    //     || (0 == evt->internal_queue_size)) {
    //     return ESP_ERR_INVALID_ARG;
    // }
    // audio_event_iface_item_t *item = audio_calloc(1, sizeof(audio_event_iface_item_t));
    // AUDIO_MEM_CHECK(TAG, item, return ESP_ERR_NO_MEM);
    // if (audio_event_iface_discard(listener) != ESP_OK) {
    //     AUDIO_ERROR(TAG, "Error cleanup listener");
    //     return ESP_FAIL;
    // }
    // item->queue = evt->internal_queue;
    // item->queue_size = evt->internal_queue_size;
    // STAILQ_INSERT_TAIL(&listener->listening_queues, item, next);
    // return audio_event_iface_update_listener(listener);
    printf("ERROR: audio_event_iface_set_msg_listener is unimplemented");
    return ESP_FAIL;

}

esp_err_t audio_event_iface_remove_listener(audio_event_iface_handle_t listen, audio_event_iface_handle_t evt)
{
    if ((0 == evt->external_queue)
        || (0 == evt->external_queue_size)) {
        return ESP_ERR_INVALID_ARG;
    }

    if (audio_event_iface_discard(listen) != ESP_OK) {
        AUDIO_ERROR(TAG, "Error cleanup listener");
        return ESP_FAIL;
    }

    evt->external_queue = 0;
    return ESP_OK;
}

esp_err_t audio_event_iface_set_cmd_waiting_timeout(audio_event_iface_handle_t evt, TickType_t wait_time)
{
    evt->wait_time = wait_time;
    return ESP_OK;
}

esp_err_t audio_event_iface_waiting_cmd_msg(audio_event_iface_handle_t evt)
{
    audio_event_iface_msg_t msg;
    if (evt->internal_queue && (audio_queue_recv(evt->internal_queue, (char*)&msg, sizeof(audio_event_iface_msg_t), evt->wait_time) == sizeof(audio_event_iface_msg_t))) {
        if (evt->on_cmd) {
            return evt->on_cmd((void *)&msg, evt->context);
        }
    }
    return ESP_OK;
}

esp_err_t audio_event_iface_cmd(audio_event_iface_handle_t evt, audio_event_iface_msg_t *msg)
{
    if (evt->internal_queue) {
        int status = 0;
        status = audio_queue_send(evt->internal_queue, (char*)msg, sizeof(audio_event_iface_msg_t), 0);
        if (status < 0) {
            printf("audio_event_iface_cmd: ERROR audio_queue_send failure=%d on msg\n",status);
            return ESP_FAIL;
        }
    }
    return ESP_OK;
}

esp_err_t audio_event_iface_sendout(audio_event_iface_handle_t evt, audio_event_iface_msg_t *msg)
{
    if (evt->external_queue) {
        int status = 0;
        status = audio_queue_send(evt->external_queue, (char*)msg, sizeof(audio_event_iface_msg_t), 0);
        if (status < 0) {
            printf("audio_event_iface_sendout: ERROR audio_queue_send failure=%d on msg\n",status);
            return ESP_FAIL;
        }
    }
    return ESP_OK;
}

esp_err_t audio_event_iface_discard(audio_event_iface_handle_t evt)
{
    audio_event_iface_msg_t msg;
    if (evt->external_queue && evt->external_queue_size) {
        while((audio_queue_message_available(evt->external_queue) > 0) &&
                    (audio_queue_recv(evt->external_queue, (char*)&msg, sizeof(audio_event_iface_msg_t), 0) == sizeof(audio_event_iface_msg_t)));
    }
    if (evt->internal_queue && evt->internal_queue_size) {
        while((audio_queue_message_available(evt->internal_queue) > 0) &&
                    (audio_queue_recv(evt->internal_queue, (char*)&msg, sizeof(audio_event_iface_msg_t), 0) == sizeof(audio_event_iface_msg_t)));
    }
    return ESP_OK;
}

esp_err_t audio_event_iface_listen(audio_event_iface_handle_t evt, audio_event_iface_msg_t *msg, TickType_t wait_time)
{
    if (!evt) {
        return ESP_FAIL;
    }
    if (audio_event_iface_read(evt, msg, wait_time) != ESP_OK) {
        return ESP_FAIL;
    }
    return ESP_OK;
}

mqd_t audio_event_iface_get_queue_handle(audio_event_iface_handle_t evt)
{
    if (!evt) {
        return 0;
    }
    return evt->external_queue;
}

mqd_t audio_event_iface_get_msg_queue_handle(audio_event_iface_handle_t evt)
{
    if (!evt) {
        return 0;
    }
    return evt->internal_queue;
}

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

#include <sys/unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include "errno.h"

#include "pcm_stream.h"
#include "audio_common.h"
#include "audio_mem.h"
#include "audio_element.h"
#include "wav_head.h"
#include "esp_log.h"
#include "unistd.h"
#include "fcntl.h"

static const char *TAG = "PCM_STREAM";

typedef struct pcm_stream {
    pcm_stream_cfg_t config;
    audio_stream_type_t type;
    bool is_open;
    // bool use_alc;
    // void *volume_handle;
    // int volume
} pcm_stream_t;

static esp_err_t _pcm_open(audio_element_handle_t self) {
    pcm_stream_t *pcm_stream = (pcm_stream_t *)audio_element_getdata(self);
    if (pcm_stream->is_open) {
        return ESP_OK;
    }
    pcm_stream_cfg_t *cfg = audio_calloc(1, sizeof(pcm_stream_cfg_t));
    memcpy(cfg, &pcm_stream->config, sizeof(pcm_stream_cfg_t));
    unsigned int card = cfg->card;
    unsigned int device = cfg->device;
    unsigned int flags = PCM_OUT;
    if (pcm_stream->type == AUDIO_STREAM_READER) {
        flags = PCM_IN;
    }
    if (cfg) {
        free(cfg);
    }
    struct pcm *pcm_t = audio_calloc(1, sizeof(struct pcm));
    pcm_t = pcm_open(card, device, flags, &(pcm_stream->config.pcm.config));
    // pcm_stream->config.pcm = pcm_open(card, device, flags, &(pcm_stream->config.pcm.config));
    if (!pcm_is_ready(pcm_t)) {
        ESP_LOGE(TAG, "failed to open for pcm %u,%u. %s\n", card, device, pcm_get_error(pcm_t));
        pcm_close(pcm_t);
        return ESP_FAIL;
    }
    memcpy(&pcm_stream->config.pcm, pcm_t, sizeof(struct pcm));
    audio_element_setdata(self, pcm_stream);
    if (pcm_t) {
        free(pcm_t);
    }
    pcm_stream->is_open = true;
    return ESP_OK;
}

static esp_err_t _pcm_close(audio_element_handle_t self) {
    pcm_stream_t *pcm_stream = (pcm_stream_t *)audio_element_getdata(self);
    if (pcm_stream->is_open) {
        struct pcm *pcm_t = audio_calloc(1, sizeof(struct pcm));
        memcpy(pcm_t, &pcm_stream->config.pcm, sizeof(struct pcm));
        pcm_close(pcm_t);
        pcm_stream->is_open = false;
    }
    if (AEL_STATE_PAUSED != audio_element_get_state(self)) {
        audio_element_report_info(self);
        audio_element_set_byte_pos(self, 0);
    }
    return ESP_OK;
}

static int _pcm_process(audio_element_handle_t self, char *in_buffer, int in_len) {
    int r_size = audio_element_input(self, in_buffer, in_len);
    int w_size = 0;
    if (r_size > 0) {
        w_size = audio_element_output(self, in_buffer, r_size);
    } else {
        w_size = r_size;
    }
    return w_size;
}

static esp_err_t _pcm_destroy(audio_element_handle_t self) {
    pcm_stream_t *pcm_stream = (pcm_stream_t *)audio_element_getdata(self);
    audio_free(pcm_stream);
    return ESP_OK;
}

static int _pcm_write(audio_element_handle_t self, char *buffer, int len, TickType_t ticks_to_wait, void *context) {
    pcm_stream_t *pcm_stream = (pcm_stream_t *)audio_element_getdata(self);
    audio_element_info_t info;
    audio_element_getinfo(self, &info);
    struct pcm *pcm_t = &pcm_stream->config.pcm;
    int written_frames = pcm_writei(pcm_t, buffer, pcm_bytes_to_frames(pcm_t, len));
    if (written_frames < 0) {
        ESP_LOGE(TAG, "The error is happened in writing data. Error message: %s", strerror(errno));
        return written_frames;
    }
    int written_bytes = pcm_frames_to_bytes(pcm_t, written_frames);
    info.byte_pos += written_bytes;
    audio_element_setinfo(self, &info);
    return written_bytes;
}

static int _pcm_read(audio_element_handle_t self, char *buffer, int len, TickType_t ticks_to_wait, void *context) {
    pcm_stream_t *pcm_stream = (pcm_stream_t *)audio_element_getdata(self);
    audio_element_info_t info;
    audio_element_getinfo(self, &info);
    struct pcm *pcm_t = &pcm_stream->config.pcm;
    // int read_frames = pcm_readi(pcm_t, buffer, pcm_bytes_to_frames(pcm_t, len));
    int read_frames = pcm_readi(pcm_t, buffer, pcm_get_buffer_size(pcm_t));
    if (read_frames < 0) {
        ESP_LOGE(TAG, "The error is happened in reading data. Error message: %s", strerror(errno));
        return read_frames;
    }
    int read_bytes = pcm_frames_to_bytes(pcm_t, read_frames);
    info.byte_pos += read_bytes;
    audio_element_setinfo(self, &info);
    return read_bytes;
}

audio_element_handle_t pcm_stream_init(pcm_stream_cfg_t *config)
{
    audio_element_handle_t el;
    pcm_stream_t *pcm = audio_calloc(1, sizeof(pcm_stream_t));

    AUDIO_MEM_CHECK(TAG, pcm, return NULL);

    audio_element_cfg_t cfg = DEFAULT_AUDIO_ELEMENT_CONFIG();
    cfg.open = _pcm_open;
    cfg.close = _pcm_close;
    cfg.process = _pcm_process;
    cfg.destroy = _pcm_destroy;
    cfg.task_stack = config->task_stack;
    cfg.task_prio = config->task_prio;
    cfg.task_core = config->task_core;
    cfg.out_rb_size = config->out_rb_size;
    cfg.buffer_len = config->buf_sz;
    if (cfg.buffer_len == 0) {
        cfg.buffer_len = PCM_STREAM_BUF_SIZE;
    }
    cfg.tag = "pcm";
    memcpy(&pcm->config, config, sizeof(pcm_stream_cfg_t));
    pcm->type = config->type;

    if (config->type == AUDIO_STREAM_WRITER) {
        cfg.write = _pcm_write;
    } else {
        cfg.read = _pcm_read;
    }
    el = audio_element_init(&cfg);

    AUDIO_MEM_CHECK(TAG, el, goto _pcm_init_exit);
    audio_element_setdata(el, pcm);
    audio_element_info_t info;
    audio_element_getinfo(el, &info);
    info.sample_rates = config->pcm.config.rate;
    info.channels = config->pcm.config.channels;
    info.bits = pcm_format_to_bits(config->pcm.config.format);
    audio_element_setinfo(el, &info);
    return el;
_pcm_init_exit:
    audio_free(pcm);
    return NULL;
}

int pcm_volume_get(audio_element_handle_t self, int* volume) {
    pcm_stream_t *pcm_stream = (pcm_stream_t *)audio_element_getdata(self);
    struct mixer *mixer;
    struct mixer_ctl *ctl;
    unsigned int card = pcm_stream->config.card;
    mixer = mixer_open(card);
    if (!mixer) {
        ESP_LOGE(TAG, "Failed to open mixer\n");
        return ESP_FAIL;
    }
    ctl = mixer_get_ctl_by_name(mixer, "Master Playback Volume");
    *volume =  mixer_ctl_get_value(ctl, 0);
    // volume[1] =  mixer_ctl_get_value(ctl, 1);
    mixer_close(mixer);
    return ESP_OK;
}

int pcm_volume_set(audio_element_handle_t self, int volume) {
    // if (volume == NULL || sizeof(volume) / sizeof(int) < 2) {
    //     ESP_LOGE(TAG, "Missing volume parameter\n");
    //     return ESP_FAIL;
    // }
    pcm_stream_t *pcm_stream = (pcm_stream_t *)audio_element_getdata(self);
    struct mixer *mixer;
    struct mixer_ctl *ctl;
    unsigned int card = pcm_stream->config.card;
    mixer = mixer_open(card);
    if (!mixer) {
        ESP_LOGE(TAG, "Failed to open mixer\n");
        return ESP_FAIL;
    }
    ctl = mixer_get_ctl_by_name(mixer, "Master Playback Volume");
    mixer_ctl_set_value(ctl, 0, volume);
    // mixer_ctl_set_value(ctl, 1, volume[1]);
    mixer_close(mixer);
    return ESP_OK;
}
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
#include <string.h>

#include "esp_log.h"
#include "errno.h"
#include "http_stream.h"
#include "audio_mem.h"
#include "audio_element.h"
#include "esp_http_client.h"
#include <strings.h>

static const char *TAG = "HTTP_STREAM";
#define HTTP_STREAM_BUFFER_SIZE (2048)

typedef struct http_stream {
    audio_stream_type_t             type;
    bool                            is_open;
    esp_http_client_handle_t        client;
} http_stream_t;

static esp_err_t _http_open(audio_element_handle_t self)
{
    http_stream_t *http = (http_stream_t *)audio_element_getdata(self);
    esp_err_t err;
    char *uri = NULL;
    audio_element_info_t info;
    ESP_LOGD(TAG, "_http_open");

    if (http->is_open) {
        ESP_LOGE(TAG, "already opened");
        return ESP_FAIL;
    }

    uri = audio_element_get_uri(self);
    if (uri == NULL) {
        ESP_LOGE(TAG, "Error open connection, uri = NULL");
        return ESP_FAIL;
    }
    audio_element_getinfo(self, &info);
    ESP_LOGD(TAG, "URI=%s", uri);
    // if not initialize http client, initial it
    if (http->client == NULL) {
        esp_http_client_config_t http_cfg = {
            .url = uri,
            .host = malloc(1024*sizeof(char))
        };
        http->client = esp_http_client_init(&http_cfg);
        AUDIO_MEM_CHECK(TAG, http->client, return ESP_ERR_NO_MEM);
    }
    if ((err = esp_http_client_open(http->client)) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open http stream");
        return err;
    }
    info.total_bytes = esp_http_client_get_content_length(http->client);

    ESP_LOGI(TAG, "total_bytes=%d", (int)info.total_bytes);
    int status_code = esp_http_client_get_status_code(http->client);
    if (status_code != 200
        && (esp_http_client_get_status_code(http->client) != 206)) {
        ESP_LOGE(TAG, "Invalid HTTP stream, status code = %d", status_code);
        return ESP_FAIL;
    }

    audio_element_set_total_bytes(self, info.total_bytes);

    http->is_open = true;
    return ESP_OK;
}

static esp_err_t _http_close(audio_element_handle_t self)
{
    http_stream_t *http = (http_stream_t *)audio_element_getdata(self);
    ESP_LOGD(TAG, "_http_close");

    if (http->client) {
        esp_http_client_close(http->client);
        esp_http_client_cleanup(http->client);
        http->client = NULL;
    }
    http->is_open = false;
    return ESP_OK;
}

static int _http_read(audio_element_handle_t self, char *buffer, int len, TickType_t ticks_to_wait, void *context)
{
    http_stream_t *http = (http_stream_t *)audio_element_getdata(self);
    audio_element_info_t info;
    audio_element_getinfo(self, &info);
    int rlen = esp_http_client_read(http->client, buffer, len);
    audio_element_update_byte_pos(self, rlen);
    
    ESP_LOGD(TAG, "req lengh=%d, read=%d, pos=%d/%d", len, rlen, (int)info.byte_pos, (int)info.total_bytes);
    return rlen;
}

static int _http_write(audio_element_handle_t self, char *buffer, int len, TickType_t ticks_to_wait, void *context)
{
    http_stream_t *http = (http_stream_t *)audio_element_getdata(self);
    int wlen = 0;
    if ((wlen = esp_http_client_write(http->client, buffer, len)) <= 0) {
        ESP_LOGE(TAG, "Failed to write data to http stream, wlen=%d, errno=%d", wlen, errno);
    }
    return wlen;
}

static int _http_process(audio_element_handle_t self, char *in_buffer, int in_len)
{
    int r_size = audio_element_input(self, in_buffer, in_len);
    if (audio_element_is_stopping(self) == true) {
        ESP_LOGW(TAG, "No output due to stopping");
        return AEL_IO_ABORT;
    }
    int w_size = 0;
    if (r_size > 0){
        w_size = audio_element_output(self, in_buffer, r_size);
    }
    else{
        w_size = r_size;
    }
    
    return w_size;
}

static esp_err_t _http_destroy(audio_element_handle_t self)
{
    http_stream_t *http = (http_stream_t *)audio_element_getdata(self);
    if (http->client) {
        audio_free(http->client->response->head_buffer);
        audio_free(http->client->request->head_buffer);
        audio_free(http->client->request);
        audio_free(http->client->response);
        audio_free(http->client);
    }
    audio_free(http);
    http = NULL;
    return ESP_OK;
}

audio_element_handle_t http_stream_init(http_stream_cfg_t *config)
{
    audio_element_handle_t el;
    http_stream_t *http = audio_calloc(1, sizeof(http_stream_t));

    AUDIO_MEM_CHECK(TAG, http, return NULL);

    audio_element_cfg_t cfg = DEFAULT_AUDIO_ELEMENT_CONFIG();
    cfg.open = _http_open;
    cfg.close = _http_close;
    cfg.process = _http_process;
    cfg.destroy = _http_destroy;
    cfg.task_stack = config->task_stack;
    cfg.task_prio = config->task_prio;
    cfg.task_core = config->task_core;
    cfg.stack_in_ext = config->stack_in_ext;
    cfg.out_rb_size = config->out_rb_size;

    cfg.tag = "http";

    http->type = config->type;

    if (config->type == AUDIO_STREAM_READER) {
        cfg.read = _http_read;
    } else if (config->type == AUDIO_STREAM_WRITER) {
        cfg.write = _http_write;
    }

    el = audio_element_init(&cfg);
    AUDIO_MEM_CHECK(TAG, el, {
        audio_free(http);
        return NULL;
    });
    audio_element_setdata(el, http);
    return el;
}

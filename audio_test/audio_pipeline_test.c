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


#include "audio_pipeline.h"
#include "esp_log.h"
#include "esp_err.h"
#include "audio_test.h"

static const char *TAG = "AUDIO_PIPELINE_TEST";

static esp_err_t _el_open(audio_element_handle_t self)
{
    ESP_LOGI(TAG, "[%s] _el_open", audio_element_get_tag(self));
    return ESP_OK;
}

static int _el_read(audio_element_handle_t self, char *buffer, int len, TickType_t ticks_to_wait)
{
    ESP_LOGI(TAG, "[%s] _el_read", audio_element_get_tag(self));
    return len;
}

static int _el_process(audio_element_handle_t self, char *in_buffer, int in_len, char *out_buffer, int out_len)
{
    ESP_LOGI(TAG, "[%s] _el_process, in_len=%d, outlen=%d", audio_element_get_tag(self), in_len, out_len);
    usleep(10000);
    return in_len;
}

static int _el_write(audio_element_handle_t self, char *buffer, int len, TickType_t ticks_to_wait)
{
    ESP_LOGI(TAG, "[%s] _el_write", audio_element_get_tag(self));
    return len;
}

static esp_err_t _el_close(audio_element_handle_t self)
{
    ESP_LOGI(TAG, "[%s] _el_close", audio_element_get_tag(self));
    return ESP_OK;
}

void audio_pipeline_test()
{
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("AUDIO_PIPELINE", ESP_LOG_VERBOSE);
    esp_log_level_set("AUDIO_ELEMENT", ESP_LOG_VERBOSE);
    esp_log_level_set("AUDIO_EVENT", ESP_LOG_VERBOSE);

    ESP_LOGI(TAG, "[✓] init audio_pipeline and audio_element");
    audio_element_handle_t first_el, mid_el, last_el;
    audio_element_cfg_t el_cfg = DEFAULT_AUDIO_ELEMENT_CONFIG();

    el_cfg.open = _el_open;
    el_cfg.read = _el_read;
    el_cfg.process = _el_process;
    el_cfg.close = _el_close;
    first_el = audio_element_init(&el_cfg);
    assert(first_el != NULL);

    el_cfg.read = NULL;
    el_cfg.write = NULL;
    mid_el = audio_element_init(&el_cfg);
    assert(mid_el != NULL);

    el_cfg.write = _el_write;
    last_el = audio_element_init(&el_cfg);
    assert(last_el != NULL);

    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    audio_pipeline_handle_t pipeline = audio_pipeline_init(&pipeline_cfg);
    assert(pipeline != NULL);

    assert(ESP_OK == audio_pipeline_register(pipeline, first_el, "first"));
    assert(ESP_OK == audio_pipeline_register(pipeline, mid_el, "mid"));
    assert(ESP_OK == audio_pipeline_register(pipeline, last_el, "last"));

    assert(ESP_OK == audio_pipeline_link(pipeline, (const char *[]){"first", "mid", "last"}, 3));

    assert(ESP_OK == audio_pipeline_run(pipeline));

    usleep(1000000);
    assert(ESP_OK == audio_pipeline_stop(pipeline));
    assert(ESP_OK == audio_pipeline_wait_for_stop(pipeline));
    assert(ESP_OK == audio_pipeline_unlink(pipeline));
    assert(ESP_OK == audio_pipeline_run(pipeline));
    usleep(500000);
    assert(ESP_OK == audio_pipeline_stop(pipeline));
    assert(ESP_OK == audio_pipeline_wait_for_stop(pipeline));
    assert(ESP_OK == audio_pipeline_deinit(pipeline));
    // ERROR:audio_element_deinit is called again.
    // assert(ESP_OK == audio_element_deinit(first_el));
    // assert(ESP_OK == audio_element_deinit(mid_el));
    // assert(ESP_OK == audio_element_deinit(last_el));

}

/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2019 <ESPRESSIF SYSTEMS (SHANGHAI) CO., LTD>
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "esp_http_client.h"
#include "audio_test.h"
#include "esp_log.h"
#include "audio_pipeline.h"
#include "audio_mem.h"
#include "audio_element.h"
#include "audio_event_iface.h"
#include "http_stream.h"
#include "fatfs_stream.h"
#include "mp3_decoder.h"
#include "auto_mp3_dec.h"
#include "pcm_stream.h"


static const char *TAG = "HTTP STREAM UNITEST";

static const char URL_RANDOM[] = "0123456789abcdefghijklmnopqrstuvwxyuzABCDEFGHIJKLMNOPQRSTUVWXYUZ-_.!@#$&*()=:/,;?+~";
#define AAC_STREAM_URI "http://open.ls.qingting.fm/live/274/64k.m3u8?format=aac"
#define UNITEST_HTTP_SERVRE_URI  "http://192.168.199.168:8000/upload" 

#define TEST_HTTP_READER "http://downsc.chinaz.net/Files/DownLoad/sound1/201906/11582.mp3?WebShieldDRSessionVerify=bLNWllmAVhBsdx3h5vid"

void http_stream_init_memory()
{
    esp_log_level_set("AUDIO_ELEMENT", ESP_LOG_DEBUG);
    audio_element_handle_t http_stream_reader;
    http_stream_cfg_t http_cfg = HTTP_STREAM_CFG_DEFAULT();
    http_cfg.type = AUDIO_STREAM_READER;
    int cnt = 2;
    AUDIO_MEM_SHOW("BEFORE HTTP_STREAM_INIT MEMORY TEST");
    while (cnt--) {
        http_stream_reader = http_stream_init(&http_cfg);
        audio_element_deinit(http_stream_reader);
    }
    AUDIO_MEM_SHOW("AFTER HTTP_STREAM_INIT MEMORY TEST");
}



void http_stream_read_test()
{
    audio_pipeline_handle_t pipeline;
    audio_element_handle_t http_stream_reader, fatfs_stream_writer;

    esp_log_level_set("AUDIO_PIPELINE", ESP_LOG_DEBUG);
    esp_log_level_set("AUDIO_ELEMENT", ESP_LOG_DEBUG);

    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline = audio_pipeline_init(&pipeline_cfg);
    TEST_ASSERT_NOT_NULL(pipeline);

    fatfs_stream_cfg_t fatfs_cfg = FATFS_STREAM_CFG_DEFAULT();
    fatfs_cfg.type = AUDIO_STREAM_WRITER;
    fatfs_stream_writer = fatfs_stream_init(&fatfs_cfg);
    TEST_ASSERT_NOT_NULL(fatfs_stream_writer);
    TEST_ASSERT_EQUAL(ESP_OK, audio_element_set_uri(fatfs_stream_writer, "/home/user/code/pipeline_audio/mytest.mp3"));

    http_stream_cfg_t http_cfg = HTTP_STREAM_CFG_DEFAULT();
    http_cfg.type = AUDIO_STREAM_READER;
    http_stream_reader = http_stream_init(&http_cfg);
    TEST_ASSERT_NOT_NULL(http_stream_reader);

    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_register(pipeline, http_stream_reader, "http"));
    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_register(pipeline, fatfs_stream_writer,  "fatfs"));

    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_link(pipeline, (const char *[]) { "http", "fatfs" }, 2));

    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    audio_event_iface_handle_t evt = audio_event_iface_init(&evt_cfg);

    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_set_listener(pipeline, evt));


    TEST_ASSERT_EQUAL(ESP_OK, audio_element_set_uri(http_stream_reader, AAC_STREAM_URI));
    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_run(pipeline));

    // while (1) {
    //     audio_event_iface_msg_t msg;
    //     esp_err_t ret = audio_event_iface_listen(evt, &msg, portMAX_DELAY);
    //     if (ret != ESP_OK) {
    //         ESP_LOGE(TAG, "[ * ] Event interface error : %d", ret);
    //         continue;
    //     }

    //   if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT && msg.source == (void *) http_stream_reader
    //         && msg.cmd == AEL_MSG_CMD_REPORT_STATUS && (int) msg.data == AEL_STATUS_ERROR_OPEN) {
    //             break;
    //         continue;
    //     }
    // }
sleep(10);
    
    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_terminate(pipeline));
    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_unregister(pipeline, http_stream_reader));
    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_unregister(pipeline, fatfs_stream_writer));
    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_remove_listener(pipeline));
    TEST_ASSERT_EQUAL(ESP_OK, audio_event_iface_destroy(evt));
    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_deinit(pipeline));
    TEST_ASSERT_EQUAL(ESP_OK, audio_element_deinit(http_stream_reader));
    TEST_ASSERT_EQUAL(ESP_OK, audio_element_deinit(fatfs_stream_writer));
}

void http_stream_play_url_test()
{
    audio_pipeline_handle_t pipeline;
    audio_element_handle_t http_stream_reader, mp3_decoder, pcm_stream_writer;
    esp_log_level_set("AUDIO_PIPELINE", ESP_LOG_DEBUG);
    esp_log_level_set("AUDIO_ELEMENT", ESP_LOG_DEBUG);

    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline = audio_pipeline_init(&pipeline_cfg);
    TEST_ASSERT_NOT_NULL(pipeline);

    http_stream_cfg_t http_cfg = HTTP_STREAM_CFG_DEFAULT();
    http_cfg.type = AUDIO_STREAM_READER;
    http_stream_reader = http_stream_init(&http_cfg);
    TEST_ASSERT_NOT_NULL(http_stream_reader);

    mp3_decoder_cfg_t mp3_cfg = DEFAULT_MP3_DECODER_CONFIG(); 
    mp3_decoder = mp3_decoder_init(&mp3_cfg);

    pcm_stream_cfg_t pcm_writer_cfg = PCM_STREAM_CFG_DEFAULT();
    pcm_writer_cfg.type = AUDIO_STREAM_WRITER;
    pcm_stream_writer = pcm_stream_init(&pcm_writer_cfg);
    TEST_ASSERT_NOT_NULL(pcm_stream_writer);

    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_register(pipeline, http_stream_reader, "http_reader"));
    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_register(pipeline, mp3_decoder, "mp3_decoder"));
    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_register(pipeline, pcm_stream_writer, "pcm_writer"));

    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_link(pipeline, (const char *[]) {"http_reader", "mp3_decoder", "pcm_writer"}, 3));

    TEST_ASSERT_EQUAL(ESP_OK, audio_element_set_uri(http_stream_reader, TEST_HTTP_READER));

    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    evt_cfg.oflags = O_RDWR | O_CREAT;
    audio_event_iface_handle_t evt = audio_event_iface_init(&evt_cfg);

    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_set_listener(pipeline, evt));

    // TEST_ASSERT_EQUAL(ESP_OK, audio_event_iface_set_listener(esp_periph_set_get_event_iface(set), evt));

    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_run(pipeline));

    // while (1) {
    //     // int volume = 41, volume_get = 0;
    //     // pcm_volume_set(pcm_stream_writer, volume);
    //     // pcm_volume_get(pcm_stream_writer, &volume_get);
    //     // printf("volume_get **************************** %d\n", volume_get);
    //     audio_event_iface_msg_t msg;
    //     esp_err_t ret = audio_event_iface_listen(evt, &msg, portMAX_DELAY);
    //     if (ret != ESP_OK) {
    //         ESP_LOGE(TAG, "[ * ] Event interface error : %d", ret);
    //         continue;
    //     }

    //     if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT && msg.source == (void *) pcm_stream_writer
    //         && msg.cmd == AEL_MSG_CMD_REPORT_STATUS
    //         && (((int)msg.data == AEL_STATUS_STATE_STOPPED) || ((int)msg.data == AEL_STATUS_STATE_FINISHED))) {
    //         ESP_LOGW(TAG, "[ * ] Stop event received");
    //         break;
    //     }
    //     // sleep(2);
    //     // volume = 65;
    //     // pcm_volume_set(pcm_stream_writer, volume);
    //     // pcm_volume_get(pcm_stream_writer, &volume_get);
    //     // printf("volume_get **************************** %d\n", volume_get);
    //     // sleep(2);
    // }
while(1);
    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_terminate(pipeline));
    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_unregister(pipeline, http_stream_reader));
    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_unregister(pipeline, mp3_decoder));
    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_unregister(pipeline, pcm_stream_writer));
    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_remove_listener(pipeline));

    TEST_ASSERT_EQUAL(ESP_OK, audio_event_iface_destroy(evt));
    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_deinit(pipeline));
    TEST_ASSERT_EQUAL(ESP_OK, audio_element_deinit(http_stream_reader));
    TEST_ASSERT_EQUAL(ESP_OK, audio_element_deinit(mp3_decoder));
    TEST_ASSERT_EQUAL(ESP_OK, audio_element_deinit(pcm_stream_writer));

}

void http_stream_test(void)
{
    //http_stream_init_memory();
    http_stream_read_test();
    //http_stream_play_url_test();
}
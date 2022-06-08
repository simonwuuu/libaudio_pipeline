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

#include "esp_err.h"
#include "esp_log.h"

#include "audio_pipeline.h"
#include "audio_mem.h"
#include "fatfs_stream.h"
#include "audio_test.h"
#include "mp3_decoder.h"
#include "auto_mp3_dec.h"
#include "pcm_stream.h"

static const char *TAG = "PCM_STREAM_TEST";

#define TEST_FATFS_READER  "/home/user/code/pipeline_audio/test.mp3"
#define TEST_FATFS_WRITER  "/home/user/code/pipeline_audio/pcmin.wav"

void pcm_write_stream_loop()
{
    audio_pipeline_handle_t pipeline;
    audio_element_handle_t fatfs_stream_reader, mp3_decoder, pcm_stream_writer;
    esp_log_level_set("AUDIO_PIPELINE", ESP_LOG_DEBUG);
    esp_log_level_set("AUDIO_ELEMENT", ESP_LOG_DEBUG);

    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline = audio_pipeline_init(&pipeline_cfg);
    TEST_ASSERT_NOT_NULL(pipeline);

    fatfs_stream_cfg_t fatfs_reader_cfg = FATFS_STREAM_CFG_DEFAULT();
    fatfs_reader_cfg.type = AUDIO_STREAM_READER;
    fatfs_stream_reader = fatfs_stream_init(&fatfs_reader_cfg);
    TEST_ASSERT_NOT_NULL(fatfs_stream_reader);

    mp3_decoder_cfg_t mp3_cfg = DEFAULT_MP3_DECODER_CONFIG(); 
    mp3_decoder = mp3_decoder_init(&mp3_cfg);

    pcm_stream_cfg_t pcm_writer_cfg = PCM_STREAM_CFG_DEFAULT();
    pcm_writer_cfg.type = AUDIO_STREAM_WRITER;
    pcm_stream_writer = pcm_stream_init(&pcm_writer_cfg);
    TEST_ASSERT_NOT_NULL(pcm_stream_writer);

    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_register(pipeline, fatfs_stream_reader, "file_reader"));
    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_register(pipeline, mp3_decoder, "mp3_decoder"));
    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_register(pipeline, pcm_stream_writer, "pcm_writer"));

    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_link(pipeline, (const char *[]) {"file_reader", "mp3_decoder", "pcm_writer"}, 3));

    TEST_ASSERT_EQUAL(ESP_OK, audio_element_set_uri(fatfs_stream_reader, TEST_FATFS_READER));

    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    evt_cfg.oflags = O_RDWR | O_CREAT;
    audio_event_iface_handle_t evt = audio_event_iface_init(&evt_cfg);

    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_set_listener(pipeline, evt));

    // TEST_ASSERT_EQUAL(ESP_OK, audio_event_iface_set_listener(esp_periph_set_get_event_iface(set), evt));

    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_run(pipeline));

    while (1) {
        // int volume = 41, volume_get = 0;
        // pcm_volume_set(pcm_stream_writer, volume);
        // pcm_volume_get(pcm_stream_writer, &volume_get);
        // printf("volume_get **************************** %d\n", volume_get);
        audio_event_iface_msg_t msg;
        esp_err_t ret = audio_event_iface_listen(evt, &msg, portMAX_DELAY);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "[ * ] Event interface error : %d", ret);
            continue;
        }

        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT && msg.source == (void *) pcm_stream_writer
            && msg.cmd == AEL_MSG_CMD_REPORT_STATUS
            && (((int)msg.data == AEL_STATUS_STATE_STOPPED) || ((int)msg.data == AEL_STATUS_STATE_FINISHED))) {
            ESP_LOGW(TAG, "[ * ] Stop event received");
            break;
        }
        // sleep(2);
        // volume = 65;
        // pcm_volume_set(pcm_stream_writer, volume);
        // pcm_volume_get(pcm_stream_writer, &volume_get);
        // printf("volume_get **************************** %d\n", volume_get);
        // sleep(2);
    }

    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_terminate(pipeline));
    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_unregister(pipeline, fatfs_stream_reader));
    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_unregister(pipeline, mp3_decoder));
    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_unregister(pipeline, pcm_stream_writer));
    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_remove_listener(pipeline));

    TEST_ASSERT_EQUAL(ESP_OK, audio_event_iface_destroy(evt));
    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_deinit(pipeline));
    TEST_ASSERT_EQUAL(ESP_OK, audio_element_deinit(fatfs_stream_reader));
    TEST_ASSERT_EQUAL(ESP_OK, audio_element_deinit(mp3_decoder));
    TEST_ASSERT_EQUAL(ESP_OK, audio_element_deinit(pcm_stream_writer));

}

void fatfs_init_memory()
{
    esp_log_level_set("AUDIO_ELEMENT", ESP_LOG_DEBUG);
    audio_element_handle_t fatfs_stream_reader;
    fatfs_stream_cfg_t fatfs_cfg = FATFS_STREAM_CFG_DEFAULT();
    fatfs_cfg.type = AUDIO_STREAM_READER;
    int cnt = 2000;
    AUDIO_MEM_SHOW("BEFORE FATFS_STREAM_INIT MEMORY TEST");
    while (cnt--) {
        fatfs_stream_reader = fatfs_stream_init(&fatfs_cfg);
        audio_element_deinit(fatfs_stream_reader);
    }
    AUDIO_MEM_SHOW("AFTER FATFS_STREAM_INIT MEMORY TEST");
}

void pcm_read_stream_loop() {
    audio_pipeline_handle_t pipeline;
    audio_element_handle_t pcm_stream_reader, fatfs_stream_writer;
    esp_log_level_set("AUDIO_PIPELINE", ESP_LOG_DEBUG);
    esp_log_level_set("AUDIO_ELEMENT", ESP_LOG_DEBUG);

    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline = audio_pipeline_init(&pipeline_cfg);
    TEST_ASSERT_NOT_NULL(pipeline);

    pcm_stream_cfg_t pcm_reader_cfg = PCM_STREAM_CFG_DEFAULT();
    pcm_reader_cfg.type = AUDIO_STREAM_READER;
    pcm_stream_reader = pcm_stream_init(&pcm_reader_cfg);
    TEST_ASSERT_NOT_NULL(pcm_stream_reader);

    fatfs_stream_cfg_t fatfs_writer_cfg = FATFS_STREAM_CFG_DEFAULT();
    fatfs_writer_cfg.type = AUDIO_STREAM_WRITER;
    fatfs_stream_writer = fatfs_stream_init(&fatfs_writer_cfg);
    TEST_ASSERT_NOT_NULL(fatfs_stream_writer);
    
    audio_element_info_t info;
    audio_element_getinfo(pcm_stream_reader, &info);
    audio_element_setinfo(fatfs_stream_writer, &info);


    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_register(pipeline, pcm_stream_reader, "pcm_reader"));
    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_register(pipeline, fatfs_stream_writer, "file_writer"));

    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_link(pipeline, (const char *[]) {"pcm_reader", "file_writer"}, 2));

    TEST_ASSERT_EQUAL(ESP_OK, audio_element_set_uri(fatfs_stream_writer, TEST_FATFS_WRITER));

    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    evt_cfg.oflags = O_RDWR | O_CREAT;
    audio_event_iface_handle_t evt = audio_event_iface_init(&evt_cfg);

    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_set_listener(pipeline, evt));

    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_run(pipeline));

    audio_event_iface_msg_t msg;
    esp_err_t ret = audio_event_iface_listen(evt, &msg, portMAX_DELAY);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "[ * ] Event interface error : %d", ret);
    }

    sleep(5);
    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_stop(pipeline));

    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_terminate(pipeline));
    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_unregister(pipeline, pcm_stream_reader));
    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_unregister(pipeline, fatfs_stream_writer));
    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_remove_listener(pipeline));
    // TEST_ASSERT_EQUAL(ESP_OK, esp_periph_set_stop_all(set));
    // TEST_ASSERT_EQUAL(ESP_OK, audio_event_iface_remove_listener(esp_periph_set_get_event_iface(set), evt));
    TEST_ASSERT_EQUAL(ESP_OK, audio_event_iface_destroy(evt));
    TEST_ASSERT_EQUAL(ESP_OK, audio_pipeline_deinit(pipeline));
    TEST_ASSERT_EQUAL(ESP_OK, audio_element_deinit(pcm_stream_reader));
    TEST_ASSERT_EQUAL(ESP_OK, audio_element_deinit(fatfs_stream_writer));
    // TEST_ASSERT_EQUAL(ESP_OK, esp_periph_set_destroy(set));
}

void pcm_stream_test()
{
    // fatfs_init_memory();
    pcm_read_stream_loop();
    // pcm_write_stream_loop();

}

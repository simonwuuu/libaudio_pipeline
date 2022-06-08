#include <assert.h>
#include <pthread.h>

#include "audio_mem.h"
#include "audio_test.h"
#include "esp_log.h"
#include "esp_err.h"

static const char* TAG = "AUDIO_MEM_TEST";


void audio_mem()
{
    esp_log_level_set("AUDIO_MEM", ESP_LOG_VERBOSE);

    ESP_LOGI(TAG, "[✓] audio_malloc,audio_callo,audio_calloc_inner, audio_inaudio memory print");
    AUDIO_MEM_SHOW(TAG);
    uint8_t* pdata = audio_malloc(1024);
    assert(pdata != NULL);
    AUDIO_MEM_SHOW(TAG);
    audio_free(pdata);

    AUDIO_MEM_SHOW(TAG);
    pdata = audio_calloc(1, 2 * 1024 * 1024);
    assert(pdata != NULL);
    AUDIO_MEM_SHOW(TAG);
    audio_free(pdata);

    AUDIO_MEM_SHOW(TAG);
    pdata = audio_calloc_inner(1, 1024);
    // assert(pdata != NULL);
    AUDIO_MEM_SHOW(TAG);
    audio_free(pdata);
    AUDIO_MEM_SHOW(TAG);
}

void audio_strdup_test()
{
    esp_log_level_set("AUDIO_STRDUP", ESP_LOG_VERBOSE);
    ESP_LOGI(TAG, "[✓] audio_strdup");
    AUDIO_MEM_SHOW(TAG);
    char * strings = "audio string dump";
    char* pdata = audio_strdup(strings);
    assert(pdata != NULL);
    AUDIO_MEM_SHOW(TAG);
    audio_free(pdata);
    AUDIO_MEM_SHOW(TAG);
}

void audio_realloc_test()
{
    esp_log_level_set("AUDIO_REALLOC", ESP_LOG_VERBOSE);
    ESP_LOGI(TAG, "[✓] audio_realloc");
    AUDIO_MEM_SHOW(TAG);
    uint8_t* pdata = audio_malloc(1024);
    assert(pdata != NULL);
    AUDIO_MEM_SHOW(TAG);
    pdata = audio_realloc(pdata, 2 * 1024);
    assert(pdata != NULL);
    AUDIO_MEM_SHOW(TAG);
    audio_free(pdata);
    AUDIO_MEM_SHOW(TAG);
}


void audio_mem_test() {
    audio_mem();
    audio_strdup_test();
    audio_realloc_test();
}

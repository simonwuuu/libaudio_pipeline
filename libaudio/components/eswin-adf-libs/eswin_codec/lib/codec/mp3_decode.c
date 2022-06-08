#include "auto_mp3_dec.h"
#include "audio_element.h"
#include "mp3dec.h"
#include "ringbuf.h"
#include "mp3_decoder.h"
#include "coder.h"

#include "esp_err.h"
#include "esp_log.h"

static const char *TAG = "MP3_DECODE";

typedef struct mp3_decoder
{
    HMP3Decoder Mp3Dec_ptr;
    MP3FrameInfo Mp3FrameInfo;
    short output[2304];
    //mp3_file_type mp3_type;
    bool is_open;
} mp3_decoder_t;

audio_element_handle_t mp3_decoder_init(mp3_decoder_cfg_t *config)
{
    audio_element_cfg_t cfg = DEFAULT_AUDIO_ELEMENT_CONFIG();
    audio_element_handle_t el;
    cfg.open = mp3_decoder_open;
    cfg.close = mp3_decoder_close;
    cfg.process = mp3_decoder_process;
    cfg.seek = mp3_decoder_get_pos;
    cfg.task_stack = config->task_stack;
    cfg.task_prio = config->task_prio;
    cfg.task_core = config->task_core;
    cfg.out_rb_size = config->out_rb_size;
    cfg.stack_in_ext = config->stack_in_ext;
    cfg.tag = "mp3";

    HMP3Decoder Mp3Dec_ptr = MP3InitDecoder();
    AUDIO_MEM_CHECK(TAG, Mp3Dec_ptr, return NULL);

    mp3_decoder_t *mp3Decder = audio_calloc(1, sizeof(mp3_decoder_t));
    AUDIO_MEM_CHECK(TAG, mp3Decder, return NULL);

    mp3Decder->Mp3Dec_ptr = Mp3Dec_ptr;

    el = audio_element_init(&cfg);
    AUDIO_MEM_CHECK(TAG, el, goto _mp3_decoder_init_exit);
    audio_element_setdata(el, mp3Decder);

    return el;
_mp3_decoder_init_exit:
    audio_free(mp3Decder->Mp3Dec_ptr);
    audio_free(mp3Decder);
    return NULL;
}

esp_err_t mp3_decoder_get_pos(audio_element_handle_t self, void *in_data, int in_size, void *out_data, int *out_size)
{
    //mp3_decoder_t *mp3Decder = (mp3_decoder_t *)audio_element_getdata(self);

    //mp3Decder->mp3_path=mp3Path;
    return ESP_OK;
}

esp_err_t mp3_decoder_open(audio_element_handle_t el)
{

    mp3_decoder_t *mp3Decder = (mp3_decoder_t *)audio_element_getdata(el);

    if (mp3Decder->is_open == true)
    {
        ESP_LOGE(TAG, "already opened");
        return ESP_FAIL;
    }
    mp3Decder->is_open = true;
    ESP_LOGI(TAG, "open down");
    return ESP_OK;
}

esp_err_t mp3_decoder_close(audio_element_handle_t el)
{
    mp3_decoder_t *mp3Decder = (mp3_decoder_t *)audio_element_getdata(el);

    if (mp3Decder->is_open)
    {
        mp3Decder->is_open = false;
    }
    if (AEL_STATE_PAUSED != audio_element_get_state(el))
    {
        audio_element_report_info(el);
        audio_element_set_byte_pos(el, 0);
    }
    ESP_LOGI(TAG, "close down");

    return ESP_OK;
}

esp_codec_err_t mp3_decoder_process(audio_element_handle_t el, char *in_buffer, int in_len)
{
    static int last_left;
    int w_size = 0;
    int r_size = audio_element_input(el, in_buffer + last_left, in_len - last_left);
    int framesize;

    if (r_size == AEL_IO_TIMEOUT)
    {
        {
            memset(in_buffer, 0x00, in_len);
        }
        r_size = in_len;
        w_size = audio_element_output(el, in_buffer, r_size);
    }
    else if (r_size > 0)
    {
        mp3_decoder_t *mp3Decder = (mp3_decoder_t *)audio_element_getdata(el);
        HMP3Decoder Mp3Decoder = mp3Decder->Mp3Dec_ptr;
        MP3FrameInfo *Mp3FrameInfo = &mp3Decder->Mp3FrameInfo;
        audio_element_info_t info;
        audio_element_getinfo(el, &info);
        if(info.reserve_data.user_data_0>200)
        {
            framesize = info.reserve_data.user_data_0;
        }
        else{
            framesize =512;
        }

        // int framesize = info.reserve_data.user_data_0;
        printf("framesize=%d\n", framesize);

        int decoder_err = 0;
        int pcm_num_per_frame = 0;
        int offset = 0;
        int left = last_left + r_size;
        char *readPtr = in_buffer;
        short *output = mp3Decder->output;

        while (1)
        {
            offset = MP3FindSyncWord(readPtr, left);
            if ((offset < 0) || (left - offset) < framesize)
            { // not find sync
                ESP_LOGI(TAG, "no found sync!");
                memmove(in_buffer, readPtr, left);
                last_left = left;
                //printf("process down\n");
                return ESP_CODEC_ERR_CONTINUE;
            }

            readPtr += offset;
            left = left - offset;

            printf("after findSyncword left=%d\n", left);

            decoder_err = MP3Decode(Mp3Decoder, &readPtr, &left, output, 0);
            if (decoder_err != 0)
            {
                ESP_LOGE(TAG, "MP3Decode Failed!");
                return ESP_CODEC_ERR_FAIL;
            }

            MP3GetLastFrameInfo(Mp3Decoder, Mp3FrameInfo);
            pcm_num_per_frame = Mp3FrameInfo->outputSamps;

            if(info.bits != Mp3FrameInfo->bitsPerSample || info.channels != Mp3FrameInfo->nChans  || \
                info.sample_rates != Mp3FrameInfo->samprate || info.bps != Mp3FrameInfo->bitrate  || \
                info.codec_fmt != ESP_CODEC_TYPE_MP3  ||  info.reserve_data.user_data_0 != (int)144*Mp3FrameInfo->bitrate/Mp3FrameInfo->samprate+1)
            {
                info.bits = Mp3FrameInfo->bitsPerSample;
                info.channels = Mp3FrameInfo->nChans;
                info.sample_rates = Mp3FrameInfo->samprate;
                info.bps = Mp3FrameInfo->bitrate;
                info.codec_fmt = ESP_CODEC_TYPE_MP3;
                info.reserve_data.user_data_0 = (int)144*Mp3FrameInfo->bitrate/Mp3FrameInfo->samprate+1;
                audio_element_setinfo(el, &info);
                audio_element_report_info(el);
            }
            // printf(" \r\n Bitrate       %dKbps", Mp3FrameInfo->bitrate / 1000);
            // printf(" \r\n Samprate      %dHz", Mp3FrameInfo->samprate);
            // printf(" \r\n BitsPerSample %db", Mp3FrameInfo->bitsPerSample);
            // printf(" \r\n nChans        %d", Mp3FrameInfo->nChans);
            // printf(" \r\n Layer         %d", Mp3FrameInfo->layer);
            // printf(" \r\n Version       %d", Mp3FrameInfo->version);
            // printf(" \r\n OutputSamps   %d", Mp3FrameInfo->outputSamps);
            // printf(" \r\n ");


            if (pcm_num_per_frame > 0)
            {
                if (Mp3FrameInfo->nChans == 1){
                    for(int i = pcm_num_per_frame - 1; i >= 0; i--){
                        mp3Decder->output[i * 2] = mp3Decder->output[i];
                        mp3Decder->output[i * 2 + 1] = mp3Decder->output[i];
                    }
                    pcm_num_per_frame =pcm_num_per_frame * 2;
                }

                pcm_num_per_frame = pcm_num_per_frame * sizeof(short);
                w_size = audio_element_output(el, output, pcm_num_per_frame);
                if (w_size != pcm_num_per_frame)
                {
                    ESP_LOGE(TAG, "audio_element_output Failed!");
                    break;
                }
            }
        }
    }
    else if ((r_size == AEL_IO_DONE) && last_left > 0) {
        mp3_decoder_t *mp3Decder = (mp3_decoder_t *)audio_element_getdata(el);
        HMP3Decoder Mp3Decoder = mp3Decder->Mp3Dec_ptr;
        MP3FrameInfo *Mp3FrameInfo = &mp3Decder->Mp3FrameInfo;
        audio_element_info_t info;
        audio_element_getinfo(el, &info);

        int decoder_err = 0;
        int pcm_num_per_frame = 0;
        int offset = 0;
        int left = last_left;
        char *readPtr = in_buffer;
        short *output = mp3Decder->output;

        while (1)
        {
            offset = MP3FindSyncWord(readPtr, left);
            readPtr += offset;
            left = left - offset;

            printf("after findSyncword left=%d\n", left);

            decoder_err = MP3Decode(Mp3Decoder, &readPtr, &left, output, 0);
            if (decoder_err != 0)
            {
                ESP_LOGW(TAG, "left: %d", left);
                ESP_LOGW(TAG, "MP3Decode Failed!");
                last_left = left;
                return ESP_CODEC_ERR_DONE;
            }

            MP3GetLastFrameInfo(Mp3Decoder, Mp3FrameInfo);
            pcm_num_per_frame = Mp3FrameInfo->outputSamps;

            if(info.bits != Mp3FrameInfo->bitsPerSample || info.channels != Mp3FrameInfo->nChans  || \
                info.sample_rates != Mp3FrameInfo->samprate || info.bps != Mp3FrameInfo->bitrate  || \
                info.codec_fmt != ESP_CODEC_TYPE_MP3  ||  info.reserve_data.user_data_0 != (int)144*Mp3FrameInfo->bitrate/Mp3FrameInfo->samprate+1)
            {
                info.bits = Mp3FrameInfo->bitsPerSample;
                info.channels = Mp3FrameInfo->nChans;
                info.sample_rates = Mp3FrameInfo->samprate;
                info.bps = Mp3FrameInfo->bitrate;
                info.codec_fmt = ESP_CODEC_TYPE_MP3;
                info.reserve_data.user_data_0 = (int)144*Mp3FrameInfo->bitrate/Mp3FrameInfo->samprate+1;
                audio_element_setinfo(el, &info);
                audio_element_report_info(el);
            }

            if (pcm_num_per_frame > 0)
            {
                if (Mp3FrameInfo->nChans == 1){
                    for(int i = pcm_num_per_frame - 1; i >= 0; i--){
                        mp3Decder->output[i * 2] = mp3Decder->output[i];
                        mp3Decder->output[i * 2 + 1] = mp3Decder->output[i];
                    }
                    pcm_num_per_frame =pcm_num_per_frame * 2;
                }

                pcm_num_per_frame = pcm_num_per_frame * sizeof(short);
                w_size = audio_element_output(el, output, pcm_num_per_frame);
                if (w_size != pcm_num_per_frame)
                {
                    ESP_LOGE(TAG, "audio_element_output Failed!");
                    break;
                }
            }
        }
    }
    else {
        w_size = r_size;
    }
    return w_size;
}

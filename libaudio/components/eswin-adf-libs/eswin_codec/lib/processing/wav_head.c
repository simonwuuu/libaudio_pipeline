#include <tinyalsa/asoundlib.h>
#include "wav_head.h"
#include "audio_error.h"
#include "esp_log.h"
static const char * TAG = "WAV_HEAD";

#define WAVE_FORMAT_PCM 0x0001
#define WAVE_FORMAT_IEEE_FLOAT 0x0003

esp_err_t wav_check_type(uint8_t *in_data, int len)
{
  AUDIO_UNIMPLEMENTED(TAG, "wav_check_type not implemented");
  return ESP_FAIL;
}


void wav_head_init(wav_header_t *wavhead, int sample_rate, int bits, int channels)
{
  enum pcm_format format;
  chunk_riff_t *riff = &wavhead->riff;
  chunk_fmt_t *fmt = &wavhead->fmt;
  chunk_data_t *data = &wavhead->data;

  riff->chunk_id = CHUNKID_RIFF;
  riff->chunk_size = 0;
  riff->format = CHUNKID_WAVE;
  
  fmt->chunk_id= CHUNKID_FMT;
  fmt->chunk_size = 16;
  fmt->audio_format = WAVE_FORMAT_PCM;
  fmt->num_of_channels = channels;
  fmt->samplerate = sample_rate;

  switch (bits) {
    case 32:
        format = PCM_FORMAT_S32_LE;
        break;
    case 24:
        format = PCM_FORMAT_S24_LE;
        break;
    case 16:
        format = PCM_FORMAT_S16_LE;
        break;
    default:
        ESP_LOGE(TAG, "%u bits is not supported.", bits);
        return 1;
    }

  fmt->bits_per_sample =  pcm_format_to_bits(format);
  fmt->byterate = (fmt->bits_per_sample / 8) *  channels * sample_rate;
  fmt->block_align = channels * (fmt->bits_per_sample / 8);

  data->chunk_id = CHUNKID_DATA;
  data->chunk_size = 0;
}

void wav_adpcm_head_init(wav_adpcm_header_t *wavhead, int sample_rate, int bits, int channels)
{
  AUDIO_UNIMPLEMENTED(TAG, "wav_adpcm_head_init not implemented");
}

esp_err_t wav_parse_header(void *codec_data, uint8_t *inData, int len, wav_info_t *info)
{
  AUDIO_UNIMPLEMENTED(TAG, "wav_parse_header not implemented");
  return ESP_FAIL;
}

void wav_head_size(wav_header_t *wavhead, uint32_t dataSize)
{
  chunk_riff_t *riff = &wavhead->riff;
  chunk_data_t *data = &wavhead->data;

  data->chunk_size = dataSize;
  riff->chunk_size = dataSize + sizeof(wav_header_t) - 8;
}

void wav_adpcm_head_size(wav_adpcm_header_t *wavhead, uint32_t data_size)
{
  AUDIO_UNIMPLEMENTED(TAG, "wav_adpcm_head_size not implemented");
}
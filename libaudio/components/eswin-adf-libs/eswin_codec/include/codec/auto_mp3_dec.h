// Copyright 2018 Espressif Systems (Shanghai) PTE LTD
// All rights reserved.

#ifndef _AUTO_MP3_DEC_H_
#define _AUTO_MP3_DEC_H_

#include "audio_type_def.h"
#include <inttypes.h>
#include <stdint.h>
#include "mp3_decoder.h"

#ifdef __cplusplus
extern "C" {
#endif

enum
{
	STA_IDLE = 0,
	STA_PLAYING,
	STA_ERR,
};

typedef struct
{
	uint8_t ucVolume;
	uint8_t ucStatus;
	uint32_t ucFreq;
} MP3_TYPE;

/**
 * @brief      Open an Audio Element type data
 *
 * @param      el   The audio element handle
 *
 * @return
 *             ESP_OK
 *             ESP_FAIL
 */
esp_err_t mp3_decoder_open(audio_element_handle_t el);

/**
 * @brief      Close an Audio Element type data
 *
 * @param      el   The audio element handle
 *
 * @return
 *             ESP_OK
 *             ESP_FAIL
 */
 esp_err_t mp3_decoder_close(audio_element_handle_t el);

/**
 * @brief      Do music data to decode
 *
 * @param      el   The audio element handle
 *
 * @return
 *             ESP_OK
 *             ESP_FAIL
 */
esp_codec_err_t mp3_decoder_process(audio_element_handle_t el, char *in_buffer, int in_len);

/**
 * @brief      Seek postion of music data to decode
 *
 * @param      self         The audio element handle
 * @param      in_data      The parameter required by callback function format. Not used by this function.
 * @param      in_size      The parameter required by callback function format. Not used by this function.
 * @param      out_data     The position of music data
 * @param      out_size     the size of `outdata`
 *
 * @return
 *             ESP_OK
 *             ESP_FAIL
 */
esp_err_t mp3_decoder_get_pos(audio_element_handle_t self, void *in_data, int in_size, void *out_data, int *out_size);

esp_err_t mp3_destroy(audio_element_handle_t self);

#ifdef __cplusplus
}
#endif

#endif

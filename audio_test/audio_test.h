#ifndef __APPS_TESTING_OSTEST_OSTEST_H
#define __APPS_TESTING_OSTEST_OSTEST_H

/****************************************************************************
 * Included Files
 ****************************************************************************/
#include <assert.h>
#define STACKSIZE 8192

#define TEST_ASSERT_NOT_NULL(pointer)  assert((pointer) != NULL)
#define TEST_ASSERT_EQUAL(left, right)  assert((left) == (right))
// Add test function declaration

void audio_mem_test(void);

void audio_queue_test(void);

void audio_event_iface_test(void);

void audio_mutex_test(void);

void audio_thread_test(void);

void audio_element_test(void);

void audio_pipeline_test(void);

void fatfs_stream_test(void);

void mp3_decoder_test(void);

void pcm_stream_test(void);

void http_stream_test(void);

#endif /* __APPS_TESTING_OSTEST_OSTEST_H */
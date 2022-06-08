
#include <sys/wait.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sched.h>
#include <errno.h>



#include "audio_test.h"
/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#define HALF_SECOND_USEC 500000L

static struct mallinfo g_mmbefore;
static struct mallinfo g_mmprevious;
static struct mallinfo g_mmafter;

static void show_memory_usage(struct mallinfo *mmbefore,
                              struct mallinfo *mmafter)
{
  printf("VARIABLE  BEFORE   AFTER\n");
  printf("======== ======== ========\n");
  printf("arena    %8x %8x\n", mmbefore->arena,    mmafter->arena);
  printf("ordblks  %8d %8d\n", mmbefore->ordblks,  mmafter->ordblks);
  // printf("mxordblk %8x %8x\n", mmbefore->mxordblk, mmafter->mxordblk);
  printf("uordblks %8x %8x\n", mmbefore->uordblks, mmafter->uordblks);
  printf("fordblks %8x %8x\n", mmbefore->fordblks, mmafter->fordblks);
}

/****************************************************************************
 * Name: check_test_memory_usage
 ****************************************************************************/

static void check_test_memory_usage(void)
{
  /* Wait a little bit to let any threads terminate */

  usleep(HALF_SECOND_USEC);

  /* Get the current memory usage */

  g_mmafter = mallinfo();

  /* Show the change from the previous time */

  printf("\nEnd of test memory usage:\n");
  show_memory_usage(&g_mmprevious, &g_mmafter);

  /* Set up for the next test */

  g_mmprevious = g_mmafter;
}

int main(){
  /* Sample the memory usage now */

  usleep(HALF_SECOND_USEC);

  g_mmbefore = mallinfo();
  g_mmprevious = g_mmbefore;

  printf("\n--------------------------audio_test_main: Started!!! --------------------------\n\n");
  /* Checkout audio_mem.c */
  // printf("\n--------------------------audio_test_main: audio_mem_test() test --------------------------\n");
  // audio_mem_test();
  // check_test_memory_usage();

  // /* Checkout audio_thread.c */
  // // printf("\n--------------------------audio_test_main: audio_thread_test() test --------------------------\n");
  // // audio_thread_test();
  // // check_test_memory_usage();

  // //   /* Checkout audio_queue.c */
  // // printf("\n--------------------------audio_test_main: audio_queue_test() test --------------------------\n");
  // // audio_queue_test();
  // // check_test_memory_usage();

  // /* Checkout audio_event_iface.c */
  // printf("\n--------------------------audio_test_main:  audio_event_iface_test() test --------------------------\n");
  // audio_event_iface_test();
  // check_test_memory_usage();

  // //       /* Checkout audio_mutex_test.c */
  // // printf("\n--------------------------audio_test_main:  audio_mutex_test() test --------------------------\n");
  // // audio_mutex_test();
  // // check_test_memory_usage();

  // /* Checkout audio_mutex_test.c */
  // printf("\n--------------------------audio_test_main:  audio_element_test() test --------------------------\n");
  // audio_element_test();
  // check_test_memory_usage();

  // /* Checkout audio_mutex_test.c */
  // printf("\n--------------------------audio_test_main:  audio_pipeline_test() test --------------------------\n");
  // audio_pipeline_test();
  // check_test_memory_usage();

  // printf("\n--------------------------audio_test_main:Exiting!!! --------------------------\n\n");

  // /* Checkout fatfs_stream_test.c */
  // printf("\n--------------------------audio_test_main:  fatfs_stream_test() test --------------------------\n");
  // fatfs_stream_test();
  // check_test_memory_usage();

  // /* Checkout mp3_decoder_test.c */
  // printf("\n--------------------------audio_test_main:  fatfs_stream_test() test --------------------------\n");
  // mp3_decoder_test();
  // check_test_memory_usage();

  /* Checkout pcm_stream_test.c */
  // printf("\n--------------------------audio_test_main:  pcm_stream_test() test --------------------------\n");
  // pcm_stream_test();
  // check_test_memory_usage();

  // printf("\n--------------------------audio_test_main:Exiting!!! --------------------------\n\n");
  printf("\n--------------------------audio_test_main:  http_stream_test() test --------------------------\n");
  http_stream_test();
  check_test_memory_usage();

  printf("\n--------------------------audio_test_main:Exiting!!! --------------------------\n\n");

  return 0;
}


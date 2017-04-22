/* Tests that the highest-priority thread waiting on a semaphore
   is the first to wake up. */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "devices/timer.h"

#define thread_cnt 20

struct thread_info 
  {
    int64_t start_time;
    int tick_count;
    int64_t meta;
  };


static void load_thread (void *aux);

void 
prueba (void)
{
  struct thread_info info[thread_cnt];
  int64_t start_time;
  int i;

  start_time = timer_ticks ();
  msg ("Starting %d threads...", thread_cnt);
  for (i = 0; i < thread_cnt; i++) 
    {
      struct thread_info *ti = &info[i];
      char name[16];
      ti->start_time = start_time;
      ti->tick_count = 0;
      ti->meta = i*i*10000;

      snprintf(name, sizeof name, "Thread %d", i);
      thread_create (name, PRI_DEFAULT, load_thread, ti);

    }

  msg ("Sleeping 10 seconds to let threads run, please wait...");
  timer_sleep (10 * TIMER_FREQ);
  
  for (i = 0; i < thread_cnt; i++)
    msg ("Thread %d received %d ticks.", i, info[i].tick_count);
}

static void
load_thread (void *ti_) 
{
  struct thread_info *ti = ti_;
  int i=0;
  int64_t last_time = 0;
  int64_t sleep_time = 1 * TIMER_FREQ;
  timer_sleep (sleep_time - timer_elapsed (ti->start_time));
  while ( i < ti-> meta ) 
    {
      int64_t cur_time = timer_ticks ();
      if (cur_time != last_time)
        ti->tick_count++;
      last_time = cur_time;
      i++;
    }
	printf("Fin del thread\n");
}

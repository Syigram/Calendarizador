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
      ti->meta = i*100;

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
  while ( i < ti-> meta ) 
    {
      i++;
    }
}

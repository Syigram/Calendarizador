/* Tests that the highest-priority thread waiting on a semaphore
   is the first to wake up. */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "devices/timer.h"
#include "threads/io.h"

int thread_cnt;

struct thread_info
  {
    int64_t start_time;
    int tick_count;
    int64_t meta;
		int nice;
  };


static void load_thread_cpu (void *aux);
static void load_thread_io (void *aux);
static int getNice(void);

void
prueba (void)
{
	thread_cnt= cantidad;
  struct thread_info info[thread_cnt];
  int64_t start_time;
  int nice;
  start_time = timer_ticks ();
  thread_set_nice (-20);
  nice = -20;
  msg ("Starting %d threads...", thread_cnt);
  
	if (bounded)
	{
		for (int i = 0; i < thread_cnt; i++)
		{
			struct thread_info *ti = &info[i];
			char name[16];
			ti->start_time = start_time;
			ti->tick_count = 0;
			ti->meta = i*i*10000;
			ti->nice = nice;

		 	snprintf(name, sizeof name, "Thread %d", i);
			// I/O Bounded
			if (porcentaje == 0)
				thread_create (name, PRI_DEFAULT, load_thread_io, ti);
			// CPU Bounded
			else
				thread_create (name, PRI_DEFAULT, load_thread_cpu, ti);
			nice += getNice;
		}
		if (sjf){
			for (int j = thread_cnt - 1; j >= 0; j--)
			{
				struct thread_info *ti = &info[j];
				char name[16];
				ti->start_time = start_time;
				ti->tick_count = 0;
				ti->meta = j*j*10000;
				ti->nice = nice;

			 	snprintf(name, sizeof name, "Thread %d", j);
				// I/O Bounded
				if (porcentaje == 0)
					thread_create (name, PRI_DEFAULT, load_thread_io, ti);
				// CPU Bounded
				else
					thread_create (name, PRI_DEFAULT, load_thread_cpu, ti);
				nice += getNice;
			}	
			thread_cnt = thread_cnt *2; 	

		}
	}
	else		
	{
		int i=0;
		int cant_io= thread_cnt * porcentaje / 100;
		int cant_cpu= thread_cnt - cant_io;
		msg("Cantidad de hilos\t I/O: %d\t CPU: %d", cant_io, cant_cpu);
		
		while (i < thread_cnt)
		{
			struct thread_info *ti = &info[i];
			char name[16];
			ti->start_time = start_time;
			ti->tick_count = 0;
			ti->meta = i*i*100000;
			ti->nice = nice;
			snprintf(name, sizeof name, "Thread %d", i);
	
			if (i < cant_io)
				thread_create (name, PRI_DEFAULT, load_thread_io, ti);
			else
				thread_create (name, PRI_DEFAULT, load_thread_cpu, ti);
			nice += getNice;
			
			i++;
		}
		if (sjf){
			int j=0;
			while (j < thread_cnt)
			{
				struct thread_info *ti = &info[j];
				char name[16];
				ti->start_time = start_time;
				ti->tick_count = 0;
				ti->meta = j*j*100000;
				ti->nice = nice;
				snprintf(name, sizeof name, "Thread %d", j);

				if (j < cant_io){
					thread_create (name, PRI_DEFAULT, load_thread_io, ti);
				}
				else
					thread_create (name, PRI_DEFAULT, load_thread_cpu, ti);
				nice += getNice;
				
				j++;
			}
			thread_cnt = thread_cnt *2; 
			

		}
	}
 
	timer_sleep (thread_cnt * TIMER_FREQ );
	printf("Total average wait: %d \t Total threads: %d\t", total_wait_time / total_ready_threads, total_ready_threads);
  print_kernel_ticks();
}

/* CPU BOUNDED THREAD FUNCTION */
static void
load_thread_cpu (void *ti_)
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

			i= i+2;
    }
}

/* I/O BOUNDED THREAD FUNCTION */
static void
load_thread_io (void *ti_)
{
  struct thread_info *ti = ti_;
	int i=0;
  int64_t last_time = 0;
  int64_t sleep_time = 1 * TIMER_FREQ;
  timer_sleep (sleep_time - timer_elapsed (ti->start_time));
	setBound(true);
  
	while ( i < 1000)
	{
		int64_t cur_time = timer_ticks ();
		if (cur_time != last_time)
  		ti->tick_count++;
  	last_time = cur_time;

		uint32_t readValue = inl(0x100000+i);
		if (verbose)
			printf("%d'", readValue);
		i++;
	}
	if (verbose)
		printf("\n");
}

static int 
getNice (void){
	int i= 0;
	bool suma = true;
	if (suma)
		i++;
	else
		i--;

	if (i ==-20)
		suma = true;
	else if (i == 20)
		suma = false;
}


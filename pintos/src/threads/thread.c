#include "devices/timer.h"
#include <debug.h>
#include <stddef.h>
#include <random.h>
#include <stdio.h>
#include <string.h>
#include "threads/thread.h"
#include "threads/flags.h"
#include "threads/interrupt.h"
#include "threads/intr-stubs.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
#include "threads/switch.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include "fixedpoint.h"
#ifdef USERPROG
#include "userprog/process.h"
#endif

/* Random value for struct thread's `magic' member.
   Used to detect stack overflow.  See the big comment at the top
   of thread.h for details. */
#define THREAD_MAGIC 0xcd6abf4b
#define QUANTUM 7          /* # of timer ticks to give each thread. */
/* List of processes in THREAD_READY state, that is, processes
   that are ready to run but not actually running. */
static struct list ready_list;

/* List of all processes.  Processes are added to this list
   when they are first scheduled and removed when they exit. */
static struct list all_list;

/* List of all processes that have run since the boot SJF.
It keeps a unique reference for a given thread. */
static struct list all_all_list;

/* Idle thread. */
static struct thread *idle_thread;

/* Initial thread, the thread running init.c:main(). */
static struct thread *initial_thread;

/* Lock used by allocate_tid(). */
static struct lock tid_lock;
// static struct lock ready_print;

/* Stack frame for kernel_thread(). */
struct kernel_thread_frame
  {
    void *eip;                  /* Return address. */
    thread_func *function;      /* Function to call. */
    void *aux;                  /* Auxiliary data for function. */
  };

/* Statistics. */
static long long idle_ticks;    /* # of timer ticks spent idle. */
static long long kernel_ticks;  /* # of timer ticks in kernel threads. */
static long long user_ticks;    /* # of timer ticks in user programs. */

/* Scheduling. */

static unsigned thread_ticks;   /* # of timer ticks since last yield. */

/* If false (default), use round-robin scheduler.
   If true, use multi-level feedback queue scheduler.
   Controlled by kernel command-line option "-o mlfqs". */
bool thread_mlfqs;
bool roundrobin;
bool fcfs;
bool sjf;
bool verbose;

bool threads_print = true;
int total_ready_threads;
int total_wait_time;

fp load_avg;

static void kernel_thread (thread_func *, void *aux);

static void idle (void *aux UNUSED);
static struct thread *running_thread (void);
static struct thread *next_thread_to_run (void);
static void init_thread (struct thread *, const char *name, int priority);
static bool is_thread (struct thread *) UNUSED;
static void *alloc_frame (struct thread *, size_t size);
static void schedule (void);
void thread_schedule_tail (struct thread *prev);
static tid_t allocate_tid (void);

/* Initializes the threading system by transforming the code
   that's currently running into a thread.  This can't work in
   general and it is possible in this case only because loader.S
   was careful to put the bottom of the stack at a page boundary.

   Also initializes the run queue and the tid lock.

   After calling this function, be sure to initialize the page
   allocator before trying to create any threads with
   thread_create().

   It is not safe to call thread_current() until this function
   finishes. */
void
thread_init (void)
{
  ASSERT (intr_get_level () == INTR_OFF);
  lock_init (&tid_lock);
  list_init (&ready_list);
  list_init (&all_list);
  total_wait_time = 0;
  total_ready_threads = 0;
	if (sjf)
	  list_init (&all_all_list);

  /* Set up a thread structure for the running thread. */
  initial_thread = running_thread ();
  init_thread (initial_thread, "main", 0);
  initial_thread->status = THREAD_RUNNING;
  initial_thread->tid = allocate_tid ();
}

/* Starts preemptive thread scheduling by enabling interrupts.
   Also creates the idle thread. */
void
thread_start (void)
{
  /* Create the idle thread. */
  struct semaphore idle_started;
  sema_init (&idle_started, 0);
  thread_create ("idle", PRI_MIN, idle, &idle_started);

  /* Start preemptive thread scheduling. */
  intr_enable ();

  /* Wait for the idle thread to initialize idle_thread. */
  sema_down (&idle_started);
}

/* Called by the timer interrupt handler at each timer tick.
   Thus, this function runs in an external interrupt context. */
void
thread_tick (void)
{
  struct thread *cur = thread_current ();

  /* Update statistics. Added initial_thread condition so
   it doesn't effect our other threads durations*/
  if (cur == idle_thread || cur == initial_thread)
    idle_ticks++;
#ifdef USERPROG
  else if (cur->pagedir != NULL)
    user_ticks++;
#endif
  else
    kernel_ticks++;

	if (sjf)
      cur->curr_exec += 1; //SJFs

  else if (roundrobin)
  {
    if (++cur->ticks >= QUANTUM){
      intr_yield_on_return ();
      cur->ticks = 0;
    }
  }

  else if (thread_mlfqs)
  {
		if ( list_empty(&ready_list) )
    {
	  	return;
		}
		else
		{
			struct thread *next = list_entry (list_front (&ready_list), struct thread, elem);

		  /* Update thread statistics every second. */
		  if (timer_ticks () % TIMER_FREQ == 0)
		  {
		    refresh_load_avg ();
		    thread_foreach (refresh_cpu, NULL);
		  }
			if (cur->status == THREAD_RUNNING)
		  	cur->recent_cpu = add_int (cur->recent_cpu, 1);

	 		/* Recalculate priority every 4th second. */
		  if (timer_ticks () % (TIMER_FREQ * 4) == 0)
		  {
				  refresh_priority();
				  list_sort (&ready_list, priority_cmp, NULL);
				  printf("Updating priorities\n");

					if (verbose){
						struct list_elem *tmp;
						int ready_length = list_size (&ready_list);
						for (tmp = list_begin (&ready_list); tmp != list_end (&ready_list); tmp = list_next (tmp))
						{
						  struct thread *t = list_entry (tmp, struct thread, elem);
							printf("%s  ", t->name);
							for (int i=0; i< t->pos; i++){
								printf("Pri %d: %d  ", i, t->prio[i] );
							}
							printf("\n");
						}

						if (cur != idle_thread)
		        	printf("%s  ", cur->name);
							for (int i=0; i< cur->pos; i++){
								printf("Pri %d: %d  ", i, cur->prio[i] );
							}
							printf("\n");
					}
		  }
			if ( ++thread_ticks >= QUANTUM  &&  cur->priority <= next->priority )
			{
				   intr_yield_on_return ();
				   list_sort (&ready_list, priority_cmp, NULL);
			}
		}

  }
}

/* Prints thread statistics. */
void
thread_print_stats (void)
{
  printf ("Thread: %lld idle ticks, %lld kernel ticks, %lld user ticks\n",
          idle_ticks, kernel_ticks, user_ticks);
}
void
print_kernel_ticks(void)
{
  printf ("Total time: %lld ticks\n", kernel_ticks);
}

/* Creates a new kernel thread named NAME with the given initial
   PRIORITY, which executes FUNCTION passing AUX as the argument,
   and adds it to the ready queue.  Returns the thread identifier
   for the new thread, or TID_ERROR if creation fails.

   If thread_start() has been called, then the new thread may be
   scheduled before thread_create() returns.  It could even exit
   before thread_create() returns.  Contrariwise, the original
   thread may run for any amount of time before the new thread is
   scheduled.  Use a semaphore or some other form of
   synchronization if you need to ensure ordering.

   The code provided sets the new thread's `priority' member to
   PRIORITY, but no actual priority scheduling is implemented.
   Priority scheduling is the goal of Problem 1-3. */
tid_t
thread_create (const char *name, int priority,
               thread_func *function, void *aux)
{
  struct thread *t;
  struct kernel_thread_frame *kf;
  struct switch_entry_frame *ef;
  struct switch_threads_frame *sf;
  tid_t tid;
  enum intr_level old_level;
  // struct priory *pri;
  // lock_acquire(&memory_lock);
  // pri = malloc(sizeof *pri);
  // lock_release(&memory_lock);
  ASSERT (function != NULL);

  /* Allocate thread. */
  t = palloc_get_page (PAL_ZERO);
  if (t == NULL)
    return TID_ERROR;

  /* Initialize thread. */
  init_thread (t, name, priority);
  tid = t->tid = allocate_tid ();

  /* Prepare thread for first run by initializing its stack.
     Do this atomically so intermediate values for the 'stack'
     member cannot be observed. */
  old_level = intr_disable ();

  /* Stack frame for kernel_thread(). */
  kf = alloc_frame (t, sizeof *kf);
  kf->eip = NULL;
  kf->function = function;
  kf->aux = aux;

  /* Stack frame for switch_entry(). */
  ef = alloc_frame (t, sizeof *ef);
  ef->eip = (void (*) (void)) kernel_thread;

  /* Stack frame for switch_threads(). */
  sf = alloc_frame (t, sizeof *sf);
  sf->eip = switch_entry;
  sf->ebp = 0;

  intr_set_level (old_level);

  /* Add to run queue. */
  thread_unblock (t);

  return tid;
}

/* Puts the current thread to sleep.  It will not be scheduled
   again until awoken by thread_unblock().

   This function must be called with interrupts turned off.  It
   is usually a better idea to use one of the synchronization
   primitives in synch.h. */
void
thread_block (void)
{
  ASSERT (!intr_context ());
  ASSERT (intr_get_level () == INTR_OFF);

  thread_current ()->status = THREAD_BLOCKED;
  schedule ();
}

/* Transitions a blocked thread T to the ready-to-run state.
   This is an error if T is not blocked.  (Use thread_yield() to
   make the running thread ready.)

   This function does not preempt the running thread.  This can
   be important: if the caller had disabled interrupts itself,
   it may expect that it can atomically unblock a thread and
   update other data. */
void
thread_unblock (struct thread *t)
{
  enum intr_level old_level;
  if (t != idle_thread && t != initial_thread)
    t->recent_wait = timer_ticks();
  ASSERT (is_thread (t));

  old_level = intr_disable ();
  ASSERT (t->status == THREAD_BLOCKED);

  list_push_back (&ready_list, &t->elem);
  t->status = THREAD_READY;
  intr_set_level (old_level);
}

/* Returns the name of the running thread. */
const char *
thread_name (void)
{
  return thread_current ()->name;
}

/* Returns the running thread.
   This is running_thread() plus a couple of sanity checks.
   See the big comment at the top of thread.h for details. */
struct thread *
thread_current (void)
{
  struct thread *t = running_thread ();

  /* Make sure T is really a thread.
     If either of these assertions fire, then your thread may
     have overflowed its stack.  Each thread has less than 4 kB
     of stack, so a few big automatic arrays or moderate
     recursion can cause stack overflow. */
  ASSERT (is_thread (t));
  ASSERT (t->status == THREAD_RUNNING);

  return t;
}

/* Returns the running thread's tid. */
tid_t
thread_tid (void)
{
  return thread_current ()->tid;
}

/* Deschedules the current thread and destroys it.  Never
   returns to the caller. */
void
thread_exit (void)
{
  ASSERT (!intr_context ());

#ifdef USERPROG
  process_exit ();
#endif

  struct thread *t = thread_current();

  /* Remove thread from all threads list, set our status to dying,
     and schedule another process.  That process will destroy us
     when it calls thread_schedule_tail(). */
  int rep = t->repeat;



  intr_disable ();
  if (sjf){
    calc_new_total_exec(t->total_exec, t->curr_exec);
    thread_foreach(get_total_exec_each, NULL);
    list_sort(&ready_list, duration_cmp, NULL);
  }
  if (t != initial_thread && t != idle_thread){
      total_wait_time += (t->total_wait / rep);

			if (t->iobound)
      	printf("(End) Pid: %2d\t Wait: %3d\t Times: %d\t I/O Bounded\n", t->tid, t->total_wait / rep , rep);
			else
				printf("(End) Pid: %2d\t Wait: %3d\t Times: %d\t CPU Bounded\n", t->tid, t->total_wait / rep , rep);
  }

  list_remove (&t->allelem);
  thread_current ()->status = THREAD_DYING;
  schedule ();
  NOT_REACHED ();
}

/* Yields the CPU.  The current thread is not put to sleep and
   may be scheduled again immediately at the scheduler's whim. */
void
thread_yield (void)
{
  struct thread *cur = thread_current ();
  enum intr_level old_level;
  if (cur != idle_thread && cur != initial_thread)
    cur->recent_wait = timer_ticks();
  ASSERT (!intr_context ());

  old_level = intr_disable ();

  if (cur != idle_thread)
    list_push_back (&ready_list, &cur->elem);	//Insertar en orden
  cur->status = THREAD_READY;
  schedule ();
  intr_set_level (old_level);
}

/* Invoke function 'func' on all threads, passing along 'aux'.
   This function must be called with interrupts off. */
void
thread_foreach (thread_action_func *func, void *aux)
{
  struct list_elem *e;

  ASSERT (intr_get_level () == INTR_OFF);

  for (e = list_begin (&all_list); e != list_end (&all_list);
       e = list_next (e))
    {
      struct thread *t = list_entry (e, struct thread, allelem);
      func (t, aux);
    }
}

/* Sets the current thread's priority to NEW_PRIORITY. */
void
thread_set_priority (int new_priority)
{
  thread_current ()->priority = new_priority;
   /* Recalculate priority. */
  if (thread_mlfqs)
    refresh_priority();
}

/* Returns the current thread's priority. */
int
thread_get_priority (void)
{
  return thread_current ()->priority;
}

/* Sets the current thread's nice value to NICE. */
void
thread_set_nice (int nice)
{
  struct thread *t = thread_current ();
  t->nice = nice;
  /* Recalculate priority. */
  if (thread_mlfqs)
    refresh_priority ();
}

/* Returns the current thread's nice value. */
int
thread_get_nice (void)
{
  struct thread *t = thread_current ();
  return t->nice;
}

/* Returns 100 times the system load average. */
int
thread_get_load_avg (void)
{
  return fp_to_int_round (mult_int (load_avg, 100));;
}

/* Returns 100 times the current thread's recent_cpu value. */
int
thread_get_recent_cpu (void)
{
  struct thread *t = running_thread ();
  return fp_to_int_round (mult_int (t->recent_cpu, 100));;
}

/* Idle thread.  Executes when no other thread is ready to run.

   The idle thread is initially put on the ready list by
   thread_start().  It will be scheduled once initially, at which
   point it initializes idle_thread, "up"s the semaphore passed
   to it to enable thread_start() to continue, and immediately
   blocks.  After that, the idle thread never appears in the
   ready list.  It is returned by next_thread_to_run() as a
   special case when the ready list is empty. */
static void
idle (void *idle_started_ UNUSED)
{
  struct semaphore *idle_started = idle_started_;
  idle_thread = thread_current ();
  sema_up (idle_started);

  for (;;)
    {
      /* Let someone else run. */
      intr_disable ();
      thread_block ();

      /* Re-enable interrupts and wait for the next one.

         The `sti' instruction disables interrupts until the
         completion of the next instruction, so these two
         instructions are executed atomically.  This atomicity is
         important; otherwise, an interrupt could be handled
         between re-enabling interrupts and waiting for the next
         one to occur, wasting as much as one clock tick worth of
         time.

         See [IA32-v2a] "HLT", [IA32-v2b] "STI", and [IA32-v3a]
         7.11.1 "HLT Instruction". */
      asm volatile ("sti; hlt" : : : "memory");
    }
}

/* Function used as the basis for a kernel thread. */
static void
kernel_thread (thread_func *function, void *aux)
{
  ASSERT (function != NULL);

  intr_enable ();       /* The scheduler runs with interrupts off. */
  function (aux);       /* Execute the thread function. */
  thread_exit ();       /* If function() returns, kill the thread. */
}

/* Returns the running thread. */
struct thread *
running_thread (void)
{
  uint32_t *esp;

  /* Copy the CPU's stack pointer into `esp', and then round that
     down to the start of a page.  Because `struct thread' is
     always at the beginning of a page and the stack pointer is
     somewhere in the middle, this locates the curent thread. */
  asm ("mov %%esp, %0" : "=g" (esp));
  return pg_round_down (esp);
}

/* Returns true if T appears to point to a valid thread. */
static bool
is_thread (struct thread *t)
{
  return t != NULL && t->magic == THREAD_MAGIC;
}

/* Does basic initialization of T as a blocked thread named
   NAME. */
static void
init_thread (struct thread *t, const char *name, int priority)
{
  ASSERT (t != NULL);
  ASSERT (PRI_MIN <= priority && priority <= PRI_MAX);
  ASSERT (name != NULL);

	bool iobound;

  memset (t, 0, sizeof *t);

  t->status = THREAD_BLOCKED;
  strlcpy (t->name, name, sizeof t->name);
  t->stack = (uint8_t *) t + PGSIZE;
  t->priority = priority;
  t->magic = THREAD_MAGIC;
  sema_init (&t->timer_sema, 0);
  t->curr_exec = 0;
  t->total_exec = 0;
  t->recent_wait = 0;
  t->total_wait = 0;
  t->ticks = 0;
  t->pos = 0;	

  if (thread_mlfqs)
  {
    t->nice = 0;
    t->recent_cpu = 0;
    t->repeat= 0;
  }

	if (sjf)
    t->total_exec = get_total_exec(t);

	if (strcmp(name, "idle") != 0 && strcmp(name, "s") != 0)
  {
		t->prio[t->pos]= priority; 
  }

  list_push_back (&all_list, &t->allelem);
}

/* Allocates a SIZE-byte frame at the top of thread T's stack and
   returns a pointer to the frame's base. */
static void *
alloc_frame (struct thread *t, size_t size)
{
  /* Stack data is always allocated in word-size units. */
  ASSERT (is_thread (t));
  ASSERT (size % sizeof (uint32_t) == 0);

  t->stack -= size;
  return t->stack;
}

/* Chooses and returns the next thread to be scheduled.  Should
   return a thread from the run queue, unless the run queue is
   empty.  (If the running thread can continue running, then it
   will be in the run queue.)  If the run queue is empty, return
   idle_thread. */
static struct thread *
next_thread_to_run (void)
{
  if (list_empty (&ready_list))
    return idle_thread;
  else
    return list_entry (list_pop_front (&ready_list), struct thread, elem);
}

/* Completes a thread switch by activating the new thread's page
   tables, and, if the previous thread is dying, destroying it.

   At this function's invocation, we just switched from thread
   PREV, the new thread is already running, and interrupts are
   still disabled.  This function is normally invoked by
   thread_schedule() as its final action before returning, but
   the first time a thread is scheduled it is called by
   switch_entry() (see switch.S).

   It's not safe to call printf() until the thread switch is
   complete.  In practice that means that printf()s should be
   added at the end of the function.

   After this function and its caller returns, the thread switch
   is complete. */
void
thread_schedule_tail (struct thread *prev)
{
  struct thread *cur = running_thread ();

  cur->recent_wait = timer_ticks() - cur->recent_wait;
  cur->total_wait += cur->recent_wait;
  cur->recent_wait = 0;
  ASSERT (intr_get_level () == INTR_OFF);

  /* Mark us as running. */
  cur->status = THREAD_RUNNING;

  /* Start new time slice. */
  thread_ticks = 0;

#ifdef USERPROG
  /* Activate the new address space. */
  process_activate ();
#endif

  /* If the thread we switched from is dying, destroy its struct
     thread.  This must happen late so that thread_exit() doesn't
     pull out the rug under itself.  (We don't free
     initial_thread because its memory was not obtained via
     palloc().) */
  if (prev != NULL && prev->status == THREAD_DYING && prev != initial_thread)
    {
      ASSERT (prev != cur);
			if (!sjf)
      	palloc_free_page (prev);
    }
}

/* Schedules a new process.  At entry, interrupts must be off and
   the running process's state must have been changed from
   running to some other state.  This function finds another
   thread to run and switches to it.

   It's not safe to call printf() until thread_schedule_tail()
   has completed. */
static void
schedule (void)
{
  struct thread *cur = running_thread ();
  struct thread *next = next_thread_to_run ();
  struct thread *prev = NULL;


  ASSERT (intr_get_level () == INTR_OFF);
  ASSERT (cur->status != THREAD_RUNNING);
  ASSERT (is_thread (next));

  if (cur != next)
    prev = switch_threads (cur, next);
  thread_schedule_tail (prev);
	
	if (next != idle_thread && next != initial_thread && threads_print){
    total_ready_threads = list_size (&ready_list);
    print_ready_list();
    threads_print = false;
  }

  cur->repeat += 1;
  if (strcmp(&cur->name, "idle") != 0 && verbose){
  	printf("(Schedule) %s\t", running_thread ()->name);
		if (thread_mlfqs){
		  printf("Avg: %3d  ", thread_get_load_avg());
		  printf("Pri: %2d  ", running_thread ()->priority);
		  printf("CPU: %d  ", running_thread ()->recent_cpu);
			printf("Time: %2d  ", running_thread ()->repeat);
		}
		else if (sjf)
			printf("Run: %3d  ", running_thread ()->total_exec );
		else if (roundrobin)
			printf("Time: %2d  ", running_thread ()->repeat);
		printf("\n");
  }
	
	

}

/* Returns a tid to use for a new thread. */
static tid_t
allocate_tid (void)
{
  static tid_t next_tid = 1;
  tid_t tid;

  lock_acquire (&tid_lock);
  tid = next_tid++;
  lock_release (&tid_lock);

  return tid;
}

/* Refresh the priority of the ready threads */
void
refresh_priority ()
{

  struct list_elem *tmp;
  struct thread *r = running_thread();

  for (tmp = list_begin (&ready_list); tmp != list_end (&ready_list); tmp = list_next (tmp))
  {
    struct thread *t = list_entry (tmp, struct thread, elem);
    if (t!=initial_thread){
      refresh_priority_thread(t, NULL);
    }

  }
  refresh_priority_thread(r, NULL);
}

void
refresh_priority_thread(struct thread *t, void *aux){
		int pri = PRI_MAX - fp_to_int_round(div_int (t->recent_cpu, 4)) - (t->nice * 2);
    t->priority = pri;
	
    if (t->priority > PRI_MAX)
      t->priority = PRI_MAX;
    else if (t->priority < PRI_MIN)
      t->priority = PRI_MIN;

		if (t->pos != 10){
			t->prio[t->pos]= pri;
			t->pos++;
		}
}

/* Refresh the priority of the ready threads */
void
refresh_cpu (struct thread *t, void *aux)
{
  if (t != idle_thread)
  {
    fp first_term = mult_int(load_avg, 2);
    fp second_term = add_int(first_term, 1);
    fp third_term = div_fp(first_term, second_term);
    t->recent_cpu = add_int (mult_fp (third_term, t->recent_cpu), t->nice);
  }
}

/* Refresh the priority of the ready threads */
void
refresh_load_avg ()
{
    /* Update load_avg. */
  int ready_thread_count = list_size (&ready_list);
  if (thread_current () != idle_thread)
    ++ready_thread_count;

  int ready_length = list_size (&ready_list);
  load_avg = (add_fp (div_int (mult_int (load_avg, 59), 60), div_int (int_to_fp (ready_length), 60) ));
}

/* Compare 2 threads for wakeup time or priority */
bool
wakeup_cmp (const struct list_elem *left,const struct list_elem *right,void *aux)
{
  struct thread *t_left = list_entry (left, struct thread, timer_elem);
  struct thread *t_right = list_entry (right, struct thread, timer_elem);

  if (t_left->wakeup_time < t_right->wakeup_time)
    return true;
  return false;
}

/* Set Bound value */
bool
setBound(bool bound){
	struct thread *t = running_thread();
	t->iobound= bound;
}

/* Comparison function that prefers the thread with higher priority. */
bool
priority_cmp (const struct list_elem *left, const struct list_elem *right, void *aux)
{
  struct thread *t_left = list_entry (left, struct thread, elem);
  struct thread *t_right = list_entry (right, struct thread, elem);

	if (t_left->priority > t_right->priority)
		return true;
  return false;
}

/* Funcion utilizada para ordenar de forma ascendente, la lista
   de los threads listos (ready_list) */
bool
duration_cmp (const struct list_elem *left, const struct list_elem *right, void *aux UNUSED)
{
  struct thread *t_left = list_entry (left, struct thread, elem);
  struct thread *t_right = list_entry (right, struct thread, elem);

  return t_left->total_exec < t_right->total_exec;
}

/* Funcion utilizada para obtener el execute time de un
   thread dado. Si el valor es 0, es decir, el thread no
   ha corrido, se agrega el thread a la lista all_all_list */
int
get_total_exec (struct thread *t)
{
  struct list_elem *tmp;
  for (tmp = list_begin (&all_all_list); tmp != list_end (&all_all_list); tmp = list_next (tmp))
  {
    struct thread *t_tmp = list_entry (tmp, struct thread, allallelem);
    if (strcmp(&t_tmp->name, &t->name) == 0)
      return t_tmp->total_exec;
  }
  list_push_back (&all_all_list, &t->allallelem);
  return 0;
}

void get_total_exec_each(struct thread *t, void *aux UNUSED)
{
    t->total_exec = get_total_exec(t);
}

void
print_all_all_list() {
  struct list_elem *tmp;
  for (tmp = list_begin (&all_all_list); tmp != list_end (&all_all_list); tmp = list_next (tmp))
  {
    struct thread *t_tmp = list_entry (tmp, struct thread, allallelem);
    printf("i: %s,", t_tmp->name );
  }
}
void
print_ready_list()
{
  struct list_elem *tmp;
	int i=1;
	printf("------------------------------------------------------Ready Stack------------------------------------------------------------\n");
  for (tmp = list_begin (&ready_list); tmp != list_end (&ready_list); tmp = list_next (tmp))
  {
    struct thread *t_tmp = list_entry (tmp, struct thread, elem);
    printf("Name: %9s     Pid: %2d",t_tmp->name, t_tmp->tid );
		if (i%4 == 0)
			printf("\n");
		else
			printf("\t   ");
		i++;
  }
	printf("\n-----------------------------------------------------------------------------------------------------------------------------\n");
}
/* Funcion utilizada para guardar el nuevo execute time del
   thread. Es llamada cuando el thread esta por terminar*/

void
update_exec_time (const char *name, int total_exec)
{
    struct list_elem *tmp;
    for (tmp = list_begin (&all_all_list); tmp != list_end (&all_all_list); tmp = list_next (tmp))
    {
      struct thread *t = list_entry (tmp, struct thread, allallelem);
      t->total_exec = total_exec;
    }
}

/* Para SJF, recalcula la duracion aproximada que tarda en
   ejecutarse el thread. Utilza la formula:
   t_new = alpha * t_recent + (1 - alpha) * t_old */
void
calc_new_total_exec(int old, int recent)
{
    int res;
    if (old == 0)
      res = recent;
    else
    {
        fp one_fp = int_to_fp(1);
        fp alpha = div_int(one_fp,2);
        fp alpha_alt = sub_fp(one_fp, alpha);
        fp recent_val = mult_int(alpha, recent);
        fp old_val = mult_int(alpha_alt, old);
        res = fp_to_int_round(add_fp(recent_val, old_val));
        // struct thread_dur *t_dur = malloc (sizeof *t_dur);
        // t_dur->name = thread_name()
        // strlcpy (t_dur->name, thread_name(), sizeof t_dur->name);
        // t_dur->total_exec = t_new;
    }
    thread_current()->total_exec = res;
    // printf("T: %s \tExec: %d\n", thread_name(), thread_current()->total_exec );
    // update_exec_time(thread_current()->name, res);
}
// void
// print_priority_values(struct thread *t){
//   struct list_elem *tmp;
//   for (tmp = list_begin (&t->priority_values); tmp != list_end (&t->priority_values); tmp = list_next (tmp))
//   {
//     struct priory *pri = list_entry (tmp, struct priory, priority_elem);
//     printf("Prioridad:%d, \n", pri->value);
//   }
// }

/* Offset of `stack' member within `struct thread'.
   Used by switch.S, which can't figure it out on its own. */
uint32_t thread_stack_ofs = offsetof (struct thread, stack);

#ifndef THREADS_THREAD_H
#define THREADS_THREAD_H

#include "threads/synch.h"
//#include "fixed_point.h"
#include <debug.h>
#include <list.h>
#include <stdint.h>

/* States in a thread's life cycle. */
enum thread_status
  {
    THREAD_RUNNING,     /* Running thread. */
    THREAD_READY,       /* Not running but ready to run. */
    THREAD_BLOCKED,     /* Waiting for an event to trigger. */
    THREAD_DYING        /* About to be destroyed. */
  };

/* Thread identifier type.
   You can redefine this to whatever type you like. */
typedef int tid_t;
#define TID_ERROR ((tid_t) -1)          /* Error value for tid_t. */

/* Thread priorities. */
#define PRI_MIN 0                       /* Lowest priority. */
#define PRI_DEFAULT 31                  /* Default priority. */
#define PRI_MAX 63                      /* Highest priority. */
#define fp int

/* A kernel thread or user process.

   Each thread structure is stored in its own 4 kB page.  The
   thread structure itself sits at the very bottom of the page
   (at offset 0).  The rest of the page is reserved for the
   thread's kernel stack, which grows downward from the top of
   the page (at offset 4 kB).  Here's an illustration:

        4 kB +---------------------------------+
             |          kernel stack           |
             |                |                |
             |                |                |
             |                V                |
             |         grows downward          |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             +---------------------------------+
             |              magic              |
             |                :                |
             |                :                |
             |               name              |
             |              status             |
        0 kB +---------------------------------+

   The upshot of this is twofold:

      1. First, `struct thread' must not be allowed to grow too
         big.  If it does, then there will not be enough room for
         the kernel stack.  Our base `struct thread' is only a
         few bytes in size.  It probably should stay well under 1
         kB.

      2. Second, kernel stacks must not be allowed to grow too
         large.  If a stack overflows, it will corrupt the thread
         state.  Thus, kernel functions should not allocate large
         structures or arrays as non-static local variables.  Use
         dynamic allocation with malloc() or palloc_get_page()
         instead.

   The first symptom of either of these problems will probably be
   an assertion failure in thread_current(), which checks that
   the `magic' member of the running thread's `struct thread' is
   set to THREAD_MAGIC.  Stack overflow will normally change this
   value, triggering the assertion. */
/* The `elem' member has a dual purpose.  It can be an element in
   the run queue (thread.c), or it can be an element in a
   semaphore wait list (synch.c).  It can be used these two ways
   only because they are mutually exclusive: only a thread in the
   ready state is on the run queue, whereas only a thread in the
   blocked state is on a semaphore wait list. */
struct thread
  {
    /* Owned by thread.c. */
    tid_t tid;                          /* Thread identifier. */
    enum thread_status status;          /* Thread state. */
    char name[16];                      /* Name (for debugging purposes). */
    uint8_t *stack;                     /* Saved stack pointer. */
    struct list_elem allelem;           /* List element for all threads list. */

		/* Stats */
    int recent_wait;
    int total_wait;
		int repeat;
		int prio[10];
		int pos;
		bool iobound;
    /* --- Stats */

    /* RR */
    int ticks;
    /* --RR*/

    /* SJF */
    int curr_exec;                      /* Current ticks used */
    int total_exec;                     /* Aprox. avg ticks used */
    struct list_elem allallelem;        /* List element for all_all_list */
    /* --- SJF */

		/* MLFQS */
		int priority;
    int nice;
    fp recent_cpu;

		/* MLFQS */

		/* Timer */
    int64_t wakeup_time;                /* Thread wakeup time in ticks. */
    struct semaphore timer_sema;
    struct list_elem timer_elem;        /* List element for timer_wait_list. */
		/* --- Timer */

    /* Shared between thread.c and synch.c. */
    struct list_elem elem;              /* List element. */

#ifdef USERPROG
    /* Owned by userprog/process.c. */
    uint32_t *pagedir;                  /* Page directory. */
#endif

    /* Owned by thread.c. */
    unsigned magic;                     /* Detects stack overflow. */
  };


// struct priory
// {
//   int value;
//   struct list_elem priority_elem;
// };
/* Algorithm selection*/
extern bool thread_mlfqs;
extern bool roundrobin;
extern bool fcfs;
extern bool sjf;
/* --- Algorithm selection*/

/* Statistics variables */
extern int total_ready_threads;
extern int total_wait_time;
/* Statistics variables */

extern bool verbose;

void thread_init (void);
void thread_start (void);

void thread_tick (void);
void thread_print_stats (void);

typedef void thread_func (void *aux);
tid_t thread_create (const char *name, int priority, thread_func *, void *);

void thread_block (void);
void thread_unblock (struct thread *);

struct thread *thread_current (void);
tid_t thread_tid (void);
const char *thread_name (void);

void thread_exit (void) NO_RETURN;
void thread_yield (void);

/* Performs some operation on thread t, given auxiliary data AUX. */
typedef void thread_action_func (struct thread *t, void *aux);
void thread_foreach (thread_action_func *, void *);

/* Funciones timer */
bool wakeup_cmp (const struct list_elem *left, const struct list_elem *right, void *aux);
/* --- Funciones timer */
bool setBound(bool);

/* Funciones MLFQS */
bool priority_cmp (const struct list_elem *left, const struct list_elem *right, void *aux);
int thread_get_priority (void);
void thread_set_priority (int);

int thread_get_nice (void);
void thread_set_nice (int);
int thread_get_recent_cpu (void);
int thread_get_load_avg (void);
void refresh_priority(void);
void refresh_load_avg(void);
void refresh_cpu(struct thread *t, void *aux);
void refresh_priority_thread(struct thread *t, void *);
/* --- Funciones MLFQS */

/* Funciones para algoritmo SJF */
bool duration_cmp (const struct list_elem *left, const struct list_elem *right, void *aux);
void update_exec_time(const char *name, int total_exec);
int get_total_exec(struct thread *t);
void get_total_exec_each(struct thread *t, void *aux UNUSED);
void calc_new_total_exec(int old, int recent);
void print_all_all_list(void);

/* --- Funciones para algoritmo SJF */



/* Funciones para estadisticas*/
void print_priority_values(struct thread *t);
void print_kernel_ticks(void);
#endif /* threads/thread.h */

#ifndef Q_THREAD_H
#define Q_THREAD_H

#include "os_defs.h"
#include "os_thread.h"
#include "os_mutex.h"

#include "q_inline.h"

#if defined (__cplusplus)
extern "C" {
#endif

/* Things don't go wrong if CACHE_LINE_SIZE is defined incorrectly,
   they just run slower because of false cache-line sharing. It can be
   discovered at run-time, but in practice it's 64 for most CPUs and
   128 for some. */
#define CACHE_LINE_SIZE 64

typedef os_uint32 vtime_t;
typedef os_int32 svtime_t; /* signed version */

/* GCC has a nifty feature allowing the specification of the required
   alignment: __attribute__ ((aligned (CACHE_LINE_SIZE))) in this
   case. Many other compilers implement it as well, but it is by no
   means a standard feature.  So we do it the old-fashioned way. */


/* These strings are used to indicate the required scheduling class to the "create_thread()" */
#define Q_THREAD_SCHEDCLASS_REALTIME  "Realtime"
#define Q_THREAD_SCHEDCLASS_TIMESHARE "Timeshare"

/* When this value is used, the platform default for scheduling priority will be used */
#define Q_THREAD_SCHEDPRIO_DEFAULT 0

enum thread_state {
  THREAD_STATE_ZERO,
  THREAD_STATE_ALIVE
};

struct logbuf;

/*
 * watchdog indicates progress for the service lease liveliness mechsanism, while vtime
 * indicates progress for the Garbage collection purposes.
 *  vtime even : thread awake
 *  vtime odd  : thread asleep
 */
#define THREAD_BASE                             \
  volatile vtime_t vtime;                       \
  volatile vtime_t watchdog;                    \
  os_threadId tid;                              \
  os_threadId extTid;                           \
  enum thread_state state;                      \
  struct logbuf *lb;                            \
  char *name /* note: no semicolon! */

struct thread_state_base {
  THREAD_BASE;
};

struct thread_state1 {
  THREAD_BASE;
  char pad[CACHE_LINE_SIZE
           * ((sizeof (struct thread_state_base) + CACHE_LINE_SIZE - 1)
              / CACHE_LINE_SIZE)
           - sizeof (struct thread_state_base)];
};
#undef THREAD_BASE

struct thread_states {
  os_mutex lock;
  int nthreads;
  struct thread_state1 *ts; /* [nthreads] */
};

extern struct thread_states thread_states;
#if OS_HAS_TSD_USING_THREAD_KEYWORD
extern __thread struct thread_state1 *tsd_thread_state;
#endif

int thread_states_init (int maxthreads);
void thread_states_fini (void);

void upgrade_main_thread (void);
void downgrade_main_thread (void);
const struct config_thread_properties_listelem *lookup_thread_properties (const char *name);
struct thread_state1 *create_thread (const char *name, void * (*f) (void *arg), void *arg);
struct thread_state1 *lookup_thread_state_real (void);
int join_thread (struct thread_state1 *ts1, void **ret);

#if defined (__cplusplus)
}
#endif

#if NN_HAVE_C99_INLINE && !defined SUPPRESS_THREAD_INLINES
#include "q_thread_template.c"
#else
#if defined (__cplusplus)
extern "C" {
#endif
int vtime_awake_p (vtime_t vtime);
int vtime_asleep_p (vtime_t vtime);
int vtime_gt (vtime_t vtime1, vtime_t vtime0);

struct thread_state1 *lookup_thread_state (void);
void thread_state_asleep (struct thread_state1 *ts1);
void thread_state_awake (struct thread_state1 *ts1);
void thread_state_blocked (struct thread_state1 *ts1);
void thread_state_unblocked (struct thread_state1 *ts1);
#if defined (__cplusplus)
}
#endif
#endif

#endif /* Q_THREAD_H */

/* SHA1 not available (unoffical build.) */

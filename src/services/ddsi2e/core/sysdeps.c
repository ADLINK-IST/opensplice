/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#include <assert.h>
#include <stdlib.h>

#include "os_defs.h"
#include "os_if.h"
#include "os_heap.h"
#include "os_thread.h"
#include "os_socket.h"
#include "os_atomics.h"
#include "os_errno.h"
#ifndef SYSDEPS_HAVE_RANDOM
#include "os_stdlib.h"
#endif

#include "q_error.h"
#include "q_log.h"
#include "q_config.h"
#include "sysdeps.h"

#ifdef NEED_ARM_MEMBAR_SUPPORT
static void q_membar_autodecide (void);
void (*q_maybe_membar) (void) = q_membar_autodecide;

static void q_membar_nop (void) { }
static void q_membar_dmb (void) { MemoryBarrierARM; }

static void q_membar_autodecide (void)
{
  SYSTEM_INFO sysinfo;
  GetSystemInfo (&sysinfo);
  assert (sysinfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ARM);
  if (sysinfo.wProcessorLevel >= 7)
  {
    q_maybe_membar = q_membar_dmb;
  }
  else
  {
    q_maybe_membar = q_membar_nop;
  }
  q_maybe_membar ();
}
#endif

/* MISSING IN OS ABSTRACTION LAYER ------------------------------------- */

#if ! SYSDEPS_HAVE_RECVMSG
os_ssize_t recvmsg (os_handle fd, struct msghdr *message, int flags)
{
  os_ssize_t ret;
  assert (message->msg_iovlen == 1);
#if SYSDEPS_MSGHDR_ACCRIGHTS
  assert (message->msg_accrightslen == 0);
#else
  assert (message->msg_controllen == 0);
#endif
#if SYSDEPS_MSGHDR_FLAGS
  message->msg_flags = 0;
#endif
  ret = recvfrom (fd, message->msg_iov[0].iov_base, message->msg_iov[0].iov_len, flags,
                  message->msg_name, &message->msg_namelen);
#if defined (_WIN32)
  /* Windows returns an error for too-large messages, Unix expects
     original size and the MSG_TRUNC flag.  MSDN says it is truncated,
     which presumably means it returned as much of the message as it
     could - so we return that the message was 1 byte larger than the
     available space, and set MSG_TRUNC if we can. */
  if (ret == -1 && os_getErrno () == WSAEMSGSIZE) {
    ret = message->msg_iov[0].iov_len + 1;
#if SYSDEPS_MSGHDR_FLAGS
    message->msg_flags |= MSG_TRUNC;
#endif
  }
#endif
  return ret;
}
#endif

#if ! SYSDEPS_HAVE_SENDMSG
#if !(defined _WIN32 && !defined WINCE)
os_ssize_t sendmsg (os_handle fd, const struct msghdr *message, int flags)
{
  char stbuf[3072], *buf;
  os_ssize_t sz, ret;
  os_size_t sent = 0;
  unsigned i;

#if SYSDEPS_MSGHDR_ACCRIGHTS
  assert (message->msg_accrightslen == 0);
#else
  assert (message->msg_controllen == 0);
#endif

  if (message->msg_iovlen == 1)
  {
    /* if only one fragment, skip all copying */
    buf = message->msg_iov[0].iov_base;
    sz = message->msg_iov[0].iov_len;
  }
  else
  {
    /* first determine the size of the message, then select the
       on-stack buffer or allocate one on the heap ... */
    sz = 0;
    for (i = 0; i < message->msg_iovlen; i++)
    {
      sz += message->msg_iov[i].iov_len;
    }
    if (sz <= sizeof (stbuf))
    {
      buf = stbuf;
    }
    else
    {
      buf = os_malloc (sz);
    }
    /* ... then copy data into buffer */
    sz = 0;
    for (i = 0; i < message->msg_iovlen; i++)
    {
      memcpy (buf + sz, message->msg_iov[i].iov_base, message->msg_iov[i].iov_len);
      sz += message->msg_iov[i].iov_len;
    }
  }

  while (TRUE)
  {
    ret = sendto (fd, buf + sent, sz - sent, flags, message->msg_name, message->msg_namelen);
    if (ret < 0)
    {
      break;
    }
    sent += (os_size_t) ret;
    if (sent == sz)
    {
      ret = sent;
      break;
    }
  }

  if (buf != stbuf)
  {
    os_free (buf);
  }
  return ret;
}
#else /* _WIN32 && !WINCE */
os_ssize_t sendmsg (os_handle fd, const struct msghdr *message, int flags)
{
  WSABUF stbufs[128], *bufs;
  DWORD sent;
  unsigned i;
  os_ssize_t ret;

#if SYSDEPS_MSGHDR_ACCRIGHTS
  assert (message->msg_accrightslen == 0);
#else
  assert (message->msg_controllen == 0);
#endif

  if (message->msg_iovlen <= (int)(sizeof(stbufs) / sizeof(*stbufs)))
    bufs = stbufs;
  else
    bufs = os_malloc (message->msg_iovlen * sizeof (*bufs));
  for (i = 0; i < message->msg_iovlen; i++)
  {
    bufs[i].buf = (void *) message->msg_iov[i].iov_base;
    bufs[i].len = (unsigned) message->msg_iov[i].iov_len;
  }
  if (WSASendTo (fd, bufs, message->msg_iovlen, &sent, flags, (SOCKADDR *) message->msg_name, message->msg_namelen, NULL, NULL) == 0)
    ret = (os_ssize_t) sent;
  else
    ret = -1;
  if (bufs != stbufs)
    os_free (bufs);
  return ret;
}
#endif
#endif

#ifndef SYSDEPS_HAVE_RANDOM
long random (void)
{
  /* rand() is a really terribly bad PRNG */
  union { long x; unsigned char c[4]; } t;
  int i;
  for (i = 0; i < 4; i++)
    t.c[i] = (unsigned char) ((os_rand () >> 4) & 0xff);
#if RAND_MAX == 0x7fffffff || RAND_MAX == 0x7fff
  t.x &= RAND_MAX;
#elif RAND_MAX <= 0x7ffffffe
  t.x %= (RAND_MAX+1);
#else
#error "RAND_MAX out of range"
#endif
  return t.x;
}
#endif

#if SYSDEPS_HAVE_CLOCK_THREAD_CPUTIME
os_int64 get_thread_cputime (void)
{
  struct timespec ts;
  clock_gettime (CLOCK_THREAD_CPUTIME_ID, &ts);
  return ts.tv_sec * (os_int64) 1000000000 + ts.tv_nsec;
}
#else
os_int64 get_thread_cputime (void)
{
  return 0;
}
#endif

#if ! OS_HAVE_THREADEQUAL
int os_threadEqual (os_threadId a, os_threadId b)
{
  /* on pthreads boxes, pthread_equal (a, b); as a workaround: */
  os_ulong_int ai = os_threadIdToInteger (a);
  os_ulong_int bi = os_threadIdToInteger (b);
  return ai == bi;
}
#endif

#if defined __sun && __GNUC__ && defined __sparc__
int __S_exchange_and_add (volatile int *mem, int val)
{
  /* Hopefully cache lines are <= 64 bytes, we then use 8 bytes, 64
     bytes.  Should be a lot better than just one lock byte if it gets
     used often, but the overhead is a bit larger, so who knows what's
     best without trying it out?.  We want 3 bits + 6 lsbs zero, so
     need to shift addr_hash by 23 bits. */
  static unsigned char locks[8 * 64];
  unsigned addr_hash = 0xe2c7 * (unsigned short) ((os_address) mem >> 2);
  unsigned lock_idx = (addr_hash >> 23) & ~0x1c0;
  unsigned char * const lock = &locks[lock_idx];
  int result, tmp;

  __asm__ __volatile__("1:      ldstub  [%1], %0\n\t"
                       "        cmp     %0, 0\n\t"
                       "        bne     1b\n\t"
                       "         nop"
                       : "=&r" (tmp)
                       : "r" (lock)
                       : "memory");
  result = *mem;
  *mem += val;
  __asm__ __volatile__("stb     %%g0, [%0]"
                       : /* no outputs */
                       : "r" (lock)
                       : "memory");
  return result;
}
#endif

#if !(defined __APPLE__ || defined __linux) || (__GNUC__ > 0 && (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) < 40100)
void log_stacktrace (const char *name, os_threadId tid)
{
  OS_UNUSED_ARG (name);
  OS_UNUSED_ARG (tid);
}
#else
#include <execinfo.h>

static pa_uint32_t log_stacktrace_flag = PA_UINT32_INIT(0);
static struct {
  int depth;
  void *stk[64];
} log_stacktrace_stk;


static void log_stacktrace_sigh (int sig __attribute__ ((unused)))
{
  int e = os_getErrno();
  log_stacktrace_stk.depth = backtrace (log_stacktrace_stk.stk, (int) (sizeof (log_stacktrace_stk.stk) / sizeof (*log_stacktrace_stk.stk)));
  pa_inc32 (&log_stacktrace_flag);
  os_setErrno(e);
}

void log_stacktrace (const char *name, os_threadId tid)
{
  if (config.enabled_logcats == 0)
    ; /* no op if nothing logged */
  else if (!config.noprogress_log_stacktraces)
    nn_log (~0u, "-- stack trace of %s requested, but traces disabled --\n", name);
  else
  {
    const os_duration d = OS_DURATION_INIT(0, 1000000);
    struct sigaction act, oact;
    char **strs;
    int i;
    nn_log (~0u, "-- stack trace of %s requested --\n", name);
    act.sa_handler = log_stacktrace_sigh;
    act.sa_flags = 0;
    (void)sigfillset (&act.sa_mask);
    while (!pa_cas32 (&log_stacktrace_flag, 0, 1))
      ospl_os_sleep (d);
    sigaction (SIGXCPU, &act, &oact);
    pthread_kill (tid, SIGXCPU);
    while (!pa_cas32 (&log_stacktrace_flag, 2, 3))
      ospl_os_sleep (d);
    sigaction (SIGXCPU, &oact, NULL);
    nn_log (~0u, "-- stack trace follows --\n");
    strs = backtrace_symbols (log_stacktrace_stk.stk, log_stacktrace_stk.depth);
    for (i = 0; i < log_stacktrace_stk.depth; i++)
      nn_log (~0u, "%s\n", strs[i]);
    free (strs);
    nn_log (~0u, "-- end of stack trace --\n");
    pa_st32 (&log_stacktrace_flag, 0);
  }
}
#endif

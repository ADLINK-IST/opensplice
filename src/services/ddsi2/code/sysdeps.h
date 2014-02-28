#ifndef SYSDEPS_H
#define SYSDEPS_H

#include "os_defs.h"
#include "os_thread.h"
#include "os_socket.h"
#include "os_abstract.h"

#include "q_inline.h"

#ifndef os_sockECONNRESET
#ifdef WSAECONNRESET
#define os_sockECONNRESET WSAECONNRESET
#else
#define os_sockECONNRESET ECONNRESET
#endif
#endif

#ifndef os_sockEPERM
#ifdef WSAEACCES
#define os_sockEPERM WSAEACCES
#else
#define os_sockEPERM EPERM
#endif
#endif

#if defined (__linux) || defined (__sun) || defined (__APPLE__) || defined (INTEGRITY) || defined (AIX) || defined (OS_RTEMS_DEFS_H) || defined (VXWORKS_RTP) || defined (__Lynx__) || defined (_WRS_KERNEL) || defined (__linux__)
#define SYSDEPS_HAVE_MSGHDR 1
#define SYSDEPS_HAVE_RECVMSG 1
#define SYSDEPS_HAVE_SENDMSG 1
#endif

#if defined (__linux) || defined (__sun) || defined (__APPLE__) || defined (INTEGRITY) || defined (AIX) || defined (OS_RTEMS_DEFS_H) || defined (VXWORKS_RTP) || defined (_WRS_KERNEL) || defined (__linux__)
#define SYSDEPS_HAVE_IOVEC 1
#endif

#if defined (__linux) || defined (__sun) || defined (__APPLE__) || defined (AIX) || defined (__Lynx__)
#define SYSDEPS_HAVE_RANDOM 1
#include <unistd.h>
#endif

#if defined (__linux) || defined (__sun) || defined (__APPLE__) || defined (AIX)
#define SYSDEPS_HAVE_GETRUSAGE 1
#include <sys/time.h> /* needed for Linux, exists on all four */
#include <sys/times.h> /* needed for AIX, exists on all four */
#include <sys/resource.h>
#endif

#if defined (__linux) && defined (CLOCK_THREAD_CPUTIME_ID)
#define SYSDEPS_HAVE_CLOCK_THREAD_CPUTIME 1
#endif

#if defined (INTEGRITY)
#include <sys/uio.h>
#include <limits.h>
#endif

#if defined (VXWORKS_RTP)
#include <net/uio.h>
#endif

#if defined (_WIN32)
typedef SOCKET os_handle;
#define Q_VALID_SOCKET(s) ((s) != INVALID_SOCKET)
#define Q_INVALID_SOCKET INVALID_SOCKET
#else /* All Unixes have socket() return -1 on error */
typedef int os_handle;
#define Q_VALID_SOCKET(s) ((s) != -1)
#define Q_INVALID_SOCKET -1
#endif

#if defined (__sun) && !defined (_XPG4_2)
#define SYSDEPS_MSGHDR_ACCRIGHTS 1
#else
#define SYSDEPS_MSGHDR_ACCRIGHTS 0
#endif
#if SYSDEPS_MSGHDR_ACCRIGHTS
#define SYSDEPS_MSGHDR_FLAGS 0
#else
#define SYSDEPS_MSGHDR_FLAGS 1
#endif

#if NN_HAVE_C99_INLINE && !defined SUPPRESS_SYSDEPS_INLINES
#include "sysdeps_template.c"
#else
#if defined (__cplusplus)
extern "C" {
#endif
os_uint32 atomic_inc_u32_nv (volatile os_uint32 *value);
os_uint32 atomic_dec_u32_nv (volatile os_uint32 *value);
void atomic_add_u32_noret (volatile os_uint32 *value, os_uint32 addend);
os_uint32 atomic_sub_u32_nv (volatile os_uint32 *value, os_uint32 subtrahend);
os_uint32 atomic_read_u32 (volatile const os_uint32 *loc);
void atomic_store_u32 (volatile os_uint32 *loc, os_uint32 new);

void pa_membar_enter (void);
void pa_membar_exit (void);
void pa_membar_producer (void);
void pa_membar_consumer (void);
#if defined (__cplusplus)
}
#endif
#endif

#if defined (__cplusplus)
extern "C" {
#endif

#define ASSERT_RDLOCK_HELD(x) ((void) 0)
#define ASSERT_WRLOCK_HELD(x) ((void) 0)
#define ASSERT_MUTEX_HELD(x) ((void) 0)

#if ! SYSDEPS_HAVE_IOVEC
struct iovec {
  void *iov_base;
  os_size_t iov_len;
};
#endif

#if ! SYSDEPS_HAVE_MSGHDR
struct msghdr
{
  void *msg_name;
  socklen_t msg_namelen;
  struct iovec *msg_iov;
  os_size_t msg_iovlen;
  void *msg_control;
  os_size_t msg_controllen;
  int msg_flags;
};
#endif

#ifndef MSG_TRUNC
#define MSG_TRUNC 1
#endif

#if ! SYSDEPS_HAVE_RECVMSG
/* Only implements iovec of length 1, no control */
os_ssize_t recvmsg (os_handle fd, struct msghdr *message, int flags);
#endif
#if ! SYSDEPS_HAVE_SENDMSG
os_ssize_t sendmsg (os_handle fd, const struct msghdr *message, int flags);
#endif
#if ! SYSDEPS_HAVE_RANDOM
long random (void);
#endif

os_int64 get_thread_cputime (void);

int os_threadEqual (os_threadId a, os_threadId b);

#if defined (__cplusplus)
}
#endif

#if !(defined __APPLE__ && defined __OPTIMIZE__ && NN_HAVE_C99_INLINE)
#define HAVE_ATOMIC_LIFO 0
#else
#define HAVE_ATOMIC_LIFO 1
#include <libkern/OSAtomic.h>
typedef OSQueueHead os_atomic_lifo_t;
NN_C99_INLINE void os_atomic_lifo_init (os_atomic_lifo_t *head) {
  OSQueueHead q = OS_ATOMIC_QUEUE_INIT;
  memcpy ((void *) head, (void *) &q, sizeof (*head));
}
NN_C99_INLINE void os_atomic_lifo_push (os_atomic_lifo_t *head, void *elem, os_size_t linkoff) {
  OSAtomicEnqueue (head, elem, linkoff);
}
NN_C99_INLINE void *os_atomic_lifo_pop (os_atomic_lifo_t *head, os_size_t linkoff) {
  return OSAtomicDequeue (head, linkoff);
}
#endif

#endif /* SYSDEPS_H */

/* SHA1 not available (unoffical build.) */

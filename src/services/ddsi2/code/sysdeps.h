#ifndef SYSDEPS_H
#define SYSDEPS_H

#include "os_defs.h"
#include "os_thread.h"
#include "os_socket.h"

#include "nn_inline.h"

#ifndef os_sockECONNRESET
#ifdef WSAECONNRESET
#define os_sockECONNRESET WSAECONNRESET
#else
#define os_sockECONNRESET ECONNRESET
#endif
#endif

#if defined (__linux) || defined (__sun) || defined (__APPLE__) || defined (INTEGRITY) || defined (AIX)
#define SYSDEPS_HAVE_IOVEC 1
#define SYSDEPS_HAVE_MSGHDR 1
#define SYSDEPS_HAVE_RECVMSG 1
#define SYSDEPS_HAVE_SENDMSG 1
#endif

#if defined (__linux) || defined (__sun) || defined (__APPLE__) || defined (AIX)
#define SYSDEPS_HAVE_RANDOM 1
#include <unistd.h>
#endif

#if defined (INTEGRITY)
#include <sys/uio.h>
#include <limits.h>
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

#if NN_HAVE_C99_INLINE
#include "sysdeps.template"
#else
os_uint32 atomic_inc_u32_nv (volatile os_uint32 *value);
os_uint32 atomic_dec_u32_nv (volatile os_uint32 *value);
void atomic_add_u32_noret (volatile os_uint32 *value, os_uint32 addend);
os_uint32 atomic_sub_u32_nv (volatile os_uint32 *value, os_uint32 subtrahend);
#endif

#if ! SYSDEPS_HAVE_IOVEC
struct iovec {
  void *iov_base;
  size_t iov_len;
};
#endif

#if ! SYSDEPS_HAVE_MSGHDR
struct msghdr
{
  void *msg_name;
  socklen_t msg_namelen;
  struct iovec *msg_iov;
  size_t msg_iovlen;
  void *msg_control;
  size_t msg_controllen;
  int msg_flags;
};
#endif

#ifndef MSG_TRUNC
#define MSG_TRUNC 1
#endif

#if ! SYSDEPS_HAVE_RECVMSG
/* Only implements iovec of length 1, no control */
ssize_t recvmsg (int fd, struct msghdr *message, int flags);
#endif
#if ! SYSDEPS_HAVE_SENDMSG
ssize_t sendmsg (int fd, const struct msghdr *message, int flags);
#endif
#if ! SYSDEPS_HAVE_RANDOM
long random (void);
#endif

int os_threadEqual (os_threadId a, os_threadId b);

#endif /* SYSDEPS_H */

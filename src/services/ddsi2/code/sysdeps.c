#include <stdlib.h>

#include "os_defs.h"
#include "os_if.h"
#include "os_heap.h"
#include "os_thread.h"
#include "os_abstract.h"
#include "os_socket.h"

#include "sysdeps.h"

#if ! NN_HAVE_C99_INLINE
#include "sysdeps.template"
#endif

/* MISSING IN OS ABSTRACTION LAYER ------------------------------------- */

int os_threadEqual (os_threadId a, os_threadId b)
{
  /* on pthreads boxes, pthread_equal (a, b); as a workaround: */
  os_ulong_int ai = os_threadIdToInteger (a);
  os_ulong_int bi = os_threadIdToInteger (b);
  return ai == bi;
}

#if ! SYSDEPS_HAVE_RECVMSG
ssize_t recvmsg (int fd, struct msghdr *message, int flags)
{
  ssize_t ret;
  assert (message->msg_iovlen == 1);
#if SYSDEPS_MSGHDR_ACCRIGHTS
  assert (message->msg_accrightslen == 0);
#else
  assert (message->msg_controllen == 0);
#endif
  ret = recvfrom (fd, message->msg_iov[0].iov_base, message->msg_iov[0].iov_len, flags,
                  message->msg_name, &message->msg_namelen);
  message->msg_flags = 0;
  return ret;
}
#endif

#if ! SYSDEPS_HAVE_SENDMSG
ssize_t sendmsg (int fd, const struct msghdr *message, int flags)
{
  char stbuf[2048], *buf;
  ssize_t sz, ret;
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
      sz += message->msg_iov[i].iov_len;
    if (sz <= sizeof (stbuf))
      buf = stbuf;
    else
    {
      if ((buf = os_malloc (sz)) == NULL)
        return -1;
    }
    /* ... then copy data into buffer */
    sz = 0;
    for (i = 0; i < message->msg_iovlen; i++)
    {
      memcpy (buf + sz, message->msg_iov[i].iov_base, message->msg_iov[i].iov_len);
      sz += message->msg_iov[i].iov_len;
    }
  }

  ret = sendto (fd, buf, sz, flags, message->msg_name, message->msg_namelen);
  if (buf != stbuf)
    os_free (buf);
  return ret;
}
#endif

#ifndef SYSDEPS_HAVE_RANDOM
long random (void)
{
  /* rand() is a really terribly bad PRNG */
  union { long x; unsigned char c[4]; } t;
  int i;
  for (i = 0; i < 4; i++)
    t.c[i] = (unsigned char) ((rand () >> 4) & 0xff);
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

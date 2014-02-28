#if defined (WIN32) || defined (OSPL_LINUX)
#define FD_SETSIZE 4096
#endif

#include <assert.h>
#include <stdlib.h>

#include "os_defs.h"
#include "os_if.h"
#include "os_heap.h"
#include "os_thread.h"
#include "os_socket.h"
#include "os_stdlib.h"

#include "q_sockwaitset.h"
#include "q_config.h"
#include "q_log.h"
#include "ddsi_tran.h"

#define WAITSET_DELTA 8

#if defined (VXWORKS_RTP) || defined (_WRS_KERNEL)
#include "pipeDrv.h"
#include "errno.h"
#include "ioLib.h"
#include "strings.h"
#include <selectLib.h>
#define OSPL_PIPENAMESIZE 26
#endif

#if defined (_WIN32)

static int pipe (os_handle fd[2])
{
  struct sockaddr_in addr;
  socklen_t asize = sizeof (addr);
  os_socket listener = socket (AF_INET, SOCK_STREAM, 0);
  os_socket s1 = socket (AF_INET, SOCK_STREAM, 0);
  os_socket s2 = Q_INVALID_SOCKET;

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
  addr.sin_port = 0;
  if (bind (listener, (struct sockaddr *) &addr, sizeof (addr)) == -1)
  {
    goto fail;
  }
  if (getsockname (listener, (struct sockaddr *) &addr, &asize) == -1)
  {
    goto fail;
  }
  if (listen (listener, 1) == -1)
  {
    goto fail;
  }
  if (connect (s1, (struct sockaddr *) &addr, sizeof (addr)) == -1)
  {
    goto fail;
  }
  if ((s2 = accept (listener, 0, 0)) == -1)
  {
    goto fail;
  }

  closesocket (listener);

  /* Equivalent to FD_CLOEXEC */

  SetHandleInformation ((HANDLE) s1, HANDLE_FLAG_INHERIT, 0);
  SetHandleInformation ((HANDLE) s2, HANDLE_FLAG_INHERIT, 0);

  fd[0] = s1;
  fd[1] = s2;

  return 0;

fail:

  closesocket (listener);
  closesocket (s1);
  closesocket (s2);

  return -1;
}

#define FD_COPY(s,d) (*(d) = *(s))

#else

#ifndef VXWORKS_RTP
#if defined (AIX) || defined (__Lynx__)
#include <fcntl.h>
#else
#if ! defined (_WRS_KERNEL) && ! defined(INTEGRITY)
#include <sys/fcntl.h>
#endif
#endif
#endif
#ifndef _WRS_KERNEL
#include <sys/select.h>
#endif
#ifdef __sun
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#endif /* _WIN32 */

#ifdef FD_COPY
#define WS_FD_COPY(nfds,src,dst) FD_COPY ((src), (dst))
#else
#define WS_FD_COPY(nfds,src,dst) my_fd_copy ((nfds), (src), (dst))

static void my_fd_copy (int nfds, const fd_set *src, fd_set *dst)
{
  /* Implement FD_COPY-like semantics using defined interface.  NFDS is
     as the first parameter of select() */
  int i;
  FD_ZERO (dst);
  for (i = 0; i < nfds; i++)
  {
    if (FD_ISSET (i, src))
    {
      FD_SET (i, dst);
    }
  }
}
#endif

struct os_sockWaitset_s
{
  int fdmax_plus_1;           /* value for first parameter of select() */
  unsigned events;            /* union of all requested events */
  unsigned sz;                /* max number of fds in waitset */
  unsigned n;                 /* actual number of fds in waitset */
  unsigned index;             /* cursor for enumerating, index in fds or sz */
  os_handle pipe[2];          /* pipe used for triggering */
  os_handle * fds;            /* file descriptors in set */
  ddsi_tran_base_t * conns;   /* connections and listeners in set */
  fd_set rdset, wrset;        /* fd sets used for select, enumerating */
  fd_set rdset0, wrset0;      /* working sets, copied into actual by wait */
};

os_sockWaitset os_sockWaitsetNew (void)
{
  const int sz = WAITSET_DELTA;
  os_sockWaitset ws = os_malloc (sizeof (*ws));

  ws->fds = os_malloc (sz * sizeof (*ws->fds));
  ws->conns = os_malloc (sz * sizeof (*ws->conns));
#if ! defined (_WIN32)
  ws->fdmax_plus_1 = 0;
#else
  ws->fdmax_plus_1 = FD_SETSIZE;
#endif
  ws->sz = sz;
  ws->n = 1;
  ws->events = OS_EVENT_READ;
  ws->index = ws->sz;
  FD_ZERO (&ws->rdset0);
  FD_ZERO (&ws->wrset0);

#if defined (VXWORKS_RTP) || defined (_WRS_KERNEL)
  {
    int result;
    char pipename[OSPL_PIPENAMESIZE];
    int pipecount=0;
    do
    {
      snprintf ((char*)&pipename, sizeof(pipename), "/pipe/ospl%d", pipecount++ );
    } 
    while ((result = pipeDevCreate ((char*) &pipename, 1, 1)) == -1 && errno == EINVAL);
    if (result != -1)
    {
      result = open ((char*) &pipename, O_RDWR, 0644);
      if (result != -1)
      {
        ws->pipe[0] = result;
        result =open ((char*) &pipename, O_RDWR, 0644);
        if (result != -1)
        {
          ws->pipe[1] = result;
        }
        else
        {
          close (ws->pipe[0]);
          pipeDevDelete (pipename, 0);
        }
      }
    }
    if (result == -1)
    {
       goto fail_pipe;
    }
  }
#else
  if (pipe (ws->pipe) == -1)
  {
    goto fail_pipe;
  }
#endif

  ws->fds[0] = ws->pipe[0];
  ws->conns[0] = NULL;

#if ! defined (VXWORKS_RTP) && ! defined ( _WRS_KERNEL ) && ! defined (_WIN32)
  fcntl (ws->pipe[0], F_SETFD, fcntl (ws->pipe[0], F_GETFD) | FD_CLOEXEC);
  fcntl (ws->pipe[1], F_SETFD, fcntl (ws->pipe[1], F_GETFD) | FD_CLOEXEC);
#endif
  FD_SET (ws->fds[0], &ws->rdset0);
#if ! defined (_WIN32)
  ws->fdmax_plus_1 = ws->fds[0] + 1;
#endif
  return ws;

fail_pipe:

  os_free (ws->fds);
  os_free (ws->conns);
  os_free (ws);

  return NULL;
}

static void os_sockWaitsetGrow (os_sockWaitset ws)
{
  ws->sz += WAITSET_DELTA;
  ws->conns = os_realloc (ws->conns, ws->sz * sizeof (*ws->conns));
  ws->fds = os_realloc (ws->fds, ws->sz * sizeof (*ws->fds));
  ws->index = ws->sz;
}

void os_sockWaitsetFree (os_sockWaitset ws)
{
#ifdef VXWORKS_RTP
  char nameBuf[OSPL_PIPENAMESIZE];
  ioctl (ws->pipe[0], FIOGETNAME, &nameBuf);
#endif
#if defined (_WIN32)
  closesocket (ws->pipe[0]);
  closesocket (ws->pipe[1]);
#else
  close (ws->pipe[0]);
  close (ws->pipe[1]);
#endif
#ifdef VXWORKS_RTP
  pipeDevDelete ((char*) &nameBuf, 0);
#endif
  os_free (ws->fds);
  os_free (ws->conns);
  os_free (ws);
}

os_result os_sockWaitsetTrigger (os_sockWaitset ws)
{
  char buf = 0;
  int n;
  int err;

#if defined (_WIN32)
  n = send (ws->pipe[1], &buf, 1, 0);
#else
  n = (int) write (ws->pipe[1], &buf, 1);
#endif
  if (n != 1)
  {
    err = os_sockError ();
    NN_WARNING1 ("os_sockWaitsetWait: read failed on trigger pipe, errno = %d", err);
    return os_resultFail;
  }
  return os_resultSuccess;
}

os_result os_sockWaitsetAdd
(
  os_sockWaitset ws,
  ddsi_tran_base_t base,
  unsigned events
)
{
  os_handle handle = ddsi_tran_handle (base);

  if 
  (
    (handle < 0)
#if ! defined (_WIN32)
    || (handle >= FD_SETSIZE)
#endif
  )
  {
    return os_resultFail;
  }
  if (ws->n == ws->sz)
  {
    os_sockWaitsetGrow (ws);
  }
  ws->events |= events;
  if (events & OS_EVENT_READ)
  {
    FD_SET (handle, &ws->rdset0);
  }
  if (events & OS_EVENT_WRITE)
  {
    FD_SET (handle, &ws->wrset0);
  }
#if ! defined (_WIN32)
  if ((int) handle >= ws->fdmax_plus_1)
  {
    ws->fdmax_plus_1 = handle + 1;
  }
#endif
  ws->conns[ws->n] = base;
  ws->fds[ws->n] = handle;
  ws->n++;
  return os_resultSuccess;
}

void os_sockWaitsetPurge (os_sockWaitset ws, unsigned index)
{
  unsigned i;

  if ((ws->index != ws->sz) || (index + 1 > ws->n))
  {
    return;
  }

  for (i = index + 1; i < ws->n; i++)
  {
    FD_CLR (ws->fds[i], &ws->rdset0);
    FD_CLR (ws->fds[i], &ws->wrset0);
  }
  ws->n = index + 1;
}

void os_sockWaitsetRemove (os_sockWaitset ws, ddsi_tran_base_t base)
{
  unsigned i;

  for (i = 0; i < ws->n; i++)
  {
    if (base == ws->conns[i])
    {
      FD_CLR (ws->fds[i], &ws->rdset0);
      FD_CLR (ws->fds[i], &ws->wrset0);
      ws->n--;
      if (i != ws->n)
      {
        ws->fds[i] = ws->fds[ws->n];
        ws->conns[i] = ws->conns[ws->n];
      }
      ddsi_tran_free (base);
      break;
    }
  }
}

os_result os_sockWaitsetWait (os_sockWaitset ws, int timeout_ms)
{
  struct timeval sto;
  struct timeval *to = NULL;
  fd_set *rdset = NULL;
  fd_set *wrset = NULL;
  int n;
  int err;

  assert (-1 <= timeout_ms && timeout_ms < 1000);
  assert (0 < ws->n && ws->n <= ws->sz);
  assert (ws->index == ws->sz);
#if ! defined (_WIN32)
  assert (ws->fdmax_plus_1 > 0);
#endif

  if (timeout_ms > 0)
  {
    sto.tv_sec = 0;
    sto.tv_usec = 1000 * timeout_ms;
    to = &sto;
  }

  if (ws->events & OS_EVENT_READ)
  {
    rdset = &ws->rdset;
    WS_FD_COPY (ws->fdmax_plus_1, &ws->rdset0, rdset);
  }
  if (ws->events & OS_EVENT_WRITE)
  {
    wrset = &ws->wrset;
    WS_FD_COPY (ws->fdmax_plus_1, &ws->wrset0, wrset);
  }

  do
  {
    n = select (ws->fdmax_plus_1, rdset, wrset, NULL, to);
    if (n == -1)
    {
      err = os_sockError ();
      if ((err != os_sockEINTR) && (err != os_sockEAGAIN)) {
        NN_WARNING1 ("os_sockWaitsetWait: select failed, errno = %d", err);
      }
    }
  }
  while ((n == -1) && ((err == os_sockEINTR) || (err == os_sockEAGAIN)));

  if (n == 0)
  {
    return os_resultTimeout;
  }
  else if (n > 0)
  {
    /* this simply skips the trigger fd */
    ws->index = 1;
    if (FD_ISSET (ws->fds[0], rdset))
    {
      char buf;
      int n;
#if defined (_WIN32)
      n = recv (ws->fds[0], &buf, 1, 0);
#else
      n = (int) read (ws->fds[0], &buf, 1);
#endif
      if (n != 1)
      {
        err = os_sockError ();
        NN_WARNING1 ("os_sockWaitsetWait: read failed on trigger pipe, errno = %d", err);
        assert (0);
      }
    }
    return os_resultSuccess;
  }
  else if (err == os_sockEBADF)
  {
    /* EBADF happens when a socket is closed but that's a valid use case for DDSI2 */
    return os_resultTimeout;
  }

  return os_resultTimeout;
}

int os_sockWaitsetNextEvent
(
  os_sockWaitset ws,
  ddsi_tran_base_t * base,
  unsigned * events
)
{
  assert (ws->index >= 1);
  while (ws->index < ws->n)
  {
    int idx = ws->index++;
    os_handle fd = ws->fds[idx];
    unsigned ev = /* counting on the compiler to do this efficiently */
      ((ws->events & OS_EVENT_READ) && (FD_ISSET (fd, &ws->rdset) ? OS_EVENT_READ : 0)) |
      ((ws->events & OS_EVENT_WRITE) && (FD_ISSET (fd, &ws->wrset) ? OS_EVENT_WRITE : 0));
    if (ev)
    {
      *base = ws->conns[idx];
      *events = ev;
      return idx - 1;
    }
  }
  ws->index = ws->sz;
  return -1;
}

/* SHA1 not available (unoffical build.) */

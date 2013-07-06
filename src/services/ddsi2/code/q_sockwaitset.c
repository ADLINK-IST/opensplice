#include <assert.h>
#include <stdlib.h>

#include "os_defs.h"
#include "os_if.h"
#include "os_heap.h"
#include "os_thread.h"
#include "os_socket.h"
#include "os_report.h"

#include "q_sockwaitset.h"
#include "q_config.h"

#include "os_stdlib.h"

#if defined ( VXWORKS_RTP ) || defined ( _WRS_KERNEL )
#include "pipeDrv.h"
#include "errno.h"
#include "ioLib.h"
#include "strings.h"
#include <selectLib.h>
#define OSPL_PIPENAMESIZE 26
#endif

#if defined (_WIN32)

#ifndef WSA_INVALID_EVENT /* for WinCE -- needed according to the docs, haven't checked with reality */
#define WSA_INVALID_EVENT 0
#endif

struct os_sockWaitset {
  int sz;                       /* allocated size */
  int n;                        /* sockets/events [ 0 .. n-1 ] are occupied */
  int index;                    /* index that triggered the last wakeup, or -1 */
  int interruptible;            /* 0 if not interruptible, 1 if it is */
  os_socket *sockets;           /* array of sockets caller cares for */
  WSAEVENT *events;             /* array of WSAEVENTs associated with those sockets */
};

os_sockWaitset os_sockWaitsetNew (int interruptible)
{
  const int sz = 8; /* initial size, arbitrary value but must be >= !!interruptible */
  os_sockWaitset ws;
  if ((ws = os_malloc (sizeof (*ws))) == NULL)
    goto fail;
  if ((ws->sockets = os_malloc (sz * sizeof (*ws->sockets))) == NULL)
    goto fail_sockets;
  if ((ws->events = os_malloc (sz * sizeof (*ws->events))) == NULL)
    goto fail_events;
  ws->interruptible = !!interruptible;
  ws->sz = sz;
  ws->n = ws->interruptible;
  ws->index = -1;
  if (interruptible)
  {
    if ((ws->events[0] = WSACreateEvent ()) == WSA_INVALID_EVENT)
      goto fail_trigger;
    else
      ws->sockets[0] = INVALID_SOCKET;
  }
  return ws;

 fail_trigger:
  os_free (ws->events);
 fail_events:
  os_free (ws->sockets);
 fail_sockets:
  os_free (ws);
 fail:
  return NULL;
}

static os_result os_sockWaitsetGrow (os_sockWaitset ws, int sz)
{
  int i;
  os_socket *sockets;
  WSAEVENT *events;
  assert (sz > ws->sz);
  if ((sockets = os_malloc (sz * sizeof (*sockets))) == NULL)
    return os_resultFail;
  if ((events = os_malloc (sz * sizeof (*events))) == NULL)
  {
    os_free (sockets);
    return os_resultFail;
  }
  for (i = 0; i < ws->n; i++)
  {
    sockets[i] = ws->sockets[i];
    events[i] = ws->events[i];
  }
  os_free (ws->sockets);
  os_free (ws->events);
  ws->sz = sz;
  ws->sockets = sockets;
  ws->events = events;
  return os_resultSuccess;
}

void os_sockWaitsetFree (os_sockWaitset ws)
{
  assert (ws->n <= ws->sz);
  assert (!ws->interruptible || ws->n > 0);
  os_sockWaitsetRemoveSockets (ws, 0);
  if (ws->interruptible)
    WSACloseEvent (ws->events[0]);
  os_free (ws->sockets);
  os_free (ws->events);
  os_free (ws);
}

os_result os_sockWaitsetTrigger (os_sockWaitset ws)
{
  if (!ws->interruptible)
    return os_resultInvalid;
  assert (ws->n > 0);
  assert (ws->sockets[0] == INVALID_SOCKET);
  if (!WSASetEvent (ws->events[0])) {
    OS_REPORT_2 (OS_WARNING, config.servicename, 0, "os_sockWaitsetTrigger: WSASetEvent(%x) failed, error %d", (os_uint32) ws->events[0], WSAGetLastError ());
    return os_resultFail;
  }
  return os_resultSuccess;
}

os_result os_sockWaitsetAddSocket (os_sockWaitset ws, os_socket sock, unsigned events)
{
  unsigned event_flags;

  WSAEVENT ev;
  if (events == 0 || (events & ~(OS_SOCKEVENT_READ | OS_SOCKEVENT_WRITE)) != 0)
    return os_resultInvalid;
  if (ws->n == MAXIMUM_WAIT_OBJECTS)
    return os_resultFail;
  if (ws->n == ws->sz)
  {
    os_result res;
    if ((res = os_sockWaitsetGrow (ws, ws->sz + 8)) != os_resultSuccess)
      return res;
  }
  assert (ws->n < ws->sz);
#ifndef NDEBUG
  {
    int i;
    for (i = 0; i < ws->n; i++)
      if (ws->sockets[i] == sock)
        return os_resultInvalid;
  }
#endif
  event_flags = 0;
  if (events & OS_SOCKEVENT_READ)
    event_flags |= FD_READ;
  if (events & OS_SOCKEVENT_WRITE)
    event_flags |= FD_WRITE;
  if ((ev = WSACreateEvent ()) == WSA_INVALID_EVENT)
  {
    OS_REPORT_1 (OS_WARNING, config.servicename, 0, "os_sockWaitsetAddSocket: WSACreateEvent failed, error %d", WSAGetLastError ());
    goto fail_event;
  }
  if (WSAEventSelect (sock, ev, event_flags) == SOCKET_ERROR)
  {
    OS_REPORT_4 (OS_WARNING, config.servicename, 0, "os_sockWaitsetAddSocket: WSAEventSelect(%x,%x,%x) failed, error %d", (os_uint32) sock, (os_uint32) ev, event_flags, WSAGetLastError ());
    goto fail_select;
  }
  ws->sockets[ws->n] = sock;
  ws->events[ws->n] = ev;
  ws->n++;
  return os_resultSuccess;

 fail_select:
  WSACloseEvent (ev);
 fail_event:
  return os_resultFail;
}

os_result os_sockWaitsetRemoveSockets (os_sockWaitset ws, int index)
{
  int i;
  if (index < 0 || index + ws->interruptible > ws->n)
    return os_resultInvalid;
  for (i = index + ws->interruptible; i < ws->n; i++)
  {
    assert (ws->sockets[i] != INVALID_SOCKET);
    if (WSAEventSelect (ws->sockets[i], 0, 0) == SOCKET_ERROR)
    {
      int err = WSAGetLastError ();
      if (err != WSAENOTSOCK)
      {
        /* Closing a socket, then removing it from the waitset may (or
           may not, depends on whether the sockets are reference
           counted ...) cause this operation to fail, but that is
           expected. */
        OS_REPORT_2 (OS_WARNING, config.servicename, 0, "os_sockWaitsetFree: WSAEventSelect(%x, 0, 0): error %d", (os_uint32) ws->sockets[i], err);
      }
    }
    if (!WSACloseEvent (ws->events[i]))
      OS_REPORT_2 (OS_WARNING, config.servicename, 0, "os_sockWaitsetFree: WSACloseEvent(%x failed, error %d", (os_uint32) ws->events[i], WSAGetLastError ());
  }
  ws->n = index + ws->interruptible;
  return os_resultSuccess;
}

os_result os_sockWaitsetWait (os_sockWaitset ws, int timeout_ms)
{
  int idx;

  assert (-1 <= timeout_ms && timeout_ms < 1000);
  assert (0 < ws->n - ws->interruptible && ws->n <= ws->sz);
  assert (ws->index == -1);

  /* Hopefully a good compiler will optimise this away if WSA_INFINITE
     == -1; behaviour for timeout_ms >= 0 is ok. */
  if (timeout_ms == -1)
    timeout_ms = WSA_INFINITE;

  if ((idx = WSAWaitForMultipleEvents (ws->n, ws->events, FALSE, timeout_ms, FALSE)) == WSA_WAIT_FAILED)
  {
    OS_REPORT_3 (OS_WARNING, config.servicename, 0, "os_sockWaitsetWait: WSAWaitForMultipleEvents(%d,...,0,%d,0) failed, error %d", ws->n, timeout_ms, WSAGetLastError ());
    return os_resultFail;
  }

#ifndef WAIT_IO_COMPLETION /* curious omission in the WinCE headers */
#define TEMP_DEF_WAIT_IO_COMPLETION
#define WAIT_IO_COMPLETION 0xc0L
#endif
  if (idx >= WSA_WAIT_EVENT_0 && idx < WSA_WAIT_EVENT_0 + ws->n) {
    ws->index = idx - WSA_WAIT_EVENT_0;
    if (ws->interruptible && ws->index == 0)
    {
      /* pretend a spurious wakeup */
      WSAResetEvent (ws->events[0]);
      ws->index = -1;
    }
    return os_resultSuccess;
  } else if (idx == WSA_WAIT_TIMEOUT) {
    return os_resultTimeout;
  } else if (idx == WAIT_IO_COMPLETION) {
    /* Presumably can't happen with alertable = FALSE */
    OS_REPORT_2 (OS_WARNING, config.servicename, 0, "os_sockWaitsetWait: WSAWaitForMultipleEvents(%d,...,0,%d,0) returned unexpected WAIT_IO_COMPLETION", ws->n, timeout_ms);
    return os_resultTimeout;
  } else {
    OS_REPORT_3 (OS_WARNING, config.servicename, 0, "os_sockWaitsetWait: WSAWaitForMultipleEvents(%d,...,0,%d,0) returned unrecognised %d", ws->n, timeout_ms, idx);
    return os_resultTimeout;
  }
#ifdef TEMP_DEF_WAIT_IO_COMPLETION
#undef WAIT_IO_COMPLETION
#undef TEMP_DEF_WAIT_IO_COMPLETION
#endif
}

/* This implementation follows the pattern of simply looking at the
   socket that triggered the wakeup; alternatively, one could scan the
   entire set as we do for select().  If the likelihood of two sockets
   having an event simultaneously is small, this is better, but if it
   is large, the lower indices may get a disproportionally large
   amount of attention. */
int os_sockWaitsetNextEvent (os_sockWaitset ws, os_socket *sock, unsigned *events)
{
  assert (-1 <= ws->index && ws->index < ws->n);
  assert (0 < ws->n && ws->n <= ws->sz);
  if (ws->index == -1)
    return -1;
  else
  {
    WSANETWORKEVENTS nwev;
    int idx = ws->index;
    ws->index = -1;
    assert (idx >= ws->interruptible);
    assert (ws->sockets[idx] != INVALID_SOCKET);
    if (WSAEnumNetworkEvents (ws->sockets[idx], ws->events[idx], &nwev) == SOCKET_ERROR)
    {
      int err = WSAGetLastError ();
      if (err != WSAENOTSOCK)
      {
        /* May have a wakeup and a close in parallel, so the handle
           need not exist anymore. */
        OS_REPORT_3 (OS_ERROR, config.servicename, 0, "os_sockWaitsetNextEvent: WSAEnumNetworkEvents(%x,%x,...) failed, error %d", (os_uint32) ws->sockets[idx], (os_uint32) ws->events[idx], err);
      }
      return -1;
    }

    *sock = ws->sockets[idx];
#if FD_READ == OS_SOCKEVENT_READ && FD_WRITE == OS_SOCKEVENT_WRITE
    *events = nwev.lNetworkEvents;
#else
    *events = 0;
    if (nwev.lNetworkEvents & FD_READ)
      *events |= OS_SOCKEVENT_READ;
    if (nwev.lNetworkEvents & FD_WRITE)
      *events |= OS_SOCKEVENT_WRITE;
#endif
    return idx - ws->interruptible;
  }
}

#else /* select() based -- should have a poll-based one as well */

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
/* Some have FD_COPY, some don't; some have a huge value for FD_SETSIZE,
   some not ...  It's supposed to be a macro when it exists, it seems. */
#ifdef FD_COPY
#define MY_FD_COPY(nfds, src, dst) FD_COPY ((src), (dst))
#else
#define MY_FD_COPY(nfds, src, dst) my_fd_copy ((nfds), (src), (dst))
#endif

#define OS_SOCKEVENT_READ 1u
#define OS_SOCKEVENT_WRITE 2u

struct os_sockWaitset {
  int maxsock_plus_1;           /* value for first parameter of select() */
  int interruptible;            /* 0 or 1, depending on whether it is interruptible */
  unsigned events;              /* union of all requested events */
  int sz;                       /* max number of fds in waitset */
  int n;                        /* actual number of fds in waitset */
  int index;                    /* cursor for enumerating, index in fds or SZ */
  int pipe[2];                  /* pipe used for triggering */
  int *fds;                     /* file descriptors in set */
  fd_set rdset, wrset;          /* fd sets used for select, enumerating */
  fd_set rdset0, wrset0;        /* sets to which fds are added, copied into rdset, wrset by wait */
};

#ifndef FD_COPY
static void my_fd_copy (int nfds, const fd_set *src, fd_set *dst)
{
  /* Implement FD_COPY-like semantics using defined interface.  NFDS is
     as the NFDS parameter of select() */
  int i;
  FD_ZERO (dst);
  for (i = 0; i < nfds; i++)
    if (FD_ISSET (i, src))
      FD_SET (i, dst);
}
#endif

os_sockWaitset os_sockWaitsetNew (int interruptible)
{
  const int sz = 8; /* initial size, arbitrary value but must be >= !!interruptible */
  os_sockWaitset ws;
  if ((ws = os_malloc (sizeof (*ws))) == NULL)
    goto fail;
  if ((ws->fds = os_malloc (sz * sizeof (*ws->fds))) == NULL)
    goto fail_fds;
  ws->interruptible = !!interruptible;
  ws->maxsock_plus_1 = 0;
  ws->sz = sz;
  ws->n = ws->interruptible;
  ws->events = ws->interruptible ? OS_SOCKEVENT_READ : 0;
  ws->index = ws->sz;
  FD_ZERO (&ws->rdset0);
  FD_ZERO (&ws->wrset0);
  if (interruptible)
  {
#if !defined ( VXWORKS_RTP ) && !defined ( _WRS_KERNEL )
    if (pipe (ws->pipe) == -1)
      goto fail_pipe;
#else
    {
      int result;
      char pipename[OSPL_PIPENAMESIZE];
      int pipecount=0;
      do
      {
        snprintf( (char *)&pipename, sizeof(pipename), "/pipe/ospl%d", pipecount++ );
      } while ((result = pipeDevCreate ((char *)&pipename, 1, 1)) == -1 && errno == EINVAL);
      if ( result != -1 )
      {
        result = open((char *)&pipename, O_RDWR, 0644);
        if ( result != -1 )
        {
          ws->pipe[0] = result;
          result =open((char *)&pipename, O_RDWR, 0644);
          if ( result != -1 )
          {
            ws->pipe[1] = result;
          }
          else
          {
            close( ws->pipe[0] );
            pipeDevDelete( pipename, 0 );
          }
        }
      }
      if ( result == -1 )
      {
        goto fail_pipe;
      }
    }
#endif
    ws->fds[0] = ws->pipe[0];
#ifndef VXWORKS_RTP
    {
      int i;
      for (i = 0; i < 2; i++)
        fcntl (ws->pipe[i], F_SETFD, fcntl (ws->pipe[i], F_GETFD) | FD_CLOEXEC);
    }
#endif
    FD_SET (ws->fds[0], &ws->rdset0);
  }
  return ws;

 fail_pipe:
  os_free (ws->fds);
 fail_fds:
  os_free (ws);
 fail:
  return NULL;
}

static os_result os_sockWaitsetGrow (os_sockWaitset ws, int sz)
{
  int *fds;
  assert (sz > ws->sz);
  if (ws->index != ws->sz)
    return os_resultInvalid;
  if ((fds = os_realloc (ws->fds, sz * sizeof (*ws->fds))) == NULL)
    return os_resultFail;
  ws->fds = fds;
  ws->sz = sz;
  ws->index = sz;
  return os_resultSuccess;
}

void os_sockWaitsetFree (os_sockWaitset ws)
{
  if (ws->interruptible)
  {
#ifdef VXWORKS_RTP
    char nameBuf[OSPL_PIPENAMESIZE];
    ioctl (ws->pipe[0], FIOGETNAME, &nameBuf);
#endif
    close (ws->pipe[0]);
    close (ws->pipe[1]);
#ifdef VXWORKS_RTP
    pipeDevDelete( (char *)&nameBuf, 0 );
#endif
  }
  os_free (ws->fds);
  os_free (ws);
}

os_result os_sockWaitsetTrigger (os_sockWaitset ws)
{
  char buf = 0;
  int n;
  if (!ws->interruptible)
    return os_resultInvalid;
  if ((n = write (ws->pipe[1], &buf, 1)) != 1)
  {
    /* as far as I know can't fail unless someone's doing bad things */
    OS_REPORT_2 (OS_WARNING, config.servicename, 0, "os_sockWaitsetWait: read failed on trigger pipe, n = %d, errno = %d", n, errno);
    return os_resultFail;
  }
  return os_resultSuccess;
}

os_result os_sockWaitsetAddSocket (os_sockWaitset ws, os_socket sock, unsigned events)
{
  if (events == 0 || (events & ~(OS_SOCKEVENT_READ | OS_SOCKEVENT_WRITE)) != 0)
    return os_resultInvalid;
  if (sock < 0 || sock >= FD_SETSIZE)
    return os_resultFail;
  if (ws->n == ws->sz)
  {
    os_result res;
    if ((res = os_sockWaitsetGrow (ws, ws->sz + 8)) != os_resultSuccess)
      return res;
  }
  ws->events |= events;
  if (events & OS_SOCKEVENT_READ)
    FD_SET (sock, &ws->rdset0);
  if (events & OS_SOCKEVENT_WRITE)
    FD_SET (sock, &ws->wrset0);
  if (sock >= ws->maxsock_plus_1)
    ws->maxsock_plus_1 = sock + 1;
  ws->fds[ws->n++] = sock;
  return os_resultSuccess;
}

os_result os_sockWaitsetRemoveSockets (os_sockWaitset ws, int index)
{
  int i;
  if (ws->index != ws->sz)
    return os_resultInvalid;
  if (index < 0 || index + ws->interruptible > ws->n)
    return os_resultInvalid;
  for (i = index + ws->interruptible; i < ws->n; i++)
  {
    FD_CLR (ws->fds[i], &ws->rdset0);
    FD_CLR (ws->fds[i], &ws->wrset0);
  }
  ws->n = index + ws->interruptible;
  return os_resultSuccess;
}

os_result os_sockWaitsetWait (os_sockWaitset ws, int timeout_ms)
{
  struct timeval sto, *to;
  fd_set *rdset, *wrset;
  int n;

  assert (-1 <= timeout_ms && timeout_ms < 1000);
  assert (0 < ws->n && ws->n <= ws->sz);
  assert (ws->index == ws->sz);
  assert (ws->maxsock_plus_1 > 0);

  if (timeout_ms == -1)
    to = NULL;
  else
  {
    sto.tv_sec = 0;
    sto.tv_usec = 1000 * timeout_ms;
    to = &sto;
  }

  if (!(ws->events & OS_SOCKEVENT_READ))
    rdset = NULL;
  else
  {
    rdset = &ws->rdset;
    MY_FD_COPY (ws->maxsock_plus_1, &ws->rdset0, rdset);
  }
  if (!(ws->events & OS_SOCKEVENT_WRITE))
    wrset = NULL;
  else
  {
    wrset = &ws->wrset;
    MY_FD_COPY (ws->maxsock_plus_1, &ws->wrset0, wrset);
  }

  if ((n = select (ws->maxsock_plus_1, rdset, wrset, NULL, to)) == 0) {
    return os_resultTimeout;
  } else if (n > 0) {
    /* interruptible = 0 or 1 => this simply skips the trigger fd */
    ws->index = ws->interruptible;
    if (ws->interruptible && FD_ISSET (ws->fds[0], rdset))
    {
      char buf;
      int n;
      if ((n = read (ws->fds[0], &buf, 1)) != 1)
      {
        /* as far as I know can't fail: pipe with waiting data always returns */
        OS_REPORT_2 (OS_WARNING, config.servicename, 0, "os_sockWaitsetWait: read failed on trigger pipe, n = %d, errno = %d", n, errno);
        assert (0);
      }
    }
    return os_resultSuccess;
  } else if (errno == EAGAIN || errno == EINTR || errno == EBADF) {
    /* EBADF happens when a socket is closed but that's a valid use case for DDSI2 */
    return os_resultTimeout;
  } else {
    OS_REPORT_1 (OS_WARNING, config.servicename, 0, "os_sockWaitsetWait: select failed, errno = %d", errno);
    return os_resultTimeout;
  }
}

int os_sockWaitsetNextEvent (os_sockWaitset ws, os_socket *sock, unsigned *events)
{
  assert (ws->index >= ws->interruptible);
  while (ws->index < ws->n)
  {
    int idx = ws->index++;
    int fd = ws->fds[idx];
    unsigned ev = /* counting on the compiler to do this efficiently */
      ((ws->events & OS_SOCKEVENT_READ) && (FD_ISSET (fd, &ws->rdset) ? OS_SOCKEVENT_READ : 0)) |
      ((ws->events & OS_SOCKEVENT_WRITE) && (FD_ISSET (fd, &ws->wrset) ? OS_SOCKEVENT_WRITE : 0));
    if (ev)
    {
      *sock = fd;
      *events = ev;
      return idx - ws->interruptible;
    }
  }
  ws->index = ws->sz;
  return -1;
}
#endif

/* SHA1 not available (unoffical build.) */

/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>

#include "os_stdlib.h"
#include "os_heap.h"

#include "q_config.h"
#include "q_globals.h"
#include "q_thread.h"
#include "q_time.h"
#include "q_log.h"

#define MAX_TIMESTAMP_LENGTH (10 + 1 + 6)
#define MAX_TID_LENGTH 10
#define MAX_HDR_LENGTH (MAX_TIMESTAMP_LENGTH + 1 + MAX_TID_LENGTH +  + 2)

#define BUF_OFFSET MAX_HDR_LENGTH

static void logbuf_flush_real (struct thread_state1 *self, logbuf_t lb)
{
  if (config.tracingOutputFile != NULL)
  {
    const char *tname = self ? self->name : "(anon)";
    char hdr[MAX_HDR_LENGTH + 1];
    int n, tsec, tusec;
    if (lb->tstamp < 0)
      lb->tstamp = now ();
    time_to_sec_usec (&tsec, &tusec, lb->tstamp);
    lb->tstamp = -1;
    n = snprintf (hdr, sizeof (hdr), "%d.%06d/%*.*s: ", tsec, tusec, MAX_TID_LENGTH, MAX_TID_LENGTH, tname);
    assert (0 < n && n <= BUF_OFFSET);
    memcpy (lb->buf + BUF_OFFSET - n, hdr, n);
    fwrite (lb->buf + BUF_OFFSET - n, 1, lb->pos - BUF_OFFSET + n, config.tracingOutputFile);
    fflush (config.tracingOutputFile);
  }
  lb->pos = BUF_OFFSET;
  lb->buf[lb->pos] = 0;
}

static void logbuf_flush (struct thread_state1 *self, logbuf_t lb)
{
  if (lb->pos > BUF_OFFSET)
  {
    if (lb->pos < (int) sizeof (lb->buf))
      lb->buf[lb->pos++] = '\n';
    else
      lb->buf[sizeof (lb->buf) - 1] = '\n';
    logbuf_flush_real (self, lb);
  }
}

void logbuf_init (logbuf_t lb)
{
  lb->bufsz = sizeof (lb->buf);
  lb->pos = BUF_OFFSET;
  lb->tstamp = -1;
  lb->buf[lb->pos] = 0;
}

logbuf_t logbuf_new (void)
{
  logbuf_t lb = os_malloc (sizeof (*lb));
  logbuf_init (lb);
  return lb;
}

void logbuf_free (logbuf_t lb)
{
  logbuf_flush (lookup_thread_state (), lb);
  os_free (lb);
}

/* LOGGING ROUTINES */

static void nn_vlogb (struct thread_state1 *self, const char *fmt, va_list ap)
{
  int n, trunc = 0, nrem;
  logbuf_t lb;
  if (*fmt == 0)
    return;
  if (self && self->lb)
    lb = self->lb;
  else
  {
    lb = &gv.static_logbuf;
    if (gv.static_logbuf_lock_inited)
    {
      /* not supposed to be multi-threaded when mutex not
         initialized */
      os_mutexLock (&gv.static_logbuf_lock);
    }
  }

  nrem = lb->bufsz - lb->pos;
  if (nrem > 0)
  {
    n = os_vsnprintf (lb->buf + lb->pos, nrem, fmt, ap);
    if (n < nrem)
      lb->pos += n;
    else
    {
      lb->pos += nrem;
      trunc = 1;
    }
    if (trunc)
    {
      static const char msg[] = "(trunc)\n";
      const int msglen = (int) sizeof (msg) - 1;
      assert (lb->pos <= lb->bufsz);
      assert (lb->pos >= msglen);
      memcpy (lb->buf + lb->pos - msglen, msg, msglen);
    }
  }
  if (fmt[strlen (fmt) - 1] == '\n')
  {
    logbuf_flush_real (self, lb);
  }

  if (lb == &gv.static_logbuf && gv.static_logbuf_lock_inited)
  {
    os_mutexUnlock (&gv.static_logbuf_lock);
  }
}

int nn_vlog (logcat_t cat, const char *fmt, va_list ap)
{
  if (config.enabled_logcats & cat)
  {
    struct thread_state1 *self = lookup_thread_state ();
    nn_vlogb (self, fmt, ap);
  }
  return 0;
}

int nn_log (logcat_t cat, const char *fmt, ...)
{
  if (config.enabled_logcats & cat)
  {
    struct thread_state1 *self = lookup_thread_state ();
    va_list ap;
    va_start (ap, fmt);
    nn_vlogb (self, fmt, ap);
    va_end (ap);
  }
  return 0;
}

int nn_trace (const char *fmt, ...)
{
  if (config.enabled_logcats & LC_TRACE)
  {
    struct thread_state1 *self = lookup_thread_state ();
    va_list ap;
    va_start (ap, fmt);
    nn_vlogb (self, fmt, ap);
    va_end (ap);
  }
  return 0;
}

void nn_log_set_tstamp (os_int64 tnow)
{
  struct thread_state1 *self = lookup_thread_state ();
  if (self && self->lb)
    self->lb->tstamp = tnow;
}

/* SHA1 not available (unoffical build.) */

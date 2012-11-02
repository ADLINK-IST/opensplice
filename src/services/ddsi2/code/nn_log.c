/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
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

#include "os_stdlib.h"
#include "os_report.h"

#include "nn_config.h"
#include "nn_log.h"

int nn_vlogb (logbuf_t lb, logcat_t cat, const char *fmt, va_list ap)
{
  int n, trunc = 0, nrem;
  lb->included_logcats |= cat;
  nrem = lb->bufsz - lb->pos;
  if (nrem == 0)
    return 0;
  if (!(config.enabled_logcats & cat))
    return 0;
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
  return n;
}

int nn_logb (logbuf_t lb, logcat_t cat, const char *fmt, ...)
{
  va_list ap;
  int n;
  va_start (ap, fmt);
  n = nn_vlogb (lb, cat, fmt, ap);
  va_end (ap);
  return n;
}

int nn_logb_flush (logbuf_t lb)
{
  int n = 0;

  if (config.tracingOutputFile != NULL && (config.enabled_logcats & lb->included_logcats))
  {
    n = fwrite (lb->buf, 1, lb->pos, config.tracingOutputFile);
    fflush (config.tracingOutputFile);
  }

  /* FATAL, ERROR and WARNING are always reported throught the central
     reporting system. The report context I presume to be the service
     name, meaningful file & line numbers are unavailable, and a
     reportCode I don't know either ... */
  if (lb->included_logcats & (LC_FATAL | LC_ERROR | LC_WARNING))
  {
    os_reportType rt = OS_INFO;
    if (lb->included_logcats & LC_FATAL)
      rt = OS_FATAL;
    else if (lb->included_logcats & LC_ERROR)
      rt = OS_ERROR;
    else if (lb->included_logcats & LC_WARNING)
      rt = OS_WARNING;
    assert (rt != OS_INFO);
    os_report (rt, config.servicename, __FILE__, __LINE__, 0, "%s", lb->buf);
  }

  lb->included_logcats = 0;
  lb->pos = 0;
  return n;
}

int nn_vlog (logcat_t cat, const char *fmt, va_list ap)
{
  int n;
  LOGBUF_DECLNEW (lb);
  n = nn_vlogb (lb, cat, fmt, ap);
  nn_logb_flush (lb);
  LOGBUF_FREE (lb);
  return n;
}

int nn_log (logcat_t cat, const char *fmt, ...)
{
  va_list ap;
  int n;
  va_start (ap, fmt);
  n = nn_vlog (cat, fmt, ap);
  va_end (ap);
  return n;
}

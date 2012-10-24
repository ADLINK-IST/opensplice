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
#ifndef NN_LOG_H
#define NN_LOG_H

#include <stdarg.h>

#define LC_FATAL 1u
#define LC_ERROR 2u
#define LC_WARNING 4u
#define LC_CONFIG 8u
#define LC_INFO 16u
#define LC_DISCOVERY 32u
#define LC_DATA 64u
#define LC_TRACE 128u
#define LC_RADMIN 256u
#define LC_ALLCATS (LC_FATAL | LC_ERROR | LC_WARNING | LC_CONFIG | LC_INFO | LC_DISCOVERY | LC_DATA | LC_TRACE)

typedef unsigned logcat_t;

typedef struct logbuf {
  char buf[2048];
  unsigned included_logcats;
  int bufsz;
  int pos;
} *logbuf_t;

#define LOGBUF_DECLNEW(name)                                            \
  {                                                                     \
    struct logbuf name##_ST;                                            \
    logbuf_t name = &name##_ST;                                         \
    name##_ST.pos = 0;                                                  \
    name##_ST.bufsz = sizeof (name##_ST.buf);                           \
    name##_ST.included_logcats = 0;                                     \
    name##_ST.buf[0] = 0;                                               \
    {
#define LOGBUF_FREE(name)                       \
    }                                           \
  }

int nn_vlogb (logbuf_t lb, logcat_t cat, const char *fmt, va_list ap);
int nn_logb (logbuf_t lb, logcat_t cat, const char *fmt, ...);
int nn_logb_flush (logbuf_t lb);

int nn_vlog (logcat_t cat, const char *fmt, va_list ap);
int nn_log (logcat_t cat, const char *fmt, ...);

#endif /* NN_LOG_H */

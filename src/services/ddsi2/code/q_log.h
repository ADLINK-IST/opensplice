/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef NN_LOG_H
#define NN_LOG_H

#include <stdarg.h>

#include "os_report.h"
#include "os_defs.h"
#include "q_time.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_DDSI2
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

#define LC_FATAL 1u
#define LC_ERROR 2u
#define LC_WARNING 4u
#define LC_CONFIG 8u
#define LC_INFO 16u
#define LC_DISCOVERY 32u
#define LC_DATA 64u
#define LC_TRACE 128u
#define LC_RADMIN 256u
#define LC_TIMING 512u
#define LC_TRAFFIC 1024u
#define LC_TOPIC 2048u
#define LC_TCP 4096u
#define LC_PLIST 8192u
#define LC_WHC 16384u
#define LC_ALLCATS (LC_FATAL | LC_ERROR | LC_WARNING | LC_CONFIG | LC_INFO | LC_DISCOVERY | LC_DATA | LC_TRACE | LC_TIMING | LC_TRAFFIC | LC_TCP)

typedef unsigned logcat_t;

typedef struct logbuf {
  char buf[2048];
  size_t bufsz;
  size_t pos;
  nn_wctime_t tstamp;
} *logbuf_t;

logbuf_t logbuf_new (void);
void logbuf_init (logbuf_t lb);
void logbuf_free (logbuf_t lb);

int nn_vlog (logcat_t cat, const char *fmt, va_list ap);
OS_API int nn_log (logcat_t cat, const char *fmt, ...) __attribute_format__((printf,2,3));
OS_API int nn_trace (const char *fmt, ...) __attribute_format__((printf,1,2));
void nn_log_set_tstamp (nn_wctime_t tnow);

#define TRACE(args) ((config.enabled_logcats & LC_TRACE) ? (nn_trace args) : 0)

#define LOG_THREAD_CPUTIME(guard) do {                                  \
    if (config.enabled_logcats & LC_TIMING)                             \
    {                                                                   \
      nn_mtime_t tnow = now_mt ();                                      \
      if (tnow.v >= (guard).v)                                          \
      {                                                                 \
        os_int64 ts = get_thread_cputime ();                            \
        nn_log (LC_TIMING, "thread_cputime %d.%09d\n",                  \
                (int) (ts / T_SECOND), (int) (ts % T_SECOND));          \
        (guard).v = tnow.v + T_SECOND;                                  \
      }                                                                 \
    }                                                                   \
  } while (0)


#define NN_WARNING0(fmt) do {   \
    nn_log (LC_WARNING, (fmt)); \
    os_report (OS_WARNING, config.servicename, __FILE__, __LINE__, 0, (fmt)); \
  } while (0)
#define NN_WARNING1(fmt,a) do { \
    nn_log (LC_WARNING, (fmt), a); \
    os_report (OS_WARNING, config.servicename, __FILE__, __LINE__,  0, (fmt), a); \
  } while (0)
#define NN_WARNING2(fmt,a,b) do { \
    nn_log (LC_WARNING, (fmt), a, b); \
    os_report (OS_WARNING, config.servicename, __FILE__, __LINE__,  0, (fmt), a, b); \
  } while (0)
#define NN_WARNING3(fmt,a,b,c) do { \
    nn_log (LC_WARNING, (fmt), a, b, c); \
    os_report (OS_WARNING, config.servicename, __FILE__, __LINE__,  0, (fmt), a, b, c); \
  } while (0)
#define NN_WARNING4(fmt,a,b,c,d) do { \
    nn_log (LC_WARNING, (fmt), a, b, c, d); \
    os_report (OS_WARNING, config.servicename, __FILE__, __LINE__,  0, (fmt), a, b, c, d); \
  } while (0)
#define NN_WARNING5(fmt,a,b,c,d,e) do { \
    nn_log (LC_WARNING, (fmt), a, b, c, d, e); \
    os_report (OS_WARNING, config.servicename, __FILE__, __LINE__,  0, (fmt), a, b, c, d, e); \
  } while (0)
#define NN_WARNING6(fmt,a,b,c,d,e,f) do { \
    nn_log (LC_WARNING, (fmt), a, b, c, d, e, f); \
    os_report (OS_WARNING, config.servicename, __FILE__, __LINE__,  0, (fmt), a, b, c, d, e, f); \
  } while (0)
#define NN_WARNING7(fmt,a,b,c,d,e,f,g) do { \
    nn_log (LC_WARNING, (fmt), a, b, c, d, e, f, g); \
    os_report (OS_WARNING, config.servicename, __FILE__, __LINE__,  0, (fmt), a, b, c, d, e, f, g); \
  } while (0)
#define NN_WARNING8(fmt,a,b,c,d,e,f,g,h) do { \
    nn_log (LC_WARNING, (fmt), a, b, c, d, e, f, g, h); \
    os_report (OS_WARNING, config.servicename, __FILE__, __LINE__,  0, (fmt), a, b, c, d, e, f, g, h); \
  } while (0)
#define NN_WARNING9(fmt,a,b,c,d,e,f,g,h,i) do { \
    nn_log (LC_WARNING, (fmt), a, b, c, d, e, f, g, h, i); \
    os_report (OS_WARNING, config.servicename, __FILE__, __LINE__,  0, (fmt), a, b, c, d, e, f, g, h, i); \
  } while (0)
#define NN_WARNING10(fmt,a,b,c,d,e,f,g,h,i,j) do { \
    nn_log (LC_WARNING, (fmt), a, b, c, d, e, f, g, h, i, j); \
    os_report (OS_WARNING, config.servicename, __FILE__, __LINE__,  0, (fmt), a, b, c, d, e, f, g, h, i, j); \
  } while (0)

#define NN_ERROR0(fmt) do { \
    nn_log (LC_ERROR, (fmt)); \
    os_report (OS_ERROR, config.servicename, __FILE__, __LINE__, 0, (fmt)); \
  } while (0)
#define NN_ERROR1(fmt,a) do { \
    nn_log (LC_ERROR, (fmt), a); \
    os_report (OS_ERROR, config.servicename, __FILE__, __LINE__,  0, (fmt), a); \
  } while (0)
#define NN_ERROR2(fmt,a,b) do { \
    nn_log (LC_ERROR, (fmt), a, b); \
    os_report (OS_ERROR, config.servicename, __FILE__, __LINE__,  0, (fmt), a, b); \
  } while (0)
#define NN_ERROR3(fmt,a,b,c) do { \
    nn_log (LC_ERROR, (fmt), a, b, c); \
    os_report (OS_ERROR, config.servicename, __FILE__, __LINE__,  0, (fmt), a, b, c); \
  } while (0)
#define NN_ERROR4(fmt,a,b,c,d) do { \
    nn_log (LC_ERROR, (fmt), a, b, c, d); \
    os_report (OS_ERROR, config.servicename, __FILE__, __LINE__,  0, (fmt), a, b, c, d); \
  } while (0)
#define NN_ERROR5(fmt,a,b,c,d,e) do { \
    nn_log (LC_ERROR, (fmt), a, b, c, d, e); \
    os_report (OS_ERROR, config.servicename, __FILE__, __LINE__,  0, (fmt), a, b, c, d, e); \
  } while (0)
#define NN_ERROR6(fmt,a,b,c,d,e,f) do { \
    nn_log (LC_ERROR, (fmt), a, b, c, d, e, f); \
    os_report (OS_ERROR, config.servicename, __FILE__, __LINE__,  0, (fmt), a, b, c, d, e, f); \
  } while (0)
#define NN_ERROR7(fmt,a,b,c,d,e,f,g) do { \
    nn_log (LC_ERROR, (fmt), a, b, c, d, e, f, g); \
    os_report (OS_ERROR, config.servicename, __FILE__, __LINE__,  0, (fmt), a, b, c, d, e, f, g); \
  } while (0)
#define NN_ERROR8(fmt,a,b,c,d,e,f,g,h) do { \
    nn_log (LC_ERROR, (fmt), a, b, c, d, e, f, g, h); \
    os_report (OS_ERROR, config.servicename, __FILE__, __LINE__,  0, (fmt), a, b, c, d, e, f, g, h); \
  } while (0)
#define NN_ERROR9(fmt,a,b,c,d,e,f,g,h,i) do { \
    nn_log (LC_ERROR, (fmt), a, b, c, d, e, f, g, h, i); \
    os_report (OS_ERROR, config.servicename, __FILE__, __LINE__,  0, (fmt), a, b, c, d, e, f, g, h, i); \
  } while (0)
#define NN_ERROR10(fmt,a,b,c,d,e,f,g,h,i,j) do { \
    nn_log (LC_ERROR, (fmt), a, b, c, d, e, f, g, h, i, j); \
    os_report (OS_ERROR, config.servicename, __FILE__, __LINE__,  0, (fmt), a, b, c, d, e, f, g, h, i, j); \
  } while (0)

#define NN_FATAL0(fmt) do { \
    nn_log (LC_FATAL, (fmt)); \
    os_report (OS_FATAL, config.servicename, __FILE__, __LINE__, 0, (fmt)); \
    abort (); \
  } while (0)
#define NN_FATAL1(fmt,a) do { \
    nn_log (LC_FATAL, (fmt), a); \
    os_report (OS_FATAL, config.servicename, __FILE__, __LINE__,  0, (fmt), a); \
    abort (); \
  } while (0)
#define NN_FATAL2(fmt,a,b) do { \
    nn_log (LC_FATAL, (fmt), a, b); \
    os_report (OS_FATAL, config.servicename, __FILE__, __LINE__,  0, (fmt), a, b); \
    abort (); \
  } while (0)
#define NN_FATAL3(fmt,a,b,c) do { \
    nn_log (LC_FATAL, (fmt), a, b, c); \
    os_report (OS_FATAL, config.servicename, __FILE__, __LINE__,  0, (fmt), a, b, c); \
    abort (); \
  } while (0)
#define NN_FATAL4(fmt,a,b,c,d) do { \
    nn_log (LC_FATAL, (fmt), a, b, c, d); \
    os_report (OS_FATAL, config.servicename, __FILE__, __LINE__,  0, (fmt), a, b, c, d); \
    abort (); \
  } while (0)
#define NN_FATAL5(fmt,a,b,c,d,e) do { \
    nn_log (LC_FATAL, (fmt), a, b, c, d, e); \
    os_report (OS_FATAL, config.servicename, __FILE__, __LINE__,  0, (fmt), a, b, c, d, e); \
    abort (); \
  } while (0)
#define NN_FATAL6(fmt,a,b,c,d,e,f) do { \
    nn_log (LC_FATAL, (fmt), a, b, c, d, e, f); \
    os_report (OS_FATAL, config.servicename, __FILE__, __LINE__,  0, (fmt), a, b, c, d, e, f); \
    abort (); \
  } while (0)
#define NN_FATAL7(fmt,a,b,c,d,e,f,g) do { \
    nn_log (LC_FATAL, (fmt), a, b, c, d, e, f, g); \
    os_report (OS_FATAL, config.servicename, __FILE__, __LINE__,  0, (fmt), a, b, c, d, e, f, g); \
    abort (); \
  } while (0)
#define NN_FATAL8(fmt,a,b,c,d,e,f,g,h) do { \
    nn_log (LC_FATAL, (fmt), a, b, c, d, e, f, g, h); \
    os_report (OS_FATAL, config.servicename, __FILE__, __LINE__,  0, (fmt), a, b, c, d, e, f, g, h); \
    abort (); \
  } while (0)
#define NN_FATAL9(fmt,a,b,c,d,e,f,g,h,i) do { \
    nn_log (LC_FATAL, (fmt), a, b, c, d, e, f, g, h, i); \
    os_report (OS_FATAL, config.servicename, __FILE__, __LINE__,  0, (fmt), a, b, c, d, e, f, g, h, i); \
    abort (); \
  } while (0)
#define NN_FATAL10(fmt,a,b,c,d,e,f,g,h,i,j) do { \
    nn_log (LC_FATAL, (fmt), a, b, c, d, e, f, g, h, i, j); \
    os_report (OS_FATAL, config.servicename, __FILE__, __LINE__,  0, (fmt), a, b, c, d, e, f, g, h, i, j); \
    abort (); \
  } while (0)
#undef OS_API
#if defined (__cplusplus)
}
#endif

#endif /* NN_LOG_H */

/* SHA1 not available (unoffical build.) */

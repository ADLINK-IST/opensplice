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
#ifndef NN_TIME_H
#define NN_TIME_H

#include "os_defs.h"
#include "os_time.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_DDSI2
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

#define T_NEVER 0x7fffffffffffffffll
#define T_MILLISECOND 1000000ll
#define T_SECOND (1000 * T_MILLISECOND)
#define T_MICROSECOND (T_MILLISECOND/1000)

typedef struct {
  int seconds;
  unsigned fraction;
} nn_ddsi_time_t;

#if DDSI_DURATION_ACCORDING_TO_SPEC /* what the spec says */
typedef struct { /* why different from ddsi_time_t? */
  int sec;
  int nanosec;
} nn_duration_t;
#else /* this is what I used to do & what wireshark does - probably right */
typedef nn_ddsi_time_t nn_duration_t;
#endif

typedef struct {
  os_int64 v;
} nn_mtime_t;

typedef struct {
  os_int64 v;
} nn_wctime_t;

typedef struct {
  os_int64 v;
} nn_etime_t;

extern const nn_ddsi_time_t invalid_ddsi_timestamp;
extern const nn_ddsi_time_t ddsi_time_infinite;
extern const nn_duration_t duration_infinite;

int valid_ddsi_timestamp (nn_ddsi_time_t t);

OS_API nn_wctime_t now (void);       /* wall clock time */
nn_mtime_t now_mt (void);     /* monotonic time */
nn_etime_t now_et (void);     /* elapsed time */
void mtime_to_sec_usec (os_int64 *sec, int *usec, nn_mtime_t t);
void wctime_to_sec_usec (os_int64 *sec, int *usec, nn_wctime_t t);
void wctime_to_sec_usec_32 (int *sec, int *usec, nn_wctime_t t);
void etime_to_sec_usec (os_int64 *sec, int *usec, nn_etime_t t);
nn_mtime_t mtime_round_up (nn_mtime_t t, os_int64 round);
nn_mtime_t add_duration_to_mtime (nn_mtime_t t, os_int64 d);
nn_wctime_t add_duration_to_wctime (nn_wctime_t t, os_int64 d);
nn_etime_t add_duration_to_etime (nn_etime_t t, os_int64 d);

nn_ddsi_time_t nn_wctime_to_ddsi_time (nn_wctime_t t);
OS_API nn_wctime_t nn_wctime_from_ddsi_time (nn_ddsi_time_t x);
OS_API nn_duration_t nn_to_ddsi_duration (os_int64 t);
OS_API os_int64 nn_from_ddsi_duration (nn_duration_t x);

os_timeW nn_wctime_to_os_timeW(nn_wctime_t t);
#undef OS_API
#if defined (__cplusplus)
}
#endif

#endif /* NN_TIME_H */

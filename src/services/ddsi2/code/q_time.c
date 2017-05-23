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
#include <assert.h>

#include "os_time.h"

#include "q_time.h"

const nn_ddsi_time_t invalid_ddsi_timestamp = { -1, 0xffffffff };
const nn_ddsi_time_t ddsi_time_infinite = { 0x7fffffff, 0xffffffff };
#if DDSI_DURATION_ACCORDING_TO_SPEC
const nn_duration_t duration_infinite = { 0x7fffffff, 0x7fffffff };
#else
const nn_duration_t duration_infinite = { 0x7fffffff, 0xffffffff };
#endif

nn_wctime_t now (void)
{
  /* This function uses the wall clock.
   * This clock is not affected by time spent in suspend mode.
   * This clock is affected when the real time system clock jumps
   * forwards/backwards */
  os_timeW tv;
  nn_wctime_t t;
  tv = os_timeWGet ();
  t.v = ((os_int64) OS_TIMEW_GET_VALUE(tv) );
  return t;
}

nn_mtime_t now_mt (void)
{
  /* This function uses the monotonic clock.
   * This clock stops while the system is in suspend mode.
   * This clock is not affected by any jumps of the realtime clock. */
  os_timeM tv;
  nn_mtime_t t;
  tv = os_timeMGet();
  t.v = ((os_int64) OS_TIMEM_GET_VALUE(tv) );
  return t;
}

nn_etime_t now_et (void)
{
  /* This function uses the elapsed clock.
   * This clock is not affected by any jumps of the realtime clock.
   * This clock does NOT stop when the system is in suspend mode.
   * This clock stops when the system is shut down, and starts when the system is restarted.
   * When restarted, there are no assumptions about the initial value of clock. */
  os_timeE tv;
  nn_etime_t t;
  tv = os_timeEGet();
  t.v = ((os_int64) OS_TIMEE_GET_VALUE(tv) );
  return t;
}

static void time_to_sec_usec (os_int64 *sec, int *usec, os_int64 t)
{
  *sec = (os_int64) (t / T_SECOND);
  *usec = (int) (t % T_SECOND) / 1000;
}

void mtime_to_sec_usec (os_int64 *sec, int *usec, nn_mtime_t t)
{
  time_to_sec_usec (sec, usec, t.v);
}

void wctime_to_sec_usec (os_int64 *sec, int *usec, nn_wctime_t t)
{
  time_to_sec_usec (sec, usec, t.v);
}

void wctime_to_sec_usec_32 (int *sec, int *usec, nn_wctime_t t)
{
    *sec = (int) (t.v / T_SECOND);
    *usec = (int) (t.v % T_SECOND) / 1000;
}

void etime_to_sec_usec (os_int64 *sec, int *usec, nn_etime_t t)
{
  time_to_sec_usec (sec, usec, t.v);
}


nn_mtime_t mtime_round_up (nn_mtime_t t, os_int64 round)
{
  /* This function rounds up t to the nearest next multiple of round.
     t is nanoseconds, round is milliseconds.  Avoid functions from
     maths libraries to keep code portable */
  assert (t.v >= 0 && round >= 0);
  if (round == 0 || t.v == T_NEVER)
  {
    return t;
  }
  else
  {
    os_int64 remainder = t.v % round;
    if (remainder == 0)
    {
      return t;
    }
    else
    {
      nn_mtime_t u;
      u.v = t.v + round - remainder;
      return u;
    }
  }
}

static os_int64 add_duration_to_time (os_int64 t, os_int64 d)
{
  /* assumed T_NEVER <=> MAX_INT64 */
  os_int64 sum = t + d;
  assert (t >= 0 && d >= 0);
  return sum < t ? T_NEVER : sum;
}

nn_mtime_t add_duration_to_mtime (nn_mtime_t t, os_int64 d)
{
  /* assumed T_NEVER <=> MAX_INT64 */
  nn_mtime_t u;
  u.v = add_duration_to_time (t.v, d);
  return u;
}

nn_wctime_t add_duration_to_wctime (nn_wctime_t t, os_int64 d)
{
  /* assumed T_NEVER <=> MAX_INT64 */
  nn_wctime_t u;
  u.v = add_duration_to_time (t.v, d);
  return u;
}

nn_etime_t add_duration_to_etime (nn_etime_t t, os_int64 d)
{
  /* assumed T_NEVER <=> MAX_INT64 */
  nn_etime_t u;
  u.v = add_duration_to_time (t.v, d);
  return u;
}

int valid_ddsi_timestamp (nn_ddsi_time_t t)
{
  return t.seconds != invalid_ddsi_timestamp.seconds && t.fraction != invalid_ddsi_timestamp.fraction;
}

static nn_ddsi_time_t nn_to_ddsi_time (os_int64 t)
{
  if (t == T_NEVER)
    return ddsi_time_infinite;
  else
  {
    /* ceiling(ns * 2^32/10^9) -- can't change the ceiling to round-to-nearest
       because that would break backwards compatibility, but round-to-nearest
       of the inverse is correctly rounded anyway, so it shouldn't ever matter. */
    nn_ddsi_time_t x;
    int ns = (int) (t % T_SECOND);
    x.seconds = (int) (t / T_SECOND);
    x.fraction = (unsigned) (((T_SECOND-1) + ((os_int64) ns << 32)) / T_SECOND);
    return x;
  }
}

nn_ddsi_time_t nn_wctime_to_ddsi_time (nn_wctime_t t)
{
  return nn_to_ddsi_time (t.v);
}

static os_int64 nn_from_ddsi_time (nn_ddsi_time_t x)
{
  if (x.seconds == ddsi_time_infinite.seconds && x.fraction == ddsi_time_infinite.fraction)
    return T_NEVER;
  else
  {
    /* Round-to-nearest conversion of DDSI time fraction to nanoseconds */
    int ns = (int) (((os_int64) 2147483648u + (os_int64) x.fraction * T_SECOND) >> 32);
    return x.seconds * (os_int64) T_SECOND + ns;
  }
}

nn_wctime_t nn_wctime_from_ddsi_time (nn_ddsi_time_t x)
{
  nn_wctime_t t;
  t.v = nn_from_ddsi_time (x);
  return t;
}

#if DDSI_DURATION_ACCORDING_TO_SPEC
nn_duration_t nn_to_ddsi_duration (os_int64 t)
{
  if (t == T_NEVER)
    return duration_infinite;
  else
  {
    nn_duration_t x;
    x.sec = (int) (t / T_SECOND);
    x.nanosec = (int) (t % T_SECOND);
    return x;
  }
}

os_int64 nn_from_ddsi_duration (nn_duration_t x)
{
  os_int64 t;
  if (x.sec == duration_infinite.sec && x.nanosec == duration_infinite.nanosec)
    t = T_NEVER;
  else
    t = x.sec * T_SECOND + x.nanosec;
  return t;
}
#else
nn_duration_t nn_to_ddsi_duration (os_int64 x)
{
  return nn_to_ddsi_time (x);
}

os_int64 nn_from_ddsi_duration (nn_duration_t x)
{
  return nn_from_ddsi_time (x);
}
#endif

os_timeW
nn_wctime_to_os_timeW(nn_wctime_t t)
{
  os_timeW tw;
  assert (t.v >= 0);
  tw = OS_TIMEW_INIT(0,(os_uint64)t.v);
  return tw;
}

/* SHA1 not available (unoffical build.) */

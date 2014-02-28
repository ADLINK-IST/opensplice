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

os_int64 now (void)
{
  os_time tv;
  tv = os_timeGet ();
  return ((os_int64) tv.tv_sec * T_SECOND + tv.tv_nsec);
}

void time_to_sec_usec (int *sec, int *usec, os_int64 t)
{
  *sec = (int) (t / T_SECOND);
  *usec = (int) (t % T_SECOND) / 1000;
}

os_int64 time_round_up (os_int64 t, os_int64 round)
{
  /* This function rounds up t to the nearest next multiple of round.
     t is nanoseconds, round is milliseconds.  Avoid functions from
     maths libraries to keep code portable */
  assert (t >= 0 && round >= 0);
  if (round == 0 || t == T_NEVER)
  {
    return t;
  }
  else
  {
    os_int64 remainder = t % round;
    if (remainder == 0)
    {
      return t;
    }
    else
    {
      return t + round - remainder;
    }
  }
}

os_int64 add_duration_to_time (os_int64 t, os_int64 d)
{
  /* assumed T_NEVER <=> MAX_INT64 */
  os_int64 sum = t + d;
  assert (t >= 0 && d >= 0);
  return sum < t ? T_NEVER : sum;
}

int valid_ddsi_timestamp (nn_ddsi_time_t t)
{
  return t.seconds != invalid_ddsi_timestamp.seconds && t.fraction != invalid_ddsi_timestamp.fraction;
}

nn_ddsi_time_t nn_to_ddsi_time (os_int64 t)
{
  if (t == T_NEVER)
    return ddsi_time_infinite;
  else
  {
    /* Do a fixed-point conversion to DDSI time; this mapping maps
       nanoseconds = 0 => fraction = 0. */
    nn_ddsi_time_t x;
    int ns = (int) (t % T_SECOND);
    x.seconds = (int) (t / T_SECOND);
    x.fraction = (unsigned) (((T_SECOND-1) + ((os_int64) ns << 32)) / T_SECOND);
    return x;
  }
}

os_int64 nn_from_ddsi_time (nn_ddsi_time_t x)
{
  if (x.seconds == ddsi_time_infinite.seconds && x.fraction == ddsi_time_infinite.fraction)
    return T_NEVER;
  else
  {
    /* Truncating conversion of DDSI time fraction to nanoseconds;
       exhaustively tested for correct conversion of fractions
       computed by nn_to_ddsi_time.  But whether all other
       implementations are truncating, I don't know. */
    int ns = (int) (((os_int64) x.fraction * T_SECOND) >> 32);
    return x.seconds * (os_int64) T_SECOND + ns;
  }
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

/* SHA1 not available (unoffical build.) */

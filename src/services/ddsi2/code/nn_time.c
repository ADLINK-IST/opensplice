#include "os_time.h"

#include "nn_time.h"

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
  return ((os_int64) tv.tv_sec * 1000000000ll + tv.tv_nsec);
}

void time_to_sec_usec (int *sec, int *usec, os_int64 t)
{
  *sec = (int) (t / 1000000000);
  *usec = (int) (t % 1000000000) / 1000;
}

os_int64 add_duration_to_time (os_int64 t, os_int64 d)
{
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
    int ns = (int) (t % 1000000000);
    x.seconds = (int) (t / 1000000000);
    x.fraction = (unsigned) ((999999999 + ((os_int64) ns << 32)) / 1000000000);
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
    int ns = (int) (((os_int64) x.fraction * 1000000000) >> 32);
    return x.seconds * (os_int64) 1000000000 + ns;
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
    x.sec = (int) (t / 1000000000);
    x.nanosec = (int) (t % 1000000000);
    return x;
  }
}

os_int64 nn_from_ddsi_duration (nn_duration_t x)
{
  os_int64 t;
  if (x.sec == duration_infinite.sec && x.nanosec == duration_infinite.nanosec)
    t = T_NEVER;
  else
    t = x.sec * 1000000000ll + x.nanosec;
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

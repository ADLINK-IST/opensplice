#ifndef NN_TIME_H
#define NN_TIME_H

#include "os_defs.h"

#if defined (__cplusplus)
extern "C" {
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

extern const nn_ddsi_time_t invalid_ddsi_timestamp;
extern const nn_ddsi_time_t ddsi_time_infinite;
extern const nn_duration_t duration_infinite;

int valid_ddsi_timestamp (nn_ddsi_time_t t);

os_int64 now (void);
void time_to_sec_usec (int *sec, int *usec, os_int64 t);
os_int64 time_round_up (os_int64 t, os_int64 round);
os_int64 add_duration_to_time (os_int64 t, os_int64 d);

nn_ddsi_time_t nn_to_ddsi_time (os_int64 t);
os_int64 nn_from_ddsi_time (nn_ddsi_time_t x);
nn_duration_t nn_to_ddsi_duration (os_int64 t);
os_int64 nn_from_ddsi_duration (nn_duration_t x);

#if defined (__cplusplus)
}
#endif

#endif /* NN_TIME_H */

/* SHA1 not available (unoffical build.) */

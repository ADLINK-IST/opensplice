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

#include "xbe_time.h"


#ifdef _WIN32

/* Number of 100 nanosecond units from 1/1/1601 to 1/1/1970 */
#define EPOCH_BIAS  (116444736000000000i64)

union FT_t
{
    unsigned __int64 ft_i64;
    FILETIME ft_val;
};

static struct timespec last_time;

int clock_gettime (clockid_t clk_id, struct timespec * tp)
{
   union FT_t ft;
   int ret;

   /* this returns time in 100-nanosecond units  (i.e.tens of usecs) */
   GetSystemTimeAsFileTime (&ft.ft_val);

   switch (clk_id)
   {
      case CLOCK_REALTIME:
      {
         tp->tv_sec = (long)((ft.ft_i64 - EPOCH_BIAS) / 10000000);
         tp->tv_nsec = (long)(ft.ft_i64 % 10000000 * 100 );
         break;
      }
      case CLOCK_MONOTONIC:
      {
         tp->tv_sec = (long)((ft.ft_i64 - EPOCH_BIAS) / 10000000);
         tp->tv_nsec = (long)(ft.ft_i64 % 10000000 * 100 );

         if (tp->tv_sec < last_time.tv_sec
             || ( tp->tv_sec == last_time.tv_sec
                  && tp->tv_nsec <= last_time.tv_nsec ) )
         {
            tp->tv_sec = last_time.tv_sec;
            tp->tv_nsec = last_time.tv_nsec + 1;
            if (tp->tv_nsec == 1000000000)
            {
               tp->tv_sec++;
               tp->tv_nsec = 0;
            }
         }
         
         last_time.tv_sec = tp->tv_sec;
         last_time.tv_nsec = tp->tv_nsec;
         break;
      }
      default:
      {
         tp->tv_sec = 0;
         tp->tv_nsec = 0;
         return -1;
      }
   }
   return 0;
}

#endif

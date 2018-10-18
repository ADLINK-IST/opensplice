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
/* -*- c -*- */

#include "sysdeps.h"
#include "os_atomics.h"
#include "q_static_assert.h"

#if defined SUPPRESS_THREAD_INLINES && defined NN_C99_INLINE
#undef NN_C99_INLINE
#define NN_C99_INLINE
#endif

NN_C99_INLINE struct thread_state1 *lookup_thread_state (void)
{
#if OS_HAS_TSD_USING_THREAD_KEYWORD
  if (tsd_thread_state == NULL)
    tsd_thread_state = lookup_thread_state_real ();
  return tsd_thread_state;
#else
  return lookup_thread_state_real ();
#endif
}

NN_C99_INLINE int vtime_awake_p (vtime_t vtime)
{
  return (vtime % 2) == 0;
}

NN_C99_INLINE int vtime_asleep_p (vtime_t vtime)
{
  return (vtime % 2) == 1;
}

NN_C99_INLINE int vtime_gt (vtime_t vtime1, vtime_t vtime0)
{
  Q_STATIC_ASSERT_CODE (sizeof (vtime_t) == sizeof (svtime_t));
  return (svtime_t) (vtime1 - vtime0) > 0;
}

NN_C99_INLINE void thread_state_asleep (struct thread_state1 *ts1)
{
  vtime_t vt = ts1->vtime;
  vtime_t wd = ts1->watchdog;
  if (vtime_awake_p (vt))
  {
    pa_fence_rel ();
    ts1->vtime = vt + 1;
  }
  else
  {
    pa_fence_rel ();
    ts1->vtime = vt + 2;
    pa_fence_acq ();
  }

  if ( wd % 2 ){
    ts1->watchdog = wd + 2;
  } else {
    ts1->watchdog = wd + 1;
  }
 }

NN_C99_INLINE void thread_state_awake (struct thread_state1 *ts1)
{
  vtime_t vt = ts1->vtime;
  vtime_t wd = ts1->watchdog;
  if (vtime_asleep_p (vt))
    ts1->vtime = vt + 1;
  else
  {
    pa_fence_rel ();
    ts1->vtime = vt + 2;
  }
  pa_fence_acq ();

  if ( wd % 2 ){
    ts1->watchdog = wd + 1;
  } else {
    ts1->watchdog = wd + 2;
  }

}

NN_C99_INLINE void thread_state_blocked (struct thread_state1 *ts1)
{
  vtime_t wd = ts1->watchdog;
  if ( wd % 2 ){
    ts1->watchdog = wd + 2;
  } else {
    ts1->watchdog = wd + 1;
  }
}

NN_C99_INLINE void thread_state_unblocked (struct thread_state1 *ts1)
{
  vtime_t wd = ts1->watchdog;
  if ( wd % 2 ){
    ts1->watchdog = wd + 1;
  } else {
    ts1->watchdog = wd + 2;
  }
}



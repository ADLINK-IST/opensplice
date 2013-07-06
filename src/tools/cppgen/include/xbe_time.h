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

#ifndef _C_XBETIME_H_
#define _C_XBETIME_H_

#include <time.h>
#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32

#ifndef HAVE_STRUCT_TIMESPEC
#define HAVE_STRUCT_TIMESPEC 1
struct timespec {
        long tv_sec;
        long tv_nsec;
};
#endif /* HAVE_STRUCT_TIMESPEC */

#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 1

typedef int clockid_t;

int clock_gettime(clockid_t clk_id, struct timespec * tp);

#endif

#ifdef __cplusplus
}
#endif

#endif

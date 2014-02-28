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
#ifndef OS_LINUX_STDLIB_H
#define OS_LINUX_STDLIB_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "../posix/include/os_stdlib.h"

/* Some (older) gcc versions don't supply these unless you
specify C99, and we don't do that yet */

#ifndef LLONG_MAX
#define LLONG_MAX 9223372036854775807LL
#endif
#ifndef LLONG_MIN
#define LLONG_MIN (-LLONG_MAX - 1LL)
#endif

#if defined (__cplusplus)
}
#endif

#endif /* OS_LINUX_STDLIB_H */

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
#ifndef OS_SOLARIS_STDLIB_H
#define OS_SOLARIS_STDLIB_H

#if defined (__cplusplus)
extern "C" {
#endif

/* local function of the required posix function, Solaris 9 does not contain strerror_r */
char * strerror_r(int errnum, char *buf, size_t n);

#include "../posix/include/os_stdlib.h"

#if defined (__cplusplus)
}
#endif

#endif /* OS_SOLARIS_STDLIB_H */

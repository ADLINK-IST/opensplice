/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

#ifndef PA_LINUX_ABSTRACT_H
#define PA_LINUX_ABSTRACT_H

#if defined (__cplusplus)
extern "C" {
#endif

/* include OS specific PLATFORM definition file */
#include <endian.h>

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define PA__LITTLE_ENDIAN
#else
#define PA__BIG_ENDIAN
#endif

#ifdef __x86_64__
#define PA__64BIT
#endif

#if defined (__cplusplus)
}
#endif

#endif /* PA_LINUX_ABSTRACT_H */

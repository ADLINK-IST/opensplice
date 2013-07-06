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

#ifndef PA_DARWIN_ABSTRACT_H
#define PA_DARWIN_ABSTRACT_H

#if defined (__cplusplus)
extern "C" {
#endif

/* include OS specific PLATFORM definition file */
#include <machine/endian.h>

#if __DARWIN_BYTE_ORDER == __DARWIN_LITTLE_ENDIAN
#define PA__LITTLE_ENDIAN
#else
#define PA__BIG_ENDIAN
#endif

#ifdef _LP64
#define PA__64BIT
#endif

#if defined (__cplusplus)
}
#endif

#endif /* PA_DARWIN_ABSTRACT_H */

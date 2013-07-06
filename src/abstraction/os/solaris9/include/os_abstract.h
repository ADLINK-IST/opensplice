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

#ifndef PA_SOLARIS_ABSTRACT_H
#define PA_SOLARIS_ABSTRACT_H

#if defined (__cplusplus)
extern "C" {
#endif

/* include OS specific PLATFORM definition file */
#include <sys/isa_defs.h>

#ifdef _LITTLE_ENDIAN
#define PA__LITTLE_ENDIAN
#else
#define PA__BIG_ENDIAN
#endif

#if defined (__cplusplus)
}
#endif

#endif /* PA_SOLARIS_ABSTRACT_H */

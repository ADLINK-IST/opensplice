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
#ifndef _INCLUDE_H_
#define _INCLUDE_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_CPP
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

extern void init_include (void);
OS_API extern void Ifile (char *);

#undef OS_API

#ifdef __cplusplus
}
#endif

#endif

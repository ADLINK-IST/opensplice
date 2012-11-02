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
#ifndef C_LAPTIME_H
#define C_LAPTIME_H

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_DB
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

typedef struct c_laptime_s *c_laptime;

OS_API c_laptime c_laptimeCreate (const char *id);
OS_API void      c_laptimeDelete (c_laptime laptime);
OS_API void      c_laptimeReset  (c_laptime laptime);
OS_API void      c_laptimeStart  (c_laptime laptime);
OS_API void      c_laptimeStop   (c_laptime laptime);
OS_API void      c_laptimeReport (c_laptime laptime, const char *info);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* C_LAPTIME_H */

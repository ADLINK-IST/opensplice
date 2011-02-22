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
/**@file api/cm/xml/include/cmx_waitset.h
 * Represents a waitset in Splice in XML format.
 */
#ifndef CMX_WAITSET_H
#define CMX_WAITSET_H

#include "c_typebase.h"
#include "v_time.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_CMXML
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

OS_API c_char*         cmx_waitsetNew          (const c_char* participant);

OS_API const c_char*   cmx_waitsetAttach       (const c_char* waitset,
                                                const c_char* entity);
    
OS_API const c_char*   cmx_waitsetDetach       (const c_char* waitset, 
                                                const c_char* entity);
 
OS_API c_char*         cmx_waitsetWait         (const c_char* waitset);

OS_API c_char*         cmx_waitsetTimedWait    (const c_char* waitset,
                                                const c_time t);

OS_API c_ulong         cmx_waitsetGetEventMask (const c_char* waitset);

OS_API const c_char*   cmx_waitsetSetEventMask (const c_char* waitset,
                                                c_ulong mask);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* CMX_WAITSET_H*/

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
#ifndef DDS_DCPSSPLTYPES_H
#define DDS_DCPSSPLTYPES_H

#include "c_base.h"
#include "c_misc.h"
#include "c_sync.h"
#include "c_collection.h"
#include "c_field.h"
#include "os_if.h"

#ifdef OSPL_BUILD_DCPSSAC
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

struct _DDS_Duration_t {
    c_long sec;
    c_ulong nanosec;
};

struct _DDS_Time_t {
    c_long sec;
    c_ulong nanosec;
};

OS_API c_bool
__DDS_Duration_t__copyIn(
    c_base base,
    void *_from,
    void *_to);

OS_API c_bool
__DDS_Time_t__copyIn(
    c_base base,
    void *_from,
    void *_to);

OS_API void
__DDS_Duration_t__copyOut(
    void *_from,
    void *_to);

OS_API void
__DDS_Time_t__copyOut(
    void *_from,
    void *_to);

#undef OS_API

#endif /* DDS_DCPSSPLTYPES_H */

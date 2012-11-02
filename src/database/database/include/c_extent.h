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
#ifndef C_EXTENT_H
#define C_EXTENT_H

#include "c_metabase.h"
#include "c_sync.h"

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

#define c_extent(o)  ((c_extent)(o))

OS_API c_extent
c_extentNew(
    c_type type,
    const c_long blockSize);
    
OS_API c_extent
c_extentSyncNew(
    c_type type,
    const c_long blockSize,
    c_bool sync);
    
OS_API c_object
c_extentCreate(
    c_extent _this);
    
OS_API void
c_extentDelete(
    c_extent _this,
    c_object object);

OS_API c_type
c_extentType(
    c_extent _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* C_EXTENT_H */

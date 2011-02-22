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
#ifndef U_DATAVIEW_H
#define U_DATAVIEW_H

#include "u_types.h"
#include "u_reader.h"
#include "u_dataReader.h"
#include "os_if.h"

#ifdef OSPL_BUILD_USER
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#if defined (__cplusplus)
extern "C" {
#endif

#define u_dataView(o) \
        ((u_dataView)u_entityCheckType(u_entity(o),U_DATAVIEW))

/* DataReaderView creation/deletion */

OS_API u_dataView 
u_dataViewNew(
    u_dataReader r,
    const c_char *name,
    v_dataViewQos qos);

OS_API u_result    
u_dataViewFree(
    u_dataView _this);

OS_API u_result    
u_dataViewRead(
    u_dataView _this,
    u_readerAction action,
    c_voidp actionArg);
                
OS_API u_result    
u_dataViewTake(
    u_dataView _this,
    u_readerAction action,
    c_voidp actionArg);

OS_API u_result    
u_dataViewReadInstance(
    u_dataView _this,
    u_instanceHandle h,
    u_readerAction action,
    c_voidp actionArg);
                
OS_API u_result    
u_dataViewTakeInstance(
    u_dataView _this,
    u_instanceHandle h,
    u_readerAction action,
    c_voidp actionArg);
                
OS_API u_result    
u_dataViewReadNextInstance(
    u_dataView _this,
    u_instanceHandle h,
    u_readerAction action,
    c_voidp actionArg);
                
OS_API u_result    
u_dataViewTakeNextInstance(
    u_dataView _this,
    u_instanceHandle h,
    u_readerAction action,
    c_voidp actionArg);

OS_API u_dataReader    
u_dataViewSource(
    u_dataView _this);
    
OS_API u_result
u_dataViewLookupInstance(
    u_dataView _this,
    c_voidp keyTemplate,
    u_copyIn copyIn,
    u_instanceHandle *handle);
    
#undef OS_API 

#if defined (__cplusplus)
}
#endif

#endif

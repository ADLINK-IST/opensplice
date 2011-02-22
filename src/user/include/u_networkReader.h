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
#ifndef U_NETWORKREADER_H
#define U_NETWORKREADER_H

#include "u_types.h"
#include "u_reader.h"
#include "v_readerQos.h"
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

#define u_networkReader(o) \
        ((u_networkReader)u_entityCheckType(u_entity(o), U_NETWORKREADER))

OS_API u_networkReader
u_networkReaderNew(
    u_subscriber s,
    const c_char *name,
    v_readerQos qos,
    c_bool ignoreReliabilityQoS);
    
OS_API u_result        
u_networkReaderInit(
    u_networkReader _this,
    u_subscriber s);
    
OS_API u_result        
u_networkReaderCreateQueue(
    u_networkReader _this,
    c_ulong queueSize,
    c_ulong priority,
    c_bool reliable,
    c_bool P2P,
    c_time resolution,
    c_bool useAsDefault,
    c_ulong *queueId, /* out */
    const c_char *name);
    
OS_API u_result        
u_networkReaderTrigger(
    u_networkReader _this,
    c_ulong queueId);
    
OS_API u_result       
u_networkReaderFree(
    u_networkReader _this);
    
OS_API u_result        
u_networkReaderDeinit(
    u_networkReader _this);

OS_API u_result        
u_networkReaderRemoteActivityDetected(
    u_networkReader _this);
    
OS_API u_result        
u_networkReaderRemoteActivityLost(
    u_networkReader _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif

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
#ifndef V_NETWORKREADERENTRY_H
#define V_NETWORKREADERENTRY_H

#define RELIABILITY_IN_KERNEL 0

#include "v_kernel.h"
#include "v_entry.h"
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/* -------------------------- v_networkReaderEntry --------------------- */

/**
 * \brief The <code>v_networkReaderEntry</code> cast method.
 *
 * This method casts an object to a <code>v_networkReaderEntry</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_networkReaderEntry</code> or
 * one of its subclasses.
 */
#define v_networkReaderEntry(o) (C_CAST(o, v_networkReaderEntry))

OS_API v_networkReaderEntry
v_networkReaderEntryNew(
    v_networkReader reader,
    v_group group,
    v_networkId networkId,
    c_ulong channelsToConnect,
    v_networkPartitionId networkPartitionId);
                         
OS_API void
v_networkReaderEntryFree(
    v_networkReaderEntry entry);

OS_API v_writeResult
v_networkReaderEntryWrite(
    v_networkReaderEntry entry,
    v_message message,
    v_networkId writingNetworkId);

OS_API v_writeResult
v_networkReaderEntryReceive(
    v_networkReaderEntry entry,
    v_message message,
    c_ulong sequenceNumber,
    v_gid sender,
    c_bool sendTo, /* for p2p writing */
    v_gid receiver);
                    
OS_API void
v_networkReaderEntryNotifyConnected(
    v_networkReaderEntry entry,
    const c_char* serviceName);

#undef OS_API

#endif /* V_NETWORKREADERENTRY_H */

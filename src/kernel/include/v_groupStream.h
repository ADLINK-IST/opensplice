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
#ifndef V_GROUPSTREAM_H
#define V_GROUPSTREAM_H

/** \file kernel/include/v_groupStream.h
 *  \brief This file defines the interface
 *
 */

#include "v_kernel.h"
#include "kernelModule.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#if defined (__cplusplus)
}
#endif

/**
 * \brief The <code>v_groupStream</code> cast method.
 *
 * This method casts an object to a <code>v_groupStream</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_groupStream</code> or
 * one of its subclasses.
 */
#define v_groupStream(s) (C_CAST(s,v_groupStream))

OS_API void            
v_groupStreamInit(
    v_groupStream stream,
    const c_char *name,
    v_subscriber subscriber,
    v_readerQos qos);
                                         
OS_API void            
v_groupStreamDeinit(
    v_groupStream stream);

OS_API void            
v_groupStreamFree(
    v_groupStream stream);

#undef OS_API

#endif /* V_GROUPSTREAM_H */

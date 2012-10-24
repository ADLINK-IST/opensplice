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
#ifndef V_ENTRY_H
#define V_ENTRY_H

/** \file kernel/include/v_entry.h
 *  \brief This file defines the interface
 *
 */

#include "v_kernel.h"
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * \brief The <code>v_entry</code> cast method.
 *
 * This method casts an object to a <code>v_entry</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_entry</code> or
 * one of its subclasses.
 */
#define v_entry(_this) (C_CAST(_this,v_entry))

OS_API v_writeResult
v_entryWrite (
    v_entry _this,
    v_message msg,
    v_networkId id,
    v_instance *inst);

OS_API v_writeResult
v_entryResend (
    v_entry _this,
    v_message msg);

OS_API c_bool
v_entrySubscribe (
    v_entry _this,
    v_partition d);

OS_API c_bool
v_entryUnSubscribe (
    v_entry _this,
    v_partition d);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif

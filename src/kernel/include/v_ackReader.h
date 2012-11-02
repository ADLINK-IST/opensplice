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
#ifndef V_ACKREADER_H
#define V_ACKREADER_H

/** \file kernel/include/v_ackReader.h
 *  \brief This file defines the interface of AckReader objects.
 *
 * Objects implement a data storage of instances and data samples.
 * This interface provides read access to instance and samples
 * and provides acces to meta data on status and data.
 * The data is inserted into the storage by the kernel.
 *
 */

#include "v_kernel.h"

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

/**
 * \brief The <code>v_ackReader</code> cast method.
 *
 * This method casts an object to a <code>v_ackReader</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_ackReader</code> or
 * one of its subclasses.
 */
#define v_ackReader(o) (C_CAST(o,v_ackReader))

OS_API v_ackReader
v_ackReaderNew(
    v_subscriber subscriber,
    const c_char *name);

OS_API void
v_ackReaderFree(
    v_ackReader _this);

OS_API v_result
v_ackReaderEnable(
    v_ackReader _this);

#if defined (__cplusplus)
}
#endif

#endif

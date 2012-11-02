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
#ifndef V_LIFESPANSAMPLE_H
#define V_LIFESPANSAMPLE_H

/** \file kernel/include/v_lifespanSample.h
 *  \brief This file defines the interface
 *
 */

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
 * \brief The <code>v_lifespanSample</code> cast method.
 *
 * This method casts an object to a <code>v_lifespanSample</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_lifespanSample</code> or
 * one of its subclasses.
 */
#define v_lifespanSample(o) (C_CAST((o),v_lifespanSample))

typedef c_bool
(*v_lifespanSampleAction)(
    v_lifespanSample sample,
    c_voidp arg);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* V_LIFESPANSAMPLE_H */

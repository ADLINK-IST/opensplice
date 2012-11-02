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
#ifndef V_AVGVALUE_H
#define V_AVGVALUE_H

/** \file kernel/include/v_avgValue.h
 *  \brief This file defines the interface of the avgValue type.
 *
 * The v_avgValue type defines average value objects.
 * Objects are set to zero by construction and values can
 * be added to the average by v_avgValueSetValue().
 * The v_avgValueGetValue() method will return the actual average value.
 * Objects can be reused by resetting the value using the v_avgValueReset
 * method.
 */


#include "kernelModule.h"
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

OS_API void
v_avgValueInit (
    v_avgValue *_this);

OS_API void
v_avgValueReset (
    v_avgValue *_this);

OS_API c_float
v_avgValueGetValue (
    v_avgValue *_this);

OS_API void
v_avgValueSetValue (
    v_avgValue *_this,
    c_ulong value);

/* Option: 
 * OS_API c_time  v_avgValueGetTime    (v_avgValue *_this);
 */

#undef OS_API

#endif

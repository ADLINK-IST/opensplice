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
#ifndef V_STATISTICS_H
#define V_STATISTICS_H

#include "v_kernel.h"
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_statistics</code> cast method.
 *
 * This method casts an object to a <code>v_statistics</code> object.
 * Before the cast is performed, if the NDEBUG flag is not set,
 * the type of the object is checked to be <code>v_statistics</code> or
 * one of its subclasses.
 */
#define v_statistics(s) (C_CAST(s,v_statistics))

OS_API c_bool
v_statisticsReset(
    v_statistics _this,
    const c_char *fieldName);

#undef OS_API

#endif

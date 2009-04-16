/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef V_DOMAIN_H
#define V_DOMAIN_H

/** \file kernel/include/v_domain.h
 *  \brief This file defines the interface
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
 * \brief The <code>v_domain</code> cast method.
 *
 * This method casts an object to a <code>v_domain</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_domain</code> or
 * one of its subclasses.
 */
#define v_domain(_this) (C_CAST(_this,v_domain))

#define v_partitionName(_this) (v_entityName(v_domain(_this)))

OS_API v_domain
v_domainNew (
    v_kernel kernel,
    const c_char *name,
    v_domainQos qos);

OS_API void
v_domainFree (
    v_domain _this);

OS_API void
v_domainDeinit (
    v_domain _this);

OS_API c_iter
v_domainLookupPublishers (
    v_domain _this);

OS_API c_iter
v_domainLookupSubscribers (
    v_domain _this);


#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif

/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef V_RNR_H
#define V_RNR_H

/** \file kernel/include/v_rnr.h
 *  \brief This file defines the interface
 *
 */

#include "kernelModule.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_rnr</code> cast method.
 *
 * This method casts an object to a <code>v_rnr</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_rnr</code> or
 * one of its subclasses.
 */
#define v_rnr(o) (C_CAST(o,v_rnr))

/**
 * \brief The <code>v_rnr</code> constructor.
 *
 * This method creates a record and replay service participant.
 *
 * Currently this class is part of the kernel but as being a pluggable service
 * This definition should be part of the service itself.
 *
 * \param manager        the kernel service manager
 * \param name           the participant name of the service
 * \param extStateName   the name by which the service state is identified.
 *                       the state will be made accessible via this name.
 * \param qos            the service participant QoS policy values.
 *
 * \return <code>NULL</code> if construction fails, otherwise
 *         a reference to a newly instantiated service object.
 */
OS_API v_rnr
v_rnrNew(
    v_serviceManager manager,
    const c_char *name,
    const c_char *extStateName,
    v_participantQos qos);

/**
 * \brief The <code>v_rnr</code> destructor.
 *
 * This method destroys the record and replay service participant.
 *
 * \param service  the service to be destroyed.
 *
 */
OS_API void
v_rnrFree(
    v_rnr service);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* V_RNR_H */

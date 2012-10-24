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
#ifndef V_BUILTIN_H
#define V_BUILTIN_H

/** \file kernel/include/v_builtin.h
 *  \brief This file defines the interface
 *
 */

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"
#include "v_kernel.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/* built-in topic definitions */
#define V_BUILTIN_PARTITION       "__BUILT-IN PARTITION__"

#define V_PARTICIPANTINFO_NAME   "DCPSParticipant"
#define V_TOPICINFO_NAME         "DCPSTopic"
#define V_PUBLICATIONINFO_NAME   "DCPSPublication"
#define V_SUBSCRIPTIONINFO_NAME  "DCPSSubscription"
#define V_DELIVERYINFO_NAME      "DCPSDelivery"
#define V_HEARTBEATINFO_NAME     "DCPSHeartbeat"
#define V_C_AND_M_COMMAND_NAME   "DCPSCandMCommand"

/**
 * \brief The <code>v_builtin</code> cast method.
 *
 * This method casts an object to a <code>v_builtin</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_builtin</code> or
 * one of its subclasses.
 */
#define v_builtin(_this) (C_CAST(_this,v_builtin))

#define v_builtinTopicLookup(_this, _id) \
        (_this == NULL ? NULL : \
        v_topic(v_builtin(_this)->topics[_id]))

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif

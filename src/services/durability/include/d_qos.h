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

#ifndef D_QOS_H
#define D_QOS_H

#include "d__types.h"
#include "v_qos.h"
#include "u_user.h"

#if defined (__cplusplus)
extern "C" {
#endif

v_publisherQos
d_publisherQosNew (
    const c_char* partition);

void
d_publisherQosFree (
    v_publisherQos qos);

v_subscriberQos
d_subscriberQosNew (
    const c_char* partition);

void
d_subscriberQosFree (
    v_subscriberQos qos);

v_readerQos
d_readerQosNew (
    v_durabilityKind durability,
    v_reliabilityKind reliability);

void
d_readerQosFree (
    v_readerQos qos);

v_writerQos
d_writerQosNew (
    v_durabilityKind durability,
    v_reliabilityKind reliability,
    v_duration latencyBudget,
    c_long transportPriority);

void
d_writerQosFree (
    v_writerQos qos);

v_topicQos
d_topicQosNew (
    v_durabilityKind durability,
    v_reliabilityKind reliability);

void
d_topicQosFree (
    v_topicQos qos);

#if defined (__cplusplus)
}
#endif

#endif /* D_QOS_H */

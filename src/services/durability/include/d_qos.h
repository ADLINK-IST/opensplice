/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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
    v_orderbyKind orderKind,
    os_duration latencyBudget,
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

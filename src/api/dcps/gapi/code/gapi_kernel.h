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
#ifndef GAPI_KERNEL_H
#define GAPI_KERNEL_H

#include "u_entity.h"
#include "u_domain.h"
#include "u_writer.h"
#include "u_participant.h"

#include "gapi.h"

/* Indicates the maximum number policies, copy this value from 
   v_kernel.odl::Vgapi_qosPolicyCount
*/
#define MAX_POLICY_COUNT_ID     (32) 

typedef struct kernelKeyValueList_s {
    c_ulong size;
    c_value list[1];
} *kernelKeyValueList;

c_base
kernelGetBase(
    u_entity e);

c_long
kernelStatusGet (
    u_entity e);

v_kernel
kernelGetKernelId (
    u_entity e);

u_instanceHandle
kernelGetInstanceHandle (
    u_entity e);

u_instanceHandle
kernelLookupInstanceHandle (
    u_entity      entity,
    const c_char *builtinTopicName);

gapi_returnCode_t
kernelResultToApiResult(
    u_result r);

void
kernelCopyInDuration (
    const gapi_duration_t *from,
    v_duration      *to);

void
kernelCopyOutDuration (
    const v_duration *from,
    gapi_duration_t  *to);

gapi_returnCode_t
kernelCopyInTime (
    const gapi_time_t *from,
    c_time            *to);

gapi_returnCode_t
kernelCopyOutTime (
    const c_time *from,
    gapi_time_t  *to);

gapi_boolean
kernelKeyValueListEqual (
    kernelKeyValueList l1,
    kernelKeyValueList l2);

c_ulong
kernelGetHashKeyFromKeyValueList (
    kernelKeyValueList list);

void 
kernelKeyValueListFree (
    kernelKeyValueList list);

gapi_boolean
kernelCheckTopicKeyList (
    u_topic topic,
    const gapi_char *keyList);

gapi_char *
kernelTopicGetKeys (
    u_topic topic);

gapi_boolean
gapi_kernelReaderQosCopyIn (
    const gapi_dataReaderQos *srcQos,
    v_readerQos               dstQos);

gapi_boolean
gapi_kernelReaderQosCopyOut (
    const v_readerQos   srcQos,
    gapi_dataReaderQos *dstQos);

#endif

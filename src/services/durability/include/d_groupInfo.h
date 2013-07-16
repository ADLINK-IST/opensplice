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
#ifndef D_GROUPINFO_H
#define D_GROUPINFO_H

#include "durabilityModule2.h"
#include "u_participant.h"
#include "d_storeMMF.h"
#include "d_topicInfo.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * \brief The <code>d_groupInfo</code> cast method.
 *
 * This method casts an object to a <code>d_groupInfo</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>d_groupInfo</code> or
 * one of its subclasses.
 */
#define d_groupInfo(o)       (C_CAST(o,d_groupInfo))

#define d_groupInfoTopicInfo(_this) \
        (d_groupInfo(_this)->topic)

#define d_groupInfoTopicName(_this) \
        (d_topicInfoName(d_groupInfoTopicInfo(_this)))

#define d_groupInfoTopicTypeName(_this) \
        (d_topicInfoTypeName(d_groupInfoTopicInfo(_this)))

#define d_groupInfoTopicKeyExpr(_this) \
        (d_topicInfoKeyExpr(d_groupInfoTopicInfo(_this)))

#define d_groupInfoTopicQos(_this) \
        (d_topicInfoQos(d_groupInfoTopicInfo(_this)))

#define d_groupInfoPartition(_this) \
        (d_groupInfo(_this)->partition)

#define d_groupInfoCompleteness(_this) \
        (d_groupInfo(_this)->completeness)

#define d_groupInfoQuality(_this) \
        (d_groupInfo(_this)->quality)

#define d_groupInfoSetQuality(_this, quality) \
        (d_groupInfo(_this)->quality = quality)


d_groupInfo
d_groupInfoNew (
    const d_storeMMFKernel kernel,
    const d_topicInfo topicInfo,
    const d_group group);

d_storeResult
d_groupInfoWrite(
    d_groupInfo _this,
    const d_store store,
    const v_groupAction action,
    d_sample sample);

d_storeResult
d_groupInfoDispose(
    d_groupInfo _this,
    const d_store store,
    const v_groupAction action,
    d_sample sample);

d_storeResult
d_groupInfoExpungeSample(
    d_groupInfo _this,
    const d_store store,
    const v_groupAction action);

d_storeResult
d_groupInfoExpungeInstance(
    d_groupInfo _this,
    const d_store store,
    const v_groupAction action);

d_storeResult
d_groupInfoDeleteHistoricalData(
    d_groupInfo _this,
    const d_store store,
    const v_groupAction action);


d_storeResult
d_groupInfoInject(
    d_groupInfo _this,
    const d_store store,
    u_participant participant,
    d_group* group);

d_storeResult
d_groupInfoDataInject(
    d_groupInfo _this,
    const d_store store,
    d_group group);

d_storeResult
d_groupInfoBackup(
    d_groupInfo _this,
    const d_store store,
    d_groupInfo* backup);


d_sample
d_groupInfoSampleNew (
    d_groupInfo _this,
    d_instance instance,
    v_message msg);

#if defined (__cplusplus)
}
#endif

#endif

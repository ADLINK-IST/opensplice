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
#ifndef D_TOPICINFO_H_
#define D_TOPICINFO_H_

#if defined (__cplusplus)
extern "C" {
#endif

#include "durabilityModule2.h"
#include "d_storeMMF.h"
#include "u_participant.h"

/**
 * \brief The <code>d_topicInfo</code> cast method.
 *
 * This method casts an object to a <code>d_topicInfo</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>d_topicInfo</code> or
 * one of its subclasses.
 */
#define d_topicInfo(o) (C_CAST(o,d_topicInfo))

#define d_topicInfoName(_this) \
        (d_topicInfo(_this)->name)

#define d_topicInfoTypeName(_this) \
        (d_topicInfo(_this)->typeName)

#define d_topicInfoKeyExpr(_this) \
        (d_topicInfo(_this)->keyExpr)

#define d_topicInfoQos(_this) \
        (d_topicInfo(_this)->qos)


d_topicInfo
d_topicInfoNew(
    d_storeMMFKernel kernel,
    const v_topic vtopic);

c_type
d_topicInfoGetMessageType(
    d_topicInfo topicInfo);

c_type
d_topicInfoGetInstanceType(
    d_topicInfo topicInfo);

c_string
d_topicInfoGetInstanceKeyExpr(
    d_topicInfo topicInfo);

c_string
d_topicInfoGetName(
    d_topicInfo topicInfo);

d_storeResult
d_topicInfoInject(
    d_topicInfo _this,
    d_store store,
    u_participant participant);

#if defined (__cplusplus)
}
#endif

#endif /* D_TOPICINFO_H_ */

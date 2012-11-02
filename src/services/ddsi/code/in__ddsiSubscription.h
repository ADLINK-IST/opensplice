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
#ifndef IN__DDSISUBSCRIPTION_H_
#define IN__DDSISUBSCRIPTION_H_

#include "in_ddsiSubscription.h"
#include "kernelModule.h"
#include "in_ddsiElements.h"
#include "in_locatorList.h"
#include "in__object.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**
 */
OS_STRUCT(in_ddsiSubscriptionBuiltinTopicData)
{
	struct v_subscriptionInfo info;
};

/**
 */
OS_STRUCT(in_ddsiReaderProxy)
{
	OS_STRUCT(in_ddsiGuid) remoteReaderGuid;
	os_boolean expectsInlineQos;
	in_locatorList unicastLocatorList;
	in_locatorList multicastLocatorList;
};

/** derives in_object
 */
OS_STRUCT(in_ddsiDiscoveredReaderData)
{
	OS_EXTENDS(in_object);
	OS_STRUCT(in_ddsiSubscriptionBuiltinTopicData) topicData;
	OS_STRUCT(in_ddsiReaderProxy) proxy;
};



#if defined (__cplusplus)
}
#endif


#endif /* IN__DDSISUBSCRIPTION_H_ */

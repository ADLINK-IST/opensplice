/*
 * in__ddsiSubscription.h
 *
 *  Created on: Feb 27, 2009
 *      Author: frehberg
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

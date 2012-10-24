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
#ifndef IN__DDSITOPIC_H_
#define IN__DDSITOPIC_H_

#include "in_ddsiTopic.h"
#include "kernelModule.h"
#include "in_ddsiElements.h"
#include "in__object.h"

#if defined (__cplusplus)
extern "C" {
#endif


/** */
OS_STRUCT(in_ddsiTopicBuiltinTopicData)
{
	struct v_topicInfo info;
};

/** derives in_object */
OS_STRUCT(in_ddsiDiscoveredTopicData)
{
	OS_EXTENDS(in_object);
	OS_STRUCT(in_ddsiTopicBuiltinTopicData) topicData;
};

#if defined (__cplusplus)
}
#endif


#endif /* IN__DDSITOPIC_H_ */

/*
 * in__ddsiTopic.h
 *
 *  Created on: Feb 27, 2009
 *      Author: frehberg
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

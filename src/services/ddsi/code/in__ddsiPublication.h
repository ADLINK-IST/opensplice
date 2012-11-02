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
#ifndef IN__DDSIPUBLICATION_H_
#define IN__DDSIPUBLICATION_H_

#include "in_ddsiPublication.h"
#include "kernelModule.h"
#include "in_ddsiElements.h"
#include "in_locatorList.h"
#include "in__object.h"

#if defined (__cplusplus)
extern "C" {
#endif


/** */
OS_STRUCT(in_ddsiPublicationBuiltinTopicData)
{
	struct v_publicationInfo info;
};

/** */
OS_STRUCT(in_ddsiWriterProxy)
{
	OS_STRUCT(in_ddsiGuid) remoteWriterGuid;
	in_locatorList unicastLocatorList;
	in_locatorList multicastLocatorList;
};

/** derives fromin_object */
OS_STRUCT(in_ddsiDiscoveredWriterData)
{
	OS_EXTENDS(in_object);
	OS_STRUCT(in_ddsiPublicationBuiltinTopicData) topicData;
	OS_STRUCT(in_ddsiWriterProxy) proxy;
};

#if defined (__cplusplus)
}
#endif


#endif /* IN__DDSIPUBLICATION_H_ */

/*
 * in__ddsiParticipantBuiltinTopicData.h
 *
 *  Created on: Feb 26, 2009
 *      Author: frehberg
 */

#ifndef IN__DDSIPARTICIPANT_H_
#define IN__DDSIPARTICIPANT_H_

#include "in__ddsiParticipant.h"

#if defined (__cplusplus)
extern "C" {
#endif

/** */
OS_STRUCT(in_ddsiParticipantBuiltinTopicData)
{
	struct v_participantInfo info;
};


/** */
OS_STRUCT(in_ddsiParticipantProxy)
{
	OS_STRUCT(in_ddsiProtocolVersion) protocolVersion;
	in_ddsiGuidPrefix guidPrefix; /* in_octet[12] */
	in_ddsiVendorId vendorId;
	os_boolean expectsInlineQos;
	OS_STRUCT(in_ddsiBuiltinEndpointSet) availableBuiltinEndpoints;
	Coll_List metatrafficUnicastLocatorList;
	Coll_List metatrafficMulticastLocatorList;
	Coll_List defaultMulticastLocatorList;
	Coll_List defaultUnicastLocatorList;
	OS_STRUCT(in_ddsiCount) manualLivelinessCount;
};

/** derices in_object  */
OS_STRUCT(in_ddsiDiscoveredParticipantData)
{
	OS_EXTENDS(in_object);
	OS_STRUCT(in_ddsiParticipantBuiltinTopicData) builtinTopicData;
	OS_STRUCT(in_ddsiParticipantProxy) proxy;
};

/** */
OS_STRUCT(in_ddsiSPDPdiscoveredParticipantData)
{
	OS_EXTENDS(in_ddsiDiscoveredParticipantData);
	v_duration leaseDuration;
};


#if defined (__cplusplus)
}
#endif


#endif /* IN__DDSIPARTICIPANTBUILTINTOPICDATA_H_ */

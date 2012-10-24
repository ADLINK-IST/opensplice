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
#ifndef IN__DDSIRECEIVER_H_
#define IN__DDSIRECEIVER_H_

#include "in_ddsiReceiver.h"

#include "in_commonTypes.h"
#include "in_ddsiElements.h"
#include "in_locatorList.h"
#include "c_time.h"

#if defined (__cplusplus)
extern "C" {
#endif

OS_STRUCT(in_ddsiReceiver)
{
	OS_STRUCT(in_ddsiProtocolVersion) sourceVersion;
	in_ddsiVendorId sourceVendorId; /* in_octet[2] */
	in_ddsiGuidPrefix sourceGuidPrefix; /* in_octet[14] */
	in_ddsiGuidPrefix destGuidPrefix;   /* in_octet[14] */
	in_locatorList unicastReplyLocatorList;
	in_locatorList multicastReplyLocatorList;
	os_boolean haveTimestamp;
	c_time timestamp;
};

#if defined (__cplusplus)
}
#endif


#endif /* IN__DDSIRECEIVER_H_ */

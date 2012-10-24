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
#ifndef IN__DDSIPARAMETER_H_
#define IN__DDSIPARAMETER_H_

#include "in_ddsiParameter.h"
#include "in_ddsiElements.h"

#if defined (__cplusplus)
extern "C" {
#endif


OS_STRUCT(in_ddsiParameterHeader)
{
	OS_STRUCT(in_ddsiParameterId) id;
	os_ushort octetsToNextParameter;
};

#define in_ddsiParameterHeaderId(_o) \
	((_o)->id.value)

#if defined (__cplusplus)
}
#endif


#endif /* IN__DDSIPARAMETER_H_ */

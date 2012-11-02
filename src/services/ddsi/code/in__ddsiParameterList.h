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
#ifndef IN__DDSIPARAMETERLIST_H_
#define IN__DDSIPARAMETERLIST_H_

#include "in__object.h"

#if defined (__cplusplus)
extern "C" {
#endif

OS_STRUCT(in_ddsiParameterList)
{
	in_ddsiParameterToken firstParameter;
	os_size_t totalOctetLength;
	os_boolean isBigEndian;
};

#if defined (__cplusplus)
}
#endif


#endif /* IN__DDSIPARAMETERLIST_H_ */

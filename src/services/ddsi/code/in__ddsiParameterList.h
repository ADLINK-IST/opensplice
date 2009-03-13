/*
 * in__ddsiParameterList.h
 *
 *  Created on: Feb 24, 2009
 *      Author: frehberg
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

/*
 * in__ddsiParameter.h
 *
 *  Created on: Mar 9, 2009
 *      Author: frehberg
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


#if defined (__cplusplus)
}
#endif


#endif /* IN__DDSIPARAMETER_H_ */

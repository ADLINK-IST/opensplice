/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
/*
 * in_ddsiParameter.h
 *
 *  Created on: Mar 9, 2009
 *      Author: frehberg
 */

#ifndef IN_DDSIPARAMETER_H_
#define IN_DDSIPARAMETER_H_

#include "in_commonTypes.h"
#include "in_ddsiDeserializer.h"

#if defined (__cplusplus)
extern "C" {
#endif

in_long
in_ddsiParameterHeaderInitFromBuffer(in_ddsiParameterHeader _this,
		in_ddsiDeserializer deserializer);

#if defined (__cplusplus)
}
#endif


#endif /* IN_DDSIPARAMETER_H_ */

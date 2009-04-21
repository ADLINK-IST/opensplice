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
 * in__ddsiStreamWriter.h
 *
 *  Created on: Mar 2, 2009
 *      Author: frehberg
 */

#ifndef IN__DDSISTREAMWRITER_H_
#define IN__DDSISTREAMWRITER_H_

#include "in_ddsiStreamWriterImpl.h"
#include "in__streamWriter.h"
#include "os_time.h"
#include "in__ddsiSerializer.h"
#include "in__messageSerializer.h"d

#if defined (__cplusplus)
extern "C" {
#endif

	/** extends from in_objects */
OS_STRUCT(in_ddsiStreamWriterImpl)
{
	OS_EXTENDS(in_streamWriter);

	in_transportSender transportSender;
	in_abstractSendBuffer currentSendBuffer;
	OS_STRUCT(in_ddsiSerializer) serializer;
	os_time timeout;
	in_connectivityAdmin connectivityAdmin;
	in_messageSerializer messageSerializer;
};


#if defined (__cplusplus)
}
#endif


#endif /* IN__DDSISTREAMWRITER_H_ */

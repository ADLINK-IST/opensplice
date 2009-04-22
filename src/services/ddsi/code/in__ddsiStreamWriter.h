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
#include "in__messageSerializer.h"
#include "in__configChannel.h"

#if defined (__cplusplus)
extern "C" {
#endif

	/** extends from in_objects */
OS_STRUCT(in_ddsiStreamWriterImpl)
{
	OS_EXTENDS(in_streamWriter);

	/* un-managed objects */
    os_time timeout;

	/* managed objects */
	in_transportSender transportSender;
	in_abstractSendBuffer currentSendBuffer;
	OS_STRUCT(in_ddsiSerializer) serializer;
	in_messageSerializer messageSerializer;
	in_plugKernel plugKernel;
};


#if defined (__cplusplus)
}
#endif


#endif /* IN__DDSISTREAMWRITER_H_ */

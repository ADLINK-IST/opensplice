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

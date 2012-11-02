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
#ifndef IN_DDSISTREAMREADERIMPL_H_
#define IN_DDSISTREAMREADERIMPL_H_

#include "in_streamReader.h"
#include "in__configChannel.h"
#include "in_connectivityAdmin.h"
#include "in_transportReceiver.h"

#if defined (__cplusplus)
extern "C" {
#endif


/** */
#define in_ddsiStreamReaderImpl(_obj) \
	((in_ddsiStreamReaderImpl)_obj)

in_ddsiStreamReaderImpl
in_ddsiStreamReaderImplNew(
		in_configChannel config,
		in_transportReceiver receiver,
		in_plugKernel plug);

#define in_ddsiStreamReaderImplFree(i) in_objectFree(in_object(i))

#if defined (__cplusplus)
}
#endif


#endif /* IN_DDSISTREAMREADERIMPL_H_ */

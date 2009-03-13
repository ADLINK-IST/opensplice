/*
 * in_ddsiStreamReaderImpl.h
 *
 *  Created on: Mar 5, 2009
 *      Author: frehberg
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
		in_connectivityAdmin connectivityAdmin);

#if defined (__cplusplus)
}
#endif


#endif /* IN_DDSISTREAMREADERIMPL_H_ */

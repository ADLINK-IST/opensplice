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
#ifndef IN__STREAMREADER_H_
#define IN__STREAMREADER_H_

#include "in_streamReader.h"
#include "in_result.h"
#include "in_ddsiElements.h"
#include "in__object.h"

#if defined (__cplusplus)
extern "C" {
#endif

/** */
typedef in_result  (*in_streamReaderScanFunc)(
        in_streamReader _this,
        in_streamReaderCallbackTable callbackTable,
        in_streamReaderCallbackArg   callbackArg,
        os_time *timeout);

/** */
OS_CLASS(in_streamReaderPublicVTable);
/** */
OS_STRUCT(in_streamReaderPublicVTable)
{
    in_streamReaderScanFunc scan;
};


/** */
OS_STRUCT(in_streamReader)
{
	OS_EXTENDS(in_object);
	/* operations that are access by external objects */
	in_streamReaderPublicVTable    publicVTable;
};


/** */
os_boolean
in_streamReaderInit(
        in_streamReader _this,
        in_objectKind kind,
        in_objectDeinitFunc deinit,
        in_streamReaderPublicVTable staticPublicVTable);

/** */
void
in_streamReaderDeinit(in_streamReader _this);

#if defined (__cplusplus)
}
#endif


#endif /* IN__STREAMREADER_H_ */

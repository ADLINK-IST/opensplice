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
 /* interface */
#include "in__streamPair.h"

/* implementation */
#include "in_commonTypes.h"
#include "in__object.h"

/** \brief init */
os_boolean
in_streamPairInit(
    in_streamPair _this,
    in_objectKind kind,
    in_objectDeinitFunc deinit,
    in_streamPairGetReaderFunc getReader,
    in_streamPairGetWriterFunc getWriter)
{
    os_boolean success;

	success = in_objectInit(OS_SUPER(_this), kind, deinit);
    if(success)
    {
    	_this->getReader = getReader;
	    _this->getWriter = getWriter;
    }

    return success;
}

void
in_streamPairDeinit(
    in_streamPair _this)
{
	in_objectDeinit(OS_SUPER(_this));
}

in_streamReader
in_streamPairGetReader(
    in_streamPair _this)
{
	return _this->getReader(_this);
}

in_streamWriter
in_streamPairGetWriter(
    in_streamPair _this)
{
	return _this->getWriter(_this);
}

void
in_streamPairFree(in_streamPair _this)
{
	in_objectFree(in_object(_this));
}

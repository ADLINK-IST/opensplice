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
#ifndef IN__STREAM_PAIR_H
#define IN__STREAM_PAIR_H

#include "in__object.h"
#include "in_commonTypes.h"
#include "in_streamPair.h"

#ifdef STREAM_DUMMY
#include "in_streamReader.h"
#include "in_streamWriter.h"
#endif
/**
 * Allow usage of this C code from C++ code. Always include this in a header
 * file.
 */
#if defined (__cplusplus)
extern "C" {
#endif

/** \brief virtual operations */
typedef OS_STRUCT(in_streamReader)* (*in_streamPairGetReaderFunc)(in_streamPair pair);
typedef OS_STRUCT(in_streamWriter)* (*in_streamPairGetWriterFunc)(in_streamPair pair);

/** \brief abstract class  */
OS_STRUCT(in_streamPair)
{
    OS_EXTENDS(in_object);
    in_streamPairGetReaderFunc getReader;
    in_streamPairGetWriterFunc getWriter;
};

/** \brief init */
os_boolean
in_streamPairInit(
    in_streamPair _this,
    in_objectKind kind,
    in_objectDeinitFunc deinit,
    in_streamPairGetReaderFunc getReader,
    in_streamPairGetWriterFunc getWriter);

/** \brief deinit */
void
in_streamPairDeinit(
    in_streamPair _this);

/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif

#endif /* IN__STREAM_PAIR_H */

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
#ifndef IN_STREAM_PAIR_H
#define IN_STREAM_PAIR_H

#include "in__object.h"
#include "in_commonTypes.h"
#include "in_streamReader.h"
#include "in_streamWriter.h"

/**
 * Allow usage of this C code from C++ code. Always include this in a header
 * file.
 */
#if defined (__cplusplus)
extern "C" {
#endif

#define in_streamPair(o) ((in_streamPair)(o))

/**
 * Macro to make the in_objectKeep operation type specific
 */
#define in_streamPairKeep(_this) in_streamPair(in_objectKeep(in_object(_this)))

/** \brief destructor */
void
in_streamPairFree(in_streamPair _this);

in_streamReader
in_streamPairGetReader(
    in_streamPair _this);

in_streamWriter
in_streamPairGetWriter(
    in_streamPair _this);

/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif

#endif /* IN_STREAM_PAIR_H */

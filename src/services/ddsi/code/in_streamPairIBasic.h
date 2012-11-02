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
#ifndef IN_STREAM_PAIR_IBASIC_H
#define IN_STREAM_PAIR_IBASIC_H

#include "in_stream.h"
#include "in__configChannel.h"
#include "in__plugKernel.h"
#include "in_transport.h"

/**
 * Allow usage of this C code from C++ code. Always include this in a header
 * file.
 */
#if defined (__cplusplus)
extern "C" {
#endif

#define in_streamPairIBasic(_o) ((in_streamPairIBasic)_o)

in_streamPairIBasic
in_streamPairIBasicNew(
    const in_configChannel channelConfig,
    in_transport           transport,
    in_plugKernel          plug);


void
in_streamPairIBasicFree(
    in_streamPairIBasic _this);

/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif

#endif /* IN_STREAM_PAIR_IBASIC_H */

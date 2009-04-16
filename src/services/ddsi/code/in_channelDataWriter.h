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
#ifndef IN_CHANNEL_DATA_WRITER_H
#define IN_CHANNEL_DATA_WRITER_H

/* OS abstraction includes. */
#include "os_defs.h"
#include "os_classbase.h"
#include "os_stdlib.h"
#include "u_networkReader.h"
#include "in__configChannel.h"
#include "in_channelTypes.h"
/**
 * Allow usage of this C code from C++ code.
 */
#if defined (__cplusplus)
extern "C" {
#endif

in_channelDataWriter
in_channelDataWriterNew(
    in_channelData data,
    in_configChannel config,
    u_networkReader reader);

/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif

#endif /* IN_CHANNEL_DATA_WRITER_H */


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


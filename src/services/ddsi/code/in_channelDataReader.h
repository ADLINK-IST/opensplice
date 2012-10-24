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
#ifndef IN_CHANNEL_DATA_READER_H
#define IN_CHANNEL_DATA_READER_H

/* OS abstraction includes. */
#include "in__object.h"
#include "u_networkReader.h"
/**
 * Allow usage of this C code from C++ code.
 */
#if defined (__cplusplus)
extern "C" {
#endif

/* The usual cast-method for class in_channelDataReader. Note that because
 * in_channelDataReader does not contain any metadata there is no type checking
 * performed.
 */
#define in_channelDataReader(reader) ((in_channelDataReader)reader)

/**
 * Calls the destructor of the parent class. When the reference count of the
 * base object reaches 0, the deinitializer is called automatically.
 */
#define in_channelDataReaderFree(c) in_objectFree(in_object(c))

#define in_channelDataReaderKeep(c) in_objectKeep(in_object(c))

#define in_channelDataReaderIsValid(c) \
    in_objectIsValidWithKind(in_object(c), IN_OBJECT_KIND_DATA_CHANNEL_READER)

in_channelDataReader
in_channelDataReaderNew(
    in_channelData data,
    in_configChannel config,
    u_networkReader reader);

/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif

#endif /* IN_CHANNEL_DATA_READER_H */


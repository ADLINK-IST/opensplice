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
#ifndef IN__READSTREAM_H
#define IN__READSTREAM_H

#include "in__messageTransformer.h"
#include "in_result.h"
#include "kernelModule.h"

#if defined (__cplusplus)
extern "C" {
#endif

OS_CLASS(in_messageDeserializer);

/**
 * Macro that allows the implementation of type checking when casting an
 * object. The signature of the 'casting macro' must look like this:
 */
#define in_messageDeserializer(_this) ((in_messageDeserializer)_this)

/**
 * Macro that calls the in_objects validity check function with the
 * appropiate type
 */
#define in_messageDeserializerIsValid(_this) in_objectIsValid(in_object(_this))

/**
 * Macro to make the in_objectKeep operation type specific
 */
#define in_messageDeserializerKeep(_this) in_messageDeserializer(in_objectKeep(in_object(_this)))

/**
 * Macro to make the in_objectFree operation type specific
 */
#define in_messageDeserializerFree(_this) in_objectFree(in_object(_this))

/* \brief Creates a new read stream. It is reference counted and must be
 * freed using in_messageDeserializerFree(...)
 */
in_messageDeserializer
in_messageDeserializerNew(
    in_messageTransformerGetBufferFunc getBufferFunc,
    c_voidp getBufferFuncArg);

os_boolean
in_messageDeserializerBegin(
    in_messageDeserializer _this,
    in_data buffer,
    os_uint32 length);

in_data
in_messageDeserializerEnd(
    in_messageDeserializer _this);

/**
 * Deserializes the current buffer into a v_message. The current buffer is
 * expected to only hold userData and NOT a serialized v_message. Therefore
 * the resulting v_message only has the userData part initialized. All other
 * fields like writeTime, sequenceNumber, qos, etc. are NOT filled and need
 * to be set after this call by the caller itself.
 *
 * @param _this The deserializer object.
 * @param type The type of the v_message
 * @param message A pointer to a message that will be allocated and initialized
 *                by this routine by deserialing the contents of the buffer.
 */
in_result
in_messageDeserializerRead(
    in_messageDeserializer _this,
    v_topic topic,
    os_boolean bigEndian,
    v_message* object);

#if defined (__cplusplus)
}
#endif

#endif /* IN__READSTREAM_H */

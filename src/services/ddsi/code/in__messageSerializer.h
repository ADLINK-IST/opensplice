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
#ifndef IN_MESSAGESERIALIZER__H
#define IN_MESSAGESERIALIZER__H

#include "in__messageTransformer.h"
#include "in_result.h"
#include "kernelModule.h"

#if defined (__cplusplus)
extern "C" {
#endif

OS_CLASS(in_messageSerializer);

/**
 * Macro that allows the implementation of type checking when casting an
 * object. The signature of the 'casting macro' must look like this:
 */
#define in_messageSerializer(_this) ((in_messageSerializer)_this)

/**
 * Macro that calls the in_objects validity check function with the
 * appropiate type
 */
#define in_messageSerializerIsValid(_this) in_objectIsValid(in_object(_this))

/**
 * Macro to make the in_objectKeep operation type specific
 */
#define in_messageSerializerKeep(_this) in_objectKeep(in_object(_this))

/**
 * Macro to make the in_objectFree operation type specific
 */
#define in_messageSerializerFree(_this) in_objectFree(in_object(_this))

/**
 * Macro to tell default message serializer codec
 *   */
#ifdef PA_BIG_ENDIAN
#define IN_MESSAGE_SERIALIZER_CODEC_ID  IN_CODEC_CDR_BE
#else
#define IN_MESSAGE_SERIALIZER_CODEC_ID   IN_CODEC_CDR_LE
#endif


in_messageSerializer
in_messageSerializerNew(
    in_messageTransformerGetBufferFunc getBufferFunc,
    c_voidp getBufferFuncArg);

void
in_messageSerializerBegin(
    in_messageSerializer _this,
    in_data buffer,
    os_uint32 length);

in_data
in_messageSerializerEnd(
    in_messageSerializer _this);

/**
 * Serializes ONLY the userData field of the provided v_message into the current
 * buffer. All other fields of the v_message like writeTime, sequenceNumber,
 * qos, etc. are NOT serialized.
 *
 * @param _this The serializer object.
 * @param message The message of which the userData field will be serialized.
 * @param size The number of bytes written into the buffer. This will be filled
 *             by this function.
 */
in_result
in_messageSerializerWrite(
    in_messageSerializer _this,
    v_message message,
    c_long topicDataOffset,
    os_uint32* size);

#if defined (__cplusplus)
}
#endif

#endif /* IN_MESSAGESERIALIZER__H */

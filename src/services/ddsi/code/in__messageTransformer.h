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
#ifndef IN_MESSAGETRANSFORMER__H
#define IN_MESSAGETRANSFORMER__H

/* Database includes */
#include "c_typebase.h"

/* DDSi includes */
#include "in__object.h"
#include "in_locator.h"
#include "in__endianness.h"

#if defined (__cplusplus)
extern "C" {
#endif

OS_CLASS(in_messageTransformer);

typedef unsigned char  *in_data;

typedef void
(* in_messageTransformerGetBufferFunc)(
    in_data* bufferPtr,
    os_uint32* length,
    c_voidp userData);

/**
 * Macro that allows the implementation of type checking when casting an
 * object. The signature of the 'casting macro' must look like this:
 */
#define in_messageTransformer(_this) ((in_messageTransformer)_this)

/**
 * Macro that calls the in_objects validity check function with the
 * appropiate type
 */
#define in_messageTransformerIsValid(_this) in_objectIsValid(in_object(_this))

/**
 * Macro to make the in_objectKeep operation type specific
 */
#define in_messageTransformerKeep(_this) in_objectKeep(in_object(_this))

/**
 * Macro to make the in_objectFree operation type specific
 */
#define in_messageTransformerFree(_this) in_objectFree(in_object(_this))

/**
 * The following enumeration is used to specify the copy method used by
 * read and write operations performed on a stream.
 * Each stream has a copy kind attribute and the value will determine
 * if a read or write operation will swap the data or not.
 */
typedef enum in_messageTransformerCMKind
{
    IN_MESSAGE_TRANSFORMER_CM_KIND_COPY,
    IN_MESSAGE_TRANSFORMER_CM_KIND_SWAP,
    IN_MESSAGE_TRANSFORMER_CM_KIND_COUNT
} in_messageTransformerCMKind;


/**
 * The following in_streamPair class implements the basic stream attributes.
 * This class is the base class for the specialized in_writeStream and
 * in_readStream classes.
 * The common functionality implemented by this base class are:
 *  - association to the channel that provides the network buffers and
 *  - management of the actual network buffer.
 *  - selection of copy method (to swap or not to swap).
 */
OS_STRUCT(in_messageTransformer)
{
    OS_EXTENDS(in_object);

    /* reference to credit value for throttling. */
    os_int32 *bytesLeft;
    /* current buffer */
    in_data bufferPtr;
    /* available space in current buffer. */
    os_uint32 length;
    /* method to retreive next buffer */
    in_messageTransformerGetBufferFunc getBufferFunc;
    c_voidp getBufferFuncArg;
    /* copy with or without swappping */
    in_messageTransformerCMKind copyKind;
    /* indicates the current octet index */
    os_ushort curCdrIndex;
    /* indicates the number of octets in the stream. */
    os_ushort cdrLength;
    os_boolean fragmented;
};

os_boolean
in_messageTransformerInit(
    in_messageTransformer _this,
    in_objectKind kind,
    in_objectDeinitFunc deinit,
    in_messageTransformerGetBufferFunc getBufferFunc,
    c_voidp getBufferFuncArg);

void
in_messageTransformerDeinit(
    in_object _this);


void
in_messageTransformerSetLength(
    in_messageTransformer _this,
    os_uint32 length);

in_data*
in_messageTransformerGetBuffer(
    in_messageTransformer _this);

void
in_messageTransformerSetBuffer(
    in_messageTransformer _this,
    in_data* buffer);

void
in_messageTransformerSetCopyKind(
    in_messageTransformer _this,
    in_messageTransformerCMKind kind);

void
in_messageTransformerSetCdrLength(
    in_messageTransformer _this,
    os_ushort length);

void
in_messageTransformerBegin(
    in_messageTransformer _this);

/**
 * The following macro-methods implement buffer management functions.
 * The goal of these methods is to hide the low level buffer handling
 * of buffer attributes.
 */

/**
 * This buffer handling macro returns the head of the current active
 * network buffer. The head is the first position in the buffer not yet
 * accessed. I.e. the first free position for write buffer and the first
 * to be read position for receive buffer).
 */
#define in_messageTransformerGetHead(_this)                                    \
    in_messageTransformer(_this)->bufferPtr

/**
 * This buffer handling macro returns the number of available bytes of the
 * current active network buffer.
 * I.e. the number of free bytes for a send buffer and the number of bytes
 * to be read for a receive buffer.
 */
#define in_messageTransformerGetAvailable(_this)                               \
    in_messageTransformer(_this)->length

/**
 * This buffer handling macro 'claims' the specified number of bytes from
 * the current active network buffer. I.e. it will set the head position
 * of the current buffer to the new position just after the claimed size
 * and decrease the available bytes with the network buffer with the
 * requested size.
 * Note: that the requested size may NOT exceed the number of available bytes.
 */
#define in_messageTransformerClaim(_this, size)                                \
    assert(in_messageTransformerGetAvailable(_this) >= size);                  \
    in_messageTransformerGetHead(_this) =                                      \
        C_DISPLACE(in_messageTransformerGetHead(_this), size);                 \
    in_messageTransformerGetAvailable(_this) -= size;                          \
	in_messageTransformer(_this)->curCdrIndex += size

/**
 * This buffer handling macro 'renews' the current active network buffer.
 * I.e. it will get a new send or receive buffer and reset the head and
 * number of available bytes.
 */
#define in_messageTransformerRenew(_this)                                      \
    in_messageTransformer(_this)->fragmented = OS_FALSE;                       \
    in_messageTransformer(_this)->getBufferFunc(                               \
        &in_messageTransformer(_this)->bufferPtr,                              \
        &in_messageTransformer(_this)->length,                                 \
        in_messageTransformer(_this)->getBufferFuncArg)

#if defined (__cplusplus)
}
#endif

#endif /* IN_MESSAGETRANSFORMER__H */

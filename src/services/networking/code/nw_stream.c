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
/**
 * This component implements a network stream.
 * A stream is an interface that is an abstraction of the network
 * send and receive buffers.
 * The stream implements the functionality to serialize data to
 * and de-serialize data from the network buffers.
 *
 * This file contains the implementation of 3 classes:
 * - a public  nw_stream class.
 * - a private nw_writeStream class.
 * - a private nw_readStream class.
 *
 * The nw_stream class is the base class for the nw_writeStream and
 * nw_readStream classes.
 */

/* Interface */
#include "nw_stream.h"

/* implementation */
#include <string.h>
#include "os_abstract.h" /* big or little endianness */
#include "os_heap.h"
#include "os_if.h"
#include "c_typebase.h"
#include "c_base.h"
#include "c_metabase.h"
#include "c_collection.h"
#include "nw_plugSendChannel.h"
#include "nw_plugReceiveChannel.h"
#include "nw_report.h"

#include <ctype.h> /* debug: isalnum*/

#ifdef PA_BIG_ENDIAN
#define _COPY_KIND_ CM_COPY
#else
#define _COPY_KIND_ CM_SWAP
#endif

/* ------------------ action arguments ------------------ */

/**
 * These arguments are used by nw_writeStream class as well as by
 * nw_readStream class to pass context information when walking the metadata
 * of collection types and structured types.
 */
typedef struct PropertyActionArg {
    nw_stream stream;   /**< stream */
    c_object object;    /**< property data. */
    c_ulong result;
} *PropertyActionArg;

typedef struct CollectionActionArg {
    nw_stream stream;   /**< stream */
    c_type type;        /**< metadata of the collection's subType. */
    c_ulong result;
} *CollectionActionArg;

/**
 * -------------- nw_stream implementation --------------
 */

/**
 * The usual cast-method for class nw_stream. Note that because nw_stream
 * does not contain any metadata there is no type checking performed.
 */
#define nw_stream(stream) ((nw_stream)stream)

/**
 * The following enumeration is used to specify the copy method used by
 * read and write operations performed on a stream.
 * Each stream has a copy kind attribute and the value will determine
 * if a read or write operation will swap the data or not.
 */
enum nw_copy_method_kind { CM_COPY, CM_SWAP, CM_COUNT };

/**
 * The stream implementation encapsulated the network buffer management.
 * The following typedef specifies the signature of the method that will
 * be used by streams to retrieve network buffers for read or write operations.
 */
typedef void
(* nw_stream_getBufferMethod) (
    nw_plugChannel channel,
    nw_data *buffer,
    nw_length *length);

/**
 * The following nw_stream class implements the basic stream attributes.
 * This class is the base class for the specialized nw_writeStream and
 * nw_readStream classes.
 * The common functionality implemented by this base class are:
 *  - association to the channel that provides the network buffers and
 *  - management of the actual network buffer.
 *  - selection of copy method (to swap or not to swap).
 */
NW_STRUCT(nw_stream) {
    nw_plugChannel            channel;   /* associated channel (buffer provider) */
    nw_signedLength          *bytesLeft; /* reference to credit value for throttling. */

    nw_data                   bufferPtr; /* current buffer */
    nw_length                 length;    /* available space in current buffer. */
    nw_stream_getBufferMethod action;    /* method to retreive next buffer */

    enum nw_copy_method_kind  copyKind;  /* copy with or without swappping */
};

/**
 * The following macro-methods implement buffer management functions.
 * The goal of these methods is to hide the low level buffer handling
 * of buffer attributes.
 */

/**
 * This buffer handling macro returns the head of the current active
 * networtk buffer. The head is the first position in the buffer not yet
 * accessed. I.e. the first free position for write buffer and the first
 * to be read position for receive buffer).
 */
#define nw_stream_head(stream) \
        nw_stream(stream)->bufferPtr

/**
 * This buffer handling macro returns the number of available bytes of the
 * current active network buffer.
 * I.e. the number of free bytes for a send buffer and the number of bytes
 * to be read for a receive buffer.
 */
#define nw_stream_available(stream) \
        nw_stream(stream)->length

/**
 * This buffer handling macro 'claims' the specified number of bytes from
 * the current active network buffer. I.e. it will set the head position
 * of the current buffer to the new position just after the claimed size
 * and decrease the available bytes with the netowork buffer with the
 * requested size.
 * Note: that the requested size may NOT exceed the number of available bytes.
 */
#define nw_stream_claim(stream,size) \
        assert(nw_stream_available(stream) >= size); \
        nw_stream_head(stream) = C_DISPLACE(nw_stream_head(stream),size); \
        nw_stream_available(stream) -= size

/**
 * This buffer handling macro 'renews' the current active network buffer.
 * I.e. it will get a new send or receive buffer and reset the head and
 * number of available bytes.
 */
#define nw_stream_renew(stream) \
        nw_stream(stream)->action(nw_stream(stream)->channel, \
                                  &nw_stream(stream)->bufferPtr, \
                                  &nw_stream(stream)->length);

nw_plugChannel
nw_stream_channel (
    nw_stream _this)
{
    nw_plugChannel result = NULL;

    if (_this) {
        result = _this->channel;
    }
    return result;
}

/**
 * The public methods implemented by the nw_stream class.
 * Currently only a method to close a stream is implemented.
 * construction and access to the stream is specific to the derived classes.
 */
void
nw_stream_close (
    nw_stream _this)
{
    os_free(_this);
}


/**
 * ------------ nw_writeStream implementation ------------
 */

/**
 * The nw_writeStream class extends from the nw_stream class and implements
 * methods to write (serialize) data from database format to network buffers.
 */
C_CLASS(nw_writeStream);

/**
 * The following four function typedefs specify the function signature
 * of type/format specific methods used by this class to copy the data.
 * These methods will be cached in function pointer arrays in the
 * nw_writeStream class. These function pointer arrays together with
 * macro's defined later on provide a mechanism to select the type specific
 * write method by indexing the array with the type kind meta information.
 */
typedef c_ulong
(*nw_stream_writeType_method) (
    nw_stream stream,
    c_type type,
    c_voidp data);

typedef c_ulong
(*nw_stream_writeCollection_method) (
    nw_stream stream,
    c_type type,
    c_voidp data);

typedef c_ulong
(*nw_stream_writePrim_method) (
    nw_stream stream,
    c_ulong typeSize,
    c_voidp data);

typedef c_ulong
(*nw_stream_writeArray_method) (
    nw_stream stream,
    c_ulong typeSize,
    c_ulong length,
    c_voidp data);

/**
 * The following struct is the implementation of the nw_writeStream class.
 * The class contains 4 function pointer tables.
 * - The writePrim table has two entries (CM_COPY and CM_SWAP).
 *   Each entry contains a method to copy or swap primitive types to the
 *   network buffers.
 * - The writeArray table has two entries (CM_COPY and CM_SWAP).
 *   Each entry contains a method to copy or swap an array of primitive types
 *   to the network buffers.
 * - The writeType function pointer table contains an entry for each meta
 *   object kind. Only the entries for types are initialized.
 *   The nw_stream_writeType macro will use this table to select the type
 *   specific copy method by means of indexing the table with the meta object
 *   kind.
 * - The writeCollection function pointer table contains an entry for each meta
 *   collection kind. The nw_stream_writeCollection macro will use this table
 *   to select the collection specific copy method by means of indexing the
 *   table with the meta collection kind.
 * @extends nw_stream_s
 */
NW_STRUCT(nw_writeStream) {
    NW_EXTENDS(nw_stream);
    nw_stream_writePrim_method       writePrim[CM_COUNT];
    nw_stream_writeArray_method      writeArray[CM_COUNT];
    nw_stream_writeType_method       writeType[M_COUNT];
    nw_stream_writeCollection_method writeCollection[C_COUNT];
};

/**
 * The following macro-methods implement type specific stream write methods.
 * The macro implementation abstract the redirection of the call to array indexed
 * more type specific read and write calls.
 * The choice to redirect the calls via an array of functionpointers stems from
 * performance requirements. Redirection via arrays indexed by means of the type
 * kind is faster than performing huge switch statements.
 */

#define nw_writeStream(stream) ((nw_writeStream)stream)

/**
 * The following macro-method implements the functionality to write a primitive
 * value to the stream send buffer. This method will automatically select
 * the desired copy algorithm (to swap or not to swap) by means of the
 * stream specific copy policy (stream->copyKind).
 */
#define nw_stream_writePrim(stream,size,data) \
        nw_writeStream(stream)->writePrim[nw_stream(stream)->copyKind](stream,size,data)

/**
 * The following macro-method implements the functionality to write an array of
 * primitive values to the stream send buffer.
 * This method will automatically select the most optimum copy algorithm.
 * in case the primitive size is 1 there is no need to swap so in that
 * case the write opaq method is called otherwise the desired copy algorithm
 * (to swap or not to swap) is selected by means of the stream specific copy
 * policy (stream->copyKind).
 */
#define nw_stream_writePrimArray(stream,size,length,data) \
        (size == 1 ? nw_stream_writeOpaq(stream,length,data) : \
        nw_writeStream(stream)->writeArray[nw_stream(stream)->copyKind](stream,size,length,data))

/**
 * The following macro-method implements the functionality to write a type
 * value to the stream send buffer. This method will automatically select
 * the type specific copy algorithm by means of the metatdata specified
 * object kind (c_baseObjectKind).
 */
#define nw_stream_writeType(stream, type, object) \
        nw_writeStream(stream)->writeType[c_baseObjectKind(c_baseObject(type))](stream,c_type(type),object)

/**
 * The following macro-method implements the functionality to write a collection
 * type object to the stream send buffer. This method will automatically select
 * the type specific copy algorithm by means of the metatdata specified
 * collection kind (c_collectionTypeKind).
 */
#define nw_stream_writeCollection(stream,type,data) \
        nw_writeStream(stream)->writeCollection[c_collectionTypeKind(type)](stream,type,data)


/* --------- Implementation of the Stream write methods ---------- */

/**
 * This method will write a sequence of octets of the specified length
 * to the network buffers
 * The data is copied into the buffer specified by the method's src argument
 * and the number of bytes copied is returned by this method.
 */
c_ulong
nw_stream_writeOpaq(
    nw_stream stream,
    c_ulong dataSize,
    c_voidp src)
{
    c_octet *dstPtr;
    c_octet *srcPtr;
    c_ulong remainingSize;
    c_ulong copySize;

    srcPtr = (c_octet *)src;
    remainingSize = dataSize;
    while (remainingSize > 0) {
        copySize = nw_stream_available(stream);
        if (copySize == 0) {
            nw_stream_renew(stream);
            copySize = nw_stream_available(stream);
        }
        if (remainingSize > copySize) {
            remainingSize -= copySize;
        } else {
            copySize = remainingSize;
            remainingSize = 0;
        }
        dstPtr = nw_stream_head(stream);
        nw_stream_claim(stream,copySize);
        memcpy(dstPtr, srcPtr, copySize);
        srcPtr = &(srcPtr[copySize]);
    }
    return dataSize;
}

/**
 * This method will write a primitive type to the network buffers based upon
 * the type size.
 * The size is sufficient information to determine how to copy or swap
 * the data and is faster than determining it on the primitive kind.
 * The method will return the number of bytes copied.
 */
static c_ulong
nw_stream__writePrim(
    nw_stream stream,
    c_ulong dataSize,
    c_voidp data)
{
    c_octet *src = (c_octet *)data;
    c_octet *srcPtr;
    c_octet *dstPtr;
    c_ulong i,size,remainingSize;

    dstPtr = nw_stream_head(stream);
    if (dataSize <= nw_stream_available(stream)) {
        nw_stream_claim(stream,dataSize);
        for (i=0; i<dataSize; i++) {
            dstPtr[i] = src[i];
        }
    } else {
        size = nw_stream_available(stream);
        remainingSize = dataSize - size;

        nw_stream_claim(stream,size);
        for (i=0; i<size; i++) {
            dstPtr[i] = src[i];
        }

        nw_stream_renew(stream);
        assert(nw_stream_available(stream) > remainingSize);

        dstPtr = nw_stream_head(stream);
        srcPtr = (c_octet *)&src[size];

        nw_stream_claim(stream,remainingSize);
        for (i=0; i<remainingSize; i++) {
            dstPtr[i] = srcPtr[i];
        }
    }
    return dataSize;
}

/**
 * This method will write a primitive type to the network buffers based upon
 * the type size.
 * The endianess of the data will also be swapped by this method.
 * The size is sufficient information to determine how to copy or swap
 * the data and is faster than determining it on the primitive kind.
 * The method will return the number of bytes copied.
 */
static c_ulong
nw_stream__writePrimSwapped(
    nw_stream stream,
    c_ulong dataSize,
    c_voidp data)
{
    c_octet *src = (c_octet *)data;
    c_octet *dstPtr;
    c_ulong i,size,max,remainingSize;

    dstPtr = nw_stream_head(stream);
    if (dataSize <= nw_stream_available(stream)) {
        max = dataSize-1;

        nw_stream_claim(stream,dataSize);
        for (i=0; i<dataSize; i++) {
            dstPtr[i] = src[max-i];
        }
    } else {
        size = nw_stream_available(stream);
        remainingSize = dataSize - size;
        max = dataSize-1;

        nw_stream_claim(stream,size);
        for (i=0; i<size; i++) {
            dstPtr[i] = src[max-i];
        }
        nw_stream_renew(stream);
        assert(nw_stream_available(stream) > remainingSize);

        dstPtr = nw_stream_head(stream);
        max = remainingSize-1;

        nw_stream_claim(stream,remainingSize);
        for (i=0; i<remainingSize; i++) {
            dstPtr[i] = src[max-i];
        }
    }
    return dataSize;
}

/**
 * This method will write an array of primitive type to the network buffers
 * based upon the type size and array length.
 * The size is sufficient information to determine how to copy or swap
 * the data and is faster than determining it on the primitive kind.
 * The method will return the number of bytes copied.
 */
static c_ulong
nw_stream__writePrimArray(
    nw_stream stream,
    c_ulong dataSize,
    c_ulong length,
    c_voidp data)
{
    c_octet *src = (c_octet *)data;
    c_octet *dstPtr,*srcPtr;
    c_ulong i,rest,size,avail;

    rest = length*dataSize;
    srcPtr = src;

    while (rest) {
        dstPtr = nw_stream_head(stream);
        avail = nw_stream_available(stream);
        if (rest <= avail) {
            size = rest;
            rest = 0;
        } else {
            size = avail;
            rest -= avail;
        }
        nw_stream_claim(stream,size);
        for (i=0; i<size; i++) {
            dstPtr[i] = srcPtr[i];
        }
        if (rest) {
            nw_stream_renew(stream);
            srcPtr = C_DISPLACE(srcPtr,size);
        }
    }
    return dataSize;
}

/**
 * This method will write an array of primitive type to the network buffers
 * based upon the type size and array length.
 * The endianess of the data will also be swapped by this method.
 * The size is sufficient information to determine how to copy or swap
 * the data and is faster than determining it on the primitive kind.
 * The method will return the number of bytes copied.
 */
static c_ulong
nw_stream__writePrimArraySwapped(
    nw_stream stream,
    c_ulong dataSize,
    c_ulong length,
    c_voidp data)
{
    c_octet *src = (c_octet *)data;
    c_octet *dstPtr,*srcPtr;
    c_ulong i,j,n,rest,max,restElements,maxElements;

    restElements = length;
    max = dataSize-1;
    srcPtr = src;

    while (restElements) {
        /* Determine the maximum number of elements
         * that can be copied at once.
         */
        maxElements = nw_stream_available(stream) / dataSize;
        if (restElements <= maxElements) {
            n = restElements;
            restElements = 0;
        } else {
            n = maxElements;
            restElements -= maxElements;
        }

        /* Copy the elements.
         */
        dstPtr = nw_stream_head(stream);
        nw_stream_claim(stream,(n*dataSize));
        for (i=0; i<n; i++) {
            for (j=0;j<dataSize;j++) {
                dstPtr[j] = srcPtr[max-j];
            }
            dstPtr = &dstPtr[j];
            srcPtr = C_DISPLACE(srcPtr,dataSize);
        }

        if (restElements > 0) {
            rest = nw_stream_available(stream);
            if (rest) {
                /* In this case not all elements are copied yet.
                 * Because there was not enough buffer space available.
                 * However there is still some buffer space available
                 * less than the element size.
                 * So the next element is divided over two buffers.
                 */

                /* Copy the first part of the element to the rest of
                 * the current buffer.
                 */
                nw_stream_claim(stream,rest);
                for (j=0; j<rest; j++) {
                    dstPtr[j] = srcPtr[max-j];
                }

                /* Get the next buffer. */
                nw_stream_renew(stream);
                dstPtr = nw_stream_head(stream);

                /* Copy the last part of the element to the head of
                 * the new buffer.
                 */
                nw_stream_claim(stream,(dataSize-rest));
                for (j=rest;j<dataSize;j++) {
                    dstPtr[j-rest] = srcPtr[max-j];
                }
                srcPtr = C_DISPLACE(srcPtr,dataSize);
                restElements--;
            } else {
                nw_stream_renew(stream);
            }
        }
    }
    return dataSize;
}

static c_bool
nw_stream_writePropertyAction (
    c_object object,
    c_voidp arg)
{
    PropertyActionArg a = (PropertyActionArg)arg;
    c_property property;
    c_type type;
    c_object o;

    if (c_baseObjectKind(object) == M_ATTRIBUTE) {
        property = c_property(object);
        type = c_typeActualType(property->type);
        o = C_DISPLACE(a->object, (c_address)property->offset);
        a->result += nw_stream_writeType(a->stream, type, o);
    }
    return TRUE;
}

static c_bool
nw_stream_writeCollectionAction(
    c_object object,
    c_voidp arg)
{
    CollectionActionArg a = (CollectionActionArg)arg;

    a->result += nw_stream_writeType(a->stream, a->type, object);
    return TRUE;
}

static c_ulong
nw_stream_writeStructure (
    nw_stream stream,
    c_type type,
    c_voidp data)
{
    c_structure _this = c_structure(type);
    c_member member;
    c_type memberType;
    c_ulong size, i, result;
    c_voidp ptr;

    result = 0;
    size = c_arraySize(_this->members);
    for (i=0; i<size; i++) {
        member = _this->members[i];
        ptr = C_DISPLACE(data, (c_address)member->offset);
        memberType = c_typeActualType(c_specifierType(member));

        result += nw_stream_writeType(stream, memberType, ptr);
    }
    return result;
}

static c_ulong
nw_stream_writeClass(
    nw_stream stream,
    c_type type,
    c_voidp data)
{
    c_class stack[256];
    c_ulong n = 0, result;
    c_class cls = c_class(type);
    c_bool isValidRef;
    struct PropertyActionArg arg;

    isValidRef = (data != NULL);

    result = nw_stream_writePrim(stream, 1, &isValidRef);

    if (isValidRef) {
        arg.object = data;
        arg.stream = stream;
        arg.result = 0;

        while (cls) {
            stack[n++] = cls;
            cls = cls->extends;
        }
        while(n>0) {
            cls = stack[--n];
            c_metaWalk(c_metaObject(cls),
                       (c_metaWalkAction)nw_stream_writePropertyAction,
                       &arg);
        }
        result += arg.result;
    }
    return result;
}

static c_ulong
nw_stream_writePrimitive (
    nw_stream stream,
    c_type type,
    c_voidp data)
{
    return nw_stream_writePrim(stream, type->size, data);
}

static c_ulong
nw_stream_writeUnion(
    nw_stream stream,
    c_type _type,
    c_voidp data)
{
    c_union utype = c_union(_type);
    c_type type, switchType;
    c_unionCase deflt;
    c_unionCase activeCase;
    c_unionCase currentCase;
    c_value switchValue;
    c_literal label;
    c_object o;
    c_ulong length, i, j, result;
    c_ulong n;

    /* action for the union itself */
    /* No action, but separate actions for the switch and the data */
    /* action(c_type(utype), object, actionArg); */
    /* action for the switch */
    switchType = c_typeActualType(utype->switchType);
    result = nw_stream_writeType(stream, switchType, data);

    /* Determine value of the switch field */
    switch (c_baseObjectKind(switchType)) {
    case M_PRIMITIVE:
        switch (c_primitive(switchType)->kind) {
#define __CASE__(prim, type) \
        case prim: switchValue = type##Value(*((type *)data)); break;
        __CASE__(P_BOOLEAN,c_bool)
        __CASE__(P_CHAR,c_char)
        __CASE__(P_SHORT,c_short)
        __CASE__(P_USHORT,c_ushort)
        __CASE__(P_LONG,c_long)
        __CASE__(P_ULONG,c_ulong)
        __CASE__(P_LONGLONG,c_longlong)
        __CASE__(P_ULONGLONG,c_ulonglong)
#undef __CASE__
        default:
            switchValue = c_undefinedValue();
            assert(FALSE);
        break;
        }
    break;
    case M_ENUMERATION:
        switchValue = c_longValue(*(c_long *)data);
    break;
    default:
        switchValue = c_undefinedValue();
        assert(FALSE);
    break;
    }

    /* Determine the label corresponding to this field */

    activeCase = NULL;
    deflt = NULL;
    length = c_arraySize(utype->cases);

    for (i=0; (i<length) && !activeCase; i++) {
        currentCase = c_unionCase(utype->cases[i]);
        n = c_arraySize(currentCase->labels);
        if (n > 0) {
            for (j=0; (j<n) && !activeCase; j++) {
                label = c_literal(currentCase->labels[j]);
                if (c_valueCompare(switchValue, label->value) == C_EQ) {
                    activeCase = currentCase;
                }
            }
        } else {
            deflt = currentCase;
        }
    }
    if (!activeCase) {
        activeCase = deflt;
    }

    if (activeCase) {
        if (c_type(utype)->alignment >= switchType->size) {
            length = c_type(utype)->alignment;
        } else {
            length = switchType->size;
        }
        o = C_DISPLACE(data, (c_address)length);
        type = c_typeActualType(c_specifierType(activeCase));
        result += nw_stream_writeType(stream, type, o);
    }
    return result;
}

c_ulong
nw_stream_writeString (
    nw_stream stream,
    c_type type,
    c_voidp data)
{
    c_char *ptr = *(c_string *)data;
    c_bool isValidRef;
    nw_data dst;
    c_ulong size, len, avail, result;

    isValidRef = (ptr != NULL);

    result = nw_stream_writePrim(stream, 1, &isValidRef);
    if (isValidRef) {
        len = strlen(ptr) + 1;
        result += len;
        while (len) {
            avail = nw_stream_available(stream);
            if (avail == 0) {
                nw_stream_renew(stream);
                avail = nw_stream_available(stream);
            }
            dst = nw_stream_head(stream);
            size = (len < avail ? len : avail);
            nw_stream_claim(stream,size);
            memcpy(dst,ptr,size);
            ptr = C_DISPLACE(ptr,size);
            len -= size;
        }
    }
    return result;
}

static c_ulong
nw_stream_writeArray (
    nw_stream stream,
    c_type type,
    c_voidp data)
{
    c_collectionType ctype = c_collectionType(type);
    c_ulong length, size, i, result;
    c_type subType;
    c_voidp array;
    c_bool isValidRef;

    if (ctype->maxSize > 0) {
        array = data;
        length = ctype->maxSize;
    } else {
        array = *(c_voidp *)data;
        length = c_arraySize(array);
    }

    result = 0;

    isValidRef = (array != NULL);

    if (isValidRef) {
        if (ctype->maxSize == 0) {
            result += nw_stream_writePrim(stream, 1, &isValidRef);
            result += nw_stream_writePrim(stream, sizeof(c_ulong), &length);
        }
        subType = c_typeActualType(ctype->subType);
        if ((c_baseObjectKind(subType) == M_PRIMITIVE) ||
            (c_baseObjectKind(subType) == M_ENUMERATION))
        {
            result += nw_stream_writePrimArray(stream,subType->size,length,array);
        } else {
            if (c_typeIsRef(subType)) {
                size = sizeof(c_voidp);
            } else {
                size = subType->size;
            }
            for (i=0; i<length; i++) {
                result += nw_stream_writeType(stream, subType, array);
                array = C_DISPLACE(array, size);
            }
        }
    } else {
        if (ctype->maxSize == 0) {
            result += nw_stream_writePrim(stream, 1, &isValidRef);
        }
    }
    return result;
}

static c_ulong
nw_stream_writeSequence (
    nw_stream stream,
    c_type type,
    c_voidp data)
{
    c_collectionType ctype = c_collectionType(type);
    c_ulong length, size, i, result;
    c_type subType;
    c_voidp array;
    c_bool isValidRef;

    array = *(c_voidp *)data;
    length = c_arraySize(array);
    result = 0;

    isValidRef = (array != NULL);

    if (isValidRef) {
        result += nw_stream_writePrim(stream, 1, &isValidRef);
        result += nw_stream_writePrim(stream, sizeof(c_ulong), &length);
        subType = c_typeActualType(ctype->subType);

        if ((c_baseObjectKind(subType) == M_PRIMITIVE) ||
            (c_baseObjectKind(subType) == M_ENUMERATION))
        {
            result += nw_stream_writePrimArray(stream,subType->size,length,array);
        } else {
            if (c_typeIsRef(subType)) {
                size = sizeof(c_voidp);
            } else {
                size = subType->size;
            }
            for (i=0; i<length; i++) {
                result += nw_stream_writeType(stream, subType, array);
                array = C_DISPLACE(array, size);
            }
        }
    } else {
        result += nw_stream_writePrim(stream, 1, &isValidRef);
    }
    return result;
}


static c_ulong
nw_stream_writeSet (
    nw_stream stream,
    c_type type,
    c_voidp data)
{
    c_set set = (c_set)data;
    c_collectionType ctype = c_collectionType(type);
    c_bool isValidRef;
    c_ulong size, result;
    struct CollectionActionArg arg;


    isValidRef = (set != NULL);

    result = nw_stream_writePrim(stream, 1, &isValidRef);

    if (isValidRef) {
        size = c_setCount(set);
        result += nw_stream_writePrim(stream, sizeof(c_ulong), &size);

        arg.type = c_typeActualType(ctype->subType);
        arg.stream = stream;
        arg.result = 0;
        c_setWalk(set, nw_stream_writeCollectionAction, &arg);
        result += arg.result;
    }
    return result;
}

static c_ulong
nw_stream__writeCollection (
    nw_stream stream,
    c_type type,
    c_voidp data)
{
    return nw_stream_writeCollection(stream,type,data);
}

/**
 * Public API
 */
nw_stream
nw_stream_writeOpen (
    nw_plugChannel channel)
{
    nw_stream _this;
    nw_writeStream str;

    assert(channel);
    if (channel) {
        _this = os_malloc(sizeof(NW_STRUCT(nw_writeStream)));

        _this->channel                   = channel;
        _this->copyKind                  = _COPY_KIND_;

        _this->action                    = nw_plugSendChannelGetNextFragment;

        str = nw_writeStream(_this);

        str->writePrim[CM_COPY]          = nw_stream__writePrim;
        str->writePrim[CM_SWAP]          = nw_stream__writePrimSwapped;
        str->writeArray[CM_COPY]         = nw_stream__writePrimArray;
        str->writeArray[CM_SWAP]         = nw_stream__writePrimArraySwapped;

        str->writeType[M_PRIMITIVE]      = nw_stream_writePrimitive;
        str->writeType[M_ENUMERATION]    = nw_stream_writePrimitive;
        str->writeType[M_STRUCTURE]      = nw_stream_writeStructure;
        str->writeType[M_EXCEPTION]      = nw_stream_writeStructure;
        str->writeType[M_CLASS]          = nw_stream_writeClass;
        str->writeType[M_UNION]          = nw_stream_writeUnion;
        str->writeType[M_COLLECTION]     = nw_stream__writeCollection;

        str->writeCollection[C_SET]      = nw_stream_writeSet;
        str->writeCollection[C_STRING]   = nw_stream_writeString;
        str->writeCollection[C_ARRAY]    = nw_stream_writeArray;
        str->writeCollection[C_SEQUENCE] = nw_stream_writeSequence;
    } else {
        _this = NULL;
    }

    return _this;
}

nw_bool
nw_stream_writeBegin (
    nw_stream _this,
    v_networkPartitionId partitionId,
    nw_signedLength *bytesLeft,
    plugSendStatistics pss)
{
    _this->bytesLeft = bytesLeft;
    return nw_plugSendChannelMessageStart(_this->channel,
                                          &_this->bufferPtr,
                                          &_this->length,
                                          partitionId,
                                          bytesLeft,
                                          pss);
}

c_ulong
nw_stream_write (
    nw_stream stream,
    c_object object)
{
    c_type type;
    c_ulong result;

    type = c_getType(object);
    result = nw_stream_writeType(stream, type, object);

    return result;
}

void
nw_stream_writeEnd  (
    nw_stream _this,
    plugSendStatistics pss)
{
    nw_plugSendChannelMessageEnd(_this->channel,
                                 _this->bufferPtr,
                                 pss);
}

/* ------------ nw_readStream implementation ------------ */

/**
 * The nw_readStream class extends from the nw_stream class and implements
 * methods to read (de-serialize) data from network buffers to database format.
 */
C_CLASS(nw_readStream);

/**
 * The following four function typedefs specify the function signature
 * of type/format specific methods used by this class to copy the data.
 * These methods will be cached in function pointer arrays in the
 * nw_readStream class. These function pointer arrays together with
 * macro's defined later on provide a mechanism to select the type specific
 * read method by indexing the array with the type kind meta information.
 */

typedef c_voidp
(*nw_stream_readType_method) (
    nw_stream stream,
    c_type type,
    c_voidp data);

typedef c_voidp
(*nw_stream_readCollection_method) (
    nw_stream stream,
    c_type type,
    c_voidp data);

typedef c_voidp
(*nw_stream_readPrim_method) (
    nw_stream stream,
    c_ulong typeSize,
    c_voidp data);

typedef c_voidp
(*nw_stream_readArray_method) (
    nw_stream stream,
    c_ulong typeSize,
    c_ulong length,
    c_voidp data);

/**
 * The following struct is the implementation of the nw_writeStream class.
 * The class contains 4 function pointer tables.
 * - The writePrim table has two entries (CM_COPY and CM_SWAP).
 *   Each entry contains a method to copy or swap primitive types to the
 *   network buffers.
 * - The writeArray table has two entries (CM_COPY and CM_SWAP).
 *   Each entry contains a method to copy or swap an array of primitive types
 *   to the network buffers.
 * - The writeType function pointer table contains an entry for each meta
 *   object kind. Only the entries for types are initialized.
 *   The nw_stream_writeType macro will use this table to select the type
 *   specific copy method by means of indexing the table with the meta object
 *   kind.
 * - The writeCollection function pointer table contains an entry for each meta
 *   collection kind. The nw_stream_writeCollection macro will use this table
 *   to select the collection specific copy method by means of indexing the
 *   table with the meta collection kind.
 *
 * @extends nw_stream_s
 */
NW_STRUCT(nw_readStream) {
    NW_EXTENDS(nw_stream);
    nw_stream_readPrim_method        readPrim[CM_COUNT];
    nw_stream_readArray_method       readArray[CM_COUNT];
    nw_stream_readType_method        readType[M_COUNT];
    nw_stream_readCollection_method  readCollection[C_COUNT];
};



#define nw_readStream(stream) ((nw_readStream)stream)
/**
 * The following macro-method implements the functionality to read a primitive
 * value from the stream receive buffer. This method will automatically select
 * the desired copy algorithm (to swap or not to swap) by means of the
 * stream specific copy policy (stream->copyKind).
 */
#define nw_stream_readPrim(stream,size,data) \
        nw_readStream(stream)->readPrim[nw_stream(stream)->copyKind](stream,size,data)

/**
 * The following macro-method implements the functionality to read an array of
 * primitive values from the stream receive buffer.
 * This method will automatically select the most optimum copy algorithm.
 * in case the primitive size is 1 there is no need to swap so in that
 * case the read opaq method is called otherwise the desired copy algorithm
 * (to swap or not to swap) is selected by means of the stream specific copy
 * policy (stream->copyKind).
 */
#define nw_stream_readPrimArray(stream,size,length,data) \
        (size == 1 ? nw_stream_readOpaq(stream,length,data) : \
        nw_readStream(stream)->readArray[nw_stream(stream)->copyKind](stream,size,length,data))

/**
 * The following macro-method implements the functionality to read a type
 * value to the stream receive buffer. This method will automatically select
 * the type specific copy algorithm by means of the metatdata specified
 * object kind (c_baseObjectKind).
 */
#define nw_stream_readType(stream, type, data) \
        nw_readStream(stream)->readType[c_baseObjectKind(c_baseObject(type))](stream,c_type(type),data)

/**
 * The following macro-method implements the functionality to read a collection
 * type object from the stream send buffer. This method will automatically select
 * the type specific copy algorithm by means of the metatdata specified
 * collection kind (c_collectionTypeKind).
 */
#define nw_stream_readCollection(stream,type,data) \
        nw_readStream(stream)->readCollection[c_collectionTypeKind(type)](stream,type,data)

/* ----------------- stream read methods --------------------- */

/**
 * This method will read a sequence of octets of the specified length
 * from the network buffers
 * The data is copied into memory specified by the method's data argument
 * and a pointer to the data is returned by this method.
 */
c_voidp
nw_stream_readOpaq(
    nw_stream stream,
    c_ulong dataSize,
    c_voidp data)
{
    c_octet *dst = (c_octet *)data;
    c_octet *dstPtr;
    c_octet *srcPtr;
    c_ulong restSize,copySize;

    dstPtr = dst;
    restSize = dataSize;
    while (restSize > 0) {
        copySize = nw_stream_available(stream);
        if (copySize == 0) {
            nw_stream_renew(stream);
            copySize = nw_stream_available(stream);
        }
        if (copySize < restSize) {
            restSize -= copySize;
        } else {
            copySize = restSize;
            restSize = 0;
        }
        srcPtr = nw_stream_head(stream);
        nw_stream_claim(stream,copySize);
        memcpy(dstPtr, srcPtr, copySize);
        dstPtr = &(dstPtr[copySize]);
    }

    return data;
}

/**
 * This method will read a primitive type from the network buffers based upon
 * the type size.
 * The size is sufficient information to determine how to copy or swap
 * the data and is faster than determining it on the primitive kind.
 * The method will return the address of the primitive.
 */
static c_voidp
nw_stream__readPrim(
    nw_stream stream,
    c_ulong dataSize,
    c_voidp data)
{
    c_octet *dst = (c_octet *)data;
    c_octet *dstPtr;
    c_octet *srcPtr;
    c_ulong i,size,remainingSize;

    srcPtr = nw_stream_head(stream);
    if (dataSize <= nw_stream_available(stream)) {
        nw_stream_claim(stream,dataSize);
        for (i=0; i<dataSize; i++) {
            dst[i] = srcPtr[i];
        }
    } else {
        size = nw_stream_available(stream);
        remainingSize = dataSize - size;

        nw_stream_claim(stream,size);
        for (i=0; i<size; i++) {
            dst[i] = srcPtr[i];
        }

        nw_stream_renew(stream);
        assert(nw_stream_available(stream) >= remainingSize);

        srcPtr = nw_stream_head(stream);
        dstPtr = &dst[size];

        nw_stream_claim(stream,remainingSize);
        for (i=0; i<remainingSize; i++) {
            dstPtr[i] = srcPtr[i];
        }
    }

    return data;
}

/**
 * This method will read a primitive type from the network buffers based upon
 * the type size.
 * The endianess of the data will also be swapped by this method.
 * The size is sufficient information to determine how to copy or swap
 * the data and is faster than determining it on the primitive kind.
 * The method will return the address of the primitive.
 */
static c_voidp
nw_stream__readPrimSwapped(
    nw_stream stream,
    c_ulong dataSize,
    c_voidp data)
{
    c_octet *dst = (c_octet *)data;
    c_octet *srcPtr = nw_stream_head(stream);
    c_ulong i,size,remainingSize,max;

    srcPtr = nw_stream_head(stream);
    if (dataSize <= nw_stream_available(stream)) {
        max = dataSize-1;

        nw_stream_claim(stream,dataSize);
        for (i=0; i<dataSize; i++) {
            dst[max-i] = srcPtr[i];
        }
    } else {
        size = nw_stream_available(stream);
        remainingSize = dataSize - size;
        max = dataSize-1;
        nw_stream_claim(stream,size);
        for (i=0; i<size; i++) {
            dst[max-i] = srcPtr[i];
        }
        nw_stream_renew(stream);
        assert(nw_stream_available(stream) >= remainingSize);

        srcPtr = nw_stream_head(stream);
        max = remainingSize-1;

        nw_stream_claim(stream,remainingSize);
        for (i=0; i<remainingSize; i++) {
            dst[max-i] = srcPtr[i];
        }
    }

    return data;
}

/**
 * This method will read an array of primitive type from the network buffers
 * based upon the type size and array length.
 * The size is sufficient information to determine how to copy or swap
 * the data and is faster than determining it on the primitive kind.
 * The method will return the address of the resulting array.
 */
static c_voidp
nw_stream__readPrimArray(
    nw_stream stream,
    c_ulong dataSize,
    c_ulong length,
    c_voidp data)
{
    c_octet *dstPtr,*srcPtr;
    c_ulong i,size,rest,avail;

    rest = length*dataSize;
    dstPtr = (c_octet *)data;

    while (rest) {
        srcPtr = nw_stream_head(stream);
        avail = nw_stream_available(stream);
        /* Determine the maximum number of elements
         * that can be copied at once.
         */
        if (avail < rest) {
            /* There is less data in the current buffer than required.
             * So assign size to the available amount of bytes that can
             * be copied and reduce the rest amount (restElements) with
             * the available amount.
             */
            size = avail;
            rest -= avail;
        } else {
            /* There is enough data to fill the array.
             * So assign size to the full amount and set the rest amount
             * (restElements) to zero.
             */
            size = rest;
            rest = 0;
        }
        /* Copy the elements.
         */
        nw_stream_claim(stream,size);
        for (i=0; i<size; i++) {
            dstPtr[i] = srcPtr[i];
        }
        if (rest > 0) {
            /* Not all elements are received, so get a new buffer and
             * displace the destination pointer to the current first free
             * position.
             */
            nw_stream_renew(stream);
            dstPtr = C_DISPLACE(dstPtr,size);
        }
    }

    return data;
}

/**
 * This method will read an array of primitive type from the network buffers
 * based upon the type size and array length.
 * The endianess of the data will also be swapped by this method.
 * The size is sufficient information to determine how to copy or swap
 * the data and is faster than determining it on the primitive kind.
 * The method will return the address of the resulting array.
 */
static c_voidp
nw_stream__readPrimArraySwapped(
    nw_stream stream,
    c_ulong dataSize,
    c_ulong length,
    c_voidp data)
{
    c_octet *dstPtr,*srcPtr;
    c_ulong i,j,n,rest,restElements,maxElements;
    c_ulong max = dataSize-1;

    restElements = length;
    dstPtr = (c_octet *)data;

    while (restElements) {
        /* Determine the maximum number of elements
         * that can be copied at once.
         */
        maxElements = nw_stream_available(stream) / dataSize;
        if (maxElements >= restElements) {
            n = restElements;
            restElements = 0;
        } else {
            n = maxElements;
            restElements -= maxElements;
        }
        /* Copy the elements.
         */
        srcPtr = nw_stream_head(stream);
        nw_stream_claim(stream,(n*dataSize));
        for (i=0; i<n; i++) {
            for (j=0;j<dataSize;j++) {
                dstPtr[max-j] = srcPtr[j];
            }
            dstPtr = &dstPtr[j];
            srcPtr = C_DISPLACE(srcPtr,dataSize);
        }
        if (restElements > 0) {
            rest = nw_stream_available(stream);
            if (rest) {
                /* In this case not all elements are copied yet.
                 * Because there was not enough buffer space available.
                 * However there is still some buffer space available
                 * less than the element size.
                 * So the next element is devided over two buffers.
                 */

                /* Copy the first part of the element from the rest of
                 * the current buffer.
                 */
                srcPtr = nw_stream_head(stream);
                nw_stream_claim(stream,rest);
                for (j=0; j<rest; j++) {
                    dstPtr[max-j] = srcPtr[j];
                }

                /* Get the next buffer. */
                nw_stream_renew(stream);

                /* Copy the last part of the element from the rest of
                 * the new buffer.
                 */
                srcPtr = nw_stream_head(stream);
                nw_stream_claim(stream,(dataSize-rest));
                for (j=rest;j<dataSize;j++) {
                    dstPtr[max-j] = srcPtr[j-rest];
                }
                dstPtr = &dstPtr[j];
                restElements--;
            } else {
                nw_stream_renew(stream);
            }
        }
    }

    return data;
}

C_CLASS(nw_stream_fragment);
NW_STRUCT(nw_stream_fragment) {
    c_char *data;
    c_long size;
};

c_voidp
nw_stream_readString(
    nw_stream stream,
    c_type type,
    c_voidp data)
{
    c_string str = NULL;
    c_iter fragments = NULL;
    nw_stream_fragment fragment;
    c_char *copyPtr;
    c_char *stringStart;
    c_char *stringSave;
    c_ulong len, size, totalLength;
    c_ulong saveLen;
    c_bool isValidRef;
    c_bool isReady;

    nw_stream_readPrim(stream, 1, &isValidRef);

    /* assert(isValidRef==0 || isValidRef == 1);  paranoid check for boolean value */
    NW_REPORT_INFO_1(5, "nw_stream__readString ?/%d", stream->length);

    if (isValidRef) {
        stringStart = (c_char *)nw_stream_head(stream);
        copyPtr = stringStart;
        stringSave = NULL;
        saveLen = 0;
        totalLength = 0;
        isReady = FALSE;
        do {
            len = nw_stream_available(stream);
            if (len == 0) {
                nw_stream_renew(stream);
                len = nw_stream_available(stream);
                stringStart = (c_char *)(nw_stream_head(stream));
                copyPtr = stringStart;
            }
            for (size=0; (size<len) && (*copyPtr != '\0'); size++) {
                copyPtr++;
            }
            if (size == len) {
                nw_stream_claim(stream,len);
                fragment = os_malloc(sizeof(NW_STRUCT(nw_stream_fragment)));
                fragment->size = size;
                fragment->data = os_malloc(size);
                memcpy(fragment->data, stringStart, size);
                fragments = c_iterAppend(fragments,fragment);
            } else {
                size++;
                nw_stream_claim(stream,size);
                isReady = TRUE;
            }
            totalLength += size;
        } while (!isReady);

        if (c_iterLength(fragments)) {
            if (type) {
                str = c_stringMalloc(c_getBase(type), totalLength);
            } else {
                str = os_malloc(totalLength);
            }
            fragment = c_iterTakeFirst(fragments);
            copyPtr = str;
            while (fragment) {
                memcpy(copyPtr,fragment->data,fragment->size);
                copyPtr = &copyPtr[fragment->size];
                totalLength -= fragment->size;
                os_free(fragment->data);
                os_free(fragment);
                fragment = c_iterTakeFirst(fragments);
            }
            /* c_iterFree(fragments); // TODO FIXME, "fragments" still pointing onto first element which has been free-ed already !! */
            assert(totalLength > 0);
            assert(stringStart[totalLength-1] == '\0');
            memcpy(copyPtr,stringStart,totalLength);
        } else {
            if (type) {
                str = c_stringNew(c_getBase(type), stringStart);
            } else {
                str = os_malloc(totalLength);
                assert(stringStart[totalLength-1] == '\0');
                memcpy(str,stringStart,totalLength);
            }
        }
    }

    *(c_string *)data = str;
    return str;
}

static c_bool
nw_stream_readPropertyAction (
    c_object object,
    c_voidp arg)
{
    PropertyActionArg a = (PropertyActionArg)arg;
    c_property property;
    c_type type;
    c_voidp data;

    /* For now, we are interested in properties only */
    if (c_baseObjectKind(object) == M_ATTRIBUTE) {
        property = c_property(object);
        type = c_typeActualType(property->type);
        data = C_DISPLACE(a->object, (c_address)property->offset);
        nw_stream_readType(a->stream, type, data);
    }
    return TRUE;
}

static c_voidp
nw_stream_readPrimitive (
    nw_stream stream,
    c_type type,
    c_object data)
{
    return nw_stream_readPrim(stream, type->size, data);
}

static c_voidp
nw_stream_readStructure (
    nw_stream stream,
    c_type type,
    c_voidp data)
{
    c_structure _this = c_structure(type);
    c_member member;
    c_ulong size, i;
    c_voidp o;

    size = c_arraySize(_this->members);
    for (i=0; i<size; i++) {
        member = _this->members[i];
        o = C_DISPLACE(data, (c_address)member->offset);
        nw_stream_readType(stream,
                           c_typeActualType(c_specifier(member)->type),
                           o);
    }

    return data;
}

static c_voidp
nw_stream_readClass (
    nw_stream stream,
    c_type type,
    c_voidp data)
{
    c_voidp result;
    c_class cls = c_class(type);
    c_class stack[256];
    c_ulong n;
    c_bool isValidRef;
    struct PropertyActionArg arg;

    nw_stream_readPrim(stream, 1, &isValidRef);
    if (isValidRef) {
        if (data) {
            result = data;
        } else {
            result = c_new(type);
        }
        if (result) {
            arg.object = result;
            arg.stream = stream;

            n=0;
            while (cls) {
                stack[n++] = cls;
                cls = cls->extends;
            }

            while(n>0) {
                cls = stack[--n];
                c_metaWalk(c_metaObject(cls),
                           (c_metaWalkAction)nw_stream_readPropertyAction,
                           &arg);
            }
        }
    } else {
        result = NULL;
    }

    return result;
}

static c_voidp
nw_stream_readUnion (
    nw_stream stream,
    c_type type,
    c_voidp data)
{
    c_union utype = c_union(type);
    c_ulong length, i, j;
    c_voidp o;
    c_type switchType, caseType;
    c_unionCase deflt;
    c_unionCase activeCase;
    c_unionCase currentCase;
    c_value switchValue;
    c_literal label;
    c_ulong n;

    /* action for the union itself */
    /* No action, but separate actions for the switch and the data */
    /* action(c_type(utype), object, actionArg); */
    /* action for the switch */
    switchType = c_typeActualType(utype->switchType);
    nw_stream_readType(stream, switchType, data);

    /* Determine value of the switch field */
    switch (c_baseObjectKind(switchType)) {
    case M_PRIMITIVE:
        switch (c_primitiveKind(switchType)) {
#define __CASE__(prim, type) \
        case prim: switchValue = type##Value(*((type *)data)); break;
        __CASE__(P_BOOLEAN,c_bool)
        __CASE__(P_CHAR,c_char)
        __CASE__(P_SHORT,c_short)
        __CASE__(P_USHORT,c_ushort)
        __CASE__(P_LONG,c_long)
        __CASE__(P_ULONG,c_ulong)
        __CASE__(P_LONGLONG,c_longlong)
        __CASE__(P_ULONGLONG,c_ulonglong)
#undef __CASE__
        default:
            switchValue = c_undefinedValue();
            assert(FALSE);
        break;
        }
    break;
    case M_ENUMERATION:
        switchValue = c_longValue(*(c_long *)data);
    break;
    default:
        switchValue = c_undefinedValue();
        assert(FALSE);
    break;
    }

    /* Determine the label corresponding to this field */

    activeCase = NULL;
    deflt = NULL;
    length = c_arraySize(utype->cases);

    for (i=0; (i<length) && !activeCase; i++) {
        currentCase = c_unionCase(utype->cases[i]);
        n = c_arraySize(currentCase->labels);
        if (n > 0) {
            for (j=0; (j<n) && !activeCase; j++) {
                label = c_literal(currentCase->labels[j]);
                if (c_valueCompare(switchValue, label->value) == C_EQ) {
                    activeCase = currentCase;
                }
            }
        } else {
            deflt = currentCase;
        }
    }
    if (!activeCase) {
        activeCase = deflt;
    }

    if (activeCase) {
        if (c_type(utype)->alignment >= utype->switchType->size) {
            length = c_type(utype)->alignment;
        } else {
            length = utype->switchType->size;
        }

        o = C_DISPLACE(data, (c_address)length);
        caseType = c_typeActualType(c_specifierType(activeCase));
        nw_stream_readType(stream, caseType, o);
    }

    return data;
}

static c_voidp
nw_stream_readArray (
    nw_stream stream,
    c_type type,
    c_voidp data)
{
    c_collectionType ctype = c_collectionType(type);
    c_bool isValidRef;
    c_ulong size, length, i;
    c_voidp array = NULL;
    c_object o;
    c_type actualType;

    assert(stream);
    assert(type);
    assert(data);

    if (ctype->maxSize > 0) {
        array = data;
        length = ctype->maxSize;
    } else {
        nw_stream_readPrim(stream, 1, &isValidRef);
        if (isValidRef) {
            nw_stream_readPrim(stream, sizeof(c_ulong), &length);
            array = c_newArray(ctype,length);
            *(c_voidp *)data = array;
        }
    }

    if (array) {
        actualType = c_typeActualType(ctype->subType);

        if ((c_baseObject(actualType)->kind == M_PRIMITIVE) ||
            (c_baseObject(actualType)->kind == M_ENUMERATION))
        {
            nw_stream_readPrimArray(stream,actualType->size,length,array);
        } else {
            if (c_typeIsRef(actualType)) {
                size = sizeof(c_voidp);
            } else {
                size = actualType->size;
            }
            o = array;
            for (i=0; i<length; i++) {
                nw_stream_readType(stream, actualType, o);
                o = C_DISPLACE(o, size);
            }
        }
    }

    return array;
}

static c_voidp
nw_stream_readSequence (
    nw_stream stream,
    c_type type,
    c_voidp data)
{
    c_collectionType ctype = c_collectionType(type);
    c_bool isValidRef;
    c_ulong size, length, i;
    c_voidp sequence = NULL;
    c_object o;
    c_type actualType;

    assert(stream);
    assert(type);
    assert(data);

    nw_stream_readPrim(stream, 1, &isValidRef);

    if (isValidRef) {
        nw_stream_readPrim(stream, sizeof(c_ulong), &length);
        sequence = c_newSequence(ctype,length);
        *(c_voidp *)data = sequence;
    }

    if (sequence) {
        actualType = c_typeActualType(ctype->subType);

        if ((c_baseObject(actualType)->kind == M_PRIMITIVE) ||
            (c_baseObject(actualType)->kind == M_ENUMERATION))
        {
            nw_stream_readPrimArray(stream,actualType->size,length,sequence);
        } else {
            if (c_typeIsRef(actualType)) {
                size = sizeof(c_voidp);
            } else {
                size = actualType->size;
            }
            o = sequence;
            for (i=0; i<length; i++) {
                nw_stream_readType(stream, actualType, o);
                o = C_DISPLACE(o, size);
            }
        }
    }

    return sequence;
}

static c_voidp
nw_stream_readSet (
    nw_stream stream,
    c_type type,
    c_voidp data)
{
    c_collectionType ctype = c_collectionType(type);
    c_bool isValidRef;
    c_set set = NULL;
    c_ulong size, i;
    c_object o, inserted;

    nw_stream_readPrim(stream, 1, &isValidRef);

    if (isValidRef) {
        set = c_setNew(ctype->subType);

        /* Read the number of elements to be read. */
        nw_stream_readPrim(stream, sizeof(c_ulong), &size);

        /* Read all elements and insert them into the set. */
        for (i=0; i<size; i++) {
            o = c_new(ctype->subType);
            if (o) {
                nw_stream_readType(stream, c_typeActualType(ctype->subType), o);
                inserted = c_setInsert(set, o);
                assert(inserted == o);
                c_free(o);
            } else {
                assert(FALSE);
            }
        }
    }

    return (c_voidp)set;
}

static c_voidp
nw_stream__readCollection (
    nw_stream stream,
    c_type type,
    c_voidp data)
{
    return nw_stream_readCollection(stream,type,data);
}

/**
 * Public methods of the nw_readStream class.
 */

nw_stream
nw_stream_readOpen (
    nw_plugChannel channel)
{
    nw_stream _this;
    nw_readStream str;

    assert(channel);
    if (channel) {
        _this = os_malloc(sizeof(NW_STRUCT(nw_readStream)));

        _this->channel                   = channel;
        _this->copyKind                  = _COPY_KIND_;

        _this->action                    = nw_plugReceiveChannelGetNextFragment;

        str = nw_readStream(_this);

        str->readPrim[CM_COPY]           = nw_stream__readPrim;
        str->readPrim[CM_SWAP]           = nw_stream__readPrimSwapped;
        str->readArray[CM_COPY]          = nw_stream__readPrimArray;
        str->readArray[CM_SWAP]          = nw_stream__readPrimArraySwapped;

        str->readType[M_PRIMITIVE]       = nw_stream_readPrimitive;
        str->readType[M_ENUMERATION]     = nw_stream_readPrimitive;
        str->readType[M_STRUCTURE]       = nw_stream_readStructure;
        str->readType[M_EXCEPTION]       = nw_stream_readStructure;
        str->readType[M_CLASS]           = nw_stream_readClass;
        str->readType[M_UNION]           = nw_stream_readUnion;
        str->readType[M_COLLECTION]      = nw_stream__readCollection;

        str->readCollection[C_SET]       = nw_stream_readSet;
        str->readCollection[C_STRING]    = nw_stream_readString;
        str->readCollection[C_ARRAY]     = nw_stream_readArray;
        str->readCollection[C_SEQUENCE]  = nw_stream_readSequence;
    } else {
        _this = NULL;
    }

    return _this;
}

c_bool
nw_stream_readBegin (
    nw_stream _this,
    nw_senderInfo sender,
    plugReceiveStatistics prs)
{

    c_bool result = nw_plugReceiveChannelMessageStart(_this->channel,
                                             &_this->bufferPtr,
                                             &_this->length,
                                             sender,
                                             prs);

    return result;
}

c_object
nw_stream_read (
    nw_stream stream,
    c_type type)
{
    c_object object = NULL;

    if (type) {
        if (c_typeIsRef(type)) {
            object = c_new(type);
            if (object) {
                object = nw_stream_readType(stream, type, object);
                assert(object);
            } else {
                assert(FALSE);
            }
        } else {
            /* Class and interface reference created by deser algorithm */
            object = NULL;
            assert(0);
        }
    } else {
        nw_plugReceiveChannelMessageIgnore(stream->channel);
    }

    return object;
}

void
nw_stream_readEnd  (
    nw_stream _this,
    plugReceiveStatistics prs)
{
    nw_plugReceiveChannelMessageEnd(_this->channel,prs);
}


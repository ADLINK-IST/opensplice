/* OS abstraction layer includes */
#include "os_heap.h"

/* Database includes */
#include "c_base.h"
#include "v_topic.h"
/* DDSi includes */
#include "in_locator.h"
#include "in__messageDeserializer.h"
#include "in_align.h"

/**
 * The following four function typedefs specify the function signature
 * of type/format specific methods used by this class to copy the data.
 * These methods will be cached in function pointer arrays in the
 * in_messageDeserializer class. These function pointer arrays together with
 * macro's defined later on provide a mechanism to select the type specific
 * read method by indexing the array with the type kind meta information.
 */
typedef void
(*in_messageDeserializerReadTypeMethod)(
    in_messageDeserializer _this,
    c_type type,
    c_voidp data);

typedef void
(*in_messageDeserializerReadCollectionMethod)(
    in_messageDeserializer _this,
    c_type type,
    c_voidp data);

typedef void
(*in_messageDeserializerReadPrimMethod)(
    in_messageDeserializer _this,
    os_uint32 typeSize,
    c_voidp data);

typedef void
(*in_messageDeserializerReadArrayMethod)(
    in_messageDeserializer _this,
    os_uint32 typeSize,
    os_uint32 length,
    c_voidp data);

/* Struct used in a callback routine  to read properties of classes */
typedef struct in_messageDeserializerPropertyActionArg
{
    in_messageDeserializer transformer;
    c_object object;
} in_messageDeserializerPropertyActionArg;

/**
 * The following struct is the implementation of the in_messageDeserializer class.
 * The class contains 4 function pointer tables.
 * - The readPrim table has two entries (IN_MESSAGE_TRANSFORMER_CM_KIND_COPY and
 *   IN_MESSAGE_TRANSFORMER_CM_KIND_SWAP).
 *   Each entry contains a method to copy or swap primitive types from the
 *   network buffers.
 * - The readArray table has two entries (IN_MESSAGE_TRANSFORMER_CM_KIND_COPY
 *   and IN_MESSAGE_TRANSFORMER_CM_KIND_SWAP).
 *   Each entry contains a method to copy or swap an array of primitive types
 *   from the network buffers.
 * - The readType function pointer table contains an entry for each
 *   meta object kind. Only the entries for types are initialized.
 *   The in_messageDeserializerReadType macro will use this table to select the type
 *   specific copy method by means of indexing the table with the meta object
 *   kind.
 * - The readCollection function pointer table contains an entry for each
 *   relevant meta collection kind. The in_messageDeserializerReadCollection macro will
 *   use this table to select the collection specific copy method by means of
 *   indexing the table with the meta collection kind.
 */
OS_STRUCT(in_messageDeserializer)
{
    OS_EXTENDS(in_messageTransformer);
    in_messageDeserializerReadPrimMethod        readPrim[IN_MESSAGE_TRANSFORMER_CM_KIND_COUNT];
    in_messageDeserializerReadArrayMethod       readArray[IN_MESSAGE_TRANSFORMER_CM_KIND_COUNT];
    in_messageDeserializerReadTypeMethod        readType[M_COUNT];
    in_messageDeserializerReadCollectionMethod  readCollection[C_COUNT];
};

static os_boolean
in_messageDeserializerInit(
    in_messageDeserializer _this,
    in_objectKind kind,
    in_objectDeinitFunc deinit,
    in_messageTransformerGetBufferFunc getBufferFunc,
    c_voidp getBufferFuncArg);

static void
in_messageDeserializerDeinit(
    in_object _this);

static void
in_messageDeserializerReadStructure(
    in_messageDeserializer _this,
    c_type type,
    c_voidp data);

static void
in_messageDeserializerReadUnion(
    in_messageDeserializer _this,
    c_type type,
    c_voidp data);

static void
in_messageDeserializerReadPrimitive(
    in_messageDeserializer _this,
    c_type type,
    c_object data);

static void
in_messageDeserializer__readPrim(
    in_messageDeserializer _this,
    os_uint32 size,
    c_voidp data);

static void
in_messageDeserializer__readPrimSwapped(
    in_messageDeserializer _this,
    os_uint32 dataSize,
    c_voidp data);

static void
in_messageDeserializerReadArray(
    in_messageDeserializer _this,
    c_type type,
    c_voidp data);

static void
in_messageDeserializer__readPrimArray(
    in_messageDeserializer _this,
    os_uint32 dataSize,
    os_uint32 length,
    c_voidp data);

static void
in_messageDeserializer__readPrimArraySwapped(
    in_messageDeserializer _this,
    os_uint32 dataSize,
    os_uint32 length,
    c_voidp data);

static void
in_messageDeserializer__readCollection(
    in_messageDeserializer _this,
    c_type type,
    c_voidp data);

static void
in_messageDeserializerReadString(
    in_messageDeserializer _this,
    c_type type,
    c_voidp data);

static os_boolean
in_messageDeserializerReadPropertyAction(
    c_object object,
    c_voidp arg);

static void
in_messageDeserializerReadClass(
    in_messageDeserializer _this,
    c_type type,
    c_voidp data);


/** */
static os_boolean
_requiresSwap(os_boolean bigEndianess)
{
#ifdef PA_BIG_ENDIAN
    os_boolean requiresSwap = !bigEndianess;
    return requiresSwap;
#else
    os_boolean requiresSwap = bigEndianess;
    return requiresSwap;
#endif
}

/* The following macro remove any CDR inflicated padding from the transformer
 * and ensures the transformer is set to the right position to read the next data
 * value. This may involve updating the transformer head to the next buffer!
 * SIDE EFFECT: it will update the curCdrIndex parameter to it's new value!
 */
#define in_messageDeserializerRemoveCDRPaddingFromStream(_this,size)          \
    do {                                                                          \
    in_messageTransformer __str = in_messageTransformer(_this);                \
    do {                                                                          \
        os_uint32 __padding;                                                   \
                                                                               \
        __padding = IN_ALIGN_UINT_PAD(__str->curCdrIndex,size);                \
		/* printf("index %d size %d padding %d\n", __str->curCdrIndex, size, __padding) ; */       \
        if(__padding != 0)                                                     \
        {                                                                      \
            if(__padding <= in_messageTransformerGetAvailable(__str))          \
            {                                                                  \
                in_messageTransformerClaim(__str, __padding);                  \
            } else                                                             \
            {																   \
			    os_uint32 __remPadding;                                        \
                __remPadding = __padding;                                      \
                __remPadding -= in_messageTransformerGetAvailable(__str);      \
                in_messageTransformerClaim(__str,                              \
                    in_messageTransformerGetAvailable(__str));                 \
                in_messageTransformerRenew(__str);                             \
                assert(in_messageTransformerGetAvailable(__str) >=             \
                    __remPadding);                                             \
                in_messageTransformerClaim(__str, __remPadding);               \
            }                                                                  \
        }                                                                      \
    }while(0);  }while(0)

/**
 * The following macro-method implements the functionality to read a type
 * value to the transformer receive buffer. This method will automatically select
 * the type specific copy algorithm by means of the metadata specified
 * object kind (c_baseObjectKind).
 */
#define in_messageDeserializerReadType(deser, type, data)                      \
        deser->readType[c_baseObjectKind(c_baseObject(type))](                 \
            deser,                                                             \
            c_type(type),                                                      \
            data)                                                              \

/**
 * The following macro-method implements the functionality to read a primitive
 * value from the transformer receive buffer. This method will automatically select
 * the desired copy algorithm (to swap or not to swap) by means of the
 * transformer specific copy policy (transformer->copyKind).
 */
#define in_messageDeserializerReadPrim(deser, size, data)                     \
        deser->readPrim[in_messageTransformer(deser)->copyKind](deser, size, data)

/**
 * The following macro-method implements the functionality to read an array of
 * primitive values from the transformer receive buffer.
 * This method will automatically select the most optimum copy algorithm.
 * the desired copy algorithm (to swap or not to swap) is selected by means of
 * the transformer specific copy policy (transformer->copyKind).
 */
#define in_messageDeserializerReadPrimArray(deser, size, length, data)         \
        deser->readArray[in_messageTransformer(deser)->copyKind](              \
            deser,                                                             \
            size,                                                              \
            length,                                                            \
            data))                                                             \

/**
 * The following macro-method implements the functionality to read a collection
 * type object from the transformer receive buffer. This method will automatically
 * select the type specific copy algorithm by means of the metatdata specified
 * collection kind (c_collectionTypeKind).
 */
#define in_messageDeserializerReadCollection(deser, type, data)                \
        deser->readCollection[c_collectionTypeKind(type)](deser, type, data)

in_messageDeserializer
in_messageDeserializerNew(
    in_messageTransformerGetBufferFunc getBufferFunc,
    c_voidp getBufferFuncArg)
{
    in_messageDeserializer _this = NULL;


    _this = os_malloc(sizeof(OS_STRUCT(in_messageDeserializer)));
    if(_this != NULL)
    {
        os_boolean success;

        success = in_messageDeserializerInit(
            _this,
            IN_OBJECT_KIND_MESSAGE_DESERIALIZER,
            in_messageDeserializerDeinit,
            getBufferFunc,
            getBufferFuncArg);

        if(!success)
        {
            os_free(_this);
            _this = NULL;
        }
    }
    return _this;
}

os_boolean
in_messageDeserializerInit(
    in_messageDeserializer _this,
    in_objectKind kind,
    in_objectDeinitFunc deinit,
    in_messageTransformerGetBufferFunc getBufferFunc,
    c_voidp getBufferFuncArg)
{
    os_boolean success;

    assert(_this);
    assert(kind < IN_OBJECT_KIND_COUNT);
    assert(kind > IN_OBJECT_KIND_INVALID);
    assert(deinit);

    success = in_messageTransformerInit(
        in_messageTransformer(_this),
        kind,
        deinit,
        getBufferFunc,
        getBufferFuncArg);

    if(success)
    {
        _this->readType[M_PRIMITIVE] = in_messageDeserializerReadPrimitive;
        _this->readType[M_ENUMERATION] = in_messageDeserializerReadPrimitive;
        _this->readType[M_STRUCTURE] = in_messageDeserializerReadStructure;
        _this->readType[M_EXCEPTION] = in_messageDeserializerReadStructure;
        _this->readType[M_CLASS] = in_messageDeserializerReadClass;
        _this->readType[M_UNION] = in_messageDeserializerReadUnion;
        _this->readType[M_COLLECTION] = in_messageDeserializer__readCollection;

        _this->readPrim[IN_MESSAGE_TRANSFORMER_CM_KIND_COPY] = in_messageDeserializer__readPrim;
        _this->readPrim[IN_MESSAGE_TRANSFORMER_CM_KIND_SWAP] = in_messageDeserializer__readPrimSwapped;
        _this->readArray[IN_MESSAGE_TRANSFORMER_CM_KIND_COPY] = in_messageDeserializer__readPrimArray;
        _this->readArray[IN_MESSAGE_TRANSFORMER_CM_KIND_SWAP] = in_messageDeserializer__readPrimArraySwapped;

        _this->readCollection[C_STRING] = in_messageDeserializerReadString;
        _this->readCollection[C_ARRAY] = in_messageDeserializerReadArray;
        _this->readCollection[C_SEQUENCE] = in_messageDeserializerReadArray;
    }
    return success;
}

void
in_messageDeserializerDeinit(
    in_object _this)
{
    assert(_this);
    assert(in_messageDeserializerIsValid(_this));

    in_messageTransformerDeinit(_this);
}

os_boolean
in_messageDeserializerBegin(
    in_messageDeserializer _this,
    in_data buffer,
    os_uint32 length)
{
    os_boolean result = OS_TRUE;
    in_messageTransformer transformer = NULL;

    assert(_this);
    assert(in_messageDeserializerIsValid(_this));

    transformer = in_messageTransformer(_this);
    in_messageTransformerBegin(transformer);
    in_messageTransformerSetBuffer(transformer, &buffer);
    in_messageTransformerSetLength(transformer, length);

    return result;
}

in_data
in_messageDeserializerEnd(
    in_messageDeserializer _this)
{
    in_messageTransformer transformer = NULL;
    in_data* buffer = NULL;

    assert(_this);
    assert(in_messageDeserializerIsValid(_this));

    transformer = in_messageTransformer(_this);
    buffer = in_messageTransformerGetBuffer(transformer);
    return *buffer;
}

/* may return null */
in_result
in_messageDeserializerRead(
    in_messageDeserializer _this,
    v_topic topic,
    os_boolean isBigEndian,
    v_message* object)
{
    in_result result = IN_RESULT_OK;
    in_messageTransformer transformer = in_messageTransformer(_this);
    c_object displacedMessage;
    c_type messageType;

    assert(_this);
    assert(in_messageDeserializerIsValid(_this));
    assert(object);
    assert(topic);

    if (topic)
    {
        /* work around, replace cdrLength in transformer by ->length */
        in_messageTransformerSetCdrLength(transformer, (os_ushort) (transformer->length));
        if (_requiresSwap(isBigEndian)) {
            in_messageTransformerSetCopyKind(transformer, IN_MESSAGE_TRANSFORMER_CM_KIND_SWAP);
        } else {
            in_messageTransformerSetCopyKind(transformer, IN_MESSAGE_TRANSFORMER_CM_KIND_COPY);
        }
        messageType = v_topicMessageType(topic);

        if (c_typeIsRef(messageType))
        {
            *object = v_topicMessageNew(topic);

            if(*object == NULL)
            {
                result = IN_RESULT_OUT_OF_MEMORY;
            } else
            {
                displacedMessage = v_topicData(topic, *object);
                in_messageDeserializerReadType(_this, v_topicDataType(topic),
                    displacedMessage);
            }
        } else
        {
            assert(FALSE);
            result = IN_RESULT_ERROR;
            *object = NULL;
        }
    } else
    {
        *object = NULL;
    }
    return result;
}

void
in_messageDeserializerReadStructure(
    in_messageDeserializer _this,
    c_type type,
    c_voidp data)
{
    c_structure structure = c_structure(type);
    c_member member;
    c_type memberType;
    os_uint32 size;
    os_uint32 i;
    c_voidp o;

    assert(_this);
    assert(in_messageDeserializerIsValid(_this));
    assert(type);
    assert(data);

    size = c_arraySize(structure->members);
    for (i = 0; i < size; i++)
    {
        member = structure->members[i];
        assert(member);
        o = C_DISPLACE(data, (c_address)member->offset);
        memberType = c_typeActualType(c_specifierType(member));
        in_messageDeserializerReadType(_this, memberType, o);
    }
}

void
in_messageDeserializerReadUnion(
    in_messageDeserializer _this,
    c_type type,
    c_voidp data)
{
    c_union utype = c_union(type);
    c_type caseType;
    c_type switchType;
    c_unionCase deflt;
    c_unionCase activeCase;
    c_unionCase currentCase;
    c_value switchValue;
    c_literal label;
    c_voidp o;
    os_uint32 length;
    os_uint32 i;
    os_uint32 j;
    os_uint32 n;

    assert(_this);
    assert(type);
    assert(data);

    /* action for the switch */
    switchType = c_typeActualType(utype->switchType);
    in_messageDeserializerReadType(_this, switchType, data);

    /* Determine value of the switch field */
    switch (c_baseObjectKind(switchType))
    {
    case M_PRIMITIVE:
        switch (c_primitiveKind(switchType))
        {
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

    for (i = 0; (i < length) && !activeCase; i++)
    {
        currentCase = c_unionCase(utype->cases[i]);
        n = c_arraySize(currentCase->labels);
        if (n > 0)
        {
            for (j = 0; (j < n) && !activeCase; j++)
            {
                label = c_literal(currentCase->labels[j]);
                if (c_valueCompare(switchValue, label->value) == C_EQ)
                {
                    activeCase = currentCase;
                }
            }
        } else
        {
            deflt = currentCase;
        }
    }
    if(!activeCase)
    {
        activeCase = deflt;
    }
    assert(activeCase);
    if (activeCase)
    {
        if (c_type(utype)->alignment >= utype->switchType->size)
        {
            length = c_type(utype)->alignment;
        } else
        {
            length = utype->switchType->size;
        }
        o = C_DISPLACE(data, (c_address)length);
        caseType = c_typeActualType(c_specifierType(activeCase));
        in_messageDeserializerReadType(_this, caseType, o);
    }
}

void
in_messageDeserializerReadPrimitive(
    in_messageDeserializer _this,
    c_type type,
    c_object data)
{
    assert(_this);
    assert(in_messageDeserializerIsValid(_this));
    assert(type);
    assert(data);

    in_messageDeserializerReadPrim(_this, type->size, data);
}

void
in_messageDeserializer__readPrim(
    in_messageDeserializer _this,
    os_uint32 size,
    c_voidp data)
{
    c_octet* dstPtr = (c_octet*)data;
    c_octet* srcPtr;
    os_uint32 i;
    os_uint32 sizeAvail;
    os_uint32 remainingSize;

    assert(_this);
    assert(in_messageDeserializerIsValid(_this));
    assert(data);

    /* remove excess padding, if any or needed. The transformer may be renewed! */
    in_messageDeserializerRemoveCDRPaddingFromStream(_this, size);

    srcPtr = in_messageTransformerGetHead(_this);
    if (size <= in_messageTransformerGetAvailable(_this))
    {
        in_messageTransformerClaim(_this, size);
        for (i = 0; i < size; i++)
        {
            dstPtr[i] = srcPtr[i];
        }
    } else
    {
        sizeAvail = in_messageTransformerGetAvailable(_this);
        remainingSize = size - sizeAvail;

        in_messageTransformerClaim(_this, sizeAvail);
        for (i = 0; i < sizeAvail; i++)
        {
            dstPtr[i] = srcPtr[i];
        }

        in_messageTransformerRenew(_this);
        assert(in_messageTransformerGetAvailable(_this) >= remainingSize);

        srcPtr = in_messageTransformerGetHead(_this);
        dstPtr = (c_octet*)&dstPtr[sizeAvail];

        in_messageTransformerClaim(_this, remainingSize);
        for (i = 0; i < remainingSize; i++)
        {
            dstPtr[i] = srcPtr[i];
        }
    }
}

void
in_messageDeserializer__readPrimSwapped(
    in_messageDeserializer _this,
    os_uint32 dataSize,
    c_voidp data)
{
    c_octet *dst = (c_octet *)data;
    c_octet *srcPtr;
    os_uint32 i;
    os_uint32 size;
    os_uint32 remainingSize;
    os_uint32 max;

    assert(_this);
    assert(in_messageDeserializerIsValid(_this));
    assert(data);

    /* remove excess padding, if any or needed. The transformer may be renewed! */
    in_messageDeserializerRemoveCDRPaddingFromStream(_this, dataSize);

    srcPtr = in_messageTransformerGetHead(_this);
    if (dataSize <= in_messageTransformerGetAvailable(_this))
    {
        max = dataSize-1;

        in_messageTransformerClaim(_this, dataSize);
        for (i = 0; i < dataSize; i++)
        {
            dst[max-i] = srcPtr[i];
        }
    } else
    {
        size = in_messageTransformerGetAvailable(_this);
        remainingSize = dataSize - size;
        max = dataSize-1;
        in_messageTransformerClaim(_this, size);
        for (i = 0; i < size; i++)
        {
            dst[max-i] = srcPtr[i];
        }
        in_messageTransformerRenew(_this);
        assert(in_messageTransformerGetAvailable(_this) >= remainingSize);

        srcPtr = in_messageTransformerGetHead(_this);
        max = remainingSize-1;

        in_messageTransformerClaim(_this, remainingSize);
        for (i = 0; i < remainingSize; i++)
        {
            dst[max-i] = srcPtr[i];
        }
    }
}

/* TODO should propagate the result code, might have an out of memory error
 * if allocation of the array failed
 */
void
in_messageDeserializerReadArray(
    in_messageDeserializer _this,
    c_type type,
    c_voidp data)
{
    in_result result = IN_RESULT_OK;
    c_collectionType ctype = c_collectionType(type);
    os_uint32 size;
    os_uint32 length;
    os_uint32 i;
    c_voidp array = NULL;
    c_object o;
    c_collKind typeKind;
    c_metaKind subTypeKind;

    assert(_this);
    assert(in_messageDeserializerIsValid(_this));
    assert(type);
    assert(data);

    /* First determine the length of the array. For IDL array types the length
     * is known, but for sequences it is encoded ahead of the elements as an
     * unsigned long
     */
    typeKind = ctype->kind;

    if(typeKind == C_ARRAY)
    {
        array = data;
        length = ctype->maxSize;
        assert(array);
    } else
    {
        assert(typeKind == C_SEQUENCE);
        /* For a sequence the length is encoded as an unsigned long, which in
         * CDR is 4 octets(bytes) in size.
         */
        assert(sizeof(os_uint32) == 4);
        in_messageDeserializerReadPrim(_this, 4, &length);
        /* Verify that the length is equal to or smaller then the maxSize if
         * maxSize is bigger then 0
         */
        if(ctype->maxSize > 0)
        {
            array = data;
            assert((c_long)length <= ctype->maxSize);
            assert(array);
        } else
        {
            if(*(c_voidp *)data)
            {
                array = *(c_voidp *)data;
            } else
            {
                array = c_arrayNew(type, (c_long)length);
                if(length > 0 && !array)
                {
                    result = IN_RESULT_OUT_OF_MEMORY;
                }
                *(c_voidp *)data = array;
            }
        }
    }

    if (length > 0 && result == IN_RESULT_OK)
    {
        assert(array);
        subTypeKind = c_baseObjectKind(c_baseObject(ctype->subType));
        if((subTypeKind == M_PRIMITIVE) ||(subTypeKind == M_ENUMERATION))
        {
            o = array;
           /* in_messageDeserializerReadPrimArray(
                _this,
                ctype->subType->size,
                length,
                o);*/
        } else
        {
            if (c_typeIsRef(ctype->subType))
            {
                size = sizeof(c_voidp);
            } else
            {
                size = ctype->subType->size;
            }
            o = array;
            for (i = 0; i < length; i++)
            {
                in_messageDeserializerReadType(_this, ctype->subType, o);
                o = C_DISPLACE(o, size);
            }
        }
    }
}

void
in_messageDeserializer__readPrimArray(
    in_messageDeserializer _this,
    os_uint32 dataSize,
    os_uint32 length,
    c_voidp data)
{
    c_octet* dstPtr;
    c_octet* srcPtr;
    os_uint32 i;
    os_uint32 avail;
    os_uint32 rest;
    os_uint32 size;

    assert(_this);
    assert(in_messageDeserializerIsValid(_this));
    assert(data);

    rest = length * dataSize;
    dstPtr = (c_octet*)data;

    /* remove excess padding, if any or needed. The transformer may be renewed! */
    in_messageDeserializerRemoveCDRPaddingFromStream(_this, dataSize);

    while (rest)
    {
        srcPtr = in_messageTransformerGetHead(_this);
        avail = in_messageTransformerGetAvailable(_this);
        if (avail < rest)
        {
            size = avail;
            rest -= avail;
        } else
        {
            size = rest;
            rest = 0;
        }
        in_messageTransformerClaim(_this, size);
        for (i = 0; i < size; i++)
        {
            dstPtr[i] = srcPtr[i];
        }

        if (rest > 0)
        {
            /* Not all elements are received, so get a new buffer and
             * displace the destination pointer to the current first free
             * position.
             */
            in_messageTransformerRenew(_this);
            dstPtr = C_DISPLACE(dstPtr, size);
        }
    }
}

void
in_messageDeserializer__readPrimArraySwapped(
    in_messageDeserializer _this,
    os_uint32 dataSize,
    os_uint32 length,
    c_voidp data)
{
    c_octet* dstPtr;
    c_octet* srcPtr;
    os_uint32 i;
    os_uint32 j;
    os_uint32 n;
    os_uint32 rest;
    os_uint32 restElements;
    os_uint32 maxElements;
    os_uint32 max;

    assert(_this);
    assert(in_messageDeserializerIsValid(_this));
    assert(data);

    /* remove excess padding, if any or needed. The transformer may be renewed! */
    in_messageDeserializerRemoveCDRPaddingFromStream(_this, dataSize);

    max = dataSize-1;
    restElements = length;
    dstPtr = (c_octet *)data;

    while (restElements)
    {
        /* Determine the maximum number of elements
         * that can be copied at once.
         */
        maxElements = in_messageTransformerGetAvailable(_this) / dataSize;
        if (maxElements >= restElements)
        {
            n = restElements;
            restElements = 0;
        } else
        {
            n = maxElements;
            restElements -= maxElements;
        }
        /* Copy the elements.
         */
        srcPtr = in_messageTransformerGetHead(_this);
        in_messageTransformerClaim(_this, (n*dataSize));


        for (i = 0; i < n; ++i)
        {
            for (j = 0; j < dataSize; ++j) {
                dstPtr[max-j] = srcPtr[j];
            }
            dstPtr = C_DISPLACE(dstPtr, dataSize);
            srcPtr = C_DISPLACE(srcPtr, dataSize);
        }
    
        if (restElements > 0)
        {
            rest = in_messageTransformerGetAvailable(_this);
            if (rest)
            {
            	/* Following the philosophie: write as strict as possible 
            	 * and read with as much goodwill as possible, we deal 
            	 * with data not following the CDR fragmentation rules, 
            	 * seperating fragments at 8-octet boundaries. 
            	 * 
            	 * With correct 8-octet boundary seperation,
            	 * the following would not be necessary, 
            	 * except the renewing! */
            	
                /* In this case not all elements are copied yet.
                 * Because there was not enough buffer space available.
                 * However there is still some buffer space available
                 * less than the element size.
                 * So the next element is devided over two buffers.
                 */

                /* Copy the first part of the element from the rest of
                 * the current buffer.
                 * It is asserted that the srcPtr is displaced towards the
                 * current head position, so we do not need to get the head
                 * again, as the srcPtr is already on the correct location.
                 * It is also assumed the dstPrt has been placed at the first
                 * available spot to place data into.
                 */
                assert(srcPtr == in_messageTransformerGetHead(_this));
                in_messageTransformerClaim(_this, rest);
                for (j = 0; j < rest; ++j)
                {
                    dstPtr[max-j] = srcPtr[j];
                }
                
                /* Get the next buffer. */
                in_messageTransformerRenew(_this);

                /* Copy the last part of the element from the rest of
                 * the new buffer.
                 */
                srcPtr = in_messageTransformerGetHead(_this);
                in_messageTransformerClaim(_this, (dataSize-rest));

                for (j = rest; j < dataSize; j++)
                {
                    dstPtr[max-j] = srcPtr[j-rest];
                }
                restElements--;
            }
        }
    }
}

void
in_messageDeserializer__readCollection(
    in_messageDeserializer _this,
    c_type type,
    c_voidp data)
{
    assert(_this);
    assert(in_messageDeserializerIsValid(_this));
    assert(type);
    assert(data);

    in_messageDeserializerReadCollection(_this, type, data);
}

void
in_messageDeserializerReadString(
    in_messageDeserializer _this,
    c_type type,
    c_voidp data)
{
    in_result result = IN_RESULT_OK;
    c_string str = NULL;
    os_uint32 stringLength = 0;
    os_uint32 lenAvail;
    os_uint32 copyLength;
    os_char* stringStart;

    assert(_this);
    assert(in_messageDeserializerIsValid(_this));
    assert(type);
    assert(data);

    /* remove excess padding, if any or needed. The transformer may be renewed! */
    in_messageDeserializerRemoveCDRPaddingFromStream(_this, sizeof(os_uint32));

    /* first read the length of the string, i.e. the number of chars. This
     * info is stored in an unsigned long, which has size '4'
     */
    assert(sizeof(os_uint32) == 4);
    in_messageDeserializerReadPrim(_this, sizeof(os_uint32), &stringLength);
    if (stringLength > 0)
    {
        str = c_stringMalloc(c_getBase(type), stringLength);
        if(str == NULL)
        {
            result = IN_RESULT_OUT_OF_MEMORY;
        } else
        {
            while(stringLength != 0)
            {
                lenAvail = in_messageTransformerGetAvailable(_this);
                if (lenAvail == 0)
                {
                    in_messageTransformerRenew(_this);
                    lenAvail = in_messageTransformerGetAvailable(_this);
                }
                if(lenAvail >= stringLength)
                {
                    copyLength = stringLength;
                } else
                {
                    copyLength = lenAvail;
                }
                stringStart = (os_char *)in_messageTransformerGetHead(_this);
                memcpy(str, stringStart, copyLength);
                in_messageTransformerClaim(_this, copyLength);
                stringLength -= copyLength;
            } 
        }
    }
    *(c_string *)data = str;
}

void
in_messageDeserializerReadClass(
    in_messageDeserializer _this,
    c_type type,
    c_voidp data)
{
    c_class cls;
    in_messageDeserializerPropertyActionArg arg;

    assert(_this);
    assert(in_messageDeserializerIsValid(_this));
    assert(type);
    assert(data);

    cls = c_class(type);

    arg.object = data;
    arg.transformer = _this; /* no need to increase ref count, obviously */

    /* Walk over all properties of the class */
    c_metaWalk(
        c_metaObject(cls),
        (c_metaWalkAction)in_messageDeserializerReadPropertyAction,
        &arg);
}

os_boolean
in_messageDeserializerReadPropertyAction(
    c_object object,
    c_voidp arg)
{
    in_messageDeserializerPropertyActionArg* a = (in_messageDeserializerPropertyActionArg*)arg;
    c_property property;
    c_type type;
    c_voidp data;

    assert(object);
    assert(arg);
    assert(a->object);
    assert(a->transformer);
    assert(in_messageDeserializerIsValid(a->transformer));

    /* For now, we are interested in properties only */
    if (c_baseObjectKind(object) == M_ATTRIBUTE)
    {
        property = c_property(object);
        type = c_typeActualType(property->type);
        data = C_DISPLACE(a->object, (c_address)property->offset);
        in_messageDeserializerReadType(a->transformer, type, data);
    }
    return OS_TRUE;
}

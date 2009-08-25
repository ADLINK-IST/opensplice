/* OS abstraction layer includes */
#include "os_heap.h"

/* Database includes */
#include "c_base.h"

/* DDSi includes */
#include "in__messageSerializer.h"
#include "in__endianness.h"
#include "in_report.h"
#include "in_align.h"

/**
 * The following two function typedefs specify the function signature
 * of type/format specific methods used by this class to copy the data.
 * These methods will be cached in function pointer arrays in the
 * in_messageSerializer class. These function pointer arrays together with
 * macro's defined later on provide a mechanism to select the type specific
 * write method by indexing the array with the type kind meta information.
 */
typedef os_uint32
(*in_messageSerializerWriteTypeMethod)(
    in_messageSerializer _this,
    c_type type,
    c_voidp data);

typedef os_uint32
(*in_messageSerializerWriteCollectionMethod)(
    in_messageSerializer _this,
    c_type type,
    c_voidp data);

typedef struct in_messageSerializerPropertyActionArg_s
{
    in_messageSerializer serializer;
    c_object object;
    os_uint32 result;
} in_messageSerializerPropertyActionArg;

/**
 * The following struct is the implementation of the in_messageSerializer class.
 * The class contains 2 function pointer tables.
 * - The writeType function pointer table contains an entry for each meta
 *   object kind. Only the entries for types are initialized.
 *   The in_messageSerializerWriteType macro will use this table to select the type
 *   specific copy method by means of indexing the table with the meta object
 *   kind.
 * - The writeCollection function pointer table contains an entry for each meta
 *   collection kind. The in_messageSerializerWriteCollection macro will use this table
 *   to select the collection specific copy method by means of indexing the
 *   table with the meta collection kind.
 */
OS_STRUCT(in_messageSerializer)
{
    OS_EXTENDS(in_messageTransformer);
    in_messageSerializerWriteTypeMethod writeType[M_COUNT];
    in_messageSerializerWriteCollectionMethod writeCollection[C_COUNT];
    os_boolean fragmented;
};

static os_boolean
in_messageSerializerInit(
    in_messageSerializer _this,
    in_objectKind kind,
    in_objectDeinitFunc deinit,
    in_messageTransformerGetBufferFunc getBufferFunc,
    c_voidp getBufferFuncArg);

static void
in_messageSerializerDeinit(
    in_object _this);

static os_uint32
in_messageSerializerWriteStructure(
    in_messageSerializer _this,
    c_type type,
    c_voidp data);

static os_uint32
in_messageSerializerWriteUnion(
    in_messageSerializer _this,
    c_type _type,
    c_voidp data);

static os_uint32
in_messageSerializerWritePrimitive(
    in_messageSerializer _this,
    c_type type,
    c_voidp data);

static os_uint32
in_messageSerializerWritePrim(
    in_messageSerializer _this,
    os_uint32 dataSize,
    c_voidp data);

static os_uint32
in_messageSerializerWriteArray(
    in_messageSerializer _this,
    c_type type,
    c_voidp data);

static os_uint32
in_messageSerializerWritePrimArray(
    in_messageSerializer _this,
    os_uint32 dataSize,
    os_uint32 length,
    c_voidp data);

static os_uint32
in_messageSerializer__writeCollection(
    in_messageSerializer _this,
    c_type type,
    c_voidp data);

static os_uint32
in_messageSerializerWriteString(
    in_messageSerializer _this,
    c_type type,
    c_voidp data);

static os_boolean
in_messageSerializerWritePropertyAction(
    c_object object,
    c_voidp arg);

static os_uint32
in_messageSerializerWriteClass(
    in_messageSerializer _this,
    c_type type,
    c_voidp data);

/**
 * The following macro-method implements the functionality to write a type
 * value to the stream send buffer. This method will automatically select
 * the type specific copy algorithm by means of the metatdata specified
 * object kind (c_baseObjectKind).
 */
#define in_messageSerializerWriteType(serializer, type, object)                      \
        in_messageSerializer(serializer)->writeType[c_baseObjectKind(c_baseObject(type))](\
            serializer,                                                        \
            c_type(type),                                                      \
            object)

/**
 * The following macro-method implements the functionality to write a collection
 * type object to the stream send buffer. This method will automatically select
 * the type specific copy algorithm by means of the metatdata specified
 * collection kind (c_collectionTypeKind).
 */
#define in_messageSerializerWriteCollection(serializer, type, data)            \
        in_messageSerializer(serializer)->writeCollection[c_collectionTypeKind(type)]( \
            serializer,                                                        \
            type,                                                              \
            data)

/* The following macro adds any CDR required padding to the stream
 * and ensures the stream is set to the right position to write the next data
 * value. This may involve updating the stream head to the next buffer!
 * SIDE EFFECT: it will update the curCdrIndex parameter to it's new value!
 */
#define in_messageSerializerAddCDRPaddingToStream(_this, size, result)         \
    do {                                                                          \
        os_uint32 padding;                                                     \
                                                                               \
        padding = IN_ALIGN_UINT_PAD((_this->curCdrIndex),size);                                   \
        if(padding != 0)                                                       \
        {                                                                      \
            result += padding;                                                 \
            if(padding <= in_messageTransformerGetAvailable(_this))            \
            {                                                                  \
                in_messageTransformerClaim(_this, padding);                    \
            } else                                                             \
            {                                                                  \
                /* TODO if crossing buffer boundaries with padding it kinda    \
                 * makes sense to discard all remaining data from the current  \
                 * buffer and to start at zero with the next buffer            \
                 */                                                            \
                os_uint32 remPadding;                                          \
                                                                               \
                remPadding = padding;                                          \
                remPadding -= in_messageTransformerGetAvailable(_this);        \
                in_messageTransformerClaim(_this,                              \
                    in_messageTransformerGetAvailable(_this));                 \
                in_messageTransformerRenew(_this);                             \
                assert(in_messageTransformerGetAvailable(_this) >= remPadding);\
                in_messageTransformerClaim(_this, remPadding);                 \
            }                                                                  \
        }                                                                      \
    }  while(0)

in_messageSerializer
in_messageSerializerNew(
    in_messageTransformerGetBufferFunc getBufferFunc,
    c_voidp getBufferFuncArg)
{
    in_messageSerializer _this = NULL;

    _this = os_malloc(sizeof(OS_STRUCT(in_messageSerializer)));
    if(_this != NULL)
    {
        os_boolean success;

        success = in_messageSerializerInit(
            _this,
            IN_OBJECT_KIND_MESSAGE_SERIALIZER,
            in_messageSerializerDeinit,
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
in_messageSerializerInit(
    in_messageSerializer _this,
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
        IN_OBJECT_KIND_MESSAGE_SERIALIZER,
        in_messageSerializerDeinit,
        getBufferFunc,
        getBufferFuncArg);

    if(success)
    {
        _this->writeType[M_PRIMITIVE] = in_messageSerializerWritePrimitive;
        _this->writeType[M_ENUMERATION] = in_messageSerializerWritePrimitive;
        _this->writeType[M_STRUCTURE] = in_messageSerializerWriteStructure;
        _this->writeType[M_EXCEPTION] = in_messageSerializerWriteStructure;
        _this->writeType[M_CLASS] = in_messageSerializerWriteClass;
        _this->writeType[M_UNION] = in_messageSerializerWriteUnion;
        _this->writeType[M_COLLECTION] = in_messageSerializer__writeCollection;

        _this->writeCollection[C_STRING] = in_messageSerializerWriteString;
        _this->writeCollection[C_ARRAY] = in_messageSerializerWriteArray;
        _this->writeCollection[C_SEQUENCE] = in_messageSerializerWriteArray;
    }

    return success;
}

void
in_messageSerializerDeinit(
    in_object _this)
{
    assert(_this);
    assert(in_messageSerializerIsValid(_this));

    in_messageTransformerDeinit(_this);
}

void
in_messageSerializerBegin(
    in_messageSerializer _this,
    in_data buffer,
    os_uint32 length)
{
    in_messageTransformer transformer;

    assert(_this);
    assert(buffer);
    assert(in_messageSerializerIsValid(_this));

    if (length != IN_ALIGN_UINT_CEIL(length, 8)) {
    	IN_REPORT_ERROR_1(IN_SPOT, "write buffer length is expected to be multiple of 8 to "
    			                 "fit CDR fragmentation constraints (%ud)", length);
    }
    
    transformer = in_messageTransformer(_this);
    in_messageTransformerBegin(transformer);
    in_messageTransformerSetBuffer(transformer, &buffer);
    in_messageTransformerSetLength(transformer, length);
}

in_data
in_messageSerializerEnd(
    in_messageSerializer _this)
{
    in_messageTransformer transformer;
    in_data* buffer;

    assert(_this);
    assert(in_messageSerializerIsValid(_this));

    transformer = in_messageTransformer(_this);
    buffer = in_messageTransformerGetBuffer(transformer);

    return *buffer;
}

in_result
in_messageSerializerWrite(
    in_messageSerializer _this,
    v_message message,
    c_long topicDataOffset,
    os_uint32* size)
{
    c_type type;
    os_uint32 length;
    in_result result = IN_RESULT_OK;

    assert(_this);
    assert(in_messageSerializerIsValid(_this));

    type = c_getType(message);
    assert(type);

    _this->fragmented = OS_FALSE;

    length = in_messageSerializerWriteType(_this, type, (c_voidp)message);

    if(_this->fragmented)
    {
        result = IN_RESULT_ERROR;
        /* TODO report error */
    }

    *size = length;

    return result;
}

os_uint32
in_messageSerializerWriteStructure(
    in_messageSerializer _this,
    c_type type,
    c_voidp data)
{
    c_structure structure = c_structure(type);
    c_member member;
    c_type memberType;
    os_uint32 size;
    os_uint32 i;
    os_uint32 result = 0;
    c_voidp o;

    assert(_this);
    assert(in_messageSerializerIsValid(_this));
    assert(type);
    assert(data);

    size = c_arraySize(structure->members);
    for (i = 0; i < size; i++)
    {
        member = structure->members[i];
        o = C_DISPLACE(data, (c_address)member->offset);
        memberType = c_typeActualType(c_specifierType(member));
        result += in_messageSerializerWriteType(_this, memberType, o);
    }
    return result;
}

os_uint32
in_messageSerializerWriteUnion(
    in_messageSerializer _this,
    c_type _type,
    c_voidp data)
{
    c_union utype = c_union(_type);
    c_type type;
    c_type switchType;
    c_unionCase deflt;
    c_unionCase activeCase;
    c_unionCase currentCase;
    c_value switchValue;
    c_literal label;
    c_object o;
    os_uint32 length;
    os_uint32 i;
    os_uint32 j;
    os_uint32 result;
    os_uint32 n;

    assert(_this);
    assert(in_messageSerializerIsValid(_this));
    assert(type);
    assert(data);

    /* action for the switch */
    switchType = c_typeActualType(utype->switchType);
    result = in_messageSerializerWriteType(_this, switchType, data);

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
        if (c_type(utype)->alignment >= switchType->size)
        {
            length = c_type(utype)->alignment;
        } else
        {
            length = switchType->size;
        }
        o = C_DISPLACE(data, (c_address)length);
        type = c_typeActualType(c_specifierType(activeCase));
        result += in_messageSerializerWriteType(_this, type, o);
    }
    return result;
}

os_uint32
in_messageSerializerWritePrimitive(
    in_messageSerializer _this,
    c_type type,
    c_voidp data)
{
    assert(_this);
    assert(in_messageSerializerIsValid(_this));
    assert(type);
    assert(data);

    return in_messageSerializerWritePrim(_this, type->size, data);
}


/**
 * This method will write a primitive type to the network buffers based upon
 * the type size.
 * The size is sufficient information to determine how to copy or swap
 * the data and is faster than determining it on the primitive kind.
 * The method will return the number of bytes copied.
 */
os_uint32
in_messageSerializerWritePrim(
    in_messageSerializer _this,
    os_uint32 size,
    c_voidp data)
{
    c_octet* srcPtr = (c_octet*)data;
    c_octet* dstPtr;
    os_uint32 i;
    os_uint32 sizeAvail;
    os_uint32 remainingSize;
    os_uint32 result = 0;

    assert(_this);
    assert(in_messageSerializerIsValid(_this));
    assert(data);

    /* Add required padding, if any or needed. The stream may be renewed! */
    in_messageSerializerAddCDRPaddingToStream(in_messageTransformer(_this), size, result);

    dstPtr = in_messageTransformerGetHead(_this);
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

        dstPtr = in_messageTransformerGetHead(_this);
        srcPtr = (c_octet*)&srcPtr[sizeAvail];

        in_messageTransformerClaim(_this, remainingSize);
        for (i = 0; i < remainingSize; i++)
        {
            dstPtr[i] = srcPtr[i];
        }
    }
    result += size;
    return result;
}

os_uint32
in_messageSerializerWriteArray(
    in_messageSerializer _this,
    c_type type,
    c_voidp data)
{
    c_collectionType ctype = c_collectionType(type);
    os_uint32 size;
    os_uint32 length;
    os_uint32 i;
    os_uint32 result;
    c_voidp array = NULL;
    c_metaKind subTypeKind;

    assert(_this);
    assert(in_messageSerializerIsValid(_this));
    assert(type);
    assert(data);

	result = 0;

    if (ctype->maxSize > 0) {
        array = data;
        length = ctype->maxSize;
    } else {
        array = *(c_voidp *)data;
        length = c_arraySize(array);
    }
    if (ctype->kind == C_SEQUENCE)
    {
        /* For a sequence the length is encoded as an unsigned long, which in
         * CDR is 4 octets(bytes) in size.
         */
        assert(sizeof(os_uint32) == 4);
        result += in_messageSerializerWritePrim(_this, 4, &length);
    }
    if(length > 0)
    {
        assert(array);
        subTypeKind = c_baseObjectKind(c_baseObject(ctype->subType));
        if((subTypeKind == M_PRIMITIVE) ||(subTypeKind == M_ENUMERATION))
        {
            result += in_messageSerializerWritePrimArray(
                _this,
                ctype->subType->size,
                length,
                array);
        } else
        {
            if (c_typeIsRef(ctype->subType))
            {
                size = sizeof(c_voidp);
            } else
            {
                size = ctype->subType->size;
            }
            for (i = 0; i < length; i++)
            {
                result += in_messageSerializerWriteType(_this, ctype->subType, array);
                array = C_DISPLACE(array, size);
            }
        }
    }
    return result;
}

os_uint32
in_messageSerializerWritePrimArray(
    in_messageSerializer _this,
    os_uint32 dataSize,
    os_uint32 length,
    c_voidp data)
{
    c_octet* dstPtr;
    c_octet* srcPtr = (c_octet *)data;
    os_uint32 i;
    os_uint32 rest;
    os_uint32 size = 4;
    os_uint32 avail;
    os_uint32 result = 0;

    assert(_this);
    assert(in_messageSerializerIsValid(_this));
    assert(data);

    rest = length * dataSize;

    /* Add required padding, if any or needed. The stream may be renewed! */
    in_messageSerializerAddCDRPaddingToStream(in_messageTransformer(_this), size, result);

    while (rest)
    {
        dstPtr = in_messageTransformerGetHead(_this);
        avail = in_messageTransformerGetAvailable(_this);
        if (rest <= avail)
        {
            size = rest;
            rest = 0;
        } else
        {
            size = avail;
            rest -= avail;
        }
        in_messageTransformerClaim(_this, size);
        for (i = 0; i < size; i++)
        {
            dstPtr[i] = srcPtr[i];
        }
        if (rest)
        {
            in_messageTransformerRenew(_this);
            srcPtr = C_DISPLACE(srcPtr, size);
        }
    }
    result += dataSize;
    return result;
}

os_uint32
in_messageSerializer__writeCollection(
    in_messageSerializer _this,
    c_type type,
    c_voidp data)
{
    assert(_this);
    assert(in_messageSerializerIsValid(_this));
    assert(type);
    assert(data);

    return in_messageSerializerWriteCollection(_this, type, data);
}

os_uint32
in_messageSerializerWriteString(
    in_messageSerializer _this,
    c_type type,
    c_voidp data)
{
    os_char *ptr = *(c_string *)data;
    in_data dst = NULL;
    os_uint32 size = 0;
    os_uint32 len = 0;
    os_uint32 avail = 0;
    os_uint32 result = 0;

    assert(_this);
    assert(in_messageSerializerIsValid(_this));
    assert(type);
    assert(data);

    if(ptr != NULL)
    {
        len = strlen(ptr) + 1;
        result = len;
    }

    assert(sizeof(os_uint32) == 4);
    /* Add required padding, if any or needed. The stream may be renewed! */
    in_messageSerializerAddCDRPaddingToStream(
        in_messageTransformer(_this),
        sizeof(os_uint32),
        result);

    result += in_messageSerializerWritePrim(_this, sizeof(os_uint32), &len);

    while (len)
    {
        avail = in_messageTransformerGetAvailable(_this);
        if (avail == 0)
        {
            in_messageTransformerRenew(_this);
            avail = in_messageTransformerGetAvailable(_this);
        }
        dst = in_messageTransformerGetHead(_this);
        size = (len < avail ? len : avail);
        in_messageTransformerClaim(_this, size);
        memcpy(dst, ptr, size);
        ptr = C_DISPLACE(ptr, size);
        len -= size;
    }
    return result;
}

os_uint32
in_messageSerializerWriteClass(
    in_messageSerializer _this,
    c_type type,
    c_voidp data)
{
    in_messageSerializerPropertyActionArg arg;
    c_class cls;
    os_uint32 result;

    assert(_this);
    assert(in_messageSerializerIsValid(_this));
    assert(type);
    assert(data);

    arg.object = data;
    arg.serializer = _this;
    arg.result = 0;
    cls = c_class(type);

    c_metaWalk(
        c_metaObject(cls),
        (c_metaWalkAction)in_messageSerializerWritePropertyAction,
        &arg);

    result = arg.result;

    return result;
}

os_boolean
in_messageSerializerWritePropertyAction(
    c_object object,
    c_voidp arg)
{
    in_messageSerializerPropertyActionArg* a = (in_messageSerializerPropertyActionArg*)arg;
    c_property property;
    c_type type;
    c_object o;

    assert(object);
    assert(arg);

    if (c_baseObjectKind(object) == M_ATTRIBUTE)
    {
        property = c_property(object);
        type = c_typeActualType(property->type);
        o = C_DISPLACE(a->object, (c_address)property->offset);
        a->result += in_messageSerializerWriteType(a->serializer, type, o);
    }
    return OS_TRUE;
}

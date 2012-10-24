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
/* Interface */
#include "c_serialize.h"

/* implementation */
#include "os_abstract.h" /* big or little endianness */
#include "os_heap.h"
#include "c__base.h"
#include "c__scope.h"
#include "c_typebase.h"
#include "c_metabase.h"
#include "c_collection.h"


/* --------------------------- deepwalk ------------------------ */

typedef void
c_deepwalkFunc(
    c_type type,
    c_object *objectPtr,
    c_voidp actionArg);

static void
c_deepwalkType(
    c_type type,
    c_object *objectPtr,
    c_deepwalkFunc action,
    c_voidp actionArg);


/** \brief Routine for handling union types
 *
 *  This function descends into the general \b deepwalkType function
 *  passing the metadata of the switchType. Furthermore, it will
 *  determine the active case label from the data and again
 *  descend into \b deepwalkType using both the metadata of the type of
 *  the active label and the address of the union data as parameters.
 *
 *  \param v_union   Metadata information for the union type.
 *  \param objectPtr Pointer to the object insTance of the union
 *                   type.
 *  \param action    The callback to be executed
 *  \param actionArg The user-defined pointer to be passed to the
 *                   action function.
 */

static void
c_deepwalkUnion(
    c_union v_union,
    c_object *objectPtr,
    c_deepwalkFunc action,
    void *actionArg)
{
    c_type switchType;
    c_type caseType;
    c_unionCase deflt;
    c_unionCase activeCase;
    c_unionCase currentCase;
    c_object unionData;
    c_value switchValue;
    c_literal label;
    int i,j, nCases, nLabels;

    c_long dataOffset;

    /* action for the union itself */
    /* No action, but separate actions for the switch and the data */
    /* action(c_type(v_union), objectPtr, actionArg); */
    /* action for the switch */
    c_deepwalkType(v_union->switchType, objectPtr,
                   action, actionArg);

    switchType = c_typeActualType(v_union->switchType);
    /* Determine value of the switch field */
    switch (c_baseObject(switchType)->kind) {
    case M_PRIMITIVE:
        switch (c_primitive(switchType)->kind) {
#define __CASE__(prim, type) \
        case prim: switchValue = type##Value(*((type *)*objectPtr)); break;
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
        switchValue = c_longValue(*(c_long *)*objectPtr);
    break;
    default:
        switchValue = c_undefinedValue();
        assert(FALSE);
    break;
    }

    /* Determine the label corresponding to this field */

    activeCase = NULL;
    deflt = NULL;
    nCases = c_arraySize(v_union->cases);

    for (i=0; (i<nCases) && !activeCase; i++) {
        currentCase = c_unionCase(v_union->cases[i]);
        nLabels = c_arraySize(currentCase->labels);
        if (nLabels > 0) {
            for (j=0; (j<nLabels) && !activeCase; j++) {
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
    assert(activeCase);
    if (activeCase) {
        caseType = c_specifier(activeCase)->type;
        if (c_type(v_union)->alignment >= v_union->switchType->size) {
            dataOffset = c_type(v_union)->alignment;
        } else {
            dataOffset = v_union->switchType->size;
        }
        unionData = C_DISPLACE(*objectPtr, (c_address)dataOffset);
        c_deepwalkType(caseType, &unionData, action, actionArg);
    }
}


/** \brief Helper structure for deepwalkInterface */
typedef struct c_interfaceContext{
    c_object *objectPtr;     /**< address of the current interface object */
    c_deepwalkFunc *action;  /**< interface attribute callback function. */
    void *actionArg;         /**< callback function argument. */
} *c_interfaceContext;


/** \brief Helper callback function for deepwalkInterface.
 *         This function handles one attribute of an interface.
 */

static c_bool
c_deepwalkProperty(
    c_metaObject object,
    c_metaWalkActionArg actionArg)
{
    c_interfaceContext context = (c_interfaceContext)actionArg;
    c_property property;
    c_type propertyType;
    c_object propertyData;

    /* For now, we are interested in properties only */
    if (c_baseObject(object)->kind == M_ATTRIBUTE) {
        property = c_property(object);
        propertyType = c_typeActualType(property->type);
        /* For now, skip attributes which are class-references */
        propertyData = C_DISPLACE(*context->objectPtr,
                                  (c_address)property->offset);
        c_deepwalkType(propertyType, &propertyData,
                       context->action, context->actionArg);
    }

    return TRUE;
}


/** \brief Routine for the walking over interface attributes */
static void
c_deepwalkInterface(
    c_interface interf,
    c_object *objectPtr,
    c_deepwalkFunc action,
    void *actionArg)
{
    struct c_interfaceContext context;
    c_object *inst;

    /* members are stored in scope (unordered), so use scopeWalk */
    inst = (c_object *)(*objectPtr);
    /* Only walk over the properties if the reference is valid. */
    if (inst && *inst) {
        context.objectPtr = inst;
        context.action = action;
        context.actionArg = actionArg;
        c_metaWalk(c_metaObject(interf),
                   (c_metaWalkAction)c_deepwalkProperty, &context);
    }
}

/** \brief Routine for the walking over baseclasses of a class and over
 *         the (interface)attributes
 */
static void
c_deepwalkClass(
    c_class class,
    c_object *objectPtr,
    c_deepwalkFunc action,
    void *actionArg)
{
    if (class->extends) {
        c_deepwalkClass(class->extends, objectPtr, action, actionArg);
    }
    c_deepwalkInterface(c_interface(class), objectPtr, action, actionArg);
}

/** \brief Helper structure for deepwalkComplexCollectionElements */
typedef struct c_collectionContext{
    c_type type;              /**< metadata of the collection's subType. */
    c_deepwalkFunc *action;   /**< collection element callback function. */
    void *actionArg;          /**< callback function argument */
} *c_collectionContext;


/** \brief Helper callback function for \b deepwalkComplexCollectionElements.
 *         This function handles one element of a complect collection
 *         (e.g. \b SET).
 */

static c_bool
c_deepwalkCollectionAction(
    c_object object,
    c_voidp actionArg)
{
    c_collectionContext context = (c_collectionContext)actionArg;
    c_object placeHolder;

    if (!(int)c_typeIsRef(context->type)) {
        placeHolder = object;
    } else {
        placeHolder = (c_object)&object;
    }

    c_deepwalkType(context->type, &placeHolder,
                   context->action, context->actionArg);

    return TRUE;
}

#if 0
/** \brief Routine for walking over all collection elements.
 *
 *  This routine handles elements of scopes which are metaObjects.
 *  Such scopes can be walked over by the
 *  \b c_scopeWalk function. During the walk, all meta objects will be
 *  passed to \b deepwalkType with their metadata and address as
 *  parameters. This is done by the function
 *  \b deepwalkComplexCollectionCallback.
 *
 *  \param collection Metadata information for the scope type.
 *  \param scope     The scope insTance.
 *                   type.
 *  \param action    The callback to be executed
 *  \param actionArg The user-defined pointer to be passed to the
 *                   action function.
 */

static void
c_deepwalkMetaScopeElements(
    c_collectionType collectionType,
    c_object *objectPtr,
    c_deepwalkFunc action,
    void *actionArg)
{
    struct c_collectionContext context;
    c_scope scopeInst;

    assert(collectionType->kind == C_SCOPE);

    scopeInst = *((c_scope *)(*objectPtr));
    if (scopeInst) {
        /* Walk over the elements */
        context.type = collectionType->subType;
        context.action = action;
        context.actionArg = actionArg;
        c_scopeWalk(scopeInst,
                    (c_scopeWalkAction)c_deepwalkCollection,
                    &context);
    }
}
#endif

/** \brief Function for handling collection types.
 *
 *  For collections, first the action routine is called with the
 *  metadata information collection itself and the address as
 *  parameters. This is sufficient for strings, but arrays and sets
 *  need some more actions in order to walk over their elements. This
 *  is delegated to \b deepwalkArrayElements and
 *  \b deepwalkComplexCollectionElements.
 *
 *  \param collectionType Metadata information for the collection type.
 *  \param objectPtr Pointer to the object insTance of the collection
 *                   type.
 *  \param action    The callback to be executed
 *  \param actionArg The user-defined pointer to be passed to the
 *                   action function.
 */

static void
c_deepwalkCollection(
    c_collectionType collectionType,
    c_object *objectPtr,
    c_deepwalkFunc action,
    void *actionArg)
{
    c_bool isBunchOfBytes;
    c_long size, i;
    c_object baseObject;
    c_object currentObject;


    /* Action for the array type itself */
    action(c_type(collectionType), objectPtr, actionArg);
    /* For complex types, walk over all elements */
    switch (collectionType->kind) {
    case C_ARRAY:
    case C_SEQUENCE:
        /* An optimization for arrays or sequences of bytes */
        isBunchOfBytes = FALSE;
        if (c_baseObject(collectionType->subType)->kind == M_PRIMITIVE) {
            if ((c_primitive(collectionType->subType)->kind == P_OCTET) ||
                (c_primitive(collectionType->subType)->kind == P_CHAR)) {
                isBunchOfBytes = TRUE;
            }
        }
        if (!isBunchOfBytes) {
            /* Walk over all entries */
            switch (collectionType->kind) {
            case C_ARRAY:
                if (collectionType->maxSize == 0) {
                    size = c_arraySize(*((c_array *)(*objectPtr)));
                    baseObject = *((c_object *)(*objectPtr));
                } else {
                    size = collectionType->maxSize;
                    baseObject = *objectPtr;
                }
            break;
            case C_SEQUENCE:
                size = c_arraySize(*((c_array *)(*objectPtr)));
                baseObject = *((c_object *)(*objectPtr));
            break;
            default:
                size = 0;
                baseObject = NULL;
                assert(FALSE);
            break;
            }

            currentObject = baseObject;
            for (i=0; i<size; i++) {
                c_deepwalkType(collectionType->subType,
                               &currentObject,
                               action,
                               actionArg);
                if (c_typeIsRef(collectionType->subType)) {
                    currentObject = C_DISPLACE(currentObject,
                                               sizeof(c_voidp));
                } else {
                    currentObject = C_DISPLACE(currentObject,
                                               collectionType->subType->size);
                }
            }
        }
    break;
    case C_STRING:
        ; /* No action here, chars are not treated separately */
    break;
    case C_SET:
    case C_LIST:
    case C_BAG:
    case C_DICTIONARY:
    case C_QUERY:
    {
        struct c_collectionContext context;
        c_collection collectionInst;

        collectionInst = *((c_collection *)(*objectPtr));
        if (collectionInst) {
            /* Walk over the elements */
            context.type = collectionType->subType;
            context.action = action;
            context.actionArg = actionArg;
            c_walk(collectionInst,
                   (c_action)c_deepwalkCollectionAction,
                   &context);
        }
    }
    break;
#if 0
    case C_SCOPE:
        c_deepwalkMetaScopeElements(collectionType, objectPtr,
                                     action, actionArg);
    break;
#endif
    default:
        assert(FALSE);
    break;
    }
}


/** \brief Routine for walking over any type
 *
 *  This routine determines which handling function to call by
 *  selecting on the metadata kind. It is the general entry
 *  point of \b deepwalk and is recursively called by most handling
 *  functions.
 *
 *  \param type      Metadata information for the type.
 *  \param objectPtr Pointer to the object insTance of the given type.
 *  \param action    The callback to be executed
 *  \param actionArg The user-defined pointer to be passed to the
 *                   action function.
 */

static void
c_deepwalkType(
    c_type type,
    c_object *objectPtr,
    c_deepwalkFunc action,
    void *actionArg)
{
    c_type actualType;
    c_object object;
    c_member member;
    c_long size, i;

    actualType = c_typeActualType(type);

    /* Determine which action to take */
    switch (c_baseObject(actualType)->kind) {
    case M_PRIMITIVE:
    case M_ENUMERATION:
        action(actualType, objectPtr, actionArg);
    break;
    case M_STRUCTURE:
    case M_EXCEPTION:
        size = c_arraySize(c_structure(actualType)->members);
        for (i=0; i<size; i++) {
            member = c_structure(actualType)->members[i];
            object = C_DISPLACE(*objectPtr, (c_address)member->offset);
            c_deepwalkType(c_specifier(member)->type, &object,
                           action, actionArg);
        }
    break;
    case M_COLLECTION:
        c_deepwalkCollection(c_collectionType(actualType), objectPtr,
                             action, actionArg);
    break;
    case M_CLASS:
        action(actualType, objectPtr, actionArg);
        c_deepwalkClass(c_class(actualType), objectPtr,
                        action, actionArg);
    break;
#ifndef NDEBUG
    case M_INTERFACE:
        action(actualType, objectPtr, actionArg);
        c_deepwalkInterface(c_interface(actualType), objectPtr,
                            action, actionArg);
    break;
#endif
    case M_UNION:
        c_deepwalkUnion(c_union(actualType), objectPtr,
                        action, actionArg);
    break;
    default:
        assert(FALSE); /* Only descendants of type possible */
    break;
    }
}


/** \brief Routine for recursively walking over all data-containing
 *         elements of a database object.
 *
 *  A general mechanism for walking over the complete structure of an
 *  object can be reused by several kinds of serializers. The \b deepwalk
 *  function offers such a mechanism. By passing an object and a
 *  userCallback, the user will be capable of executing his own actions
 *  for each data element contained by the object.
 *
 *  The routine itself will determine the metadata of the object and use
 *  this for walking over its members, attributes or elements.
 *
 *  \param type      Metadata information for the object
 *  \param objectPtr Pointer to the object insTance to walk over.
 *  \param action    The callback to be executed
 *  \param actionArg The user-defined pointer to be passed to the
 *                   action function.
 */

static void
c_deepwalk(
    c_type type,
    c_object *objectPtr,
    c_deepwalkFunc action,
    void *actionArg)
{
    c_object *placeHolder;

    assert(objectPtr);

    if (c_typeIsRef(type)) {
        placeHolder = (c_object *)&objectPtr;
    } else {
        placeHolder = objectPtr;
    }

    c_deepwalkType(type, placeHolder, action, actionArg);
}


/* -------------------------- action argument ------------------ */

/* The action parameter used by the serialization routines */
typedef struct c_serializeContext_s {
    c_octet **dataPtrPtr;
    c_ulong *lengthPtr;
    c_serializeGetBufferAction action;
    c_serializeActionArg actionArg;
} *c_serializeContext;


/* ----------------- copying/swapping for serialization --------------------- */
#ifdef PA_BIG_ENDIAN
static void
c_copyDataSer(
    c_ulong dataSize,
    const c_octet *src,
    c_serializeContext context)
{
    c_octet *srcPtr;
    c_octet *dstPtr;
    c_ulong i,size,remainingSize;

    dstPtr = *context->dataPtrPtr;
    if (dataSize <= *context->lengthPtr) {
        for (i=0; i<dataSize; i++) {
            dstPtr[i] = src[i];
        }
        *context->dataPtrPtr = C_DISPLACE(*context->dataPtrPtr,dataSize);
        *context->lengthPtr -= dataSize;
    } else {
        size = *context->lengthPtr;
        remainingSize = dataSize - size;
        for (i=0; i<size; i++) {
            dstPtr[i] = src[i];
        }
        *context->dataPtrPtr = C_DISPLACE(*context->dataPtrPtr,size);
        *context->lengthPtr = 0;
        context->action(context->dataPtrPtr,
                        context->lengthPtr,
                        context->actionArg);
        assert(*context->lengthPtr > remainingSize);
        dstPtr = *context->dataPtrPtr;
        srcPtr = (c_octet *)&src[size];
        size = remainingSize;
        for (i=0; i<size; i++) {
            dstPtr[i] = srcPtr[i];
        }
        *context->dataPtrPtr = C_DISPLACE(*context->dataPtrPtr,size);
        *context->lengthPtr -= remainingSize;
    }
}
#endif

#ifdef PA_LITTLE_ENDIAN
static void
c_copyDataSwappedSer(
    c_ulong dataSize,
    const c_octet *src,
    c_serializeContext context)
{
    c_octet *dstPtr;
    c_ulong i,size,max,remainingSize;

    dstPtr = *context->dataPtrPtr;
    if (dataSize <= *context->lengthPtr) {
        max = dataSize-1;
        for (i=0; i<dataSize; i++) {
            dstPtr[i] = src[max-i];
        }
        *context->dataPtrPtr = C_DISPLACE(*context->dataPtrPtr,dataSize);
        *context->lengthPtr -= dataSize;
    } else {
        size = *context->lengthPtr;
        remainingSize = dataSize - size;
        max = dataSize-1;
        for (i=0; i<size; i++) {
            dstPtr[i] = src[max-i];
        }
        *context->dataPtrPtr = C_DISPLACE(*context->dataPtrPtr,size);
        *context->lengthPtr = 0;
        context->action(context->dataPtrPtr,
                        context->lengthPtr,
                        context->actionArg);
        assert(*context->lengthPtr > remainingSize);
        dstPtr = *context->dataPtrPtr;
        size = remainingSize;
        max = size-1;
        for (i=0; i<size; i++) {
            dstPtr[i] = src[max-i];
        }
        *context->dataPtrPtr = C_DISPLACE(*context->dataPtrPtr,size);
        *context->lengthPtr -= remainingSize;
    }
}
#endif

static void
c_copyDataBunchOfBytesSer(
    c_ulong dataSize,
    const c_octet *src,
    c_serializeContext context)
{
    c_octet *dstPtr;
    const c_octet *srcPtr;
    c_ulong remainingSize;
    c_ulong copySize;

    srcPtr = src;
    remainingSize = dataSize;
    while (remainingSize > 0) {
        dstPtr = *context->dataPtrPtr;
        if (remainingSize <= *context->lengthPtr) {
            copySize = remainingSize;
        } else {
            copySize = *context->lengthPtr;
        }
        memcpy(dstPtr, srcPtr, copySize);
        *context->dataPtrPtr = &(dstPtr[copySize]);
        *context->lengthPtr -= copySize;
        remainingSize -= copySize;

        if (remainingSize > 0) {
            /* This can only happen if we did not have enough space */
            assert(*context->lengthPtr == 0);
            /* Get new buffer, we have not finished yet */
            context->action(context->dataPtrPtr,
                            context->lengthPtr,
                            context->actionArg);
            /* Move the srcPtr to the next bunch of bytes */
            srcPtr = &(srcPtr[copySize]);
        }
    }
}

/* ------------------------------- Conversion to endianness ----------------------- */


#ifdef PA_BIG_ENDIAN

#define C_SERIALIZE_N(context,src,size) \
        c_copyDataSer(size,src,context)

#endif /* PA_BIG_ENDIAN */

#ifdef PA_LITTLE_ENDIAN

#define C_SERIALIZE_N(context,src,size) \
        c_copyDataSwappedSer(size,src,context)

#endif /* PA_LITTLE_ENDIAN */


/** \brief Function for determining if an object is a valid reference.
 *         If valid TRUE is returned otherwise FALSE*/
#define c_isValidReference(object) \
        ((object != NULL) && (*(c_object *)object != NULL))

/* -------------------------- serialization -------------------- */

#define C_FORMAT_ID      (0x5332U)    /* "S2" */
#define C_FORMAT_VERSION (0x0001U)

/** \brief Convenience function for copying a zero-terminated string */
static void
c_bigESerString(
    const c_string src,
    c_serializeContext context)
{
    const c_char *currentString = src;
    c_bool done;

    assert(context->dataPtrPtr != NULL);
    assert(*context->dataPtrPtr != NULL);
    assert(src != NULL);
    assert(context->lengthPtr != NULL);
    assert(context->action != NULL);

    do {
        if (*context->lengthPtr == 0) {
            context->action(context->dataPtrPtr, context->lengthPtr,
                context->actionArg);
            assert(*context->lengthPtr != 0);
        }
        done = (*currentString == '\0');
        **context->dataPtrPtr = *currentString;
        *context->dataPtrPtr = &((*context->dataPtrPtr)[1]);
        (*context->lengthPtr)--;
        currentString = &(currentString[1]);
    } while (!done);
}

/** \brief Function for serializing any collection type.
 *         Currently supported types are string, array, sequence and set.
 */
static void
c_bigESerCollection(
    c_collectionType collectionType,
    c_object object,
    c_serializeContext context)
{
    c_long colSize;
    c_bool isBunchOfBytes;
    c_bool isValidRef;

    assert(context->dataPtrPtr != NULL);
    assert(*context->dataPtrPtr != NULL);
    assert(object != NULL);
    assert(context->lengthPtr != NULL);
    assert(context->action != NULL);

    /* An optimization for arrays or sequences of bytes */
    isBunchOfBytes = FALSE;
    if (c_baseObject(collectionType->subType)->kind == M_PRIMITIVE) {
        if ((c_primitive(collectionType->subType)->kind == P_OCTET) ||
            (c_primitive(collectionType->subType)->kind == P_CHAR)) {
            isBunchOfBytes = TRUE;
        }
    }

    /* First serialize the size of the array, if needed */
    /* Different behaviour for reftypes and non-reftypes */
    if (((collectionType->kind == C_ARRAY) ||
         (collectionType->kind == C_SEQUENCE)) &&
         !(int)c_typeIsRef(c_type(collectionType))) {

        /* Nothing to do for fixed size arrays and sequences */
        /* Only do something in case of optimized serialization */
        if (isBunchOfBytes) {
            colSize = collectionType->maxSize;
            assert(colSize > 0);
            c_copyDataBunchOfBytesSer(colSize, ((c_octet *)object), context);
        }

    } else {

        /* first serialize the validity of the object. We have to do this in
         * all situations, whether the reference is valid or not */
        isValidRef = c_isValidReference(object);
        C_SERIALIZE_N(context, (c_octet *)(&isValidRef), 1);

        /* Now check if this was a valid reference. In that case, we have to
         * handle the data itself as well */
        if (c_isValidReference(object)) {

            /* Only serialize the collection size in case of list/set/bag/etc.
             */
            switch (collectionType->kind) {
            case C_STRING:
                c_bigESerString(*((c_string *)(object)), context);
            break;
            case C_ARRAY:
            case C_SEQUENCE:
                assert(c_typeIsRef(c_type(collectionType)));
                /* Serialize the size */
                colSize = c_arraySize(*((c_array *)(object)));
                C_SERIALIZE_N(context, (c_octet *)(&colSize), sizeof(c_ulong));
                /* Only do something in case of optimized serialization */
                if (isBunchOfBytes) {
                    assert(colSize > 0);
                    c_copyDataBunchOfBytesSer(colSize,
                                              *((c_octet **)object),
                                              context);
                }
            break;
            case C_SET:
                /* Serialize the size */
                colSize = c_setCount(*(c_collection *)object);
                C_SERIALIZE_N(context, (c_octet *)(&colSize), sizeof(c_ulong));
            break;
            case C_LIST:
            case C_BAG:
            case C_DICTIONARY:
            case C_QUERY:
            default:
                assert(FALSE); /* No other collection types supported */
            break;
            }
        }
    }
}

/** \brief Callback function for deepwalk when serializing */
static void
c_bigESerCallback(
    c_type type,
    c_object *objectPtr,
    void *actionArg)
{
    c_bool isValidRef;
    c_object object = *objectPtr;
    c_serializeContext context = (c_serializeContext)actionArg;

    assert(context->dataPtrPtr != NULL);
    assert(*context->dataPtrPtr != NULL);
    assert(object != NULL);
    assert(context->lengthPtr != NULL);
    assert(context->action != NULL);

    switch (c_baseObject(type)->kind) {
    case M_COLLECTION:
        c_bigESerCollection(c_collectionType(type), object, context);
    break;
    case M_PRIMITIVE:
    case M_ENUMERATION:
        C_SERIALIZE_N(context, object, type->size);
    break;
    case M_INTERFACE:
    case M_CLASS:
        isValidRef = c_isValidReference(object);
        C_SERIALIZE_N(context, (c_octet *)(&isValidRef), 1);
    break;
    default:
        assert(FALSE); /* No other expected than these */
    break;
    }
}

void
c_serialize(
    c_object object,
    c_octet **bufferPtr,
    c_ulong *lengthPtr,
    c_serializeGetBufferAction action,
    c_serializeActionArg arg)
{
    c_type type;
    struct c_serializeContext_s contextStruct;

    contextStruct.dataPtrPtr = bufferPtr;
    contextStruct.lengthPtr = lengthPtr;
    contextStruct.action = action;
    contextStruct.actionArg = arg;

    type = c__getType(object);
    c_deepwalk(type, &object, c_bigESerCallback, &contextStruct);
}

/* -------------------------- deserialization ------------------------------- */

#ifdef PA_BIG_ENDIAN
static void
c_copyDataDeser(
    c_ulong dataSize,
    c_octet *dst,
    c_serializeContext context)
{
    c_octet *dstPtr;
    c_octet *srcPtr;
    c_ulong i,size,remainingSize;

    srcPtr = *context->dataPtrPtr;
    if (dataSize <= *context->lengthPtr) {
        for (i=0; i<dataSize; i++) {
            dst[i] = srcPtr[i];
        }
        *context->dataPtrPtr = C_DISPLACE(*context->dataPtrPtr,dataSize);
        *context->lengthPtr -= dataSize;
    } else {
        size = *context->lengthPtr;
        remainingSize = dataSize - size;
        for (i=0; i<size; i++) {
            dst[i] = srcPtr[i];
        }
        *context->dataPtrPtr = C_DISPLACE(*context->dataPtrPtr,size);
        *context->lengthPtr = 0;
        context->action(context->dataPtrPtr,
                        context->lengthPtr,
                        context->actionArg);
        assert(*context->lengthPtr >= remainingSize);
        srcPtr = *context->dataPtrPtr;
        dstPtr = &dst[size];
        size = remainingSize;
        for (i=0; i<size; i++) {
            dstPtr[i] = srcPtr[i];
        }
        *context->dataPtrPtr = C_DISPLACE(*context->dataPtrPtr,size);
        *context->lengthPtr -= size;
    }
}
#endif

#ifdef PA_LITTLE_ENDIAN
static void
c_copyDataSwappedDeser(
    c_ulong dataSize,
    c_octet *dst,
    c_serializeContext context)
{
    c_octet *srcPtr = *context->dataPtrPtr;
    c_ulong i,size,remainingSize,max;

    srcPtr = *context->dataPtrPtr;
    if (dataSize <= *context->lengthPtr) {
        max = dataSize-1;
        for (i=0; i<dataSize; i++) {
            dst[max-i] = srcPtr[i];
        }
        *context->dataPtrPtr = C_DISPLACE(*context->dataPtrPtr,dataSize);
        *context->lengthPtr -= dataSize;
    } else {
        size = *context->lengthPtr;
        remainingSize = dataSize - size;
        max = dataSize-1;
        for (i=0; i<size; i++) {
            dst[max-i] = srcPtr[i];
        }
        *context->dataPtrPtr = C_DISPLACE(*context->dataPtrPtr,size);
        *context->lengthPtr = 0;
        context->action(context->dataPtrPtr,
                        context->lengthPtr,
                        context->actionArg);
        assert(*context->lengthPtr >= remainingSize);
        srcPtr = *context->dataPtrPtr;
        size = remainingSize;
        max = size-1;
        for (i=0; i<size; i++) {
            dst[max-i] = srcPtr[i];
        }
        *context->dataPtrPtr = C_DISPLACE(*context->dataPtrPtr,size);
        *context->lengthPtr -= size;
    }
}
#endif

static void
c_copyDataBunchOfBytesDeser(
    c_ulong dataSize,
    c_octet *dst,
    c_serializeContext context)
{
    c_octet *dstPtr;
    c_octet *srcPtr;
    c_ulong remainingSize;
    c_ulong copySize;

    dstPtr = dst;
    remainingSize = dataSize;
    while (remainingSize > 0) {
        srcPtr = *context->dataPtrPtr;
        if (remainingSize <= *context->lengthPtr) {
            copySize = remainingSize;
        } else {
            copySize = *context->lengthPtr;
        }
        memcpy(dstPtr, srcPtr, copySize);
        *context->dataPtrPtr = &(srcPtr[copySize]);
        *context->lengthPtr -= copySize;
        remainingSize -= copySize;
        if (remainingSize > 0) {
            /* This can only happen if we did not have enough space */
            assert(*context->lengthPtr == 0);
            /* Get new buffer, we have not finished yet */
            context->action(context->dataPtrPtr,
                            context->lengthPtr,
                            context->actionArg);
            /* Shift the destination pointer to the next bunch */
            dstPtr = &(dstPtr[copySize]);
        }
    }
}

#ifdef PA_BIG_ENDIAN

#define C_DESERIALIZE_N(context,src,size) \
        c_copyDataDeser(size,src,context)

#endif /* PA_BIG_ENDIAN */


#ifdef PA_LITTLE_ENDIAN

#define C_DESERIALIZE_N(context,src,size) \
        c_copyDataSwappedDeser(size,src,context)

#endif /* PA_LITTLE_ENDIAN */

static void
c_bigEDeserString(
    c_base base,
    c_string *dst,
    c_serializeContext context)
{
    const c_char *copyPtr;
    c_ulong len;
    c_ulong saveLen;
    const c_char *stringStart;
    c_char *stringSave;
    c_char *stringNew;
    c_bool copyNeeded = FALSE;
    c_bool done = FALSE;

    assert(context->dataPtrPtr != NULL);
    assert(*context->dataPtrPtr != NULL);
    assert(dst != NULL);
    assert(context->lengthPtr != NULL);
    assert(context->action != NULL);

    copyPtr = (const c_char *)*context->dataPtrPtr;
    stringStart = copyPtr;
    stringSave = NULL;
    stringNew  = NULL;
    saveLen = 0;
    len = 0;
    do {
        if (*context->lengthPtr != 0) {
            if (*copyPtr == '\0') {
                /* End of the string reached */
                copyNeeded = TRUE;
                done = TRUE;
            }
            len++;
            (*context->lengthPtr)--;
            copyPtr = &(copyPtr[1]);
        } else {
            /* End of this fragment reached */
            copyNeeded = TRUE;
        }
        if (copyNeeded) {
            if (len > 0) {
                stringNew = os_malloc(len);
                assert(len > saveLen);
                assert((saveLen == 0) == (stringSave == NULL));
                if (stringSave != NULL) {
                    memcpy(stringNew, stringSave, saveLen);
                    os_free(stringSave);
                }
                memcpy(&(stringNew[saveLen]), stringStart, len-saveLen);
                stringSave = stringNew;
                saveLen = len;
            }
            if (!done) {
                /* Not finished yet, need to retrieve more fragments */
                *context->dataPtrPtr = (c_octet *)copyPtr;
                context->action(context->dataPtrPtr, context->lengthPtr,
                    context->actionArg);
                assert(*context->dataPtrPtr != NULL);
                assert(*context->lengthPtr > 0);
                copyPtr = (const c_char *)(*context->dataPtrPtr);
                stringStart = copyPtr;
            }
            copyNeeded = FALSE;
        }
    } while (!done);

    *context->dataPtrPtr = (c_octet *)copyPtr;

    assert((len-1) == strlen(stringNew));
    *dst = c_stringNew(base, stringNew);
    os_free(stringNew);
}


/** \brief Function for deserializing any collection type.
 *         Currently supported types are string, array, sequence and set.
 */
static void
c_bigEDeserCollection(
    c_collectionType collectionType,
    c_object *objectPtr,
    c_serializeContext context)
{
    c_long colSize;
    c_set set;
    c_object object, inserted;
    c_long i;
    c_bool isValidRef;
    c_bool isBunchOfBytes;

    /* An optimization for arrays or sequences of bytes */
    isBunchOfBytes = FALSE;
    if (c_baseObject(collectionType->subType)->kind == M_PRIMITIVE) {
        if ((c_primitive(collectionType->subType)->kind == P_OCTET) ||
            (c_primitive(collectionType->subType)->kind == P_CHAR)) {
            isBunchOfBytes = TRUE;
        }
    }

    /* Different behaviour for reftypes and non-reftypes */
    if (((collectionType->kind == C_ARRAY) ||
         (collectionType->kind == C_SEQUENCE)) &&
         !(int)c_typeIsRef(c_type(collectionType))) {
        /* Do nothing for non-referenced types */
        /* Only do something in case of optimized copying */
        if (isBunchOfBytes) {
            colSize = collectionType->maxSize;
            assert(colSize > 0);
            c_copyDataBunchOfBytesDeser(colSize, ((c_octet *)(*objectPtr)), context);
        }
    } else {
        C_DESERIALIZE_N(context, (c_octet *)&isValidRef, 1);

        if (isValidRef) {

            /* Only serialize the collection size in case of list/set/bag/etc */
            switch (collectionType->kind) {
            case C_STRING:
                c_bigEDeserString(c__getBase(collectionType),
                    (c_string *)(*objectPtr), context);
            break;
            case C_ARRAY:
            case C_SEQUENCE:
                /* Deserialize into new array if necessary */
                assert(c_typeIsRef(c_type(collectionType)));
                C_DESERIALIZE_N(context, (c_octet *)(&colSize), sizeof(c_ulong));
                *((c_array *)(*objectPtr)) = c_newBaseArrayObject(collectionType, colSize);
                if (isBunchOfBytes) {
                    assert(colSize > 0);
                    c_copyDataBunchOfBytesDeser(colSize, *((c_octet **)(*objectPtr)), context);
                }
            break;
            /* Other collectionTypes not yet implemented */
            case C_SET:
                /* Get the size */
                C_DESERIALIZE_N(context, (c_octet *)(&colSize), sizeof(c_ulong));
                /* Create the set */
                set = c_setNew(collectionType->subType);
                *((c_set *)(*objectPtr)) = set;
                /* And initialize it with objects */
                for (i=0; i<colSize; i++) {
                    object = c_new(collectionType->subType);
                    assert(object);
                    inserted = c_setInsert(set, object);
                    assert(inserted == object);
                    /* Let go of this insTance */
                    c_free(object);
                }
            break;
            case C_LIST:
            case C_BAG:
            case C_DICTIONARY:
            case C_QUERY:
                assert(FALSE);
            break;
            default:
                assert(FALSE); /* No other collection types supported */
            break;
            }
        }
    }
}

/** \brief Callback function for deepwalk when deserializing */
static void
c_bigEDeserCallback(
    c_type type,
    c_object *objectPtr,
    void *actionArg)
{
    c_bool isValidRef;
    c_serializeContext context = (c_serializeContext)actionArg;

    assert(context->dataPtrPtr != NULL);
    assert(*context->dataPtrPtr != NULL);
    assert(objectPtr != NULL);
    assert(context->lengthPtr != NULL);
    assert(context->action != NULL);

    switch (c_baseObject(type)->kind) {
    case M_COLLECTION:
        c_bigEDeserCollection(c_collectionType(type), objectPtr, context);
    break;
    case M_PRIMITIVE:
    case M_ENUMERATION:
        C_DESERIALIZE_N(context, (c_octet *)(*objectPtr), type->size);
    break;
    case M_INTERFACE:
    case M_CLASS:
        C_DESERIALIZE_N(context, (c_octet *)&isValidRef, 1);
        if (isValidRef) {
            if (!(**(c_object **)objectPtr)) {
                **(c_object **)objectPtr = c_new(type);
            }
        } else {
            /* Not a valid reference */
            **(c_object **)objectPtr = NULL;
        }
    break;
    default:
        assert(FALSE); /* No other expected than these */
    break;
    }
}


#undef C_FORMAT_ID
#undef C_FORMAT_VERSION


c_object
c_deserialize(
    c_type type,
    c_octet **bufferPtr,
    c_ulong *lengthPtr,
    c_serializeGetBufferAction action,
    c_serializeActionArg arg)
{
    c_object result;
    struct c_serializeContext_s contextStruct;

    contextStruct.dataPtrPtr = bufferPtr;
    contextStruct.lengthPtr = lengthPtr;
    contextStruct.action = action;
    contextStruct.actionArg = arg;

    if (!(int)c_typeIsRef(type)) {
        result = c_new(type);
        assert(result != NULL);
    } else {
        /* Class and interface reference created by deser algorithm */
        result = NULL;
    }
    c_deepwalk(type, &result, c_bigEDeserCallback, &contextStruct);

    return result;
}

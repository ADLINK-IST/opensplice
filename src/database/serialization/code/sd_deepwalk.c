/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
/** \file services/serialization/code/sd_deepwalk.c
 *  \brief Implementation of the deepwalk algorithm for serialization.
 *
 *  This is a general purpose mechanism for walking over all
 *  data elements of a database object.
 *
 *  All serializers make use of metadata in order to serialize
 *  the data. The recursive walk through this metadata is abstracted by the
 *  function \b deepwalk. When walking through the metadata, an object
 *  pointer pointing to an actual object instance is also walking through
 *  the data. In that way, the complete data instance is walked over.
 *  During the walk, a user-defined function is callbacked.
 *  This gives the user the possibility to execute custom actions for
 *  different types.
 *
 *  The algorithm of \b deepwalk has the following properties:
 *    - The caller passes a \b c_type which contains that metadata information
 *      on which the deepwalk has to be performed. This parameter is introduced
 *      for serializing and deserializing base-classes of classes. Usually,
 *      the \b c_type will be equal to the type of the object.
 *    - The caller passes a \b c_object which is the object to be walked.
 *      Since this object might be either serialized or deserialized,
 *      the object is supposed to be an in/out parameter. This is
 *      achieved by declaring the object parameter as a pointer to
 *      \b c_object.
 *      Typically, the object is an in-argument when serializing and an
 *      out-argument when deserializing.
 *    - The caller passes a void pointer value, a so-called action argument.
 *      This argument is considered an in/out parameter of the walk and
 *      is passed to the callback function.
 *      Typically, the action argument is an out-argument when serializing
 *      and an in-argument when deserializing.
 *    - The caller gives a function which is to be callbacked. The
 *      current object pointer will be passed to the callback, as
 *      well as the corresponding metadata and the action argument.
 *    - The callback function is called for types containing data only.
 *      These are:
 *      - primitive types like integers, floats, characters, etc.
 *      - enumeration types
 *      - union switches
 *      - the union case corresponding to the switch
 *      - collection types
 *      - collection type elements
 *    - For types containing no data, other actions are taken:
 *      - a walk over all members for structure types
 *      - a walk over all attributes for class and interface types
 *
 *  The following example illustrates the algorithm. Suppose that we have
 *  a database object which is a structure with the following layout:
 *
 *  \verbatim

    1   structure deepwalkExample {
    2     long     m_long;
    3     float    m_float;
    4     someEnum m_enum;
    5     union innerUnion switch (boolean) {
            case TRUE:  char m_u_char;
            case FALSE: short m_u_short;
            } m_union;
    6     short m_array[20];
    7     string m_string;
    8     struct innerStruct {
    9       char  m_i_char;
   10       float m_i_float;
          } m_innerStruct;
        };\endverbatim
 *
 *  Now assume the following code:
 *
 *  \verbatim

   void userCallback(c_type type, c_object *object, void *arg);

   sd_deepwalk(c_getType(object), &object, userCallback, (void *)&someVariable);\endverbatim
 *
 *  Here, \b object is the instance of type deepwalkExample and \b someVariable
 *  can be any useful variable to be used in the callback. When \b deepwalk is
 *  called, the metadata information of \b object will be retrieved. With this
 *  type, the \b deepwalkType function is called, which leads to the following
 *  steps (the list numbers refer to the line numbers as given in the code
 *  example):
 *  <ol>
 *  <li> \b deepwalkType dispatches to \b deepwalkStructure. This function will
 *    not call \b userCallback immediately because a struct does not contain any
 *    data itself. Instead, it will loop over all members and for each member
 *    determine the offset and type and call \b deepwalkType. Steps 2-8 are
 *    in this loop.
 *  <li> \b deepwalkType is called with the metadata of \b long and the address
 *    of \b m_long as parameters. This will be dispatched to
 *    \b deepwalkPrimitive, which in its turn will call \b userCallback using
 *    the same parameters.
 *  <li> \b deepwalkType is called with the metadata of \b float and the address
 *    of \b m_float as parameters. This will be dispatched to
 *    \b deepwalkPrimitive, which in its turn will call \b userCallback using
 *    the same parameters.
 *  <li> \b deepwalkType is called with the metadata of \b someEnum and the
 *    address of \b m_enum as parameters. This will be dispatched to
 *    \b deepwalkEnumeration, which in its turn will call \b userCallback using
 *    the same parameters.
 *  <li> \b deepwalkType is called with the metadata of \b innerUnion and the
 *    address of \b m_union as parameters. This will be dispatched to
 *    \b deepwalkUnion. The latter will first call \b deepwalkType with
 *    metadata of \b boolean and the address of the switch, which is the
 *    address of \b m_union. This results in a call to \b deepwalkPrimitive
 *    which in its turn will call the userCallback.
 *    After this, \b deepwalkUnion will investigate the contents of the switch
 *    field and determine the corresponding case label. Suppose the switch has
 *    value FALSE, then \b deepwalkType will be called with the metadata of
 *    \b short and the address of \b m_u_short. This dispatches to
 *    \b deepwalkPrimitive, which calls \b userCallback.
 *  <li> \b deepwalkType is called with the metadata of a fixed size array and
 *    the address of \b m_array as parameters. This will be dispatched to
 *    \b deepwalkCollection, which will first call \b userCallback with the
 *    given metadata and address. Then it will call \b deepwalkArrayElements,
 *    which will loop over all 20 elements in \b m_array. For each element,
 *    \b deepwalkType is called with the metadata of \b short and the address of
 *    the element as parameters. This will dispatch to \b deepwalkPrimitive,
 *    which will call \b userCallback.
 *  <li> \b deepwalkType is called with the metadata of \b string and the
 *    address of \b m_string. This dispatches to \b deepwalkCollection, which
 *    calls \b userCallback.
 *  <li> \b deepwalkType is called with the metadata of \b innerStruct and the
 *    address of \b m_innerstruct. This dispatches to \b deepwalkStructure which
 *    again will loop over its members. This leads to steps 9 and 10.
 *  <li> \b deepwalkType is called with the metadata of \b char and the address
 *    of \b m_i_char as parameters. This will be dispatched to
 *    \b deepwalkPrimitive, which in its turn will call \b userCallback using
 *    the same parameters.
 *  <li> \b deepwalkType is called with the metadata of \b float and the
 *    address of \b m_i_float as parameters. This will be dispatched to
 *    \b deepwalkPrimitive, which in its turn will call \b userCallback using
 *    the same parameters.
 *  </ol>
 *
 *  For a more extensive example of the use of deepwalk, see the serializerBigE
 *  class.
 *
 *  A more general purpose version of \b deepwalk is \b deepwalkMeta. See
 *  \b sd_deepwalkMeta.c for a description.
 */

/* interface */
#include "sd_deepwalk.h"

/* implementation */
#include "os_heap.h"
#include "c_typebase.h"
/* Note: private file included here because c_scope is a private type.
 * Serialization will probably be moved to the database in the future. */
#include "c__scope.h"
#include "c_metabase.h"
#include "c_collection.h"
#include "sd__confidence.h"
#include "sd_misc.h"


/* -------------------------- helpers -------------------------- */


/* --------------------------- deepwalk ------------------------ */


static void
sd_deepwalkType(
    c_type type, c_object *objectPtr,
    sd_deepwalkFunc action, void *actionArg);

/** \brief Helper structure for deepwalkInterface */
typedef struct sd_interfaceContext{
    c_object *objectPtr;      /**< address of the current interface object */
    sd_deepwalkFunc *action;  /**< action called for each interface attribute */
    void *actionArg;          /**< argument passed when calling the action */
} *sd_interfaceContext;


/** \brief Helper callback function for deepwalkInterface.
 *         This function handles one attribute of an interface.
 */
 
static c_bool
sd_deepwalkInterface(
    c_metaObject object,
    c_metaWalkActionArg actionArg)
{
    sd_interfaceContext context = (sd_interfaceContext)actionArg;
    c_property property;
    c_object propertyData;

    /* For now, we are interested in properties only */

    assert(c_baseObject(object)->kind == M_ATTRIBUTE);

    property = c_property(object);
    propertyData = C_DISPLACE(*context->objectPtr, property->offset);
    sd_deepwalkType(property->type, &propertyData,
                    context->action, context->actionArg);

    return TRUE;
}

/** \brief Routine for the walking over baseclasses of a class and over
 *         the (interface) attributes
 */
static void
sd_deepwalkClass(
    c_class class,
    c_object *objectPtr,
    sd_deepwalkFunc action,
    void *actionArg)
{
    struct sd_interfaceContext context;
    if (class->extends) {
        /* QAC EXPECT 3670; Recursive call is OK */
        sd_deepwalkClass(class->extends, objectPtr, action, actionArg);
    }
    if (*(c_object *)(*objectPtr)) {
        context.objectPtr = (c_object *)(*objectPtr);
        context.action = action;
        context.actionArg = actionArg;
        c_metaWalk(c_metaObject(class),
                   (c_metaWalkAction)sd_deepwalkInterface, &context);
    }
}


/** \brief Helper structure for deepwalkCollectionElements */
typedef struct sd_collectionContext{
    c_type type;             /**< metadata of the subType of the collection */
    sd_deepwalkFunc *action; /**< action called for each collection element */
    void *actionArg;         /**< argument passed when calling the action */
} *sd_collectionContext;


/** \brief Helper callback function for \b deepwalkCollectionElements.
 *         This function handles one element of a collection.
 */
 
static c_bool
sd_deepwalkCollection(
    c_object object,
    c_voidp actionArg)
{
    sd_collectionContext context = (sd_collectionContext)actionArg;
    c_object placeHolder;

    if (!(int)c_typeIsRef(context->type)) {
        placeHolder = object;
    } else {
        placeHolder = (c_object)&object;
    }

    sd_deepwalkType(context->type, &placeHolder,
                   context->action, context->actionArg);

    return TRUE;
}


/** \brief Routine for walking over any type
 *
 *  This routine determines which handling function to call by
 *  selecting on the metadata kind. It is the general entry
 *  point of \b deepwalk and is recursively called by most handling
 *  functions. 
 *
 *  \param type      Metadata information for the type.
 *  \param objectPtr Pointer to the object instance of the given type.
 *  \param action    The callback to be executed
 *  \param actionArg The user-defined pointer to be passed to the
 *                   action function.
 */

static void
sd_deepwalkType(
    c_type type,
    c_object *objectPtr,
    sd_deepwalkFunc action,
    void *actionArg)
{
    c_type actualType;
    int i, size;
    
    actualType = c_typeActualType(type);
    
    /* Determine which action to take */
    switch (c_baseObject(actualType)->kind) {
    case M_COLLECTION:
    {
        c_collectionType ctype;

        /* Action for the array type itself */
        action(actualType, objectPtr, actionArg);

        ctype = c_collectionType(actualType);
        switch (ctype->kind) {
        case C_ARRAY:
        case C_SEQUENCE:
        {
            c_object o;
            c_long size;

            if ((ctype->kind == C_ARRAY) &&
                (ctype->maxSize != 0)) {
                size = ctype->maxSize;
                o = *objectPtr;
            } else {
                o = *((c_object *)(*objectPtr));
                size = c_arraySize(o);
            }
            for (i=0; i<size; i++) {
                sd_deepwalkType(ctype->subType, &o, action, actionArg);
                if (c_typeIsRef(ctype->subType)) {
                    o = C_DISPLACE(o, sizeof(c_voidp));
                } else {
                    o = C_DISPLACE(o, ctype->subType->size);
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
            if (*(c_object *)(*objectPtr)) {
                struct sd_collectionContext context;
                /* Walk over the elements */
                context.type = ctype->subType;
                context.action = action;
                context.actionArg = actionArg;
                c_walk(*(c_object *)(*objectPtr),
                       (c_action)sd_deepwalkCollection,
                       &context);
            }               
        break;
        default:
            SD_CONFIDENCE(FALSE);
        break;
        }
    }
    break;
    case M_STRUCTURE:
    case M_EXCEPTION:
    {
        c_member member;
        c_structure structure = c_structure(actualType);
        c_object memberData;

        size = c_arraySize(structure->members);
        for (i=0; i<size; i++) {
            member = structure->members[i];
            memberData = C_DISPLACE(*objectPtr, (c_address)member->offset);
            sd_deepwalkType(c_specifier(member)->type,
                            &memberData,
                            action, actionArg);
        }
    }
    break;
    case M_CLASS:
        /* Action for the class itself */
        action(actualType, objectPtr, actionArg);
        /* Walk over the baseclasses and their attributes */
        sd_deepwalkClass(c_class(actualType), objectPtr, action, actionArg);
    break;
#ifndef NDEBUG
    case M_INTERFACE:
    {
        struct sd_interfaceContext context;

        /* action for the interface itself */
        action(actualType, objectPtr, actionArg);
        /* Only walk over the properties if the reference is valid. */
        if (*objectPtr && *(c_object *)(*objectPtr)) {
            context.objectPtr = (c_object *)(*objectPtr);
            context.action = action;
            context.actionArg = actionArg;
            c_metaWalk(c_metaObject(actualType),
                       (c_metaWalkAction)sd_deepwalkInterface,
                       &context);
        }
    }
    break;
#endif
    case M_UNION:
    {
        c_union un;
        c_unionCase activeCase;
        c_object unionData;
        c_long dataOffset;

        un = c_union(actualType);

        sd_deepwalkType(un->switchType, objectPtr, action, actionArg);

        /* Determine which case is valid and do action for this case */
        activeCase = sd_unionDetermineActiveCase(un, *objectPtr);
        SD_CONFIDENCE(activeCase);
        if (activeCase) {
            if (c_type(un)->alignment >= un->switchType->size) {
                dataOffset = c_type(un)->alignment;
            } else {
                dataOffset = un->switchType->size;
            }
            unionData = C_DISPLACE(*objectPtr, dataOffset);
            sd_deepwalkType(c_specifier(activeCase)->type,
                            &unionData,
                            action, actionArg);
        }
    }
    break;
    case M_PRIMITIVE:
    case M_ENUMERATION:
        action(actualType, objectPtr, actionArg);
    break;
    default:
        SD_CONFIDENCE(FALSE); /* Only descendants of type possible */
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

void
sd_deepwalk(
    c_type type,
    c_object *objectPtr,
    sd_deepwalkFunc action,
    void *actionArg)
{
    c_object **placeHolder;

    SD_CONFIDENCE(objectPtr);

    if (c_typeIsRef(type)) {
        placeHolder = &objectPtr;
    } else {
        placeHolder = (c_object**)objectPtr;
    }

    sd_deepwalkType(type, (c_object*)placeHolder, action, actionArg);
}

/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
/** \file services/serialization/code/sd_deepwalkMeta.c
 *  \brief Implementation of the deepwalkMeta algorithm for serialization.
 *
 *  This function extends the deepwalk function (see sd_deepwalk.c)
 *  by additionally walking over any elements having a name.
 *  This is useful for readable serialization types like XML.
 *  In general, \b deepwalkMeta can be considered a general purpose
 *  objectwalk, whereas \b deepwalk is used for walking over data
 *  elements only.
 *
 *  All serializers make use of metadata in order to serialize
 *  the data. The recursive walk through this metadata is abstracted by the
 *  function \b deepwalk. When walking through the metadata, an object
 *  pointer pointing to an actual object instance is also walking through
 *  the data. In that way, the complete data instance is walked over.
 *  During the walk, a user-defined function is callbacked.
 *  This gives the user the possibility to execute custom actions for
 *  different types.
 *  For readable types, not only data elements need to be walked over, but
 *  also all elements having a name. Furthermore, it sometimes is necessary to
 *  be callbacked when before starting the recursive walk and after
 *  finishing the recursive walk. The function \b deepwalkMeta offers these
 *  possibilities.
 *
 *  The algorithm of \b deepwalkMeta has the following properties:
 *    - The caller passes a \b c_object which is the object to be walked.
 *      Since this object might be either serialized or deserialized,
 *      the object is supposed to be an in/out parameter. This is
 *      achieved by declaring the object parameter as a pointer to
 *      \b c_object.
 *      Typically, the object is an in-argument when serializing and an
 *      out-argument when deserializing.
 *    - The caller passes a void pointer value, a so called action argument.
 *      This argument is considered an in/out parameter of the walk and
 *      is passed to the callback function.
 *      Typically, the action argument is an out-argument when serializing
 *      and an in-argument when deserializing.
 *    - The caller gives two functions which are to be callbacked. The first
 *      callback is invoked before any walk actions, the second is invoked
 *      after any walk actions. The current object pointer will be passed to
 *      the callbacks, as well as the name of the object (if applicable), the
 *      corresponding metadata and the action argument.
 *    - For some types, extra actions are taken in between the callbacks:
 *      - a walk over all members for structure types
 *      - a walk over all attributes for class and interface types
 *
 *  The following example illustrates the algorithm. Suppose that we have
 *  a database object which is a structure with the following layout:
 *
 *  \verbatim

    1   structure deepwalkMetaExample {
    2     long     m_long;
    3     float    m_float;
    4     someEnum m_enum;
    5     union innerUnion switch (boolean) {
            case TRUE:  char m_u_char;
    5       case FALSE: short m_u_short;
    5       } m_union;
    6     short m_array[20];
    7     string m_string;
    8     struct innerStruct {
    9       char  m_i_char;
   10       float m_i_float;
    8     } m_innerStruct;
    1   };\endverbatim
 *
 *  Now assume the following code:
 *
 *  \verbatim

   void userCallbackPre(const c_char *name, c_type type,
                        c_object *object, void *arg);
   void userCallbackPost(const c_char *name, c_type type,
                         c_object *object, void *arg);

   sd_deepwalkMeta(&object,
                   userCallbackPre,
                   userCallbackPost,
                   (void *)&someVariable);\endverbatim
 *
 *  Here, \b object is the instance of type deepwalkMetaExample and
 *  \b someVariable can be any useful variable to be used in the callback. When
 *  \b deepwalkMeta is called, the metadata information of \b object will be
 *  retrieved. With this type, the \b deepwalkMetaType function is called,
 *  which leads to the following steps (the list numbers refer to the line
 *  numbers as given in the code example):
 *  <ol>
 *  <li> \b deepwalkMetaType dispatches to \b deepwalkMetaStructure. This
 *    function will first call \b userCallbackPre with a NULL name parameter,
 *    the metadata of \b deepwalkMetaExample as type and \b object as address.
 *    Then it will loop over all members and for each member
 *    determine the offset and type and call \b deepwalkMetaType. Steps 2-8 are
 *    in this loop. Finally it will call \b userCalbackPost analogously to
 *    \b userCallbackPre.
 *  <li> \b deepwalkMetaType is called with the name \b "m_long", the metadata
 *    of \b long and the address of \b m_long as parameters. This will be
 *    dispatched to \b deepwalkMetaPrimitive, which in its turn will call
 *    \b userCallbackPre and \b userCallbackPost using the same parameters.
 *  <li> \b deepwalkMetaType is called with the name \b "m_float", the metadata
 *    of \b float and the address of \b m_float as parameters. This will be
 *    dispatched to \b deepwalkMetaPrimitive,  which in its turn will call
 *    \b userCallbackPre and \b userCallbackPost using the same parameters.
 *  <li> \b deepwalkMetaType is called with the name \b "m_enum", the metadata
 *    of \b someEnum and the address of \b m_enum as parameters. This will be
 *    dispatched to \b deepwalkMetaEnumeration, which in its turn will call
 *    \b userCallbackPre and \b userCallbackPost using the same parameters.
 *  <li> \b deepwalkMetaType is called with the name \b "m_union", the
 *    metadata of \b innerUnion and the address of \b m_union as parameters.
 *    This will be dispatched to \b deepwalkMetaUnion. The latter will first
 *    call \b userCallbackPre with the same parameters. Then it will call
 *    \b deepwalkMetaType with the name "switch", the metadata of \b boolean
 *    and the address of the switch, which is the address of \b m_union. This
 *    results in a call to \b deepwalkMetaPrimitive which in its turn will call
 *    the user callbacks. After this, \b deepwalkMetaUnion will investigate the
 *    contents of the switch field and determine the corresponding case label.
 *    Suppose the switch has value FALSE, then \b deepwalkMetaType will be
 *    called with the name \b "m_u_short", metadata of \b short and the
 *    address of \b m_u_short. This dispatches to \b deepwalkPrimitive, which
 *    calls the user callbacks.
 *  <li> \b deepwalkMetaType is called with the name "m_array", the metadata of
 *    a fixed size array and the address of \b m_array as parameters. This will
 *    be dispatched to \b deepwalkMetaCollection, which will first call
 *    \b userCallbackPre with the given name, metadata and address.
 *    Then it will call \b deepwalkMetaArrayElements, which will loop over all
 *    20 elements in \b m_array. For each element, \b deepwalkMetaType is
 *    called with the name "m_array", the metadata of \b short and the address
 *    of the element as parameters. This will dispatch to
 *    \b deepwalkMetaPrimitive, which will call the user callbacks.
 *  <li> \b deepwalkMetaType is called with the name \b "m_string", the metadata
 *    of \b string and the address of \b m_string. This dispatches to
 *    \b deepwalkMetaCollection, which calls the user callbacks.
 *  <li> \b deepwalkMetaType is called with the name \b "m_innerStruct", the
 *    metadata of \b innerStruct and the address of \b m_innerstruct. This
 *    dispatches to \b deepwalkMetaStructure which will call \b userCallbackPre,
 *    loop over the structure members and call \b userCallbackPost. The looping
 *    leads to steps 9 and 10.
 *  <li> \b deepwalkMetaType is called with the name "m_i_char", the metadata
 *    of \b char and the address of \b m_i_char as parameters. This will be
 *    dispatched to \b deepwalkMetaPrimitive, which in its turn will call
 *    the user callbacks.
 *  <li> \b deepwalkMetaType is called with the name "m_i_float", the metadata
 *    of \b float and the address of \b m_i_float as parameters. This will be
 *    dispatched to \b deepwalkMetaPrimitive, which in its turn will call the
 *    user callbacks.
 *  </ol>
 *
 *  For a more extensive example of the use of deepwalkMeta, see the
 *  serializerXML class.
 */

/* interface */
#include "sd__deepwalkMeta.h"

/* implementation */
#include "os_heap.h"
#include "c__scope.h"
#include "c_typebase.h"
#include "c_metabase.h"
#include "c_collection.h"
#include "sd__confidence.h"
#include "sd__resultCodes.h"
#include "sd_misc.h"


#define SD_ERRNO(errorType)   SD_ERRNO_##errorType
#define SD_MESSAGE(errorType) SD_MESSAGE_##errorType

#define SD_RETURN_ON_ERROR(context) \
    do { if ((context)->errorInfo) { \
        return FALSE; \
    } } while (0)

#define SD_SET_ERROR(context, errorType, name, location) \
    do { (context)->errorInfo = sd_errorInfoNew(SD_ERRNO(errorType), name, SD_MESSAGE(errorType), location); } while (0)

/* ----------------------------- errorInfo ---------------------------------- */

C_STRUCT(sd_errorInfo) {
    c_ulong errorNumber;
    c_char *name;
    c_char *message;
    void   *location;
};


c_char *
sd_errorInfoGetName(
    sd_errorInfo errorInfo)
{
    c_char *result = NULL;

    if (errorInfo) {
       result = errorInfo->name;
    }

    return result;
}


void
sd_errorInfoSetName(
    sd_errorInfo errorInfo,
    const c_char *name)
{
    if (errorInfo) {
        if (name) {
            errorInfo->name = sd_stringDup(name);
        } else {
             errorInfo->name = NULL;
        }
    }
}


sd_errorInfo
sd_errorInfoNew(
    c_ulong errorNumber,
    const c_char *name,
    const c_char *message,
    c_char *location)
{
    sd_errorInfo result;

    result = os_malloc(sizeof(*result));

    if (result) {
        result->errorNumber = errorNumber;
        sd_errorInfoSetName(result, name);
        if (message) {
            result->message = sd_stringDup(message);
        } else {
             result->message = NULL;
        }
        if (location) {
            while ((*location == ' ') || (*location == '\t') || (*location == '\n')) {
                location = &(location[1]);
            }
        }
        result->location = location;
    }

    return result;
}

void
sd_errorInfoFree(
    sd_errorInfo errorInfo)
{
    if (errorInfo) {
        if (errorInfo->name) {
            os_free(errorInfo->name);
        }
        if (errorInfo->message) {
            os_free(errorInfo->message);
        }
        os_free(errorInfo);
    }
}

c_bool
sd_errorInfoGet(
    sd_errorInfo errorInfo,
    c_ulong *errorNumber,
    c_char **name,
    c_char **message,
    c_char **location)
{
    if (errorInfo == NULL) {
        return FALSE;
    } else {
        *errorNumber = errorInfo->errorNumber;
        *name = errorInfo->name;
        *message = sd_stringDup(errorInfo->message);
        *location = errorInfo->location;
        return TRUE;
    }
}



/* ----------------------- deepwalkMetaContext ------------------------------ */

/* Private */

static c_bool sd_deepwalkMetaContextPre(sd_deepwalkMetaContext context, const char *name, c_type type, c_object *object) __nonnull((1, 3, 4)) __attribute_warn_unused_result__;
static c_bool sd_deepwalkMetaContextHook(c_bool *doRecurse, sd_deepwalkMetaContext context, const char *name, c_baseObject propOrMem, c_object *object) __nonnull_all__ __attribute_warn_unused_result__;
static c_bool sd_deepwalkMetaContextPost(sd_deepwalkMetaContext context, const char *name, c_type type, c_object *object) __nonnull((1, 3, 4)) __attribute_warn_unused_result__;


static c_bool
sd_deepwalkMetaContextHook(
    c_bool *doRecurse,
    sd_deepwalkMetaContext context,
    const char *name,
    c_baseObject propOrMem,
    c_object *object)
{
    return context->beforeAction(doRecurse, name, propOrMem, object, context->actionArg, &context->errorInfo, context->userData);
}


static c_bool
sd_deepwalkMetaContextPre(
    sd_deepwalkMetaContext context,
    const char *name,
    c_type type,
    c_object *object)
{
    return context->actionPre(name, type, object, context->actionArg, &context->errorInfo, context->userData);
}


static c_bool
sd_deepwalkMetaContextPost(
    sd_deepwalkMetaContext context,
    const char *name,
    c_type type,
    c_object *object)
{
    return context->actionPost(name, type, object, context->actionArg, &context->errorInfo, context->userData);
}

/* Public */

static c_bool sd_deepwalkMetaFuncDummy (const c_char *name, c_type type, c_object *object, void *arg, sd_errorInfo *errorInfo, void *userData)
{
    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(type);
    OS_UNUSED_ARG(object);
    OS_UNUSED_ARG(arg);
    OS_UNUSED_ARG(errorInfo);
    OS_UNUSED_ARG(userData);
    return TRUE;
}

static c_bool sd_deepwalkMetaHookDummy (c_bool *doRecurse, const char *name, c_baseObject propOrMem, c_object *object, void *arg, sd_errorInfo *errorInfo, void *userData)
{
    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(propOrMem);
    OS_UNUSED_ARG(object);
    OS_UNUSED_ARG(arg);
    OS_UNUSED_ARG(errorInfo);
    OS_UNUSED_ARG(userData);
    *doRecurse = TRUE;
    return TRUE;
}

void
sd_deepwalkMetaContextInit(
    sd_deepwalkMetaContext context,
    const sd_deepwalkMetaFunc actionPre,
    const sd_deepwalkMetaFunc actionPost,
    const sd_deepwalkMetaHook beforeAction,
    void *actionArg,
    void *userData)
{
    context->actionPre = actionPre ? actionPre : sd_deepwalkMetaFuncDummy;
    context->actionPost = actionPost ? actionPost : sd_deepwalkMetaFuncDummy;
    context->beforeAction = beforeAction ? beforeAction : sd_deepwalkMetaHookDummy;
    context->actionArg = actionArg;
    context->userData = userData;
    context->errorInfo = NULL;
}


void
sd_deepwalkMetaContextDeinit(
    sd_deepwalkMetaContext context)
{
    sd_errorInfoFree(context->errorInfo);
}

c_bool
sd_deepwalkMetaContextGetErrorInfo(
    sd_deepwalkMetaContext context,
    c_ulong *errorNumber,
    c_char **name,
    c_char **message,
    c_char **location)
{
    return sd_errorInfoGet(context->errorInfo, errorNumber, name, message, location);
}

/* --------------------------- deepwalkMeta ------------------------ */

static c_bool sd_deepwalkMetaType(c_type type, const c_char *name, c_object *objectPtr, sd_deepwalkMetaContext context) __nonnull((1, 3, 4)) __attribute_warn_unused_result__;


/** \brief Routine for handling primitive types.
 *
 *  This function does nothing but calling the actionPre and actionPost
 *  routines with the given name, primitive, the pointer to the data and
 *  the action argument.
 *
 *  \param primitive Metadata information for the primitive type.
 *  \param name      The name of the given object.
 *  \param objectPtr Pointer to the object instance of the primitive
 *                   type.
 *  \param context   Context object containing information on action routines,
 *                   action arguments and validation context
 */

static c_bool sd_deepwalkMetaPrimitive(c_primitive primitive, const c_char *name, c_object *objectPtr, sd_deepwalkMetaContext context) __nonnull((1, 3, 4)) __attribute_warn_unused_result__;

static c_bool
sd_deepwalkMetaPrimitive(
    c_primitive primitive,
    const c_char *name,
    c_object *objectPtr,
    sd_deepwalkMetaContext context)
{
    if (!sd_deepwalkMetaContextPre(context, name, c_type(primitive), objectPtr)) {
        return FALSE;
    }
    return sd_deepwalkMetaContextPost(context, name, c_type(primitive), objectPtr);
}


/** \brief Routine for handling enumeration types.
 *
 *  This function does nothing but calling the actionPre and actionPost
 *  routines with the given name, enumeration, the pointer to the data
 *  and the action argument.
 *
 *  \param enumeration Metadata information for the enumeration type.
 *  \param name      The name of the given object.
 *  \param objectPtr Pointer to the object instance of the enumeration
 *                   type.
 *  \param context   Context object containing information on action routines,
 *                   action arguments and validation context
 */

static c_bool sd_deepwalkMetaEnumeration(c_enumeration enumeration, const c_char *name, c_object *objectPtr, sd_deepwalkMetaContext context) __nonnull((1, 3, 4)) __attribute_warn_unused_result__;

static c_bool
sd_deepwalkMetaEnumeration(
    c_enumeration enumeration,
    const c_char *name,
    c_object *objectPtr,
    sd_deepwalkMetaContext context)
{
    if (!sd_deepwalkMetaContextPre(context, name, c_type(enumeration), objectPtr)) {
        return FALSE;
    }
    return sd_deepwalkMetaContextPost(context, name, c_type(enumeration), objectPtr);
}


/** \brief Routine for handling union types
 *
 *  First, the \b actionPre routine is called with the given name, metadata
 *  and address.
 *  Then this function descends into the general \b deepwalkMetaType function
 *  passing "switch" as name, the metadata of the switch type and the address of
 *  the data. Furthermore, it will
 *  determine the active case label from the data and again
 *  descend into \b deepwalkMetaType using the name of the active label,
 *  the metadata of the type of
 *  the active label and the address of the union data as parameters.
 *  Finally, the \b actionPost routine is called.
 *
 *  \param v_union   Metadata information for the union type.
 *  \param name      The name of the given object.
 *  \param objectPtr Pointer to the object instance of the union type.
 *  \param context   Context object containing information on action routines,
 *                   action arguments and validation context
 */

static c_bool sd_deepwalkMetaUnion(c_union v_union, const c_char *name, c_object *objectPtr, sd_deepwalkMetaContext context) __nonnull((1, 3, 4)) __attribute_warn_unused_result__;

static c_bool
sd_deepwalkMetaUnion(
    c_union v_union,
    const c_char *name,
    c_object *objectPtr,
    sd_deepwalkMetaContext context)
{
    c_unionCase activeCase;
    c_type caseType;
    c_char *caseName;
    c_object unionData;
    os_size_t dataOffset;

    if (!sd_deepwalkMetaContextPre(context, name, c_type(v_union), objectPtr)) {
        return FALSE;
    }

    /* action for the switch */
    if (!sd_deepwalkMetaType(v_union->switchType, "switch", objectPtr, context)) {
        return FALSE;
    }

    /* Determine which case is valid and do action for this case */
    activeCase = sd_unionDetermineActiveCase(v_union, *objectPtr);
    /* Make sure that a case is found */
    if (activeCase) {
        caseType = c_specifier(activeCase)->type;
        caseName = c_specifier(activeCase)->name;
        if (c_type(v_union)->alignment >= v_union->switchType->size) {
            dataOffset = c_type(v_union)->alignment;
        } else {
            dataOffset = v_union->switchType->size;
        }
        unionData = C_DISPLACE(*objectPtr, C_ADDRESS(dataOffset));
        if (!sd_deepwalkMetaType(caseType, caseName, &unionData, context)) {
            return FALSE;
        }
    }

    return sd_deepwalkMetaContextPost(context, name, c_type(v_union), objectPtr);
}


/** \brief Routine for handling structure types
 *
 *  For each member of the structure, this function will determine
 *  the address and type information and send these to the general
 *  deepwalkType function.
 *
 *  \param structure Metadata information for the structure type.
 *  \param name      The name of the given object.
 *  \param objectPtr Pointer to the object instance of the structure type.
 *  \param context   Context object containing information on action routines,
 *                   action arguments and validation context
 */

static c_bool sd_deepwalkMetaStructure(c_structure structure, const c_char *name, c_object *objectPtr, sd_deepwalkMetaContext context) __nonnull((1, 3, 4)) __attribute_warn_unused_result__;

static c_bool
sd_deepwalkMetaStructure(
    c_structure structure,
    const c_char *name,
    c_object *objectPtr,
    sd_deepwalkMetaContext context)
{
    c_ulong i, size;
    c_member member;
    c_type memberType;
    c_char *memberName;
    c_object memberData;

    if (!sd_deepwalkMetaContextPre(context, name, c_type(structure), objectPtr)) {
        return FALSE;
    }

    /* action for members */
    size = c_arraySize(structure->members);
    for (i=0; i<size; i++) {
        c_bool doRecurse;
        member = structure->members[i];
        memberName = c_specifier(member)->name;
        memberData = C_DISPLACE(*objectPtr, C_ADDRESS(member->offset));
        if (!sd_deepwalkMetaContextHook(&doRecurse, context, memberName, c_baseObject(member), &memberData)) {
            return FALSE;
        } else if (doRecurse) {
            memberType = c_typeActualType(c_specifier(member)->type);
            if (!sd_deepwalkMetaType(memberType, memberName, &memberData, context)) {
                return FALSE;
            }
        }
    }

    return sd_deepwalkMetaContextPost(context, name, c_type(structure), objectPtr);
}

/** \brief Helper structure for deepwalkInterface */
typedef struct sd_interfaceMetaContext{
    c_object *objectPtr; /**< address of the current interface object */
    const c_char *name;  /**< name of the current interface object */
    sd_deepwalkMetaContext context;
} *sd_interfaceMetaContext;


/** \brief Helper callback function for deepwalkInterface.
 *         This function handles one attribute of an interface.
 */

static c_bool sd_deepwalkMetaInterfaceCallback(c_metaObject object, c_metaWalkActionArg actionArg) __nonnull((1, 2)) __attribute_warn_unused_result__;

static c_bool
sd_deepwalkMetaInterfaceCallback(
    c_metaObject object,
    c_metaWalkActionArg actionArg)
{
    sd_interfaceMetaContext ifContext = actionArg;
    c_property property;
    c_type propertyType;
    c_char *propertyName;
    c_object propertyData;

    /* Get out of here in case of error */
    if (ifContext->context->errorInfo != NULL) {
        return FALSE;
    }

    /* For now, we are interested in properties only */
    if (c_baseObject(object)->kind == M_ATTRIBUTE) {
        c_bool doRecurse;
        property = c_property(object);
        propertyData = C_DISPLACE(*ifContext->objectPtr, C_ADDRESS(property->offset));
        propertyName = c_metaObject(property)->name;
        if (!sd_deepwalkMetaContextHook(&doRecurse, ifContext->context, propertyName, c_baseObject(property), &propertyData)) {
            return FALSE;
        } else if (doRecurse) {
            propertyType = property->type;
            if (!sd_deepwalkMetaType(propertyType, propertyName, &propertyData, ifContext->context)) {
                return FALSE;
            }
        }
    }

    return (c_bool)(ifContext->context->errorInfo == NULL);
}

/** \brief Routine called by sd_deepwalkMetaInterface to do the
 *         actual walking over attributes
 */
static c_bool sd_deepwalkMetaDoInterface(c_interface interf, const c_char *name, c_object *objectPtr, sd_deepwalkMetaContext context) __nonnull((1, 3, 4)) __attribute_warn_unused_result__;

static c_bool
sd_deepwalkMetaDoInterface(
    c_interface interf,
    const c_char *name,
    c_object *objectPtr,
    sd_deepwalkMetaContext context)
{
    struct sd_interfaceMetaContext ifContext;
    c_object *inst;

    inst = (c_object *)(*objectPtr);
    /* Only walk over properties if the reference is valid */
    if (inst && *inst) {
        /* action for the members */
        /* members are stored in scope (unordered), so use c_walk */
        ifContext.name = name;
        ifContext.objectPtr = inst;
        ifContext.context = context;
        if (!c_metaWalkBool(c_metaObject(interf), sd_deepwalkMetaInterfaceCallback, &ifContext)) {
            return FALSE;
        }
    }

    return TRUE;
}

#ifndef NDEBUG
/** \brief Routine for handling interface types
 *
 *  This function first calls \b actionPre. Then it walks over all interface
 *  attributes. The walk will call the general deepwalkMetaType function
 *  with the corresponding type and datapointer. This is done by
 *  \b sd_deepwalkMetaInterfaceCallback. Finally, \b actionPost is called.
 *
 *  \param interface Metadata information for the interface type.
 *  \param name      The name of the given object.
 *  \param objectPtr Pointer to the object instance of the interface type.
 *  \param context   Context object containing information on action routines,
 *                   action arguments and validation context.
 */

static c_bool sd_deepwalkMetaInterface(c_interface interf, const c_char *name, c_object *objectPtr, sd_deepwalkMetaContext context) __nonnull((1, 3, 4)) __attribute_warn_unused_result__;

static c_bool
sd_deepwalkMetaInterface(
    c_interface interf,
    const c_char *name,
    c_object *objectPtr,
    sd_deepwalkMetaContext context)
{
    if (!sd_deepwalkMetaContextPre(context, name, c_type(interf), objectPtr)) {
        return FALSE;
    }
    if (!sd_deepwalkMetaDoInterface(interf, name, objectPtr, context)) {
        return FALSE;
    }
    return sd_deepwalkMetaContextPost(context, name, c_type(interf), objectPtr);
}
#endif


/** \brief Routine called by sd_deepwalkMetaClass to do the
 *         walking over parent classes and to call the routine to
 *         walk over attributes
 */
static c_bool sd_deepwalkMetaDoClass(c_class class, const c_char *name, c_object *objectPtr, sd_deepwalkMetaContext context) __nonnull((1, 3, 4)) __attribute_warn_unused_result__;

static c_bool
sd_deepwalkMetaDoClass(
    c_class class,
    const c_char *name,
    c_object *objectPtr,
    sd_deepwalkMetaContext context)
{
    if (class->extends) {
        if (!sd_deepwalkMetaDoClass(class->extends, name, objectPtr, context)) {
            return FALSE;
        }
    }
    return sd_deepwalkMetaDoInterface(c_interface(class), name, objectPtr, context);
}

/** \brief Routine for handling class types.
 *
 *  A class can be extended from a base class. This routine walks
 *  up in the inheritance tree and call deepwalkInterface for
 *  each inheritance level.
 *
 *  \param class     Metadata information for the class type.
 *  \param name      The name of the given object.
 *  \param objectPtr Pointer to the object instance of the class
 *                   type.
 *  \param actionPre The callback to be executed right after entering.
 *  \param actionPost The callback to be executed just before leaving.
 *  \param actionArg The user-defined pointer to be passed to the
 *                   action functions.
 */

static c_bool sd_deepwalkMetaClass(c_class class, const c_char *name, c_object *objectPtr, sd_deepwalkMetaContext context) __nonnull((1, 3, 4)) __attribute_warn_unused_result__;

#define SD_CONSTANT_NAME "c_constant"
static c_bool
sd_deepwalkMetaClass(
    c_class class,
    const c_char *name,
    c_object *objectPtr,
    sd_deepwalkMetaContext context)
{
    c_class actualClass;
    int isConst = FALSE;

    if ((*objectPtr) && *((c_class *)(*objectPtr))) {
        /* For deepwalkMeta, we are interested in the actual class. We might
         * have a baseclass here so downcast to the real class
         */
        actualClass = c_class(c_getType(*((c_type *)(*objectPtr))));
    } else {
        actualClass = class;
    }
    /* Workaround for const values */
    isConst = (c_metaObject(actualClass)->name &&
               (strncmp(c_metaObject(actualClass)->name, SD_CONSTANT_NAME, sizeof(SD_CONSTANT_NAME)) == 0));

    if (!isConst) {

        if (!sd_deepwalkMetaContextPre(context, name, c_type(class), objectPtr)) {
            return FALSE;
        }

        /* Again re-evaluate the actual class because the Pre-function might
         * have changed its value. This is the case for virtual classes
         */
        if ((*objectPtr) && *((c_class *)(*objectPtr))) {
            /* For deepwalkMeta, we are interested in the actual class. We might
             * have a baseclass here so downcast to the real class
             */
            actualClass = c_class(c_getType(*((c_type *)(*objectPtr))));
        } else {
            actualClass = class;
        }

        if (!sd_deepwalkMetaDoClass(actualClass, name, objectPtr, context)) {
            return FALSE;
        }

        if (!sd_deepwalkMetaContextPost(context, name, c_type(class), objectPtr)) {
            return FALSE;
        }
    } else {
        /* Workaround for const values */
        c_constant constant;
        c_string constName;
        c_string *namePtr;
        c_string** placeHolder;
        c_type stringType;

        assert(*objectPtr);
        constant = *((c_constant *)(*objectPtr));
        if (constant) {
            constName = c_metaObject(constant)->name;
            stringType = c_getType(constName);
            SD_CONFIDENCE(stringType);
            SD_CONFIDENCE(c_baseObject(stringType)->kind == M_COLLECTION);
            SD_CONFIDENCE(c_collectionType(stringType)->kind == OSPL_C_STRING);
            namePtr = &constName;
            placeHolder = &namePtr;
            if (!sd_deepwalkMetaType(stringType, name, (c_object *)placeHolder, context)) {
                return FALSE;
            }
        } else {
            if (!sd_deepwalkMetaContextPre(context, name, c_type(actualClass), objectPtr)) {
                return FALSE;
            }
            if (!sd_deepwalkMetaContextPost(context, name, c_type(actualClass), objectPtr)) {
                return FALSE;
            }
        }
    }

    return TRUE;
}
#undef SD_CONSTANT_NAME


/** \brief Routine for walking over array elements
 *
 *  Arrays need to treated in a special way because they exist of
 *  several elements. This function walks over all elements and for each
 *  element determines the address and calls \b deepwalkType with the
 *  array name, the array subtype metadata and the address as parameters.
 *
 *  \param array     Metadata information for the array type.
 *  \param name      The name of the given object.
 *  \param objectPtr Pointer to the object instance of the array
 *                   type.
 *  \param context   Context object containing information on action routines,
 *                   action arguments and validation context
 */
static c_bool sd_deepwalkMetaArrayElements(c_collectionType array, const c_char *name, c_object *objectPtr, sd_deepwalkMetaContext context) __nonnull((1, 3, 4)) __attribute_warn_unused_result__;

static c_bool
sd_deepwalkMetaArrayElements(
    c_collectionType array,
    const c_char *name,
    c_object *objectPtr,
    sd_deepwalkMetaContext context)
{
    c_ulong i, arrayLength;
    c_object baseObject;
    c_object currentObject;

    /* Walk over all entries */
    switch (c_collectionType(array)->kind) {
        case OSPL_C_ARRAY:
            if (array->maxSize == 0) {
                arrayLength = c_arraySize(*((c_array *)(*objectPtr)));
                baseObject = *((c_object *)(*objectPtr));
            } else {
                arrayLength = array->maxSize;
                baseObject = *objectPtr;
            }
            break;
        case OSPL_C_SEQUENCE:
            arrayLength = c_arraySize(*((c_array *)(*objectPtr)));
            baseObject = *((c_object *)(*objectPtr));
            break;
        default:
            SD_CONFIDENCE(FALSE);
            arrayLength = 0;
            baseObject = NULL;
            break;
    }

    currentObject = baseObject;
    for (i=0; i<arrayLength; i++) {
        if (!sd_deepwalkMetaType(array->subType, name, &currentObject, context)) {
            return FALSE;
        }

        if (c_typeIsRef(array->subType)) {
            currentObject = C_DISPLACE(currentObject,C_ADDRESS(sizeof(c_voidp)));
        } else {
            currentObject = C_DISPLACE(currentObject,C_ADDRESS(array->subType->size));
        }
    }

    return TRUE;
}


/** \brief Helper structure for deepwalkComplexCollectionElements */
typedef struct sd_collectionMetaContext{
    c_type type;         /**< metadata information of the current subtype */
    const c_char *name;  /**< name of the current collection object */
    sd_deepwalkMetaContext context;
} *sd_collectionMetaContext;


/** \brief Helper callback function for deepwalkComplexCollectionElements.
 *         This function handles one element of a complex collection
 *         (e.g. \b SET).
 */
static c_bool sd_deepwalkMetaCollectionCallback(c_object object, c_voidp actionArg) __nonnull((1)) __attribute_warn_unused_result__;

static c_bool
sd_deepwalkMetaCollectionCallback(
    c_object object,
    c_voidp actionArg)
{
    sd_collectionMetaContext cContext = actionArg;
    c_object placeHolder;

    if (!c_typeIsRef(cContext->type)) {
        placeHolder = object;
    } else {
        placeHolder = (c_object)&object;
    }

    return sd_deepwalkMetaType(cContext->type, cContext->name, &placeHolder, cContext->context);
}

static c_bool sd_deepwalkMetaCollectionScopeWalkAction(c_metaObject o, c_scopeWalkActionArg arg) __nonnull((1)) __attribute_warn_unused_result__;

static c_bool
sd_deepwalkMetaCollectionScopeWalkAction(
    c_metaObject o,
    c_scopeWalkActionArg arg)
{
    return sd_deepwalkMetaCollectionCallback(c_object(o), arg);
}


/** \brief Routine for walking over all collection elements.
 *
 *  This routine handles elements of collections which are neither
 *  arrays nor strings. Such collections can be walked over by the
 *  \b c_walk function. During the walk, all elements will be
 *  passed to \b deepwalkMetaType with their name, metadata and address as
 *  parameters. This is done by the function
 *  \b sd_deepwalkMetaCollectionCallback.
 *
 *  \param collectionType Metadata information for the collection type.
 *                   Currently, only the \b SET type is supported.
 *  \param name      The name of the given object.
 *  \param objectPtr Pointer to the object instance of the collection
 *                   type.
 *  \param context   Context object containing information on action routines,
 *                   action arguments and validation context.
 */

static c_bool sd_deepwalkMetaArrayElements(c_collectionType array, const c_char *name, c_object *objectPtr, sd_deepwalkMetaContext context) __nonnull((1, 3, 4)) __attribute_warn_unused_result__;

static c_bool
sd_deepwalkMetaCollectionElements(
    c_collectionType collectionType,
    const c_char *name,
    c_object *objectPtr,
    sd_deepwalkMetaContext context)
{
    struct sd_collectionMetaContext cContext;
    c_collection collectionInst;

    SD_CONFIDENCE((collectionType->kind == OSPL_C_LIST) ||
                  (collectionType->kind == OSPL_C_BAG)  ||
                  (collectionType->kind == OSPL_C_SET)  ||
                  (collectionType->kind == OSPL_C_DICTIONARY) ||
                  (collectionType->kind == OSPL_C_QUERY));

    collectionInst = *((c_collection *)(*objectPtr));
    if (collectionInst) {
        /* Walk over the elements */
        cContext.name = name;
        cContext.type = collectionType->subType;
        cContext.context = context;
        return c_walk(collectionInst, sd_deepwalkMetaCollectionCallback, &cContext);
    } else {
        return TRUE;
    }
}


/** \brief Routine for walking over all collection elements.
 *
 *  This routine handles elements of collections which are neither
 *  arrays nor strings. Such collections can be walked over by the
 *  \b c_walk function. During the walk, all elements will be
 *  passed to \b deepwalkMetaType with their name, metadata and address as
 *  parameters. This is done by the function
 *  \b sd_deepwalkMetaCollectionCallback.
 *
 *  \param collectionType Metadata information for the collection type.
 *                   Currently, only the \b SET type is supported.
 *  \param name      The name of the given object.
 *  \param objectPtr Pointer to the object instance of the collection
 *                   type.
 *  \param context   Context object containing information on action routines,
 *                   action arguments and validation context.
 */

static c_bool sd_deepwalkMetaArrayElements(c_collectionType array, const c_char *name, c_object *objectPtr, sd_deepwalkMetaContext context) __nonnull((1, 3, 4)) __attribute_warn_unused_result__;

static c_bool
sd_deepwalkMetaScopeElements(
    c_collectionType collectionType,
    const c_char *name,
    c_object *objectPtr,
    sd_deepwalkMetaContext context)
{
    struct sd_collectionMetaContext cContext;
    c_scope scopeInst;

    SD_CONFIDENCE(collectionType->kind == OSPL_C_SCOPE);

    scopeInst = *((c_scope *)(*objectPtr));
    if (scopeInst) {
        /* Walk over the elements */
        cContext.name = name;
        cContext.type = collectionType->subType;
        cContext.context = context;
        return c_scopeWalkBool(scopeInst, sd_deepwalkMetaCollectionScopeWalkAction, &cContext);
    } else {
        return TRUE;
    }
}


/** \brief Function for handling collection types.
 *
 *  For collections, first \b actionPre is called with the given name,
 *  metadata information and the address as parameters.
 *  This is sufficient for strings, but arrays and sets need some more
 *  actions in order to walk over their elements. This is delegated to
 *  \b deepwalkArrayElements and \b deepwalkComplexCollectionElements.
 *  Finally, \b callbackPost is called.
 *
 *  \param collectionType Metadata information for the collection type.
 *  \param name      The name of the given object.
 *  \param objectPtr Pointer to the object instance of the collection type.
 *  \param context   Context object containing information on action routines,
 *                   action arguments and validation context.
 */

static c_bool sd_deepwalkMetaCollection(c_collectionType collectionType, const c_char *name, c_object *objectPtr, sd_deepwalkMetaContext context) __nonnull((1, 3, 4)) __attribute_warn_unused_result__;

static c_bool
sd_deepwalkMetaCollection(
    c_collectionType collectionType,
    const c_char *name,
    c_object *objectPtr,
    sd_deepwalkMetaContext context)
{
    if (!sd_deepwalkMetaContextPre(context, name, c_type(collectionType), objectPtr)) {
        return FALSE;
    }

    switch (collectionType->kind) {
        case OSPL_C_ARRAY:
        case OSPL_C_SEQUENCE:
            if (!sd_deepwalkMetaArrayElements(collectionType, "element", objectPtr, context)) {
                return FALSE;
            }
            break;
        case OSPL_C_STRING:
            ; /* No action here, chars are not treated separately */
            break;
        case OSPL_C_SET:
        case OSPL_C_LIST:
        case OSPL_C_BAG:
        case OSPL_C_DICTIONARY:
        case OSPL_C_QUERY:
            if (!sd_deepwalkMetaCollectionElements(collectionType, "element", objectPtr, context)) {
                return FALSE;
            }
            break;
        case OSPL_C_SCOPE:
            if (!sd_deepwalkMetaScopeElements(collectionType, "element", objectPtr, context)) {
                return FALSE;
            }
            break;
        default:
            SD_CONFIDENCE(FALSE);
            break;
    }

    if (!sd_deepwalkMetaContextPost(context, name, c_type(collectionType), objectPtr)) {
        return FALSE;
    }

    return TRUE;
}



/** \brief Routine for handling any type
 *
 *  This routine determines which handling function to call by
 *  selecting on the metadata kind. It is the general entry
 *  point of \b deepwalkMeta and is recursively called by most handling
 *  functions.
 *
 *  \param type      Metadata information for the type.
 *  \param name      The name of the given object.
 *  \param objectPtr Pointer to the object instance of the given type.
 *  \param context   Context object containing information on action routines,
 *                   action arguments and validation context.
 */

static c_bool
sd_deepwalkMetaType(
    c_type type,
    const c_char *name,
    c_object *objectPtr,
    sd_deepwalkMetaContext context)
{
    c_type const actualType = c_typeActualType(type);
    c_bool ok;

    switch (c_baseObject(actualType)->kind) {
        case M_COLLECTION:
            ok = sd_deepwalkMetaCollection(c_collectionType(actualType), name, objectPtr, context);
            break;
        case M_STRUCTURE:
        case M_EXCEPTION:
            ok = sd_deepwalkMetaStructure(c_structure(actualType), name, objectPtr, context);
            break;
        case M_CLASS:
            ok = sd_deepwalkMetaClass(c_class(actualType), name, objectPtr, context);
            break;
#ifndef NDEBUG
        case M_INTERFACE:
            ok = sd_deepwalkMetaInterface(c_interface(actualType), name, objectPtr, context);
            break;
#endif
        case M_UNION:
            ok = sd_deepwalkMetaUnion(c_union(actualType), name, objectPtr, context);
            break;
        case M_PRIMITIVE:
            ok = sd_deepwalkMetaPrimitive(c_primitive(actualType), name, objectPtr, context);
            break;
        case M_ENUMERATION:
            ok = sd_deepwalkMetaEnumeration(c_enumeration(actualType), name, objectPtr, context);
            break;
        default:
            SD_CONFIDENCE(FALSE); /* Only descendants of type possible */
            ok = FALSE;
            break;
    }

    assert (ok || context->errorInfo);
    return ok;
}


/** \brief Routine for recursively walking over all data-containing elements of
 *         a database object.
 *
 *  A general mechanism for walking over the complete structure of an
 *  object can be reused by several kinds of serializers. The \b deepwalkMeta
 *  function offers a general mechanism for this. By passing an object and a
 *  userCallbackPre and userCallbackPost, the user will be capable of executing
 *  his own actions for each element contained by the object. In order to
 *  allow serialization and deserialization to readable formats, the name of
 *  each element is a parameter of the callbacks as well.
 *
 *  The routine itself will determine the metadata of the object and use this
 *  for walking over its members, attributes or elements.
 *
 *  \param type      Metadata information for the object.
 *  \param name      Readable name for use in output routines
 *  \param objectPtr Pointer to the object instance to walk over.
 *  \param context   Context object ontaining information on which actions to
 *                   execute.
 */

c_bool
sd_deepwalkMeta(
    c_type type,
    const c_char *name,
    c_object *objectPtr,
    sd_deepwalkMetaContext context)
{
    c_object **placeHolder;

    if (c_typeIsRef(type)) {
        placeHolder = &objectPtr;
    } else {
        placeHolder = (c_object**)objectPtr;
    }

    return sd_deepwalkMetaType(type, name, (c_object*)placeHolder, context);
}

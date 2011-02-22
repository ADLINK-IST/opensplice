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
 * The 'in_object' class is the base class within the DDSi component. It
 * provides facilities for reference counting and type checking. The comments
 * in this file and the accompanying C implementation file for this object also
 * describe the coding conventions for all code within this component.
 *
 * The structure of a header file is respectively:
 *  1)  Comment header with the name of the file and the date it was created. It
 *      can also be used to capture the history.
 *  2)  A 'define' that prevents failure due to multiple inclusion of the
 *      header file in implementation files.
 *  3)  Include statements.
 *  4)  C++ 'define' to allow inclusion in C++ code.
 *  5)  For PUBLIC header files only there is an additional requirement to
 *      support the Windows platform. Since a public header file is potentially
 *      included by files in other components, some additional work is needed.
 *      Depending on whether the component, which the header file is part of,
 *      is compiled itself or another component is compiled that includes the
 *      header file, all functions must respectively be declared to be
 *      'exported' or 'imported'. In OpenSplice an additional definition is
 *      needed in the public header files to support both modes depending on the
 *      definition of a specific symbol that is unique for that component:
 *
 *          #include "os_if.h"
 *
 *          #ifdef OSPL_BUILD_<COMPONENT_ID>
 *          #define OS_API OS_API_EXPORT
 *          #else
 *          #define OS_API OS_API_IMPORT
 *          #endif
 *
 *      When the component is compiled the OSPL_BUILD_<COMPONENT_ID> must be
 *      set (using a define in the makefile).
 *
 *      Besides that, each function declaration that follows after this must
 *      be prefixed with the macro: OS_API.
 *  6)  Declaration of types (typedefs, structs, classes, cast macros, etc). The
 *      order may vary due to dependencies between them.
 *  7)  Constructor (depending on the scope).
 *  8)  Initializer (depending on the scope).
 *  9)  Deinitializer (depending on the scope).
 *  10) Destructor (depending on the scope).
 *  11) Getter and setter functions grouped by class member (depending on the
 *      scope).
 *  12) All other functions of the class (depending on the scope).
 *  13) The undefine of OS_API (#undef OS_API) in case the header file is public
 *  14) The closing brace for the C++ support (extern "C")
 *  15) The #endif to match the multiple inclusion #define.
 */


/**
 * Names of classes must be prefixed with the component identifier followed by
 * and underscore and a 'self-describing' name in 'camelback' notation. The
 * component indentifier is prefixed to assure global uniqueness of the
 * class name. It can be considered similar to a package in the Java language.
 *
 * Example: in_hashTable
 */

/**
 * A header file provides the interface of a class. The scope of (parts of) the
 * interface determine the name and contents of the header file.
 * 1) Scope 'public'
 *      The header file must be called <prefix>_<className>.h and be located in
 *      the include directory of the component. The part of the class that is
 *      made available in this file can be used by all other classes, also the
 *      ones in other components.
 *
 * 2) Scope 'protected' or 'package'
 *      The header file must be called <prefix>__<className>.h (with 2
 *      underscores) in the code directory of the component. The part of the
 *      class that is made available in this file can be user by all other
 *      classes in the same component.
 *
 * 3) Scope 'private'
 *      This will NOT be in a header file at all.
 */

/* Prevent failure due to multiple inclusion of this file. */
#ifndef _IN__OBJECT_H
#define _IN__OBJECT_H

/*Includes. */
#include "os_defs.h"
#include "os_classbase.h"
#include "os_stdlib.h"
#include "in_commonTypes.h"
/**
 * Allow usage of this C code from C++ code. Always include this in a header
 * file.
 */
#if defined (__cplusplus)
extern "C" {
#endif

/**
 * Each available class has its own kind. When a new class is added, a new
 * objectKind must be introduced in this enumeration. The first and the
 * last element are reserved for type checking reasons.
 */
typedef enum in_objectKind
{
    /*Represents the invalid type to be used when freeing an object */
    IN_OBJECT_KIND_INVALID,
    /*Represents the locator object kind */
    IN_OBJECT_KIND_LOCATOR,
    /*Represents the locator (embedded) object kind */
    IN_OBJECT_KIND_LOCATOR_EMBEDDED,
    /*Represents the messageDeserializer object kind */
    IN_OBJECT_KIND_MESSAGE_DESERIALIZER,
    /*Represents the messageSerializer object kind */
    IN_OBJECT_KIND_MESSAGE_SERIALIZER,
    /*Represents the peerReader object kind */
    IN_OBJECT_KIND_PEER_READER,
    /*Represents the writerFacade object kind */
    IN_OBJECT_KIND_WRITER_FACADE,
    /*Represents the readerFacade object kind */
    IN_OBJECT_KIND_READER_FACADE,
    /*Represents the plugReceiveChannel object kind */
    IN_OBJECT_KIND_RECEIVE_CHANNEL,
    /*Represents the plugSendChannel object kind */
    IN_OBJECT_KIND_SEND_CHANNEL,
    /*Represents the in_dataBufferIBasic implementation */
    IN_OBJECT_KIND_DATA_BUFFER_BASIC,
    /*Represents the in_sendBuffer implementation */
    IN_OBJECT_KIND_SEND_BUFFER_BASIC,
    /*Represents the in_receiveBuffer implementation */
    IN_OBJECT_KIND_RECEIVE_BUFFER_BASIC,
    /*Represents the in_transportReaderIBasic implementation*/
    IN_OBJECT_KIND_TRANSPORT_RECEIVER_BASIC,
    /*Represents the in_transportWriterIBasic implementation*/
    IN_OBJECT_KIND_TRANSPORT_SENDER_BASIC,
    /*Represents the in_transportReaderIBasic implementation*/
    IN_OBJECT_KIND_TRANSPORT_PAIR_BASIC,
    /*Represents the in_ddsiStreamWriterImpl implementation*/
    IN_OBJECT_KIND_STREAM_WRITER_BASIC,
    /*Represents the in_ddsiStreamReaderImpl implementation*/
    IN_OBJECT_KIND_STREAM_READER_BASIC,
    /*Represents the in_streamPair implementation*/
    IN_OBJECT_KIND_STREAM_PAIR_BASIC,
    /*Represents the Connectivity Admin object kind */
    IN_OBJECT_KIND_CONNECTIVITY_ADMIN,
    /*Represents the participantFacade object kind */
    IN_OBJECT_KIND_PARTICIPANT_FACADE,
    /*Represents the peerParticipant object kind */
    IN_OBJECT_KIND_PEER_PARTICIPANT,
    /*Represents the peerWriter object kind */
    IN_OBJECT_KIND_PEER_WRITER,
    /*Represents the dataChannel object kind */
    IN_OBJECT_KIND_DATA_CHANNEL,
    /*Represents the data channel writer object kind */
    IN_OBJECT_KIND_DATA_CHANNEL_WRITER,
    /*Represents the data channel reader object kind */
    IN_OBJECT_KIND_DATA_CHANNEL_READER,
    /* Always the last label, to allow determination of the number of labels*/
    /*Represents the simple discovery protocol channel object kind */
    IN_OBJECT_KIND_SDP_CHANNEL,
    /*Represents the simple discovery protocol writer object kind */
    IN_OBJECT_KIND_SDP_WRITER,
    /*Represents the simple discovery protocol reader object kind */
    IN_OBJECT_KIND_SDP_READER,
    /*Represents the interface to the kernel for channels*/
    IN_OBJECT_KIND_PLUG_KERNEL,

    /*Represents the data discovered about peer participants*/
    IN_OBJECT_KIND_DISCOVERED_PARTICIPANT_DATA,
    /*Represents the data discovered about peer writers*/
    IN_OBJECT_KIND_DISCOVERED_WRITER_DATA,
    /*Represents the data discovered about peer readers*/
    IN_OBJECT_KIND_DISCOVERED_READER_DATA,
    /*Represents the own proxy info being populated to other peers */
    IN_OBJECT_KIND_ENDPOINT_DISCOVERY_DATA,
    /*Represents the socket object */
    IN_OBJECT_KIND_SOCKET,
    /* Always the last label, to allow determination of the number of labels*/
    IN_OBJECT_KIND_COUNT
} in_objectKind;

/**
 * Declaration of the ‘in_object’ base class.
 *
 * The location of the declaration of a class depends on its scope:
 * 1) Scope 'private':
 *      In the C-file that contains the implementation of the class. For
 *      instance for the implementation of inner classes.
 *
 * 2) Scope 'protected' or 'package':
 *      In the private header file. The class can only be used by other classes
 *      in the same component as this class.
 *
 * 3)Scope 'public':
 *      In the public header file. The class can be used by all classes in all
 *      components. (Only applicable for components that form an API and not for
 *      components that form an executable).
 */
OS_CLASS(in_object);

/**
 * Declaration of the signature of a deinitialization function that must be
 * implemented for each class.
 */
typedef void (*in_objectDeinitFunc) (in_object _this);

/**
 * The definition of the ‘in_object’ class.
 *
 * The location of the definition of a class depends on the scope of its
 * members. The members can only have the same scope. If the scope of one or
 * more members must be different, a derived class should be implemented.
 *
 * 1)Scope 'private':
 *      In the C-file that contains the implementation of the class. Members
 *      can only be accessed within the class implementation itself. This
 *      is the preferred configuration.
 *
 * 2) Scope 'protected' or 'package':
 *      In the private header file. The members can be used by other classes
 *      in the same component as this class.
 *
 * 3) Scope 'public':
 *      In the public header file. The members of the class can be used by all
 *      classes in all components. (Only applicable for components that form an
 *      API and not for components that form an executable).
 */
OS_STRUCT(in_object)
{
    os_uint confidence;         /* Used to check whether the object is still valid. */
    in_objectKind kind;         /* The class of the object */
    os_uint32 refCount;         /* The reference count */
    in_objectDeinitFunc deinit; /* The class specific deinitialization function */
};

/**
 * Macro that allows the implementation of type checking when casting an
 * object. The signature of the 'casting macro' must look like this:
 *
 * #define <prefix>_<className>(_this) ((<prefix>_<className>)(_this))
 *
 * The definition of this macro allows additional functionality to be
 * implemented when casting (for instance runtime type checking in the debug
 * environment by redefining the macro with an assertion to force correctness of
 * the type of the object).
 *
 */
#define in_object(_this) ((in_object)(_this))

/**
 * The reference counting principle is very powerful. Each class has members
 * that can be instances of other classes, which are reference counted as well.
 * However, in the situation where classes reference each other, one cannot use
 * the 'default' reference counting. In that situation the reference count of
 * instances of these classes will never become 0, because the other one holds
 * the reference to that instance. In that situation there is always one 'owner'
 * and the other one has a 'back-reference' to the first one. To prevent memory
 * leakage, the 'back-reference' member must be implemented as an 'in_objectRef'
 * instead of a reference-counted object.
 *
 * Example: Consider a car that has a wheel and the wheel also belongs to a
 *          single car. The car object 'owns' the wheel. That means that the
 *          lifecycle of the wheel is coupled to the lifecycle of the car
 *          (composite relationship).
 *
 * OS_CLASS(in_car);
 *
 * OS_STRUCT(in_car)
 * {
 *     OS_EXTENDS(in_object);
 *     in_wheel wheel;
 * };
 *
 * OS_CLASS(in_wheel);
 *
 * THE FOLLOWING IS WRONG!!!
 *
 * OS_STRUCT(in_wheel)
 * {
 *     OS_EXTENDS(in_object);
 *     in_car car; //THIS WON'T WORK
 * };
 *
 * THE FOLLOWING IS RIGHT:
 *
 * OS_STRUCT(in_wheel)
 * {
 *     OS_EXTENDS(in_object);
 *     in_objectRef car;
 * };
 *
 */
typedef void* in_objectRef;


/**
 * The initializer is responsible to initialize the created object. Because the
 * initialization is split from the allocation, it is possible to create classes
 * that extend from this one and allow them to initialize all members of the
 * super class correctly.
 *
 * In case the initialization fails, all created resources must be cleaned up
 * by this function itself. First of all the resources for members of this class
 * and second also the resources of the parent. The second part must be done by
 * calling the parent deinitializer.
 *
 * The first three parameters are the same for all initializers.
 *
 * Initializes the provided object by applying the kind and storing the
 * deinitialization function. Besides that the reference counting is
 * initialized. The function will fail if the object has not been allocated yet.
 *
 * @param _this  The object to initialize. The object must be properly
 *               allocated already.
 * @param kind   The kind of the object.
 * @param deinit The deinitialization function for the provided object. This
 *               function will be called when the refCount reaches 0.
 * @return OS_TRUE if successfully initialized and OS_FALSE otherwise.
 */
os_boolean
in_objectInit(
    in_object _this,
    in_objectKind kind,
    in_objectDeinitFunc deinit);

/**
 * The deinitializer is responsible to deallocate all resource within the
 * created object, but not the object itself. After that the deinitializer must
 * call the deinitializer of the parent class. The signature of all
 * deinitializer functions are equal.
 *
 *
 * The base object does not need a deinitializer and therefore the deinitializer
 * maps to an empty macro for now. If some functionality is needed in the
 * future, it can easily be implemented here.
 *
 * @param _this The object to deinitialize.
 */
#define in_objectDeinit(_this)

/**
 * The destructor must call the destructor of the parent class. The base object
 * will call the object specific deinitializer function automatically when the
 * reference count of the object reaches 0.
 *
 * Decreases the reference count of the object by one. In case the reference
 * count reaches 0, the deinitialization function of the object is called and
 * the object is freed afterwards.
 *
 * @param _this The object to perform the operation on.
 */
void
in_objectFree(
    in_object _this);


/**
 * After the constructor, initializer, deinitializer and destructor the
 * Getters and Setters must be declared. Members of a class can be made
 * accessible by defining get and set functions. This is the desired method for
 * a clean encapsulation of the members in the class. The signature of the
 * getters and setters is always the same:
 *
 * 1)   Getter signature:
 *          <memberType>
 *          <prefix>_<className>Get<memberName>(
 *              <prefix>_<className> _this);
 *
 *          In case the 'getter' returns an instance of a class (a
 *          non-primitive),the reference count must be increased by one when
 *          returning the member! In case the member is a string, the getter
 *          must return a copy! The calling function is responsible to decrease
 *          the reference count when it no longer needs the object.
 *
 * 2)   Setter signature:
 *          void
 *          <prefix>_<className>Set<memberName>(
 *              <prefix>_<className> _this,
 *              <memberType> <memberName>);
 *
 *          In case the 'setter' function applies to a non-primitive member, it
 *          must make sure that the reference count of the object is increased
 *          by one when assigning the provided object to the member. In case of
 *          a string, the function must assign a copy of the string to the
 *          member.
 */

/**
 * Resolves the reference count from the object.
 *
 * @param _this The object to perform the operation on.
 * @return The reference count of the object or 0 in case the object is invalid.
 */
os_uint32
in_objectGetRefCount(
    in_object _this);


/**
 * Resolves the kind of the provided object.
 *
 * @param _this The object to resolve the kind of.
 * @return The kind of the object or NI_KIND_INVALID if the object is not valid.
 */
in_objectKind
in_objectGetKind(
    in_object _this);

/**
 * All other functions must be declared after the getters and setters like
 * it is done below.
 */

/**
 * Debugging function to check whether the provided object is valid. An object
 * is valid if it is allocated and correctly initialized.
 *
 * @param _this The object to check.
 * @return OS_TRUE if the object is valid. OS_FALSE otherwise.
 */
os_boolean
in_objectIsValid(
    in_object _this);

/**
 * Debugging function to check whether the provided object is valid and has
 * the expected type. The validity of the object is performed by calling
 * in_objectIsValid and if that succeeds, the type of the object is compared
 * with the provided kind.
 *
 * @param _this The object to check.
 * @param kind The expected kind of the object.
 * @return OS_TRUE if the object is valid and has the expected kind, OS_FALSE
 *         otherwise.
 */
os_boolean
in_objectIsValidWithKind(
    in_object _this,
    in_objectKind kind);

/**
 * Increases the reference count of the object by one in case the object is
 * valid.
 *
 * @param _this The object to perform the operation on.
 * @return The object that was provided as a parameter in case the action
 *         succeeds or NULL if it failed.
 */
in_object
in_objectKeep(
    in_object _this);

/**
 * There are functions to resolve the object from a reference and to
 * get a reference for an object. This allows type-checking when transforming
 * the one to the other and also allows modification of the mapping between
 * them later on. All code must use the functions to do the transformation.
 *
 * in_object    in_objectFromObjectRef(in_objectRef _this);
 *
 * in_objectRef in_objectRefFromObject(in_object _this);
 *
 * There can be different requirements at different times for the implementation
 * of a specific function. In a development environment it is important that all
 * kinds of assumptions are validated at runtime (like pre- and post conditions
 * and object validity) to be able to detect errors in an earlier stage.
 * These checks typically degrade performance of the code and won't prevent
 * problems by itself. Therefore it would be unwise to keep performing these
 * checks in a production environment, because performance is very important in
 * that situation. However, constantly adapting the source-code when switching
 * environments is not a feasible solution. Therefore the build system
 * provides a facility to support both situations in one code-base.
 *
 * The build system defines NDEBUG (no debugging) when compiling for a
 * production environment. By checking if this has been defined in the code, two
 * implementations can be made for one specific piece of functionality. For an
 * example see the in_objectFromObjectRef and in_objectRefFromObject functions
 * below.
 */

/**
 * Translates an object reference to the actual object. In a production
 * environment this maps directly to a cast for performance reasons. In the
 * development version, this maps to a function that checks the validity of
 * the object.
 *
 * @param _this The object reference to translate to an object.
 * @return The object that belongs to the object reference.
 */
#ifdef NDEBUG
#define in_objectFromObjectRef(_this) ((in_object)(_this))
#else
in_object
in_objectFromObjectRef(
    in_objectRef _this);
#endif

/**
 * Translates an object reference to the actual object. In a production
 * environment this maps directly to a cast for performance reasons. In the
 * development version, this maps to a function that checks the validity of
 * the object.
 *
 * @param _this The object reference to translate to an object.
 * @return The object that belongs to the object reference.
 */
#ifdef NDEBUG
#define in_objectRefFromObject(_this) ((in_objectRef)(_this))
#else
in_objectRef
in_objectRefFromObject(
    in_object _this);
#endif

/** Validates whether the expected amount of objects are allocated.
 *
 * @param expected The amount of expected objects.
 * @return OS_TRUE if the expected amount is equal to the actual amount.
 *         OS_FALSE otherwise.
 */
c_bool
in_objectValidate(
    c_ulong expected);

/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif

#endif /* _IN__OBJECT_H */

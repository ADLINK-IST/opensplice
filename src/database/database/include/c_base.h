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
#ifndef C_BASE_H
#define C_BASE_H

/** \file c_base.h
 *  \brief This file specifies the main database interface.
 *
 * It provides the functionality to create, destroy and lookup data in
 * the database.
 * As being the main part of the database it also provides the means to
 * create or open a database.
 *
 * The following methods are provided:
 *
 *     c_base     c_create    (const c_char *name, void *addr, c_size size);
 *     c_base     c_open      (const c_char *name, void *addr);
 *
 *     c_type     c_resolve   (c_base base, const c_char *typeName);
 *
 *     c_object   c_new       (c_type type);
 *     void       c_free      (c_object object);
 *     c_object   c_keep      (c_object object);
 *     c_type     c_getType   (c_object object);
 *     c_base     c_getBase   (c_object object);
 *     c_long     c_refCount  (c_object object);
 *
 *     c_object   c_bind      (c_object object, const c_char *name);
 *     c_object   c_unbind    (c_base base, const c_char *name);
 *     c_object   c_lookup    (c_base base, const c_char *name);
 *
 *     c_string   c_stringNew (c_base base, const c_char *str);
 *     c_array    c_arrayNew  (c_type subType, c_long size);
 *     c_long     c_arraySize (c_array a);
 */

#define c_object(o) ((c_object)(o))
#define c_base(o)   ((c_base)(o))

#include "c_misc.h"
#include "c_typebase.h"
#include "c_metabase.h"
#include "os_if.h"

#ifdef OSPL_BUILD_DB
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

#if defined (__cplusplus)
extern "C" {
#endif

/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief This operation creates a new database in the specified memory.
 *
 * The memory is specified by its address and size.
 * The name required by this operation is set in the database as an
 * identification label, processes that want to open an existing database
 * must specify the correct name otherwise no connection will be established.
 * The creation of the database will fail if the specified memory size is too
 * small. The specified memory will initially be filled with zeroes.
 *
 * \param name The name that will be associated to the created database.
 * \param addr The base address of the database memory segment.
 * \param size The size in bytes of the specified memory.
 * \param threshold The size in bytes of the specified memory threshold, i.e.,
 *                  the minimum free memory.
 *
 * \return A successful operation will return the database object,
 *         otherwise the operation will return NULL.
 */
OS_API c_base
c_create (
    const c_char *name,
    void *addr,
    c_size size,
    c_size threshold);

/**
 * \brief This operation opens an existing database.
 *
 * The database is identified by its memory location and its name.
 * This operation will verify if the specified name matches the database
 * name as a precondition for opening the database.
 * This operation will fail if the name cannot be found in the specified
 * address region.
 *
 * \param name The name that will be associated to the created database.
 * \param addr The base address of the database memory segment.
 *
 * \return A successful operation will return the database object,
 *         otherwise the operation will return NULL.
 */
OS_API c_base
c_open (
    const c_char *name,
    void *addr);

/**
 * \brief This operation will resolve a meta description of a type from
 *        the database.
 *
 * This operation will search for the type description specified by the
 * given name within the Metadata repository of the specified database.
 * If a type definition is found for the specified name this operation will
 * return a new reference to the found definition otherwise NULL will be
 * returned. This convenience operation specialized to resolve type
 * definitions known by the database. The Metadata repository API can also
 * resolve these types, the database itself is a Metadata module that
 * implements a container for all Metadata objects and via the Metadata
 * repository API the same search operation can be performed on the database.
 *
 * \param _this The database object from which the meta data must be resolved.
 * \param typeName The type name that specifies the required mete data object.
 *
 * \return The metadata object or NULL in case the specified type name is
 *         unknown by the specified database.
 */
OS_API c_type
c_resolve (
    c_base _this,
    const c_char *typeName);

/**
 * \brief This operation creates an object of the specified type.
 *
 * This operation creates an object of the specified type.
 * The type itself, as all objects, has a reference to the database that
 * is used by the operation to determine the database in which the object
 * shall be created.
 * The memory of the created object is initialized with zeroes.
 *
 * \param type is a reference to a database specific meta type description.
 *
 * \return A successful operation returns the created object.
 *         otherwise if an illegal type is specified or there are not enough
 *         resources this operation will fail and return NULL.
 */
OS_API c_object
c_new (
    c_type type);

OS_API c_memoryThreshold
c_baseGetMemThresholdStatus(
    c_base _this);

/* c_new() method specificly for arrays and sequences (only).
 *
 * Use the macro definitions c_newArray and c_newSequence instead, respectively
 * for creating an array or a sequence for legibility.
 *
 * This operation either creates an array or a sequence object of the specified
 * type. The type itself, as all objects, has a reference to the database that
 * is used by the operation to determine the database in which the object
 * shall be created.
 * The memory of the created object is initialized with zeroes.
 * \param type is a reference to a database specific meta type description.
 * \size the size of the array or sequence
 *
 * \return A successful operation returns the created object.
 *         otherwise if an illegal type is specified or there are not enough
 *         resources this operation will fail and return NULL.
 */
OS_API c_object
c_newBaseArrayObject (
    c_collectionType arrayType,
    c_long size);

#define c_newArray(t,s)         c_newBaseArrayObject(t,s); assert(c_collectionTypeKind(t) == C_ARRAY)
#define c_newSequence(t,s)      c_newBaseArrayObject(t,s); assert(c_collectionTypeKind(t) == C_SEQUENCE)

/**
 * \brief This operation notifies the database that the application will no
 *        longer use the specified object reference.
 *
 * The database will decrease the internal reference count of the object and
 * if when the last reference is removed the database will free all resources.
 *
 * \param object The object reference that is of no use anymore.
 */
#ifdef __DA_
OS_API c_long
c___free (
    c_object object);
#define c_free(o) (o?((pa_decrement(&c_header(o)->refCount) == 0)?c___free(o):0):0)
#else
OS_API void
c_free (
    c_object object);
#endif
/**
 * \brief This operation will inform the database to increase the internal
 *        reference count for the specified object.
 *
 * To be able to track all references the database needs to be informed about
 * every assignment to objects managed by the database.
 * In some languages this can be performed by overloading the functionality
 * of the assignment operator, however in a language such as C the application
 * must actively inform the database about every assignment.
 * This operation will inform the database to increase the internal reference
 * count for the specified object; this method returns the specified object
 * making it easily invoke-able in combination with assignment operations.
 *
 * \param object The object reference that will be increased.
 *
 * \return The specified object.
 */
OS_API c_object
c_keep (
    c_object object);

/**
 * \brief This operation will return the metadata that describes the type of
 *        the object it operates on.
 *
 * This operation will return a metadata object of class c_type that is
 * accessible via the Metadata repository API.
 * Note: although in general all methods returning database object will
 * increase the internal reference count this operation will not increase
 * the reference count. This may seem awkward but it is considered that the
 * returned information is in general only required during the lifetime of
 * the object and in that case the type information will stay referenced via
 * the object.
 *
 * \param object The object it operates on.
 *
 * \return The type that describes the given object.
 */
OS_API c_type
c_getType (
    c_object object);

/**
 * \brief This operation will return the database object in which the
 *        specified object is located.
 *
 * This operation will return the database object in which the specified
 * object is located.
 * Note: although in general all methods returning database object will
 * increase the internal reference count this operation will not increase
 * the reference count. This may seem awkward but it is considered that the
 * database will never be destroyed if information located within the
 * database is still required so in this case reference counting is
 * superfluous.
 *
 * \param object The object it operates on.
 *
 * \return The database in which the given object resides.
 */
OS_API c_base
c_getBase (
    c_object object);

/**
 * \brief This operation will return the actual reference count to the
 *        specified object.
 *
 * This operation will return the actual reference count to the specified
 * object.
 *
 * \param object The object it operates on.
 *
 * \return The actual number of references to the given object.
 */
#ifdef __DA_
#define c_refCount(o) c__refCount(o)
#else
OS_API c_long
c_refCount (
    c_object object);
#endif
/**
 * \brief This operation binds the given object to the specified name.
 *
 * This operation requests the database to manage a reference to the specified
 * object under the given name. If this operation succeeds the database will
 * keep a reference to the object and thereby keeping it alive even without
 * the need of applications referencing the object. It is allowed to bind an
 * object multiple times.
 *
 * \param object The object it operates on.
 * \param name The specified name that must be bind to the object.
 *
 * \return On a successful operation this operation will return the specified
 *         object.
 *         The operation will fail if the specified name is already used or in
 *         case of insufficient resources, in those cases this operation will
 *         respectively
 *         return the currently bound object or NULL.
 */
OS_API c_object
c_bind (
    c_object object,
    const c_char *name);

/**
 * \brief This operation requests the database to stop maintaining the
 *        specified named object reference.
 *
 * This operation requests the database to stop maintaining the specified
 * named object reference.
 *
 * \param object The object it operates on.
 *
 * \return A successful operation will return the unbound object and the name
 *         is free to be reused. If the specified name is unknown to the
 *         database this operation will return NULL.
 */
OS_API c_object
c_unbind (
    c_base _this,
    const c_char *name);

/**
 * \brief This operation will search the database for a named object.
 *
 * This operation will perform a search action for a named object reference
 * maintained by the specified database by the given name. On a successful
 * operation a new reference to object found is created and returned,
 * otherwise if no reference is found this operation will return NULL.
 *
 * \param _this The database on which this operation operates.
 * \param name  The name of the object that is requested.
 *
 * \return On a successful operation a new reference to object found is
 *         created and returned. Otherwise if no object is found this
 *         operation will return NULL.
 */
OS_API c_object
c_lookup (
    c_base _this,
    const c_char *name);

/**
 * \brief This operation will create a new database string object and copy
 *        the value of the given string.
 *
 * This operation will create a new database string object and copy the value
 * of the given string.
 *
 * \param _this The database in which the new string object must reside.
 * \param str   The string value that must be assigned to the created string
 *              object.
 *
 * \return On a successful operation a reference to the created string object
 *         is returned. Otherwise NULL is returned.
 */
OS_API c_string
c_stringNew (
    c_base _this,
    const c_char *str);

/**
 * \brief This operation will create a new database string object of the
 *        specified length.
 *
 * This operation will create a new database string of the specified length.
 *
 * \param _this The database in which the new string object must reside.
 * \param length The string length that must be allocated.
 *
 * \return On a successful operation a reference to the created string object
 *         is returned. Otherwise NULL is returned.
 */
OS_API c_string
c_stringMalloc(
    c_base base,
    c_long length);

/**
 * \brief 	This operation behaves the same as c_arrayNew, with the exeption that
 * 			it will always return an object with valid header.
 *
 * This operation is provided by the database primarily to be able to serialize
 * sequences\arrays with a zero length, where c_arrayNew would return a NULL object.
 *
 * \param subType The element type of the array that must be created.
 * \param size The number of elements of the array that must be created.
 *
 * \return On a successful operation a reference to the created array is
 *         returned. Otherwise NULL is returned.
 */
OS_API c_array
c_arrayNew_w_header(
    c_collectionType arrayType,
    c_long length);

OS_API typedef void (*c_baseOutOfMemoryAction)(c_voidp arg);

OS_API void
c_baseOnOutOfMemory(
    c_base base,
    c_baseOutOfMemoryAction action,
    c_voidp arg);

OS_API typedef void (*c_baseLowOnMemoryAction)(c_voidp arg);

OS_API void
c_baseOnLowOnMemory(
    c_base base,
    c_baseLowOnMemoryAction action,
    c_voidp arg);

OS_API void
c_destroy (
    c_base _this);

OS_API c_type c_voidp_t     (c_base _this);
OS_API c_type c_address_t   (c_base _this);
OS_API c_type c_object_t    (c_base _this);
OS_API c_type c_bool_t      (c_base _this);
OS_API c_type c_octet_t     (c_base _this);
OS_API c_type c_char_t      (c_base _this);
OS_API c_type c_short_t     (c_base _this);
OS_API c_type c_long_t      (c_base _this);
OS_API c_type c_longlong_t  (c_base _this);
OS_API c_type c_uchar_t     (c_base _this);
OS_API c_type c_ushort_t    (c_base _this);
OS_API c_type c_ulong_t     (c_base _this);
OS_API c_type c_ulonglong_t (c_base _this);
OS_API c_type c_float_t     (c_base _this);
OS_API c_type c_double_t    (c_base _this);
OS_API c_type c_array_t     (c_base _this);
OS_API c_type c_string_t    (c_base _this);
OS_API c_type c_wchar_t     (c_base _this);
OS_API c_type c_wstring_t   (c_base _this);
OS_API c_type c_type_t      (c_base _this);
OS_API c_type c_valueKind_t (c_base _this);

OS_API void c_baseSerLock   (c_base _this);
OS_API void c_baseSerUnlock (c_base _this);

/* This operation checks if the given address is in the database
 * address space and if it is the begin address of a database object.
 * This operation will return NULL if the address is outside the
 * database address space and return the begin address of the database
 * object the ptr refers to. Note that if the ptr refers to an address
 * inside an object (so not the begin address) it will return the begin
 * addres of the object.
 */
OS_API c_object c_baseCheckPtr (c_base _this, void *ptr);

/* The following define enables the DAT tool functionality:
 * #define OBJECT_WALK
 */

#ifndef NDEBUG
#ifdef OBJECT_WALK
/* Function for walking over all objects currently present in the
 * database. Available in the -devdat version only
 */

typedef void *c_baseWalkActionArg;
typedef void (*c_baseWalkAction)(c_object object, c_baseWalkActionArg arg);

OS_API void
c_baseObjectWalk(
    c_base _this,
    c_baseWalkAction action,
    c_baseWalkActionArg arg);

#endif
#endif

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif

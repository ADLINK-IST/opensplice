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
#ifndef V_HANDLE_H
#define V_HANDLE_H

/** \file v_handle.h
 *  \brief Kernel Handle Server: manages access to registered objects.
 *
 * The Kernel HandleServer class implements a object access manager service.
 * Objects can be registered to the handle server and in return the handle
 * server will return a unique handle for this object.
 * Processes must claim this handle to gain access to the required object,
 * when access is no longer required by the process the handle must be released.
 * When an object will never be accessed anymore the handle can be deregistered.
 * Once a handle is deregistered it will become invalid and any future use will fail.
 *
 * The following methods are provided:
 *
 *    v_handleServer v_handleServerNew      (c_base base);
 *    void           v_handleServerFree     (v_handleServer s);
 *
 *    v_handle       v_handleServerRegister (v_handleServer s, c_object o);
 *    v_object       v_handleClaim          (v_handle h);
 *    void           v_handleRelease        (v_handle h);
 *    void           v_handleDeregister     (v_handle h);
 */

#include "v_kernel.h"
#include "kernelModule.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

OS_API extern const v_handle V_HANDLE_NIL;

/**
 * \brief The <code>v_handleServer</code> cast method.
 *
 * This method casts an object to a <code>v_handleServer</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_handleServer</code> or
 * one of its subclasses.
 */
#define v_handleServer(o)  (C_CAST(o,v_handleServer))
#define v_handle(o)        ((v_handle)(o))

#define v_handleIsEqual(h1,h2) \
        (h1.server != h2.server ? FALSE : \
         h1.index  != h2.index  ? FALSE : \
         h1.serial != h2.serial ? FALSE : TRUE)

#define v_handleSetNil(handle) \
        (handle).server = 0; \
        (handle).index = 0; \
        (handle).serial = 0

#define v_handleIsNil(handle) \
        (((handle).server == 0) && \
         ((handle).index == 0) && \
         ((handle).serial == 0))

#define v_handleServerId(handle) (handle).server

/**
 * \brief The HandleServer constructor.
 *
 * This contructor will create a new handle server instance in the shared
 * database.
 *
 * \param base The database object in which the handle server will be created.
 *
 * \return a reference to the newly created handle server upon a successful
 *         operation or NULL in case an error has occured.
 */
OS_API v_handleServer
v_handleServerNew (
    c_base base);


/**
 * \brief The HandleServer destructor.
 *
 * This method will free all resources related to the given HandleServer object.
 *
 * \param _this The given HandleServer object that must be destroyed.
 **
OS_API void
v_handleServerFree(
    v_handleServer _this);
 */

/**
 * \brief The register method that creates a new Handle for a specified object.
 *
 * This method will register the given object and create a new unique handle
 * for it.
 * A reference to the given object is stored in the HandleServer keeping the
 * object alive.
 * The created handle can be used to claim and release access to the registered
 * object.
 *
 * \param _this The HandleServer where the specified object will be registered.
 * \param o     The object that will be registered at the specified HandleServer.
 *
 * \return The created handle,
 *         Note: the handle is implemented as a struct and is returned by value.
 */
OS_API v_handle
v_handleServerRegister(
    v_handleServer _this,
    c_object o);

/**
 * \brief The handle claim method.
 *
 * This method will claim access to the object associated to the given handle.
 *
 * \param _this The handle which specifies the required object.
 * \param o An out parameter which will contain a reference to the required
 *          object if the handle is valid.
 * \return The result indicates the state of the supplied handle.
 *         The reference count is NOT increased by this call,
 *         only a claim count is maintained.
 *         If the object associated to the given handle no longer exists
 *         this method will return NULL indicating the handle is not valid.
 */
typedef enum {
    V_HANDLE_OK,      /* The handle is valid */
    V_HANDLE_EXPIRED, /* The handle was valid once but is not valid anymore */
    V_HANDLE_ILLEGAL, /* The handle is bogus */
    V_HANDLE_SUSPENDED/* The handle server is suspended. */
} v_handleResult;
 
OS_API v_handleResult
v_handleClaim(
    v_handle _this,
    v_object *o);

/**
 * \brief The handle release method.
 *
 * This method will release a previous claimed handle and thereby notifying
 * the HandleServer that the object associated to this handle will not be
 * accessed anymore.
 * 
 * \param _this The handle which specifies the required object.
 * \return  The result indicates the state of the supplied handle
 */
OS_API v_handleResult
v_handleRelease(
    v_handle _this);

/**
 * \brief The handle deregister method.
 *
 * This method deregisteres a handle from the specified HandleServer.
 * The handle will become invalid for future use.
 * Any attempt to claim this handle will fail.
 * Depending if any claims exist the associated object will be dereferenced,
 * in case claims exist the dereferencing of
 * the object is delayed until the last claim is released.
 *
 * \param _this The handle which specifies the required object.
 * \return  The result indicates the state of the supplied handle
 */
OS_API v_handleResult
v_handleDeregister(
    v_handle _this);

/**
 * \brief Future extensions.
 *
 * The following methods are foreseen future extensions but not implemented yet.
 */
OS_API c_long
v_handleServerCount (
    v_handleServer _this);

OS_API c_long
v_handleServerClaims (
    v_handleServer _this);

OS_API c_iter
v_handleServerLookup (
    v_handleServer _this,
    c_object o);

OS_API void
v_handleServerSuspend (
    v_handleServer _this);

OS_API void
v_handleServerResume (
    v_handleServer _this);


#if defined (__cplusplus)
}
#endif

#undef OS_API

#endif /* ifndef V_HANDLE_H */

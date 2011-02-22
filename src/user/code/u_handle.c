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
#include "u__handle.h"
#include "u__types.h"
#include "u__domain.h"
#include "u__user.h"
#include "v_public.h"

/* The U_HANDLE_SERIAL_MASK defines the bits in the highest 4 bytes
 * of a handle that represents the handle lifecycle number.
 * The U_HANDLE_ID_MASK defines the bits in the highest 4 bytes
 * of a handle that represents the kernel id.
 * The lower 4 bytes of the handle represents the handle id.
 */
#define U_HANDLE_SERIAL_MASK (0x00ffffff)
#define U_HANDLE_ID_MASK     (0x7f000000)   /* Max 127 kernels per process. */
                                            /* MSB reserved for GID use.    */

/* Helper Union to transformate handle into 2 integers and visa versa.
 */
typedef union {
    struct {
        c_long globalId;
        c_long localId;
    } lid;
    u_handle handle;
} u_handleTranslator;

const u_handle U_HANDLE_NIL = 0;

u_handle
u_handleNew(
    v_public object)
{
    v_handle handle;
    u_handleTranslator translator;
    c_long id;

    if (object) {
        handle = v_publicHandle(object);
        id = u_userServerId(object);
        translator.lid.globalId = (handle.serial | id);
        translator.lid.localId = handle.index;
    } else {
        translator.handle = 0;
    }
    return translator.handle;
}
    
u_result
u_handleClaim (
    u_handle _this,
    c_voidp  instance)
{
    u_result result;
    v_handle handle;
    u_handleTranslator translator;

    if (instance == NULL) {
        result = U_RESULT_ILL_PARAM;
    } else if (_this == 0) {
        result = U_RESULT_ILL_PARAM;
    } else {
        translator.handle = _this;

        handle.serial = (translator.lid.globalId & U_HANDLE_SERIAL_MASK);
        handle.index  = translator.lid.localId;
        /* The u_userServer method will verify liveliness
         * and return the handle server.
         * If NOT alive U_HANDLE_NIL is returned and the
         * following v_handle claim will fail.
         */
        handle.server = u_userServer(translator.lid.globalId & U_HANDLE_ID_MASK);

        result = u__handleResult(v_handleClaim(handle,instance));
    }

    return result;
}

u_result
u_handleRelease(
    u_handle _this)
{
    u_result result;
    v_handle handle;
    u_handleTranslator translator;

    if (_this == 0) {
        result = U_RESULT_ILL_PARAM;
    } else {
        translator.handle = _this;

        handle.serial = (translator.lid.globalId & U_HANDLE_SERIAL_MASK);
        handle.index  = translator.lid.localId;
        handle.server = u_userServer(translator.lid.globalId & U_HANDLE_ID_MASK);

        result = u__handleResult(v_handleRelease(handle));
    }

    return result;
}

c_bool
u_handleIsEqual(
    u_handle h1,
    u_handle h2)
{
    return (h1 == h2);
}

c_bool
u_handleIsNil(
    u_handle _this)
{
    return (_this == U_HANDLE_NIL);
}


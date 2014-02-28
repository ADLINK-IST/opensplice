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
#include "u__types.h"
#include "u__domain.h"
#include "u__user.h"
#include "v_public.h"

const u_handle U_HANDLE_NIL;

static int u__handleResult (v_handleResult result)
{
    return ((result == V_HANDLE_OK) ? U_RESULT_OK :
            (result == V_HANDLE_EXPIRED) ? U_RESULT_ALREADY_DELETED :
            U_RESULT_ILL_PARAM);
}

u_handle
u_handleNew(
    v_public object)
{
    v_handle handle;
    assert (object != NULL);
    handle = v_publicHandle(object);
    return handle;
}

u_result
u_handleClaim (
    u_handle _this,
    c_voidp  instance)
{
    return u__handleResult (v_handleClaim (_this, instance));
}

u_result
u_handleRelease(
    u_handle _this)
{
    return u__handleResult (v_handleRelease (_this));
}

c_bool
u_handleIsEqual(
    u_handle h1,
    u_handle h2)
{
    return v_handleIsEqual (h1, h2);
}

c_bool
u_handleIsNil(
    u_handle h)
{
    return v_handleIsNil (h);
}

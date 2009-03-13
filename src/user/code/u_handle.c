#include "u_handle.h"
#include "u__types.h"
#include "u__kernel.h"
#include "u__user.h"

C_CLASS(checkHandleArg);
C_STRUCT(checkHandleArg) {
    c_long    server;
    u_result *result;
};

const u_handle U_HANDLE_NIL = {0, 0, 0};

static void
checkHandle(
    u_kernel kernel,
    c_voidp arg)
{
    checkHandleArg a = (checkHandleArg)arg;

    if (u_kernelCheckHandleServer(kernel,a->server)) {
        *(a->result) = U_RESULT_OK;
    }
}
    
u_result
u_handleClaim (
    u_handle handle,
    c_voidp  instance)
{
    C_STRUCT(checkHandleArg) arg;
    u_result result = U_RESULT_UNDEFINED;

    arg.server = v_handleServerId(handle);
    arg.result = &result;
    u_userWalk(checkHandle,&arg);
    if (result == U_RESULT_OK) {
        result = u_handleResult(v_handleClaim(handle,instance));
    } else {
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_handleRelease(
    u_handle handle)
{
    C_STRUCT(checkHandleArg) arg;
    u_result result = U_RESULT_UNDEFINED;

    arg.server = v_handleServerId(handle);
    arg.result = &result;
    u_userWalk(checkHandle,&arg);
    if (result == U_RESULT_OK) {
        result = u_handleResult(v_handleRelease(handle));
    } else {
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}


#ifndef U_HANDLE_H
#define U_HANDLE_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "u_types.h"
#include "v_handle.h"
#include "os_if.h"

#ifdef OSPL_BUILD_USER
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

typedef v_handle u_handle;

OS_API extern const u_handle U_HANDLE_NIL;

#define u_handleResult(result) \
        ((result == V_HANDLE_OK) ? U_RESULT_OK : \
         (result == V_HANDLE_EXPIRED) ? U_RESULT_PRECONDITION_NOT_MET : \
                                        U_RESULT_ILL_PARAM)

OS_API u_result u_handleClaim   (u_handle handle, c_voidp instance);
OS_API u_result u_handleRelease (u_handle handle);

#define u_handleClaimUnsafe(h,i) u_handleResult(v_handleClaim((h),(i)))
#define u_handleReleaseUnsafe(h) u_handleResult(v_handleRelease((h)))

#define u_handleIsEqual(h1,h2) v_handleIsEqual((h1),(h2))
#define u_handleIsNil(handle)  v_handleIsNil(handle)
#define u_handleSetNil(handle) v_handleSetNil(handle)

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif

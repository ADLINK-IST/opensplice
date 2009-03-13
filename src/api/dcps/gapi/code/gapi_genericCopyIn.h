#ifndef GAPI_COPYIN_H
#define GAPI_COPYIN_H

#include "c_typebase.h"

#include "gapi_genericCopyCache.h"
#include "os_if.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_DCPSGAPI
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

C_CLASS(gapi_srcInfo);

C_STRUCT(gapi_srcInfo) {
    void * src;
    gapi_copyCache copyProgram;
};

OS_API gapi_boolean
gapi_copyInStruct (
    c_base base,
    void *src,
    void *dst);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif

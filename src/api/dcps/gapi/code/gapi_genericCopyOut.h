#ifndef GAPI_COPYOUT_H
#define GAPI_COPYOUT_H

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

C_CLASS(gapi_dstInfo);

C_STRUCT(gapi_dstInfo) {
    void * dst;
    gapi_copyCache copyProgram;
    void * buf;
};

OS_API void
gapi_copyOutStruct (
    void *src,
    void *dst);

OS_API void *
gapi_copyOutAllocBuffer (
    gapi_copyCache copyCache,
    gapi_unsigned_long len);

/*
 * This macro inserts the copycache in a header,
 * if we are using the generic copy routines
 */
#define PREPEND_COPYOUTCACHE(cc,d,b) \
        if (cc){ \
            void * tmp = d; \
            d = os_malloc(C_SIZEOF(gapi_dstInfo)); \
            ((gapi_dstInfo)d)->dst = tmp; \
            ((gapi_dstInfo)d)->copyProgram = cc; \
            ((gapi_dstInfo)d)->buf = b; \
        }
/*
 * This macro removes the copycache from a header,
 * if we are using the generic copy routines
 */
#define REMOVE_COPYOUTCACHE(cc,d) \
        if (cc){ \
            void * tmp = ((gapi_dstInfo)d)->dst; \
            os_free(d); \
            d = tmp; \
        }        


#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif

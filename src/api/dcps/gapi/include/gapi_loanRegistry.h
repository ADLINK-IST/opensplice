#ifndef GAPI_LOANREGISTRY_H
#define GAPI_LOANREGISTRY_H

#include "gapi.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_DCPSGAPI
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

C_CLASS(gapi_loanRegistry);

OS_API gapi_loanRegistry
gapi_loanRegistry_new (
    void);

OS_API gapi_returnCode_t
gapi_loanRegistry_free (
    gapi_loanRegistry _this);

OS_API gapi_returnCode_t
gapi_loanRegistry_register (
    gapi_loanRegistry _this,
    void *data_buffer,
    void *info_buffer);

OS_API gapi_returnCode_t
gapi_loanRegistry_deregister (
    gapi_loanRegistry _this,
    void *data_buffer,
    void *info_buffer);

OS_API gapi_boolean
gapi_loanRegistry_is_loan (
    gapi_loanRegistry _this,
    void *data_buffer,
    void *info_buffer);

OS_API gapi_boolean
gapi_loanRegistry_is_empty (
    gapi_loanRegistry _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif

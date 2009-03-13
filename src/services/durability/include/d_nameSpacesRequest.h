
#ifndef D_NAMESPACESREQUEST_H
#define D_NAMESPACESREQUEST_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_nameSpacesRequest(s) ((d_nameSpacesRequest)(s))

d_nameSpacesRequest d_nameSpacesRequestNew  (d_admin admin);

void                d_nameSpacesRequestFree  (d_nameSpacesRequest request);

#if defined (__cplusplus)
}
#endif

#endif /*D_NAMESPACESREQUEST_H*/

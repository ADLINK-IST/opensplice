#ifndef GAPI_FOOTYPESUPPORT_H
#define GAPI_FOOTYPESUPPORT_H

#include "gapi_common.h"

#define _FooTypeSupport(o) ((_FooTypeSupport)(o))

#define gapi_fooTypeSupportClaim(h,r) \
        (_FooTypeSupport(gapi_objectClaim(h,OBJECT_KIND_FOOTYPESUPPORT,r)))

#define gapi_fooTypeSupportClaimNB(h,r) \
        (_FooTypeSupport(gapi_objectClaimNB(h,OBJECT_KIND_FOOTYPESUPPORT,r)))

#endif

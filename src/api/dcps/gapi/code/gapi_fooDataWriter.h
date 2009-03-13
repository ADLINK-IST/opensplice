#ifndef GAPI_FOODATAWRITER_H
#define GAPI_FOODATAWRITER_H

#include "gapi.h"
#include "gapi_common.h"
#include "gapi_dataWriter.h"

#define _FooDataWriter(o) ((_FooDataWriter)(o))

#define gapi_fooDataWriterClaim(h,r) \
        (_FooDataWriter(gapi_objectClaim(h,OBJECT_KIND_FOODATAWRITER,r)))

#define gapi_fooDataWriterClaimNB(h,r) \
        (_FooDataWriter(gapi_objectClaimNB(h,OBJECT_KIND_FOODATAWRITER,r)))

#endif

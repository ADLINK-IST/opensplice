#ifndef GAPI_FOODATAVIEW_H
#define GAPI_FOODATAVIEW_H

#include "gapi.h"
#include "gapi_common.h"
#include "gapi_dataReaderView.h"

#define _FooDataReaderView(o) ((_FooDataReaderView)(o))

#define gapi_fooDataReaderViewClaim(h,r) \
        (_FooDataReaderView(gapi_objectClaim(h,OBJECT_KIND_FOODATAVIEW,r)))

#define gapi_fooDataReaderViewClaimNB(h,r) \
        (_FooDataReaderView(gapi_objectClaimNB(h,OBJECT_KIND_FOODATAVIEW,r)))

#endif

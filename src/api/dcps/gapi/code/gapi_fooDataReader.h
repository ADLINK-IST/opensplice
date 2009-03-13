#ifndef GAPI_FOODATAREADER_H
#define GAPI_FOODATAREADER_H

#include "gapi.h"
#include "gapi_common.h"
#include "gapi_dataReader.h"

#define _FooDataReader(o) ((_FooDataReader)(o))

#define gapi_fooDataReaderClaim(h,r) \
        (_FooDataReader(gapi_objectClaim(h,OBJECT_KIND_FOODATAREADER,r)))

#define gapi_fooDataReaderClaimNB(h,r) \
        (_FooDataReader(gapi_objectClaimNB(h,OBJECT_KIND_FOODATAREADER,r)))

#endif

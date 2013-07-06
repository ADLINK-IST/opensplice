/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
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

c_bool gapi_matchesReaderMask (c_object o, c_voidp args);

#endif

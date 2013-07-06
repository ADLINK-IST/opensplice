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

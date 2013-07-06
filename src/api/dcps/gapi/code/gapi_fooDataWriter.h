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

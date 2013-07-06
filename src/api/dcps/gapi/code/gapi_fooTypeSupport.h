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
#ifndef GAPI_FOOTYPESUPPORT_H
#define GAPI_FOOTYPESUPPORT_H

#include "gapi_common.h"

#define _FooTypeSupport(o) ((_FooTypeSupport)(o))

#define gapi_fooTypeSupportClaim(h,r) \
        (_FooTypeSupport(gapi_objectClaim(h,OBJECT_KIND_FOOTYPESUPPORT,r)))

#define gapi_fooTypeSupportClaimNB(h,r) \
        (_FooTypeSupport(gapi_objectClaimNB(h,OBJECT_KIND_FOOTYPESUPPORT,r)))

#endif

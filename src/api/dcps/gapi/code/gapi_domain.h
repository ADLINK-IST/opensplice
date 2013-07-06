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
#ifndef GAPI_DOMAIN_H
#define GAPI_DOMAIN_H

#include "gapi_common.h"
#include "gapi_object.h"

#include "u_user.h"

C_CLASS(_Domain);

#define _Domain(o) ((_Domain)(o))

#define _DomainAlloc()                              \
        (_Domain(_ObjectAlloc(OBJECT_KIND_DOMAIN,   \
        C_SIZEOF(_Domain),                          \
        NULL)))

#define gapi_domainClaim(h,r) \
        (_Domain(gapi_objectClaim(h,OBJECT_KIND_DOMAIN,r)))

#define gapi_domainClaimNB(h,r) \
        (_Domain(gapi_objectClaimNB(h,OBJECT_KIND_DOMAIN,r)))

_Domain
_DomainNew(
    gapi_domainName_t domainId);

void
_DomainFree(
    _Domain _this);

u_domain
_DomainGetKernel(
     _Domain _this);

#endif

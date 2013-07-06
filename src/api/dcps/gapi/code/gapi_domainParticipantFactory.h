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
#ifndef GAPI_DOMAINPARTICIPANTFACTORY_H
#define GAPI_DOMAINPARTICIPANTFACTORY_H

#include "gapi_common.h"
#include "gapi_object.h"

#define _DomainParticipantFactory(o) ((_DomainParticipantFactory)(o))

#define INVALID_DOMAIN_ID 0x7fffffff /* should match the DOMAIN_ID_DEFAULT global */

#define gapi_domainParticipantFactoryClaim(h,r) \
        (_DomainParticipantFactory(gapi_objectClaim(h,OBJECT_KIND_DOMAINPARTICIPANTFACTORY,r)))

#define _DomainParticipantFactoryAlloc() \
        ((_DomainParticipantFactory)_ObjectAlloc(OBJECT_KIND_DOMAINPARTICIPANTFACTORY, \
                                                 C_SIZEOF(_DomainParticipantFactory), \
                                                 NULL))

void
_DomainParticipantFactoryRegister (
    _Object object);

_DomainParticipant
_DomainParticipantFactoryFindParticipantFromType (
    _TypeSupport typeSupport);

gapi_boolean
_DomainParticipantFactoryIsContentSubscriptionAvailable(
    void);

#endif /* GAPI_DOMAINPARTICIPANTFACTORY_H */

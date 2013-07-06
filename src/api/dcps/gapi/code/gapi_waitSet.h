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
#ifndef GAPI_WAITSET_H
#define GAPI_WAITSET_H

#include "gapi_common.h"
#include "gapi_domainParticipant.h"
#include "gapi_condition.h"
#include "c_iterator.h"

#define _WaitSet(o) ((_WaitSet)(o))

#define gapi_waitSetClaim(h,r) \
        (_WaitSet(gapi_objectClaim(h,OBJECT_KIND_WAITSET,r)))

#define gapi_waitSetClaimNB(h,r) \
        (_WaitSet(gapi_objectClaimNB(h,OBJECT_KIND_WAITSET,r)))

#define _WaitSetFromHandle(h) \
        (_WaitSet(gapi_objectPeek(h,OBJECT_KIND_WAITSET)))

#define _WaitSetAlloc() \
        ((_WaitSet)_ObjectAlloc(OBJECT_KIND_WAITSET, \
                                C_SIZEOF(_WaitSet), \
                                _WaitSetFree))

C_STRUCT(_WaitSet) {
    C_EXTENDS(_Object);
    gapi_boolean busy;
    gapi_boolean multidomain;
    os_mutex     mutex;
    os_cond      cv;
    c_voidp      conditions;
    c_long       length;
    c_iter       domains;
};

_WaitSet
_WaitSetNew(void);

gapi_boolean
_WaitSetFree(
    void *_waitset);

void
_WaitSetNotify(
    _WaitSet _this,
    _Condition cond);

void
_WaitSetDetachParticipant(
    _WaitSet           _this,
    _DomainParticipant participant);

#endif

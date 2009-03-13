#ifndef GAPI_SUBSCRIBERSTATUS_H
#define GAPI_SUBSCRIBERSTATUS_H

#include "gapi_common.h"
#include "gapi_status.h"

#define _SubscriberStatusAlloc() \
        ((_SubscriberStatus)_ObjectAlloc(OBJECT_KIND_SUBSCRIBER_STATUS, \
                                         C_SIZEOF(_SubscriberStatus), \
                                         NULL))

C_CLASS(_SubscriberStatus);
#define _SubscriberStatus(o)   ((_SubscriberStatus)(o))

C_STRUCT(_SubscriberStatus) {
    C_EXTENDS(_Status);
};

_SubscriberStatus
_SubscriberStatusNew(
    _Subscriber entity,
    const struct gapi_subscriberListener *_listener,
    const gapi_statusMask mask);

void
_SubscriberStatusFree (
    _SubscriberStatus info);

gapi_boolean
_SubscriberStatusSetInterest (
    _Status _this,
    _ListenerInterestInfo _info);

gapi_boolean
_SubscriberStatusSetListener(
    _SubscriberStatus _this,
    const struct gapi_subscriberListener *_listener,
    gapi_statusMask mask);

struct gapi_subscriberListener
_SubscriberStatusGetListener(
    _SubscriberStatus _this);

#endif

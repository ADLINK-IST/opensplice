#ifndef GAPI_PUBLISHERSTATUS_H
#define GAPI_PUBLISHERSTATUS_H

#include "gapi_common.h"
#include "gapi_status.h"

#define _PublisherStatusAlloc() \
        ((_PublisherStatus)_ObjectAlloc(OBJECT_KIND_PUBLISHER_STATUS, \
                                        C_SIZEOF(_PublisherStatus), \
                                        NULL))

C_CLASS(_PublisherStatus);
#define _PublisherStatus(o) ((_PublisherStatus)(o))

C_STRUCT(_PublisherStatus) {
    C_EXTENDS(_Status);
};

_PublisherStatus
_PublisherStatusNew(
    _Publisher entity,
    const struct gapi_publisherListener *_listener,
    const gapi_statusMask mask);

void
_PublisherStatusFree (
    _PublisherStatus info);

gapi_boolean
_PublisherStatusSetInterest (
    _Status _this,
    _ListenerInterestInfo _info);

gapi_boolean
_PublisherStatusSetListener(
    _PublisherStatus _this,
    const struct gapi_publisherListener *_listener,
    gapi_statusMask mask);

struct gapi_publisherListener
_PublisherStatusGetListener(
    _PublisherStatus _this);

#endif

#ifndef GAPI_TOPICSTATUS_H
#define GAPI_TOPICSTATUS_H

#include "gapi_common.h"
#include "gapi_status.h"

#define _TopicStatusAlloc() \
        ((_TopicStatus)_ObjectAlloc(OBJECT_KIND_TOPIC_STATUS, \
                                    C_SIZEOF(_TopicStatus), \
                                    NULL))

C_CLASS(_TopicStatus);
#define _TopicStatus(o) ((_TopicStatus)(o))

C_STRUCT(_TopicStatus) {
    C_EXTENDS(_Status);
};

_TopicStatus
_TopicStatusNew (
    _Topic entity,
    const struct gapi_topicListener *_listener,
    const gapi_statusMask mask);

void
_TopicStatusFree (
    _TopicStatus info);

gapi_boolean
_TopicStatusSetListener(
    _TopicStatus _this,
    const struct gapi_topicListener *_listener,
    gapi_statusMask mask);

struct gapi_topicListener
_TopicStatusGetListener(
    _TopicStatus _this);

#endif

#ifndef GAPI_DATAWRITERSTATUS_H
#define GAPI_DATAWRITERSTATUS_H

#include "gapi_common.h"
#include "gapi_status.h"

C_CLASS(_DataWriterStatus);

C_STRUCT(_DataWriterStatus) {
    C_EXTENDS(_Status);
};

#define _DataWriterStatus(o) ((_DataWriterStatus)(o))

#define _DataWriterStatusAlloc() \
        _DataWriterStatus(_ObjectAlloc(OBJECT_KIND_WRITER_STATUS, \
                                       C_SIZEOF(_DataWriterStatus), \
                                       NULL))

_DataWriterStatus
_DataWriterStatusNew(
    _DataWriter entity,
    const struct gapi_dataWriterListener *_listener,
    const gapi_statusMask mask);

void
_DataWriterStatusFree (
    _DataWriterStatus info);

gapi_boolean
_DataWriterStatusSetListener(
    _DataWriterStatus _this,
    const struct gapi_dataWriterListener *_listener,
    gapi_statusMask mask);

struct gapi_dataWriterListener
_DataWriterStatusGetListener(
    _DataWriterStatus _this);

#endif /* GAPI_DATAWRITERSTATUS_H */

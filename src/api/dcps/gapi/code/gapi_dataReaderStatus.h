/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef GAPI_DATAREADERSTATUS_H
#define GAPI_DATAREADERSTATUS_H

#include "gapi_common.h"
#include "gapi_status.h"

#define _DataReaderStatusAlloc() \
        ((_DataReaderStatus)_ObjectAlloc(OBJECT_KIND_READER_STATUS, \
                                         C_SIZEOF(_DataReaderStatus), \
                                         NULL))

C_CLASS(_DataReaderStatus);
#define _DataReaderStatus(o)   ((_DataReaderStatus)(o))

C_STRUCT(_DataReaderStatus) {
    C_EXTENDS(_Status);
};

_DataReaderStatus
_DataReaderStatusNew(
    _DataReader entity,
    const struct gapi_dataReaderListener *_listener,
    const gapi_statusMask mask);

void
_DataReaderStatusFree (
    _DataReaderStatus info);

gapi_boolean
_DataReaderStatusSetListener(
    _DataReaderStatus _this,
    const struct gapi_dataReaderListener *_listener,
    gapi_statusMask mask);

struct gapi_dataReaderListener
_DataReaderStatusGetListener(
    _DataReaderStatus _this);

#endif /* GAPI_DATAREADERSTATUS_H */

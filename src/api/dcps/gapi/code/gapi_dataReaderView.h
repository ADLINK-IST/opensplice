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
#ifndef GAPI_DATAVIEW_H
#define GAPI_DATAVIEW_H

#include "gapi.h"
#include "gapi_common.h"
#include "gapi_entity.h"
#include "gapi_status.h"
#include "gapi_loanRegistry.h"
#include "u_dataView.h"

#define U_DATAREADERVIEW_GET(r) \
        u_dataView(U_ENTITY_GET(r))

#define U_DATAREADERVIEW_SET(r,e) \
        _EntitySetUserEntity(_Entity(r), u_entity(e))

#define _DataReaderView(o) ((_DataReaderView)(o))

#define gapi_dataReaderViewClaim(h,r) \
        (_DataReaderView(gapi_objectClaim(h,OBJECT_KIND_DATAVIEW,r)))

#define gapi_dataReaderViewClaimNB(h,r) \
        (_DataReaderView(gapi_objectClaimNB(h,OBJECT_KIND_DATAVIEW,r)))

#define _DataReaderViewAlloc() \
        (_DataReaderView(_ObjectAlloc(OBJECT_KIND_DATAVIEW, \
                                      C_SIZEOF(_DataReaderView), \
                                      NULL)))

C_STRUCT(_DataReaderView) {
    C_EXTENDS(_Entity);
   _DataReader        datareader;
    gapi_loanRegistry loanRegistry;
    gapi_readerMask   reader_mask;
};

_DataReaderView
_DataReaderViewNew (
    const gapi_dataReaderViewQos * qos,
    const _DataReader datareader);

gapi_returnCode_t
_DataReaderViewFree (
    _DataReaderView _this);

gapi_boolean
_DataReaderViewPrepareDelete (
    _DataReaderView _this,
    gapi_context *context);

_DataReader
_DataReaderViewDataReader(
    _DataReaderView _this);

u_dataView
_DataReaderViewUreaderView (
    _DataReaderView _this);

#endif

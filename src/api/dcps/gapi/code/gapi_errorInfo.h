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
#ifndef GAPI_ERRORINFO_H
#define GAPI_ERRORINFO_H

#include "gapi_common.h"
#include "gapi_object.h"

#define _ErrorInfo(o) ((_ErrorInfo)(o))

#define gapi_errorInfoClaim(h,r) \
        (_ErrorInfo(gapi_objectClaim(h,OBJECT_KIND_ERRORINFO,r)))

#define gapi_errorInfoClaimNB(h,r) \
        (_ErrorInfo(gapi_objectClaimNB(h,OBJECT_KIND_ERRORINFO,r)))

#define _ErrorInfoFromHandle(h) \
        (_ErrorInfo(gapi_objectPeek(h, OBJECT_KIND_ERRORINFO)))

#define _ErrorInfoAlloc() \
        (_ErrorInfo(_ObjectAlloc(OBJECT_KIND_ERRORINFO, \
                                 C_SIZEOF(_ErrorInfo), \
                                 _ErrorInfoFree)))

C_STRUCT(_ErrorInfo) {
    C_EXTENDS(_Object);
    gapi_boolean valid;
    gapi_errorCode_t code;
    gapi_string location;
    gapi_string source_line;
    gapi_string stack_trace;
    gapi_string message;
};


_ErrorInfo
_ErrorInfoNew (
    void);

gapi_boolean
_ErrorInfoFree (
    void * _errorInfo);

#endif

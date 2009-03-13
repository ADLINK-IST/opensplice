#ifndef GAPI_DOMAINPARTICIPANTSTATUS_H
#define GAPI_DOMAINPARTICIPANTSTATUS_H

#include "gapi_common.h"
#include "gapi_status.h"

#define _DomainParticipantStatusAlloc() \
        ((_DomainParticipantStatus)_ObjectAlloc(OBJECT_KIND_PARTICIPANT_STATUS, \
                                                C_SIZEOF(_DomainParticipantStatus), \
                                                NULL))

C_CLASS(_DomainParticipantStatus);
#define _DomainParticipantStatus(o)  ((_DomainParticipantStatus)(o))

C_STRUCT(_DomainParticipantStatus) {
    C_EXTENDS(_Status);
};

_DomainParticipantStatus
_DomainParticipantStatusNew (
    _DomainParticipant             entity,
    const struct gapi_domainParticipantListener *_listener,
    const gapi_statusMask mask
    );

void
_DomainParticipantStatusFree (
    _DomainParticipantStatus info
    );

gapi_boolean
_DomainParticipantStatusSetInterest (
    _Status               _this,
    _ListenerInterestInfo _info
    );
  
gapi_boolean
_DomainParticipantStatusSetListener(
    _DomainParticipantStatus       _this,
    const struct gapi_domainParticipantListener *_listener,
    gapi_statusMask            mask
    );
      
struct gapi_domainParticipantListener
_DomainParticipantStatusGetListener(
    _DomainParticipantStatus   _this);

#endif /* GAPI_DOMAINPARTICIPANTSTATUS_H */

/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef IN_CONNECTIVITYPEERPARTICIPANT_H_
#define IN_CONNECTIVITYPEERPARTICIPANT_H_

#include "in_commonTypes.h"
#include "in_ddsiElements.h"
#include "in_connectivityPeerEntity.h"
#include "in_result.h"
#include "in_locator.h"
#include "Coll_List.h"

/* \brief Is proxy for peer Participant */


/* The usual cast-method for class in_connectivityPeerParticipant. Note that because
 * in_connectivityPeerParticipant does not contain any metadata there is no type checking
 * performed.
 */
#define in_connectivityPeerParticipant(Participant) ((in_connectivityPeerParticipant)Participant)

#define in_connectivityPeerParticipantFree(c) in_objectFree(in_object(c))

#define in_connectivityPeerParticipantKeep(c) in_connectivityPeerParticipant(in_objectKeep(in_object(c)))

#define in_connectivityPeerParticipantIsValid(c) \
    in_objectIsValidWithKind(in_object(c), IN_OBJECT_KIND_PEER_PARTICIPANT)

in_connectivityPeerParticipant
in_connectivityPeerParticipantNew(
    in_ddsiDiscoveredParticipantData info);

Coll_List*
in_connectivityPeerParticipantGetUnicastLocators(
    in_connectivityPeerParticipant _this);

Coll_List*
in_connectivityPeerParticipantGetMulticastLocators(
    in_connectivityPeerParticipant _this);

Coll_List*
in_connectivityPeerParticipantGetDefaultUnicastLocators(
    in_connectivityPeerParticipant _this);

Coll_List*
in_connectivityPeerParticipantGetDefaultMulticastLocators(
    in_connectivityPeerParticipant _this);

in_ddsiDiscoveredParticipantData
in_connectivityPeerParticipantGetInfo(
    in_connectivityPeerParticipant _this);

in_ddsiGuidPrefixRef
in_connectivityPeerParticipantGetGuidPrefix(
    in_connectivityPeerParticipant _this);

in_ddsiSequenceNumber
in_connectivityPeerParticipantGetLastWriterSNRef(
    in_connectivityPeerParticipant _this);

in_ddsiSequenceNumber
in_connectivityPeerParticipantGetLastReaderSNRef(
    in_connectivityPeerParticipant _this);

void
in_connectivityPeerParticipantAddWriterSN(
    in_connectivityPeerParticipant _this,
    in_ddsiSequenceNumber seq);

void
in_connectivityPeerParticipantAddReaderSN(
    in_connectivityPeerParticipant _this,
    in_ddsiSequenceNumber seq);


#endif /* IN_CONNECTIVITYPEERPARTICIPANT_H_ */

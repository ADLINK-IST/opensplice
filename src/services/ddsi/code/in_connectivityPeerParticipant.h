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
/*
 * in_connectivityPeerParticipant.h
 *
 *  Created on: Oct 13, 2008
 *      Author: frehberg
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

#define in_connectivityPeerParticipantKeep(c) in_objectKeep(in_object(c))

#define in_connectivityPeerParticipantIsValid(c) \
    in_objectIsValidWithKind(in_object(c), IN_OBJECT_KIND_PEER_PARTICIPANT)

in_connectivityPeerParticipant
in_connectivityPeerParticipantNew(
    struct v_participantInfo *info);

in_result
in_connectivityPeerParticipantAddUnicastLocator(
    in_connectivityPeerParticipant _this,
    in_locator locator);

in_result
in_connectivityPeerParticipantAddMulticastLocator(
    in_connectivityPeerParticipant _this,
    in_locator locator);

Coll_List*
in_connectivityPeerParticipantGetUnicastLocators(
    in_connectivityPeerParticipant _this);

Coll_List*
in_connectivityPeerParticipantGetMulticastLocators(
    in_connectivityPeerParticipant _this);

struct v_participantInfo *
in_connectivityPeerParticipantGetInfo(
    in_connectivityPeerParticipant _this);


#endif /* IN_CONNECTIVITYPEERPARTICIPANT_H_ */

/*
 * in_connectivityParticipantFacade.h
 *
 */

#ifndef IN_CONNECTIVITYPARTICIPANTFACADE_H_
#define IN_CONNECTIVITYPARTICIPANTFACADE_H_


/* collection includes */
#include "Coll_Set.h"

#include "in_commonTypes.h"
#include "in_channel.h"
#include "in_ddsiElements.h"
#include "in_connectivityPeerParticipant.h"

#if defined (__cplusplus)
extern "C" {
#endif

/* The usual cast-method for class in_connectivityParticipantFacade. Note that because
 * in_connectivityParticipantFacade does not contain any metadata there is no type checking
 * performed.
 */
#define in_connectivityParticipantFacade(facade) ((in_connectivityParticipantFacade)facade)

/**
 * Calls the destructor of the parent class. When the reference count of the
 * base object reaches 0, the deinitializer is called automatically.
 */
#define in_connectivityParticipantFacadeFree(c) in_objectFree(in_object(c))


#define in_connectivityParticipantFacadeKeep(c) in_objectKeep(in_object(c))

#define in_connectivityParticipantFacadeIsValid(c) \
    in_objectIsValidWithKind(in_object(c), IN_OBJECT_KIND_WRITER_FACADE)

in_connectivityParticipantFacade
in_connectivityParticipantFacadeNew(
        struct v_participantInfo *participant);

in_result
in_connectivityParticipantFacadeAddMatchedPeer(
    in_connectivityParticipantFacade _this,
    in_connectivityPeerParticipant peer);

in_result
in_connectivityParticipantFacadeRemoveMatchedPeer(
    in_connectivityParticipantFacade _this,
    in_connectivityPeerParticipant peer);

struct v_participantInfo *
in_connectivityParticipantFacadeGetInfo(
    in_connectivityParticipantFacade _this);

Coll_Set*
in_connectivityParticipantFacadeGetMatchedPeerParticipants(
    in_connectivityParticipantFacade _this);

#if defined (__cplusplus)
}
#endif

#endif /* IN_CONNECTIVITYPARTICIPANTFACADE_H_ */

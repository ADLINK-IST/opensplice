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
#ifndef IN_CONNECTIVITYREADERFACADE_H_
#define IN_CONNECTIVITYREADERFACADE_H_


#include "Coll_Set.h"

#include "in_commonTypes.h"
#include "in_ddsiElements.h"
#include "in_connectivityReaderFacade.h"
#include "in_connectivityPeerWriter.h"

#if defined (__cplusplus)
extern "C" {
#endif

/* \brief mapping internal entity onto external ddsi entity  */


/* The usual cast-method for class in_connectivityReaderFacade. Note that because
 * in_connectivityReaderFacade does not contain any metadata there is no type checking
 * performed.
 */
#define in_connectivityReaderFacade(facade) ((in_connectivityReaderFacade)facade)

/**
 * Calls the destructor of the parent class. When the reference count of the
 * base object reaches 0, the deinitializer is called automatically.
 */
#define in_connectivityReaderFacadeFree(c) in_objectFree(in_object(c))

#define in_connectivityReaderFacadeKeep(c) in_connectivityReaderFacade(in_objectKeep(in_object(c)))

#define in_connectivityReaderFacadeIsValid(c) \
    in_objectIsValidWithKind(in_object(c), IN_OBJECT_KIND_READER_FACADE)

in_connectivityReaderFacade
in_connectivityReaderFacadeNew(
    struct v_subscriptionInfo *info,
    os_boolean hasKey,
    in_ddsiSequenceNumber seq,
    in_connectivityParticipantFacade  participant);

in_result
in_connectivityReaderFacadeAddMatchedPeer(
    in_connectivityReaderFacade _this,
    in_connectivityPeerWriter peer);

in_result
in_connectivityReaderFacadeRemoveMatchedPeer(
    in_connectivityReaderFacade _this,
    in_connectivityPeerWriter peer);

struct v_subscriptionInfo *
in_connectivityReaderFacadeGetInfo(
    in_connectivityReaderFacade _this);

Coll_Set*
in_connectivityReaderFacadeGetMatchedWriters(
    in_connectivityReaderFacade _this);

in_ddsiSequenceNumber
in_connectivityReaderFacadeGetSequenceNumber(
    in_connectivityReaderFacade _this);

os_uint32
in_connectivityReaderFacadeGetPartitionCount(
	in_connectivityReaderFacade _this);

os_boolean
in_connectivityReaderFacadeMatchesPeerWriter(
	in_connectivityReaderFacade _this,
	in_connectivityPeerWriter writer);

#if defined (__cplusplus)
}
#endif

#endif /* IN_CONNECTIVITYREADERFACADE_H_ */

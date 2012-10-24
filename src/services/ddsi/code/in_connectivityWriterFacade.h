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
#ifndef IN_CONNECTIVITYWRITERFACADE_H_
#define IN_CONNECTIVITYWRITERFACADE_H_

/* Abstraction layer includes */
#include "os_classbase.h"

/* collection includes */
#include "Coll_Set.h"
#include "Coll_List.h"
/* DDSi includes */
#include "in_result.h"
#include "in_connectivityPeerReader.h"
#include "in_connectivityEntityFacade.h"
#include "in__object.h"

#if defined (__cplusplus)
extern "C" {
#endif

/* \brief mapping internal entity onto external ddsi entity  */


/* The usual cast-method for class in_connectivityWriterFacade. Note that because
 * in_connectivityWriterFacade does not contain any metadata there is no type checking
 * performed.
 */
#define in_connectivityWriterFacade(facade) ((in_connectivityWriterFacade)facade)

/**
 * Calls the destructor of the parent class. When the reference count of the
 * base object reaches 0, the deinitializer is called automatically.
 */
#define in_connectivityWriterFacadeFree(c) in_objectFree(in_object(c))

#define in_connectivityWriterFacadeKeep(c) in_connectivityWriterFacade(in_objectKeep(in_object(c)))

#define in_connectivityWriterFacadeIsValid(c) \
    in_objectIsValidWithKind(in_object(c), IN_OBJECT_KIND_WRITER_FACADE)

in_connectivityWriterFacade
in_connectivityWriterFacadeNew(
    struct v_publicationInfo *info,
    os_boolean hasKey,
    in_ddsiSequenceNumber seq,
    in_connectivityParticipantFacade  participant);

in_result
in_connectivityWriterFacadeAddMatchedPeer(
    in_connectivityWriterFacade _this,
    in_connectivityPeerReader peer);

in_result
in_connectivityWriterFacadeRemoveMatchedPeer(
    in_connectivityWriterFacade _this,
    in_connectivityPeerReader peer);

struct v_publicationInfo *
in_connectivityWriterFacadeGetInfo(
    in_connectivityWriterFacade _this);

Coll_Set*
in_connectivityWriterFacadeGetMatchedReaders(
    in_connectivityWriterFacade _this);

Coll_List*
in_connectivityWriterFacadeGetLocators(
    in_connectivityWriterFacade _this);

in_ddsiSequenceNumber
in_connectivityWriterFacadeGetSequenceNumber(
    in_connectivityWriterFacade _this);

os_boolean
in_connectivityWriterFacadeMatchesPeerReader(
	in_connectivityWriterFacade _this,
	in_connectivityPeerReader reader);

os_uint32
in_connectivityWriterFacadeGetPartitionCount(
	in_connectivityWriterFacade _this);

#if defined (__cplusplus)
}
#endif

#endif /* IN_CONNECTIVITYWRITERFACADE_H_ */

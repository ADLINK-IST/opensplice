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
#ifndef IN_CONNECTIVITYPEERWRITER_H_
#define IN_CONNECTIVITYPEERWRITER_H_

#include "in_commonTypes.h"
#include "in_ddsiElements.h"
#include "in_connectivityPeerEntity.h"
#include "in_result.h"
#include "in_locator.h"
#include "Coll_List.h"

/* \brief Is proxy for peer reader */

/* The usual cast-method for class in_connectivityPeerWriter. Note that because
 * in_connectivityPeerWriter does not contain any metadata there is no type checking
 * performed.
 */
#define in_connectivityPeerWriter(reader) ((in_connectivityPeerWriter)reader)

#define in_connectivityPeerWriterFree(r) in_objectFree(in_object(r))

#define in_connectivityPeerWriterKeep(r) in_connectivityPeerWriter(in_objectKeep(in_object(r)))

#define in_connectivityPeerWriterIsValid(c) \
    in_objectIsValidWithKind(in_object(c), IN_OBJECT_KIND_PEER_WRITER)

in_connectivityPeerWriter
in_connectivityPeerWriterNew(
    in_ddsiDiscoveredWriterData info);


Coll_List*
in_connectivityPeerWriterGetUnicastLocators(
    in_connectivityPeerWriter _this);

Coll_List*
in_connectivityPeerWriterGetMulticastLocators(
    in_connectivityPeerWriter _this);

in_ddsiDiscoveredWriterData
in_connectivityPeerWriterGetInfo(
    in_connectivityPeerWriter _this);

/** return const string */
c_char*
in_connectivityPeerWriterGetTopicName(
    in_connectivityPeerWriter _this);


in_ddsiGuid
in_connectivityPeerWriterGetGuid(
    in_connectivityPeerWriter _this);

v_gid
in_connectivityPeerWriterGetGid(
    in_connectivityPeerWriter _this);


#endif /* IN_CONNECTIVITYPEERWRITER_H_ */

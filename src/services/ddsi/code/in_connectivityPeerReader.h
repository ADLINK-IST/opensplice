/*
 * in_connectivityPeerReader.h
 *
 *  Created on: Oct 13, 2008
 *      Author: frehberg
 */

#ifndef IN_CONNECTIVITYPEERREADER_H_
#define IN_CONNECTIVITYPEERREADER_H_

#include "in_commonTypes.h"
#include "in_ddsiElements.h"
#include "in_connectivityPeerEntity.h"
#include "in_result.h"
#include "in_locator.h"
#include "Coll_List.h"

#include "kernelModule.h"


/* \brief Is proxy for peer reader */


/* The usual cast-method for class in_connectivityPeerReader. Note that because
 * in_connectivityPeerReader does not contain any metadata there is no type checking
 * performed.
 */

#define in_connectivityPeerReader(reader) ((in_connectivityPeerReader)reader)

#define in_connectivityPeerReaderFree(r) in_objectFree(in_object(r))

#define in_connectivityPeerReaderKeep(r) in_objectKeep(in_object(r))

#define in_connectivityPeerReaderIsValid(c) \
    in_objectIsValidWithKind(in_object(c), IN_OBJECT_KIND_PEER_READER)

in_connectivityPeerReader
in_connectivityPeerReaderNew(
    struct v_subscriptionInfo *info,
    os_boolean requiresInlineQos);

in_result
in_connectivityPeerReaderAddUnicastLocator(
    in_connectivityPeerReader _this,
    in_locator locator);

in_result
in_connectivityPeerReaderAddMulticastLocator(
    in_connectivityPeerReader _this,
    in_locator locator);

Coll_List*
in_connectivityPeerReaderGetUnicastLocators(
    in_connectivityPeerReader _this);

Coll_List*
in_connectivityPeerReaderGetMulticastLocators(
    in_connectivityPeerReader _this);

struct v_subscriptionInfo *
in_connectivityPeerReaderGetInfo(
    in_connectivityPeerReader _this);


#endif /* IN_CONNECTIVITYPEERREADER_H_ */

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
#ifndef IN_connectivityPeerEntity_H_
#define IN_connectivityPeerEntity_H_

#include "in_commonTypes.h"
#include "in_ddsiElements.h"
#include "in__object.h"
#include "in_result.h"
#include "in_locator.h"
#include "Coll_List.h"

/* \brief Is proxy for peer entity */
OS_STRUCT(in_connectivityPeerEntity)
{
    OS_EXTENDS(in_object);
};

/* The usual cast-method for class in_connectivityPeerEntity. Note that because
 * in_connectivityPeerEntity does not contain any metadata there is no type checking
 * performed.
 */
#define in_connectivityPeerEntity(reader) ((in_connectivityPeerEntity)reader)


os_boolean
in_connectivityPeerEntityInit(
    in_connectivityPeerEntity _this,
    in_objectKind kind,
    in_objectDeinitFunc deinit);

void
in_connectivityPeerEntityDeinit(
    in_connectivityPeerEntity _this);


/** \brief Increment refcounter of in_locator object
 */
#define in_connectivityPeerEntityKeep(p) in_objectKeep(in_object(p))
/** \brief Decrement refcounter of in_locator object
 */
#define in_connectivityPeerEntityFree(p) in_objectFree(in_object(p))

Coll_List*
in_connectivityPeerEntityGetUnicastLocators(
    in_connectivityPeerEntity _this);

Coll_List*
in_connectivityPeerEntityGetMulticastLocators(
    in_connectivityPeerEntity _this);

#endif /* IN_connectivityPeerEntity_H_ */

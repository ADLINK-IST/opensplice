/*
 * in_connectivityPeerEntity.h
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
    OS_STRUCT(in_ddsiGuid) guid;
    Coll_List uniLocators;
    Coll_List multiLocators;
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

in_result
in_connectivityPeerEntitSetGuid(
    in_connectivityPeerEntity _this,
    in_ddsiGuid guid);

in_result
in_connectivityPeerEntityAddUnicastLocator(
    in_connectivityPeerEntity _this,
    in_locator locator);

in_result
in_connectivityPeerEntityAddMulticastLocator(
    in_connectivityPeerEntity _this,
    in_locator locator);

in_ddsiGuid
in_connectivityPeerEntityGetGuid(
    in_connectivityPeerEntity _this);

Coll_List*
in_connectivityPeerEntityGetUnicastLocators(
    in_connectivityPeerEntity _this);

Coll_List*
in_connectivityPeerEntityGetMulticastLocators(
    in_connectivityPeerEntity _this);



#endif /* IN_connectivityPeerEntity_H_ */

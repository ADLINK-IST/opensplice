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
#ifndef IN_CONNECTIVITYENTITY_FACADE_H_
#define IN_CONNECTIVITYENTITY_FACADE_H_

/* Abstraction layer includes */
#include "os_classbase.h"

/* DDSi includes */
#include "in__object.h"
#include "in_ddsiElements.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * The usual cast-method for class in_connectivityEntityFacade. Note that because
 * in_connectivityEntityFacade does not contain any metadata there is no type checking
 * performed.
 */
#define in_connectivityEntityFacade(facade) ((in_connectivityEntityFacade)facade)

OS_STRUCT(in_connectivityEntityFacade)
{
    OS_EXTENDS(in_object);
    os_uint32 matchedDcpsEntityCount;
    OS_STRUCT(in_ddsiGuid) id;
    os_time timestamp;
};

os_boolean
in_connectivityEntityFacadeInit(
    in_connectivityEntityFacade _this,
    in_objectKind kind,
    in_objectDeinitFunc destroy,
    in_ddsiGuid id);

void
in_connectivityEntityFacadeDeinit(
    in_object _this);

in_ddsiGuid
in_connectivityEntityFacadeGetGuid(
    in_connectivityEntityFacade _this);

os_time
in_connectivityEntityFacadeGetTimestamp(
    in_connectivityEntityFacade _this);

void
in_connectivityEntityFacadeAddMatchedEntity(
    in_connectivityEntityFacade _this);

void
n_connectivityEntityFacadeRemoveMatchedEntity(
    in_connectivityEntityFacade _this);

#if defined (__cplusplus)
}
#endif

#endif /* IN_CONNECTIVITYENTITY_FACADE_H_ */

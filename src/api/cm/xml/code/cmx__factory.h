/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
/**@file api/cm/xml/code/cmx__factory.h
 *
 * Offers internal factory routines.
 */
#ifndef CMX__FACTORY_H
#define CMX__FACTORY_H

#include "v_kernel.h"
#include "u_object.h"
#include "u_entity.h"
#include "cmx_factory.h"
#include "os_report.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define CM_XML_CONTEXT "C&M XML API"

#define CMX_TRACE(myprint)
#define CMX_STR(x)   #x
#define CMX_SHOW_DEFINE(x) CMX_TRACE(printf("%s=%s\n", #x, CMX_STR(x)))

C_CLASS(cmx_entity);

C_STRUCT(cmx_entity){
    pa_uint32_t claimCount;
    u_object uentity;
    cmx_entity participant;
};

/**
 * Resolves the matching kernel kind according to its string representation.
 *
 * @param kind The string representation of the kind.
 * @return The kind that matches the supplied string.
 */
v_kind
cmx_resolveKind (
    const c_char* kind);

/**
 * Constructs an XML string from a list of XML entity objects.
 *
 * @param xmlEntities The list of XML entity objects.
 * @param length The cumulative length of all XML entity objects in the list.
 * @return The XML representation of the supplied list.
 */
c_char*
cmx_convertToXMLList (
    c_iter xmlEntities,
    c_ulong length);

/**
 * Internal routine that registers the supplied entity with the factory. This
 * makes sure all resources will be freed after cmx_detach has been called.
 *
 * @param entity The entity to register.
 */
cmx_entity
cmx_registerObject (
    u_object object,
    cmx_entity participant);

/**
 * Internal routine that deregisters the supplied entity with the factory. This
 * is done when the entity is freed by the user of the API.
 *
 * @param entity The entity to deregister.
 * @result The entity that was deregistered or NULL if it was not in the list.
 */
void
cmx_deregisterObject(
    u_object object);

/**
 * Provides access to the mutex for access to the list of reader snapshots.
 * The locking and unlocking is the responsibility of the calling thread.
 *
 * @return The mutex.
 */
os_mutex
cmx_getReaderSnapshotMutex();

/**
 * Provides access to the mutex for access to the list of writer snapshots.
 * The locking and unlocking is the responsibility of the calling thread.
 *
 * @return The mutex.
 */
os_mutex
cmx_getWriterSnapshotMutex();

void
cmx_internalDetach();

cmx_entity
cmx_factoryClaimEntity(
    u_object object);

void
cmx_factoryReleaseEntity(
    cmx_entity entity);

#if defined (__cplusplus)
}
#endif

#endif /* CMX__FACTORY_H */

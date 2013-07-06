/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
/**@file api/cm/xml/code/cmx__factory.h
 * 
 * Offers internal factory routines.
 */
#ifndef CMX__FACTORY_H
#define CMX__FACTORY_H

#include "v_kernel.h"
#include "u_entity.h"
#include "os_report.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "cmx_factory.h"

#define CM_XML_CONTEXT "C&M XML API"

/**
 * Resolves the matching kernel kind according to its string representation.
 * 
 * @param kind The string representation of the kind.
 * @return The kind that matches the supplied string.
 */
v_kind                  cmx_resolveKind             (const c_char* kind);

/**
 * Constructs an XML string from a list of XML entity objects.
 * 
 * @param xmlEntities The list of XML entity objects.
 * @param length The cumulative length of all XML entity objects in the list.
 * @return The XML representation of the supplied list.
 */
c_char*                 cmx_convertToXMLList        (c_iter xmlEntities,
                                                     c_long length);

/**
 * Internal routine that registers the supplied entity with the factory. This
 * makes sure all resources will be freed after cmx_detach has been called.
 * 
 * @param entity The entity to register.
 */
void                    cmx_registerEntity          (u_entity entity);

/**
 * Internal routine that deregisters the supplied entity with the factory. This
 * is done when the entity is freed by the user of the API.
 * 
 * @param entity The entity to deregister.
 * @result The entity that was deregistered or NULL if it was not in the list.
 */
u_entity                cmx_deregisterEntity        (u_entity entity);

/**
 * Provides access to the mutex for access to the list of reader snapshots.
 * The locking and unlocking is the responsibility of the calling thread.
 * 
 * @return The mutex.
 */
os_mutex                cmx_getReaderSnapshotMutex  ();

/**
 * Provides access to the mutex for access to the list of writer snapshots.
 * The locking and unlocking is the responsibility of the calling thread.
 * 
 * @return The mutex.
 */
os_mutex                cmx_getWriterSnapshotMutex  ();

void                    cmx_internalDetach          ();

#if defined (__cplusplus)
}
#endif

#endif /* CMX__FACTORY_H */

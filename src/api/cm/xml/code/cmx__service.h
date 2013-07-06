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
/**@file api/cm/xml/code/cmx__service.h
 *
 * Offers internal routines on a service.
 */
#ifndef CMX__SERVICE_H
#define CMX__SERVICE_H

#include "c_typebase.h"
#include "v_service.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "cmx_service.h"

/**
 * Initializes the service specific part of the XML representation of the
 * supplied kernel service. This function should only be used by the
 * cmx_entityNewFromWalk function.
 *
 * @param entity The entity to create a XML representation of.
 * @return The service specific part of the XML representation of the entity.
 */
c_char* cmx_serviceInit     (v_service entity);

/**
 * Entity action routine to resolve the state of the service.
 *
 * @param service The kernel service entity.
 * @param args Must be of type cmx_walkEntityArgs. The XML entity of the state
 *             will be constructed and inserted in the args during the execution
 *             of this function.
 */
void    cmx_serviceAction   (v_entity service, c_voidp args);

#if defined (__cplusplus)
}
#endif

#endif /* CMX__SERVICE_H */

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
/**@file api/cm/xml/code/cmx__publisher.h
 * 
 * Offers internal routines on a publisher.
 */
#ifndef CMX__PUBLISHER_H
#define CMX__PUBLISHER_H

#include "c_typebase.h"
#include "v_publisher.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "cmx_publisher.h"

/**
 * Initializes the publisher specific part of the XML representation of the 
 * supplied kernel publisher. This function should only be used by the 
 * cmx_entityNewFromWalk function.
 * 
 * @param entity The entity to create a XML representation of.
 * @return The publisher specific part of the XML representation of the entity.
 */
c_char* cmx_publisherInit   (v_publisher entity);

#if defined (__cplusplus)
}
#endif

#endif /* CMX__PUBLISHER_H */

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
/**@file api/cm/xml/code/cmx__subscriber.h
 * 
 * Offers internal routines on a subscriber.
 */
#ifndef CMX__SUBSCRIBER_H
#define CMX__SUBSCRIBER_H

#include "c_typebase.h"
#include "v_subscriber.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "cmx_subscriber.h"

/**
 * Initializes the subscriber specific part of the XML representation of the 
 * supplied kernel subscriber. This function should only be used by the 
 * cmx_entityNewFromWalk function.
 * 
 * @param entity The entity to create a XML representation of.
 * @return The subscriber specific part of the XML representation of the entity.
 */
c_char* cmx_subscriberInit (v_subscriber entity);

#if defined (__cplusplus)
}
#endif

#endif /* CMX__SUBSCRIBER_H */

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
/**@file api/cm/xml/code/cmx__groupqueue.h
 * 
 * Offers internal routines on a groupqueue.
 */
#ifndef CMX__GROUPQUEUE_H
#define CMX__GROUPQUEUE_H

#include "c_typebase.h"
#include "v_kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * Initializes the groupqueue specific part of the XML representation of the 
 * supplied kernel groupqueue. This function should only be used by the 
 * cmx_entityNewFromWalk function.
 * 
 * @param entity The entity to create a XML representation of.
 * @return The groupqueue specific part of the XML representation of the entity.
 */
c_char* cmx_groupQueueInit   (v_groupQueue entity);

#if defined (__cplusplus)
}
#endif

#endif /* CMX__GROUPQUEUE_H */

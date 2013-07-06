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

#ifndef V__ENTITY_H
#define V__ENTITY_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "v_entity.h"

void
v_entityFree (
    v_entity _this);

/**
 * The initialisation of an entity object.
 * This method initialises all attributes of the entity class and must
 * be called by every derived class.
 *
 * \param _this  the reference to an entity object.
 * \param name   the name of the entity.
 */
v_result
v_entityInit (
    v_entity _this,
    const c_char *name,
    v_statistics s,
    c_bool enable);

/**
 * The de-initialisation of an entity object.
 * This method releases all used resources by the entity object and must
 * be called by every derived class.
 *
 * \param _this  the reference to an entity object.
 */
void
v_entityDeinit (
    v_entity _this);

#if defined (__cplusplus)
}
#endif

#endif /* V__ENTITY_H */

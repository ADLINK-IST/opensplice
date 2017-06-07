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

c_ulong
v_entityGetTriggerValue(
    v_entity _this);

#if defined (__cplusplus)
}
#endif

#endif /* V__ENTITY_H */

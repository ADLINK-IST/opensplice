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

/**@file api/cm/c/include/cmc_api.h
 * 
 * @brief The C version of the Control & Monitoring API.
 */
#ifndef CMC_API_H
#define CMC_API_H

#include "cmc_typebase.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**@brief Initializes the C-Control & Monitoring API.
 *  
 * This is done by calling the common 'cm_init()' function.
 * 
 * @return CM_RESULT_OK if succeeded, any other result otherwise.
 */
cm_result       cmc_init();

/**@brief Detaches the C-Control & Monitoring API.
 * 
 * This is done by calling the common 'cm_detach()' function.
 * 
 * @return CM_RESULT_OK if succeeded, any other result otherwise.
 */
cm_result       cmc_detach();

/**@brief Retrieves the root entity associated with the specified domain id.
 * 
 * The root entity is the splice 'kernel' 
 * 
 * @return The root entity (kernel) of the supplied Domain.
 */
cmc_entity      cmc_getRootEntity(      long domainId);

/**@brief Retrieves the entities whitin the supplied entity.
 * 
 * @param entity The entity where to find entities in.
 * @param subKind The kind of entities to find. All subkinds of the 
 * supplied kind are also retrieved. That means K_ENTITY retrieves all
 * subentities.
 * 
 * @return The set of entities that are located within the supplied entity
 * and match the supplied kind.
 */
cmc_entitySet   cmc_getEntities(        cmc_entity entity, 
                                        v_kind subKind);

/**@brief Retrieves the type of the supplied entity.
 * 
 * @param entity The entity to find the type of.
 * @return The type of the supplied entity.
 */
c_type          cmc_getEntityType(      cmc_entity entity);

/*FUTURE FUNCTIONS
cmc_qos         cmc_getEntityQos(       cmc_entity entity);

cmc_status      cmc_getEntityStatus(    cmc_entity entity);
*/
#if defined (__cplusplus)
}
#endif

#endif /* CMC_API_H */

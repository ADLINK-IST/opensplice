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

/**@file api/cm/c/include/cmc_entity.h
 * 
 * @brief The C version of an entity.
 */
#ifndef CMC_ENTITY_H
#define CMC_ENTITY_H

#include "cmc_typebase.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**@brief The entity class.
 */
C_STRUCT(cmc_entity){
    C_EXTENDS(cm_entity);
    c_char* name;           /*!<The name of the entity.*/
    v_kind vkind;           /*!<The kind of the entity.*/
    c_long index;           /*!<The index of the handle of the entity.*/
    c_long serial;          /*!<The serial of the handle of the entity.*/
    c_char* kernel_uri;     /*!<The kernel URI of the Domain the entity is located in.*/
};

#define cmc_entity(e) ((cmc_entity)(e))

/**@brief Creates a new cmc_entity of the supplied v_entity.
 * 
 * This function is used by the cmc_api as copy function and by the 
 * cmc_entitySetNew function and is only meant to be called that way, because 
 * it assumes it is executed in protected area. 
 * 
 * @param entity The kernel entity to create the cm_entity from.
 * @param kernel_uri The URI of the kernel that is associated with the 
 * supplied entity.
 * 
 * @return The newly created cmc_entity.
 */
c_voidp         cmc_entityNew       (v_entity entity,
                                     c_voidp kernel_uri);

/**@brief Frees the supplied cmc_entity
 * 
 * @param entity The entity to free.
 * @return CM_RESULT_OK if succeeded, CM_RESULT_BAD_PARAMTER otherwise.
 */
cm_result       cmc_entityFree      (cmc_entity entity);

/*FUTURE FUNCTIONS
cmc_qos         cmc_entityGetQos    (cmc_entity entity);

cmc_status      cmc_entityGetStatus (cmc_entity entity);
*/

#if defined (__cplusplus)
}
#endif

#endif /* CMC_ENTITY_H */

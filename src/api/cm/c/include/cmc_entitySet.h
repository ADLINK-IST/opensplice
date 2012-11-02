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

/**@file api/cm/c/include/cmc_entitySet.h
 * 
 * @brief The C version of a set of entities.
 */
#ifndef CMC_ENTITYSET_H
#define CMC_ENTITYSET_H

#include "c_iterator.h"
#include "cmc_typebase.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**@brief Set of cmc_entities.
 */
C_STRUCT(cmc_entitySet){
    C_EXTENDS(cm_entitySet);
    c_iter entities;            /*! Set of cmc_entity objects.*/
};

#define cmc_entitySet(e) ((cmc_entitySet)(e))

/**@brief Creates a new cmc_entitySet of the supplied set of kernel entities.
 * 
 * This function is used by the cmc_api: cmc_getEntities() funtion and is only
 * meant to be called that way, because it assumes it is executed in protected 
 * area. 
 * 
 * @param ventities The set of kernel entities that need to be copied.
 * @param kernel_uri The URI of the Domain, where the entities are located in.
 * @return The newly created cmc_entitySet.
 */
c_voidp         cmc_entitySetNew    (c_iter ventities,
                                     c_voidp kernel_uri);

/**@brief Frees the supplied entitySet.
 * 
 * All contained cmc_entity objects are also freed.
 * 
 * @param es The set of entities to free.
 * @return CM_RESULT_OK if succeeded, CM_RESULT_BAD_PARAMETER otherwise.
 */
cm_result       cmc_entitySetFree   (cmc_entitySet es);

#if defined (__cplusplus)
}
#endif

#endif /* CMC_ENTITYSET_H */

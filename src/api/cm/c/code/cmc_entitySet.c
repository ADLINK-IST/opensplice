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

#include "cmc_entitySet.h"
#include "cmc_entity.h"
#include "v_entity.h"


c_voidp
cmc_entitySetNew(
    c_iter ventities,
    c_voidp kernel_uri)
{
    cmc_entitySet entitySet;
    cmc_entity centity;
    v_entity entity;
    
    entitySet = cmc_entitySet(os_malloc((size_t)(C_SIZEOF(cmc_entitySet))));
    entitySet->entities = c_iterNew(NULL);
    cm_baseObject(entitySet)->kind = CMC_ENTITY_SET;
    
    if(ventities != NULL){
        entity = v_entity(c_iterTakeFirst(ventities));
        
        while(entity != NULL){
            centity = cmc_entity(cmc_entityNew(entity, kernel_uri));
            c_iterInsert(entitySet->entities, centity);
            entity = v_entity(c_iterTakeFirst(ventities));
        }
    }
    return entitySet;
}

cm_result
cmc_entitySetFree(
    cmc_entitySet es)
{
    cmc_entity e;
    cm_result r;
    
    r = CM_RESULT_OK;
    
    if( (es == NULL) || (es->entities == NULL)){
        r = CM_RESULT_BAD_PARAMETER;
    } else {
        e = cmc_entity(c_iterTakeFirst(es->entities));
        
        while(e != NULL){
            r = cmc_entityFree(e);
            
            if(r == CM_RESULT_OK){
                e = cmc_entity(c_iterTakeFirst(es->entities));
            } else{
                e = NULL;
            }
        }
        c_iterFree(es->entities);
        os_free(es);
    }
    
    return r;
}

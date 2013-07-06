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

#include "cmc_entity.h"
#include "v_kernel.h"
#include "v_entity.h"

c_voidp
cmc_entityNew(
    v_entity entity,
    c_voidp kernel_uri)
{
    cmc_entity ce;
    const c_char* uri;
    
    assert(entity != NULL);
    ce = cmc_entity(os_malloc((size_t)(C_SIZEOF(cmc_entity))));
    cm_baseObject(ce)->kind = CMC_ENTITY;
    
    if(entity->name != NULL){
        ce->name = (c_char*)(os_malloc(strlen(entity->name) + 1));
       os_strncpy(ce->name, entity->name, strlen(entity->name) + 1);
    }
    else {
        ce->name = NULL;
    }
    
    ce->vkind = v_object(entity)->kind;
    ce->index = entity->handle.index;
    ce->serial = entity->handle.serial;
    
    uri = (const char*)kernel_uri;
    ce->kernel_uri = (c_char*)(os_malloc(strlen(uri) + 1));
    memcpy(ce->kernel_uri, uri, strlen(uri) + 1);
    c_free(entity);
    
    return ce;
}

cm_result
cmc_entityFree(
    cmc_entity entity)
{
    cm_result r;
    
    r = CM_RESULT_OK;
    
    if(entity != NULL){
        if(entity->name != NULL){
            os_free(entity->name);
        }
        os_free(entity->kernel_uri);
        os_free(entity);
    }
    else{
        r = CM_RESULT_BAD_PARAMETER;
    }    
    return r;
}

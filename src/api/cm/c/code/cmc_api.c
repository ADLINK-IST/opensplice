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

#include "cm_api.h"
#include "cmc_api.h"
#include "cmc_entity.h"
#include "cmc_entitySet.h"


cm_result
cmc_init()
{
    return cm_init();
}

cm_result
cmc_detach()
{
    return cm_detach();
}

cmc_entity
cmc_getRootEntity(
    long domainId)
{
    cmc_entity entity;
    const c_char* kernel_uri;
    
    kernel_uri = cm_getKernelURI(domainId);
    

    entity = cmc_entity(
                cm_getRootEntity(
                    kernel_uri, 
                    cmc_entityNew,
                    (c_voidp)kernel_uri));
    
    return entity;
}

cmc_entitySet
cmc_getEntities(
    cmc_entity entity, 
    v_kind subKind)
{
    cmc_entitySet es;
    
    es = NULL;
    
    if(entity != NULL){
        es = cmc_entitySet(
                cm_getEntities(
                    entity->index,
                    entity->serial,
                    entity->kernel_uri,
                    subKind,
                    cmc_entitySetNew,
                    (c_voidp)(entity->kernel_uri)));
 
    }
    return es;
}

c_type
cmc_getEntityType(
    cmc_entity entity)
{
    return cm_getEntityType(entity->index,
                            entity->serial,
                            entity->kernel_uri);
}

/*
cmc_qos
cmc_getEntityQos(
    cmc_entity entity)
{
    cmc_qos qos;
    
    qos = NULL;
    
    if(entity != NULL){
        qos = cmc_qos(    
                u_entityResolveQos(
                        entity->index,
                        entity->serial,
                        entity->kernel_uri,
                        cmc_qosNew));
    }
    return qos;
}

cmc_status
cmc_getEntityStatus(
    cmc_entity entity)
{
    cmc_status status;
    
    status = NULL;
    
    if(entity != NULL){
        status = cmc_status(    
                    u_entityResolveStatus(
                        entity->index,
                        entity->serial,
                        entity->kernel_uri,
                        cmc_StatusNew));
    }
    return status;
}
*/

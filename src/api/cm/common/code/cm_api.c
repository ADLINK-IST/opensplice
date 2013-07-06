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
#include "cm_misc.h"
#include "u_user.h"
#include "v_kernel.h"
#include "v_entity.h"
#include "jni_nameService.h"
#include "jni_misc.h"
#include "c_iterator.h"

C_CLASS(cm_module);
C_CLASS(cm_participant);


C_STRUCT(cm_module){
    jni_nameService ns;  /*!<The attached nameservice*/
    c_iter participants; /*!< set of <cm_participant> objects.*/
};

C_STRUCT(cm_participant){
    u_participant participant;  /*!<The user participant.*/
    v_kernel kernel;
    const char* uri;     /*!<The associated kernel URI.*/
};

#define cm_module(e) ((cm_module)(e))
#define cm_participant(e) ((cm_participant)(e))

static cm_participant cm_getParticipant(const char* uri);

/**@brief Represents the Control & Monitoring API. 
 * 
 * For each kernel, this module creates at most one participant, but only when 
 * needed. The init() function intializes this module by initializing the
 * nameservice. The detach() function frees all resources by freeing all used
 * participants and the nameservice.
 */
static cm_module module = NULL;

/*@brief Looks up the participant in the control & monitoring module.
 * 
 * A new participant is created when none is active for the requested kernel.
 * 
 * @param uri The URI associated with the kernel of the participant.
 * @return The associated participant.
 */
static cm_participant
cm_getParticipant(
    const char* uri)
{
    cm_participant p, temp;
    c_iter copy;
    int found;
    u_domain uk;
    u_participant up;
    int scmp;
    
    p = NULL;
    found = 0;
    
    if(module != NULL){
        copy = c_iterCopy(module->participants);
        
        temp = cm_participant(c_iterTakeFirst(copy));
        
        while( (temp != NULL) && (!found) ){
           scmp = strcmp(temp->uri, uri);
           
            if(scmp == 0){
                p = temp;
                found = 1;
            }
            temp = cm_participant(c_iterTakeFirst(copy));
        }
        c_iterFree(copy);
        
        if(!found){
            up = u_participantNew(uri, 0, "CM API", NULL);
                
            if(up != NULL){
                p = cm_participant(os_malloc((size_t)(C_SIZEOF(cm_participant))));
                p->participant = up;
                p->uri = uri;
                module->participants = c_iterInsert(module->participants, p);
            }
        }
    }
    return p;
}

cm_result
cm_init()
{
    cm_result r;
    jni_nameService ns;
    r = CM_RESULT_OK;
    
    if(module == NULL){
        ns = jni_nameServiceNew();
        
        if(ns == NULL){
            r = CM_RESULT_ERROR;
        } else{
            module = cm_module(os_malloc((size_t)(C_SIZEOF(cm_module))));
            module->ns = ns;
            module->participants = c_iterNew(NULL);
        }
    }
    return r;
}

cm_result
cm_detach()
{
    jni_result r;
    cm_result cr;
    cm_participant p;
    u_result ur;
    int stop;
    
    r = JNI_RESULT_OK;
    
    if(module != NULL){
        p = cm_participant(c_iterTakeFirst(module->participants));
        stop = 0;
        
        while((p != NULL) && (!stop)){
            ur = u_participantFree(p->participant);
            r = jni_convertResult(ur);
            
            if(r == JNI_RESULT_OK){
                os_free(p);
                p = cm_participant(c_iterTakeFirst(module->participants));
            } else{
                stop = 1;
            }
        }
        
        if(r == JNI_RESULT_OK){
            r = jni_nameServiceFree();
        
            if(r == JNI_RESULT_OK){
                module->ns = NULL;
                c_iterFree(module->participants);
                os_free(module);
                module = NULL;
            }
        }
    }
    if( r == JNI_RESULT_OK){
        cr = CM_RESULT_OK;
    } else{
        cr = CM_RESULT_ERROR;
    }    
    return cr;
}

const c_char*
cm_getKernelURI(
    long domainId)
{
    const c_char* result;
    
    result = NULL;
    
    if(module != NULL){
        result = jni_nameServiceResolveURI(domainId);
    }
    return result;
}

c_voidp
cm_getRootEntity(
    const c_char* uri,
    const c_voidp (*func)(v_entity entity, c_voidp funcArg),
    c_voidp fArg)
{
    u_domain uk;
    c_voidp result;
    cm_participant p;
    
    result = NULL;

    if(module != NULL){
        p = cm_getParticipant(uri);
        
        if(p != NULL){
            uk = u_participantDomain(p->participant);
        
            if(uk != NULL){
               result = u_domainGetCopy(uk, func, fArg);
            }
        }
    }
    return result;
}

struct cm_getEntitiesArg {
    v_kind kind;
    c_iter list;
};

static c_bool
action(
    v_entity e,
    c_voidp actionArg)
{
    struct cm_getEntitiesArg *arg;
    
    arg = (struct cm_getEntitiesArg *)actionArg;
   
    switch(arg->kind){/*User filter, select all entities of the supplied+inherited kinds*/
    case K_ENTITY:/*Always add the entity.*/
        arg->list = c_iterInsert(arg->list,c_keep(e));
        break;
    case K_QUERY:
        switch(v_object(e)->kind){
        case K_QUERY:       arg->list = c_iterInsert(arg->list,c_keep(e));  break;  
        default:            break;
        }
        break;
    case K_VIEW:
        switch(v_object(e)->kind){
        case K_VIEW:        arg->list = c_iterInsert(arg->list,c_keep(e));  break;  
        default:            break;
        }
        break;
    case K_TOPIC:
        switch(v_object(e)->kind){
        case K_TOPIC:       arg->list = c_iterInsert(arg->list,c_keep(e));  break;  
        default:            break;
        }
        break;
    case K_PUBLISHER:
        switch(v_object(e)->kind){
        case K_PUBLISHER:   arg->list = c_iterInsert(arg->list,c_keep(e));  break;  
        default:            break;
        }
        break; 
    case K_SUBSCRIBER:
        switch(v_object(e)->kind){
        case K_SUBSCRIBER:  arg->list = c_iterInsert(arg->list,c_keep(e));  break;  
        default:            break;
        }
        break;
    case K_DOMAIN:
        switch(v_object(e)->kind){
        case K_DOMAIN:      arg->list = c_iterInsert(arg->list,c_keep(e));  break;  
        default:            break;
        }
        break; 
    case K_READER:
        switch(v_object(e)->kind){
        case K_READER:
        case K_DATAREADER:
        case K_QUEUE:   
                            arg->list = c_iterInsert(arg->list,c_keep(e));  break;  
        default:            break;
        }
        break; 
    case K_DATAREADER:
        switch(v_object(e)->kind){
        case K_DATAREADER:  arg->list = c_iterInsert(arg->list,c_keep(e));  break;  
        default:            break;
        }
        break; 
    case K_QUEUE:
        switch(v_object(e)->kind){
        case K_QUEUE:       arg->list = c_iterInsert(arg->list,c_keep(e));  break;  
        default:            break;
        }
        break; 
    case K_WRITER:
        switch(v_object(e)->kind){
        case K_WRITER:      arg->list = c_iterInsert(arg->list,c_keep(e));  break;  
        default:            break;
        }
        break; 
    case K_GROUP:
        switch(v_object(e)->kind){
        case K_GROUP:       arg->list = c_iterInsert(arg->list,c_keep(e));  break;  
        default:            break;
        }
        break; 
    case K_PARTICIPANT:
        switch(v_object(e)->kind){
        case K_PARTICIPANT:
        case K_SERVICE:   
                            arg->list = c_iterInsert(arg->list,c_keep(e));  break;  
        default:            break;
        }
        break;  
    case K_SERVICE:
        switch(v_object(e)->kind){
        case K_SERVICE:     arg->list = c_iterInsert(arg->list,c_keep(e));  break;  
        default:            break;
        }
        break; 
    case K_SERVICESTATE:
        switch(v_object(e)->kind){
        case K_SERVICESTATE:arg->list = c_iterInsert(arg->list,c_keep(e));  break;  
        default:            break;
        }
        break; 
    default: break;
    }  
    return TRUE; 
}

c_voidp
cm_getEntities(
    c_long index,
    c_long serial,
    const c_char* uri,
    v_kind kind,
    const c_voidp (*func)(c_iter entities, c_voidp funcArg),
    const c_voidp fArg)
{
    v_handle handle;
    u_entity entity;
    cm_participant p;
    u_domain domain;
    struct cm_getEntitiesArg arg;
    c_voidp result;
    v_entity e;
    
    result = NULL;
    arg.list = NULL;
    arg.kind = kind;
    
    if(module != NULL){
        p = cm_getParticipant(uri);
        
        if(p != NULL){
            domain = u_participantDomain(p->participant);
            
            if(domain != NULL){
                handle = v_entity(u_domainSource(domain))->handle;
                if((handle.index == index) && (handle.serial == serial)) {
#if 0 /* temporary patch */
                    u_entityWalkEntities(u_entity(domain),action,&arg);
#else                    
                    v_entityWalkEntities(v_entity(domain->kernel),action,&arg);
#endif
                    result = func(arg.list, fArg);
                } else{
                    handle.index = index;
                    handle.serial = serial;
                    handle.server = u_domainSource(domain)->handleServer;
#if 0          
                    entity = u_entity(os_malloc((size_t)(C_SIZEOF(u_entity))));
                    entity->handle = handle;
                    entity->participant = p->participant;
#else
                    entity = u_entityFromHandle(handle,p->participant);
#endif
                    u_entityWalkEntities(entity,action,&arg);
                    result = func(arg.list, fArg);
                    os_free(entity);
                }
                e = c_iterTakeFirst(arg.list);
                while (e != NULL) {
                    c_free(e);
                    e = c_iterTakeFirst(arg.list);
                }
                c_iterFree(arg.list);
            }
        }
    }
    return result;
}


c_type
cm_getEntityType(
    c_long index,
    c_long serial,
    const c_char* uri)
{
    c_type type;
    cm_participant p;
    v_handle handle;
    u_domain domain;
    u_entity entity;
    
    type = NULL;
    
    if(module != NULL){
        p = cm_getParticipant(uri);
        
        if(p != NULL){
            domain = u_participantDomain(p->participant);
            
            if(domain != NULL){
                handle.index = index;
                handle.serial = serial;
                handle.server = u_domainSource(domain)->handleServer;

                entity = u_entityFromHandle(handle,p->participant);
                
                type = u_entityResolveType(entity);
                
                os_free(entity);
            }
        }
    }
    
    return type;
}

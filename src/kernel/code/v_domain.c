/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

#include "v_domain.h"
#include "v__domain.h"
#include "v__entity.h"
#include "v_public.h"
#include "v_handle.h"
#include "v_participant.h"
#include "v_publisher.h"
#include "v_subscriber.h"

/* Protected */

c_bool
v_domainExpressionIsAbsolute(
    const char *domainExpression)
{
    c_bool result;
    
    /* Absolute expressions are those which do not contain a '?' or a '*' */
    
    result = (strchr(domainExpression, '*') == NULL);
    result = result && (strchr(domainExpression, '?') == NULL);
    
    return result;
}    


c_bool
v_domainFitsExpression(
    v_domain domain,
    const char *domainExpression)
{
    c_bool result;
    c_value domainNameValue, domainExprValue, resultValue;
    
    assert(domain != NULL);
    assert(C_TYPECHECK(domain, v_domain));
    assert(domainExpression != NULL);
    
    /* Check exact name match and expression match */
    if (v_domainExpressionIsAbsolute(domainExpression)) {
        result = (strcmp(domainExpression, v_partitionName(domain)) == 0);
    } else {
        domainNameValue = c_stringValue((char *)v_partitionName(domain));
        domainExprValue = c_stringValue((char *)domainExpression);
        resultValue = c_valueStringMatch(domainExprValue, domainNameValue);
        result = resultValue.is.Boolean;
    }
    
    return result;
}    


/* Public */

v_domain
v_domainNew(
    v_kernel kernel,
    const c_char *name,
    v_domainQos qos)
{
    v_domain domain, found;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));

    assert(name != NULL);
    assert(v_domainExpressionIsAbsolute(name));

    domain = v_domain(v_objectNew(kernel,K_DOMAIN));
    v_entityInit(v_entity(domain),name, NULL, TRUE);

    found = v_addDomain(kernel,domain);

    if (found != domain) {
        v_domainFree(domain);
        c_free(domain); /* v_domainFree has removed all dependancies, now delete local reference */
        domain = c_keep(found); /* this one will be returned, so a keep is required */
    }
    return domain;
}

void
v_domainFree(
    v_domain domain)
{
    assert(C_TYPECHECK(domain,v_domain));

    v_publicFree(v_public(domain));
}

void
v_domainDeinit(
    v_domain domain)
{
    assert(C_TYPECHECK(domain,v_domain));

    v_entityDeinit(v_entity(domain));
}

c_iter
v_domainLookupPublishers(
    v_domain domain)
{
    c_iter participants;
    c_iter result;
    c_iter entities;
    c_iter domains;
    v_participant participant;
    v_entity entity;
    v_entity domain2;
    
    result = NULL;
    participants = v_resolveParticipants(v_objectKernel(domain), "*");
    participant = v_participant(c_iterTakeFirst(participants));
    
    while (participant != NULL) {
        c_lockRead(&participant->lock);
        entities = c_select(participant->entities, 0);
        c_lockUnlock(&participant->lock);
        entity = v_entity(c_iterTakeFirst(entities));
        
        while (entity != NULL) {
            if (v_objectKind(entity) == K_PUBLISHER) {
                domains = v_publisherLookupDomains(v_publisher(entity),
                                                   v_partitionName(domain));
                
                if (c_iterLength(domains) > 0) {
                    result = c_iterInsert(result, entity); /* transfer refcount */
                } else {
                    c_free(entity);
                }
                domain2 = v_entity(c_iterTakeFirst(domains));
                
                while (domain2 != NULL) {
                    c_free(domain2);
                    domain2 = v_entity(c_iterTakeFirst(domains));
                }
                c_iterFree(domains);
            }
            /* entity is already free or refcount transferred to result */
            entity = v_entity(c_iterTakeFirst(entities));
        }
        c_iterFree(entities);
        c_free(participant);
        participant = v_participant(c_iterTakeFirst(participants));
    }
    c_iterFree(participants);
    return result;   
}

c_iter
v_domainLookupSubscribers(
    v_domain domain)
{
    c_iter participants;
    c_iter result;
    c_iter entities;
    c_iter domains;
    v_participant participant;
    v_entity entity;
    v_entity domain2;
    
    result = NULL;
    participants = v_resolveParticipants(v_objectKernel(domain), "*");
    participant = v_participant(c_iterTakeFirst(participants));
    
    while (participant != NULL) {
        c_lockRead(&participant->lock);
        entities = c_select(participant->entities, 0);
        c_lockUnlock(&participant->lock);
        entity = v_entity(c_iterTakeFirst(entities));
        
        while (entity != NULL) {
            if(v_objectKind(entity) == K_SUBSCRIBER) {
                domains = v_subscriberLookupDomains(v_subscriber(entity),
                                                    v_partitionName(domain));
                
                if (c_iterLength(domains) > 0) {
                    result = c_iterInsert(result, entity); /* transfer refcount */
                } else {
                    c_free(entity);
                }
                domain2 = v_entity(c_iterTakeFirst(domains));
                
                while (domain2 != NULL) {
                    c_free(domain2);
                    domain2 = v_entity(c_iterTakeFirst(domains));
                }
                c_iterFree(domains);
            }
            /* entity is already free or refcount transferred to result */
            entity = v_entity(c_iterTakeFirst(entities));
        }
        c_iterFree(entities);
        c_free(participant);
        participant = v_participant(c_iterTakeFirst(participants));
    }
    c_iterFree(participants);
    return result;
}


v_domainInterest
v_domainInterestNew(
    v_kernel kernel,
    const char *domainExpression)
{
    v_domainInterest result = NULL;
   

    result = c_new(v_kernelType(kernel, K_DOMAININTEREST));
    result->expression = c_stringNew(c_getBase(c_object(kernel)), domainExpression);
    
    return result;
}    

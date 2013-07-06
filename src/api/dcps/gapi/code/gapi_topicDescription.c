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
#include "gapi_domainParticipant.h"
#include "gapi_typeSupport.h"
#include "gapi_topicDescription.h"
#include "gapi_contentFilteredTopic.h"
#include "gapi_structured.h"
#include "gapi_expression.h"

#include "os_mutex.h"
#include "os_heap.h"
#include "os_stdlib.h"
#include "u_user.h"
#include "u_participant.h"

#include "v_kernel.h"

gapi_returnCode_t
_TopicDescriptionInit (
    _TopicDescription topicDescription,
    const gapi_char *topic_name,
    const gapi_char *type_name,
    const gapi_char *expression,
    _DomainParticipant participant)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;

    assert(participant);

    if ( topicDescription ) {
        topicDescription->expr = gapi_parseExpression(expression);
        if (topicDescription->expr != NULL) {
            _EntityInit (_Entity(topicDescription),
                               _Entity(participant));

            topicDescription->topic_name = gapi_string_dup(topic_name);
            if (topicDescription->topic_name == NULL) {
                result = GAPI_RETCODE_OUT_OF_RESOURCES;
            } else {
                topicDescription->type_name = gapi_string_dup(type_name);
                if (topicDescription->type_name == NULL) {
                    result = GAPI_RETCODE_OUT_OF_RESOURCES;
                    os_free(topicDescription->topic_name);
                } else {
                    topicDescription->useCount = 0;
                }
            }
        } else {
            result = GAPI_RETCODE_PRECONDITION_NOT_MET;
        }
    }
    
    return result;
}

void
_TopicDescriptionDispose (
    _TopicDescription topicDescription)
{
    gapi_free (topicDescription->topic_name);
    gapi_free (topicDescription->type_name);
    q_dispose(topicDescription->expr);
    topicDescription->topic_name   = NULL;
    topicDescription->type_name    = NULL;
    topicDescription->expr         = NULL;

    _EntityDispose(_Entity(topicDescription));
}

gapi_returnCode_t
_TopicDescriptionFree (
    _TopicDescription topicDescription)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;

    _TopicDescriptionDispose(topicDescription);
    
    return result;
}

gapi_boolean
_TopicDescriptionPrepareDelete (
    _TopicDescription topicDescription)
{
    gapi_boolean result = FALSE;
    
    assert(topicDescription);
    
    if ( topicDescription->useCount == 0 ) {
        result = TRUE;
    }

    return result;
}

gapi_boolean
_TopicDescriptionHasType (
    _TopicDescription topicDescription,
    const gapi_char *typeName)
{
    gapi_boolean equal = FALSE;
    int len;
    
    assert(topicDescription);
    assert(typeName);

    len = strlen(topicDescription->type_name);

    if ( (len > 0) && (strncmp(topicDescription->type_name, typeName, len) == 0) ) {
        equal = TRUE;
    }

    return equal;
}


/*     string
 *     get_type_name();
 */
gapi_string
gapi_topicDescription_get_type_name (
    gapi_topicDescription _this)
{
    _TopicDescription topicDescription;
    gapi_string name = NULL;

    topicDescription = gapi_topicDescriptionClaim(_this, NULL);

    if ( topicDescription != NULL ) {
        if ( topicDescription->type_name != NULL ) {
            name = gapi_string_dup(topicDescription->type_name);
        }
    }

    _EntityRelease(topicDescription);
    
    return name;
}

/*     string
 *     get_name();
 */
gapi_string
gapi_topicDescription_get_name (
    gapi_topicDescription _this)
{
    _TopicDescription topicDescription;
    gapi_string name = NULL;

    topicDescription = gapi_topicDescriptionClaim(_this, NULL);   

    if ( topicDescription != NULL ) {
        if ( topicDescription->topic_name != NULL ) {
            name = gapi_string_dup(topicDescription->topic_name);
        }
    }

    _EntityRelease(topicDescription);
  
    return name;
}

/*     DomainParticipant
 *     get_participant();
 */
gapi_domainParticipant
gapi_topicDescription_get_participant (
    gapi_topicDescription _this)
{
    _TopicDescription topicDescription;
    _Topic topic;
    _DomainParticipant participant = NULL;
    
    topicDescription = gapi_topicDescriptionClaim(_this, NULL);   

    if ( topicDescription != NULL ) {
        /* all entities except for content filtered topics have a reference
         * to a user layer entity. So except for content filtered topics
         * entities get their participant from the user layer entity.
         * Content filters instead do this via the related topic.
         */
        if (_ObjectGetKind(_Object(topicDescription)) == OBJECT_KIND_CONTENTFILTEREDTOPIC) {
            topic = _ContentFilteredTopicGetRelatedTopic(_ContentFilteredTopic(topicDescription));
            participant = _EntityParticipant(_Entity(topic));
        } else {
            participant = _EntityParticipant(_Entity(topicDescription));
        }
    }

    _EntityRelease(topicDescription);
     
    return (gapi_domainParticipant)_EntityHandle(participant);
}


/*     string
 *     get_type_name();
 */
gapi_string
_TopicDescriptionGetTypeName (
    _TopicDescription topicDescription)
{
    gapi_string name = NULL;

    assert(topicDescription);

    if ( topicDescription->type_name != NULL ) {
        name = gapi_string_dup(topicDescription->type_name);
    }

    return name;
}

/*     string
 *     get_name();
 */
gapi_string
_TopicDescriptionGetName (
    _TopicDescription topicDescription)
{
    gapi_string name = NULL;

    assert(topicDescription);

    if ( topicDescription->topic_name != NULL ) {
        name = gapi_string_dup(topicDescription->topic_name);
    }
  
    return name;
}

q_expr
_TopicDescriptionGetExpr (
    _TopicDescription topicDescription)
{
    assert(topicDescription);
    return q_exprCopy(topicDescription->expr);
}

void
_TopicDescriptionIncUse (
    _TopicDescription topicDescription)
{
    assert(topicDescription);

    topicDescription->useCount++;
}

void
_TopicDescriptionDecUse (
    _TopicDescription topicDescription)
{
    assert(topicDescription);

    topicDescription->useCount--;
}
        
gapi_unsigned_long
_TopicDescriptionMessageOffset (
    _TopicDescription topicDescription)
{
    return topicDescription->messageOffset;
}

gapi_unsigned_long
_TopicDescriptionUserdataOffset (
    _TopicDescription topicDescription)
{
    return topicDescription->userdataOffset;
}

gapi_unsigned_long
_TopicDescriptionAllocSize (
    _TopicDescription topicDescription)
{
    return topicDescription->allocSize;
}

gapi_topicAllocBuffer
_TopicDescriptionAllocBuffer (
    _TopicDescription topicDescription)
{
    return topicDescription->allocBuffer;
}

#if 0
gapi_expression
_TopicDescriptionCreateExpression (
    const char *topicName)
{
    const char      *prefix = "select * from ";
    unsigned long    len;
    char            *stmt;
    gapi_expression  expr = NULL;
    
    len = strlen(prefix) + strlen(topicName) + 1;

    stmt = (char *) os_malloc(len);
    
    if ( stmt ) {
        snprintf(stmt, len, "%s%s", prefix, topicName);
        expr = gapi_expressionNew(stmt);
        os_free(stmt);
    }

    return expr;
}
#endif

void
_TopicDescriptionCopyContext (
    _TopicDescription topicDescr,
    _TopicDescription to)
{
    to->messageOffset  = topicDescr->messageOffset;
    to->userdataOffset = topicDescr->userdataOffset;
    to->allocSize      = topicDescr->allocSize;
    to->allocBuffer    = topicDescr->allocBuffer;
}

_TypeSupport
_TopicDescriptionGetTypeSupport (
    _TopicDescription topicDescr)
{
    _TypeSupport typeSupport = NULL;
    _Topic topic;
    _DomainParticipant participant = NULL;
    
    assert(topicDescr);

    if (_ObjectGetKind(_Object(topicDescr)) == OBJECT_KIND_CONTENTFILTEREDTOPIC) {
        topic = _ContentFilteredTopicGetRelatedTopic(_ContentFilteredTopic(topicDescr));
        participant = _EntityParticipant(_Entity(topic));
    } else {
        participant = _EntityParticipant(_Entity(topicDescr));
    }

    typeSupport = _DomainParticipantFindType(
                      participant,
                      topicDescr->type_name);
    return typeSupport;
}
    


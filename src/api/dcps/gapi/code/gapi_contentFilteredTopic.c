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
#include "gapi_objManag.h"
#include "gapi_structured.h"
#include "gapi_contentFilteredTopic.h"
#include "gapi_topicDescription.h"
#include "gapi_topic.h"
#include "gapi_expression.h"

#include "v_filter.h"
#include "v_topic.h"

#include "os_report.h"
#include "os_heap.h"
#include "os_stdlib.h"

C_STRUCT(_ContentFilteredTopic) {
    C_EXTENDS(_TopicDescription);
    _Topic           relatedTopic;
    gapi_char       *expression;
    gapi_stringSeq  *parameters;
};


static gapi_expression
createExpression (
    const gapi_char *topicName,
    const gapi_char *filter
    )
{
    long        len;
    const char *prefix = "select * from ";
    const char *where  = " where ";
    char       *stmt;
    gapi_expression expr   = NULL;

    len = strlen(prefix) + strlen(where) + strlen(topicName) + strlen(filter) + 1;

    stmt = os_malloc(len);
    if ( stmt != NULL ) {
        snprintf(stmt, len, "%s%s%s%s", prefix, topicName, where, filter);
        expr = gapi_expressionNew(stmt);
        os_free(stmt);
    }

    return expr;
}

static gapi_boolean
createQueryParameters (
    const gapi_stringSeq  *parameters,
    c_value              **args
    )
{
    c_value           *valarr  = NULL;
    gapi_unsigned_long nparams = parameters->_length;
    gapi_boolean       result  = TRUE;
 
    *args = NULL;
    
    if ( nparams > 0 ) {
        valarr = (c_value *) os_malloc(nparams * sizeof(struct c_value));
        
        if ( valarr != NULL ) {
            gapi_unsigned_long i;
            for ( i = 0; i < nparams; i++ ) {
                valarr[i] = gapi_stringValue(parameters->_buffer[i]);
            }
            *args = valarr;
        } else {
            result = FALSE;
        }
    } 

    return result;
}

static gapi_long
getMaxParameterNumber (
    q_expr expr,
    gapi_long max)
{
    c_longlong pn = 0;
    q_list list;

    switch ( q_getKind(expr) ) {
        case T_VAR:
            pn = q_getVar(expr);
            if ( pn > max ) {
                max = (gapi_long)pn;
            }
            break;
        case T_FNC:
            list = q_getLst(expr, 0);
            while ( list ) {
                max = getMaxParameterNumber(q_element(list), max);
                list = q_next(list);
            }
            break;
        default:
            break;
    }
    return max;
}

_ContentFilteredTopic
_ContentFilteredTopicNew (
    const gapi_char      *contentFilteredTopicName,
    _Topic                relatedTopic,
    const gapi_char      *expression,
    const gapi_stringSeq *parameters,
    _DomainParticipant    participant
    )
{
    gapi_string topicName;
    q_expr expr;
    c_value *params;
    gapi_long max;
    _ContentFilteredTopic newTopic;
    
    assert(contentFilteredTopicName);
    assert(relatedTopic);
    assert(expression);
    assert(parameters);
    assert(participant);

    newTopic = _ContentFilteredTopicAlloc();
    
    if ( newTopic != NULL ) {
        gapi_string typeName = _TopicGetTypeName(relatedTopic);
        long        len;
        const char *format = "select * from %s where %s";
        char       *stmt;

        topicName = _TopicGetName(relatedTopic);
        
        len = strlen(format) + strlen(topicName) + strlen(expression) + 1;
        stmt = os_malloc(len);
        if ( stmt != NULL ) {
            snprintf(stmt, len, format, topicName, expression);
            
            if ( _TopicDescriptionInit(_TopicDescription(newTopic),
                                       contentFilteredTopicName,
                                       typeName,
                                       stmt,
                                       participant) == GAPI_RETCODE_OK ) {
                _TopicDescriptionCopyContext(_TopicDescription(relatedTopic),
                                             _TopicDescription(newTopic));
            } else {
                _EntityDelete(newTopic);
                newTopic = NULL;
            }
            os_free(stmt);
        } else {
            _EntityDelete(newTopic);
            newTopic = NULL;
        }
        gapi_free(topicName);
        gapi_free(typeName);
    }
    if ( newTopic ) {
        if(parameters->_length < 100){
            expr = _TopicDescriptionGetExpr(_TopicDescription(newTopic));
            max = getMaxParameterNumber(expr, -1) + 1;
            if((max <= (gapi_long)parameters->_length)) {
                newTopic->expression = gapi_string_dup(expression);
                newTopic->parameters = gapi_stringSeq_dup(parameters);
                if ((newTopic->expression != NULL) && (newTopic->parameters != NULL)) {
                    params = _ContentFilteredTopicParameters(newTopic);
                    if (u_topicContentFilterValidate(_TopicUtopic(relatedTopic), expr, params)) {
                        _TopicDescriptionIncUse(_TopicDescription(relatedTopic));
                        newTopic->relatedTopic = relatedTopic;
                    } else {
                        OS_REPORT_1(OS_API_INFO,
                            "_ContentFilteredTopicNew", 4,
                            "ContentFilteredTopic cannot be created. Filter invalid: %s", expression);
                        _ContentFilteredTopicFree(newTopic);
                        newTopic = NULL;
                    }
                } else {
                    _ContentFilteredTopicFree(newTopic);
                    newTopic = NULL;
                }
            } else {
                OS_REPORT_2(OS_API_INFO,
                    "_ContentFilteredTopicNew", 4,
                    "Number of supplied parameters (%d) not as expected (%d).",
                    parameters->_length, max);
                _ContentFilteredTopicFree(newTopic);
                newTopic = NULL;
            }
            q_dispose(expr);
        } else {
            OS_REPORT_1(OS_API_INFO,
                "_ContentFilteredTopicNew", 4,
                "Number of supplied parameters (%d) exceeds the maximum of 99.",
                parameters->_length);
            _ContentFilteredTopicFree(newTopic);
            newTopic = NULL;
        }   
    }
         
    return newTopic;
}

void
_ContentFilteredTopicFree (
    _ContentFilteredTopic topic
    )
{
    assert(topic);

    if ( _TopicDescription(topic->relatedTopic) ) {
        _TopicDescriptionDecUse(_TopicDescription(topic->relatedTopic));
    }

    if ( topic->expression ) {
        gapi_free(topic->expression);
    }
    if ( topic->parameters ) {
        gapi_free(topic->parameters);
    }

    _TopicDescriptionDispose(_TopicDescription(topic));
    
}

gapi_boolean
_ContentFilteredTopicPrepareDelete (
    _ContentFilteredTopic contentFilteredTopic
    )
{
    gapi_boolean result = FALSE;
    
    assert(contentFilteredTopic);

    result = _TopicDescriptionPrepareDelete(_TopicDescription(contentFilteredTopic));

    return result;
}

/*     string
 *     get_filter_expression();
 */
gapi_string
gapi_contentFilteredTopic_get_filter_expression (
    gapi_contentFilteredTopic _this
    )
{
    _ContentFilteredTopic topic;
    gapi_string           expression = (gapi_string) NULL;

    topic = gapi_contentFilteredTopicClaim(_this, NULL);
    if ( topic != NULL ) {
        expression = gapi_string_dup(topic->expression);
    }

    _EntityRelease(topic);
    
    return expression;
}

/*     ReturnCode_t
 *     get_expression_parameters(inout StringSeq expression_parameters);
 */
gapi_returnCode_t
gapi_contentFilteredTopic_get_expression_parameters (
    gapi_contentFilteredTopic _this,
    gapi_stringSeq *expression_parameters
    )
{
    _ContentFilteredTopic topic;
    gapi_returnCode_t result;

    topic = gapi_contentFilteredTopicClaim(_this, &result);
    if ( topic != NULL ) {
        gapi_stringSeqCopyout(topic->parameters,expression_parameters);
    }

    _EntityRelease(topic);
    
    return result;
}

c_value *
_ContentFilteredTopicParameters (
    _ContentFilteredTopic topic
    )
{
    c_value *params;
    gapi_stringSeq *parms;
    c_ulong n;

    parms = topic->parameters;
    params = (c_value *)os_malloc(parms->_length * sizeof(struct c_value));
    for (n=0;n<parms->_length;n++) {
        params[n] = gapi_stringValue(parms->_buffer[n]);
    }
    return params;
}

/*     ReturnCode_t
 *     set_expression_parameters(
 *         in StringSeq expression_parameters);
 */
gapi_returnCode_t
gapi_contentFilteredTopic_set_expression_parameters (
    gapi_contentFilteredTopic _this,
    const gapi_stringSeq *expression_parameters
    )
{
    return GAPI_RETCODE_UNSUPPORTED;
}

/*     Topic
 *     get_related_topic();
 */
gapi_topic
gapi_contentFilteredTopic_get_related_topic (
    gapi_contentFilteredTopic _this
    )
{
    _ContentFilteredTopic topic;
    gapi_topic            relatedTopic = NULL;

    topic = gapi_contentFilteredTopicClaim(_this, NULL);
    if ( topic != NULL ) {
        relatedTopic = _EntityHandle(topic->relatedTopic);
    }

    _EntityRelease(topic);
    
    return relatedTopic;
}

_Topic
_ContentFilteredTopicGetRelatedTopic (
    _ContentFilteredTopic contentFilteredTopic
    )
{
    assert(contentFilteredTopic);

    return contentFilteredTopic->relatedTopic;
}
    


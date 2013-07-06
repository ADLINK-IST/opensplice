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
#include "gapi_structured.h"
#include "gapi_multiTopic.h"
#include "gapi_topicDescription.h"

C_STRUCT(_MultiTopic) {
    C_EXTENDS(_TopicDescription);
    void	*empty;
};

_MultiTopic
_MultiTopicNew (
    void
    )
{
    return (_MultiTopic)0;
}

void
_MultiTopicFree (
    _MultiTopic multitopic
    )
{
}

/*     string
 *     get_subscription_expression();
 */
gapi_string
gapi_multiTopic_get_subscription_expression (
    gapi_multiTopic _this
    )
{
    return (gapi_string)0;
}

/*     ReturnCode_t
 *     get_expression_parameters(inout StringSeq expression_parameters);
 */
gapi_returnCode_t
gapi_multiTopic_get_expression_parameters (
    gapi_multiTopic _this,
    gapi_stringSeq *expression_parameters
    )
{
    return GAPI_RETCODE_UNSUPPORTED;
}

/*     ReturnCode_t
 *     set_expression_parameters(
 *         in StringSeq expression_parameters);
 */
gapi_returnCode_t
gapi_multiTopic_set_expression_parameters (
    gapi_multiTopic _this,
    const gapi_stringSeq *expression_parameters
    )
{
    return GAPI_RETCODE_UNSUPPORTED;
}

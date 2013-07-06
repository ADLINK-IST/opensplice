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

#include "gapi.h"

#include "dds_dcps.h"

/*     string
 *     get_subscription_expression();
 */
DDS_string
DDS_MultiTopic_get_subscription_expression (
    DDS_MultiTopic this
    )
{
    return (DDS_string)
	gapi_multiTopic_get_subscription_expression (
	    (gapi_multiTopic)this
	);
}

/*     StringSeq
 *     get_expression_parameters();
 */
DDS_ReturnCode_t
DDS_MultiTopic_get_expression_parameters (
        DDS_MultiTopic this,
        DDS_StringSeq * expression_parameters
        )
{
    return (DDS_ReturnCode_t)
	gapi_multiTopic_get_expression_parameters (
	    (gapi_multiTopic)this,
	    (gapi_stringSeq * )expression_parameters
	);
}

/*     ReturnCode_t
 *     set_expression_parameters(
 *         in StringSeq expression_parameters);
 */
DDS_ReturnCode_t
DDS_MultiTopic_set_expression_parameters (
    DDS_MultiTopic this,
    const DDS_StringSeq *expression_parameters
    )
{
    return (DDS_ReturnCode_t)
	gapi_multiTopic_set_expression_parameters (
	    (gapi_multiTopic)this,
	    (const gapi_stringSeq *)expression_parameters
	);
}

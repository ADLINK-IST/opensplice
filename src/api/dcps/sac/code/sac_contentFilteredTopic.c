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
 *     get_filter_expression();
 */
DDS_string
DDS_ContentFilteredTopic_get_filter_expression (
    DDS_ContentFilteredTopic this
    )
{
    return (DDS_string)
	gapi_contentFilteredTopic_get_filter_expression (
	    (gapi_contentFilteredTopic)this
	);
}

/*     StringSeq
 *     get_expression_parameters();
 */
 DDS_ReturnCode_t
 DDS_ContentFilteredTopic_get_expression_parameters (
         DDS_ContentFilteredTopic this,
         DDS_StringSeq * expression_parameters
         )
{
    return (DDS_ReturnCode_t)
	gapi_contentFilteredTopic_get_expression_parameters (
	    (gapi_contentFilteredTopic)this,
	    (gapi_stringSeq *) expression_parameters
	);
}

/*     ReturnCode_t
 *     set_expression_parameters(
 *         in StringSeq expression_parameters);
 */
DDS_ReturnCode_t
DDS_ContentFilteredTopic_set_expression_parameters (
    DDS_ContentFilteredTopic this,
    const DDS_StringSeq *expression_parameters
    )
{
    return (DDS_ReturnCode_t)
	gapi_contentFilteredTopic_set_expression_parameters (
	    (gapi_contentFilteredTopic)this,
	    (const gapi_stringSeq *)expression_parameters
	);
}

/*     Topic
 *     get_related_topic();
 */
DDS_Topic
DDS_ContentFilteredTopic_get_related_topic (
    DDS_ContentFilteredTopic this
    )
{
    return (DDS_Topic)
	gapi_contentFilteredTopic_get_related_topic (
	    (gapi_contentFilteredTopic)this
	);
}


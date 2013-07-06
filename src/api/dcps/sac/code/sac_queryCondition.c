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
 *     get_query_expression();
 */
DDS_string
DDS_QueryCondition_get_query_expression (
    DDS_QueryCondition this
    )
{
    return (DDS_string)
	gapi_queryCondition_get_query_expression (
	    (gapi_queryCondition)this
	);
}

/*     ReturnCode_t
 *     get_query_parameters(
 *         inout StringSeq query_arguments);
 */
DDS_ReturnCode_t 
DDS_QueryCondition_get_query_parameters (
    DDS_QueryCondition this,
    DDS_StringSeq *query_parameters
    )
{
    return (DDS_ReturnCode_t)
	gapi_queryCondition_get_query_parameters (
	    (gapi_queryCondition)this,
	    (gapi_stringSeq *)query_parameters
	);
}

/*     ReturnCode_t
 *     set_query_parameters(
 *         in StringSeq query_arguments);
 */
DDS_ReturnCode_t
DDS_QueryCondition_set_query_parameters (
    DDS_QueryCondition this,
    const DDS_StringSeq *query_parameters
    )
{
    return (DDS_ReturnCode_t)
	gapi_queryCondition_set_query_parameters (
	    (gapi_queryCondition)this,
	    (const gapi_stringSeq *)query_parameters
	);
}
/*     StringSeq
 *     get_query_arguments();
 */
DDS_StringSeq *
DDS_QueryCondition_get_query_arguments (
    DDS_QueryCondition this
    )
{
    
    gapi_stringSeq * result = gapi_stringSeq__alloc();
    (void)gapi_queryCondition_get_query_parameters (
	    (gapi_queryCondition)this,
	    result
	);
    return (DDS_StringSeq *)result;
}

/*     ReturnCode_t
 *     set_query_arguments(
 *         in StringSeq query_arguments);
 */
DDS_ReturnCode_t
DDS_QueryCondition_set_query_arguments (
    DDS_QueryCondition this,
    const DDS_StringSeq *query_arguments
    )
{
    return (DDS_ReturnCode_t)
	gapi_queryCondition_set_query_parameters (
	    (gapi_queryCondition)this,
	    (const gapi_stringSeq *)query_arguments
	);
}


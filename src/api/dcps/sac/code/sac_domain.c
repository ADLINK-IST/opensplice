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
#include "sac_structured.h"

/*     DDS_ReturnCode_t
 *     create_persistent_snapshot(
 *         in String partition_expression,
 *         in String topic_expression,
 *         in String URI);
 */
DDS_ReturnCode_t
DDS_Domain_create_persistent_snapshot (
    DDS_DomainParticipant _this,
    const DDS_char *partition_expression,
    const DDS_char *topic_expression,
    const DDS_char *URI)
{
    return (DDS_ReturnCode_t)
    gapi_domain_create_persistent_snapshot(
        (gapi_domainParticipant)_this,
        (gapi_string)partition_expression,
        (gapi_string)topic_expression,
        (gapi_string)URI);
}


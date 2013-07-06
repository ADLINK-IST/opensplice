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
 *     get_type_name();
 */
DDS_string
DDS_TopicDescription_get_type_name (
    DDS_TopicDescription this
    )
{
    return (DDS_string)
	gapi_topicDescription_get_type_name (
	    (gapi_topicDescription)this
	);
}

/*     string
 *     get_name();
 */
DDS_string
DDS_TopicDescription_get_name (
    DDS_TopicDescription this
    )
{
    return (DDS_string)
	gapi_topicDescription_get_name (
	    (gapi_topicDescription)this
	);
}

/*     DomainParticipant
 *     get_participant();
 */
DDS_DomainParticipant
DDS_TopicDescription_get_participant (
    DDS_TopicDescription this
    )
{
    return (gapi_domainParticipant)
	gapi_topicDescription_get_participant (
	    (gapi_topicDescription)this
	);
}

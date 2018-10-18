/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "dds_dcps.h"
#include "sac_common.h"

DDS_MultiTopic
DDS_MultiTopicNew (
    DDS_DomainParticipant participant,
    const DDS_char *name,
    const DDS_char *type_name,
    const DDS_char *subscription_expression,
    const DDS_StringSeq *expression_parameters)
{
    OS_UNUSED_ARG(participant);
    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(type_name);
    OS_UNUSED_ARG(subscription_expression);
    OS_UNUSED_ARG(expression_parameters);

    return NULL;
}


DDS_ReturnCode_t
DDS_MultiTopicFree (
    DDS_MultiTopic _this)
{
    DDS_ReturnCode_t result = DDS_RETCODE_UNSUPPORTED;

    OS_UNUSED_ARG(_this);

    return result;
}

/*     string
 *     get_subscription_expression();
 */
DDS_string
DDS_MultiTopic_get_subscription_expression (
    DDS_MultiTopic this)
{
    OS_UNUSED_ARG(this);

    return NULL;
}

/*     StringSeq
 *     get_expression_parameters();
 */
DDS_ReturnCode_t
DDS_MultiTopic_get_expression_parameters (
        DDS_MultiTopic this,
        DDS_StringSeq * expression_parameters)
{
    OS_UNUSED_ARG(this);
    OS_UNUSED_ARG(expression_parameters);

    return DDS_RETCODE_UNSUPPORTED;
}

/*     ReturnCode_t
 *     set_expression_parameters(
 *         in StringSeq expression_parameters);
 */
DDS_ReturnCode_t
DDS_MultiTopic_set_expression_parameters (
    DDS_MultiTopic this,
    const DDS_StringSeq *expression_parameters)
{
    OS_UNUSED_ARG(this);
    OS_UNUSED_ARG(expression_parameters);

    return DDS_RETCODE_UNSUPPORTED;
}

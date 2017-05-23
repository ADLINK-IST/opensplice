/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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

/************************************************************************
 * LOGICAL_NAME:    multitopic.h
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C programming language.
 * DATE             june 2007.
 ************************************************************************
 * 
 * This file contains the headers for all operations required to simulate 
 * the MultiTopic behavior.
 * 
 ***/

#include "dds_dcps.h"

DDS_TopicDescription
DDS_DomainParticipant_create_simulated_multitopic(
    DDS_DomainParticipant participant,
    const DDS_char *name,
    const DDS_char *type_name,
    const DDS_char *subscription_expression,
    const DDS_StringSeq *expression_parameters
);

DDS_ReturnCode_t
DDS_DomainParticipant_delete_simulated_multitopic(
    DDS_DomainParticipant participant,
    DDS_TopicDescription smt 
);

void on_message_available (
    void *listener_data,
    DDS_DataReader reader
);



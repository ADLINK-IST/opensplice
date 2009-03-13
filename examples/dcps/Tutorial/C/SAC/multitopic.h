/************************************************************************
 *  
 * Copyright (c) 2007
 * PrismTech Ltd.
 * All rights Reserved.
 * 
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



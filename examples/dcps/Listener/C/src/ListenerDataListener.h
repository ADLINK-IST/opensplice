/************************************************************************
 *  
 * Copyright (c) 2010
 * PrismTech Ltd.
 * All rights Reserved.
 * 
 * LOGICAL_NAME:    ListenerListener.h
 * FUNCTION:        Declaration of the DDS Listener's call-back methods.
 * MODULE:          Listener Example for the C programming language.
 * DATE             September 2010.
 ***********************************************************************/
#ifndef __LISTENERLISTENER_H__
#define __LISTENERLISTENER_H__

#include "dds_dcps.h"
#include "CheckStatus.h"

struct Listener_data
{
   DDS_DataReader* message_DataReader;

   DDS_GuardCondition* guardCondition;

   // This "isClosed" variable is used too stop the ListenerDataSubscriber process
   // once data is received (on_data_available),
   // or the Topic's QoS timed out (on_requested_deadline_missed).
   c_bool* isClosed;
};

void on_data_available(void *Listener_data, DDS_DataReader reader);

void on_requested_deadline_missed(void *Listener_data, DDS_DataReader reader, const DDS_RequestedDeadlineMissedStatus *status);

#endif

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

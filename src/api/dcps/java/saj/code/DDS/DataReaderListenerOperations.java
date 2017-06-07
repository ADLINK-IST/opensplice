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

package DDS;


public interface DataReaderListenerOperations  extends DDS.ListenerOperations
{
  void on_requested_deadline_missed (DDS.DataReader reader, DDS.RequestedDeadlineMissedStatus status);
  void on_requested_incompatible_qos (DDS.DataReader reader, DDS.RequestedIncompatibleQosStatus status);
  void on_sample_rejected (DDS.DataReader reader, DDS.SampleRejectedStatus status);
  void on_liveliness_changed (DDS.DataReader reader, DDS.LivelinessChangedStatus status);
  void on_data_available (DDS.DataReader reader);
  void on_subscription_matched (DDS.DataReader reader, DDS.SubscriptionMatchedStatus status);
  void on_sample_lost (DDS.DataReader reader, DDS.SampleLostStatus status);
} // interface DataReaderListenerOperations

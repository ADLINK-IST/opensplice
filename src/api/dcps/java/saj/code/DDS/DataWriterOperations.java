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


public interface DataWriterOperations  extends DDS.EntityOperations
{

  //      in InstanceHandle_t handle);
  int set_qos (DDS.DataWriterQos qos);
  int get_qos (DDS.DataWriterQosHolder qos);
  int set_listener (DDS.DataWriterListener a_listener, int mask);
  DDS.DataWriterListener get_listener ();
  DDS.Topic get_topic ();
  DDS.Publisher get_publisher ();
  int wait_for_acknowledgments(DDS.Duration_t max_wait);

  // Access the status
  int get_liveliness_lost_status (DDS.LivelinessLostStatusHolder  status);
  int get_offered_deadline_missed_status (DDS.OfferedDeadlineMissedStatusHolder status);
  int get_offered_incompatible_qos_status (DDS.OfferedIncompatibleQosStatusHolder status);
  int get_publication_matched_status (DDS.PublicationMatchedStatusHolder status);
  int assert_liveliness ();
  int get_matched_subscriptions (DDS.InstanceHandleSeqHolder subscription_handles);
  int get_matched_subscription_data (DDS.SubscriptionBuiltinTopicDataHolder subscription_data, long subscription_handle);
} // interface DataWriterOperations

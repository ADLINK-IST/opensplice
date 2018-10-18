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

package DDS;


public interface DataReaderOperations  extends DDS.EntityOperations
{

  //      in InstanceHandle_t handle);
  DDS.ReadCondition create_readcondition (int sample_states, int view_states, int instance_states);
  DDS.QueryCondition create_querycondition (int sample_states, int view_states, int instance_states, String query_expression, String[] query_parameters);
  int delete_readcondition (DDS.ReadCondition a_condition);
  int delete_contained_entities ();
  int set_qos (DDS.DataReaderQos qos);
  int get_qos (DDS.DataReaderQosHolder qos);
  int set_listener (DDS.DataReaderListener a_listener, int mask);
  DDS.DataReaderListener get_listener ();
  DDS.TopicDescription get_topicdescription ();
  DDS.Subscriber get_subscriber ();
  int get_sample_rejected_status (DDS.SampleRejectedStatusHolder status);
  int get_liveliness_changed_status (DDS.LivelinessChangedStatusHolder status);
  int get_requested_deadline_missed_status (DDS.RequestedDeadlineMissedStatusHolder status);
  int get_requested_incompatible_qos_status (DDS.RequestedIncompatibleQosStatusHolder status);
  int get_subscription_matched_status (DDS.SubscriptionMatchedStatusHolder status);
  int get_sample_lost_status (DDS.SampleLostStatusHolder status);
  int wait_for_historical_data (DDS.Duration_t max_wait);
  int wait_for_historical_data_w_condition(String filter_expression, String[] expression_parameters, DDS.Time_t min_source_timestamp, DDS.Time_t max_source_timestamp, DDS.ResourceLimitsQosPolicy resource_limits, DDS.Duration_t max_wait);
  int get_matched_publications (DDS.InstanceHandleSeqHolder publication_handles);
  int get_matched_publication_data (DDS.PublicationBuiltinTopicDataHolder publication_data, long publication_handle);
  DDS.DataReaderView create_view(DataReaderViewQos qos);
  int delete_view(DataReaderView a_view);
  int get_default_datareaderview_qos(DataReaderViewQosHolder qos);
  int set_default_datareaderview_qos(DataReaderViewQos qos);
} // interface DataReaderOperations

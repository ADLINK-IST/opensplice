/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
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

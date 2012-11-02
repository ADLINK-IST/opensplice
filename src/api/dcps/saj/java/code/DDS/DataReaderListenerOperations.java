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

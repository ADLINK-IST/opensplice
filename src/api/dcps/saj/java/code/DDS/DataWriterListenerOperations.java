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


public interface DataWriterListenerOperations  extends DDS.ListenerOperations
{
  void on_offered_deadline_missed (DDS.DataWriter writer, DDS.OfferedDeadlineMissedStatus status);
  void on_offered_incompatible_qos (DDS.DataWriter writer, DDS.OfferedIncompatibleQosStatus status);
  void on_liveliness_lost (DDS.DataWriter writer, DDS.LivelinessLostStatus status);
  void on_publication_matched (DDS.DataWriter writer, DDS.PublicationMatchedStatus status);
} // interface DataWriterListenerOperations

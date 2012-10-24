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


public interface SubscriberListenerOperations  extends DDS.DataReaderListenerOperations
{
  void on_data_on_readers (DDS.Subscriber subs);
} // interface SubscriberListenerOperations

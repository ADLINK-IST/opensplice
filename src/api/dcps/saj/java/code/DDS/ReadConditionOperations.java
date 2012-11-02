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


public interface ReadConditionOperations  extends DDS.ConditionOperations
{
  int get_sample_state_mask ();
  int get_view_state_mask ();
  int get_instance_state_mask ();
  DDS.DataReader get_datareader ();
  DDS.DataReaderView get_datareaderview ();
} // interface ReadConditionOperations

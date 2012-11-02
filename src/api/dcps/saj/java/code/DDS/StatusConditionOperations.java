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


public interface StatusConditionOperations  extends DDS.ConditionOperations
{
  int get_enabled_statuses ();
  int set_enabled_statuses (int mask);
  DDS.Entity get_entity ();
} // interface StatusConditionOperations

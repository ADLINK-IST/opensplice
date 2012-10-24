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


public interface WaitSetOperations 
{
  int _wait (DDS.ConditionSeqHolder active_conditions, DDS.Duration_t timeout);
  int attach_condition (DDS.Condition cond);
  int detach_condition (DDS.Condition cond);
  int get_conditions (DDS.ConditionSeqHolder attached_conditions);
} // interface WaitSetOperations

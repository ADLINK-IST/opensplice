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


public final class OfferedIncompatibleQosStatus 
{
  public int total_count = (int)0;
  public int total_count_change = (int)0;
  public int last_policy_id = (int)0;
  public DDS.QosPolicyCount policies[] = null;

  public OfferedIncompatibleQosStatus ()
  {
  } // ctor

  public OfferedIncompatibleQosStatus (int _total_count, int _total_count_change, int _last_policy_id, DDS.QosPolicyCount[] _policies)
  {
    total_count = _total_count;
    total_count_change = _total_count_change;
    last_policy_id = _last_policy_id;
    policies = _policies;
  } // ctor

} // class OfferedIncompatibleQosStatus

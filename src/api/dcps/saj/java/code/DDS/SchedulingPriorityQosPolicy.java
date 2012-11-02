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


public final class SchedulingPriorityQosPolicy
{
  public DDS.SchedulingPriorityQosPolicyKind kind = null;

  public SchedulingPriorityQosPolicy ()
  {
  } // ctor

  public SchedulingPriorityQosPolicy (DDS.SchedulingPriorityQosPolicyKind _kind)
  {
    kind = _kind;
  } // ctor

} // class SchedulingPriorityQosPolicy

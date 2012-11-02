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


public final class SchedulingQosPolicy
{
  public DDS.SchedulingClassQosPolicy scheduling_class = null;
  public DDS.SchedulingPriorityQosPolicy scheduling_priority_kind = null;
  public int scheduling_priority = (int)0;

  public SchedulingQosPolicy ()
  {
  } // ctor

  public SchedulingQosPolicy (DDS.SchedulingClassQosPolicy _scheduling_class, DDS.SchedulingPriorityQosPolicy _scheduling_priority_kind, int _scheduling_priority)
  {
    scheduling_class = _scheduling_class;
    scheduling_priority_kind = _scheduling_priority_kind;
    scheduling_priority = _scheduling_priority;
  } // ctor

} // class SchedulingQosPolicy

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


public class SchedulingPriorityQosPolicyKind implements org.omg.CORBA.portable.IDLEntity
{
  private        int __value;
  private static int __size = 2;
  private static DDS.SchedulingPriorityQosPolicyKind[] __array = new DDS.SchedulingPriorityQosPolicyKind [__size];

  public static final int _PRIORITY_RELATIVE = 0;
  public static final DDS.SchedulingPriorityQosPolicyKind PRIORITY_RELATIVE = new DDS.SchedulingPriorityQosPolicyKind(_PRIORITY_RELATIVE);
  public static final int _PRIORITY_ABSOLUTE = 1;
  public static final DDS.SchedulingPriorityQosPolicyKind PRIORITY_ABSOLUTE = new DDS.SchedulingPriorityQosPolicyKind(_PRIORITY_ABSOLUTE);

  public int value ()
  {
    return __value;
  }

  public static DDS.SchedulingPriorityQosPolicyKind from_int (int value)
  {
    if (value >= 0 && value < __size)
      return __array[value];
    else
      throw new org.omg.CORBA.BAD_PARAM ();
  }

  protected SchedulingPriorityQosPolicyKind (int value)
  {
    __value = value;
    __array[__value] = this;
  }
} // class SchedulingPriorityQosPolicyKind

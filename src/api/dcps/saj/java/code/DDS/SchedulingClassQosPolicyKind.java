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


public class SchedulingClassQosPolicyKind
{
  private        int __value;
  private static int __size = 3;
  private static DDS.SchedulingClassQosPolicyKind[] __array = new DDS.SchedulingClassQosPolicyKind [__size];

  public static final int _SCHEDULE_DEFAULT = 0;
  public static final DDS.SchedulingClassQosPolicyKind SCHEDULE_DEFAULT = new DDS.SchedulingClassQosPolicyKind(_SCHEDULE_DEFAULT);
  public static final int _SCHEDULE_TIMESHARING = 1;
  public static final DDS.SchedulingClassQosPolicyKind SCHEDULE_TIMESHARING = new DDS.SchedulingClassQosPolicyKind(_SCHEDULE_TIMESHARING);
  public static final int _SCHEDULE_REALTIME = 2;
  public static final DDS.SchedulingClassQosPolicyKind SCHEDULE_REALTIME = new DDS.SchedulingClassQosPolicyKind(_SCHEDULE_REALTIME);

  public int value ()
  {
    return __value;
  }

  public static DDS.SchedulingClassQosPolicyKind from_int (int value)
  {
    if (value >= 0 && value < __size)
      return __array[value];
    else
      throw new org.omg.CORBA.BAD_PARAM ();
  }

  protected SchedulingClassQosPolicyKind (int value)
  {
    __value = value;
    __array[__value] = this;
  }
} // class SchedulingClassQosPolicyKind

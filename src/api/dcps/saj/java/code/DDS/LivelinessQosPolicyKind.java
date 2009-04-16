/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

package DDS;


public class LivelinessQosPolicyKind 
{
  private        int __value;
  private static int __size = 3;
  private static DDS.LivelinessQosPolicyKind[] __array = new DDS.LivelinessQosPolicyKind [__size];

  public static final int _AUTOMATIC_LIVELINESS_QOS = 0;
  public static final DDS.LivelinessQosPolicyKind AUTOMATIC_LIVELINESS_QOS = new DDS.LivelinessQosPolicyKind(_AUTOMATIC_LIVELINESS_QOS);
  public static final int _MANUAL_BY_PARTICIPANT_LIVELINESS_QOS = 1;
  public static final DDS.LivelinessQosPolicyKind MANUAL_BY_PARTICIPANT_LIVELINESS_QOS = new DDS.LivelinessQosPolicyKind(_MANUAL_BY_PARTICIPANT_LIVELINESS_QOS);
  public static final int _MANUAL_BY_TOPIC_LIVELINESS_QOS = 2;
  public static final DDS.LivelinessQosPolicyKind MANUAL_BY_TOPIC_LIVELINESS_QOS = new DDS.LivelinessQosPolicyKind(_MANUAL_BY_TOPIC_LIVELINESS_QOS);

  public int value ()
  {
    return __value;
  }

  public static DDS.LivelinessQosPolicyKind from_int (int value)
  {
    if (value >= 0 && value < __size)
      return __array[value];
    else
      throw new org.omg.CORBA.BAD_PARAM ();
  }

  protected LivelinessQosPolicyKind (int value)
  {
    __value = value;
    __array[__value] = this;
  }
} // class LivelinessQosPolicyKind

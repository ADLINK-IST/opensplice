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


public class HistoryQosPolicyKind 
{
  private        int __value;
  private static int __size = 2;
  private static DDS.HistoryQosPolicyKind[] __array = new DDS.HistoryQosPolicyKind [__size];

  public static final int _KEEP_LAST_HISTORY_QOS = 0;
  public static final DDS.HistoryQosPolicyKind KEEP_LAST_HISTORY_QOS = new DDS.HistoryQosPolicyKind(_KEEP_LAST_HISTORY_QOS);
  public static final int _KEEP_ALL_HISTORY_QOS = 1;
  public static final DDS.HistoryQosPolicyKind KEEP_ALL_HISTORY_QOS = new DDS.HistoryQosPolicyKind(_KEEP_ALL_HISTORY_QOS);

  public int value ()
  {
    return __value;
  }

  public static DDS.HistoryQosPolicyKind from_int (int value)
  {
    if (value >= 0 && value < __size)
      return __array[value];
    else
      throw new org.omg.CORBA.BAD_PARAM ();
  }

  protected HistoryQosPolicyKind (int value)
  {
    __value = value;
    __array[__value] = this;
  }
} // class HistoryQosPolicyKind

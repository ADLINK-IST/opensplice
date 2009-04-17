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


public class DestinationOrderQosPolicyKind 
{
  private        int __value;
  private static int __size = 2;
  private static DDS.DestinationOrderQosPolicyKind[] __array = new DDS.DestinationOrderQosPolicyKind [__size];

  public static final int _BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS = 0;
  public static final DDS.DestinationOrderQosPolicyKind BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS = new DDS.DestinationOrderQosPolicyKind(_BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS);
  public static final int _BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS = 1;
  public static final DDS.DestinationOrderQosPolicyKind BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS = new DDS.DestinationOrderQosPolicyKind(_BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS);

  public int value ()
  {
    return __value;
  }

  public static DDS.DestinationOrderQosPolicyKind from_int (int value)
  {
    if (value >= 0 && value < __size)
      return __array[value];
    else
      throw new org.omg.CORBA.BAD_PARAM ();
  }

  protected DestinationOrderQosPolicyKind (int value)
  {
    __value = value;
    __array[__value] = this;
  }
} // class DestinationOrderQosPolicyKind

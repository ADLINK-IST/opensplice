
package DDS;


public class ReliabilityQosPolicyKind 
{
  private        int __value;
  private static int __size = 2;
  private static DDS.ReliabilityQosPolicyKind[] __array = new DDS.ReliabilityQosPolicyKind [__size];

  public static final int _BEST_EFFORT_RELIABILITY_QOS = 0;
  public static final DDS.ReliabilityQosPolicyKind BEST_EFFORT_RELIABILITY_QOS = new DDS.ReliabilityQosPolicyKind(_BEST_EFFORT_RELIABILITY_QOS);
  public static final int _RELIABLE_RELIABILITY_QOS = 1;
  public static final DDS.ReliabilityQosPolicyKind RELIABLE_RELIABILITY_QOS = new DDS.ReliabilityQosPolicyKind(_RELIABLE_RELIABILITY_QOS);

  public int value ()
  {
    return __value;
  }

  public static DDS.ReliabilityQosPolicyKind from_int (int value)
  {
    if (value >= 0 && value < __size)
      return __array[value];
    else
      throw new org.omg.CORBA.BAD_PARAM ();
  }

  protected ReliabilityQosPolicyKind (int value)
  {
    __value = value;
    __array[__value] = this;
  }
} // class ReliabilityQosPolicyKind


package DDS;


public class OwnershipQosPolicyKind 
{
  private        int __value;
  private static int __size = 2;
  private static DDS.OwnershipQosPolicyKind[] __array = new DDS.OwnershipQosPolicyKind [__size];

  public static final int _SHARED_OWNERSHIP_QOS = 0;
  public static final DDS.OwnershipQosPolicyKind SHARED_OWNERSHIP_QOS = new DDS.OwnershipQosPolicyKind(_SHARED_OWNERSHIP_QOS);
  public static final int _EXCLUSIVE_OWNERSHIP_QOS = 1;
  public static final DDS.OwnershipQosPolicyKind EXCLUSIVE_OWNERSHIP_QOS = new DDS.OwnershipQosPolicyKind(_EXCLUSIVE_OWNERSHIP_QOS);

  public int value ()
  {
    return __value;
  }

  public static DDS.OwnershipQosPolicyKind from_int (int value)
  {
    if (value >= 0 && value < __size)
      return __array[value];
    else
      throw new org.omg.CORBA.BAD_PARAM ();
  }

  protected OwnershipQosPolicyKind (int value)
  {
    __value = value;
    __array[__value] = this;
  }
} // class OwnershipQosPolicyKind

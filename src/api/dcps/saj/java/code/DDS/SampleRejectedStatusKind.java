
package DDS;


public class SampleRejectedStatusKind 
{
  private        int __value;
  private static int __size = 4;
  private static DDS.SampleRejectedStatusKind[] __array = new DDS.SampleRejectedStatusKind [__size];

  public static final int _NOT_REJECTED = 0;
  public static final DDS.SampleRejectedStatusKind NOT_REJECTED = new DDS.SampleRejectedStatusKind(_NOT_REJECTED);
  public static final int _REJECTED_BY_INSTANCES_LIMIT = 0;
  public static final DDS.SampleRejectedStatusKind REJECTED_BY_INSTANCES_LIMIT = new DDS.SampleRejectedStatusKind(_REJECTED_BY_INSTANCES_LIMIT);
  public static final int _REJECTED_BY_SAMPLES_LIMIT = 1;
  public static final DDS.SampleRejectedStatusKind REJECTED_BY_SAMPLES_LIMIT = new DDS.SampleRejectedStatusKind(_REJECTED_BY_SAMPLES_LIMIT);
  public static final int _REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT = 1;
  public static final DDS.SampleRejectedStatusKind REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT = new DDS.SampleRejectedStatusKind(_REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT);
  
  public int value ()
  {
    return __value;
  }

  public static DDS.SampleRejectedStatusKind from_int (int value)
  {
    if (value >= 0 && value < __size)
      return __array[value];
    else
      throw new org.omg.CORBA.BAD_PARAM ();
  }

  protected SampleRejectedStatusKind (int value)
  {
    __value = value;
    __array[__value] = this;
  }
} // class SampleRejectedStatusKind

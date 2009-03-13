
package DDS;


public final class TimeBasedFilterQosPolicy 
{
  public DDS.Duration_t minimum_separation = null;

  public TimeBasedFilterQosPolicy ()
  {
  } // ctor

  public TimeBasedFilterQosPolicy (DDS.Duration_t _minimum_separation)
  {
    minimum_separation = _minimum_separation;
  } // ctor

} // class TimeBasedFilterQosPolicy

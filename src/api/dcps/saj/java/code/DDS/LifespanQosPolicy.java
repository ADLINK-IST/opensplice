
package DDS;


public final class LifespanQosPolicy 
{
  public DDS.Duration_t duration = null;

  public LifespanQosPolicy ()
  {
  } // ctor

  public LifespanQosPolicy (DDS.Duration_t _duration)
  {
    duration = _duration;
  } // ctor

} // class LifespanQosPolicy

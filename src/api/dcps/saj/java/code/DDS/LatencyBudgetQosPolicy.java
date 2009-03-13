
package DDS;


public final class LatencyBudgetQosPolicy 
{
  public DDS.Duration_t duration = null;

  public LatencyBudgetQosPolicy ()
  {
  } // ctor

  public LatencyBudgetQosPolicy (DDS.Duration_t _duration)
  {
    duration = _duration;
  } // ctor

} // class LatencyBudgetQosPolicy

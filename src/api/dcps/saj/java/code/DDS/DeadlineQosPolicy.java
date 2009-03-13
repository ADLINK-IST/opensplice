
package DDS;


public final class DeadlineQosPolicy 
{
  public DDS.Duration_t period = null;

  public DeadlineQosPolicy ()
  {
  } // ctor

  public DeadlineQosPolicy (DDS.Duration_t _period)
  {
    period = _period;
  } // ctor

} // class DeadlineQosPolicy

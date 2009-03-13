
package DDS;


public final class ReliabilityQosPolicy 
{
  public DDS.ReliabilityQosPolicyKind kind = null;
  public DDS.Duration_t max_blocking_time = null;

  public ReliabilityQosPolicy ()
  {
  } // ctor

  public ReliabilityQosPolicy (DDS.ReliabilityQosPolicyKind _kind, DDS.Duration_t _max_blocking_time)
  {
    kind = _kind;
    max_blocking_time = _max_blocking_time;
  } // ctor

} // class ReliabilityQosPolicy

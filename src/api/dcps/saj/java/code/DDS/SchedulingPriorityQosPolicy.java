
package DDS;


public final class SchedulingPriorityQosPolicy
{
  public DDS.SchedulingPriorityQosPolicyKind kind = null;

  public SchedulingPriorityQosPolicy ()
  {
  } // ctor

  public SchedulingPriorityQosPolicy (DDS.SchedulingPriorityQosPolicyKind _kind)
  {
    kind = _kind;
  } // ctor

} // class SchedulingPriorityQosPolicy

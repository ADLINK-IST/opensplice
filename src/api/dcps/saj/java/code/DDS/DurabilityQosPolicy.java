
package DDS;


public final class DurabilityQosPolicy 
{
  public DDS.DurabilityQosPolicyKind kind = null;

  public DurabilityQosPolicy ()
  {
  } // ctor

  public DurabilityQosPolicy (DDS.DurabilityQosPolicyKind _kind)
  {
    kind = _kind;
  } // ctor

} // class DurabilityQosPolicy

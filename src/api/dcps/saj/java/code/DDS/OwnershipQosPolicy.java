
package DDS;


public final class OwnershipQosPolicy 
{
  public DDS.OwnershipQosPolicyKind kind = null;

  public OwnershipQosPolicy ()
  {
  } // ctor

  public OwnershipQosPolicy (DDS.OwnershipQosPolicyKind _kind)
  {
    kind = _kind;
  } // ctor

} // class OwnershipQosPolicy

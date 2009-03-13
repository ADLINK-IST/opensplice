
package DDS;


public final class LivelinessQosPolicy 
{
  public DDS.LivelinessQosPolicyKind kind = null;
  public DDS.Duration_t lease_duration = null;

  public LivelinessQosPolicy ()
  {
  } // ctor

  public LivelinessQosPolicy (DDS.LivelinessQosPolicyKind _kind, DDS.Duration_t _lease_duration)
  {
    kind = _kind;
    lease_duration = _lease_duration;
  } // ctor

} // class LivelinessQosPolicy


package DDS;


public final class RequestedIncompatibleQosStatus 
{
  public int total_count = (int)0;
  public int total_count_change = (int)0;
  public int last_policy_id = (int)0;
  public DDS.QosPolicyCount policies[] = null;

  public RequestedIncompatibleQosStatus ()
  {
  } // ctor

  public RequestedIncompatibleQosStatus (int _total_count, int _total_count_change, int _last_policy_id, DDS.QosPolicyCount[] _policies)
  {
    total_count = _total_count;
    total_count_change = _total_count_change;
    last_policy_id = _last_policy_id;
    policies = _policies;
  } // ctor

} // class RequestedIncompatibleQosStatus

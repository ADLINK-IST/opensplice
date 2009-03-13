
package DDS;


public final class DurabilityServiceQosPolicy 
{
  public DDS.Duration_t service_cleanup_delay = null;
  public DDS.HistoryQosPolicyKind history_kind = null;
  public int history_depth = (int)0;
  public int max_samples = (int)0;
  public int max_instances = (int)0;
  public int max_samples_per_instance = (int)0;

  public DurabilityServiceQosPolicy ()
  {
  } // ctor

  public DurabilityServiceQosPolicy (
          DDS.Duration_t _service_cleanup_delay,
          DDS.HistoryQosPolicyKind _history_kind,
          int _history_depth,
          int _max_samples,
          int _max_instances,
          int _max_samples_per_instance
          )
  {
    service_cleanup_delay = _service_cleanup_delay;
    history_kind = _history_kind;
    history_depth = _history_depth;
    max_samples = _max_samples;
    max_instances = _max_instances;
    max_samples_per_instance = _max_samples_per_instance;
  } // ctor

} // class DurabilityServiceQosPolicy

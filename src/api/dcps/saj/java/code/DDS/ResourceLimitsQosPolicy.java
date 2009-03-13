
package DDS;


public final class ResourceLimitsQosPolicy 
{
  public int max_samples = (int)0;
  public int max_instances = (int)0;
  public int max_samples_per_instance = (int)0;

  public ResourceLimitsQosPolicy ()
  {
  } // ctor

  public ResourceLimitsQosPolicy (int _max_samples, int _max_instances, int _max_samples_per_instance)
  {
    max_samples = _max_samples;
    max_instances = _max_instances;
    max_samples_per_instance = _max_samples_per_instance;
  } // ctor

} // class ResourceLimitsQosPolicy

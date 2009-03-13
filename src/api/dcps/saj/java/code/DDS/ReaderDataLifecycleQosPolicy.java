
package DDS;


public final class ReaderDataLifecycleQosPolicy 
{
  public DDS.Duration_t autopurge_nowriter_samples_delay = null;
  public DDS.Duration_t autopurge_disposed_samples_delay = null;
  public boolean enable_invalid_samples = true;

  public ReaderDataLifecycleQosPolicy ()
  {
  } // ctor

  public ReaderDataLifecycleQosPolicy (
      DDS.Duration_t _autopurge_nowriter_samples_delay,
      DDS.Duration_t _autopurge_disposed_samples_delay,
      boolean _enable_invalid_samples )
  {
    autopurge_nowriter_samples_delay = _autopurge_nowriter_samples_delay;
    autopurge_disposed_samples_delay = _autopurge_disposed_samples_delay;
    enable_invalid_samples = _enable_invalid_samples;
  } // ctor

} // class ReaderDataLifecycleQosPolicy

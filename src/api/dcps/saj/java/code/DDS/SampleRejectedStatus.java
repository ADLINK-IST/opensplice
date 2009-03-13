
package DDS;


public final class SampleRejectedStatus 
{
  public int total_count = (int)0;
  public int total_count_change = (int)0;
  public DDS.SampleRejectedStatusKind last_reason = null;
  public long last_instance_handle = (long)0L;

  public SampleRejectedStatus ()
  {
  } // ctor

  public SampleRejectedStatus (int _total_count, int _total_count_change, DDS.SampleRejectedStatusKind _last_reason, long _last_instance_handle)
  {
    total_count = _total_count;
    total_count_change = _total_count_change;
    last_reason = _last_reason;
    last_instance_handle = _last_instance_handle;
  } // ctor

} // class SampleRejectedStatus

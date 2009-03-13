
package DDS;


public final class RequestedDeadlineMissedStatus 
{
  public int total_count = (int)0;
  public int total_count_change = (int)0;
  public long last_instance_handle = (long)0L;

  public RequestedDeadlineMissedStatus ()
  {
  } // ctor

  public RequestedDeadlineMissedStatus (int _total_count, int _total_count_change, long _last_instance_handle)
  {
    total_count = _total_count;
    total_count_change = _total_count_change;
    last_instance_handle = _last_instance_handle;
  } // ctor

} // class RequestedDeadlineMissedStatus

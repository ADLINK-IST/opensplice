
package DDS;


public final class LivelinessLostStatus 
{
  public int total_count = (int)0;
  public int total_count_change = (int)0;

  public LivelinessLostStatus ()
  {
  } // ctor

  public LivelinessLostStatus (int _total_count, int _total_count_change)
  {
    total_count = _total_count;
    total_count_change = _total_count_change;
  } // ctor

} // class LivelinessLostStatus

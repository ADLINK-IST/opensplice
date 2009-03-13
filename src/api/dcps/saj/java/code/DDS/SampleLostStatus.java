
package DDS;


public final class SampleLostStatus 
{
  public int total_count = (int)0;
  public int total_count_change = (int)0;

  public SampleLostStatus ()
  {
  } // ctor

  public SampleLostStatus (int _total_count, int _total_count_change)
  {
    total_count = _total_count;
    total_count_change = _total_count_change;
  } // ctor

} // class SampleLostStatus

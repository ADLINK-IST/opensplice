
package DDS;


public final class Time_t 
{
  public int sec = (int)0;
  public int nanosec = (int)0;

  public Time_t ()
  {
  } // ctor

  public Time_t (int _sec, int _nanosec)
  {
    sec = _sec;
    nanosec = _nanosec;
  } // ctor

} // class Time_t

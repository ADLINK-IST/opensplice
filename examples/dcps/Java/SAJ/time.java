
class time {

    public long _time;

    public time (
	)
    {
	_time = 0L;
    }

    public time (
	long t
	)
    {
	_time = t;
    }

    public void timeGet (
	)
    {
	_time = java.lang.System.nanoTime()/1000L;
    }

    public long get (
	)
    {
	return _time;
    }

    public void set (
	long t
	)
    {
	_time = t;
    }

    public long sub (
        time t
	)
    {
	long nt = _time - t.get();

	return nt;
    }

}

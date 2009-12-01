// time.cs created with MonoDevelop
// User: lina at 3:24 PM 10/13/2009
//
// To change standard headers go to Edit->Preferences->Coding->Standard Headers
//

using System;

namespace PingPong
{
	
	
	public class time
	{

	public long _time;

	public time() {
		_time = 0L;
	}

	public time(long t) {
		_time = t;
	}

	public void timeGet() {
		_time = DateTime.Now.Ticks / 1000L;
	}

	public long get() {
		return _time;
	}

	public void set(long t) {
		_time = t;
	}

	public long sub(time t) {
		long nt = _time - t.get();

		return nt;
	}

}

}

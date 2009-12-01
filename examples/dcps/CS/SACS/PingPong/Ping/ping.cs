	// ping.cs created with MonoDevelop
	// User: lina at 4:13 PM 10/12/2009
	//
	// To change standard headers go to Edit->Preferences->Coding->Standard Headers
	//

	using System;

	namespace PingPong
	{
		
		
		public class ping
		{
			
			public static void Main (String[] args) {

			    pinger pinger_instance = new pinger();

	            pinger_instance.run (args);

		    }
		}
	}

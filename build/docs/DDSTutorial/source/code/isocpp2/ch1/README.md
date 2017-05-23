## Chapter 1 Code Example 

This folder contains the full source for the example shown in the
first chapter of the tutorial.

The example shows how to create a simple application that produces
TempSensorType information and another that consumes it.

## Compiling the example

To compile the example you need:

   - g++ v4.6 or higher (clang++ would also be fine)
   - CMake v2.6 or higher
   - [OpenSplice DDS](http://www.opensplice.org) v6.4 or higher

Once you have installed the required tools simply do:


     $ cmake .
     $ make


## Running the Examples

To run the examples do:

   $ ./tspub 
   USAGE:
	 tspub <sensor-id>

This message reveals that the tspub applications takes a single
argument which represent the sensor-id. Thus you can do:


	 $ ./tspub 18
	 DW << (id = 18, temp = 26.3929, hum = 0.730971, scale = C)
	 DW << (id = 18, temp = 25.223, hum = 0.633334, scale = C)
	 DW << (id = 18, temp = 27.5442, hum = 0.61266, scale = C)
	 DW << (id = 18, temp = 25.7138, hum = 0.741911, scale = C)
	 DW << (id = 18, temp = 26.7616, hum = 0.642391, scale = C)
	 DW << (id = 18, temp = 26.6769, hum = 0.669719, scale = C)
	 DW << (id = 18, temp = 27.1806, hum = 0.647648, scale = C)
	 DW << (id = 18, temp = 26.9862, hum = 0.649711, scale = C)
	 DW << (id = 18, temp = 26.0595, hum = 0.658227, scale = C)


Then you can start the consumer as follows:


	$ ./tssub
	(id = 18, temp = 26.3929, hum = 0.730971, scale = C)
	(id = 18, temp = 25.223, hum = 0.633334, scale = C)
	(id = 18, temp = 27.5442, hum = 0.61266, scale = C)
	(id = 18, temp = 25.7138, hum = 0.741911, scale = C)
	(id = 18, temp = 26.7616, hum = 0.642391, scale = C)
	(id = 18, temp = 26.6769, hum = 0.669719, scale = C)
	(id = 18, temp = 27.1806, hum = 0.647648, scale = C)
	(id = 18, temp = 26.9862, hum = 0.649711, scale = C)
	(id = 18, temp = 26.0595, hum = 0.658227, scale = C)




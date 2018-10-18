.. _`Examples`:

########
Examples
########

Examples are provided to demonstrate the Node.js DCPS features.  

The examples can be found in the following directory:

    *$OSPL_HOME/tools/nodejs/examples*

WHERE:

	Linux

		OSPL_HOME = *INSTALLDIR/ADLINK/Vortex_v2/Device/VortexOpenSplice/6.9.x/HDE/x86_64.linux*

	Windows
  
		OSPL_HOME = *INSTALLDIR\\ADLINK\\Vortex_v2\\Device\\VortexOpenSplice\\6.9.x\\HDE\\x86_64.win64*  

Example Files
*************

Each subfolder consists of a Node.js example project.

.. _`ExamplesTable`:

Examples
========

.. tabularcolumns:: | p{3.3cm} | p{11cm} |
+-------------------+-----------------------------------------------------+
| **Example**       | **Description**                                     |
+===================+=====================================================+
| HelloWorld        | Demonstrates how to read and write a simple         |
|                   | topic in DDS                                        |
|                   |                                                     |
+-------------------+-----------------------------------------------------+
| jsshapes          | Demonstrates how to read and write to a             |
|                   | DDS application                                     |
|                   |                                                     |
+-------------------+-----------------------------------------------------+
| IoTData           | Demonstrates reading and writing an IoTData topic   |
|                   |                                                     |
|                   |                                                     |
+-------------------+-----------------------------------------------------+
| PingPong          | Demonstrates reading, writing, and waitsets         |
|                   |                                                     |
|                   |                                                     |
+-------------------+-----------------------------------------------------+
| QoSExample        | Demonstrates QoS settings                           |
|                   |                                                     |
|                   |                                                     |
+-------------------+-----------------------------------------------------+
| GetSetQoSExample  | Demonstrates getting and setting QoS using          |
|                   | QoS Provider and DCPS api                           |
|                   |                                                     |
+-------------------+-----------------------------------------------------+


.. raw:: latex

    \newpage

File Types
==========

The examples directory contains files of different types.   


============= =========================================================================================
**File Type** **Description**
============= =========================================================================================
js            A program file or script written in JavaScript
package.json  Lists the packages that a project depends on
xml           An XML file that contains one or more Quality of Service (QoS) profiles for DDS entities
idl           An interface description language file used to define topic(s)
============= =========================================================================================

 

Running Examples
****************

To run a Node.js example:

1.  Setup OSPL environment variables

     **Linux**

     - For each example javascript file to be run, open a Linux terminal

     - Navigate to directory containing release.com file (OSPL_HOME)

       */INSTALLDIR/ADLINK/Vortex_v2/Device/VortexOpenSplice/6.9.x/HDE/x86_64.linux*

     - Run release.com (“. release.com”)


     **Windows**

     - For each example javascript file to be run, open a command prompt

     - Navigate to directory containing release.bat file (OSPL_HOME)

       *\\INSTALLDIR\\ADLINK\\Vortex_v2\\Device\\VortexOpenSplice\\6.9.x\\HDE\\x86_64.win64*

     - Run release.bat (“release.bat”), if OSPL environment variables are not set system wide

2. Change directory to the example folder

	Example:

		*$OSPL_HOME/tools/nodejs/examples/HelloWorld*
   

3. Run npm install to install the Node.js DCPS API (vortexdds-x.y.z.tgz) and all other dependencies

	npm install

.. note::

    If you are running the examples from another directory outside the OSPL install, then you will need to manually install the Node.js DCPS API first and then run npm install for the example dependencies as follows:

    npm install $OSPL_HOME/tools/nodejs/vortexdds-x.y.z.tgz
   
    npm install   

4. Run .js file(s) in a terminal/command shell

	Example:

		node HelloWorldSubscriber.js








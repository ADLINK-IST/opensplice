.. _`Examples`:

########
Examples
########

Several examples are provided to demonstrate the Python DCPS features.  

The examples can be found in the following directory:

         - *$OSPL_HOME/tools/python/examples*

Example Files
*************

File Types
==========

The examples directory contains files of different types.  The runnable python script files reference the xml and idl files. 


============= =========================================================================================
**File Type** **Description**
============= =========================================================================================
py            A program file or script written in Python
xml           An XML file that contains one or more Quality of Service (QoS) profiles for DDS entities.
idl           An interface description language file used to define topic(s).
============= =========================================================================================

 


Examples
========

.. _`Examples Table`:
.. tabularcolumns:: | p{3.3cm} | p{11cm} |
+-------------------+-----------------------------------------------------+
| **Example**       | **Description**                                     |
+===================+=====================================================+
| qos_example.py    | An example that demonstrates how to specify QoS     |
|                   | settings using Python DCPS APIs.   Also shows       |
|                   | usage of a waitset.                                 |
|                   |                                                     |
+-------------------+-----------------------------------------------------+
| example1.py       | Dynamic generation of Python Topic classes          |
|                   | using IDL file and name.  Uses a topic with a       |
|                   | sequence.                                           |
|                   |                                                     |
+-------------------+-----------------------------------------------------+
| example2.py       | Dynamic generation of Python Topic classes          |
|                   | using Enumeration and nested modules.               |
|                   |                                                     |
|                   |                                                     |
+-------------------+-----------------------------------------------------+
| example4.py       | Demonstrate finding DDS topics over-the-wire.       |
|                   | Find topics registered by other processes, and      |
|                   | read and write samples to those topics.             |
|                   |                                                     |
+-------------------+-----------------------------------------------------+



Running Examples
****************

To run an example python script:

1. Setup OSPL environment variables. 

   **Linux**

        - source release.com

   **Windows**
        - release.bat

      
2. Run an example python script 

        - > python3 example1.py







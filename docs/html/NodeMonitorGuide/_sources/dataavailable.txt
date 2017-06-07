.. _`Data Available through Node Monitor`:

###################################
Data Available through Node Monitor
###################################

***************
CPU Information
***************

+ Vendor
+ CPU Model
+ CPU Frequency
+ Cache size
+ Number of cores on the CPU

****************************
Operating System Information
****************************

+ OS name
+ Distribution name (*e.g.* Ubuntu 12.04)
+ Architecture of the OS
+ Linux kernel image version
+ Vendor and vendor version
+ Data model (32/64 bit *etc.*)
+ Endianness

*******************
Network Information
*******************

+ Primary interface primary address primary MAC address and primary netmask
+ Domain name
+ Default gateway default gateway interface
+ Primary DNS and secondary DNS

*****************************
Network Interface Information
*****************************

+ Interface name
+ Interface type (*e.g.* Ethernet Loopback)
+ MAC address
+ Destination
+ Broadcast
+ Netmask
+ MTU

*******************
Process Information
*******************

+ Process ID
+ Name
+ Current working directory
+ Priority
+ Number of threads in the process
+ OSPL environment variables (*e.g.* ``OSPL_URI``, ``OSPL_HOME``, *etc.*)

****************************
Network Interface Statistics
****************************

+ RX values: bytes packets errors dropped overruns and frame
+ TX values: bytes packets errors dropped overruns collisions and carrier

*****************
Memory Statistics
*****************

+ Total system memory
+ Total used system memory
+ Total free system memory
+ Actual total used system memory
+ Actual total free system memory
+ Total swap size
+ Total used swap size
+ Total free swap size
+ RAM size in MB

**************
CPU statistics
**************

+ Load and load average
+ Total number of processes in idle state
+ Total number of processes in running state
+ Total number of processes in sleeping state
+ Total number of zombie processes
+ Total number of processes
+ Total number of threads

**************************
Process Memory Statistics:
**************************

+ Process ID
+ Resident memory size used in Bytes
+ Shared memory size used in Bytes
+ Number of minor page faults
+ Number of major page faults
+ Total number of page faults (Major + Minor)
+ Virtual memory size used in Bytes

**********************
Process CPU Statistics
**********************

+ Process ID
+ Percent CPU used
+ Start time in milliseconds (Epoch time)
+ Total CPU time in milliseconds (User + System)
+ User CPU time in milliseconds
+ System CPU time in milliseconds



.. _`Example Process Memory & CPU statistics`:

.. figure:: /images/processInfoStats.png
   :height: 65mm
   :alt: Example Process Memory & CPU statistics

   **Example Process Memory & CPU statistics**







.. |caution| image:: ./images/icon-caution.*
            :height: 6mm
.. |info|   image:: ./images/icon-info.*
            :height: 6mm
.. |windows| image:: ./images/icon-windows.*
            :height: 6mm
.. |unix| image:: ./images/icon-unix.*
            :height: 6mm
.. |linux| image:: ./images/icon-linux.*
            :height: 6mm
.. |c| image:: ./images/icon-c.*
            :height: 6mm
.. |cpp| image:: ./images/icon-cpp.*
            :height: 6mm
.. |csharp| image:: ./images/icon-csharp.*
            :height: 6mm
.. |java| image:: ./images/icon-java.*
            :height: 6mm


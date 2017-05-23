.. _`Configuring Node Monitor`:

########################
Configuring Node Monitor
########################

*********************************
Setting up the configuration file
*********************************

It is possible to control the frequency of the information and metrics that are pushed
to the Vortex OpenSplice backbone. Setting the interval to ``-1`` for a specific category
will disable the push of monitoring data for that category.

To override the default configuration, create your own ``application.conf`` file in
the same directory where you start the Node Monitor.


Configuration parameters
========================

+-------------------------+---------------------------------------------------------+
| ``domain``              | Domain ID of the OpenSplice system.                     |
+-------------------------+---------------------------------------------------------+
| ``partition``           | Partition name to which monitoring data                 |
|                         | will be published. Default is NODE_INFO.                |
|                         | Do not change this when using with Vortex Insight!      |
+-------------------------+---------------------------------------------------------+
| ``historyDepth``        | History depth for ``HistoryQosPolicy``.                 |
|                         | ``-1`` to keep all history.                             |
+-------------------------+---------------------------------------------------------+
| ``cpuInfo``             | CPU information interval in milliseconds.               |
|                         | ``-1`` to disable.                                      |
+-------------------------+---------------------------------------------------------+
| ``cpuStats``            | CPU statistics interval in milliseconds.                |
|                         | ``-1`` to disable.                                      |
+-------------------------+---------------------------------------------------------+
| ``memStats``            | Memory statistics interval in milliseconds.             |
|                         | ``-1`` to disable.                                      |
+-------------------------+---------------------------------------------------------+
| ``netInfo``             | Memory statistics interval in milliseconds.             |
|                         | ``-1`` to disable.                                      |
+-------------------------+---------------------------------------------------------+
| ``netInterfaceInfo``    | Network interface information interval in milliseconds. |
|                         | ``-1`` to disable.                                      |
+-------------------------+---------------------------------------------------------+
| ``netInterfaceStats``   | Network interface statistics interval in milliseconds.  |
|                         | ``-1`` to disable.                                      |
+-------------------------+---------------------------------------------------------+
| ``operatingSystemInfo`` | Operating system information interval in milliseconds.  |
|                         | ``-1`` to disable.                                      |
+-------------------------+---------------------------------------------------------+
| ``processInfo``         | Process information interval in milliseconds.           |
|                         | ``-1`` to disable.                                      |
+-------------------------+---------------------------------------------------------+
| ``processCPUStats``     | Process CPU statistics interval in milliseconds.        |
|                         | ``-1`` to disable.                                      |
+-------------------------+---------------------------------------------------------+
| ``processMemoryStats``  | Process memory statistics interval in milliseconds.     |
|                         | ``-1`` to disable.                                      |
+-------------------------+---------------------------------------------------------+

*************************************************
Default configuration file (``application.conf``)
*************************************************

:: 

   # define default scope variables to be used for setting the value of
   other parameters.
   default {
     domain = 0
     partition = NODE_INFO
     historyDepth = 10
     interval {
       information = 60000
       statistics = 2000
     }
   }

   opensplice {
     hm {
       nodeinfoservice{
         domain = ${default.domain}
         partition = ${default.partition}
         historyDepth = ${default.historyDepth}
         interval {
           cpuInfo = ${default.interval.information}
           cpuStats = ${default.interval.statistics}
           memStats = ${default.interval.statistics}
           netInfo = ${default.interval.information}
           netInterfaceInfo = ${default.interval.information}
           netInterfaceStats = ${default.interval.statistics}
           operatingSystemInfo = ${default.interval.information}
           processCPUStats = ${default.interval.statistics}
           processInfo = ${default.interval.information}
           processMemoryStats = ${default.interval.statistics}
         }
       }
     }
   } 




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


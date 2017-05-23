.. _`ELinOS`:

######
ELinOS
######


*This chapter provides notes about deploying OpenSplice DDS on ELinOS.*


****************
Deployment notes
****************

OpenSplice may be deployed in an ELinOS system.

The following ELinOS features should be enabled:

+ Kernel support for System-V IPC

+ Use full ``shmem`` filesystem

+ ``tmpfs``

The following system libraries are required:

+ ``stdc++``

+ ``pthread``

+ ``rt``

+ ``dl``

+ ``z``

+ ``m``

  
|caution|
  We do not recommend running with less than **32M** of system memory.


***********
Limitations
***********

|caution|
  ELinOS partitions within PikeOS are supported, but DDSI networking will
  not function as it requires a multicast-capable interface.

|info|
  The native networking service is supported in a *broadcast* configuration.




.. |caution| image:: ./images/icon-caution.*
            :height: 6mm
.. |info|   image:: ./images/icon-info.*
            :height: 6mm


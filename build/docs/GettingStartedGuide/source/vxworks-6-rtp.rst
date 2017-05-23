.. _`VxWorks 6.x RTP`:

###############
VxWorks 6.x RTP
###############

*This chapter provides a brief description of how to deploy Vortex OpenSplice
on VxWorks 6.x as a Real Time Process.*


***************************
VxWorks Real Time Processes
***************************

Vortex OpenSplice is deployed on the VxWorks 6.x operating system as Real
Time Processes (RTPs). For more information about RTPs please refer to
WindRiver's VxWorks documentation.


************
Installation
************

The following instructions describe installing Vortex OpenSplice for
VxWorks 6.x on the Windows host environment.

Start the installation process by double-clicking the Vortex OpenSplice 
Host Development Environment (HDE) installer file. Follow the on-screen
instructions and complete the installation. When asked to configure the
installation with a license file, choose *No*. The installer will
create an Vortex OpenSplice entry in *Start > Programs* which contains
links to the OpenSplice tools, documentation, and an Uninstall option.

|caution|
  Please note that WindRiver's Workbench GUI must be run in an environment
  where the OpenSplice variables have already been set. If you chose to
  set the OpenSplice variables globally during the installation stage,
  then Workbench can be run directly. Otherwise, Workbench must be run
  from the Vortex OpenSplice command prompt. Start the command prompt by
  clicking *Start > Programs > Vortex OpenSplice menu entry > Vortex OpenSplice
  command prompt*, then start the Workbench GUI. On VxWorks 6.6 the
  executable is located at

  ``<WindRiver root directory>\workbench-3.0\wrwb\platform\eclipse\wrwb-x86-win32.exe``

  This executable can be found by right-clicking on the WindRiver's
  Workbench *Start* menu item and choosing *Properties*.


***************************
VxWorks Kernel Requirements
***************************

The VxWorks kernel required to support Vortex OpenSplice on VxWorks 6.x is
built using the development kernel configuration profile with the
additional posix thread components enabled. A kernel based on this
requirement can be built within Workbench, by starting the Workbench GUI
and selecting *File > New > VxWorks Image Project*.

Type a name for the project then select the appropriate Board Support
Package and Tool Chain (for example ``cpn805`` and ``gnu``). Leave the
kernel options to be used as blank, and on the *Configuration Profile*
dialog choose ``PROFILE_DEVELOPMENT`` from the drop-down list.

Once the kernel configuration project has been generated, the additional
required functionality can be enabled:

+ POSIX threads (``INCLUDE_POSIX_PTHREADS``)
+ POSIX thread scheduler in RTPs (``INCLUDE_POSIX_PTHREAD_SCHEDULER``)
+ built-in symbol table (``INCLUDE_STANDALONE_SYM_TBL``)

Note that the Workbench GUI should be used to enable these components so
that dependent components are automatically added to the project.


***************************
Deploying Vortex OpenSplice
***************************

.. note: xref to install-configure.rst

As described in the :ref:`Configuration` section, Vortex OpenSplice is started with the
Vortex OpenSplice domain service ``spliced`` and a number of optional services
described within the Vortex OpenSplice configuration file (``ospl.xml``). On
VxWorks 6.x, a Real Time Process for each of these services is deployed
on to the target hardware. The sample ``ospl.xml`` configuration file
provided with the VxWorks 6.x edition of OpenSplice has particular
settings so that these RTPs can operate effectively.

The instructions below describe how to deploy these RTPs using the
Workbench GUI and the Target Server File System (TSFS), although the
processes can be deployed by using commands and other file system types.

**Step 1**

Start the Workbench and create a connection to the target hardware using
the *Remote Systems* view.

**Step 2**

Create a connection to the host machine. In the *Properties* for the
connection, make part of the host's file system available to VxWorks
using the TSFS by specifying both the ``-R`` and ``-RW`` options to ``tgtsvr``.
For example, connecting with the option ``-R c:\x -RW`` will enable
read and write access to the contents of the ``c:\x`` directory from the
target hardware under the mount name ``/tgtsvr``.

**Step 3**

Activate the new connection by selecting it and clicking *Connect*.

**Step 4**

With a connection to the target hardware established, create a new RTP
deployment configuration for the connection by right-clicking on the
connection and choosing *Run > Run RTP on Target...*.

**Step 5**

Create a new configuration for the *spliced* deployment that points to
the ``spliced.vxe`` executable from the OpenSplice installation. The
following parameters should be set in the dialog:

+------------------------+----------------------------------------+
| RTP configuration for spliced.vxe                               |
+========================+========================================+
| Exec Path on Target    | ``/tgtsvr/spliced.vxe``                |
+------------------------+----------------------------------------+
| Arguments              | ``file://tgtsvr/ospl.xml``             |
+------------------------+----------------------------------------+
| Environment            | | ``LD_BIND_NOW=1``                    |
|                        | | ``OSPL_URI=file://tgtsvr/ospl.xml``  |
|                        | | ``PATH=/tgtsvr``                     |
+------------------------+----------------------------------------+
| Priority               | ``100``                                |
+------------------------+----------------------------------------+
| Stack Size             | ``0x10000``                            |
+------------------------+----------------------------------------+



For simplicity it has been assumed that ``spliced.vxe`` and the other
executables (located in the ``bin`` directory of the installation) and
``ospl.xml`` (located in the ``etc/config`` directory of the installation)
have been copied to the directory made available as ``/tgtsvr`` described
above. It is possible, if required, to copy the entire OpenSplice
installation directory to the ``/tgtsvr`` location so that all files are
available, but please be aware that log and information files will be
written to the same ``/tgtsvr`` location when the ``spliced.vxe`` is
deployed.

The screen shot from Workbench in
`Workbench showing spliced deployment configuration`_
shows this configuration.

.. _`Workbench showing spliced deployment configuration`:

.. figure:: /images/VxWorks-splicedConfig.png
   :height: 70mm
   :alt: Workbench showing spliced deployment configuration

   **Workbench showing spliced deployment configuration**

The configuration can be deployed by clicking *Run*, where an RTP for
each service described in the configuration file should be created.
These can be seen in Workbench in the Real Time Processes list for the
target connection. An example is shown below in
`Workbench showing deployed OpenSplice RTPs`_.
(The list may need to be refreshed with the *F5* key.)

Deployment problems are listed in ``ospl-error.txt`` and ``ospl-info.txt``,
which are created in the ``/tgtsvr`` directory if the configuration
described above is used.


.. _`Workbench showing deployed OpenSplice RTPs`:

.. figure:: /images/VxWorks-deployedRTPs.png
   :height: 70mm
   :alt: Workbench showing deployed OpenSplice RTPs

   **Workbench showing deployed OpenSplice RTPs**


*******************
OpenSplice Examples
*******************

.. note: xref to install-configure.rst

PrismTech provides a number of examples both for C and C++ that are
described in the :ref:`Examples` section. These example are provided in the form of
Workbench projects which can be easily built and then deployed on to the
target hardware in a similar process to that described above.

Each project contains a ``README`` file briefly explaining the example and
the parameters required to run it.


Importing Example Projects into Workbench
=========================================

The example projects can be imported into Workbench by clicking
*File > Import... > General > Existing Projects into Workspace*.

In the *Import Projects* dialog, browse to the ``examples`` directory of
the OpenSplice installation. Select the required projects for importing
from the list that Workbench has detected.

Ensure that the *Copy projects into workspace* box is un-checked.

Building Example Projects with Workbench
========================================

Projects in a workspace can be built individually or as a group.

+ Build a single project by selecting it and then clicking
  *Project > Build Project*.

+ Build all projects in the current workspace by clicking
  *Project > Build All*.


Deploying OpenSplice Examples
=============================

The PingPong and the Tutorial examples are run in identical ways with
the same parameters for both C and C++. These should be deployed onto
the VxWorks target with the arguments described in the ``README`` files
for each project.

Deploying PingPong
------------------

The PingPong example consists of the ``ping.vxe`` and ``pong.vxe``
executables. If these executables have been copied to the directory made
available as ``/tgtsvr`` as described in `Deploying OpenSplice DDS`_,
RTP configurations should have the following parameters:

+-----------------------------------+----------------------------------------+
| RTP configuration for pong                                                 |
+===================================+========================================+
| Exec Path on Target               | ``/tgtsvr/pong.vxe``                   |
+-----------------------------------+----------------------------------------+
| Arguments                         | ``PongRead PongWrite``                 |
+-----------------------------------+----------------------------------------+
| Environment                       | | ``LD_BIND_NOW=1``                    |
|                                   | | ``OSPL_URI=file://tgtsvr/ospl.xml``  |
+-----------------------------------+----------------------------------------+
| Priority                          | ``100``                                |
+-----------------------------------+----------------------------------------+
| Stack Size                        | ``0x10000``                            |
+-----------------------------------+----------------------------------------+
  
  
+-----------------------------------+----------------------------------------+
| RTP configuration for ping                                                 |
+===================================+========================================+
| Exec Path on Target               | ``/tgtsvr/ping.vxe``                   |
+-----------------------------------+----------------------------------------+
| Arguments                         | ``10 10 s PongRead PongWrite``         |
+-----------------------------------+----------------------------------------+
| Environment                       | | ``LD_BIND_NOW=1``                    |
|                                   | | ``OSPL_URI=file://tgtsvr/ospl.xml``  |
+-----------------------------------+----------------------------------------+
| Priority                          | ``100``                                |
+-----------------------------------+----------------------------------------+
| Stack Size                        | ``0x10000``                            |
+-----------------------------------+----------------------------------------+


When deployment is successful, the console shows output from both the
``ping`` and ``pong`` executables. The console view can be switched to show
the output for each process by clicking the *Display Selected Console*
button.

Deploying the Chat Tutorial
---------------------------

The Chat Tutorial consists of the ``chatter.vxe``, ``messageboard.vxe`` and
``userload.vxe`` executables. If these executables have been copied to the
directory made available as ``/tgtsvr`` as described in
`Deploying OpenSplice DDS`_, RTP configurations should have the following
parameters:

+-----------------------------------+----------------------------------------+
| RTP configuration for userload                                             |
+===================================+========================================+
| Exec Path on Target               | ``/tgtsvr/userload.vxe``               |
+-----------------------------------+----------------------------------------+
| Arguments                         |                                        |
+-----------------------------------+----------------------------------------+
| Environment                       | | ``LD_BIND_NOW=1``                    |
|                                   | | ``OSPL_URI=file://tgtsvr/ospl.xml``  |
+-----------------------------------+----------------------------------------+
| Priority                          | ``100``                                |
+-----------------------------------+----------------------------------------+
| Stack Size                        | ``0x10000``                            |
+-----------------------------------+----------------------------------------+
  
   
+-----------------------------------+----------------------------------------+
| RTP configuration for messageboard                                         |
+===================================+========================================+
| Exec Path on Target               | ``/tgtsvr/messageboard.vxe``           |
+-----------------------------------+----------------------------------------+
| Arguments                         |                                        |
+-----------------------------------+----------------------------------------+
| Environment                       | | ``LD_BIND_NOW=1``                    |
|                                   | | ``OSPL_URI=file://tgtsvr/ospl.xml``  |
+-----------------------------------+----------------------------------------+
| Priority                          | ``100``                                |
+-----------------------------------+----------------------------------------+
| Stack Size                        | ``0x10000``                            |
+-----------------------------------+----------------------------------------+
  
  
+-----------------------------------+----------------------------------------+
| RTP configuration for chatter                                              |
+===================================+========================================+
| Exec Path on Target               | ``/tgtsvr/chatter.vxe``                |
+-----------------------------------+----------------------------------------+
| Arguments                         | ``1 User1``                            |
+-----------------------------------+----------------------------------------+
| Environment                       | | ``LD_BIND_NOW=1``                    |
|                                   | | ``OSPL_URI=file://tgtsvr/ospl.xml``  |
+-----------------------------------+----------------------------------------+
| Priority                          | ``100``                                |
+-----------------------------------+----------------------------------------+
| Stack Size                        | ``0x10000``                            |
+-----------------------------------+----------------------------------------+
  
  
When deployment is successful, the console will show output from each
RTP. In particular the message board will show the messages sent by the
``chatter`` process. The console view can be switched to show the output
for each process by clicking the *Display Selected Console* button.



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

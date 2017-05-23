.. _`VxWorks 5.5.1`:

#############
VxWorks 5.5.1
#############

*This chapter provides a brief description of how to deploy Vortex OpenSplice
on VxWorks 5.5.1.*


*******************
VxWorks and Tornado
*******************

This chapter provides a brief description of how to build the kernel and
the supplied examples, and how to run those examples, using VxWorks
5.5.1 and the Tornado *'front end'*. For more information about VxWorks
5.5.1 and Tornado, please refer to WindRiver's documentation.

|caution|
  **NOTE:** The examples given here assume that a Solaris-hosted system is
  being used, and that Vortex OpenSplice is installed in
  ``/usr/local/vxworks5.5.1``.

*************************
Building a VxWorks Kernel
*************************

**Required modules**

The following modules are the core system components needed to build the
Vortex OpenSplice runtime. Please refer to WindRiver's documentation for
additional information describing how VxWorks kernels can be built.

+ Operating system components

  - POSIX components
  
    * POSIX timers
    * POSIX threads

  - File System and Disk Utilities
  
    * File System and Disk Utilities


**Additional modules**

The modules listed below are optional but are useful for HDE (Host
Development Environment) development. These modules are required if
deploying from the Tornado front end:

+ Development tool components

  - WDB agent components
  
    * WDB agent services

  - WDB target server file system
  
    * symbol table components


+ Platform-specific Information

  - synchronize host and target symbol tables

  - target shell components
  
    * target shell


**********************************************
Scenarios for Building the OpenSplice Examples
**********************************************

There are two scenarios included for building and deploying the
OpenSplice examples.

+ You can build one DKM (Downloadable Kernel Module) containing the example,
  OpenSplice, and all of its required services and support libraries, as well
  as a default configuration file. *(This is the recommended approach.)*

+ Alternatively, separate DKMs are supplied for each of the OpenSplice libraries and
  services, and each example can be built as a separate DKM (containing only the
  example), which we refer to as *‘AppOnly’* style.



************************************************************************
The OpenSplice Examples (All linked in one complete DKM - *recommended*)
************************************************************************

**To build the standalone C PingPong example**

|c|

At the prompt, ``cd`` to `examples/dcps/PingPong/c/standalone/``
and run ``make``.



Note about the example projects
===============================

The example builds by linking the object produced by compiling the
output of ``osplconf2c`` along with the example application, the ``splice``
deamon, and services enabled in the configuration XML, into one single
downloadable kernel module.

Users producing their own application could of course decide to link the
object and library files into a monolithic kernel image instead.


The osplconf2c tool
===================

``osplconf2c`` is required for example and user applications.

``osplconf2c`` is a tool which processes the OpenSplice configuration
XML, and produces a source file to be compiled and linked into the final
image. It contains the data from the XML file, as well as any
environment variables that you require to configure OpenSplice and
references to the symbols for the entry points of the OpenSplice
services.

Environment variables can be added using the ``-e`` option. For example,
you would use the option ``-e "OSPL_LOGPATH=/xxx/yyy"`` if you wanted the
logs to be placed in ``/xxx/yyy``.

The example ``makefiles`` runs ``osplconf2c`` automatically.




.. _`Overriding OpenSplice configuration at runtime`:

**********************************************
Overriding OpenSplice configuration at runtime
**********************************************

You can override the OpenSplice configuration XML provided to
``osplconf2c`` at runtime by specifying the URI of a file when starting
``ospl_spliced`` on the target. For example:

.. code-block:: bash

  ospl_spliced "file:///tgtsvr/ospl.xml"


It should be noted, however, that the ``osplconf2c`` will have generated
references to the symbols for the services which are specified in the
xml file when it started, and only those services may be used in the new
configuration, as other services will not be included in the image. As
an exception to this, if the ``-d`` option is specified then dynamic
loading is supported, and DKMs for additional services will be
automatically loaded; DKMs for any required 'libraries' must be
pre-loaded by the user.

|caution|
  *NOTE:* Symbol table support will be required in the kernel if the
  ``-d`` option is used. Without the ``-d`` option it should still be possible
  to statically link OpenSplice with a kernel even if symbol table support
  is not included, for example for final deployment.



********************
Running the Examples
********************

If you included the additional modules listed above 
(see `Building a VxWorks Kernel`_) in the kernel, deployment
is done *via* the target server setup from the Tornado
shell connection.


**********
Background
**********

All Vortex OpenSplice tools or services have unique entry points. These
entry points all take a string; the string is parsed into the necessary
arguments and passed on.

To start ``ospl`` on a Unix system, the command would be:

.. code-block:: bash

  ospl start file:///ospl.xml


and on VxWorks:

.. code-block:: bash

  ospl "start file:///ospl.xml"


Note that the arguments are separated by spaces.

Other commands:

.. code-block:: bash

  ospl -> ospl(char *)
  spliced -> ospl_spliced(char *)
  networking -> ospl_networking(char *)
  durability -> ospl_durability(char *)
  cmsoap -> ospl_cmsoap(char *)
  mmstat -> ospl_mmstat(char *)
  shmdump -> ospl_shmdump(char *)

The standard 'main' equivalent entry points are:

.. code-block:: bash

  ospl -> ospl_unique_main(int argc, char ** argv)
  spliced -> ospl_spliced_unique_main(int argc, char ** argv)
  networking -> ospl_networking_unique_main(int argc, char ** argv)
  durability -> ospl_durability_unique_main(int argc, char ** argv)
  cmsoap -> ospl_cmsoap_unique_main(int argc, char ** argv)
  mmstat -> ospl_mmstat_unique_main(int argc, char ** argv)
  shmdump -> ospl_shmdump_unique_main(int argc, char ** argv)

You can use the standard ``argv argc`` version entry when you need to use
arguments with embedded spaces. For example, for ``ospl`` you would use:

.. code-block:: bash

  osplArgs = malloc(12)
  *osplArgs = "ospl"
  *(osplArgs+4) = "start"
  *(osplArgs+8) = "file:///tgtsvr/etc/config/ospl.xml"
  ospl_unique_main (2, osplArgs)

*****************************************
How to start spliced and related services
*****************************************

For the example below the target server filesystem must be mounted as
``/tgtsvr`` on the target.

To start the ``spliced`` service and other additional OpenSplice services
open a *windsh* and enter the following commands.

.. code-block:: bash

  cd "$OSPL_HOME/examples/dcps/PingPong/c/standalone"
  ld 1,0,"sac_pingpong_kernel.out"
  ospl_spliced


Note that ``spliced`` will block when invoked by ``ospl_spliced``
so open a new *windsh* to run the following Pong command:

.. code-block:: bash

  pong ("PongRead PongWrite")


After the Pong application has started you can open another *windsh* and
start Ping. However, if you are running the Ping application on another
target board you must load and start ``spliced`` on that target also, as
described above.

.. code-block:: bash

  ping("100 100 m PongRead PongWrite")
  ping("100 100 q PongRead PongWrite")
  ping("100 100 s PongRead PongWrite")
  ping("100 100 b PongRead PongWrite")
  ping("100 100 f PongRead PongWrite")
  ping("1 10 t PongRead PongWrite")


The ``ospl-info.log`` file can be inspected to check the deployment has
been successful. By default, this is written to the ``/tgtsvr`` directory.

The ``moduleShow`` command can be used within the VxWorks shell to see
that the service modules have loaded, and the ``i`` command
should show that tasks have started for these services.

**********************
The osplconf2c command
**********************

**Usage**

.. code-block:: bash

  osplconf2c -h

  osplconf2c [-d [-x]] [-u <URI>] [-e <env=var> ]... [-o <file>]

**Options**

``-h, -?``
  List available command line arguments and give brief
  reminders of their functions.

``-u <URI>``
  Specifies the configuration file to use
  (default: ``${OSPL_URI}``).

``-o <file>``
  Name of the generated file.

``-e <env=var>``
  Environment setting for configuration of OpenSplice;
  *e.g.* ``-e "OSPL_LOGPATH=/xxx/yyy"``.

``-d``
  Enable dynamic loading.

``-x``
  Exclude xml.

******************************************************************
The OpenSplice Examples (Alternative scenario, with multiple DKMs)
******************************************************************

|caution| 
  Loading separate DKMs is not recommended by PrismTech.



**Note about the example projects**

Please ensure that any services called by a configuration XML contain an
explicit path reference within the command tag; for example:

  ``<Command>/tgtsvr/networking</Command>``



To build the standalone C pingpong example
==========================================

|c|

At the prompt, ``cd`` to ``examples/dcps/PingPong/c/standalone/``
and run

.. code-block:: bash

  make -f Makefile\_AppOnly




How to start spliced and related services
=========================================

To start the ``spliced`` service and other additional OpenSplice services,
load the core OpenSplice shared library that is needed by all Vortex OpenSplice
applications, and then the ``ospl`` utility symbols. This can be done
using a VxWorks shell on as many boards as needed. The ``ospl`` entry
point can then be invoked to start OpenSplice.

.. code-block:: bash

  cd "$OSPL_HOME"
  ld 1,0,"lib/libddscore.so"
  ld 1,0,"bin/ospl"
  os_putenv("OSPL_URI=file:///tgtsvr/etc/config/ospl.xml")
  ospl("start")


|caution|
  Please note that in order to deploy the cmsoap service for use with the
  OpenSplice DDS Tuner, it must be configured in ``ospl.xml`` and the
  libraries named ``libcmxml.so`` and ``libddsrrstorage.so`` must be
  pre-loaded:

.. code-block:: bash

  ld 1,0,"lib/libddscore.so"
  ld 1,0,"lib/libddsrrstorage.so"
  ld 1,0,"lib/libcmxml.so"
  ld 1,0,"bin/ospl"
  os_putenv("OSPL_URI=file:///tgtsvr/etc/config/ospl.xml")
  os_putenv("PATH=/tgtsvr/bin")
  ospl("start")



To run the C PingPong example from winsh
----------------------------------------

|c|

After the ``spliced`` and related services have started, you can start
Pong:

.. code-block:: bash

  cd "$OSPL_HOME"
  ld 1,0,"lib/libdcpsgapi.so"
  ld 1,0,"lib/libdcpssac.so"
  cd "examples/dcps/PingPong/c/standalone"
  ld 1,0,"sac_pingpong_kernel_app_only.out"
  pong("PongRead PongWrite")


After the Pong application has started you can open another *windsh* and
start Ping. However, if you are running the Ping application on another
target board you must load and start ``spliced`` on that target also, as
described above.

.. code-block:: bash

  ping("100 100 m PongRead PongWrite")
  ping("100 100 q PongRead PongWrite")
  ping("100 100 s PongRead PongWrite")
  ping("100 100 b PongRead PongWrite")
  ping("100 100 f PongRead PongWrite")
  ping("1 10 t PongRead PongWrite")


The ``ospl-info.log`` file can be inspected to check the deployment has
been successful. By default, this is written to the ``/tgtsvr`` directory.

The ``moduleShow`` command can be used within the VxWorks shell to see
that the service modules have loaded, and the ``i`` command
should show that tasks have started for these services.


Load-time Optimisation: pre-loading OpenSplice Service Symbols
==============================================================

Loading ``spliced`` and its services may take some time if done exactly as
described above. This is because the service Downloadable Kernel Modules
(DKM) and entry points are dynamically loaded as required by OpenSplice.

|info|
  It has been noted that the deployment may be slower when the symbols are
  dynamically loaded from the Target Server File System. However, it is
  possible to improve deployment times by optionally pre-loading service
  symbols that are known to be deployed by OpenSplice.

In this case OpenSplice will attempt to locate the entry point symbols
for the services and invoke those that are already available. This
removes the need for the dynamic loading of such symbols and can equate
to a quicker deployment. When the entry point symbols are not yet
available ( *i.e.* services have not been pre-loaded), OpenSplice will
dynamically load the services as usual.

For example, for an OpenSplice system that will deploy ``spliced`` with
the networking and durability services, the following commands could be
used:

.. code-block:: bash

  cd "$OSPL_HOME"
  ld 1,0,"lib/libddscore.so"
  ld 1,0,"bin/ospl"
  ld 1,0,"bin/spliced"
  ld 1,0,"bin/networking"
  ld 1,0,"bin/durability"
  os_putenv("OSPL_URI=file:///tgtsvr/etc/config/ospl.xml")
  os_putenv("PATH=/tgtsvr/bin")
  ospl("start")


The ``ospl-info.log`` file describes whether entry point symbols are
resolved having been pre-loaded, or the usual dynamic symbol loading is
required.


Notes
=====

In this scenario ``osplcon2c`` has been used with the ``-x`` and ``-d``
options to create an empty configuraion which allows dynamic loading,
and the resulting object has been included in the provided ``libddsos.so``.

If desired the end user could create a new ``libddsos.so`` based on
``libddsos.a`` and a generated file from ``osplconf2c`` without the ``-x``
option, in order to statically link some services but also allow dynamic
loading of others if the built-in xml is later overridden with a file
URI. (See `Overriding OpenSplice configuration at runtime`_.)



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


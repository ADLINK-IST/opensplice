.. _`VxWorks 6.x Kernel`:

#######################
VxWorks 6.x Kernel Mode
#######################

*This chapter provides a brief description of how to build the kernel and the supplied
examples, and how to run those examples, using VxWorks 6.x kernel and the
Workbench front end. For more information about VxWorks 6.x, please refer to
WindRiver’s documentation.*


***************************
VxWorks Kernel Requirements
***************************

The VxWorks kernel required to support Vortex OpenSplice on VxWorks 6.x is built
using the development kernel configuration profile with the additional posix thread
components enabled. A kernel based on this requirement can be built within
Workbench, by starting the Workbench GUI and choosing *File > New >
VxWorks Image Project*.


***************************
Deploying Vortex OpenSplice
***************************

Type a name for the project then select the appropriate Board Support Package
and Tool Chain (for example ``pcPentium4`` and ``gnu``).

Leave all of the kernel options to be used blank except for the *SMP* option,
which must match the Vortex OpenSplice build you are working with.

|caution|
  The *SMP* option must *only* be checked for SMP builds of OpenSplice.
  
On the *Configuration Profile* dialog choose ``PROFILE_DEVELOPMENT`` from
the drop-down list.
  
Once the kernel configuration project has been generated, the additional required
functionality can be enabled:

+ POSIX threads (``INCLUDE_POSIX_PTHREADS``)
+ built-in symbol table (``INCLUDE_STANDALONE_SYM_TBL``)
+ synchronize host and target symbol tables
+ target shell components

  - target shell

To successfully complete the C++ examples you will also require

+ C++ components > standard library (``FOLDER_CPLUS_STDLIB``)

Note that the Workbench GUI should be used to enable these components so that
dependent components are automatically added to the project.


Special notes for this platform
===============================

|caution|
  If any kernel tasks which will call onto OpenSplice API’s are to be created before
  ``ospl_spliced`` is started then the user must ensure that the function
  ``os_procInstallHook`` (which takes no parameters) is called *before* they are
  started. There only needs to be one call to ``os_procInstallHook``; however,
  mutiple calls are harmless.

*******************
OpenSplice Examples
*******************

.. note: xref to install-configure.rst

PrismTech provides the *pingpong* example both for C and C++ that are described
in the :ref:`Examples` section. These example are provided in the form of
Workbench projects which can be easily built and then deployed on to the target
hardware in a similar process to that described above.

Each project contains a ``README`` file briefly explaining the example and the
parameters required to run it.

Importing Example Projects into Workbench
=========================================

The example projects can be imported into Workbench by choosing
*File > Import... > General > Existing Projects into Workspace*.

In the *Import Projects* dialog, browse to the ``examples`` directory of the OpenSplice
installation. Select the required projects for importing from the list that Workbench
has detected.

Ensure that the *Copy projects into workspace* box is un-checked.

Building Example Projects with Workbench
========================================

Projects in a workspace can be built individually or as a group.

+ Build a single project by selecting it and then clicking
  *Project > Build Project*.
+ Build all projects in the current workspace by clicking
  *Project > Build All*.


*********************************************************************
Running the Examples (All linked in one complete DKM – *recommended*)
*********************************************************************

**Scenarios for building the OpenSplice examples**

There are two included scenarios for build and deployment of the OpenSplice
examples.

+ You can build one DKM (Downloadable Kernel Module) containing the example,
  OpenSplice, and all of its required services and support libraries, as well
  as a default configuration file. *(This is the recommended approach.)*

+ Alternatively, separate DKMs are supplied for each of the OpenSplice libraries and
  services, and each example can be built as a separate DKM (containing only the
  example), which we refer to as *‘AppOnly’* style.


Running the examples on two targets
===================================

The C pingpong example
----------------------

|c|

**Step 1**

Right-click on ``wb_sac_pingpong_kernel`` and then choose *Rebuild Build
Project*.

**Step 2**

Next configure the targets to use the target server filesystem, mapped as on the
target as ``/tgtsvr``.

**Step 3**

Copy the newly-built
``wb_sac_pingpong_kernel/PENTIUM4gnu/sac_pingpong_kernel/Debug/sac_pingpong_kernel.out``
to the target server for each board as ``sac_pingpong_kernel.out``.

**Step 4**

Open a target shell connection to each board and in the C mode shell run:

.. code-block:: bash

  ld 1,0,"/tgtsvr/sac_pingpong_kernel.out"
  ospl_spliced

**Step 5**

Open another target shell connection to one board and run:

.. code-block:: bash

  pong "PongRead PongWrite"

**Step 6**

Open another target shell on the other board and run:

.. code-block:: bash

  ping "100 100 m PongRead PongWrite"
  ping "100 100 q PongRead PongWrite"
  ping "100 100 s PongRead PongWrite"
  ping "100 100 b PongRead PongWrite"
  ping "100 100 f PongRead PongWrite"
  ping "1 10 t PongRead PongWrite"


The C++ pingpong example
------------------------

|cpp|

**Step 1**

Right-click on ``wb_sacpp_pingpong_kernel`` and then choose *Rebuild Build
Project*.

**Step 2**

Next configure the targets to use the target server filesystem, mapped as on the
target as ``/tgtsvr``.

**Step 3**

Copy the newly-built
``wb_sacpp_pingpong_kernel/PENTIUM4gnu/sacpp_pingpong_kernel/Debug/sac_pingpong_kernel.out``
to the target server for each board as ``sacpp_pingpong_kernel.out``.

**Step 4**

Open a target shell connection to each board and in the C mode shell run:

.. code-block:: bash

  ld 1,0,"/tgtsvr/sacpp_pingpong_kernel.out"
  ospl_spliced
  
**Step 5**  

Open another target shell connection to one board and run:

.. code-block:: bash

  pong "PongRead PongWrite"

**Step 6**

Open another target shell on the other board and run:

.. code-block:: bash

  ping "100 100 m PongRead PongWrite"
  ping "100 100 q PongRead PongWrite"
  ping "100 100 s PongRead PongWrite"
  ping "100 100 b PongRead PongWrite"
  ping "100 100 f PongRead PongWrite"
  ping "1 10 t PongRead PongWrite"


Running the examples on one target
==================================

The C pingpong example
----------------------

|c|

**Step 1**

Right-click on ``wb_sac_pingpong_kernel`` and then choose *Rebuild Build
Project*.

**Step 2**

Next configure the targets to use the target server filesystem, mapped as on the
target as ``/tgtsvr``.

**Step 3**

Copy the newly-built
``wb_sac_pingpong_kernel/PENTIUM4gnu/sac_pingpong_kernel/Debug/sac_pingpong_kernel.out``
to the target server as ``sac_pingpong_kernel.out``.

**Step 4**

Open a target shell connection and in the C mode shell run:

.. code-block:: bash

  ld 1,0,"/tgtsvr/sac_pingpong_kernel.out"
  ospl_spliced

**Step 5**

Open another target shell connection and run:

.. code-block:: bash

  pong "PongRead PongWrite"

**Step 6**

Open another target shell and run:

.. code-block:: bash

  ping "100 100 m PongRead PongWrite"
  ping "100 100 q PongRead PongWrite"
  ping "100 100 s PongRead PongWrite"
  ping "100 100 b PongRead PongWrite"
  ping "100 100 f PongRead PongWrite"
  ping "1 10 t PongRead PongWrite"

The C++ pingpong example
------------------------

|cpp|

**Step 1**

Right-click on ``wb_sacpp_pingpong_kernel`` and then choose *Rebuild Build
Project*.

**Step 2**

Next configure the targets to use the target server filesystem, mapped as on the
target as ``/tgtsvr``.

**Step 3**

Copy the newly-built
``wb_sacpp_pingpong_kernel/PENTIUM4gnu/sacpp_pingpong_kernel/Debug/sacpp_pingpong_kernel.out``
to the target server as ``sacpp_pingpong_kernel.out``.

**Step 4**

Open a target shell connection and in the C mode shell run:

.. code-block:: bash

  ld 1,0,"/tgtsvr/sacpp_pingpong_kernel.out"
  ospl_spliced
  
Open another target shell connection and run:
pong "PongRead PongWrite"

.. code-block:: bash

  Open another target shell and run:
  ping "100 100 m PongRead PongWrite"
  ping "100 100 q PongRead PongWrite"
  ping "100 100 s PongRead PongWrite"
  ping "100 100 b PongRead PongWrite"
  ping "100 100 f PongRead PongWrite"
  ping "1 10 t PongRead PongWrite"


Using a different path
======================

|caution|
  If you want or need to use a path other than ``/tgtsvr`` (e.g. if you are using a
  different filesystem) then you need to change the path set by the ``-e`` options of
  ``osplconf2c`` in the ``.wrmakefile``.

|info|
  You can also set other environment variables with additional ``-e`` options.

Note about the example projects
===============================

The example builds by linking the object produced by compling the output of
``osplconf2c`` along with the example application, the ``splice`` deamon, and services
enabled in the configuration XML, into one single downloadable kernel module.
Users producing their own application could of course decide to link the object and
library files into a monolithic kernel image instead.


NOTE for VxWorks kernel mode builds of OpenSplice the single process feature of the OpenSplice domain must not be enabled. i.e. "<SingleProcess>true</SingleProcess>" must not be included in the OpenSplice Configuration xml. The model used on VxWorks kernel builds is always that an area of kernel memory is allocated to store the domain database ( the size of which is controlled by the size option in the Database configuration for opensplice as is used on other platforms for the shared memory model. ) This can then be accessed by any task on the same VxWorks node.

*********************************************************************************
Running the Examples (Alternative scenario, with multiple DKMs – ‘AppOnly’ style)
*********************************************************************************

|caution|
  Loading separate DKMs is not recommended by PrismTech.

|info| |cpp|
  *NOTE:* There are no C++ examples provided for the AppOnly style and there is no
  ``libdcpssacpp.out`` DKM because VxWorks only supports C++ modules that are
  self-contained. However, it should still be possible to link your C++ application
  with the ``libdcpssacpp.a``, and then load the complete DKM after the other
  OpenSplice DKMs.

The C pingpong example
======================

|c|

**Step 1**

Right-click on ``wb_sac_pingpong_kernel_app_only`` for the C example or
``wb_sacpp_pingpong_kernel_app_only`` for C++, then choose
*Rebuild Project*.

**Step 2**

Next configure the targets to use the target server filesystem, mapped on the target
as ``/tgtsvr`` (use different host directories for each target).

**Step 3**

Copy the ``ospl.xml`` file from the distribution to the target server directories, and
adjust for your desired configuration.

**Step 4**

Copy all the services from the ``bin`` directory in the distribution to the target server
directories (for example, ``spliced.out``, ``networking.out``, *etc.*).

To run the examples on two targets, start the OpenSplice daemons on each target.

**Step 5**

Open a *Host Shell* (``windsh``) connection to each board, and in the C mode shell
enter:

.. code-block:: bash

  cd "<path to opensplice distribution>"
  ld 1,0,"lib/libddscore.out"
  ld 1,0,"bin/ospl.out"
  os_putenv("OSPL_URI=file:///tgtsvr/ospl.xml")
  os_putenv("OSPL_LOGPATH=/tgtsvr")
  os_putenv("PATH=/tgtsvr/")
  ospl("start")

Please note that in order to deploy the ``cmsoap`` service for use with the OpenSplice
DDS Tuner, it must be configured in ``ospl.xml`` and the libraries named
``libcmxml.out`` and ``libddsrrstorage.out`` must be pre-loaded:

.. code-block:: bash

  cd "<path to opensplice distribution>"
  ld 1,0,"lib/libddscore.out"
  ld 1,0,"lib/libddsrrstorage.out"
  ld 1,0,"lib/libcmxml.out"
  ld 1,0,"bin/ospl.out"
  os_putenv("OSPL_URI=file:///tgtsvr/ospl.xml")
  os_putenv("OSPL_LOGPATH=/tgtsvr")
  os_putenv("PATH=/tgtsvr/")
  ospl("start")


**Step 6**

To load and run the examples:

|c|
  For the C example:

.. code-block:: bash

  ld 1,0,"lib/libdcpsgapi.out"
  ld 1,0,"lib/libdcpssac.out"
  cd "examples/dcps/PingPong/c/standalone"
  ld 1,0,"sac_pingpong_kernel_app_only.out"


**Step 7**

Open a new *Host Shell* connection to one board and run:

.. code-block:: bash

  pong "PongRead PongWrite"

**Step 8**

Open another new *Host Shell* on the other board and run:

.. code-block:: bash

  ping "100 100 m PongRead PongWrite"
  ping "100 100 q PongRead PongWrite"
  ping "100 100 s PongRead PongWrite"
  ping "100 100 b PongRead PongWrite"
  ping "100 100 f PongRead PongWrite"
  ping "1 10 t PongRead PongWrite"


Running the examples on one target
==================================

Proceed as described in the section above, but make all windsh connections to one
board, and only load and run ospl once.

Load-time Optimisation: pre-loading OpenSplice Service Symbols
--------------------------------------------------------------

Loading spliced and its services may take some time if done exactly as described
above. This is because the service DKMs (Downloadable Kernel Modules) and
entry points are dynamically loaded as required by OpenSplice.

|info|
  It has been noted that the deployment may be slower when the symbols are
  dynamically loaded from the Target Server File System. However, it is possible to
  improve deployment times by pre-loading the symbols for the services that are
  required by OpenSplice.

On startup, OpenSplice will attempt to locate the entry point symbols for the
services and invoke them. This removes the need for the dynamic loading of the
DKMs providing the symbols, and can equate to a quicker deployment. Otherwise,
OpenSplice will dynamically load the service DKMs.

For example, for an OpenSplice system that will deploy spliced with the networking
and durability services, the following commands could be used:

.. code-block:: bash

  cd "<path to opensplice distribution>"
  ld 1,0,"lib/libddscore.out"
  ld 1,0,"bin/ospl.out"
  ld 1,0,"bin/spliced.out"
  ld 1,0,"bin/networking.out"
  ld 1,0,"bin/durability.out"
  os_putenv("OSPL_URI=file:///tgtsvr/ospl.xml")
  os_putenv("PATH=/tgtsvr/bin")
  os_putenv("OSPL_LOGPATH=/tgtsvr")
  ospl("start")

The ``ospl-info.log`` file records whether entry point symbols were pre-loaded, or
a DKM has been loaded.

Notes
-----

|info|

.. note: xref to vxworks-551.rst

In this scenario ``osplconf2c`` has been used with the ``-x`` and ``-d`` options to create an
empty configuraion which allows dynamic loading. The resulting object has been
included in the supplied ``libddsos.out``. If desired, the end user could create a new
``libddsos.out`` based on ``libddsos.a`` and a generated file from osplconf2c
without the ``-x`` option, in order to statically link some services, but also allow
dynamic loading of others if the built-in xml is later overridden using a file URI.
(See :ref:`Overriding OpenSplice configuration at runtime`.)

The osplconf2c tool
===================

``osplconf2c`` is required for example and user applications.
``osplconf2c`` is a tool which processes the OpenSplice configuration XML, and
produces a source file to be compiled and linked into the final image. It contains the
data from the XML file, as well as any environment variables that you require to
configure OpenSplice and references to the symbols for the entry points of the
OpenSplice services.

Environment variables can be added using the ``-e`` option. For example, you would
use the ``-e "OSPL_LOGPATH=/xxx/yyy"`` option if you wanted the logs to be
placed in ``/xxx/yyy``.

``osplconf2c`` is run automatically by the example projects.

Overriding OpenSplice configuration at runtime
----------------------------------------------

You can override the OpenSplice configuration XML provided to ``osplconf2c`` at
runtime by specifying the URI of a file when starting ``ospl_spliced`` on the target;
for example:
``ospl_spliced "file:///tgtsvr/ospl.xml"``

|info|
  It should be noted, however, that the ``osplconf2c`` will have generated references to
  the symbols for the services which are specified in the xml file when it started, and
  only those services may be used in the new configuration, as other services will not
  be included in the image.

The osplconf2c command
----------------------

**Usage**

.. code-block:: bash

  osplconf2c -h
  
  osplconf2c [-u <URI>] [-e <env=var> ]... [-o <file>]


**Options**

``-h, -?``
  List available command line arguments and give brief reminders of
  their functions.
  
``-u <URI>``
  Identifies the configuration file to use (default: ``${OSPL_URI}``).
  
``-o <file>``
  Name of the generated file.
  
``-e <env=var>``
  Environment setting for configuration of OpenSplice
  e.g. ``-e "OSPL_LOGPATH=/xxx/yyy"``



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


.. _`Integrity`:

#########
Integrity
#########

*This chapter provides a brief description of how to deploy Vortex OpenSplice
on Integrity.*


***********************
Integrity and GHS Multi
***********************

The ``ospl_projgen`` tool is in the ``HDE/bin`` directory of the DDS
distribution. It is a convenience tool provided for the Integrity
platform in order to aid in the creation of GHS Multi projects for
running the DDS-supplied PingPong example, the Touchstone performance
suite, and the Chatter Tutorial. If desired, these generated projects
can be adapted to suit user requirements by using Multi and the
``ospl_xml2int`` tool, which is also described in this chapter.


************************
The ospl_projgen Command
************************

**Usage**

.. code-block:: bash

  ospl_projgen -h

  ospl_projgen [-s <flash\|ram>\|-d] [-n] [-v] [-t <target>]
               [-l <c\|c++>\|c++onc] [-u <URI>] -b <bsp name>
               [-m <board model>] -o <directory> [-f]

Arguments shown between square brackets *[ ]* are optional; other
arguments are mandatory. Arguments may be supplied in any order. All of
the arguments are described in detail below.

**Arguments**

``-h``
  List the command line arguments and give brief reminders of their
  functions.

``-s <flash | ram>``
  Use this argument if you wish to generate a project
  that will be statically linked with the kernel. The two options for this
  argument determine whether the resulting kernel image will be a
  flashable image or a loadable image. If both this argument and the ``-d``
  argument are omitted the default of a statically-linked ram-loadable
  image will be generated.

``-d``
  Use this argument to produce a project file that will yield a
  dynamic download image.

|caution|
  Arguments ``-s`` and ``-d`` are mutually exclusive.

``-n``
  Use this argument if you want to include the GHS network stack in
  your project.

``-v``
  Use this argument if you want to include filesystem support in
  your project.

``-t <target>``
  Use this argument to specify which address spaces to
  include in your project. Use ``-t list`` to show a list of available
  targets. (Targets available initially are examples supplied with
  Vortex OpenSplice and Integrity itself.)

``-l <c | c++ | c++onc>``
  Use this argument to specify the language for
  your project. The default is ``c++``.

``-u <URI>``
  Use this argument to identify which configuration file to
  use. You can omit this argument if you have the environment variable
  ``OSPL_URI`` set, or use it if you want to use a different configuration
  file from the one referred to by ``OSPL_URI``. The default is
  ``$OSPL_URI``. The ``xml2int`` tool uses this configuration file when
  generating the Integrate file for your project.

``-b <bsp name>``
  Use this argument to specify the BSP name of your
  target board. Use ``-b list`` to show a list of supported target boards.

``-m <board model>``
  Use this argument to specify the model number for
  the target board. Use ``-b <bsp name> -m list`` to show a list of
  supported model numbers. (There are no separate model numbers for
  *pcx86* boards.)

``-o <directory>``
  Use this argument to specify the output directory for
  the project files. The name you supply here will also be used as the
  name for the image file that will be downloaded/flashed onto the
  Integrity board.

``-f``
  Use this argument to force overwrite of the output directory.

When you run the tool, the output directory specified with the ``-o``
argument will be created. Go into this directory, run GHS Multi, and
load the generated project.

If the output directory already exists and the *-f* argument has been
omitted, *ospl_projgen* will exit without generating any code and will
notify you that it has stopped.

|caution|
  *NOTE:* The *NetworkInterfaceAddress* configuration parameter is
  **required** for Integrity nodes which have more than one ethernet
  interface, as it is not possible to determine which are
  broadcast/multicast enabled. (See sections *3.5.2.1* and *3.9.2.1*
  *Element NetworkInterfaceAddress* in the *Vortex Deployment Guide*.)

Using mmstat and shmdump diagnostic tools on Integrity
======================================================

When ``mmstat`` or ``shmdump`` targets are specified to ``ospl_projgen`` an
address space will be added to the generated project. There will also be
an appropriate ``mmstat.c`` or ``shmdump.c`` file generated into the
project. In order to configure these, the command line arguments can be
edited in the generated ``.c`` files. The ``mmstat`` tool can be controlled
*via* telnet on port ``2323`` (by default).


****************
PingPong Example
****************

.. note: xref to install-configure.rst

(Please refer to the :ref:`Examples` section for a description of this example
application.)

|cpp|

To generate a project for the C++ PingPong example, follow these steps:

**Step 1**

The ``I_INSTALL_DIR`` environment variable must be set to point to the
Integrity installation directory on the host machine before running
``ospl_projgen``. For example:

.. code-block:: bash

  export I_INSTALL_DIR=/usr/ghs/int509

**Step 2**

Navigate to the ``examples/dcps/standalone/C++/PingPong`` directory

**Step 3**

Run ``ospl_projgen`` with the following arguments:

.. code-block:: bash

  ospl_projgen -s ram -v -n -t pingpong -l c++ -b pcx86 -o projgen

**Step 4**

Go into the ``projgen`` directory, which contains ``default.gpj`` and a
``src`` directory. (``default.gpj`` is the default Multi project that will
build all the sub-projects found in the ``src`` directory, and the ``src``
directory contains all the sub-projects and generated files produced by
the tool.)

**Step 5**

Start Multi:

.. code-block:: bash

  multi default.gpj
  
You should see a screen similar to the screenshot
`Integrity: project defaults`_ below:

.. _`Integrity: project defaults`:

.. figure:: /images/Integrity-projectDefaults.png
   :height: 70mm
   :alt: Integrity: project defaults

   **Integrity: project defaults**

If no changes are required to the project, right-click on ``default.gpj``
and then click *Build* to build the project.

Upon successful completion of the build process, an image is generated
(in our case called ``projgen``) in the ``src`` directory and you are now
ready to either dynamically download the resulting image to the board or
to load the kernel image onto the board (depending on the arguments you
have specified) and run the PingPong example.

If ``ospl_projgen`` is run and the project built as described above, the
generated image will contain:

+ GHS Integrity OS (Kernel, Networking, and Filesystem support)
+ Vortex OpenSplice (including ``spliced`` and the services described in the
  ``ospl.xml`` file)
+ The PingPong example

Once the image has been downloaded to the board, the ``pong`` "Initial
task" should be started and then the ``ping`` AddressSpace can be started
in the same way, so that the example begins the data transfer.
Parameters are not required to be passed to the Integrity processes
because the ``ospl_projgen`` tool generates code with particular values
that simulate the passing of parameters.

|info|
  This also applies to the Chat Tutorial (see :ref:`Examples`), if
  ``ospl_projgen`` is run with the ``-t chat`` argument.

.. note: xref to install-configure.rst above


***********************************
Changing the ospl_projgen Arguments
***********************************

If changes are subsequently required to the arguments that were
originally specified to the ``ospl_projgen`` tool, there are two choices:

a) Re-run the tool and amend the arguments accordingly

*or*

b) Make your changes through the Multi tool.

The first method guarantees that your project files will be produced
correctly and build without needing manual changes to the project files.
To use this method, simply follow the procedure described above but
supply different arguments.

The second method is perhaps a more flexible approach, but as well as
making some changes using Multi you will have to make other changes by
hand in order for the project to build correctly.

The following section describes the second method.


Changing the generated OpenSplice DDS project using *Multi*
===========================================================

You can make changes to any of the settings you specified with
``ospl_projgen`` by following these steps:

**Step 1**

Right-click on the highlighted ``ospl.xml`` file (as shown above) and
click *Set Options...*.

**Step 2**

Select the *All Options* tab and expand the *Advanced* section.

**Step 3**

Select *Advanced OpenSplice DDS XML To Int Convertor Options*. In the
right-hand pane you will see the options that you have set with the
``ospl_projgen`` tool with their values, similar to
`Integrity: changing project options in Multi`_ below.


.. _`Integrity: changing project options in Multi`:

.. figure:: /images/Integrity-projectOptionsMulti.png
   :height: 70mm
   :alt: Integrity: changing project options in Multi

   **Integrity: changing project options in Multi**

Right-click on the parameter that you want to change. For example, if
you don't need filesystem support to be included in the kernel image,
right-click on *Include filesystem support* and set the option to ``Off``.

The arguments for ``xml2int`` in the bottom pane are updated to reflect
any changes that you make. If you switch off filesystem support, the
``-v`` argument is removed from the arguments. (The ``xml2int`` tool is used
to generate the ``ospl.int`` Integrate file that will be used during the
Integrate phase of the project. For more information on ``xml2int``
please see section `The ospl_xml2int command`_.)

|caution|
  Note that if you do remove filesystem support from the kernel image you
  should also remove all references to the ``ivfs`` library, and make
  appropriate changes to the ``ospl_log.c`` file as well. See section
  `Amending OpenSplice DDS Configuration with Multi`_ for information
  about ``ospl_log.c``.

Similarly you can change any other option and the changes are applied
instantly.

When the changes are complete, rebuild the project by right-clicking on
*default.gpj* and then click *Build* to build the project.



The ospl_xml2int Tool
=====================

The ``ospl_xml2int`` tool is used to inspect your OpenSplice DDS
configuration file (``ospl.xml``) and generate an appropriate Integrate
file (``ospl.int``). For more information on Integrate files please
consult the Integrity manual.


The ospl_xml2int command
========================

**Usage**

.. code-block:: bash

  ospl_xml2int -h

  ospl_xml2int [-s|-d] [-v] [-n] [-t <target>] [-u <URI>]
               [-o <file>]

Arguments shown between square brackets *[ ]* are optional; other
arguments are mandatory. Arguments may be supplied in any order. All of
the arguments are described in detail below.

**Arguments**

``-h``
  List the command line arguments and give brief reminders of their
  functions.

``-s``
  Generate for static linkage with kernel.

``-d``
  Use this argument to generate an Integrate file that will yield
  a dynamic download image. If both this argument and the ``-s`` argument are
  omitted the default of a statically-linked image will be generated.

|caution|
  The arguments ``-s`` and ``-d`` are mutually exclusive.

``-v``
  Include filesystem support.

``-n``
  Include network support.

``-t <target>``
  Available targets:
     | ``chat`` include chat tutorial
     | ``pingpong`` include PingPong example
     | ``touchstone`` include Touchstone
     | ``mmstat`` include mmstat
     | ``shmdump`` include shmdump
      
  Multiple ``-t`` arguments may be given. This enables you to use
  ``mmstat`` and/or ``shmdump`` (see 
  `Using mmstat and shmdump diagnostic tools on Integrity`_)
  in conjunction with one of the examples.

``-u <URI>``
  Identifies the configuration file to use (default:
  ``${OSPL_URI}``).

``-o <file>``
  Name of the generated Integrate file.

Applications linking with OpenSplice DDS must comply with the following
requirements:

+ The ``First`` and ``Length`` parameters must match those of ``spliced``
  address space (these are generated from ``ospl.xml``).

+ The address space entry for your application in the Integrate file must
  include entries as shown in the example below.

Have a look at the ``ospl.int`` for the PingPong example if in doubt as to
what the format should be. (Make sure that you have built the project
first or else the file will be empty.)

Example ``ospl.int`` contents:

.. code-block:: bash

  AddressSpace
    .                                   
    .                                   
    .                                   
    Object 10                           
           Link                  ResourceStore                  
           Name                  ResCon                         
           OtherObjectName       DDS_Connection      
    EndObject
                               
    Object 11                           
           Link                  ResourceStore                  
           Name                  ConnectionLockLink             
           OtherObjectName       DDS_ConnectionLock  
    EndObject
                               
    Object 12                           
           MemoryRegion          your_app_name_database 
           MapTo                 splice_database               
           First 0x20000000                    
           Length 33554432                     
           Execute true                        
           Read true                           
           Write true                          
    EndObject                           
    .                                   
    .                                   
    .                                   
  EndAddressSpace                      

..


|caution|
  *Note:* If you make any changes to the ``ospl.int`` file generated by the
  project and then you make any changes to the ``ospl.xml`` file and rebuild
  the project, the changes to the ``ospl.int`` file will be overwritten.

Make sure that you also edit the ``global_table.c`` and ``mounttable.c``
files to match your setup. These files can be found under
``src/projgen/kernel.gpj/kernel_kernel.gpj`` and
``src/projgen.gpj/kernel.gpj/ivfs_server.gpj`` as shown in
`Integrity: changing global_table.c and mounttable.c`_ below:


.. _`Integrity: changing global_table.c and mounttable.c`:

.. figure:: /images/Integrity-changeMounttable.png
   :height: 70mm
   :alt: Integrity: changing global_table.c and mounttable.c

   **Integrity: changing global_table.c and mounttable.c**


Once you have made all of the required changes to ``ospl.int``, you must
rebuild the whole project. Your changes will be picked up by OpenSplice
DDS automatically.

**************************************************
Critical Warning about *Object 10* and *Object 11*
**************************************************

|caution|  |caution|
  We have used ``Object 10`` and ``Object 11`` in various address spaces to
  declare a semaphore and a connection object, but they may already be in
  use on your system.

  You *can* change these numbers in the ``ospl.int`` file, but if you do then
  you **must** change **all** of the address spaces where ``Object 10`` and
  ``Object 11`` are defined (except those for ``ResourceStore`` as noted
  below). The value replacing ``10`` must be the same for every address
  space, and likewise for the value replacing ``11``. You **must** change
  **all** references in order for OpenSplice DDS to work correctly.

  *The only exception is the* ``ResourceStore`` *address space.*
  ``Object 10`` *and* ``Object 11`` *are unique to the OpenSplice DDS*
  *ResourceStore and they* **MUST NOT** *be altered.*
  *If you do change them, OpenSplice DDS*  **WILL NOT WORK!**
|caution| |caution|


**************************************************
Amending OpenSplice DDS Configuration with *Multi*
**************************************************

You can make changes to the OpenSplice DDS configuration from Multi by
editing the files under the project
``src/projgen.gpj/opensplice_configuration.gpj/libospl_cfg.gpj``. See
`Integrity: changing OpenSplice DDS configuration in Multi`_ below:


.. _`Integrity: changing OpenSplice DDS configuration in Multi`:

.. figure:: /images/Integrity-DDSconfigMulti.png
   :height: 70mm
   :alt: Integrity: changing OpenSplice DDS configuration in Multi

   **Integrity: changing OpenSplice DDS configuration in Multi**


|caution|
  There are five files here but you may *only* change ``ospl.xml`` and
  ``ospl_log.c``. The others must **NOT** be altered!

``ospl.xml``
  This is your OpenSplice DDS configuration file. (See
  :ref:`Configuration` for more information about the options an OpenSplice DDS
  configuration file may have.)

``ospl_log.c``
  This file determines where the log entries (errors,
  warnings and informational messages) from OpenSplice DDS go. The way the
  default file is generated by ``ospl_projgen`` depends on whether you have
  specified filesystem support or not. (See comments within the file for
  more information.)


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


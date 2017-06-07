.. _`PikeOS POSIX`:

############
PikeOS POSIX
############

*This chapter provides a brief description of how to deploy
Vortex OpenSplice on PikeOS.*


***********************
How to Build for PikeOS
***********************

For this target, Vortex OpenSplice must be configured in the single-process
mode and executables must be statically linked. Also, to avoid requiring
filesystem support, the ``ospl.xml`` configuration file is built into the
executable. A tool named ``osplconf2c`` is provided to generate the code
required for this.

When executed with no arguments, ``osplconf2c`` takes the configuration
file specified by ``OSPL_URI`` (only ``file:`` URIs are supported) and
generates a C source file named ``ospl_config.c`` . The URI and filename
can be specified using the ``-u`` and ``-o`` options respectively. The
generated code should be compiled and linked with each executable. It
comprises an array representing the configuration file and also the
entry points which allow the configured services to be started as
threads.

Vortex OpenSplice is built against the BSD POSIX support in PikeOS
(``bposix``), and it also uses the ``lwIP`` networking facility. When
linking an executable for PikeOS deployment it is necessary to specify
the system libraries as follows:

.. code-block:: make

  -llwip4 -lsbuf -lm -lc -lp4 -lvm -lstand

The Vortex OpenSplice libraries for each configured service (networking,
cmsoap, *etc.*) also have to be linked in.


****************
Deployment Notes
****************

When setting up an integration project in which to deploy an OpenSplice
executable the following items should be configured:

+ Networking should be enabled using the ``LwIP`` stack.
+ The amount of available memory will generally need to be increased.

|caution|
  We recommend a minimum of **48MB** available memory for OpenSplice partitions.

**Steps in CODEO:**

1. Create a new integration project based on the ``devel-posix`` template.

2. Edit ``project.xml.conf``:

  a. Configure networking for the ``muxa`` in the service partition,
     if required.

  b. Enable ``LwIP`` in the POSIX partition, set its device name and
     IP addresses.

  c. Add a dependency in the POSIX partition on the network driver
     file provider.

3. Edit ``vmit.xml``:

  In the POSIX partition, set the ``SizeBytes`` parameter in ``Memory
  Requirement->RAM\_[partition]`` to at least ``0x03000000`` .

4. Copy your Vortex OpenSplice executable into the project's target directory
   and build the project.


***********
Limitations
***********

|caution|
  Multicast networking is not supported on PikeOS POSIX partitions.
  Consequently, the DDSI networking service is not supported.
           
|info|
  The native networking service is supported in a *broadcast* configuration.


***********************
PikeOS on Windows Hosts
***********************

Developing with Vortex OpenSplice for PikeOS on Windows differs from Vortex OpenSplice
on other Windows-hosted platforms. This is because PikeOS uses Cygwin to
present a Unix-like environment for cross-development. OpenSplice
development therefore follows similar practice to the Unix-hosted
editions; for example, you would start a session by entering a ``bash``
shell and sourcing the ``release.com`` file.

However, the Vortex OpenSplice tools (``idlpp``, ``osplconf2c``, ``ospltun``,
*etc*.) are built as Windows executables or will be running in Java for
Windows and so need to be run with Windows-style pathnames. This is
handled using the ``cygpath`` command as seen in the ``release.com`` script
and the example ``makefiles``. Tuner and Configurator may be launched
from the ``Start`` menu as usual.

Building the examples
=====================

This proceeds as would be expected in a Unix environment. Assuming that
Vortex OpenSplice and PikeOS are installed in the default locations:

Start Cygwin ``bash``

.. code-block:: bash

  declare -x PIKEOS_HOME=/opt/pikeos-3.1
  declare -x PATH=$PATH:$PIKEOS_HOME/cdk/ppc/oea/bin
  . /opt/OpenSpliceDDS-V6.2.3/HDE/ppc.pikeos3/release.com
  cd $OSPL_HOME/examples
  make



Using a custom LwIP
===================

The example ``makefiles`` are set up by default to link against the build
of ``LwIP`` distributed with PikeOS, but in some circumstances it may
be useful to rebuild ``LwIP``.

In this case, before building the examples, set the environment 
variable ``LWIP_HOME`` to a directory containing ``liblwip4.a``,
``lwipconfig.o`` and ``lwipopts.h``.



.. |caution| image:: ./images/icon-caution.*
            :height: 6mm
.. |info|   image:: ./images/icon-info.*
            :height: 6mm

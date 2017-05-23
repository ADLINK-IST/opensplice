.. _`Windows CE`:

##########
Windows CE
##########

*This chapter provides a brief description of how to deploy Vortex OpenSplice
on Windows CE.*


*************
Prerequisites
*************

Vortex OpenSplice requires certain environment variables to be present; as
Windows CE does not support traditional environment variables, these are
simulated by creating registry entries which contain the required data.
References in this chapter to \`environment variables' are therefore
actually references to values in the Windows CE registry.

The environment variables expected by Vortex OpenSplice are:

``PATH``
  The *PATH* variable must include the directory containing
  the Vortex OpenSplice executables that may be launched by
  the *ospl* utility.

``OSPL_URI``
  This variable contains the location of the default
  *ospl.xml* configuration file which is used when not
  otherwise specified.

The descriptions in this chapter assume that the values shown in the
table below have been added to the registry key

``HKEY_LOCAL_MACHINE\Software\PrismTech\OpenSpliceDDS\<OpenSpliceVersion>``

**Windows CE Registry keys**

.. note: watch out for triple backslashes in row 1 of table!

+-------------+-------------+-------------------------------+
| Name        | Type        | Data                          |
+=============+=============+===============================+
| PATH        | REG_SZ      | \NAND Flash\OpenSpliceDDS\\\  |
|             |             | <OpenSpliceVersion>\\HDE\\\   |
|             |             | armv4i.wince                  |
+-------------+-------------+-------------------------------+
| OSPL_URI    | REG_SZ      | file://NAND Flash/\           |
|             |             | OpenSpliceDDS/\               |
|             |             | <OpenSpliceVersion>/HDE/\     |
|             |             | armv4i.wince/\                |
|             |             | etc/config/ospl.xml           |
+-------------+-------------+-------------------------------+


|caution|
  All Vortex OpenSplice dynamic link library (``dll``) files must also be
  copied into the ``\Windows`` directory on the Windows CE device *prior to*
  deployment.


***************************************
Setting Registry Values with a CAB File
***************************************

In development, a ``CAB`` file can be used to register the necessary
variables in the registry. Place the ``CAB`` file in the Cold Boot
directory on the target device (*i.e.* ``\NAND Flash\ColdBootInit``)
to make the registry settings available as soon as the device has
booted.

Alternatives to CAB file
========================

Microsoft's Windows CE Remote Registry Editor can be used instead of a
``CAB`` file to set the necessary registry values. Alternatively,
PrismTech also provides a convenient method of editing the registry
variables by way of the ``ospl`` utility using the ``getenv`` and
``putenv`` parameters (described below).

Please refer to Microsoft's Windows CE documentation for detailed
information about ``CAB`` files and the Remote Registry Editor.


******************************
The Vortex OpenSplice Environment
******************************

Vortex OpenSplice requires the contents of the *bin* , *lib* and *etc*
directories from within the Vortex OpenSplice installation to be available
on the Windows CE target hardware. For development purposes, Microsoft's
ActiveSync can be used to load these on to the target system. The
following description assumes that the ``bin``, ``lib`` and ``etc``
directories have been copied from the Vortex OpenSplice installation onto
the target at the following location:

``\NAND Flash\OpenSpliceDDS\<OpenSpliceVersion>\HDE\armv4i.wince``

For simplicity the whole ``OpenSpliceDDS`` installation directory can be
copied to the ``\Nand Flash`` directory.

The following description explains deployment on Windows CE by using the
Windows CE console. It is assumed that the console's ``PATH`` variable has
been set to point to the directory containing the Vortex OpenSplice
executables. For example:

.. code-block:: bat

  PATH "\NAND Flash\OpenSpliceDDS\<OpenSpliceVersion>\
  HDE\armv4i.wince\bin";%PATH%

(All Vortex OpenSplice dynamic link library (``dll``) files must have been
copied into the ```\Windows`` directory on the Windows CE device prior to
deployment.)

When running OpenSplice executables on the command prompt, it is useful
to redirect any output to text files by using the ``>`` operator.

If the ``PATH`` and ``OSPL_URI`` variables have not already been set *via*
a ``CAB`` file on device boot up, use the following commands to set those
values manually:

.. code-block:: bat

  ospl putenv PATH "\NAND Flash\OpenSpliceDDS\<OpenSpliceVersion>\
  HDE\armv4i.wince\bin" > osplputenv-path.txt

  ospl putenv OSPL_URI "file://NAND Flash/OpenSpliceDDS/<OpenSpliceVersion>/
  HDE/armv4i.wince/etc/config/ospl.xml" > osplputenv-ospluri.txt

The values can be checked if required by using Microsoft's Windows CE
Remote Registry Editor, or by running the *ospl getenv* command:

``ospl getenv PATH > osplgetenv-path.txt``


*****************
Secure Networking
*****************

The secure networking service uses OpenSSL for cryptography support. To
use this feature, the library ``libeay32.dll`` is required; it must be
copied to the ``\Windows`` directory on the Windows CE device.

OpenSplice is tested against OpenSSL version 0.9.8i. This may be built
as described below.

Building OpenSSL for Windows CE 6.0
===================================

This section describes the steps required to get an OpenSSL build for
Windows CE. The version of OpenSSL used is 0.9.8i. The third-party
library *wcecompat* is used, which also has to be built manually for
Windows CE 6.0.

| (The description that follows is based on the one given at
| http://blog.csdn.net/sooner01/archive/2009/06/22/4289147.aspx.)


Prerequisites
-------------

The following are needed to make an OpenSSL build for Windows CE 6.0:

+ Microsoft Visual Studio 2005
    (VS2008 might also work but it has not been
    tested)

+ An installed WinCE 6.0 SDK to be targeted
    In this description the
    target SDK is ``'WinCE-GS3Target'``
    
+ Perl
    You will need to install Active Perl, from
    http://www.activestate.com/ActivePerl.
    (Note that perl by MSYS does not create correct ``makefiles``.)

+ OpenSSL
    The OpenSSL sources can be downloaded from http://www.openssl.org/.
    In this description we use version 0.9.8i. Other versions might not work
    with the steps described here.

+ *wcecompat* compatibility library
    The *wcecompat* library adds the functionality to the C Runtime Library
    implementation of Windows CE which is needed in order to build OpenSSL
    for Windows CE. Obtain this from http://github.com/mauricek/wcecompat. Note
    that you should *not* download the latest version; browse the history
    and download the version committed on November 21, 2008 named
    *updates for OpenSSL 0.9.9* with commit number *f77225b...*.


Build *wcecompat*
-----------------

Extract the *wcecompat* download to an appropriate location. In this
description the location ``C:\wcecompat`` is used, but you can use
any location you want.

**Step 1**

  Start Visual Studio 2005 and open a Visual Studio 2005 command prompt.

**Step 2**

  Go to the *wcecompat* directory (``C:\wcecompat``).

**Step 3**

  Set the building environment:

.. code-block:: make

  set OSVERSION=WCE600
  
  set TARGETCPU=ARMV4I
  
  set PLATFORM=VC-CE
  
  set PATH=C:\Program Files\Microsoft Visual Studio 8\VC\ce\bin\x86_arm;
  C:\Program Files\Microsoft Visual Studio 8\Common7\IDE;%PATH%

  set INCLUDE=C:\Program Files\Windows CE Tools\wce600\WinCE-GS3Target\
  include\ARMV4I

  set LIB=C:\Program Files\Windows CE Tools\wce600\WinCE-GS3Target\lib\
  ARMV4I;C:\Program Files\\Microsoft Visual Studio 8\VC\ce\lib\armv4

.. 

  If you target a different SDK, replace the text *WinCE-GS3Target*
  in the lines above with your own SDK.

**Step 4**

  Call *perl config.pl* to create the makefile configuration.

**Step 5**

  Call *nmake* to build the *wcecompat* library.

**Step 6**

  Exit the command prompt and exit Visual Studio to be sure of starting
  with a clean environment in the next stage.



Build OpenSSL
-------------

**Step 1**

  Extract *OpenSSL* to any location you like.

**Step 2**

  Apply the OpenSSL WinCE patch which can be found at
  http://marc.info/?l=openssl-dev&m=122595397822893&w=2.

**Step 3**

  Start Visual Studio 2005 and open a command prompt.

**Step 4**

  Go to your openSSL directory.

**Step 5**

  Set the building environment:

.. code-block:: make

  set OSVERSION=WCE600

  set TARGETCPU=ARMV4I

  set PLATFORM=VC-CE

  set PATH=C:\Program Files\Microsoft Visual Studio
  8\VC\ce\bin\x86\_arm;C:\Program Files\Microsoft Visual Studio
  8\VC\bin;C:\Program Files\Microsoft Visual Studio
  8\VC\PlatformSDK\bin;C:\Program Files\Microsoft Visual Studio
  8\Common7\Tools;C:\Program Files\Microsoft Visual Studio
  8\Common7\IDE;C:\Program Files\Microsoft Visual Studio
  8\Common\Tools;C:\Program Files\Microsoft Visual Studio
  8\Common\IDE;C:\Program Files\Microsoft Visual Studio 8\;%PATH%

  set INCLUDE=C:\Program Files\Microsoft Visual Studio
  8\VC\ce\include;C:\Program Files\Windows CE
  Tools\wce600\WinCE-GS3Target\include\ARMV4I;C:\Program
  Files\Windows CE Tools\wce600\WinCE-GS3Target\include;C:\Program
  Files\Microsoft Visual Studio 8\VC\ce\atlmfc\include;C:\Program
  Files\Microsoft Visual Studio 8\SmartDevices\SDK\SQL
  Server\Mobile\v3.0;

  set LIB=C:\Program Files\Windows CE
  Tools\wce600\WinCE-GS3Target\lib\ARMV4I;C:\Program Files\Microsoft
  Visual Studio 8\VC\ce\atlmfc\lib\ARMV4I;C:\Program
  Files\Microsoft Visual Studio 8\VC\ce\lib\ARMV4I

  set WCECOMPAT=C:\wcecompat

.. 

  If you target a different SDK, replace the text ``WinCE-GS3Target``
  in the lines above with your own SDK. Also, change the *wcecompat*
  directory to your own if you used a different location.

**Step 6**

  Type ``perl Configure VC-CE`` to set up the compiler and OS.

**Step 7**

  Type ``ms\do_ms`` to build the makefile configuration.

**Step 8**

  Type ``nmake -f ms\cedll.mak`` to build the dynamic version of
  the library.



Troubleshooting
---------------

If you get the following error message:

.. code-block:: make

  PTO -c .\\crypto\\rsa\\rsa\_pss.c

  cl : Command line warning D9002 : ignoring unknown option '/MC'
  rsa\_pss.c

  f:\\openssl\\openssl98\\crypto\\rsa\\rsa\_pss.c(165) : error C2220:
  warning treated as error - no 'object' file generated

  f:\\openssl\\openssl98\\crypto\\rsa\\rsa\_pss.c(165) : warning C4748:
  /GS can not protect parameters and local variables from local buffer
  overrun because optimizations are disabled in function

  NMAKE : fatal error U1077: '"F:\\Program Files\\Microsoft Visual Studio
  8\\VC\\ce\\bin\\x86\_arm\\cl.EXE"' : return code '0x2'

  Stop.


Remove ``/WX`` in the makefile (``ce.mak``).


************************
Deploying OpenSplice DDS
************************

``ospl start``
  This command will start the OpenSplice DDS ``splicedaemon`` and OpenSplice
  DDS services specified within the configuration referred to by the
  ``OSPL_URI`` variable:

  ``ospl start > osplstart.txt``

  A different configuration file can be specified as an additional
  parameter; for example:

  |  ``ospl start "file://NAND Flash/OpenSpliceDDS/<OpenSpliceVersion>/``
  |  ``HDE/armv4i.wince/etc/config/ospl.xml" > osplstart.txt``

``ospl list``
  This command will list all the OpenSplice DDS configurations that are
  currently running on the node.

  ``ospl list > ospllist.txt``

``ospl stop``
  This command will stop the OpenSplice DDS ``splicedaemon`` and OpenSplice
  DDS services specified within the configuration referred to by the
  ``OSPL_URI`` variable:

  ``ospl stop > osplstop.txt``

  A different configuration to be stopped can be specified as an
  additional parameter; for example:

  |  ``ospl stop "file://NAND Flash/OpenSpliceDDS/<OpenSpliceVersion>/``
  |  ``HDE/armv4i.wince/etc/config/ospl.xml" > osplstop.txt``


************************************************
Using the *mmstat* Diagnostic Tool on Windows CE
************************************************

To run ``mmstat``, use this command:

  ``start mmstat > mmstat.txt``

To see the full list of options, use this command:

  ``start mmstat -h > mmstat-help.txt``

The mechanism for terminating ``mmstat`` on Windows CE is different from
other operating systems. All running instances of ``mmstat`` can be
terminated with this command:

  ``start mmstat -q > mmstat-quit.txt``

If there are multiple instances of ``mmstat`` running, a particular
instance can be terminated by specifying the process identifier:

  ``start mmstat -q -x <process id> > mmstat-quit.txt``

where *<process id>* is displayed in the output for the particular
instance of ``mmstat``.


*******************
OpenSplice Examples
*******************

.. note: xref to install-configure.rst

Please refer to the :ref:`Examples` section for descriptions of the
OpenSplice DDS examples.


Building the examples
=====================

There is a shortcut to load the examples into Microsoft Visual Studio
which can be accessed from *Start > Programs > OpenSpliceDDS
<OpenSpliceVersion> armv4i.wince HDE > Examples*.

Once the projects are open in Microsoft Visual Studio, click
*Build/Rebuild Solution* at the appropriate level to build the required
examples.

Copy the produced executable files to the OpenSplice DDS ``bin`` directory
(*i.e.* ``\NAND
Flash\OpenSpliceDDS\<OpenSpliceVersion>\HDE\armv4i.wince\bin``) on
the Windows CE device. For the PingPong example the executable files are
``Ping.exe`` and ``Pong.exe``. For the Tutorial example the files are
``Chatter.exe``, ``MessageBoard.exe``, and ``UserLoad.exe``.

As an alternative to using the shortcut, to set up the environment
for a new project perform the following steps:

**Step 1**

  Run the OpenSplice command prompt from the *OpenSplice* entry under the
  Windows *Start* button:

  *Start > Programs > OpenSpliceDDS <OpenSpliceVersion> armv4i.wince HDE
  > OpenSpliceDDS command prompt*

**Step 2**

  Copy the Windows Microsoft Visual Studio environment variables to the
  new command prompt. To obtain these, right-click on the *Properties* for
  the *Visual Studio 2005 Command Prompt* entry located at *Start >
  Programs > Microsoft Visual Studio 2005 > Visual Studio Tools > Visual
  Studio 2005 Command Prompt* , and paste the *Shortcut Target* entry into
  the OpenSplice command prompt. For example this could be

  ``%comspec% /k ""C:\Program Files\Microsoft Visual Studio 8\VC\vcvarsall.bat"" x86``

**Step 3**

  Start Microsoft Visual Studio in this prompt:

  ``devenv``

**Step 4**

  Open the solution file at

  ``<OpenSpliceDDSInstallation>/examples/examples.sln``


Deploying the PingPong example
==============================

Start Vortex OpenSplice as described above. The Ping and Pong executables
can then be started as follows:

  ``start pong PongRead PongWrite > pong.txt``
  
  ``start ping 100 100 m PongRead PongWrite > ping.txt``

The ``ping.txt`` file produced should contain the expected Ping Pong
measurement statistics for 100 cycles. The ``Pong`` executable can be shut
down by running the ``ping shutdown`` command:

  ``start ping 1 1 t PongRead PongWrite > ping-shutdown.txt``


Deploying the Tutorial example
==============================

Start Vortex OpenSplice as described above. The Tutorial executables can
then be started as follows:

  ``start UserLoad > userload.txt``

  ``start MessageBoard > messageboard.txt``

  ``start Chatter 1 John > chatter.txt``

The ``messageboard.txt`` file produced should contain the messages
received from the ``Chatter`` executable. The ``MessageBoard`` executable
can be terminated by running ``Chatter`` again with the ``-1`` option:

  ``start Chatter -1 > chatter-shutdown.txt``



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


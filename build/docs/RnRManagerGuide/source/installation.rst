.. _`Installation`:


############
Installation
############

Pre-requisites
**************

Vortex OpenSplice RnR Manager requires Oracle Java of at least version 1.6.
OpenJDK Java is not supported.

RnR Manager does not need Vortex OpenSplice to be installed on the same 
system in order to control the RnR service. However, certain features of RnR 
Manager require a local installation of Vortex OpenSplice in order to operate. 
This includes connecting to a locally running Vortex OpenSplice domain using 
Domain URI, or Domain ID, and the storage Import/Export functions.

(Please refer to the Vortex OpenSplice *Getting Started Guide* for full
details of how to install Vortex OpenSplice.)

Other information regarding recent changes to RnR Manager, or known issues 
are given in the *Release Notes* included with the distribution.

Supported Platforms
*******************

The RnR Manager will run on the Windows and Linux operating systems.


General Installation Instructions
*********************************

To install the Vortex OpenSplice RnR Manager, start the installer and follow 
the on-screen instructions.

Installation on Unix and Linux
==============================

|unix| |linux|

On Unix-based platforms (including Linux), first ensure that execute 
permission is enabled, then run the installer from the command line:

::
   
   %  VortexOpenSpliceRNRManager-<version>-<platform>-installer.bin

where ``<version>`` is the release version number and ``<platform>`` 
is the build for your platform.

Installation on Windows
=======================

|windows|

On Windows-based platforms, start the installer by double-clicking on 
its filename in *Windows Explorer*:

:: 

   VortexOpenSpliceRNRManager-<version>-windows-installer.exe

where ``<version>`` is the release version number.

Installing the license file
===========================

|info|
  Please refer to the Vortex OpenSplice *Getting Started Guide*
  for full details of how Vortex OpenSplice is licensed.

PrismTech supplies a license file for the RnR Manager product. This file 
is *not* included in the software distribution.

During the installation, you have the option of specifying the license 
file.


.. _`Installing a License file`:

.. centered:: **Installing a License file**

.. image:: /images/002_Licensing_01.png
   :width: 90mm
   :align: center
   :alt: Installing a License file


If you did not specify a license file during the installation, you will 
see a licensing error on startup.

.. _`License file error`:

.. centered:: **License file error**

.. image:: /images/003_Licensing_02.png
   :width: 90mm
   :align: center
   :alt: License file error


Click the *Manage Licenses* button and then *Import License File*. 
Click the button *Restart Workbench* after importing the license file.



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

         
.. _`Licensing OpenSplice`:

####################
Licensing OpenSplice
####################

*Vortex OpenSplice uses Reprise License Manager (RLM) to manage licenses.
This section describes how to install a license file for Vortex OpenSplice
and how to use the license manager.*


*******
General
*******

The licensing software is automatically installed on the host machine as
part of the OpenSplice distribution. The software consists of two parts:

+ *Vortex OpenSplice binary files*
    | which are installed in ``<OpenSplice_Install_Dir>/<E>/<platform>.<os>/bin``,
    | where ``OpenSplice_Install_Dir`` is the directory
    | where Vortex OpenSplice is installed.

+ *License files*
    | which determine the terms of the license. 
    | These will be supplied by PrismTech.


|info|
  **Licenses:** PrismTech supplies an Vortex OpenSplice license file,
  ``license.lic``. This file is *not* included in the software
  distribution, but is sent separately by PrismTech.


***********************************
Development and Deployment Licenses
***********************************

Development licenses are on a per Single Named Developer basis. This
means that each developer using the product requires a license.
Vortex OpenSplice is physically licensed for development purposes.
Vortex OpenSplice is also physically licensed on enterprise platforms for
deployment.

|caution|
  Some OpenSplice components are licensed individually and you will need
  the correct feature to be unlocked for you to use them.


***************************
Installing the License File
***************************

Copy the license file to ``<OpenSplice_Install_Dir>/etc/license.lic``
on the machine that will run the license manager.
``<OpenSplice_Install_Dir>`` is the directory where OpenSplice is
installed.

This is the recommended location for the license file but you can put
the file in any location that can be accessed by the license manager
``rlm``.

If another location is used or the environment has not been setup, then
an environment variable, either ``RLM_LICENSE`` or ``prismtech_LICENSE``,
must be set to the full path and filename of the license file (either
variable can be set; there is no need to set both). For example:

  ``prismtech_LICENSE=/my/lic/dir/license.lic``

If licenses are distributed between multiple license files, the
``RLM_LICENSE`` or ``prismtech_LICENSE`` variable can be set to point to
the directory which contains the license files.


**********************************
Running the License Manager Daemon
**********************************

It is only necessary to run the License Manager Daemon for floating or
counted licenses. In this case, the license manager must be running
before OpenSplice DDS can be used. The license manager software is
responsible for allocating licenses to developers and ensuring that the
allowed number of concurrent licenses is not exceeded.

For node-locked licenses, as is the case with all evaluation licenses,
then it is not necessary to run the License Manager Daemon but the
``RLM_LICENSE`` or ``prismtech_LICENSE`` variable must be set to the
correct license file location.

To run the license manager, use the following command:

.. code-block:: bash

  rlm -c <location>

where ``<location>`` is the full path and filename of the license file. If
licenses are distributed between multiple files, ``<location>`` should be
the path to the directory that contains the license files.

The ``rlm`` command will start the PrismTech vendor daemon ``prismtech``,
which controls the licensing of the OpenSplice DDS software.

To obtain a license for OpenSplice DDS from a License Manager Daemon
that is running on a different machine, set either the ``RLM_LICENSE`` 
or ``prismtech_LICENSE`` environment variable to point to the License
Manager Daemon, using the following syntax:

.. code-block:: bash

  RLM_LICENSE=<port>@<host>

where ``<port>`` is the port the daemon is running on and ``<host>`` is the
host the daemon is running on.

The port and host values can be obtained from the information output
when the daemon is started. The format of this output is as shown in the
following example:

.. code-block:: bash

  07/05 12:05 (rlm) License server started on rhel4e
  07/05 12:05 (rlm) Server architecture: x86_l2
  07/05 12:05 (rlm) License files:
  07/05 12:05 (rlm) license.lic
  07/05 12:05 (rlm)
  07/05 12:05 (rlm) Web server starting on port 5054
  07/05 12:05 (rlm) Using TCP/IP port 5053
  07/05 12:05 (rlm) Starting ISV servers:
  07/05 12:05 (rlm) ... prismtech on port 35562
  07/05 12:05 (prismtech) RLM License Server Version 9.1BL3 for ISV "prismtech"
  07/05 12:05 (prismtech) Server architecture: x86_l2

    Copyright (C) 2006-2011, Reprise Software, Inc. All rights reserved.

    RLM contains software developed by the OpenSSL Project
    for use in the OpenSSL Toolkit (http//www.openssl.org)
    Copyright (c) 1998-2008 The OpenSSL Project. All rights reserved.
    Copyright (c) 1995-1998 Eric Young (eay@cryptsoft.com) All rights
    reserved.

  07/05 12:05 (prismtech)
  07/05 12:05 (prismtech) Server started on rhel4e (hostid: 0025643ad2a7) for:
  07/05 12:05 (prismtech) opensplice_product1 opensplice_product2
  07/05 12:05 (prismtech)
  07/05 12:05 (prismtech) License files:
  07/05 12:05 (prismtech) license.lic
  07/05 12:05 (prismtech)

.. 

The ``<port>`` value should be taken from the first line of the output.
The ``<server>`` value should be taken from the last line. From this
example, the value for ``RLM_LICENSE`` or ``prismtech_LICENSE`` would be:

  ``35562@rhel4e``


*********
Utilities
*********

A utility program, ``rlmutil``, is available for license server
management and administration. One feature of this utility is its
ability to gracefully shut down the license manager. To shut down the
license manager, preventing the checkout of licenses for the OpenSplice
DDS software, run either of the following commands:

.. code-block:: bash

  rlmutil rlmdown -vendor prismtech

  rlmutil rlmdown -c <location>

where ``<location>`` is the full path and filename of the license file.

The ``rlmutil`` program is also used to generate a host identification
code which is used to generate your license key. To generate the code,
run the following command on the license server:

|linux|

.. code-block:: bash

  rlmutil rlmhostid


|windows|

.. code-block:: bash

  rlmutil rlmhostid ether

This returns an ID code for the server, which will look similar to:

  ``Hostid of this machine: 0025643ad2a7``

This ID code must be supplied to PrismTech so that your license key can
be generated.



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


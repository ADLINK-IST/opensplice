.. _`Creating Launch Configurations`:


##############################
Creating Launch Configurations
##############################

*This section describes how to set up 'launch
configurations' for the Vortex OpenSplice Modeler 
which provide additional convenient ways to
run and control the Modeler.*  

A Vortex OpenSplice daemon must be running on the host where your code 
will be run and for the domain within which your code will 
operate.

|info|
  +------------------------------------------------------------------+
  | Starting a Vortex OpenSplice daemon is only required when        |
  | running a *Shared Memory* Deployment. For more details about     |
  | starting Vortex OpenSplice, please see the Vortex OpenSplice     |
  | *Deployment Guide*.                                              |
  |                                                                  |
  +------------------------------------------------------------------+

The Modeler includes an HDE icon |hdeicon| (located in the Eclipse 
*Icon Bar*) which can start and stop the Vortex OpenSplice daemon. 
However, configurations can be manually created to start and stop the 
daemon if desired or if the daemon does not start when using the 
HDE tool from the icon bar. 

The instructions provided in this section describe how to 
manually create, start, and stop configurations for the 
Vortex OpenSplice (OSPL) daemon.

|info|
  +------------------------------------------------------------------+
  | It is recommended that Vortex OpenSplice launch configurations   |
  | are created which can start and stop the Vortex OpenSplice       |
  | daemon: this will enable Vortex OpenSplice code to be run from   |
  | within Eclipse.                                                  |
  |                                                                  |
  | These instructions assume that the environment is configured     |
  | for the version of Vortex OpenSplice that will be used. Refer to |
  | the Vortex OpenSplice *Getting Started Guide* for configuration  |
  | information.                                                     |
  | Environment variables can be set for specific launch             |
  | configurations by selecting the launch configuration and         |
  | clicking on the *Environment* tab.                               |
  |                                                                  |
  +------------------------------------------------------------------+




Creating and Running an ``OSPL start`` Launch Configuration
***********************************************************


Creating the ``start`` Configuration
====================================

**Step 1**

  Choose *Run > External Tools > External Tools* from the 
  Eclipse *Menu Bar*. This will open the *External Tools* dialog.

**Step 2**

  Right-click on the *Program* item located in tree view (located 
  on the left-hand side of the *External Tools* dialog) and choose 
  *New*. This will create a new item under the *Program* item.

**Step 3**

  Enter ``OSPL start`` in the *Name* text field (located near the top 
  of the dialog); this is for the launch configuration.

**Step 4**

  In the *Location* text box, use the *Browse File System* button 
  to locate the ``OSPL`` executable (``ospl.exe`` on Windows platforms). 
  The full pathname for the ``OSPL`` executable should appear in the 
  *Location* text box.

  For example, on Windows this could be 
  ``"C:\Program Files (x86)\PrismTech\Vortex_v2\Device\VortexOpenSplice\6.6.0p1\HDE\x86.win32\bin\ospl.exe"``.

**Step 5**

  Enter the word ``start`` in the *Arguments* text box. This will 
  instruct the ``OSPL`` executable to start the daemon.

**Step 6**

  Click the *Apply* button to apply the changes.

**Step 7**

  Click the *Close* button to close the dialog.

Running the ``start`` Configuration
===================================

Running the ``start`` launch configuration executes the ``ospl start`` 
command and starts the daemon.

**Step 1**
  
  Choose *Run > External Tools > External Tools* from the 
  Eclipse *Menu Bar*. This will open the *External Tools* dialog.

**Step 2**

  Select the ``OSPL start`` item (located under the *Program* item) 
  in tree view (located on the left-hand side of the *External 
  Tools* dialog).

**Step 3**

  Click the *Run* button (located at the bottom of the dialog). 
  This will start the daemon.

**Step 4**

  Click the *Close* button to close the dialog.


Creating and Running an ``OSPL stop`` Launch Configuration
**********************************************************


Creating the ``stop`` Configuration
===================================


*Either:* 

Repeat all of the steps shown under Section 3.8.1.1, 
Creating the start Configuration, above, but use ``OSPL stop`` 
for the launch configuration's name, and enter ``stop`` in the 
*Arguments* text box instead of ``start``.

*OR:*

**Step 1**

  Right-click on the launch configuration created under 
  Section 3.8.1.1, Creating the start Configuration.

**Step 2**

  Click *Duplicate*.

**Step 3**

  Select *Duplicate Launch Configuration*.

**Step 4**

  Rename it from ``OSPL start (1)`` to ``OSPL stop`` by entering 
  the new name in the *Name* field

**Step 5**

  Replace ``start`` with ``stop`` in the *argument* field.

**Step 6**

  Click *Apply*.

Running the ``stop`` Configuration
==================================


Running the ``stop`` launch configuration executes the ``ospl stop`` 
command and stops the daemon.

**Step 1**
  
  Choose *Run > External Tools > External Tools* from the 
  Eclipse *Menu Bar*. This will open the *External Tools* dialog.

**Step 2**

  Select the ``OSPL stop`` item (located under the *Program* item) 
  in tree view (located on the left-hand side of the *External 
  Tools* dialog).

**Step 3**

  Click the *Run* button. 
  This will stop the daemon.

**Step 4**

  Click the *Close* button to close the dialog.


.. |hdeicon| image:: ./images/000_icon_HDE.*
            :height: 4mm


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

         
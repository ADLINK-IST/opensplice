.. _`Installation`:

############
Installation
############

This section describes the procedure to install the Vortex DDS Simulink Integration on a Linux or Windows platform. 

System Requirements
*******************

- Operating System: Windows or Linux 
- MATLAB Simulink installed
- Java 1.7 or greater

OpenSplice (OSPL) and DDS Simulink Installation
***********************************************

Steps:

1.  Install OSPL.  The DDS Simulink Integration is included in this installer.


2.  Setup OSPL license.  Copy the license.lic file into the appropriate license directory.

   */INSTALLDIR/ADLINK/Vortex_v2/license*

3.  MATLAB and Simulink files are contained in a tools/matlab folder

   Example:
   */INSTALLDIR/ADLINK/Vortex_v2/Device/VortexOpenSplice/6.8.x/HDE/x86_64.linux/tools/matlab*


OpenSplice (OSPL) Configuration
*******************************

By default OSPL uses single process configuration.

If however, shared memory configuration is used, additional OSPL configuration steps need to be taken to work with MATLAB Simulink.

Linux
=====

OSPL-9882 Linux: MATLAB/Simulink hangs when connecting to shared memory domain

**Description**
On Linux, a MATLAB script or Simulink model connecting to a Vortex OpenSplice domain via shared memory will hang.

**Resolution**
MATLAB, like Java applications requires that the environment variable LD_PRELOAD be set to reference the active Java installations libjsig.so library. 
The MATLAB user interface uses Java, and thus requires the same signal handling strategy as Java applications connecting to Vortex OpenSplice. 
The precise syntax for setting the LD_PRELOAD environment variable will depend on the shell being used. 

For Oracle JVMs, LD_PRELOAD should contain this value:
    $JAVA_HOME/jre/lib/amd64/libjsig.so

Windows
=======

OSPL-10018 MATLAB: Shared Memory Database Address on Windows needs to be changed from default

**Description**
On a Windows 64-bit system, an OpenSplice system configured with Shared Memory, MATLAB cannot connect to the OpenSplice domain if the Shared Memory Database Address is set to its default value of 0x40000000. The error log (ospl-error.log) will show entries such as:
Report : Can not Map View Of file: Attempt to access invalid address. 
Internals : OS Abstraction/code/os_sharedmem.c/1764/0/1487951812.565129500

**Resolution**
Use the configuration editor to change the default data base address. Use the 'Domain' tab, and select the 'Database' element in the tree. If necessary, right click the Database element to add an 'Address' element. Change the address. In general, a larger number is less likely to be problematic. On a test machine, appending two zeros to the default address allowed for successful connections.


Simulink Setup
**************

Steps:

1.  Open command shell and run script to setup environment variables.

     **Linux**

     - Open a Linux terminal.

     - Navigate to directory containing release.com file.

       */INSTALLDIR/ADLINK/Vortex_v2/Device/VortexOpenSplice/6.8.x/HDE/x86_64.linux*

     - Run release.com.   (Type in “. release.com” at command line.)


     **Windows**

     - Open a command prompt.

     - Navigate to directory containing release.bat file.

       *INSTALLDIR/ADLINK/Vortex_v2/Device/VortexOpenSplice/6.8.x/HDE/x86_64.win64*

     - Run release.bat.   (Type in “release.bat” at command line.)


2.  Start MATLAB using the **SAME** command shell used in Step 1.

     *NOTE:   If MATLAB is NOT started from a command shell with the correct OSPL environment variables set, exceptions will occur when attempting to use DDS Simulink blocks.*

3.  In MATLAB, navigate to file “Vortex_DDS_Block_Set.mltbx” by typing::

     cd(fullfile(getenv('OSPL_HOME'),'tools','matlab'))

.. raw:: latex

    \newpage
   
4.  Double click on the file “Vortex_DDS_Block_Set.mltbx”.   This will bring up a dialog entitled **Install Vortex_DDS_Block_Set**.    Select **Install**.

.. figure:: images/linuxInstall2.png  
        :alt: DDS


**Setup is complete!**



Examples
********

Example models have been provided in the examples folder.

*../tools/matlab/examples/simulink*  







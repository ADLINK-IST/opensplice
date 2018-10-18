.. _`Installation`:

############
Installation
############

This section describes the procedure to install the Node.js DCPS API on a Linux or Windows platform. 

Dependencies
************

The Node.js DCPS API has several dependencies that must be installed.

Linux 64-bit
============

	- Node.js LTS 8.11.1 or later (preferably Node.js LTS 8.x version)
	- npm (node package manager) version 5.6.0 or later (typically included with a Node.js install)
	- Python 2.7 (v3.x.x is not supported)
	- make
	- C/C++ compiler toolchain like GCC 


Windows 64-bit
==============

	- Node.js LTS 8.11.1 or later (preferably Node.js LTS 8.x version)
	- npm (node package manager) version 5.6.0 or later (typically included with a Node.js install)
	- Python 2.7 (v3.x.x is not supported)
	- Visual C++ build tools (VS 2015 or VS 2017)

Python and Visual C++ build tools install
-----------------------------------------


    - Install all the required tools and configurations using Microsoft's windows-build-tools by running the following from a command prompt as an administrator:

      npm install - -global - -production windows-build-tools

                        (or)

    - Install tools and configuration manually

        - Visual C++ build tools (VS 2015 or VS 2017)
        - Python 2.7 (v3.x.x is not supported)

More detailed information on installing Python and Visual C++ build tools on Windows can be found at: https://github.com/Microsoft/nodejs-guidelines/blob/master/windows-environment.md#compiling-native-addon-modules


OpenSplice (OSPL) and Node.js DCPS API Installation
***************************************************

Steps:

1.  Install OSPL. Choose an HDE type installer which is a Host Development Environment. This contains all of the services, libraries, header files and tools needed to develop 
    applications using OpenSplice. The Node.js DCPS API is included in the following installers:


    .. _`Installers Table`:
    .. tabularcolumns:: | p{5cm} | p{5cm} |
    +-------------------+----------------------+
    | **Platform**      | **Platform Code**    |
    +===================+======================+
    | Ubuntu1404 64 bit | P704                 |
    |                   |                      |
    +-------------------+----------------------+
    | Ubuntu1604 64 bit | P768                 |
    |                   |                      |
    +-------------------+----------------------+
    | Windows10 64 bit  | P738                 |
    |                   |                      |
    +-------------------+----------------------+

    Example installer:

	P704-VortexOpenSplice-6.9.x-HDE-x86.linux-gcc4.1.2-glibc2.5-installer.run

2.  Setup OSPL license. Copy the license.lic file into the appropriate license directory

   */INSTALLDIR/Vortex_v2/license*

3.  Node.js DCPS API files are contained in a tools/nodejs folder

   Example:
   *$OSPL_HOME/tools/nodejs*

Installing Node.js DCPS API in a Node.js application
****************************************************

1. Start a command shell. Setup OSPL environment variables by running release.com or release.bat which can be found in
   
   **Linux**

   */INSTALLDIR/ADLINK/Vortex_v2/Device/VortexOpenSplice/6.9.x/HDE/x86_64.linux/*

   **Windows**

   *\\INSTALLDIR\\ADLINK\\Vortex_v2\\Device\\VortexOpenSplice\\6.9.x\\HDE\\x86_64.windows\\*

2. Create a node project folder, if not created

   mkdir <project_name>

   cd <project_name>

   npm init

3. Change directory to node project folder

4. Install the Node.js DCPS API to your project by executing:

   **Linux**

   npm install $OSPL_HOME/tools/nodejs/vortexdds-x.y.z.tgz

   **Windows**

   npm install %OSPL_HOME%\\tools\\nodejs\\vortexdds-x.y.z.tgz

Examples and Documentation
**************************

1.  Examples directory:

         *$OSPL_HOME/tools/nodejs/examples*
         

2.  Node.js DCPS API documentation directory:  

         *$OSPL_HOME/docs/nodejs/html*
         

3.  Node.js DCPS User Guide (HTML and PDF) directory:

         *$OSPL_HOME/docs*

         







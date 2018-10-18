.. _`Troubleshooting`:


###############
Troubleshooting
###############

**When double clicking on a DDS block to view the block parameters, an error dialog is shown with the message: Error evaluating 'MaskDialog' callback of SubSystem block (mask).**

*Cause:    The OSPL environment variables have not been setup correctly.*

*Solution: Open a command shell and run the script to setup OSPL environment variables.*

     Linux

     - Open a Linux terminal.

     - Navigate to directory containing release.com file.

       */INSTALLDIR/ADLINK/Vortex_v2/Device/VortexOpenSplice/6.8.x/HDE/x86_64.linux*

     - Run release.com.   (Type in “. release.com” at command line.)


     Windows

     - Open a command prompt.

     - Navigate to directory containing release.bat file.

       *INSTALLDIR/ADLINK/Vortex_v2/Device/VortexOpenSplice/6.8.x/HDE/x86_64.win64*

     - Run release.bat.   (Type in “release.bat” at command line.)

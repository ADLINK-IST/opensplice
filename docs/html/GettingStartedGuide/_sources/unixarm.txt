.. _`UNIX ARM platform`:

#################
UNIX ARM platform
#################

*This chapter provides a brief description of how to deploy Vortex OpenSplice
on a UNIX ARM platform*

**********************************
Installation for UNIX ARM platform
**********************************

The installation of the Vortex OpenSplice HDE or RTS on a UNIX ARM platform is depended
on the type of installer that has been provided. Dependent on the platform the installer
is either an executable or a tar file. An executable installer has either the extension
.run or .exe. A tar file installer has the extension .tar.

The following combinations may be provided
- both the HDE and RTS installers are executables
- the HDE installer is an executable and the RTS is an tar file
- both the HDE and RTS installers are tar files.

When the installer is provided as an executable follow the normal install procedure as
described in section :ref:`Installation for UNIX and Windows Platforms`.

When the installer is provided as an tar file follow the procedure described below.

**Step 1**

  *Untar the installer tar file,*

  go to the directory where Vortex OpenSplice has to be installed.

  untar the tar file in this directory

  ``tar xf P<code>-VortexOpenSplice<version>-<E>-<platform>.<os>-<comp>-<type>-<target>-installer.tar``

  where, some being optional,

  *<code>* - PrismTech's code for the platform
  *<version>* - the Vortex OpenSplice version number,
  for example ``V6.0``

  *<E>* - the environment, either ``HDE`` or ``RTS``

  *<platform>* - the platform architecture,
  for example ``sparc`` or ``x86``

  *<os>* - the operating system
  for example ``solaris8`` or ``linux2.6``

  *<comp>* - the compiler or glibc version

  *<type>* - release, debug or dev, which is release with symbols

  *<target>* - the target architecture for host/target builds.

**Step 2**
  
  *Configure the Vortex OpenSplice environment.*

  Go to the *<install_dir>/<E>/<platform>* directory, where *<E>* is ``HDE`` of ``RTS``
  and <platform is for example ``armv7l.linux``.

  When bash is used as shell:

  Source the release.com file from the shell command line.

  Otherwise:

  First set the OSPL_HOME variable to the directory in which OpenSplice was installed
  and then source the release.com file.


#
# This file describes the actions to be executed to build SPLICE-DDS
#

Before building SPLICE-DDS for a specific platform, the required platform
must first be selected. When executing "setup" in this directory (. ./setup)
a list of possible targets is given. Via the query of "setup" the selection
can be made and will result in setting of the correct environment for building
that target. When using bash, the target can also be specified via the
command line (. ./setup sparc.solaris8-release).

When the selection is made, SPLICE-DDS can be build by typing "make install".
Generation of SPLICE-DDS will take a while.
When the generation is ready, the results can be found in install/HDE/SPLICE_HDE_*.tar
(containg all components for the host development environment), 
install/RTS/SPLICE_RTS_*.tar (containing all components for the runtime system),
install/VC/SPLICE_HDE_* and install/VC/SPLICE_RTS_* (representing virtual
carriers with installation support).

Important notes:

    For distribution, only targets ending with "-release" may be used.
    Other targets use extra builtin checks which reduce the performance severely.

For x86.linux-release target, which actually is suitable for Linux distributions
with NPTL support, a TNNL specific build script is available (bin/build.x86.linux).
Execute bin/build.x86.linux from within the top level source directory.

For sparc.solaris8-release target, which is suitable for SPARC Solaris 8 distributions
a TNNL specific build script is available (bin/build.sparc.solaris8).
Execute bin/build.sparc.solaris8 from within the top level source directory.

To build all target releases the script bin/build.all is available.
Execute bin/build.all from within the top level source directory.

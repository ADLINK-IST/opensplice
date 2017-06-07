ISO C++ API Library Recompilation                                               {#isocpp2_customlibs}
=================================

[TOC]

Rebuilding the API                                                              {#isocpp2_customlibs_general}
==================

To overide the default configurations the API should be re-compiled with one of
the macros \ref OSPL_USE_CXX11, \ref OSPL_USE_TR1, or \ref OSPL_USE_BOOST
defined.

The file [dds/core/detail/macros.hpp](@ref dds/core/detail/macros.hpp) could be
edited directly to do this, or these can be defined on the compile command line.

GCC Re-compilation                                                              {#isocpp2_customlibs_gcc}
------------------

Makefiles are provided that will re-build the DCPS API libraries _as is_ with
the compiler currently on your search path. These are found in
`$OSPL_HOME/custom_lib`. They can be edited to suit your requirements if
necessary. To rebuild the ISO C++ API you might then do, for example:

    make -C $OSPL_HOME/custom_lib -f Makefile.Build_DCPS_ISO_Cpp2_Lib

Visual Studio Re-compilation                                                    {#isocpp2_customlibs_vs}
----------------------------

[Boost]: http://www.boost.org/ "Boost"

A Visual Studio solution file `%%OSPL_HOME%\custom_lib\custom_lib.sln` is
provided. To rebuild to use [Boost] you should (optionally) edit the project
properties to specify \ref OSPL_USE_BOOST during compilation, set the
environment variable `BOOST_ROOT` to point to your [Boost] installation, and
launch Visual Studio - e.g:

    devenv /useenv custom_lib.sln

The ISO C++ API library can then be re-built.

Visual Studio re-compilation for debug                                          {#isocpp2_customlibsdebug_vs}
--------------------------------------
On Windows platforms, memory allocations are potentially different in release
and debug mode and there can be issues when mixing the Release and Debug
configurations across the application and API boundary. The application and
OpenSplice ISO C++ API code need to be compiled for the same configuration in
order to guarantee comparability across that boundary. To build a Debug version
of the OpenSplice ISO C++ API, take the following steps: 

1. If the OSPL_HOME variables are set at the system level you can just open the `custom_lib\Build_DCPS_ISO_Cpp2_Lib` project in Visual Studio (otherwise you'd need to run Visual Studio from a command prompt where the variables were set)
2. Create a new Debug configuration of the ISO C++ custom lib by cloning the existing Release configuration (to have all of those settings)
    - Name it `Debug`
    - C/C++ -> Preprocessor -> Add _DEBUG (as well as leaving NDEBUG set)
    - C/C++ -> Code Generation -> set Multi-threaded Debug DLL instead of Multi Threaded Debug DLL
3. Optional to allow you to build a debug lib/dll without overwriting the existing release one
    - General -> TargetName -> dcpsisocpp2d
    - General -> Intermediate directory -> Debug\Build_DCPS_ISO_Cpp2_Lib\AMD64\
    - Linker -> General -> OutputFile -> $(OutDir)dcpsisocpp2d.dll
    - Linker -> Advanced -> Import Library -> ..\lib\dcpsisocpp2d.lib
4. Rebuild All

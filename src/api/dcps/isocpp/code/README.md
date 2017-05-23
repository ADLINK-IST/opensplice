ISO C++ API Library Recompilation                                               {#isocpp_customlibs}
=================================

[TOC]

Rebuilding the API                                                              {#isocpp_customlibs_general}
==================

To overide the default configurations the API should be re-compiled with one of
the macros \ref OSPL_USE_CXX11, \ref OSPL_USE_TR1, or \ref OSPL_USE_BOOST
defined.

The file [dds/core/detail/macros.hpp](@ref dds/core/detail/macros.hpp) could be
edited directly to do this, or these can be defined on the compile command line.

GCC Re-compilation                                                              {#isocpp_customlibs_gcc}
------------------

Makefiles are provided that will re-build the DCPS API libraries _as is_ with
the compiler currently on your search path. These are found in
`$OSPL_HOME/custom_lib`. They can be edited to suit your requirements if
necessary. To rebuild the ISO C++ API you might then do, for example:

    make -C $OSPL_HOME/custom_lib -f Makefile.Build_DCPS_ISO_Cpp_Lib

Visual Studio Re-compilation                                                    {#isocpp_customlibs_vs}
----------------------------

[Boost]: http://www.boost.org/ "Boost"

A Visual Studio solution file `%%OSPL_HOME%\custom_lib\custom_lib.sln` is
provided. To rebuild to use [Boost] you should (optionally) edit the project
properties to specify \ref OSPL_USE_BOOST during compilation, set the
environment variable `BOOST_ROOT` to point to your [Boost] installation, and
launch Visual Studio - e.g:

    devenv /useenv custom_lib.sln

The ISO C++ API library can then be re-built.

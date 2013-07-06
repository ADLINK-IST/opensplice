ISO/IEC C++ DCPS API                                                            {#mainpage}
====================

[TOC]

API documentation                                                               {#mainpage_api}
=================

- \subpage isocpp_dcps

   + \subpage isocpp_dcps_domain
   + \subpage isocpp_dcps_topic
   + \subpage isocpp_dcps_pub
   + \subpage isocpp_dcps_sub
   + \subpage isocpp_dcps_core

Platform requirements & re-building                                             {#mainpage_build}
===================================

The ISO C++ PSM specification requires some modern compiler features.
Specifically: the header file [ref_traits.hpp](@ref dds/core/ref_traits.hpp)
requires that shared and weak smart pointer types and functions for safe
polymorphism be available and are used. The header
[macros.hpp](@ref dds/core/ref_traits.hpp) requires the existence of a _compile
time_ static assert.

To address these the OpenSplice ISO C++ DCPS API requires any one only of the
following:
   + A compiler with C++ 11 support
   + A compiler with Technical Report 1 (TR1) support
   + The availability of [Boost]. Headers only are required.

GCC requirements                                                                {#mainpage_gcc}
----------------

By default, the ISO C++ DCPS API can, and does, compile against the
TR1 templates headers available in g++ since version 4.1. Any
distributions built with an older version of GCC will have been compiled
against [Boost].

If you would like to use your own version of [Boost] or the experimental
`-std=c++0x` C++11 support to develop your applications: you should rebuild the
ISO C++ library to suit your needs.
See [the rebuilding instructions](@ref isocpp_customlibs)

Visual Studio requirements                                                      {#mainpage_vs}
--------------------------

The default compilation configuration of Visual Studio distributions depends on
the compiler version.

Version             | Default | Optional
--------------------|---------|---------------
Visual Studio 2010  | C++11   | TR1 or [Boost]
Visual Studio 2008  | TR1*    | [Boost]
Visual Stusio 2005  | [Boost] | none

* TR1 support was added to Visual Studio 2008 in Feature Pack 1. If this is not
installed you should use [Boost] instead.

If you wish to change to one of the optional supprted configurations, or would
like to switch the API to link Debug or to your own applications version of the
VC++ redistributable, then you should see
[the rebuilding instructions](@ref isocpp_customlibs).

Rebuilding                                                                      {#mainpage_rebuild}
----------

See \subpage isocpp_customlibs for instructions.

[Boost]: http://www.boost.org/ "Boost"

See also                                                                        {#mainpage_also}
========

- \subpage isocpp_examples

   + \subpage examplesdcpsHelloWorldisocpp
   + \subpage examplesdcpsListenerisocpp
   + \subpage examplesdcpsContentFilteredTopicisocpp

- \subpage ishapes_readme

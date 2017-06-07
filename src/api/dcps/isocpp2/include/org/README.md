ISO/IEC C++ DCPS API                                                            {#mainpage}
====================

[TOC]

ISO C++ is the latest DDS DCPS API for the C++ Language. It is more user friendly
and simpler to use than the 'classic' C++ API.

This is the second version of the ISOC++ Specification that has been implemented.<br>
While most things haven't changed between ISOC++ V1 and V2 regarding the API, there
are still some differences that should be considered when migrating from V1 to V2. These
differences are addressed in the [Migration Guide] (../../html/isocpp_migration.html).<br>
That [Migration Guide] (../../html/isocpp_migration.html) also contains the rationale
for the second version of ISOC++.

This documentation serves to provide a reference guide and a basic introduction to
the ISO C++ API. Due to the way that ISO C++ is implemented using delegates, the
API reference is not a 1:1 representation of the implementation structure of the API,
it is however a representation of the user accessible layer of the API.

Getting started                                                                {#mainpage_gettingstarted}
===============

See the [DDS Tutorial (pdf)] (../../pdf/OpenSplice_DDSTutorial.pdf) for more information
about the DDS specification and usage, which is a good starting point if you don't know
much about DDS yet.

If you are new to ISO C++ we would recommend that you start by looking at the example
code provided in your OpenSplice installation directory as well as the documentation
in the sections below.

A summary of each example, along with build instructions can be found on the
\subpage isocpp2_dcpsexamples page.

The OMG ISO C++ specification also gives an overview of the general design of ISO C++ and
can be found at the following link: http://www.omg.org/spec/DDS-PSM-Cxx/

How to use ISO C++ classes                                                      {#mainpage_howtouse}
--------------------------

ISO C++ was designed in such a way that all memory management is handled automatically
for you. Class instances should be created on the stack rather than the heap and the API itself
will handle the allocation and deletion of memory automatically.

There are two main types of class within ISO C++, Value and Entity. All classes within
ISO C++ inherit from one of these two classes.

Value classes are the simplest type of class within ISO C++. These are simple classes which
are created on the stack and are copied by value. Examples of this type of class include QoS policies
such as dds::core::policy::Durability and dds::core::policy::Liveliness or status conditions
such as dds::core::status::LivelinessChangedStatus.

Entity classes on the other hand are more advanced but still easy to use. These contain an
underlying smart pointer which keeps track of the number of references that exist to that
particular instance of the class. Once there are no remaining references to the class instance, any
memory used by it is then automatically deleted. This underlying smart pointer brings with it
the advantage of low overhead copying of class instances as only the smart pointer needs to be copied.
This means that any number of references may point to the same entity class instance.

Below is an example of how a dds::domain::DomainParticipant can be created and copied, other
Entity classes work in the same way.

    {
        dds::domain::DomainParticipant dp1(dds::core::null); //dds::core::null creates an uninitialised Entity
        {
            dds::domain::DomainParticipant dp2(0); //Creates a domain participant on domain 0

            dp1 = dp2; //Copies reference to dp2 into dp1, both references now point to the same class instance
        }
        //dp2 is now out of scope but the domain participant is not deleted as dp1 still has a reference to it
    }
    //dp1 is now out of scope and there are no further references to the domain participant so it is deleted automatically

More ISO C++ code examples can be found on the \subpage snippets page.

More information about the actual objects and scopes can be found [here](@ref mainpage_objects).

API documentation                                                               {#mainpage_api}
=================

The namespaces below contain all of the major functionality of the ISO C++ API:

- ::dds

   + dds::domain
    + The dds::domain namespace provides facilities for creating a DomainParticipant
      that establishes a virtual network in which an application and it's entities
      can operate.
   + dds::topic
    + The dds::topic namespace provides facilities for the creation of Topics which
      can be published or subscribed to, as well as providing more advanced filtering
      through ContentFilteredTopics.
   + dds::pub
    + The dds::pub namespace provides facilities for writing samples based on a Topic.
   + dds::sub
    + The dds::sub namespace provides facilities for reading samples based on a Topic,
      as well as providing the ability to query a read.
   + dds::core
    + The dds::core namespace provides core facilities such as QoS policies and the
      ability to wait on particular conditions using a WaitSet.

Platform requirements & re-building                                             {#mainpage_build}
===================================

The ISO C++ PSM specification requires some modern compiler features.
Specifically: the header file [ref_traits.hpp](@ref dds/core/ref_traits.hpp)
requires that shared and weak smart pointer types and functions for safe
polymorphism be available and are used. The header
[macros.hpp](@ref dds/core/macros.hpp) requires the existence of a _compile
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
See [the rebuilding instructions](@ref isocpp2_customlibs)

Visual Studio requirements                                                      {#mainpage_vs}
--------------------------

The default compilation configuration of Visual Studio distributions depends on
the compiler version.

Version             | Default | Optional
--------------------|---------|---------------
Visual Studio 2010  | C++11   | TR1 or [Boost]
Visual Studio 2008  | TR1*    | [Boost]
Visual Studio 2005  | [Boost] | none

* TR1 support was added to Visual Studio 2008 in Feature Pack 1. If this is not
installed you should use [Boost] instead.

If you wish to change to one of the optional supprted configurations, or would
like to switch the API to link Debug or to your own applications version of the
VC++ redistributable, then you should see
[the rebuilding instructions](@ref isocpp2_customlibs).

Rebuilding                                                                      {#mainpage_rebuild}
----------

See \subpage isocpp2_customlibs for instructions.

[Boost]: http://www.boost.org/ "Boost"

Regarding objects                                                               {#mainpage_objects}
=================

The way the code of the specification is set up is not very intuitive, but this is basically what happens:<br>
The dds::sub::Subscriber is a typedef to the dds::sub::TSubscriber&lt;DELEGATE&gt; template class.<br>
The dds::sub::TSubscriber&lt;DELEGATE&gt; is basically just a wrapper to the implementation template class (aka delegate) dds::sub::detail::Subscriber.

So, when you create a dds::sub::Subscriber, you create the wrapper dds::sub::TSubscriber&lt;dds::sub::detail::Subscriber&gt; to the implementation class dds::sub::detail::Subscriber.

Type dependent objects have just one more template argument:<br>
The dds::sub::DataReader is a typedef to the dds::sub::TDataReader&lt;T, DELEGATE&lt;T&gt;&gt; template class.<br>
The dds::sub::TDataReader&lt;T, DELEGATE&lt;T&gt;&gt; is the wrapper to the delegate dds::sub::detail::DataReader&lt;T&gt;.<br>
Thus dds::sub::DataReader&lt;my_type&gt; is a typedef to dds::sub::TDataReader&lt;my_type, dds::sub::detail::DataReader&lt;my_type&gt;&gt; which is an wrapper to class dds::sub::detail::DataReader&lt;my_type&gt;.

The API shields these wrappers and delegates from the user by means of the typedefs. The API documentation depicts how these typedefs should be used, e.g. as normal classes and objects.


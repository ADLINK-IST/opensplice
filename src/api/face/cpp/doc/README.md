ISO C++ 2 FACE API                                                            {#mainpage}
====================

[TOC]

A DDS Isocpp2 implementation API for the Future Airborne Capability Environment (FACE) API

Getting started                                                                {#mainpage_gettingstarted}
===============

If you are new to the FACE API we would recommend that you start by looking at the documentation
provided by the opengroup: http://www.opengroup.org/face/information

A summary of examples, along with build/run instructions can be found on the
\subpage face_examples page.


API documentation                                                               {#mainpage_api}
=================

The namespace FACE::TS contains all of the major functionality of the ISO C++ 2 FACE API.

- FACE::TS provides facilities for:
    - Initializing, Creating and Destroying a connection
    - Sending and Receiving data
    - Registering and Unregistering a callback.
    - Getting Connection Parameters

DataType generation:
--------------------
Idlpp can be used to generate the CPP datatypes for the FACE API. The option -F can be used to generate the FACE specific datatypes.


Example command for code generation from the idl Foo.idl for Isocpp2:

`idlpp -S -l isocpp2 -F Foo.idl`



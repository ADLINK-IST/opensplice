iShapes Demonstrator Application                                                {#ishapes_readme}
================================

[TOC]

What is it?                                                                     {#ishapeswhat}
===========

iShapes is a demo that has been used to test interoperability among
DDS vendors while being able to show something "moving on the screen".

The demo is about shapes that bounce within a rectangular region.

See the [code documention](@ref demos_iShapes) for more details, and
the source.

Building on Linux with make                                                     {#ishapeslinuxbuild}
===========================

To build the demo you need to have installed a QT4 development
environment. Assuming that this is the case, then you should need to
simply do the following:

    $ cd $OSPL_HOME/demos/iShapes
    $ make

Building on Windows with Visual Studio                                          {#ishapeswinbuild}
======================================

To build the demo you need to have installed a QT4 development
environment. These can be downloaded from http://qt-project.org/
or alternately: adequate binary distributions containing tools
to build and libraries to run the example can be downloaded from
opensplice.org. Choose from:

[Qt for 32 bit Windows] (http://dev.opensplice.org/releases/downloads/releases/qt4.8.4_vs2010_32bit.zip)

[Qt for 64 bit Windows] (http://dev.opensplice.org/releases/downloads/releases/qt4.8.4_vs2010_64bit.zip)

... depending on your machine architecture.

After downloading:
1. Unzip the contents to some location e.g. to `C:\` creating
`C:\Qt\4.8.4_vs2010_32bit`
2. Install the Visual Studio C++ Redistributable by double clicking
it with Windows Explorer or running it from a commnad prompt e.g.:

    C:\Qt\4.8.4_vs2010_32bit\vcredist_x86>vcredist_x86.exe /q
3. Set the environment variable QTDIR to be the directory Qt is
installed somehow before launching Visual Studio. e.g.:

    C:\>set QTDIR=C:\Qt\4.8.4_vs2010_32bit
    C:\>devenv /useenv
4. Open the `iShapes.sln` file in `%%OSPL_HOME%\demos\iShapes` and
build it.

Running iShapes                                                                 {#ishapesrunning}
===============

The iShapes demo allows you to publish and subscribe instances of
various shapes. The demo is nicer if you run it as at least two
instances, thus from a shell do one of the below then have one of
the applications publish some shapes and press the subscribe button
on the other application to receive them.

Linux:
------

Assuming Qt libs are already available on the library load path:

    $ ./release.com
    $ demo_ishapes &
    $ demo_ishapes &

Windows:
--------

The `Qt .dll`s must be available on the `PATH` for the demo to run.
Assuming, for example, that steps 1 & 2 have been followed from
[Building on Windows](#ishapeswinbuild), then from the **OpenSplice
command prompt** you might do:

    C:\> set PATH=C:\Qt\4.8.4_vs2010_32bit\bin;%PATH%
    C:\> start demo_ishapes
    C:\> start demo_ishapes
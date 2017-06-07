Examples                                                                        {#face_examples}
========

[TOC]

FACE Examples                                                                   {#face_examples_dcpslist}
=============

The examples source code can be found in the examples folder of your OpenSplice installation.
The descriptions below summarize what each example demonstrates. Towards the bottom of the page
you will find instructions on how to build and run the examples.

HelloWorld                                                                              {#helloworld}
----------
- This is a simple publisher and subscriber example. One sample is published, the sample
is then read by the subscriber.

Building in a POSIX environment with make                                       {#face_dcpsexamples_posixbuild}
=========================================

In the following sections `$OSPL_HOME` identifies the installation directory of
OpenSplice. *Note* it is not however required that this be exported as a variable
in the shell environment.

Building a single example
-------------------------

Change directory to the example source directory and `make`. E.g.:

    $ cd $OSPL_HOME/examples/face/HelloWorld/cpp
    $ make

Building all examples
---------------------

The top-level makefile `Makefile.DCPS_ISO_CPlusPlus` can be used to build all
the examples.

    $ cd $OSPL_HOME/examples
    $ make -f Makefile.DCPS_ISO_CPlusPlus

Building on Windows with Visual Studio                                          {#face_dcpsexamples_winbuild}
======================================

A link is installed on the Windows Start Menu in the *Examples* Folder under
the OpenSplice HDE folder. Alternately: navigate to the `\examples` folder in
the OpenSplice HDE installation with Windows Explorer or the Visual Studio
`file -> open` dialogue and open the solution file `DCPS_ISO_CPlusPlus.sln`.

Running examples                                                               {#face_dcpsexamples_running}
================

The environment property `OSPL_URI` can set to indicate the location of a local
domain configuration file. If this variable is not set, or is set to point to a
file that specifies a _shared memory_ domain configuration, the command
`ospl start` must be used to start the domain before an example can be run.

The default configuration file installed by the distribution uses _single process_
(i.e. heap memory) mode so this is not required.

POSIX:                                                                                  {#runposix}
------

In the following examples `$OSPL_HOME` identifies the installation directory of
OpenSplice. *Note* it is not however required that this be exported as a variable
in the shell environment.

A script `release.com` is provided. It exports a value of `OSPL_URI` to indicate
the file `$OSPL_HOME/etc/config/ospl.xml` and also makes the OpenSplice libraries
available on the library load path.

To run an example in _single process_ mode:

    $ cd $OSPL_HOME
    $ ./release.com
    $ cd examples/face/HelloWorld/cpp
    $ subscriber &
    $ publisher

To run an exmaple in _shared memory_ mode you might do:

    $ cd $OSPL_HOME
    $ ./release.com
    $ OSPL_URI=file://$OSPL_HOME/etc/config/ospl_shmem_ddsi.xml
    $ export OSPL_URI
    $ cd examples/face/HelloWorld/cpp
    $ ospl start
    $ subscriber &
    $ publisher
    $ ospl stop

Windows:                                                                                {#runwindows}
--------

In the following examples `%%OSPL_HOME%` identifies the installation directory of
OpenSplice. *Note* it is not however required that this be set as a variable
in the console environment.

A script `release.bat` is provided. It exports a value of `OSPL_URI` to indicate
the file `%%OSPL_HOME%\etc\config\ospl.xml` and also makes the OpenSplice
executables and dynamic-link libraries available on the `PATH`.

A shortcut within the OpenSplice HDE folder on the Start Menu: **OpenSplice DDS
command prompt** creates a `cmd.exe` console session with this environment ready
set.

To run an example in _single process_ mode:

    "<<< OpenSplice HDE Release V6.6.4 For x86_64.win64, Date 2016-08-01 >>>"
    cd examples\face\HelloWorld\cpp
    start /b subscriber
    start publisher

To run an example in _shared memory_ mode you might do:

    "<<< OpenSplice HDE Release V6.6.4 For x86_64.win64, Date 2016-08-01 >>>"
    set OSPL_URI=file://%OSPL_HOME%\etc\config\ospl_shmem_ddsi.xml
    cd examples\face\HelloWorld\cpp
    ospl start
    start /b subscriber
    start publisher
    ospl stop



















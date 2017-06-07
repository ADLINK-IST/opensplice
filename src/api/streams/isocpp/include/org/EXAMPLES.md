Examples                                                                        {#isocpp_streams_examples}
========

[TOC]

The examples source code can be found in the examples/streams folder of your OpenSplice installation.
The descriptions below summarise what each example demonstrates. Towards the bottom of the page
you will find instructions on how to build and run the examples.

Streams Examples                                                                {#isocpp_streams_examples_streamslist}
=============

Throughput                                                                      {#streams_throughput}
----------
- The Streams Throughput example measures data throughput in bytes per second using
the Streams API to reduce overhead and maximuse the transfer rate. The publisher allows
you to specify a payload size in bytes as well as allowing you to specify when to flush
the stream after either a specified number of samples has been sent or after a time out
duration. The publisher will continue to send data forever unless a program time out is
specified. The subscriber will receive data and output the total amount received and the
data rate in bytes per second. It will also indicate if any samples were received out of
order. The maximum samples per read can be varied, for best perormance this should match
the flush max samples. A maximum number of cycles can be specified and once this has been
reached the subscriber will terminate and output totals and averages.

Building in a POSIX environment with make                                       {#isocpp_streams_examples_posixbuild}
=========================================

In the following sections `$OSPL_HOME` identifies the installation directory of
OpenSplice. *Note* it is not however required that this be exported as a variable
in the shell environment.

Building a single example
-------------------------

Change directory to the example source directory and `make`. E.g.:

    $ cd $OSPL_HOME/examples/streams/Throughput/isocpp
    $ make

Building all examples
---------------------

The top-level makefile `Makefile.DCPS_ISO_CPlusPlus` can be used to build all
the examples.

    $ cd $OSPL_HOME/examples
    $ make -f Makefile.DCPS_ISO_CPlusPlus

Building on Windows with Visual Studio                                          {#isocpp_streams_examples_winbuild}
======================================

A link is installed on the Windows Start Menu in the *Examples* Folder under
the OpenSplice HDE folder. Alternately: navigate to the `\examples` folder in
the OpenSplice HDE installation with Windows Explorer or the Visual Studio
`file -> open` dialogue and open the solution file `DCPS_ISO_CPlusPlus.sln`.

Running examples                                                               {#isocpp_streams_examples_running}
================

The environment property `OSPL_URI` can set to indicate the location of a local
domain configuration file. If this variable is not set, or is set to point to a
file that specifies a _shared memory_ domain configuration, the command
`ospl start` must be used to start the domain before an example can be run.

The default configuration file installed by the distribution uses _single process_
(i.e. heap memory) mode so this is not required.

POSIX:
------

In the following examples `$OSPL_HOME` identifies the installation directory of
OpenSplice. *Note* it is not however required that this be exported as a variable
in the shell environment.

A script `release.com` is provided. It exports a value of `OSPL_URI` to indicate
the file `$OSPL_HOME/etc/config/ospl.xml` and also makes the OpenSplice libaries
available on the library load path.

To run an example in _single process_ mode:

    $ cd $OSPL_HOME
    $ ./release.com
    $ cd examples/streams/Throughput/isocpp
    $ subscriber &
    $ publisher

To run an exmaple in _shared memory_ mode you might do:

    $ cd $OSPL_HOME
    $ ./release.com
    $ OSPL_URI=file://$OSPL_HOME/etc/config/ospl_shmem_ddsi.xml
    $ export OSPL_URI
    $ cd examples/streams/Throughput/isocpp
    $ ospl start
    $ subscriber &
    $ publisher
    $ ospl stop

Windows:
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

    "<<< OpenSplice HDE Release V6.3.0 For x86_64.win64, Date 2013-03-27 >>>"
    C:\ospl\V6.3.0\HDE\x86_64.win64>cd examples\streams\Throughput\isocpp
    C:\ospl\V6.3.0\HDE\x86_64.win64\examples\streams\Throughput\isocpp>start subscriber
    C:\ospl\V6.3.0\HDE\x86_64.win64\examples\streams\Throughput\isocpp>start publisher

To run an example in _shared memory_ mode you might do:

    "<<< OpenSplice HDE Release V6.3.0 For x86_64.win64, Date 2013-03-27 >>>"
    C:\ospl\V6.3.0\HDE\x86_64.win64>set OSPL_URI=file://%OSPL_HOME%\etc\config\ospl_shmem_ddsi.xml
    C:\ospl\V6.3.0\HDE\x86_64.win64>cd examples\streams\Throughput\isocpp
    C:\ospl\V6.3.0\HDE\x86_64.win64\examples\streams\Throughput\isocpp>ospl start
    C:\ospl\V6.3.0\HDE\x86_64.win64\examples\streams\Throughput\isocpp>start subscriber
    C:\ospl\V6.3.0\HDE\x86_64.win64\examples\streams\Throughput\isocpp>start publisher
    C:\ospl\V6.3.0\HDE\x86_64.win64\examples\streams\Throughput\isocpp>ospl stop

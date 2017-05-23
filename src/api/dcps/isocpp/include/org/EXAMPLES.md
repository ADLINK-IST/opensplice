Examples                                                                        {#isocpp_examples}
========

[TOC]

DCPS Examples                                                                   {#isocpp_examples_dcpslist}
=============

The examples source code can be found in the examples folder of your OpenSplice installation.
The descriptions below summarize what each example demonstrates. Towards the bottom of the page
you will find instructions on how to build and run the examples.

Tutorial                                                                                {#tutorial}
--------
- This example is a good starting point as it provides an overview of how to use
the most commonly used features of the API to make a message board application.

HelloWorld                                                                              {#helloworld}
----------
- This is a simple publisher and subscriber example. One sample is published, the
sample is then read. Some non-default QoS are used to guarantee delivery and to
allow the publisher to be optionally run before the subscriber.

ContentFilteredTopic                                                                    {#contentfilteredtopic}
--------------------
- In this example a ContentFilteredTopic is used to filter the messages received by
the subscriber to be only those with a ticket equal to the one supplied as a parameter
to the program. Some non-default QoS are used to guarantee delivery.

Durability                                                                              {#durability}
----------
- This example demonstrates the two kinds of durability, transient and persistent
and allows the user to specify which durability kind is used.

Listener                                                                                {#listener}
--------
- In this example a listener is registered on the datareader of the subscriber which
listens for the receipt of a message from the publisher and handles it. Some
non-default QoS are used to guarantee delivery and to allow the publisher to be
optionally run before the subscriber.

Lifecycle                                                                               {#lifecycle}
---------
- This is example demonstrates the changes in Lifecycle states when a DataWriter is
reated, sends samples or is deleted.

Ownership                                                                               {#ownership}
---------
- This example demonstrates the Ownership QoS. The subscriber will read messages sent
by the publishers and display those with the highest ownership strength.

QueryCondition                                                                          {#querycondition}
--------------
- This is a reasonably simple case publisher and subscriber example. One sample is
published, the sample is then read. Some non-default QoS are used to guarantee
delivery and to allow the publisher to be optionally run before the subscriber.

RoundTrip                                                                               {#roundtrip}
---------
- The RoundTrip example consists of a Ping and a Pong application. Ping sends samples
to Pong by writing to the Ping partition which Pong subscribes to. Pong them sends
them back to Ping by writing on the Pong partition which Ping subscribes to. Ping
measures the amount of time taken to write and read each sample as well as the total
round trip time to send a sample to Pong and receive it back.

Throughput                                                                              {#throughput}
----------
- The Throughput example measures data throughput in bytes per second. The publisher
allows you to specify a payload size in bytes as well as allowing you to specify
whether to send data in bursts. The publisher will continue to send data forever
unless a time out is specified. The subscriber will receive data and output the
total amount received and the data rate in bytes per second. It will also indicate
if any samples were received out of order. A maximum number of cycles can be
specified and once this has been reached the subscriber will terminate and output
totals and averages.

WaitSet                                                                                 {#waitset}
-------
- This example demonstrates how a WaitSet can be used to wait on certain conditions
which will then trigger a corresponding handler functor to perform any actions
required such as reading data.

See also the \subpage ishapes_readme

Building in a POSIX environment with make                                       {#isocpp_examples_posixbuild}
=========================================

In the following sections `$OSPL_HOME` identifies the installation directory of
OpenSplice. *Note* it is not however required that this be exported as a variable
in the shell environment.

Building a single example
-------------------------

Change directory to the example source directory and `make`. E.g.:

    $ cd $OSPL_HOME/examples/dcps/HelloWorld/isocpp
    $ make

Building all examples
---------------------

The top-level makefile `Makefile.DCPS_ISO_CPlusPlus` can be used to build all
the examples.

    $ cd $OSPL_HOME/examples
    $ make -f Makefile.DCPS_ISO_CPlusPlus

Building on Windows with Visual Studio                                          {#isocpp_examples_winbuild}
======================================

A link is installed on the Windows Start Menu in the *Examples* Folder under
the OpenSplice HDE folder. Alternately: navigate to the `\examples` folder in
the OpenSplice HDE installation with Windows Explorer or the Visual Studio
`file -> open` dialogue and open the solution file `DCPS_ISO_CPlusPlus.sln`.

Running examples                                                               {#isocpp_examples_running}
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
    $ cd examples/isocpp/HelloWorld/isocpp
    $ subscriber &
    $ publisher

To run an exmaple in _shared memory_ mode you might do:

    $ cd $OSPL_HOME
    $ ./release.com
    $ OSPL_URI=file://$OSPL_HOME/etc/config/ospl_shmem_ddsi.xml
    $ export OSPL_URI
    $ cd examples/isocpp/HelloWorld/isocpp
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

    "<<< OpenSplice HDE Release V6.3.0 For x86_64.win64, Date 2013-03-27 >>>"
    C:\ospl\V6.3.0\HDE\x86_64.win64>cd examples\isocpp\HelloWorld\isocpp
    C:\ospl\V6.3.0\HDE\x86_64.win64\examples\isocpp\HelloWorld\isocpp>start subscriber
    C:\ospl\V6.3.0\HDE\x86_64.win64\examples\isocpp\HelloWorld\isocpp>start publisher

To run an example in _shared memory_ mode you might do:

    "<<< OpenSplice HDE Release V6.3.0 For x86_64.win64, Date 2013-03-27 >>>"
    C:\ospl\V6.3.0\HDE\x86_64.win64>set OSPL_URI=file://%OSPL_HOME%\etc\config\ospl_shmem_ddsi.xml
    C:\ospl\V6.3.0\HDE\x86_64.win64>cd examples\isocpp\HelloWorld\isocpp
    C:\ospl\V6.3.0\HDE\x86_64.win64\examples\isocpp\HelloWorld\isocpp>ospl start
    C:\ospl\V6.3.0\HDE\x86_64.win64\examples\isocpp\HelloWorld\isocpp>start subscriber
    C:\ospl\V6.3.0\HDE\x86_64.win64\examples\isocpp\HelloWorld\isocpp>start publisher
    C:\ospl\V6.3.0\HDE\x86_64.win64\examples\isocpp\HelloWorld\isocpp>ospl stop

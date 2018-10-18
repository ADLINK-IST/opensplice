Examples                                                                        {#sacs_dcpsexamples}
========

[TOC]

DCPS Examples                                                                   {#sacs_dcpsexamples_dcpslist}
=============

The examples source code can be found in the examples folder of your OpenSplice installation.
The descriptions below summarize what each example demonstrates. Towards the bottom of the page
you will find instructions on how to build and run the examples.

Tutorial                                                                                {#tutorial}
--------
- This example is a good starting point as it provides an overview of how to use
the most commonly used features of the API to make a message board application.
- [Example Description.] (../../../examples/dcps/Tutorial/README.html)

HelloWorld                                                                              {#helloworld}
----------
- This is a simple publisher and subscriber example. One sample is published, the
sample is then read. Some non-default QoS are used to guarantee delivery and to
allow the publisher to be optionally run before the subscriber.
- [Example Description.] (../../../examples/dcps/HelloWorld/README.html)

ContentFilteredTopic                                                                    {#contentfilteredtopic}
--------------------
- In this example a ContentFilteredTopic is used to filter the messages received by
the subscriber to be only those with a ticket equal to the one supplied as a parameter
to the program. Some non-default QoS are used to guarantee delivery.
- [Example Description.] (../../../examples/dcps/ContentFilteredTopic/README.html)

Durability                                                                              {#durability}
----------
- This example demonstrates the two kinds of durability, transient and persistent
and allows the user to specify which durability kind is used.
- [Example Description.] (../../../examples/dcps/Durability/README.html)

Listener                                                                                {#listener}
--------
- In this example a listener is registered on the datareader of the subscriber which
listens for the receipt of a message from the publisher and handles it. Some
non-default QoS are used to guarantee delivery and to allow the publisher to be
optionally run before the subscriber.
- [Example Description.] (../../../examples/dcps/Listener/README.html)

Lifecycle                                                                               {#lifecycle}
---------
- This is example demonstrates the changes in Lifecycle states when a DataWriter is
reated, sends samples or is deleted.
- [Example Description.] (../../../examples/dcps/Lifecycle/README.html)

Ownership                                                                               {#ownership}
---------
- This example demonstrates the Ownership QoS. The subscriber will read messages sent
by the publishers and display those with the highest ownership strength.
- [Example Description.] (../../../examples/dcps/Ownership/README.html)

QueryCondition                                                                          {#querycondition}
--------------
- This is a reasonably simple case publisher and subscriber example. One sample is
published, the sample is then read. Some non-default QoS are used to guarantee
delivery and to allow the publisher to be optionally run before the subscriber.
- [Example Description.] (../../../examples/dcps/QueryCondition/README.html)

RoundTrip                                                                               {#roundtrip}
---------
- The RoundTrip example consists of a Ping and a Pong application. Ping sends samples
to Pong by writing to the Ping partition which Pong subscribes to. Pong them sends
them back to Ping by writing on the Pong partition which Ping subscribes to. Ping
measures the amount of time taken to write and read each sample as well as the total
round trip time to send a sample to Pong and receive it back.
- [Example Description.] (../../../examples/dcps/RoundTrip/README.html)

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
- [Example Description.] (../../../examples/dcps/Throughput/README.html)

WaitSet                                                                                 {#waitset}
-------
- This example demonstrates how a WaitSet can be used to wait on certain conditions
which will then trigger a corresponding handler functor to perform any actions
required such as reading data.
- [Example Description.] (../../../examples/dcps/WaitSet/README.html)

BuiltinTopics                                                                           {#builtintopics}
-------------
- This example demonstrates the use of builtin topics by monitoring the number of
nodes that participate in a DDS domain.
- [Example Description.] (../../../examples/dcps/BuiltInTopics/README.html)

See also the \subpage ishapes_readme

Building on Windows with Visual Studio                                          {#sacs_dcpsexamples_winbuild}
======================================

A link is installed on the Windows Start Menu in the *Examples* Folder under
the OpenSplice HDE folder. Alternately: navigate to the `\examples` folder in
the OpenSplice HDE installation with Windows Explorer or the Visual Studio
`file -> open` dialogue and open the solution file `CSharp.sln`.

Running examples                                                               {#sacs_dcpsexamples_running}
================

The environment property `OSPL_URI` can set to indicate the location of a local
domain configuration file. If this variable is not set, or is set to point to a
file that specifies a _shared memory_ domain configuration, the command
`ospl start` must be used to start the domain before an example can be run.

The default configuration file installed by the distribution uses _single process_
(i.e. heap memory) mode so this is not required.

Windows:                                                                                {#runwindows}
--------

In the following examples `%%OSPL_HOME%` identifies the installation directory of
OpenSplice. *Note* it is not however required that this be set as a variable
in the console environment.

A script `release.bat` is provided. It exports a value of `OSPL_URI` to indicate
the file `%%OSPL_HOME%\etc\config\ospl.xml` and also makes the OpenSplice
executables and dynamic-link libraries available on the `PATH`.

A shortcut within the OpenSplice HDE folder on the Start Menu: **Vortex OpenSplice
command prompt** creates a `cmd.exe` console session with this environment ready
set.

To run an example in _single process_ mode:

    "<<< OpenSplice HDE Release V6.3.0 For x86_64.win64, Date 2013-03-27 >>>"
    C:\ospl\V6.3.0\HDE\x86_64.win64>cd examples\dcps\HelloWorld\cs\standalone
    C:\ospl\V6.3.0\HDE\x86_64.win64\examples\dcps\HelloWorld\cs\standalone>start sacs_helloworld_sub.exe
    C:\ospl\V6.3.0\HDE\x86_64.win64\examples\dcps\HelloWorld\cs\standalone>start sacs_helloworld_pub.exe

To run an example in _shared memory_ mode you might do:

    "<<< OpenSplice HDE Release V6.3.0 For x86_64.win64, Date 2013-03-27 >>>"
    C:\ospl\V6.3.0\HDE\x86_64.win64>set OSPL_URI=file://%OSPL_HOME%\etc\config\ospl_shmem_ddsi.xml
    C:\ospl\V6.3.0\HDE\x86_64.win64>cd examples\dcps\HelloWorld\cs\standalone
    C:\ospl\V6.3.0\HDE\x86_64.win64\examples\dcps\HelloWorld\sacs\standalone>ospl start
    C:\ospl\V6.3.0\HDE\x86_64.win64\examples\dcps\HelloWorld\sacs\standalone>start sacs_helloworld_sub.exe
    C:\ospl\V6.3.0\HDE\x86_64.win64\examples\dcps\HelloWorld\sacs\standalone>start sacs_helloworld_pub.exe
    C:\ospl\V6.3.0\HDE\x86_64.win64\examples\dcps\HelloWorld\sacs\standalone>ospl stop

Examples                                                                        {#java5_examples}
========

[TOC]

DCPS Examples                                                                   {#java5_examples_dcpslist}
=============

The examples source code can be found in the examples folder of your OpenSplice installation.
The descriptions below summarize what each example demonstrates. Towards the bottom of the page
you will find instructions on how to build and run the examples.

Tutorial                                                                                {#tutorial}
--------
- This example is a good starting point as it provides an overview of how to use the
most commonly used features of the API to make a message board application.

HelloWorld                                                                              {#helloworld}
----------
- This is a simple publisher and subscriber example. One sample is published, the sample
is then read. Some non-default QoS are used to guarantee delivery and to allow the
publisher to be optionally run before the subscriber.

ContentFilteredTopic                                                                    {#contentfilteredtopic}
--------------------
- In this example a ContentFilteredTopic is used to filter the messages received by the
subscriber to be only those with a ticket equal to the one supplied as a parameter to
the program. Some non-default QoS are used to guarantee delivery.

Durability                                                                              {#durability}
----------
- This example demonstrates the two kinds of durability, transient and persistent and
allows the user to specify which durability kind is used.

Listener                                                                                {#listener}
--------
In this example a listener is registered on the datareader of the subscriber which listens
for the receipt of a message from the publisher and handles it. Some non-default QoS are
used to guarantee delivery and to allow the publisher to be optionally run before the subscriber.

Lifecycle                                                                               {#lifecycle}
---------
- This is example demonstrates the changes in Lifecycle states when a DataWriter is reated,
sends samples or is deleted.

Ownership                                                                               {#ownership}
---------
- This example demonstrates the Ownership QoS. The subscriber will read messages sent by the
publishers and display those with the highest ownership strength.

QueryCondition                                                                          {#querycondition}
--------------
- This is a reasonably simple case publisher and subscriber example. One sample is published,
the sample is then read. Some non-default QoS are used to guarantee delivery and to allow
the publisher to be optionally run before the subscriber.

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


Building the examples                                           {#java5_build_examples}
=========================================

In the following sections `$OSPL_HOME` identifies the installation directory of
OpenSplice. *Note* it is not however required that this be exported as a variable
in the shell environment.

In order to build and or run applications that use java5 the library `dcpssaj5.jar` for standalone java or `dcpscj5.jar`
for CORBA java need to be in the classPath. In case of doubt use the `dcpssaj5.jar` for standalone java.
These libraries can be found inside the `$OSPL_HOME/jar` directory.


POSIX:                                                                                  {#buildposix}
------

### General posix build/run environment setup                                           {#posixgeneral}
In the following examples `$OSPL_HOME` identifies the installation directory of
OpenSplice. *Note* it is not however required that this be exported as a variable
in the shell environment.

A script `release.com` is provided. It exports a value of `OSPL_URI` to indicate
the file `$OSPL_HOME/etc/config/ospl.xml` and also makes the OpenSplice libraries
available on the library load path.

### Building a single example

Change directory to the example source directory and `make`. E.g.:

    $ cd $OSPL_HOME
    $ ./release.com
    $ cd $OSPL_HOME/examples/dcps/HelloWorld/java5
    $ make

### Building all examples

The top-level makefile `Makefile.All_Java` can be used to build all
the examples.

    $ cd $OSPL_HOME
    $ ./release.com
    $ cd $OSPL_HOME/examples
    $ make -f Makefile.All_Java

Windows:                                                                                {#buildwindows}
--------

### General windows build/run environment setup                                         {#windowsgeneral}

In the following examples `%%OSPL_HOME%` identifies the installation directory of
OpenSplice. *Note* it is not however required that this be set as a variable
in the console environment.

A script `release.bat` is provided. It exports a value of `OSPL_URI` to indicate
the file `%%OSPL_HOME%\etc\config\ospl.xml` and also makes the OpenSplice
executables and dynamic-link libraries available on the `PATH`.

A shortcut within the OpenSplice HDE folder on the Start Menu: **OpenSplice DDS
command prompt** creates a `cmd.exe` console session with this environment ready
set.

### Building a single example

Change directory to the example source directory and `make`. E.g.:

    open the OpenSplice DDS command prompt
    "<<< OpenSplice HDE Release V6.5.0 For x86_64.win64, Date 2015-01-22 >>>"
    C:\ospl\V6.5.0\HDE\x86_64.win64>cd examples\HelloWorld\java5
    C:\ospl\V6.5.0\HDE\x86_64.win64\examples\HelloWorld\java5>BUILD_java.bat

### Building all examples

The top-level makefile `Makefile.All_Java` can be used to build all
the examples.

    open the OpenSplice DDS command prompt
    "<<< OpenSplice HDE Release V6.5.0 For x86_64.win64, Date 2015-01-22 >>>"
    C:\ospl\V6.5.0\HDE\x86_64.win64>cd examples
    C:\ospl\V6.5.0\HDE\x86_64.win64\examples>BUILD_All_Java.bat

Running examples                                                               {#java5_examples_running}
================

The environment property `OSPL_URI` can set to indicate the location of a local
domain configuration file. If this variable is not set, or is set to point to a
file that specifies a _shared memory_ domain configuration, the command
`ospl start` must be used to start the domain before an example can be run.

The default configuration file installed by the distribution uses _single process_
(i.e. heap memory) mode so this is not required.

POSIX:                                                                                  {#runposix}
------

To run an example in _single process_ mode:

    $ cd $OSPL_HOME
    $ ./release.com
    $ cd examples/HelloWorld/java5/standalone
    $ java -classpath $OSPL_HOME/jar/dcpssaj5.jar:classes HelloWorldDataSubscriber &
    $ java -classpath $OSPL_HOME/jar/dcpssaj5.jar:classes HelloWorldDataPublisher

To run an exmaple in _shared memory_ mode you might do:

    $ cd $OSPL_HOME
    $ ./release.com
    $ OSPL_URI=file://$OSPL_HOME/etc/config/ospl_shmem_ddsi.xml
    $ export OSPL_URI
    $ cd examples/HelloWorld/java5/standalone
    $ ospl start
    $ java -classpath $OSPL_HOME/jar/dcpssaj5.jar:classes HelloWorldDataSubscriber &
    $ java -classpath $OSPL_HOME/jar/dcpssaj5.jar:classes HelloWorldDataPublisher
    $ ospl stop

Windows:                                                                                {#runwindows}
--------

To run an example in _single process_ mode:

    open the OpenSplice DDS command prompt
    "<<< OpenSplice HDE Release V6.5.0 For x86_64.win64, Date 2015-01-22 >>>"
    C:\ospl\V6.5.0\HDE\x86_64.win64>cd examples\HelloWorld\java5\standalone
    C:\ospl\V6.5.0\HDE\x86_64.win64\examples\HelloWorld\java5\standalone>start /b java -classpath "%OSPL_HOME%\jar\dcpssaj5.jar";classes HelloWorldDataSubscriber
    C:\ospl\V6.5.0\HDE\x86_64.win64\examples\HelloWorld\java5\standalone>java -classpath "%OSPL_HOME%\jar\dcpssaj5.jar";classes HelloWorldDataPublisher


To run an example in _shared memory_ mode you might do:

    open the OpenSplice DDS command prompt
    "<<< OpenSplice HDE Release V6.5.0 For x86_64.win64, Date 2015-01-22 >>>"
    C:\ospl\V6.5.0\HDE\x86_64.win64>set OSPL_URI=file://%OSPL_HOME%\etc\config\ospl_shmem_ddsi.xml
    C:\ospl\V6.5.0\HDE\x86_64.win64>cd examples\HelloWorld\java5\standalone
    C:\ospl\V6.5.0\HDE\x86_64.win64\examples\HelloWorld\java5\standalone>ospl start
    C:\ospl\V6.5.0\HDE\x86_64.win64\examples\HelloWorld\java5\standalone>start /b java -classpath "%OSPL_HOME%\jar\dcpssaj5.jar";classes HelloWorldDataSubscriber
    C:\ospl\V6.5.0\HDE\x86_64.win64\examples\HelloWorld\java5\standalone>java -classpath "%OSPL_HOME%\jar\dcpssaj5.jar";classes HelloWorldDataPublisher
    C:\ospl\V6.5.0\HDE\x86_64.win64\examples\HelloWorld\java5\standalone>ospl stop

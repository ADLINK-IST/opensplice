Examples                                                                        {#face_examples}
========

[TOC]

FACE Examples                                                                   {#face_examples_dcpslist}
=============

The examples source code can be found in the examples folder of your Vortex OpenSplice installation.
The descriptions below summarize what each example demonstrates. Towards the bottom of the page
you will find instructions on how to build and run the examples.

HelloWorld                                                                              {#helloworld}
----------
- This is a simple publisher and subscriber example. One sample is published, the sample
is then read by the subscriber.



Building the examples                                           {#face_build_examples}
=========================================

In the following sections `$OSPL_HOME` identifies the installation directory of
Vortex OpenSplice. *Note* it is not however required that this be exported as a variable
in the shell environment.

In order to build and or run applications that use face the library `ddsface.jar` need to be in the classPath.
This library can be found inside the `$OSPL_HOME/jar` directory.


POSIX:                                                                                  {#buildposix}
------

### General posix build/run environment setup                                           {#posixgeneral}
In the following examples `$OSPL_HOME` identifies the installation directory of
Vortex OpenSplice. *Note* it is not however required that this be exported as a variable
in the shell environment.

A script `release.com` is provided. It exports a value of `OSPL_URI` to indicate
the file `$OSPL_HOME/etc/config/ospl.xml` and also makes the Vortex OpenSplice libraries
available on the library load path.

### Building a single example

Change directory to the example source directory and `make`. E.g.:

    $ cd $OSPL_HOME
    $ ./release.com
    $ cd $OSPL_HOME/examples/face/HelloWorld/java
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
Vortex OpenSplice. *Note* it is not however required that this be set as a variable
in the console environment.

A script `release.bat` is provided. It exports a value of `OSPL_URI` to indicate
the file `%%OSPL_HOME%\etc\config\ospl.xml` and also makes the Vortex OpenSplice
executables and dynamic-link libraries available on the `PATH`.

A shortcut within the Vortex OpenSplice HDE folder on the Start Menu: **Vortex OpenSplice
command prompt** creates a `cmd.exe` console session with this environment ready
set.

### Building a single example

Change directory to the example source directory and `make`. E.g.:

    open the Vortex OpenSplice command prompt
    "<<< OpenSplice HDE Release V6.6.4 For x86_64.win64, Date 2016-08-01 >>>"
    cd examples\face\HelloWorld\java
   BUILD_java.bat

### Building all examples

The top-level makefile `Makefile.All_Java` can be used to build all
the examples.

    open the Vortex OpenSplice command prompt
    "<<< OpenSplice HDE Release V6.6.4 For x86_64.win64, Date 2016-08-01 >>>"
    cd examples
    BUILD_All_Java.bat

Running examples                                                               {#face_examples_running}
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
    $ cd examples/face/HelloWorld/java
    $ java -classpath $OSPL_HOME/jar/ddsface.jar:classes HelloWorldDataSubscriber &
    $ java -classpath $OSPL_HOME/jar/ddsface.jar:classes HelloWorldDataPublisher

To run an exmaple in _shared memory_ mode you might do:

    $ cd $OSPL_HOME
    $ ./release.com
    $ OSPL_URI=file://$OSPL_HOME/etc/config/ospl_shmem_ddsi.xml
    $ export OSPL_URI
    $ cd examples/face/HelloWorld/java
    $ ospl start
    $ java -classpath $OSPL_HOME/jar/ddsface.jar:classes HelloWorldDataSubscriber &
    $ java -classpath $OSPL_HOME/jar/ddsface.jar:classes HelloWorldDataPublisher
    $ ospl stop

Windows:                                                                                {#runwindows}
--------

To run an example in _single process_ mode:

    open the Vortex OpenSplice command prompt
    "<<< OpenSplice HDE Release V6.6.4 For x86_64.win64, Date 2016-08-01 >>>"
    cd examples\face\HelloWorld\java
    start /b java -classpath "%OSPL_HOME%\jar\ddsface.jar";classes HelloWorldDataSubscriber
    java -classpath "%OSPL_HOME%\jar\ddsface.jar";classes HelloWorldDataPublisher


To run an example in _shared memory_ mode you might do:

    open the Vortex OpenSplice command prompt
    "<<< OpenSplice HDE Release V6.6.4 For x86_64.win64, Date 2016-08-01 >>>"
    set OSPL_URI=file://%OSPL_HOME%\etc\config\ospl_shmem_ddsi.xml
    cd examples\face\HelloWorld\java
    ospl start
    start /b java -classpath "%OSPL_HOME%\jar\ddsface.jar";classes HelloWorldDataSubscriber
    java -classpath "%OSPL_HOME%\jar\ddsface.jar";classes HelloWorldDataPublisher
    ospl stop

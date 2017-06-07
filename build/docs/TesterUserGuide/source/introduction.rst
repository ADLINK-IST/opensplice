.. _`Introduction`:


.. forward XREFS to deal with when all other chapters in place

############
Introduction
############

*This section provides a brief introduction to the Vortex OpenSplice Tester.*


Features
********

The Vortex OpenSplice Automated Testing and Debugging Tool provides an easy way 
of displaying messages produced in Vortex OpenSplice and also provides means to 
publish messages manually or with a script.

(The OpenSplice Automated Testing and Debugging Tool is usually referred 
to as *Tester*; the name ``ospltest`` is used when referring to the 
executable program.)

This tool is made with the software tester, and the way he performs his 
job, in mind. A pre-defined list of topics of interest can be provided. 
For all topics a reader is created in the correct partition. Once 
started, the tool receives all instances of the topics of interest and 
will display them in the sample list in the order they were produced 
(using the source time stamp). This makes it very easy to see when 
topics are produced and in what order. It also provides feedback about 
unexpected updates.

Other features of Tester include the ability to:

+ dump a selection of topic(s) to a file
+ dump all logged topic instances to a file
+ filter the sample list based on key
+ filter the sample list based on key and topic name
+ filter the sample list based on key
+ create a script with a selection of previously sent or received topics
+ compare topic samples
+ edit topic samples and then write them or dispose the topic instance
+ create new topic samples and write them or dispose the topic instances


Location of Tester in the OpenSplice architecture
*************************************************

Tester is complementary to OpenSplice Tuner (``ospltun``). Tuner 
supports ‘white box’ application monitoring and tuning, and Tester 
supports ‘black box’ system testing, debugging, data capture, analysis, 
and visualization.


Things to Know
**************

|caution|
  **NOTE**: Tester uses the internal Control & Monitoring (C&M) API 
  for access to OpenSplice. At this time Tester only supports 
  Vortex OpenSplice systems. 
  
Tester can be used both locally (*via* shared memory or single process) 
and/or remotely (*via* a SOAP connection to the SOAP service).

Tester uses a periodic poll to read data (the default poll period is 
250 ms). The normal restrictions for storage scope apply (only keys defined 
with the topic separate topics for reading, if topics with the same key 
are produced within a polling period, then only the last topic is read).

Tester uses the default QoS for writing (as provided by the first 
application which registers the topic) and the weakest QoS for reading. 
However when specifying the topic in ``add topic`` (or ``add topics``) 
or in the topic file the QoS can be given, this QoS must be compatible 
with the topic QoS as defined when the topic was registered.

|caution|
  **NOTE**: In order for the Tester system browser to correctly show the 
  complete system, OpenSplice Durability services have to be properly 
  configured so that the transient ‘built-in-topics’ are properly aligned 
  when new nodes join the system. Monitoring the built-in topic sample set 
  on different nodes will quickly reveal any failure in correct lining-up 
  of transient data (in which case there will be different sets visible on 
  different nodes). Monitoring the ``DCPSHeartbeat`` built-in topic will 
  reveal fundamental connectivity issues in your system (you should see as 
  many unique heartbeats as there are interconnected nodes in the system).


Prerequisites
*************

Tester is included in the standard OpenSplice installation.

Tester’s minimum system requirements are the same as for OpenSplice 
itself; please refer to the *Release Notes* for both Tester and OpenSplice 
for details. The Vortex OpenSplice *Getting Started Guide* contains 
additional information about installation and configuration on various 
systems.

Note that to compile plugins you will also need to have ``ant`` and 
JDK1.6 installed (see 
:ref:`Getting Started with a Message Interface <Start with Message Interface>`).
Tester has been implemented in the Java Language, and it uses 
the Vortex OpenSplice Command and Management (C&M) API.

Although Tester uses the C&M API, it doesn’t depend on a locally 
installed or running instantiation of OpenSplice. It can operate either 
‘co-located’ with a running DDS target system, or it can operate in 
‘remote-connection’ mode (like the Tuner).

When Tester is run on the same platform as Vortex OpenSplice, it uses the 
``OSPL_HOME`` environment variable to find the necessary OpenSplice library 
files. It also uses ``OSPL_URI`` as its default OpenSplice configuration.

When Tester connects to a remote ‘target’ platform using SOAP it doesn’t 
use any local environment variables, it just needs to be installed on 
the machine where you run it.

(Note that the Vortex OpenSplice Tuner can be started with a ``-uri`` command-line 
parameter (see how to :ref:`start a remote connection <Starting - Remote Connection>`
). This is a new feature that is actually used by the Tester in the system browser, 
where you can spawn a Tuner (see how to 
:ref:`spawn a tuner from the system browser <Spawn Tuner from System Browser>`)
that then connects to the 
node/application that the browser is pointing to.)






.. |caution| image:: ./images/icon-caution.*
            :height: 6mm
.. |info|   image:: ./images/icon-info.*
            :height: 6mm
.. |windows| image:: ./images/icon-windows.*
            :height: 6mm
.. |unix| image:: ./images/icon-unix.*
            :height: 6mm
.. |linux| image:: ./images/icon-linux.*
            :height: 6mm
.. |c| image:: ./images/icon-c.*
            :height: 6mm
.. |cpp| image:: ./images/icon-cpp.*
            :height: 6mm
.. |csharp| image:: ./images/icon-csharp.*
            :height: 6mm
.. |java| image:: ./images/icon-java.*
            :height: 6mm

         
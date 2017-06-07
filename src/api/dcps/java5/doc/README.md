Java 5 DCPS API                                                            {#mainpage}
====================

[TOC]

Java 5 Language PSM for DDS is the latest DDS DCPS API for the Java Language. It is more user friendly
and simpler to use.

Getting started                                                                {#mainpage_gettingstarted}
===============

If you are new to the Java 5 PSM we would recommend that you start by looking at the example
code provided in your OpenSplice installation directory as well as the documentation
in the sections below.

A summary of each example, along with build/run instructions can be found on the
\subpage java5_examples page.

The OMG Java 5 specification also gives an overview of the general design of Java 5 and
can be found at the following link: http://www.omg.org/spec/DDS-Java/

In order to build and or run applications that use Java 5 the library `dcpssaj5.jar` for standalone Java or `dcpscj5.jar`
for CORBA Java need to be in the classPath. In case of doubt use the `dcpssaj5.jar` for standalone Java.
These libraries can be found inside the `$OSPL_HOME/jar` directory.

API documentation                                                               {#mainpage_api}
=================

The namespaces below contain all of the major functionality of the Java 5 API.

- org::omg::dds

   + org::omg.dds.domain
    + The org::omg::dds::domain namespace provides facilities for creating a DomainParticipant
      that establishes a virtual network in which an application and it's entities
      can operate.
   + org::omg::dds::topic
    + The org::omg::dds::topic namespace provides facilities for the creation of Topics which
      can be published or subscribed to, as well as providing more advanced filtering
      through ContentFilteredTopics.
   + org::omg::dds::pub
    + The org::omg::dds::pub namespace provides facilities for writing samples based on a Topic.
   + org::omg::dds::sub
    + The org::omg::dds::sub namespace provides facilities for reading samples based on a Topic,
      as well as providing the ability to query a read.
   + org::omg::dds::core
    + The org::omg::dds::core namespace provides core facilities such as QoS policies and the
      ability to wait on particular conditions using a WaitSet.

Java 5 code examples can be found on the \subpage snippets page.

Java 5 Source:
--------

A zipped version of the java5 source code can be found inside the `$OSPL_HOME/lib` directory i.e. `dcpssaj5-src.jar` or `dcpscj5-src.jar`.

To use this source jar inside Eclipse do the following:

1. Right click over the Project -> Build path -> Configure Build Path
2. In the new window, go to the "Libraries" tab.
3. Select the library `dcpssaj5.jar` or `dcpscj5.jar` and expand it.
4. There are 4 child options. Select "Source attachment" and click the "Edit" button on the right.
5. Now the `dcpssaj5-src.jar` or `dcpscj5-src.jar` can be added.

JavaDoc:
--------

A zipped version of the JavaDoc documentation for java5 can be found inside the `$OSPL_HOME/docs/java5` directory i.e. `dcpssaj5-doc.jar` or `dcpscj5-doc.jar`.

To use this jar inside Eclipse do the following:

1. Right click over the Project -> Build path -> Configure Build Path
2. In the new window, go to the "Libraries" tab.
3. Select the library `dcpssaj5.jar` or `dcpscj5.jar` and expand it.
4. There are 4 child options. Select "Javadoc location" and click the "Edit" button on the right.
5. Now the `dcpssaj5-doc.jar` or `dcpscj5-doc.jar` can be added. press the "Validate" button to check everything is ok.


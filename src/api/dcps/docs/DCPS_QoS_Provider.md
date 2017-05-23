QoS Provider              {#DCPS_QoS_Provider}
===========

[TOC]

This page provides information on using the QoS provider API of OpenSpliceDDS. The QoS provider API allows users to specify the QoS settings of their DCPS entities outside of application code in XML. This can be seen as a useful feature where code recompilation is restricted during the later stages of application development / during application support. The following sections explain the API in a language independent manner and explain how to build QoS Profiles in XML.


API reference       {#DCPS_QoS_Provider_APIref}
=============

Introduction           {#DCPS_QoS_Provider_Introduction}
-----------

The QosProvider is delivered as part of the DCPS API of OpenSpliceDDS and has no factory. It is created as an object directly by the natural means in each language binding.
This is because it is not necessarily associated with a single DomainParticipant. Below you'll find a link to the available operations, which are explained in detail there.

\if java5
\ref org.omg.dds.core.QosProvider "Java 5 QosProvider API"
\endif
\if isocpp
\ref dds::core::TQosProvider "ISOCPP QosProvider API"
\endif
\if isocpp2
\ref dds::core::TQosProvider "ISOCPP2 QosProvider API"
\endif
\if cs
\ref DDS.QosProvider "C# QosProvider API"
\endif
XML file syntax       {#DCPS_QoS_Provider_XMLsyn}
=============

The XML configuration file must follow these syntax rules:

- The syntax is XML and the character encoding is UTF-8.
- Opening tags are enclosed in `<>`; closing tags are enclosed in `</>`.
- A value is a UTF-8 encoded string. Legal values are alphanumeric characters.
- All values are case-sensitive unless otherwise stated.
- Comments are enclosed as follows: <\!-- comment -->.
- The root tag of the configuration file must be and end with .

Please also see: [OMG DDS4CCM](http://www.omg.org/spec/dds4ccm/1.1/) specification for more details on the exact notation.

The primitive types for tag values are specified in the following table:

|Type | Format | Notes|
|-----|--------|------|
Boolean | yes, 1, true or BOOLEAN_TRUE | Not case-sensitive |
| | no, 0, false or BOOLEAN_FALSE | |
|Enum | A string. Legal values are the ones defined for QoS Policies in the DCPS IDL of DDS specification. | Must be specified as a string. (Do not use numeric values.)|
|Long | -2147483648 to 2147483647 or 0x80000000 to 0x7fffffff or LENGTH_UNLIMITED | A 32-bit signed integer|
|UnsignedLong | 0 to 4294967296 or 0 to 0xffffffff | A 32-bit unsigned integer|

Entity QoS      {#DCPS_QoS_Provider_Entity_QoS}
----------

To configure the QoS for a DDS Entity using XML, the following tags have to be used:
- `<domainparticipant_qos>`
- `<publisher_qos>`
- `<subscriber_qos>`
- `<topic_qos>`
- `<datawriter_qos>`
- `<datareader_qos>`

Each XML tag with an associated name can be uniquely identified by its fully qualified name in C++ style.

### QoS Policies

The fields in a QosPolicy are described in XML using a 1-to-1 mapping with the equivalent IDL representation in the DDS specification. For example, the Reliability QosPolicy is represented with the following structures:
~~~~~~~~~~~~~{.c}
struct Duration_t {
    long sec;
    unsigned long nanosec;
};

struct ReliabilityQosPolicy {
    ReliabilityQosPolicyKind kind;
    Duration_t max_blocking_time;
};
~~~~~~~~~~~~~
The equivalent representation in XML is as follows:
~~~~~~~~~~~~~{.xml}
<reliability>
    <kind></kind>
    <max_blocking_time>
        <sec></sec>
        <nanosec></nanosec>
    </max_blocking_time>
</reliability>
~~~~~~~~~~~~~

### Sequences

In general, the sequences contained in the QoS policies are described with the following XML format:
~~~~~~~~~~~~~{.xml}
<a_sequence_member_name>
    <element>...</element>
    <element>...</element>
    ...
</a_sequence_member_name>
~~~~~~~~~~~~~
Each element of the sequence is enclosed in an `<element>` tag., as shown in the following example:
~~~~~~~~~~~~~{.xml}
<property>
    <value>
        <element>
            <name>my name</name>
            <value>my value</value>
        </element>
        <element>
            <name>my name2</name>
            <value>my value2</value>
        </element>
    </value>
</property>
~~~~~~~~~~~~~

A sequence without elements represents a sequence of length 0. For example: `<a_sequence_member_name/>` As a special case, sequences of octets are represented with a single XML tag enclosing a sequence of decimal/hexadecimal values between 0..255 separated with commas. For example:
~~~~~~~~~~~~~{.xml}
<user_data>
    <value>100,200,0,0,0,223</value>
</user_data>
<topic_data>
    <value>0xff,0x00,0x8e,0xEE,0x78</value>
</topic_data>
~~~~~~~~~~~~~
### Arrays

In general, the arrays contained in the QoS policies are described with the following XML format:
~~~~~~~~~~~~~{.xml}
<an_array_member_name>
    <element>...</element>
    <element>...</element>
    ...
</an_array_member_name>
~~~~~~~~~~~~~
Each element of the array is enclosed in an `<element>` tag. As a special case, arrays of octets are represented with a single XML tag enclosing an array of decimal/hexadecimal values between 0..255 separated with commas. For example:
~~~~~~~~~~~~~{.xml}
<datareader_qos>
    ...
    <user_data>
        <value>100,200,0,0,0,223</value>
    </user_data>
</datareader_qos>
~~~~~~~~~~~~~
### Enumerations

Enumeration values are represented using their IDL string representation. For example:
~~~~~~~~~~~~~{.xml}
<history>
    <kind>KEEP_ALL_HISTORY_QOS</kind>
</history>
~~~~~~~~~~~~~

### Time values (Durations)

Following values can be used for fields that require seconds or nanoseconds:
- DURATION_INFINITE_SEC
- DURATION_ZERO_SEC
- DURATION_INFINITE_NSEC
- DURATION_ZERO_NSEC

The following example shows the use of time values:
~~~~~~~~~~~~~{.xml}
<deadline>
    <period>
        <sec>DURATION_INFINITE_SEC</sec>
        <nanosec>DURATION_INFINITE_NSEC</nanosec>
    </period>
</deadline>
~~~~~~~~~~~~~

QoS Profiles        {#DCPS_QoS_Provider_QoS_Profiles}
------------

A QoS profile groups a set of related QoS, usually one per entity. For example:
~~~~~~~~~~~~~{.xml}
<qos_profile name="StrictReliableCommunicationProfile">
    <datawriter_qos>
        <history>
            <kind>KEEP_ALL_HISTORY_QOS</kind>
        </history>
        <reliability>
            <kind>RELIABLE_RELIABILITY_QOS</kind>
        </reliability>
    </datawriter_qos>
    <datareader_qos>
        <history>
            <kind>KEEP_ALL_HISTORY_QOS</kind>
        </history>
        <reliability>
            <kind>RELIABLE_RELIABILITY_QOS</kind>
        </reliability>
    </datareader_qos>
</qos_profile>
~~~~~~~~~~~~~
QoS Profiles with a Single QoS

The definition of an individual QoS is a shortcut for defining a QoS profile with a single QoS. For example:
~~~~~~~~~~~~~{.xml}
<datawriter_qos name="KeepAllWriter">
    <history>
        <kind>KEEP_ALL_HISTORY_QOS</kind>
    </history>
</datawriter_qos>
~~~~~~~~~~~~~
is equivalent to the following:
~~~~~~~~~~~~~{.xml}
<qos_profile name="KeepAllWriter">
    <datawriter_qos >
        <history>
            <kind>KEEP_ALL_HISTORY_QOS</kind>
        </history>
    </datawriter_qos >
</qos_profile>
~~~~~~~~~~~~~

Example     {#DCPS_QoS_Provider_Example}
=======
consider the following XML file that describes two QoS profiles;
- FooQosProfile
  + DataReaderQos - TransientKeepLast
  + DataWriterQos - Transient
- BarQosProfile
  + DataWriterQos - Persistent
~~~~~~~~~~~~~{.xml}
<dds xmlns="http://www.omg.org/dds/"
        xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
        xsi:schemaLocation="file://DDS_QoSProfile.xsd">
    <qos_profile name="FooQosProfile">
        <datareader_qos name="TransientKeepLast">
            <durability>
                <kind>TRANSIENT_DURABILITY_QOS</kind>
            </durability>
            <history>
               <kind>KEEP_LAST_HISTORY_QOS</kind>
               <depth>5</depth>
            </history>
        </datareader_qos>
        <datawriter_qos name="Transient">
            <durability>
                <kind>TRANSIENT_DURABILITY_QOS</kind>
            </durability>
        </datawriter_qos>
    </qos_profile>
    <qos_profile name="BarQosProfile">
        <datawriter_qos name="Persistent">
            <durability>
                <kind>PERSISTENT_DURABILITY_QOS</kind>
            </durability>
        </datawriter_qos>
    </qos_profile>
</dds>
~~~~~~~~~~~~~
\if c
C Example     {#DCPS_QoS_Provider_Example_C}
------------
The following C application is an example to illustrate how the QoS settings from the above XML could be accessed.

\include qos_provider.c
\endif
\if cpp
C++ Example     {#DCPS_QoS_Provider_Example_Cplusplus}
------------
The following C++ application is an example to illustrate how the QoS settings from the above XML could be accessed.

\include qos_provider.cpp
\endif
\if isocpp
ISOCPP Example     {#DCPS_QoS_Provider_Example_Isocpp}
------------
The following ISOCPP application is an example to illustrate how the QoS settings from the above XML could be accessed.

\include qos_provider_isocpp.cpp
\endif
\if isocpp2
ISOCPP2 Example     {#DCPS_QoS_Provider_Example_Isocpp}
------------
The following ISOCPP2 application is an example to illustrate how the QoS settings from the above XML could be accessed.

\include qos_provider_isocpp.cpp
\endif
\if java
Java Example     {#DCPS_QoS_Provider_Example_Java}
------------
The following Java application is an example to illustrate how the QoS settings from the above XML could be accessed.

\include QosProviderJava.java
\endif
\if java5
Java 5 Example     {#DCPS_QoS_Provider_Example_Java5}
------------
The following Java 5 application is an example to illustrate how the QoS settings from the above XML could be accessed.

\include QosProviderJava5.java
\endif
\if cs
C# Example     {#DCPS_QoS_Provider_Example_Cs}
------------
The following C# application is an example to illustrate how the QoS settings from the above XML could be accessed.

\include QosProviderExample.cs
\endif

Known limitations       {#DCPS_QoS_Provider_Limitations}
=================
- Inheritance of QoS policies and QoS profiles in XML using the "base_name" attribute is not supported yet.
- The "topic_filter" attribute for writer,reader, and topic QoSs to associate a set of topics to a specific QoS when that QoS is part of a DDS profile, is not supported yet.
- All leading and trailing spaces should be removed from the string before it is processed. For example, "<tag> value </tag>" should be the same as "<tag>value</tag>"
- Currently parsing of sequences and arrays only supports sequence of strings

FACE Configuration                                                              {#face_configuration}
========

In order to use the FACE API with DDS a configuration file is needed. This page describes all configuration options that are available and what they do.

## Vortex FACE configuration options
- [connections_list](#connections_list)
    - [connection](#connection)<br/>
        - [name](#name)<br/>
        - [type](#type)<br/>
        - [direction](#direction)<br/>
        - [platform_view_guid](#platform_view_guid)<br/>
        - [refresh_period](#refresh_period)<br/>
        - [domain_id](#domain_id)<br/>
        - [topic_name](#topic_name)<br/>
        - [type_name](#type_name)<br/>
        - [qos](#qos)<br/>
            - [uri](#uri)<br/>
            - [profile](#profile)<br/>
            - [domainparticipant_qos_id](#domainparticipant_qos_id)<br/>
            - [topic_qos_id](#topic_qos_id)<br/>
            - [publisher_qos_id](#publisher_qos_id)<br/>
            - [datawriter_qos_id](#datawriter_qos_id)<br/>
            - [subscriber_qos_id](#subscriber_qos_id)<br/>
            - [datareader_qos_id](#datareader_qos_id)<br/>

A configuration example can be found [here](#example)<br/>
Best practice tips can be found [here](#bp)<br/>

<div id='connections_list'/>
## connections_list
Container that will hold the configuration for the connections.
<br/>
- Full path: /connections_list
- Occurrences min-max: 1-1
- This element is Mandatory

___
<div id='connection'/>
### connection
This will contain the configuration of a specific connection. It is identified by `<name>`.
<br/>
- Full path: /connections_list/connection
- Occurrences min-max: 1-*
- This element is Mandatory

___
<div id='name'/>
####name
Connection identification. It has to be unique within the configuration file.
<br/>
DDS uses partitions when communicating. This name is used as partition id as well.
It can be overruled when the DDS Publisher QoS or DDS Subscriber QoS specifically state a different
partition (see \ref DCPS_QoS_Provider, `<qos><subscriber_qos_id>`and `<qos><publisher_qos_id>`).
<br/>
Often, a publisher application will use a different configuration file than the related subscriber application.
Then you can use the same name in the different configuration files, meaning that you have the same DDS partition automatically.
<br/>
- Full path: /connections_list/connection/name
- Occurrences min-max: 1-1
- This element is Mandatory

___
<div id='type'/>
#### type
Connection type. This is mapped on the FACE specification CONNECTION_TYPE enumeration.
Only "DDS" is supported.
<br/>
- Full path: /connections_list/connection/type
- Occurrences min-max: 1-1
- This element is Mandatory

___
<div id='direction'/>
#### direction
Connection direction. This is mapped on the FACE specification CONNECTION_DIRECTION_TYPE enumeration.
Only "SOURCE" and "DESTINATION" are supported.
<br/>
- Full path: /connections_list/connection/direction
- Occurrences min-max: 1-1
- This element is Mandatory

___
<div id='platform_view_guid'/>
#### platform_view_guid
The Platform View GUID is a reference to a specific Data Model Platform View defining the structure of the message.
<br/>
This will not be used by the Vortex FACE. It will only be returned during FACE Callback and Receive_Message() calls as Message Type GUID.
<br/>
- Full path: /connections_list/connection/platform_view_guid
- Occurrences min-max: 1-1
- This element is Mandatory

___
<div id='refresh_period'/>
#### refresh_period
This indicates the length of time in milliseconds a message is valid. It is only used for FACE when SAMPLING.
<br/>
Vortex FACE only supports DDS, which means that the refresh period is ignored. It is only returned in the connection status when Get_Connection_Parameters() is called.
<br/>
- Full path: /connections_list/connection/refresh_period
- Occurrences min-max: 1-1
- This element is Mandatory

___
<div id='domain_id'/>
#### domain_id
DDS domain ID.
This domain id has to be the same as the one provided in the ospl configuration xml.When no domain id is provided,
then the id present in the ospl configuration will be used to setup the connection. The value of a domain_id can be between 0 and 230
<br/>
- Full path: /connections_list/connection/domain_id
- Occurrences min-max: 0-1
- This element is Optional

___
<div id='topic_name'/>
#### topic_name
DDS Topic name. This is used by DDS as 'connection identification' part. There can be multiple 'DDS Connections' of the same type (`<type_name>`) as long as they have different topic names (`<topic_name>`).
<br/>
A topic name is an identifier for a topic, and is defined as any series of characters ‘a’, ..., ‘z’, ‘A’, ..., ‘Z’, ‘0’, ..., ‘9’, ‘-’, ‘_’ but may not start with a digit
- Full path: /connections_list/connection/topic_name
- Occurrences min-max: 1-1
- This element is Mandatory

___
<div id='type_name'/>
#### type_name
Data Type name. This is used to determine the data type. It has to be the completely scoped class name that represents the type.
This is used by DDS as 'connection identification' part. There can be multiple 'DDS Connections' of the same type (`<type_name>`) as long as they have different topic names (`<topic_name>`).
<br/>
- Full path: /connections_list/connection/type_name
- Occurrences min-max: 1-1
- This element is Mandatory

___
<div id='qos'/>
#### qos
Container to hold QosProvider information. DDS supports various Quality Of Service variables. The QosProvider is used to supply this feature to the FACE API. See \ref DCPS_QoS_Provider for more information.
<br/>
- Full path: /connections_list/connection/qos
- Occurrences min-max: 0-1
- This element is Optional

___
<div id='uri'/>
##### uri
Location and name of the QosProvider profiles XML file.
<br/>
- Full path: /connections_list/connection/qos/uri
- Occurrences min-max: 1-1
- This element is Mandatory

___
<div id='profile'/>
##### profile
The name, within the QosProvider XML file, of the profile to use.
<br/>
- Full path: /connections_list/connection/qos/profile
- Occurrences min-max: 1-1
- This element is Mandatory

___
<div id='domainparticipant_qos_id'/>
##### domainparticipant_qos_id
The name, within the QosProvider XML file, of
the DDS DomainParticipant QoS profile to use.
<br/>
When not provided, the default QoS within the
profile will be used.
When not provided in the profile either, then
the DDS default will be used.
<br/>
- Full path: /connections_list/connection/qos/domainparticipant_qos_id
- Occurrences min-max: 0-1
- This element is Optional

___
<div id='topic_qos_id'/>
##### topic_qos_id
The name, within the QosProvider XML file, of
the DDS Topic QoS profile to use.
<br/>
When not provided, the default QoS within the
profile will be used.
When not provided in the profile either, then
the DDS default will be used.
<br/>
- Full path: /connections_list/connection/qos/topic_qos_id
- Occurrences min-max: 0-1
- This element is Optional

___
<div id='publisher_qos_id'/>
##### publisher_qos_id
The name, within the QosProvider XML file, of
the DDS Publisher QoS profile to use.
<br/>
When not provided, the default QoS within the
profile will be used.
When not provided in the profile either, then
the DDS default will be used.
<br/>
- Full path: /connections_list/connection/qos/publisher_qos_id
- Occurrences min-max: 0-1
- This element is Optional

___
<div id='datawriter_qos_id'/>
##### datawriter_qos_id
The name, within the QosProvider XML file, of
the DDS DataWriter QoS profile to use.
<br/>
When not provided, the default QoS within the
profile will be used.
When not provided in the profile either, then
the DDS default will be used.
<br/>
- Full path: /connections_list/connection/qos/datawriter_qos_id
- Occurrences min-max: 0-1
- This element is Optional

___
<div id='subscriber_qos_id'/>
##### subscriber_qos_id
The name, within the QosProvider XML file, of
the DDS Subscriber QoS profile to use.
<br/>
When not provided, the default QoS within the
profile will be used.
When not provided in the profile either, then
the DDS default will be used.
<br/>
- Full path: /connections_list/connection/qos/subscriber_qos_id
- Occurrences min-max: 0-1
- This element is Optional

___
<div id='datareader_qos_id'/>
##### datareader_qos_id
The name, within the QosProvider XML file, of
the DDS DataReader QoS profile to use.
<br/>
When not provided, the default QoS within the
profile will be used.
When not provided in the profile either, then
the DDS default will be used.
<br/>
- Full path: /connections_list/connection/qos/datareader_qos_id
- Occurrences min-max: 0-1
- This element is Optional

___

<div id='bp'/>

## Best practice

For FACE Connections to be able to communicate, they have to have:
- Same DDS Partition:       either `<name>` or partition within the DDS Publisher/Subscriber QoSses
- Same DDS Topic Name:      `<topic_name>`
- Same DDS Type Name:       `<type_name>`
- Opposite FACE directions: one has to be SOURCE and the other DESTINATION

<div id='example'/>

Configuration Example
---------------------
\code{.xml}
<connections_list>
    <connection>
        <name>HelloWorldPub</name>
        <type>DDS</type>
        <direction>SOURCE</direction>
        <platform_view_guid>1</platform_view_guid>
        <refresh_period>1000</refresh_period>
        <!--domain_id>0</domain_id-->
        <topic_name>JavaFaceHelloWorld</topic_name>
        <type_name>HelloWorldData.Msg</type_name>
        <qos>
            <uri>file://defaults.xml</uri>
            <profile>HelloWorld</profile>
            <!--
            <domainparticipant_qos_id>Foo</domainparticipant_qos_id>
            <topic_qos_id>Foo</topic_qos_id>
            <publisher_qos_id>Foo</publisher_qos_id>
            <datawriter_qos_id>Foo</datawriter_qos_id>
            <subscriber_qos_id>Foo</subscriber_qos_id>
            <datareader_qos_id>Foo</datareader_qos_id>
            -->
        </qos>
    </connection>
    <connection>
        <name>HelloWorldSub</name>
        <type>DDS</type>
        <direction>DESTINATION</direction>
        <platform_view_guid>1</platform_view_guid>
        <refresh_period>1000</refresh_period>
        <!--domain_id>0</domain_id-->
        <topic_name>JavaFaceHelloWorld</topic_name>
        <type_name>HelloWorldData.Msg</type_name>
        <qos>
            <uri>file://defaults.xml</uri>
            <profile>HelloWorld</profile>
            <!--
            <domainparticipant_qos_id>Foo</domainparticipant_qos_id>
            <topic_qos_id>Foo</topic_qos_id>
            <publisher_qos_id>Foo</publisher_qos_id>
            <datawriter_qos_id>Foo</datawriter_qos_id>
            <subscriber_qos_id>Foo</subscriber_qos_id>
            <datareader_qos_id>Foo</datareader_qos_id>
            -->
        </qos>
    </connection>
</connections_list>
\endcode


.. _`Configuration`:

#############
Introduction
#############

This guide describes the various configuration elements and attributes available for
Vortex OpenSplice. The configuration items should be added to an XML file
and then the OSPL_URI environment variable should be set to point to the
path of that XML file with the "file://" URI prefix.

- e.g.
    - Linux: export OSPL_URI=file://$OSPL_HOME/etc/ospl.xml
    - Windows: set OSPL_URI=file://%OSPL_HOME%\\\\etc\\\\ospl.xml

The ospl.xml file supplied with Vortex OpenSplice contains the following:

.. code-block:: xml

   <OpenSplice>
      <Domain>
        <Name>ospl_sp_ddsi</Name>
        <Id>0</Id>
        <SingleProcess>true</SingleProcess>
        <Service name="ddsi2">
            <Command>ddsi2</Command>
        </Service>
        <Service name="durability">
            <Command>durability</Command>
        </Service>
        <Service name="cmsoap">
            <Command>cmsoap</Command>
        </Service>
      </Domain>
      <DDSI2Service name="ddsi2">
        <General>
            <NetworkInterfaceAddress>AUTO</NetworkInterfaceAddress>
            <AllowMulticast>true</AllowMulticast>
            <EnableMulticastLoopback>true</EnableMulticastLoopback>
            <CoexistWithNativeNetworking>false</CoexistWithNativeNetworking>
        </General>
        <Compatibility>
            <!-- see the release notes and/or the OpenSplice configurator on DDSI interoperability -->
            <StandardsConformance>lax</StandardsConformance>
            <!-- the following one is necessary only for TwinOaks CoreDX DDS compatibility -->
            <!-- <ExplicitlyPublishQosSetToDefault>true</ExplicitlyPublishQosSetToDefault> -->
        </Compatibility>
      </DDSI2Service>
      <DurabilityService name="durability">
        <Network>
            <Alignment>
                <TimeAlignment>false</TimeAlignment>
                <RequestCombinePeriod>
                    <Initial>2.5</Initial>
                    <Operational>0.1</Operational>
                </RequestCombinePeriod>
            </Alignment>
            <WaitForAttachment maxWaitCount="100">
                <ServiceName>ddsi2</ServiceName>
            </WaitForAttachment>
        </Network>
        <NameSpaces>
            <NameSpace name="defaultNamespace">
                <Partition>*</Partition>
            </NameSpace>
            <Policy alignee="Initial" aligner="true" durability="Durable" nameSpace="defaultNamespace"/>
        </NameSpaces>
      </DurabilityService>
      <TunerService name="cmsoap">
        <Server>
            <PortNr>Auto</PortNr>
        </Server>
      </TunerService>
      <Description>Stand-alone 'single-process' deployment and standard DDSI networking.</Description>
   </OpenSplice>

The tags in the XML file should be nested in the same way as they are in
the table of contents in this configuration guide. The nesting and numbering of
the tags in the contents of this guide allows you to see which elements are
the parent or children of one another. For example, if you wanted to find
a description of the NetworkInterfaceAddress attribute, you would first navigate
to it's parent, the General element, and inside that you would find a
heading for the child NetworkInterfaceAddress attribute along with a
description and valid values. Some attributes may state that they are required
and if so these elements must be present when the parent element is included in
the XML file.

If you wanted to add a new element, say to enable security, you would
navigate to the Security element of the guide. This has a child element
called SecurityProfile which should be nested within the Security element.
Each element lists a number of occurences, this states how many times this element
can appear in your XML file. The SecurityProfile element has three attributes,
Name, which is required, and Cipher and CipherKey  which are optional.
Attributes are added within the parent element tag in the format name="value".
Adding these new elements and attributes would result in the following XML:

.. code-block:: xml

  <OpenSplice>
    <Domain>
       <Name>ospl_sp_ddsi</Name>
       <Id>0</Id>
    </Domain>
    <DDSI2E>
        <General>
            <NetworkInterfaceAddress>AUTO</NetworkInterfaceAddress>
            <AllowMulticast>true</AllowMulticast>
            <EnableMulticastLoopback>true</EnableMulticastLoopback>
        </General>
        <Compatibility>
            <StandardsConformance>lax</StandardsConformance>
        </Compatibility>
        <Tracing>
            <Verbosity>warning</Verbosity>
        </Tracing>
        <Security>
            <SecurityProfile Name="GlobalProfile" Cipher="blowfish" CipherKey="00000000000000000000000000000000"/>
        </Security>
    </DDSI2E>
  </OpenSplice>

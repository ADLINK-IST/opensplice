.. _`The NetworkingBridge Service`:

############################
The NetworkingBridge Service
############################

*The OpenSplice NetworkingBridge is a pluggable service that allows bridging of
data between networking services. This section gives an overview of the features of
the NetworkingBridge.*

The configuration parameters that control the behaviour of the NetworkingBridge
are described in the  :ref:`Configuration <Configuration>` section.

.. _`Background`:

Background
**********

When a networking service is selected that best suits a specific deployment,
sometimes a part of the data needs to be obtained from or disclosed to a system that
is using a different kind of networking service. The NetworkingBridge allows
``DCPSPublications`` and ``DCPSSubscriptions`` to be matched and the related
data forwarded between a RTNetworking system and a DDSI2 system and *vice
versa*.

The NetworkingBridge employs a fast path in the OpenSplice kernel by directly
connecting the network queues of the bridged services. This also allows full
end-to-end flow control mechanisms to be realised across the bridge. Which
publications/subscriptions are bridged can be controlled by means of white- and
black-lists.

The NetworkingBridge relies on the discovery of publications and subscriptions by
the common means for the networking services. This means that it relies on the real
transient topics, aligned by the Durability service, for the RTNetworking part of the
bridge. For the part that connects to DDSI2 the native DDSI2 discovery of
end-points is used. In order for DDSI2 to only advertise bridged publications and
subscriptions, the ``LocalDiscoveryPartition`` used for regular discovery should
be set to a non-existent partition, as can be seen in the following example. This
discovery takes some time and can introduce a short delay before data is bridged.


.. _`Example Configuration`:

Example Configuration
*********************

In order to properly configure the NetworkingBridge for bridging data between
RTNetworking and DDSI2, both networking services (and the Durability service for
the alignment of the builtin topics of the RTNetworking side) have to be configured.
Filtering is also configured with the NetworkingBridge.

An example configuration file for bridging of all data (excluding Topic
*MyLocalTopic*) in partition *BridgedPartition* is shown below.

.. code-block:: xml
   
   <OpenSplice>
      <Domain>
         <Name>NetworkingBridgeExample</Name>
         <Id>0</Id>
         <Service name="networking">
            <Command>networking</Command>
         </Service>
         <Service name="ddsi2e">
            <Command>ddsi2e</Command>
         </Service>
         <Service name="nwbridge">
           <Command>nwbridge</Command>
         </Service>
         <Service name="durability">
            <Command>durability</Command>
         </Service>
      </Domain>
      <NetworkService name="networking">
         <Partitioning>
            <GlobalPartition Address="broadcast"/>
         </Partitioning>
         <Channels>
            <Channel default="true" enabled="true" name="BestEffort" reliable="false">
               <PortNr>54400</PortNr>
            </Channel>
            <Channel enabled="true" name="Reliable" reliable="true">
               <PortNr>54410</PortNr>
            </Channel>
         </Channels>
         <Discovery enabled="true">
            <PortNr>54420</PortNr>
         </Discovery>
      </NetworkService>
      <DDSI2EService name="ddsi2e">
         <Discovery>
           <LocalDiscoveryPartition>ThisIsNotAPartition</LocalDiscoveryPartition>
         </Discovery>
      </DDSI2EService>
      <NetworkingBridgeService name="nwbridge">
        <Include>
          <!-- Multiple entries can be added here with DCPSPartitionTopic expressions 
               on what to include -->
          <Entry DCPSPartitionTopic="BridgedPartition.*"/>
        </Include>
        <Exclude>
          <!-- Multiple entries can be added here with DCPSPartitionTopic expressions 
               on what to exclude.
               If a DCPSPublication or DCPSSubscription matches both the include- and 
               exclude expressions it will be excluded. -->
          <Entry DCPSPartitionTopic="*.MyLocalTopic"/>
        </Exclude>
      </NetworkingBridgeService>
      <DurabilityService name="durability">
         <Network>
            <Alignment>
               <TimeAlignment>FALSE</TimeAlignment>
               <RequestCombinePeriod>
                  <Initial>2.5</Initial>
                  <Operational>0.1</Operational>
               </RequestCombinePeriod>
            </Alignment>
            <WaitForAttachment maxWaitCount="10">
              <ServiceName>networking</ServiceName>
              <ServiceName>ddsi2e</ServiceName>
            </WaitForAttachment>
         </Network>
         <NameSpaces>
            <NameSpace name="defaultNamespace">
               <Partition>*</Partition>
            </NameSpace>
            <Policy nameSpace="defaultNamespace" durability="Durable" 
                              alignee="Initial" aligner="True"/>
         </NameSpaces>
      </DurabilityService>
      <Description>Federated deployment for extending an RTNetworking-based 
                   domain into a DDSI network.</Description>
   </OpenSplice>



.. EoF


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


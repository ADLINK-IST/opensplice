.. _`Using OpenSplice with Vortex Cloud`:

########################################################
Using Vortex OpenSplice with Vortex Cloud and Vortex Fog
########################################################


If Vortex OpenSplice is deployed using the *DDSI2/DDSI2E* network-services
in a LAN or a private cloud (multicast-enabled cloud) and Vortex Fog
is configured to discover user applications using UDP-multicast in the
LAN/private cloud, then Vortex OpenSplice does not need to be configured
with any particular configuration property as Vortex Fog will discover
and communicate with Vortex OpenSplice using the standardized multicast IP
address and UDP ports. The only constraint is that both Vortex OpenSplice
and Vortex Cloud are configured to participate in the same DDS domain 
(``domainID``).

If Vortex OpenSplice is deployed in a WAN (without multicast-support), then
it needs to be configured to use the TCP transport. It also needs to be
configured with the peers of one or several Discovery Services (DS) of
the deployed Vortex Cloud. If multiple DS's are available, Vortex OpenSplice
may be configured with several discovery service peers in a peers
group.

**Example with a single discovery service locator:**

.. code-block:: xml

  <DDSI2Service name="ddsi2">
  <General>
  .....
       </General>
  <TCP>
  <Enable>True</Enable>
  </TCP>
  <Discovery>
  <Peers>
  <Peer>1.1.1.1:7400</Peer>
  </Peers>
  </Discovery>
 

**Example with two discovery service locators:**

.. code-block:: xml 

  <Discovery>
  <Peers>
  <Peer>1.1.1.1:7400</Peer>
  <Peer>2.2.2.2:7400</Peer>
  </Peers>
  </Discovery>




|info|
*NOTE:* Unlike when deployed on a multicast LAN (where the ``domainID``
determines the multicast-address used for discovery), the addressed DS
of Vortex Cloud will discover (and subsequently match) data from *any*
DDS-domain utilized by Vortex OpenSplice. Utilizing multiple Vortex Cloud
discovery services in the WAN (*i.e.* one per domain) allows for
domain-specific discovery and subsequent routing if required.




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

   
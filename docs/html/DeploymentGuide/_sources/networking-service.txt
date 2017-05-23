.. _`The Networking Service`:

######################
The Networking Service
######################

*When communication endpoints are located on different computing nodes or on
different single processes, the data produced using the local Domain Service must
be communicated to the remote Domain Services and the other way around. The
Networking Service provides a bridge between the local Domain Service and a
network interface. Multiple Networking Services can exist next to each other; each
serving one or more physical network interfaces. The Networking Service is
responsible for forwarding data to the network and for receiving data from the
network.*

There are two implementations of the networking service:
`The Native Networking Service`_ and `The Secure Native Networking Service`_.

There are detailed descriptions of all of the available configuration 
parameters and their purpose in the  :ref:`Configuration <Configuration>`
section.


.. _`The Native Networking Service`:

The Native Networking Service
*****************************

For large-scale LAN-based systems that demand maximum throughput, the native
RTNetworking service is the optimal implementation of DDS networking for
Vortex OpenSplice and is both highly scalable and configurable.

*The Native Networking Service* can be configured to distinguish multiple
communication channels with different QoS policies. These policies will be used to
schedule individual messages to specific channels, which may be configured to
provide optimal performance for a specific application domain.

The exact fulfilment of these responsibilities is determined by the configuration of
the Networking Service.

Please refer to the  :ref:`Configuration <Configuration>` section for
fully-detailed descriptions of how to configure:

+  ``//OpenSplice/NetworkService``
+  ``//OpenSplice/SNetworkService``


.. _`The Secure Native Networking Service`:

The Secure Native Networking Service
************************************

There is a secure version of the native networking service available.

Please refer to the :ref:`Configuration <Configuration>`
section for details.


.. _`Compression`:

Compression
===========

This section describes the options available for configuring compression of the data
packets sent by the Networking Service.

In early OpenSplice 6.x releases, the *zlib* library was used at its default setting
whenever the compression option on a network partition was enabled. Now
it is possible to configure *zlib* for less cpu usage or for more compression effort, or
to select a compressor written specifically for high speed, or to plug in an alternative
algorithm.

The configuration for compression in a Networking Service instance is contained in
the optional top-level Element Compression.
These settings apply to all partitions in which compression is enabled.

Please refer to the  :ref:`Configuration <Configuration>` section for
a detailed description of:

+  ``//OpenSplice/NetworkService/Compression``


.. _`Availability`:

Availability
------------

The compression functionality is available on enterprise platforms (*i.e.* Linux,
Windows and Solaris). On embedded platforms there are no built-in compressors
included, but plugins may be used.


.. _`How to set the level parameter in zlib`:

How to set the level parameter in zlib
--------------------------------------

Set the Attribute ``PluginParameter`` to a single digit between ``0`` (no compression)
and ``9`` (maximum compression, more CPU usage). Leave the Attribute ``PluginLibrary``
and Attribute ``PluginInitFunction`` blank.


.. _`How to switch to other built-in compressors`:

How to switch to other built-in compressors
-------------------------------------------

Set the Attribute ``PluginInitFunction`` to the name of the initialisation 
function of one of the built-in compressors. These are ``/ospl_comp_zlib_init/``,
``/ospl_comp_lzf_init/`` and ``/ospl_comp_snappy_init/`` for *zlib*, *lzf* and
*snappy* respectively. As a convenience, the short names ``zlib``, ``lzf`` and
``snappy`` are also recognized.


|info| |caution|

  Please note that not all compressors are available on all platforms. 
  In this release *zlib* is available on Linux, Windows and Solaris; 
  *lzf* and *snappy* are available only on RedHat Linux.


.. _`How to write a plugin for another compression library`:

How to write a plugin for another compression library
-----------------------------------------------------

Other compression algorithms may be used by the Networking Service. In order to
do this it is necessary to build a library which maps the OpenSplice compression
API onto the algorithm in question. This library may contain the actual compressor
code or be dynamically linked to it.

Definitions for the compression API are provided in the include file
``plugin/nw_compPlugin.h``. 

Five functions must be implemented.

The ``maxsize`` function.
  This function is called when sizing a buffer into which to compress a network
  packet. It should therefore return the worst-case (largest) possible size of
  compressed data for a given uncompressed size. In most cases it is acceptable to
  return the uncompressed size, as the compress operation is allowed to fail if the
  resulting data is larger than the original (in which case the data is sent
  uncompressed). However, *snappy* for example will not attempt compression
  unless the destination buffer is large enough to take the worst possible result.

The ``compress`` function.
  This function takes a block of data of a given size and compresses it into a
  buffer of a given size. It returns the actual size of the compressed data, or zero 
  if an error ocurred (*e.g.* the destination buffer was not large enough).

The ``uncompress`` function.
  This function takes a block of compressed data of given size and uncompresses
  it into a buffer also of given size. It returns the actual size of the uncompressed
  data, or zero if an error ocurred (*e.g.* the data was not in a valid compressed
  format).

The ``exit`` function.
  This function is called at service shutdown and frees any resources used 
  by the plugin.

The ``init`` function.
  This function is called at service startup. It sets up the plugin by filling in a
  structure containing pointers to the four functions listed above. It also is passed
  the value of the Attribute ``PluginParameter``. The plugin configuration structure
  includes a pointer to some unspecified state data which may be used to hold this
  parameter and/or any storage required by the compressor. This pointer is passed
  into the ``compress`` and ``exit`` functions.

By way of illustration, here is a simplified version of the code for *zlib*. The
implementation is merely a veneer on the *zlib* library to present the required API.

.. code-block:: cpp

   #include "nw_compPlugin.h"
   #include "os_heap.h"
   #include
   unsigned long ospl_comp_zlib_maxsize (unsigned long srcsize)
   {
     /* if the data can't be compressed into the same size buffer we'll send
   uncompressed instead */
   return srcsize;
   }
   unsigned long ospl_comp_zlib_compress (void *dest, unsigned long destlen,
   const void *source, unsigned long srclen, void *param)
   {
     unsigned long compdsize = destlen;
     if (compress2 (dest, &compdsize, source, srclen, *(int *)param) == Z_OK)
     {
       return compdsize;
     }
     else
     {
       return 0;
     }
   }
   
   unsigned long ospl_comp_zlib_uncompress (void *dest, unsigned long
   destlen, const void *source, unsigned long srclen)
   {
     unsigned long uncompdsize = destlen;
     if (uncompress (dest, &uncompdsize, source, srclen) == Z_OK)
     {
       return uncompdsize;
     }
     else
     {
       return 0;
     }
   }
   
   void ospl_comp_zlib_exit (void *param)
   {
     os_free (param);
   }
   
   void ospl_comp_zlib_init (nw_compressor *config, const char *param)
   {
     /* param should contain an integer from 0 to 9 */
     int *iparam = os_malloc (sizeof (int));
     if (strlen (param) == 1)
     {
       *iparam = atoi (param);
     }
     else
     {
       *iparam = Z_DEFAULT_COMPRESSION;
     }
     config->maxfn = ospl_comp_zlib_maxsize;
     config->compfn = ospl_comp_zlib_compress;
     config->uncompfn = ospl_comp_zlib_uncompress;
     config->exitfn = ospl_comp_zlib_exit;
     config->parameter = (void *)iparam;
   }


.. _`How to configure for a plugin`:

How to configure for a plugin
=============================

**Step 1**:
  Set Attribute ``PluginLibrary`` to the name of the library containing
  the plugin implementation.

**Step 2**:
  Set Attribute ``PluginInitFunction`` to the name of the initialisation 
  function within that library.

**Step 3**:
  If the compression method is controlled by a parameter, set
  Attribute ``PluginParameter`` to configure it.

Please refer to the  :ref:`Configuration <Configuration>` section for
fully-detailed descriptions of how to configure:

+  ``//OpenSplice/NetworkService/Compression[@PluginLibrary]``
+  ``//OpenSplice/NetworkService/Compression[@PluginInitFunction]``
+  ``//OpenSplice/NetworkService/Compression[@PluginParameter]``


.. _`Constraints`:

Constraints
===========

|caution|

  The Networking Service packet format does *not* include identification of 
  which compressor is in use. 
  *It is therefore necessary to use the* **same** 
  *configuration on all nodes.*


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


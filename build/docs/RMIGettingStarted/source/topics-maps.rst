.. _`RMI Interface to DDS topics mapping rules`:

#########################################
RMI Interface to DDS topics mapping rules
#########################################


*This chapter demonstrates the mapping rules driving the transformation
of the IDL declarations of the RMI interfaces into the IDL declarations
of the implied DDS topics*.

+ For each ``<InterfaceName>``, a new module is created with the same name
  and scope in the module ``DDS_RMI``, where all the topics associated with
  the interface operations will be made.

+ Each ``<InterfaceName>.<operation name>`` creates two data structures,
  suffixed respectively with ``_request`` for the data structure that handles
  the request, and ``_reply`` for the data structure that handles the reply.

+ The ``<operation name>_request`` data struct will gather all ``[in]`` or
  ``[inout]`` parameters.

+ The ``<operation name>_reply`` data struct will gather the return value
  and all ``[inout]`` or ``[out]`` parameters.

+ ``req_info`` is used to enable the client service handler to pick the
  reply it is waiting for.


.. code-block:: Java

   module HelloWorld {
      local interface HelloService : ::DDS_RMI::Services
      {
         void op1 (in string p1, inout short p2, out long p3);
      };
   };

..

.. figure:: /images/RMI-diag-04.png
   :height: 20mm
   :alt: rmipp


.. code-block:: Java

   module DDS_RMI {
     module HelloWorld {
       module HelloService {

      struct op1_request {
              DDS_RMI::Request_Header req_info;
              string p1;
              short p2;
           };
      #pragma keylist op1_request req_info.client_id.client_impl 
                      req_info.client_id.client_instance

          struct op1_reply {
              DDS_RMI::Request_Header req_info;
              short p2;
              long p3;
          };
      #pragma keylist op1_reply req_info.client_id.client_impl
                      req_info.client_id.client_instance

       };
     };
   };

..


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

         
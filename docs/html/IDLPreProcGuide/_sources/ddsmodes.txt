.. _`OpenSplice Modes and Languages`:


##################################
OpenSplice Modes and Languages
##################################

*The Vortex OpenSplice IDL Pre-processor supports two modes:*

+  *Standalone* mode where the application is only used with Vortex OpenSplice
+  *ORB-integrated* mode where the application is used with an ORB as well 
   as with Vortex OpenSplice

In a *standalone* context, Vortex OpenSplice provides, apart from the DDS/DCPS
related artifacts, all the artifacts implied by the lDL language specific mapping. In
this case the used name space is DDS instead of the name space implied by the IDL
language specific mapping.

In an *ORB-integrated* context, the ORB pre-processor will provide for the artifacts
implied by the lDL language specific mapping, while Vortex OpenSplice only
provides the DDS/DCPS related artifacts. The application data type representation
provided by the ORB is also used within the Vortex OpenSplice context. In this way
application data types can be shared between the ORB and Vortex OpenSplice within
one application program.

The languages and modes that Vortex OpenSplice supports are listed in the table below.

.. centered:: **Supported Modes and Languages**

.. tabularcolumns:: | p{2.2cm} | p{3cm} | p{3.3cm} | p{5.5cm} |

+--------------------+--------------------+--------------------+--------------------+
| Language           | Mode               | OpenSplice Library | ORB Template Path  |
+====================+====================+====================+====================+
| ``C``              | Standalone         | ``dcpssac.so``     | ``SAC``            |
|                    |                    |                    |                    |
|                    |                    | ``dcpsac.lib``     |                    |
+--------------------+--------------------+--------------------+--------------------+
| ``C++``            | ORB Integrated     | ``dcpsccpp.so``    | ``CCPP/DDS_Open``\ |
|                    |                    |                    | ``Fusion_1_4_1``   |
|                    |                    |                    |                    |
|                    |                    |                    | *for UNIX-like*    |
|                    |                    |                    | *platforms, and*   |
|                    |                    |                    |                    |
|                    |                    |                    | ``CCPP\DDS_Open``\ |
|                    |                    |                    | ``Fusion_1_5_1``   |
|                    |                    |                    |                    |
|                    |                    |                    | *for the Windows*  |
|                    |                    |                    | *platform*         |
+--------------------+--------------------+--------------------+--------------------+
| ``C++``            | Standalone         | ``dcpssacpp.so``   | ``SACPP``          |
+--------------------+--------------------+--------------------+--------------------+
| ``ISOC++``         | ISOCPP Types       | ``dcpsisocpp.so``  | ``ISOCPP``         |
+--------------------+--------------------+--------------------+--------------------+
| ``ISOC++``         | ORB Integrated     | ``dcpsisocpp.so``  | ``CCPP/DDS_Open``\ |
|                    |                    |                    | ``Fusion_1_4_1``   |
|                    |                    |                    |                    |
|                    |                    |                    | *for UNIX-like*    |
|                    |                    |                    | *platforms, and*   |
|                    |                    |                    |                    |
|                    |                    |                    | ``CCPP\DDS_Open``\ |
|                    |                    |                    | ``Fusion_1_5_1``   |
|                    |                    |                    |                    |
|                    |                    |                    | *for the Windows*  |
|                    |                    |                    | *platform*         |
+--------------------+--------------------+--------------------+--------------------+
| ``ISOC++2``        | ISOCPP Types       | ``dcpsisocpp2.so`` | ``ISOCPP2``        |
+--------------------+--------------------+--------------------+--------------------+
| ``Java``           | Standalone         | ``dcpssaj.jar``    | ``SAJ``            |
+--------------------+--------------------+--------------------+--------------------+
| ``Java``           | ORB integrated     | ``dcpscj.jar``     | ``SAJ``            |
+--------------------+--------------------+--------------------+--------------------+
| ``C#``             | Standalone         | ``dcpssacs``       | ``SACS``           |
|                    |                    | ``Assembly.dll``   |                    |
+--------------------+--------------------+--------------------+--------------------+
| ``C99``            | Standalone         | ``dcpsc99.so``     | ``C99``            |
|                    |                    |                    |                    |
|                    |                    | ``dcpsc99.lib``    |                    |
+--------------------+--------------------+--------------------+--------------------+


The mappings for each language are in accordance with their respective
OMG Language Mapping Specifications (see :ref:`the Bibliography <References>` 
for a list of references).



.. EoF

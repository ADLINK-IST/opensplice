.. _`Preface`:

#######
Preface
#######

***********************************
About the RMI User Guide
***********************************

The |product_name| *RMI User Guide* is intended to
explain the steps required to take advantage of the client/server
interaction paradigm provided by |product_name| RMI layered over the
publish/subscribe paradigm of |product_name|.

**Intended Audience**

This |product_name| *RMI User Guide* is for
developers using remote invocations in DDS applications.

**Organisation**

The first two chapters give a general introduction to RMI over DDS.

:ref:`Building an RMI Application` describes the steps involved in
building applications using RMI over DDS.

:ref:`Language mapping for |product_name| RMI` gives the |rmi_langs|
mapping of the IDL types that can be declared in the RMI services
description file.

:ref:`RMI Interface to DDS topics mapping rules` shows how IDL
declarations of RMI interfaces are mapped into IDL declarations of the
implied DDS topics.

:ref:`RMI Runtime Configuration Options` describes the command-line
options available when starting the *RMI* runtime.

:ref:`QoS policies XML schema` contains the XML schema for reference.


***********
Conventions
***********

The icons shown below are used to help readers
to quickly identify information relevant to their
specific use of Vortex.

 ========= ================================================================== 
 *Icon*    *Meaning*  
 ========= ================================================================== 
 |caution| Item of special significance or where caution needs to be taken.  
 |info|    Item contains helpful hint or special information.                
 |windows| Information applies to Windows (*e.g.* XP, 2003, Windows 7) only. 
 |unix|    Information applies to Unix-based systems (*e.g.* Solaris) only.  
 |linux|   Information applies to Linux-based systems (*e.g.* Ubuntu) only.  
 |c|       C language specific.                                              
 |cpp|     C++ language specific.                                            
 |csharp|  C# language specific.                                             
 |java|    Java language specific.                                           
 ========= ================================================================== 




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

         

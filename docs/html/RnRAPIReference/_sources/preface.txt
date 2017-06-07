.. _`Preface`:

#######
Preface
#######


About The Record and Replay API Reference
******************************************

The *Record and Replay (RnR) API Reference* provides a complete description of the functions 
available *via* the API of the OpenSplice Record and Replay Service 
(RnR Service).

This API Reference is intended to be used after the Vortex OpenSplice software
including the RnR Service has been installed on the network and configured
according to the instructions in the Vortex OpenSplice *Getting Started Guide*.

Intended Audience
*****************

The API Reference is intended to be used by all Record and Replay 
Service users, including programmers, testers, system designers and 
system integrators.


Organisation
************

The :ref:`Introduction <Introduction>` gives an overview of the purpose 
and design of the Record and Replay Service.

The :ref:`Scenarios <Scenarios>` section explains the key concept of the 
‘scenario’ used in the RnR Service.

:ref:`All of the functions of the RnR Service <Topic API Overview>` 
are then described in full detail.

There is a (very short) list of :ref:`known issues <Known Issues>` which 
describes current limitations of the RnR Service.

The next section describes how the RnR Service is designed to 
minimize its :ref:`intrusiveness <Impact on DDS Domain>` on 
existing systems.

The :ref:`RnR Topic API IDL specification <Appendix A>` contains 
the complete IDL definition of the RnR Service API.

Finally, there is a :ref:`bibliography <References>` which lists
all of the publications referred to in this *Guide*. 


Conventions
***********

The icons shown below are used in PrismTech product documentation 
to help readers to quickly identify information relevant to their 
specific use of Vortex OpenSplice.


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

         
.. _`Compiling and Running`:


#####################
Compiling and Running
#####################

*This section describes how Vortex OpenSplice Modeler 
and Eclipse Workbench are used to compile and run
applications.* 


Compiling
*********


The Eclipse Workbench can be used to compile the native code by 
either

+ manually invoking the Eclipse Builder by choosing *Project > 
  Build Project, Project > Build All* from the *Menu Bar*

OR

+ enabling the *Project > Build Automatically* option (a tick mark 
  is displayed)


Running
*******


Java
====

|java|

Right-click on the Java source and choose *Run As > Java 
Application* to run the application (a Vortex OpenSplice daemon is 
expected to be running).

|caution|
The Java application must contain a ``main()`` method for this 
option to be available.

This facility requires the environment to be configured for Vortex 
OpenSplice. If not, Vortex OpenSplice Modeler comes with a handy Java 
run configuration preset targeting Vortex OpenSplice deployments, 
according to the Vortex OpenSplice preferences 
(see Section on :ref:`Setting Vortex OpenSplice Preferences <Setting 
Vortex OpenSplice Preferences>`). To make use of it, choose *Run As 
> OpenSplice Java Application*.

If you need to have additional configuration settings for your 
run, simply create a Java run configuration for the specific 
Java source file containing the ``main()`` method, then edit the 
variables from the *Environment* tab (right-click on the source 
file, choose *Run As > Run*, then create new run configuration).

C++
===

|cpp|

Right-click on the C++ source and choose *Run As > Local C++ 
Application* to run the application against the selected target 
DDS platform. Note that if the target is Vortex OpenSplice, the 
Vortex OpenSplice daemon needs to be running.

Additionally, for Vortex OpenSplice targets, this facility requires 
that the environment be configured. 

Otherwise, there is a handy C++ run configuration preset available 
under *Run As > Vortex OpenSplice C++ Application*, which utilizes the 
values set in the Vortex OpenSplice preferences 
(see Section on :ref:`Setting Vortex OpenSplice Preferences <Setting 
Vortex OpenSplice Preferences>`). 

You can also create a C++ run configuration for the 
specific C++ executable, then edit the variables from the 
*Environment* tab (right-click on the source file, choose 
*Run As > Run*, then create new run configuration).

To run a C++ application targeting Vortex Lite, you can choose 
*Run As > Local C++ Application*. However, you will need to 
manually configure the required environment variables. 

There is also a handy C++ run configuration preset available under 
*Run As > Vortex Lite C++ Application*. Otherwise, You can also create 
a C++ run configuration for the specific C++ executable, then edit 
the variables from the *Environment* tab (right-click on the 
source file, choose *Run As > Run*, then create new run 
configuration).      



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

         
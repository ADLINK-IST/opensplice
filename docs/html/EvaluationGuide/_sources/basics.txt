.. _`OpenSplice Enterprise Basics`:

############################
OpenSplice Enterprise Basics
############################


OpenSplice Enterprise is configured using an XML configuration file. 
In this file, the user specifies the architectural model and the
OpenSplice Enterprise services that are to run when the DDS 
infrastructure is started. 

The ``OSPL_URI`` environment variable refers to the specific XML 
configuration file that is used for the current deployment. The 
default value refers to the ``ospl.xml`` file located in the ``etc/config`` 
directory of the OpenSplice Enterprise installation. The installation 
directory itself can be referred to by the ``OSPL_HOME`` environment 
variable. Please see :ref:`The OpenSplice Enterprise Environment` for 
details of how to set up the OpenSplice Enterprise environment. 

A number of other sample configuration files that can be used when 
benchmarking OpenSplice Enterprise are also provided in 
the ``etc/config`` directory. 

The ``OSPL_URI`` variable is of the form: 

|linux|
   ``OSPL_URI=file://$OSPL_HOME/etc/config/ospl.xml`` 

|windows|
   ``OSPL_URI=file://%OSPL_HOME%\etc\config\ospl.xml`` 
 
You can refer to the ``OpenSplice_DeploymentGuide.pdf`` later for more details 
of the ``OSPL_URI`` variable; for now, let us see what aspects of the 
OpenSplice deployment are controlled by this file. 

|info|
  The OpenSplice Enterprise Launcher tool assists with the 
  selection of the ``OSPL_URI`` variable. There is a *Configurations* menu 
  that lists the sample configuration files that are available. 

.. _`The Launcher tool`:

.. figure:: /images/LauncherTools.png
   :height: 70mm
   :alt: The Launcher tool

   **The Launcher tool**


The OpenSplice Enterprise Launcher tool is also able to run the 
examples and performance tests that are described later in this 
document. 


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


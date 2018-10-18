. _`How to run Vortex OpenSplice`:

################################
How to run Vortex OpenSplice
################################


.. _`The Vortex OpenSplice Environment`:

*************************************
The Vortex OpenSplice Environment
*************************************

A release file is provided with the Vortex OpenSplice 
installation which contains the environment variables that 
are required. 

Create an Vortex OpenSplice environment as follows.

**First:**

|linux|

  Open a shell and source the ``release.com`` file from 
  the Vortex OpenSplice installation directory. 


|windows|

  Open a *Windows Command prompt* and run the ``release.bat``
  file in the Vortex OpenSplice installation directory. 

|windows|

  Alternatively, use the *Vortex OpenSplice Command Prompt*
  that can be accessed from the Windows *Start* menu 
  (this will implicitly run ``release.bat``). 

|info|
  Note that the Vortex OpenSplice Launcher tool also provides a 
  ``Console`` option which sets up the Vortex OpenSplice environment. 

**Next:**

Set the ``OSPL_URI`` variable to refer to the Vortex OpenSplice 
configuration that is required (see the section 
:ref:`How to select the Architectural Mode`).

**********************************************
Running Single Process and Shared Memory Modes
**********************************************

+ With an ``OSPL_URI`` variable referring to a *Single Process* deployment, 
  you just need to start the DDS application process. The 
  ``create_participant()`` operation, which is the entry into the DDS 
  Domain, will create the entire DDS infrastructure within the 
  application process and the services will be started as threads. 


+ With an ``OSPL_URI`` variable referring to a *Shared Memory* deployment, it 
  is necessary to start the DDS infrastructure before starting your DDS 
  application processes. That is done by using the ``ospl`` utility tool:

.. code-block:: bash

   ospl start
   # now run the DDS application processes as normal
   ospl stop



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


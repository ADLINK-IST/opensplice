.. _`Launcher`:

########
Launcher
########

*The Launcher application provides easy access to all of the tools,
documentation, configurations, and examples bundled with Vortex OpenSplice.*


********************
The Launcher utility
********************


.. figure:: /images/Launcher-utility.png
   :height: 70mm
   :alt: The Launcher utility


   **The Launcher utility**


*********************
Starting the Launcher
*********************

The Launcher's content is populated based on the environment variables
available at the time it is started. If the command line is used then
the correct release script must be sourced from the Vortex OpenSplice home
directory before starting the Launcher.


**Unix and Linux**

|unix| |linux|

Source the ``release.com`` file from the shell command line to set up the
Vortex OpenSplice environment.


To start Launcher, enter the following command in the shell:

.. code-block:: bash

  ospllauncher


**Windows**

|windows|

Open an *Vortex OpenSplice Command Prompt* and enter the following command:

.. code-block:: bash

  ospllauncher.bat

You can also start Launcher from the Windows *Start* Menu, which will
set the environment variables of the Vortex OpenSplice instance
appropriately.

.. _`Starting the Launcher from Windows Start menu`:

.. figure:: /images/Launcher-starting.png
   :alt: Starting the Launcher from Windows Start menu

   **Starting the Launcher from Windows Start menu**



*********************
Stopping the Launcher
*********************

**Unix and Linux**

|unix| |linux|

In the terminal where the ``ospllauncher`` file was executed,
enter *CTRL+C*, or close the Launcher application windows using
the *X* button.

**Windows**

|windows|

Close the Launcher application windows using the *X* button.


********************************
Troubleshooting Improper Startup
********************************

**Launcher starts up with incorrect, missing, or no content at all**

Check the following:

|linux|
  On Linux, ensure that the *release.com* was called prior to starting
  the Launcher.

|windows|
  On Windows, ensure you use the *Vortex OpenSplice Command Prompt* , or you
  can use the Windows command prompt if you call the *release.bat* script
  prior to starting the Launcher.

If previous versions of Vortex OpenSplice were used with
Launcher, verify that ``USER_HOME.olauncherprefs`` does not contain any
overridden values pointing to non-existent directories or files.

**Launcher starts up but cannot compile or clean example code**

Ensure that you have permissions to modify the permissions of
``OSPL_HOME/examples`` and any user-specified configuration directories,
or start Launcher with elevated privileges.

**Launcher starts up but cannot save edited configuration files opened
in Configurator that has been started from Launcher**

Ensure taht you have modify permissions in the ``OSPL_HOME/etc/config/``
directory, or start Launcher with elevated privileges.

**Launcher starts up but cannot write log files**

Ensure that you have modify permissions in the ``OSPL_HOME/`` and its
subdirectories, start Launcher with elevated privileges, or start
``ospllauncher`` from a non-write-protected directory.





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


.. _`Getting Started`:


###############
Getting Started
###############

*This section describes how to use the Tester’s main features.*

.. _`Starting and Stopping Tester`:

Starting and Stopping Tester
****************************

Tester may be started by running the OpenSplice Tester application or from a
command prompt and oslptest with the following command line arguments:

**-?** or **-help**
   Display the command line options

**-ns**
   No splash screen

**-uri <uri>**
   URI to connect

**-nac**
   No auto-connect upon application start

**-s <path to script>**
   Script to run

**-b <path to batch file>**
   Batch script to run

**-noplugins**
   Do not process plugins

**-plugindir <dir>**
   Extra plugin directory to search

**-headless**
   Run script or batch script without the GUI

**-rc <port>**
   Enable remote control (via <port>)

**-dsl <language>**
   Default script language

**-l <path to topic file>**
   Load topics from file


(These preferences can also be set from the menu *File > Preferences*.)

Starting - Local Connection
***************************

|windows|
On Windows, use either of the shortcuts created by the installer (on the desktop and  
in *Start > Programs*) to start Tester.

|linux|
On Linux go to the installation directory and execute the command:

::
   
   %  ospltest

This will start Tester with separate windows.

.. _`Starting - Remote Connection`:

Starting - Remote Connection
****************************

To connect to a remote platform, execute the command:

::
   
   %  ospltest -uri http://perf1.perfnet.ptnl:50000


(Port number 50000 is the default port in a standard DDS shared-memory 
deployment.)

Stopping
********

Stop Tester either by using the menu option *File > Exit* or by clicking 
on the main window *`close'* button |close|.

Remotely Controlling Tester
***************************

Starting Tester in remote control mode *e.g.* ``"ospltest -rc <port> 
-headless"`` allows Tester to be controlled from another application, 
shell script, *etc.*.

Use cases for remote control include:

+  Using Tester in combination with a commercial or proprietary test system;

+  Within a continuous build and test environment this would provide more 
   options to control DDS testing in combination with other application-specific 
   testing;

+  In an integrated development environment like Eclipse using Junit for testing.

A Tester instance is controlled *via* a TCP/IP connection. Text-based commands 
are sent over this connection.

The remote control application can be used by executing the command:

::
   
   ospltestrc [-p <port>] [-h <host>] <command>

where

   ``<host>`` is the host name of the machine that the Tester you wish 
   to control is running on

   ``<port>`` is the port that Tester is listening on (specified by 
   the ``-rc`` option when Tester was started)

   ``<command>`` is the command to send to Tester.

The remote control commands are:

**stop**
   Terminate the Tester instance

**batch <batch name>**
   Execute the batch with ``<batch name>``

**script <script name>**
   Execute the script with ``<script name>``

**scenario <scenario>**
   Execute a scenario which is provided in full text 
   on a single line (new lines ``"\n"`` are replaced by 
   ``"&nbsp"``)

**connect <optional uri>**
   Connect to a specified or the default Domain uri

**disconnect**
   

When a command is completed the following is reported on a single line:

  ``Done``

When a batch is executed, for each scenario two lines are returned to the test 
controller:

  ``Scenario: <index> of <count> execute: <scenario name>``
  ``Scenario: <index> result: <result>``

Trying out Tester
*****************

Once you have started Tester, you can get a feeling for how to use it with a few 
simple exercises:

+  Create a default reader for some of the registered topics.

+  Double-click one of the samples and see all the fields of the topic.

+  Browse through the list of samples using the arrow keys.

+  Select a topic in the sample list and press *[F9]*, then select a field for 
   display in the sample list.

+  Select another topic (it need not be of the same type as the one displayed 
   in the topic instance window) and press *[F2]* for a comparison between the 
   two topics.

+  Select a topic in the sample list or in the topic list and press *[F4]*, 
   then in the ``Write`` topic window set the fields to the desired value 
   and write or dispose the topic.

+  Choose *File > Dump* on the sample list and save the information of the 
   topic samples in the sample list to a file.

+  Have a try with the scripting, it can make your life a lot easier 
   (especially with recurring tasks).

The rest of this section describes the features that you will use when 
you try these exercises.


Tester Windows
**************

Main Window
===========

Once started, Tester presents the user with the following main window.

.. _`Tester main window`:

.. centered:: **Tester main window**

.. image:: /images/001_mainwindow.*
   :width: 145mm
   :align: center
   :alt: Tester main window

The Command Menu (below) provides direct access to most of the Tester 
capabilities.

.. _`Tester command menu`:

.. centered:: **Tester command menu**

.. image:: /images/002_commandmenu.*
   :width: 145mm
   :align: center
   :alt: Tester command menu

The Tester main window has three sub-frames:

1. *Main* tabbed frame for selecting items from a list, such as topics, 
   scenarios, and readers or writers.

2. *Working area* frame where you will do most of your work such as 
   editing scenarios, investigate samples, and capturing statistics

3. *Debug* frame used to debug scripts and macros.

Overview Windows
================

The user can select the type of resource to work with by selecting tabs. These can be 
the Services and Topics in the system, the Scripts and Macros they have installed, or 
the Readers for the current Tester timeline.

.. _`Tester resource tabs`:

.. centered:: **Tester resource tabs**

.. image:: /images/003_tabs.*
   :width: 90mm
   :align: center
   :alt: Tester resource tabs


Services
--------

Lists the installed services. This is a read-only list.

.. _`Tester Services list`:

.. centered:: **Tester Services list**

.. image:: /images/004_tab_services.*
   :width: 90mm
   :align: center
   :alt: Tester Services list


Scripts
-------

The script list provides a convenient way of selecting an existing script for editing 
or execution. The list is filled at startup or when clicking the *Refresh* button. All 
files in the specified script directory are added to the list. The script directory (or 
directories) are specified in the preference page.

A script can be selected in the script editor by single-clicking the entry in the table. 
When the entry is double-clicked the script is loaded in the script editor and 
executed.

.. _`Tester scripts tab`:

.. centered:: **Tester scripts tab**

.. image:: /images/005_tab_scripts.*
   :width: 90mm
   :align: center
   :alt: Tester scripts tab


Macros
------

The Macros List is similar to the Scripts List.

.. _`Tester macros tab`:

.. centered:: **Tester macros tab**

.. image:: /images/006_tab_macros.*
   :width: 90mm
   :align: center
   :alt: Tester macros tab


Topics
------

The topics list displays the list of registered topics. 

.. _`Tester topics tab`:

.. centered:: **Tester topics tab**

.. image:: /images/007_tab_topics.*
   :width: 90mm
   :align: center
   :alt: Tester topics tab


Readers
-------

The readers list displays the readers (and implicit topic writers) for the current 
Tester timeline. The default name for a reader is the same as the name of the topic it 
is subscribed to. For each reader the count of received samples (as available in the 
sample list) is displayed. A check box is provided for changing the read state or the 
show state. When *Read* is unchecked the reader stops reading topics. When *Show* is 
unchecked the topic of that topic will not be displayed in the sample list.

.. _`Tester readers tab`:

.. centered:: **Tester readers tab**

.. image:: /images/008_tab_readers.*
   :width: 90mm
   :align: center
   :alt: Tester readers tab


Working Windows
===============

These windows support testing activities.

Sample List Window
------------------
Used to view and generate samples for the current timeline (Readers).

.. _`Sample list window`:

.. centered:: **Sample list window**

.. image:: /images/009_samplelist.*
   :width: 145mm
   :align: center
   :alt: Sample list window


Statistics Window
-----------------

The statistics window provides statistics for the topics in use, 
like write count, number of alive topics, *etc.*.  Statistics 
are gathered from the local copy of OpenSplice. To gather statistics 
from remote nodes, use OpenSplice Tuner.

.. _`Statistics window`:

.. centered:: **Statistics window**

.. image:: /images/010_statistics.*
   :width: 145mm
   :align: center
   :alt: Statistics window


Browser Window
--------------

The browser window provides information about nodes, executables, 
participants (applications), readers, writers and topics. Information 
can be browsed by selecting a node/executable participant or a topic. 
When an executable or participant is selected the reader and writer 
lists (subscribed and published topics) for that executable/participant 
are shown. Together with the topic name concise information about the 
QoS and partition is shown. When the mouse cursor is hovered over the 
QoS value the hint will show detailed information about the QoS.

When a topic is selected the list of participant readers (subscribe) 
and writers (publish) are shown, together with concise information 
about the QoS and partition. By selecting a row in either the reader 
of writer list the compatible readers/writers will be shown in green 
and non-compatible (by QoS/partition) readers/writers will be shown 
in red.

.. _`Tester browser window`:

.. centered:: **Tester browser window**

.. image:: /images/011_browser.*
   :width: 145mm
   :align: center
   :alt: Tester browser window


Scripting Windows
=================

Edit Window
-----------

The script window is used for editing scripts. The editor supports 
syntax highlighting, auto-completion, and more.

.. _`Script editing window`:

.. centered:: **Script editing window**

.. image:: /images/012_editscript.*
   :width: 145mm
   :align: center
   :alt: Script editing window


Debug Window
------------

The debug window displays compile and execution results. 
Details can be filtered. Positive results are highlighted with 
green, negative results are highlighted with red.

.. _`Debug window`:

.. centered:: **Debug window**

.. image:: /images/013_debug.*
   :width: 145mm
   :align: center
   :alt: Debug window


Other Windows
=============

The following dialog windows will be used.

Add Reader Window
-----------------

Used to create/define a new Reader.

The dialog provides a drop-down list of existing partitions to 
choose to create the new Reader in.

.. _`Add Reader dialog`:

.. centered:: **Add Reader dialog**

.. image:: /images/014_AddReader.*
   :width: 90mm
   :align: center
   :alt: Add Reader dialog


Batch Window
------------

Used to Start a batch scenario and display the test results.

.. _`Batch Execute Scenarios window`:

.. centered:: **Batch Execute Scenarios window**

.. image:: /images/015_batch.*
   :width: 110mm
   :align: center
   :alt: Batch Execute Scenarios window


Batch Results Window
--------------------
Displays the detailed results of a batch of scripts. 
Detailed individual test result can be viewed by double-clicking 
on a test result. 

.. _`Batch results window`:

.. centered:: **Batch results window**

.. image:: /images/016_results.*
   :width: 110mm
   :align: center
   :alt: Batch results window




.. _`Detailed Batch results log`:

.. centered:: **Detailed Batch results log**

.. image:: /images/017_results.*
   :width: 110mm
   :align: center
   :alt: Detailed Batch results log


Chart Window
------------

Used to plot topic field values.

.. _`Topic field values graph`:

.. centered:: **Topic field values graph**

.. image:: /images/018_graph.*
   :width: 110mm
   :align: center
   :alt: Topic field values graph


Edit Sample Window
------------------

Used to create samples for a selected topic.

.. _`Edit sample window`:

.. centered:: **Edit sample window**

.. image:: /images/019_edit.*
   :width: 110mm
   :align: center
   :alt: Edit sample window


.. _`Topic Instance Window`:

Topic Instance Window
---------------------

The topic sample window is used for displaying field values of a topic. 
It can be opened by double-clicking a sample in the sample list or by 
pressing *[F3]* (additional) or *[F2]* (additional with compare) in 
the sample list while a sample is selected. Special fields are 
highlighted with colors:


   *Key field* (Green)

   *Foreign key* (Yellow)

   *Different (compare only)* (Red)

   *Not existing (compare only)* (Orange)


When a field is selected, *[Ctrl+H]* will toggle between normal and hexadecimal 
representation, and *[Ctrl+D]* will toggle between normal and degrees/radians 
representation.




.. |close| image:: ./images/138_icon_close.*
            :height: 3mm
.. |play| image:: ./images/134_icon_play.*
            :height: 3mm
.. |pause| image:: ./images/135_icon_pause.*
            :height: 3mm
.. |stop| image:: ./images/136_icon_stop.*
            :height: 3mm

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

         
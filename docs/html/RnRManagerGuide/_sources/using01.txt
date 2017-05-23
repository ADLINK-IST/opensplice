.. _`Using the Record and Replay Manager`:


###################################
Using the Record and Replay Manager
###################################

*RnR Manager does not need to be installed on a system with Vortex OpenSplice;
it just needs to connect to a computer running Vortex OpenSplice with 
RnR Services configured.*

RnR Manager Overview
********************

Starting RnR Manager
====================

RnR Manager is started by executing the ``startRnRManager`` script, located in the 
eclipse directory of the RnR Manager install directory. The script's purpose is to ensure 
the proper setting of the Vortex OpenSplice environment variables prior to application 
launch.

The script allows for command line options to be passed to the application. Any command 
line options accepted by an Eclipse application are also accepted by RnR Manager. A 
particularly useful option is ``-data <directory>``, which allows a user to specify a 
custom directory use as the RnR Manager workspace, in case the default directory is shared 
or write protected.

Example of starting RnR Manager with a non default workspace location:

::

   <path-to-RnRManager>/startRnRManager.sh -data /home/user/rnrworkspace

::

   <path-to-RnRManager>\startRnRManager.bat -data C:\Users\user\rnrworkspace

|info|
  Path arguments to the start script must be absolute paths.

Views
=====

The RnR Manager supports various views to monitor the state of a 
connected domain. It also provides two editors.

These views and editors provide users with the ability to manage OSPL 
record and replay functionality. Additionally, record and replay data 
can be queried, and edited.

The views are:

+  Domain Browser
+  Service Status 
+  Storage Status
+  Scenario Status
+  Imported Storages

The editors are:

+  Scenario Editor
+  Storage Editor

Each view and editor has a detailed section to describe its features and 
intended usage.

.. _`RnR Manager Overview`:

.. centered:: **RnR Manager Overview**

.. image:: /images/004_RnRM_overview.png
   :width: 140mm
   :align: center
   :alt: RnR Manager Overview


Preferences
===========

RnR Manager preferences can be set using the main Window/Preferences 
menu.

OpenSplice
----------

This preference page enables you to set the ``OSPL_HOME`` path for the 
RnR Manager if one is not set for the environment.

|caution|
  RnR Manager must be restarted for this preference to come into effect.

.. _`OpenSplice Preferences page`:

.. centered:: **OpenSplice Preferences page**

.. image:: /images/005_OpenSplice_prefs.png
   :width: 140mm
   :align: center
   :alt: OpenSplice Preferences page


Partition/Topic Filters
-----------------------

The *Partition/Topic Filters* page enables you to specify which partitions 
and topics you would like to filter out in the general UI display. For 
example, any topics that are filtered out would not be seen in the in 
dropdowns used to list topics or in the Topics view.

.. _`Partition/Topic Filters Preferences page`:

.. centered:: **Partition/Topic Filters Preferences page**

.. image:: /images/006_filters_prefs.png
   :width: 140mm
   :align: center
   :alt: Partition/Topic Filters Preferences page


Partition/Topic Strings
-----------------------

The *Partition/Topic Strings* page enables you to specify partition and 
topic strings that are displayed in the general UI. For example, any 
topic strings that are added here are displayed in drop-down topic lists.

.. _`Partition/Topic Strings`:

.. centered:: **Partition/Topic Strings**

.. image:: /images/007_PartitionTopic_strings.png
   :width: 120mm
   :align: center
   :alt: Partition/Topic Strings


Help
====

Various help actions are available from the main *Help* menu. These 
include actions for managing licenses, checking for updates, and product 
*About...* information.

.. _`Help menu`:

.. centered:: **Help menu**

.. image:: /images/008_HelpMenu.png
   :width: 100mm
   :align: center
   :alt: Help menu


Domain Browser
**************

The Domain Browser is used to define and connect to a domain. It also 
provides information about domain nodes, and the services the nodes 
support.

.. _`Domain Browser`:

.. centered:: **Domain Browser**

.. image:: /images/009_DomainBrowser.png
   :width: 100mm
   :align: center
   :alt: Domain Browser


Defining a domain
=================

To define an OSPL domain, click on the button in the top right corner of 
the Domain Browser.

.. _`Add Domain button`:

.. centered:: **Add Domain button**

.. image:: /images/010_AddDomain_button.png
   :width: 18mm
   :align: center
   :alt: Add Domain button


The RnR Wizard dialog will open. Multiple domain connection options are 
provided:

+  A *local* domain can be connected to using an xml file or by 
   specifying a domain id.
+  A *remote* domain can be connected to using the node URL.


.. _`RnR Domain Wizard`:

.. centered:: **RnR Domain Wizard**

.. image:: /images/011_Domain_Wizard.png
   :width: 140mm
   :align: center
   :alt: RnR Domain Wizard


Domain Connect
==============

Once a domain has been defined, an entry will appear in the Domain 
Browser.

To connect, double-click on the entry, or right-click to display the 
domain menu and choose *Connect*.

.. _`Connect to Domain`:

.. centered:: **Connect to Domain**

.. image:: /images/012_ConnectToDomain.png
   :width: 80mm
   :align: center
   :alt: Connect to Domain

When *Connect* is clicked, an attempt is made to connect to the RnR 
Services of the selected domain. If successful, the domain icon will 
change to |connectedbutton| and you will be able to expand the domain 
to browse the nodes.

RnR Manager needs to be connected to the RnR Services of a domain in 
order to manage the recording and/or replaying of sample data.

|caution|
  RnR Manager can only connect to one domain at a time.


Domain Disconnect and Delete
============================

To disconnect or delete a domain, right-click on the domain to display 
the *Domain* menu.

Service Status
**************

The service status view lists the current state of the OSPL RnR services 
available to the currently connected domain.

The list can be sorted by *Key* or *State* by clicking on the column header.

.. _`Service Status view`:

.. centered:: **Service Status view**

.. image:: /images/014_ServiceStatus.png
   :width: 80mm
   :align: center
   :alt: Service Status view


Storage Status
**************

The storage status view lists the current state of the OSPL RnR storages 
available to the currently-connected domain.

The list can be sorted by *Key* or *State* by clicking on the column header.

.. _`Storage Status view`:

.. centered:: **Storage Status view**

.. image:: /images/015_StorageStatus.png
   :width: 80mm
   :align: center
   :alt: Storage Status view


Creating a new storage
======================

To create a new storage, click on the Add new storage button in the top 
right corner of the view to open the *Create Storage* dialog.

.. _`Add new storage button`:

.. centered:: **Add new storage button**

.. image:: /images/016_AddNewStorage_button.png
   :width: 18mm
   :align: center
   :alt: Add new storage button



.. _`Create Storage dialog`:

.. centered:: **Create Storage dialog**

.. image:: /images/017_CreateStorage.png
   :width: 100mm
   :align: center
   :alt: Create Storage dialog


A wizard runs, with the input fields required for creating and configuring 
a new RnR storage.

The *RnR Service ID* field specifies for which service the storage will 
be created. 

The *Storage name* field specifies what name the RnR service 
will recognize the storage by.

(See also `Storage Attributes`_ below.)

Clicking on *Finish* will create the storage. The *Storage status* view 
should be updated with a new entry for the newly-created storage.

Storage Attributes
------------------

When creating new storages or reconfiguring existing storages, such as 
when using the *Create Storage* wizard, the following attributes can be set:

*File name* - 
  The file name and path on the RnR service's local filesystem in which 
  it will keep the storage data. Relative paths are relative to the 
  RnR service's process working directory. File names have no rules 
  or restrictions for file extensions; file names can have any extension, 
  or no extension if required.

*Storage type* - 
  The encoding format for the storage. The file formats available are 
  ``CDR`` (binary) and ``XML``. 
  The ``CDR`` format is more efficient (compact) than ``XML``.

*Max file size* - 
  The maximum size *per* storage file. When approaching the maximum size 
  while recording, a new storage file is automatically created with a 
  sequence number appended to the filename. The active file is also 
  switched transparently while replaying from a storage that contains 
  multiple data files (*eg.* if file name is ``"file"``, the name sequence 
  is ``file``, ``file0``, ``file1``, *etc.*). 

  Valid *max size* is any positive integer in denominations of bytes, 
  kilobytes, megabytes, or gigabytes, up to a maximum value 
  of ``2^64 - 1`` total bytes. Leaving the field empty, or inputting 
  the value ``0``, is considered to be unlimited file size.


Scenario Status
***************

Record and Replay is based on *Scenarios*. Scenarios are used to logically 
discriminate between different recording and replaying command sequences. 
A scenario is an instance of the scenario topic, a group of commands 
sharing the same ``scenarioName``.

The *Scenario status* view lists the current state of the OSPL RnR 
scenarios available to the currently-connected domain.

The list can be sorted by *Key* or *State* by clicking on the column header.

.. _`Scenario Status view`:

.. centered:: **Scenario Status view**

.. image:: /images/018_ScenarioStatus.png
   :width: 80mm
   :align: center
   :alt: Scenario Status view



Open Scenario Editor
====================

To open an empty scenario editor, click on the *Create scenario* button in 
top right corner of the scenario status view.

Please refer to `Scenario Editors`_ for more information on the 
Scenario Editor.

.. _`Create scenario button`:

.. centered:: **Create scenario button**

.. image:: /images/019_CreateScenario_button.png
   :width: 18mm
   :align: center
   :alt: Create scenario button


Starting Scenario(s)
====================

To start a scenario or set of scenarios, click on the *Start scenario(s)* 
button to open the *Start Scenario(s)* dialog.

.. _`Start scenario button`:

.. centered:: **Start scenario button**

.. image:: /images/020_ScenarioStart_button.png
   :width: 18mm
   :align: center
   :alt: Start scenario button

The wildcard character ``‘*’`` (asterisk) is supported. 

.. _`Start Scenario(s) dialog`:

.. centered:: **Start Scenario(s) dialog**

.. image:: /images/022_StartScenarios.png
   :width: 80mm
   :align: center
   :alt: Start Scenario(s) dialog


A ``start scenario`` command is executed using the parameters specified in 
the dialog.

Stopping Scenario(s)
====================

To stop a scenario or set of scenarios, click on the *Stop scenario(s)* 
button to open the *Stop Scenario(s)* dialog.

.. _`Stop scenario button`:

.. centered:: **Stop scenario button**

.. image:: /images/021_ScenarioStop_button.png
   :width: 18mm
   :align: center
   :alt: Stop scenario button


The wildcard character ``‘*’`` (asterisk) is supported. 

.. _`Stop Scenario(s) dialog`:

.. centered:: **Stop Scenario(s) dialog**

.. image:: /images/023_StopScenarios.png
   :width: 80mm
   :align: center
   :alt: Stop Scenario(s) dialog


A ``stop scenario`` command is executed using the parameters specified in 
the dialog.

Scenario Menu
=============

Actions are available for a scenario with a specific scenario name 
and ``rnrID``.

To access the scenario menu, right-click on a scenario name in the 
scenario status table.

.. _`Scenario Menu`:

.. centered:: **Scenario Menu**

.. image:: /images/024_ScenariosMenu.png
   :width: 80mm
   :align: center
   :alt: Scenario Menu


The scenario menu has the following options:

*Open* – 
  Opens a scenario editor on the selected scenario and RnR 
  service. Please refer to `Scenario Editors`_ for more 
  information on the Scenario Editor.

*Start* – 
  Start the selected scenario. This menu item is disabled if the 
  scenario is already in the ``RUNNING`` state.

*Stop* – 
  Stop the selected scenario. This menu item is disabled if the 
  scenario is not in the ``RUNNING`` state.


Topics – One Click Record
*************************

The *Topics* view displays all of the DDS system topics that are available 
for recording. This view enables the user to start recording quickly 
using the *Record Topic(s)* dialog. Default parameters for the recording 
are provided for the selected topics.

To verify the status of the recording, the storage and scenario views 
can be viewed.  The scenario editor can also be opened for the scenario.

.. _`The Topics list`:

.. centered:: **The Topics list**

.. image:: /images/025_TopicsList.png
   :width: 70mm
   :align: center
   :alt: The Topics list


To Record a Topic
=================

**Step 1**

  Select one or more topics to record in the topic list.

**Step 2**

  Right-click and choose *Record... to* open the *Record Topic(s)* 
  dialog.


.. _`Topics selected for recording`:

.. centered:: **Topics selected for recording**

.. image:: /images/026_TopicsSelected.png
   :width: 70mm
   :align: center
   :alt: Topics selected for recording



**Step 3**

  Modify the default parameters if required and click on *OK*.


.. _`Record Topic(s) dialog parameters`:

.. centered:: **Record Topic(s) dialog parameters**

.. image:: /images/027_TopicsRecordParameters.png
   :width: 140mm
   :align: center
   :alt: Record Topic(s) dialog parameters


The Record Topic(s) Dialog
==========================

Default parameters are provided in the dialog in order to provide ‘one 
click’ functionality. These default parameters can be edited if 
required.

**Partition/Topic pairs**

The partition/topic pairs listed in the dialog reflect the selection in 
the *Topics* view when the dialog is opened.

The partitions can be modified by selecting from a drop-down list. The 
topics are not editable.

.. _`Record Topic(s) dialog Partition parameter`:

.. centered:: **Record Topic(s) dialog Partition parameter**

.. image:: /images/028_TopicsRecordPartition.png
   :width: 140mm
   :align: center
   :alt: Record Topic(s) dialog Partition parameter

The fields are:

*RNR Service ID* –
  Specify the RNR Service ID parameter.

*Scenario Name* –
  Specify the scenario name. By default if one topic is chosen, 
  a name that reflects the topic is used. Otherwise a generic 
  record name is provided.

  The currently-known scenarios in the DDS system are checked, 
  and a scenario name is provided where no current scenarios 
  of that name are known.

*Storage Name* –
  A default storage name is provided that matches the scenario name. 
  If that storage is already known to the DDS system, a warning message 
  is provided at the top of the dialog.

  The storage name can be modified to be any string; a drop-down list 
  is provided to allow for the selection of existing storages. 

*Truncate existing storage* –
  If this option is checked, a ``truncate`` command will be added for 
  the specified storage. Otherwise, no ``truncate`` command is submitted.

*Create new storage* –
  If this option is checked, the storage specified by the *Storage name* 
  field will be created/configured with the specified storage 
  attributes. See `Storage Attributes`_ for more information.

.. _`Scenario Editors`:

Scenario Editors
****************

The actions of a Record and Replay service are organized in *scenarios*.

A scenario is an instance of the scenario topic, a group of commands 
sharing the same ``scenarioName``. Each service subscribes to the command 
topic and uses a content filter to only process commands with an ``rnrId`` 
matching the service name (or ``‘*’``).

It is possible to create an intricate nesting of scenarios by defining a 
scenario that includes control commands targeting other scenarios.

RnR Manager has two editors related to scenarios. These editors are used 
to view and edit the commands of a scenario.

**Scenario Editor** – 
  Enables viewing, adding, and editing scenarios in the DDS system.

**Scenario File Editor** – 
  Enables viewing and editing of scenario data that is persisted using 
  an xml file. The scenarios are saved as xml, and can be loaded into 
  the DDS system as required. 


Overview of the Scenario Editor
===============================

The scenario editor is used to view and edit the commands of a scenario 
within the DDS system.

The scenario editor can be opened from the *Scenario Status* view by any 
of the following actions:

+ double-clicking on a selected scenario in the table 
+ right-clicking on a selected scenario in the table and 
  then choosing *Open* 
+ clicking on the *Create Scenario* button 


.. _`The Scenario Editor`:

.. centered:: **The Scenario Editor**

.. image:: /images/030_ScenarioEditor.png
   :width: 140mm
   :align: center
   :alt: The Scenario Editor

Scenario Editor elements:

*Start* and *Stop* buttons

  The *Start* and *Stop* buttons, when clicked, submit the 
  ``Start`` and ``Stop`` commands for the specified scenario 
  and RnR service ID. These commands are published with a 
  durability setting of ``VOLATILE``.

*Scenario name* and *RnR service ID*

  A new scenario name can be entered, or an existing scenario 
  can be selected from the drop-down list.

  Scenarios with the same scenario name can exist on multiple 
  ``rnr`` services. The RnR service ID is also specified to identify 
  the appropriate scenario. The wildcard character ``‘*’`` (asterisk) 
  can be used to denote all ``rnr`` services. 

*Scenario Commands*

  The scenario commands table displays all commands for a scenario 
  with the specified RnR service ID.

*Command Parameters*

  The command parameters section displays the values associated 
  with the selected command in the scenario commands table.


Overview of the Scenario File Editor
====================================

The scenario file editor can be used to edit the commands of a scenario 
and save them to an xml file. This xml file can later be loaded and 
submitted to a DDS scenario.

The scenario file editor can be opened from the *Workspaces* view by any 
of the following actions:

+ double-clicking on a scenario file (under the *Saved Scenarios* folder)

+ right-clicking on a selected scenario file and then choosing *Open*


.. _`Workspace Saved Scenarios`:

.. centered:: **Workspace Saved Scenarios**

.. image:: /images/031_WorkspaceScenarios.png
   :width: 60mm
   :align: center
   :alt: Workspace Saved Scenarios



.. _`The Scenario File Editor`:

.. centered:: **The Scenario File Editor**

.. image:: /images/032_ScenarioFileEditor.png
   :width: 140mm
   :align: center
   :alt: The Scenario File Editor


Scenario File Editor elements:

*Save* and *Save As...* buttons

  The *Save* button saves the current xml file.

  The *Save As...* button saves the scenario to a new file and 
  opens a new scenario file editor.

*Scenario name* and *RnR service ID*

  A new scenario name can be entered, or an existing scenario 
  can be selected from the drop-down list.

  Scenarios with the same scenario name can exist on multiple 
  ``rnr`` services. The RnR service ID is also specified to 
  identify the appropriate scenario. The wildcard character 
  ``‘*’`` (asterisk) can be used to denote all ``rnr`` services.

*Scenario Commands*

  The scenario commands table displays all commands for a scenario 
  with the specified RnR service ID. 

*Command Parameters*

  The command parameters section displays the values associated 
  with the selected command in the scenario commands table.

Re-sizing the Editors
=====================

**Full View**

In order to maximize the view of an editor, double-click on the editor 
tab.

.. _`Scenario Editor tab`:

.. centered:: **Scenario Editor tab**

.. image:: /images/033_ScenarioEditor_tab.png
   :width: 50mm
   :align: center
   :alt: Scenario Editor tab


To restore the regular view, click on the ‘restore’ button, or 
double-click again on the editor tab.

.. _`Resizing an Editor`:

.. centered:: **Resizing an Editor**

.. image:: /images/034_ResizeAnEditor.png
   :width: 140mm
   :align: center
   :alt: Resizing an Editor


**Resize Editor groups**

The widths of the Scenario Commands and Command Parameters groups can be 
changed by dragging the border with the mouse.

.. _`Resizing Editor groups`:

.. centered:: **Resizing Editor groups**

.. image:: /images/035_ResizeEditorGroups.png
   :width: 140mm
   :align: center
   :alt: Resizing Editor groups


Scenario Editor Features
========================

.. _`A Scenario Editor`:

.. centered:: **A Scenario Editor**

.. image:: /images/030_ScenarioEditor.png
   :width: 140mm
   :align: center
   :alt: A Scenario Editor

The scenario editor displays the current state and the existing commands 
of a scenario in the DDS system.

+ Existing commands for a specific scenario are displayed 
  and are not editable. 

+ New commands can be added and submitted.

The functionality provided by the editor changes depending on the state 
of the scenario.

The state of a scenario can also be determined by viewing the scenario 
in the *Scenario Status* view.


Running *vs.* Stopped Scenarios
-------------------------------

For ``RUNNING`` scenarios:
..........................

The *Start* button is disabled, the *Stop* button is enabled.

The *Submit* button is enabled.

Previously-submitted commands are displayed in the scenario commands 
table, but are ‘view only’. A visual indicator of this is that the 
font color is light gray. These commands can not be removed or edited.

.. _`Editable and non-editable commands`:

.. centered:: **Editable and non-editable commands**

.. image:: /images/037_EditableAndUneditable.png
   :width: 60mm
   :align: center
   :alt: Editable and non-editable commands


For ``STOPPED`` scenarios:
..........................

The *Start* button is enabled, the *Stop* button is disabled.

The *Submit* button is disabled, as it is not necessary. All commands are 
submitted if the *Start* button is clicked.

Previously-submitted commands displayed in the scenario commands table 
that were uneditable while the scenario was in the ``RUNNING`` state 
become editable again when in the ``STOPPED`` state.


Adding new commands
-------------------

To add a new command, click on the *Add* button in the 
*Scenario Commands* section.

Specify the appropriate parameters for this new command in 
the *Command Parameters* section.

+ If a scenario is already in the ``RUNNING`` state, click on 
  the *Submit* button to submit the command or commands.

+ If a scenario is in the ``STOPPED`` state, click on the 
  *Start* button at the top of the editor to submit all 
  the new commands and start the scenario.

.. _`A Running Scenario`:

.. centered:: **A Running Scenario**

.. image:: /images/038_RunningScenario.png
   :width: 140mm
   :align: center
   :alt: A Running Scenario


Existing commands
-----------------

|caution|
  Once a command has been submitted for a scenario, it 
  cannot be deleted or edited.

Removing commands
-----------------

The *Remove* button is only enabled if a command is editable.

Only newly-added commands are editable.

Ordering commands
-----------------

The *Up* and *Down* buttons allow for the commands to be re-ordered, if the selected command is editable.

Alternatively, the editable selected command can be moved by dragging and dropping it in the commands table.

Only newly-added commands are editable in running Scenarios. If a scenario is stopped, then all commands can be re-ordered.

Duplicating commands
-----------------

Scenario commands can be duplicated using *Copy* and *Paste* buttons available by right-clicking them to bring up the context menu. 

Users can copy any command, which will copy all of the settings included in that command and place it on RnRManager's clipboard. 

Users can only paste a copied command where the "Paste" context menu entry is enabled. Pasting a command will insert it after a selected 
command, however users cannot insert commands after already submitted commands in a running scenario; they will have to stop the scenario 
to do that similar to the re-ordering of the commands in the previous section.

To paste a command:

#. Select the command after which you want to insert it
#. Right click on that command
#. Press the "Paste" button

All scenario commands can be duplicated, whether they have been submitted or not in both Live and Offline mode in the Scenario Editor.

Validation
----------

Commands that have invalid parameters are highlighted with red text in 
the scenario commands table. Individual invalid command parameters also 
have their labels highlighted with red text.

If the command is edited to correct the parameters, the entry in the 
scenario commands table will no longer be highlighted with red.

The *Submit* and *Start* buttons will do nothing if there are validation 
errors. 

.. _`Invalid command parameters`:

.. centered:: **Invalid command parameters**

.. image:: /images/039_InvalidParameters.png
   :width: 140mm
   :align: center
   :alt: Invalid command parameters



*Save As...*
------------

The scenario editor has a *Save As...* button in the 
top-right corner.

.. _`Scenario Editor Save As... button`:

.. centered:: **Scenario Editor** *Save As...* **button**

.. image:: /images/040_SaveAs_button.png
   :width: 18mm
   :align: center
   :alt: Scenario Editor Save As... button


The *Save As...* functionality enables the saving of a scenario’s 
commands and command parameters as an xml file that is not known 
to the DDS system.

Please refer to the next section, `Scenario File Editor Features`_,
for more information on saving scenarios to files.

Scenario File Editor Features
=============================


.. _`The Scenario File Editor`:

.. centered:: **The Scenario File Editor**

.. image:: /images/032_ScenarioFileEditor.png
   :width: 140mm
   :align: center
   :alt: The Scenario File Editor


Creating New Files
------------------

**Scenario Editor Save As**

A new scenario file can be created by clicking on the 
*Save As...* button in the top right corner of the scenario 
editor. This will save the commands specified in the scenario 
editor to an xml file.

This allows for the saving of a specific DDS system scenario 
to be loaded at a later date.

**New Scenario File**

A new file can be created by right-clicking on a folder under 
the *Saved Scenarios* directory structure in the *Workspace* 
view and choosing *New Scenario File...*.

.. _`New Scenario File`:

.. centered:: **New Scenario File**

.. image:: /images/042_NewScenarioFile.png
   :width: 70mm
   :align: center
   :alt: New Scenario File


Editing Commands
----------------

+ To add a new command, click on the *Add* button in the 
  *Scenario Commands* section.

  Specify the appropriate parameters for this new command 
  in the *Command Parameters* section.

+ To remove a command, click on the *Remove* button in the 
  *Scenario Commands* section.

Load Scenario
-------------

A scenario file can be quickly loaded and submitted into 
the DDS system.

**Step 1**

  Select the xml file, right-click and choose *Load Scenario*.


.. _`Load Scenario`:

.. centered:: **Load Scenario**

.. image:: /images/031_WorkspaceScenarios.png
   :width: 70mm
   :align: center
   :alt: Load Scenario


**Step 2**

  Specify merge options in the *Load Scenario* wizard.


  .. _`Load Scenario wizard`:

  .. centered:: **Load Scenario wizard**

  .. image:: /images/044_LoadScenarioWizard.png
     :width: 100mm
     :align: center
     :alt: Load Scenario wizard


  If a senario with the same name already exists, the wizard 
  offers these merge options:

  *Overwrite existing scenario* – 
    The existing scenario with the same name 
    will be stopped (if not already stopped) and disposed.

  *Rename scenario in file* – 
    The scenario name in the file will be 
    changed, and that scenario name will be loaded.

  *Automatically create referenced storages* – 
    Create any storages referenced in the file if they do 
    not already exist in the DDS system.

  A new scenario editor opens and the commands from the file are displayed.


**Step 3**

  Click on *Play* to submit the scenario commands to the DDS system.


Validation
----------

Commands that have invalid parameters are highlighted with red text in 
the scenario commands table. Individual invalid command parameters also 
have their labels highlighted with red text.

If the command is edited to correct the parameters, the entry in the 
scenario commands table will no longer be highlighted with red.

|info|
  A scenario file *can* be saved with invalid parameters. 


Command Parameters
==================

To view or edit the command parameters of a scenario command, choose the 
command in the *Scenario Commands* table. The values associated with that 
command will populate the scenario editor’s *Command Parameters* 
section. If the command is editable, the parameters can be modified. 

**Common Parameters**

Various command types are supported. To specify the command type, use 
the dropdown box entitled *Kind*. 

The RnR service ID is also a common parameter for all command kinds.

**Kind Parameters**

The *Kind Parameters* section is where all the command kind-specific 
parameters are edited or viewed.

The command kinds supported are:

 | Add Record
 | Add Replay
 | Remove Record
 | Remove Replay
 | Set Replay Speed
 | Truncate
 | Start 
 | Stop
 | Suspend
 | Config


Add Record
----------

The add record command is used to specify an interest to record data to 
a storage.

.. _`Add Record parameters`:

.. centered:: **Add Record parameters**

.. image:: /images/045_AddRecordParameters.png
   :width: 100mm
   :align: center
   :alt: Add Record parameters

The parameters are:

*Storage name* – 
  The name of the storage in which the data is to be stored. 
  If the storage cannot be resolved, the command is ignored. 
  To assist you, a dropdown list of recognized storages is provided.

*Partition/Topic Sets* – 
  One or more partition/topic sets can be added to define the 
  record interest.

Wildcards are supported: ``‘?’`` will match a single character and 
``‘*’`` will match any number of characters. If expressions (partially) 
overlap, data will only be recorded once.

Add Replay
----------

The add replay command is used to specify an interest to replay data 
from a storage.

.. _`Add Replay parameters`:

.. centered:: **Add Replay parameters**

.. image:: /images/046_AddReplayParameters.png
   :width: 100mm
   :align: center
   :alt: Add Replay parameters



.. _`Add Replay time ranges`:

.. centered:: **Add Replay time ranges**

.. image:: /images/046a_AddReplayTimeRanges.png
   :width: 100mm
   :align: center
   :alt: Add Replay time ranges



.. _`Add Replay QoS transformations`:

.. centered:: **Add Replay QoS transformations**

.. image:: /images/046b_AddReplayQoStransforms.png
   :width: 100mm
   :align: center
   :alt: Add Replay QoS transformations

The parameters are:

*Storage name* –
  The name of the storage from which the data will be 
  replayed. If the storage cannot be resolved the command 
  is ignored.

*Use original timestamps* – 
  With the default value of ``true``, when a sample is recorded, 
  its original write and allocation timestamps are preserved. 
  When this sample is replayed, it will be delivered to readers 
  with these original timestamps. Depending on resource limits and 
  QoS settings, readers may discard the replayed data if data with 
  more recent timestamps is available. By setting 
  ``useOriginalTimestamps`` to ``false``, the timestamps will be 
  updated with the current time upon replay.

*Skip to first sample* – 
  By default this value is false and when a sample matches interest 
  expressions but doesn’t match any of the supplied time-ranges, 
  the Record and Replay service tries to mimic original timing behaviour 
  by sleeping until the next sample is evaluated based on record 
  timestamps. Sometimes this is not the required behavior and the 
  service should simply skip all non-matching samples and start 
  replaying immediately the first sample that matches an interest 
  expression and time-range. This behaviour can be enabled by setting 
  ``skipToFirstSample`` to ``true``.

*Partition/Topic Sets* – 
  One or more partition/topic sets can be added to define the 
  replay interest.

  Wildcards are supported: ``‘?’`` will match a single character and 
  ``‘*’`` will match any number of characters. If expressions (partially) 
  overlap, data will only be published into DDS once.

*Time Ranges* – 
  Specifying time ranges is optional.

  Time ranges are used in combination with partition/topic sets to select 
  a subset of data available in a storage for replay. Each time range is 
  applied to each partition/topic set. A sample read from a storage is 
  only replayed if its partition/topic can be matched against the 
  partition/topic sets and its record-time can be matched against the 
  time-ranges. The time ranges are optional; when omitted, a sample is 
  replayed when a partition/topic set matches.
  
*QoS Transformations* - 
  Specifying QoS transformations is optional.
  
  A series of transformations to apply to each sample's QoS upon replay. 
  Transformations are only applied for replay samples that have matched 
  the partition/topic set *AND* the time ranges (if specified). The supported 
  transformations types and their valid values are described in the tables
  below.

  .. tabularcolumns:: | p{3cm} | p{10.5cm} |

  +-------------+--------------------------------------------------------+
  | QoS Type    | Description                                            |
  +=============+========================================================+
  | Partition   | Partition in which the sample is replayed              |
  +-------------+--------------------------------------------------------+
  | Deadline    | Deadline QoS policy                                    |
  +-------------+--------------------------------------------------------+
  | Latency     | Latency budget QoS policy                              |
  +-------------+--------------------------------------------------------+
  | Ownership   | Ownership strength QoS policy (applies only to samples |
  | strength    | written with exclusive ownership-kind QoS policy)      |
  +-------------+--------------------------------------------------------+
  | Transport   | Transport priority QoS policy                          |
  | priority    |                                                        |
  +-------------+--------------------------------------------------------+
  | Lifespan    | Lifespan QoS policy                                    |
  +-------------+--------------------------------------------------------+


  .. tabularcolumns:: | p{3cm} | p{10.5cm} |

  +-------------+--------------------------------------------------------+
  | QoS Type    | Valid Values                                           |
  +=============+========================================================+
  | Partition   | Partition transformations are supported for non-empty  |
  |             | partition names consisting of alphanumeric and         |
  |             | special characters ``‘-’``, ``‘/’`` and ``‘_’``.       |
  +-------------+--------------------------------------------------------+
  | Ownership   |                                                        |
  | strength    |                                                        |
  |             | Any positive 32 bit integer.                           |
  | Transport   |                                                        |
  | priority    |                                                        |
  +-------------+--------------------------------------------------------+
  | Deadline    | A floating point value for seconds and nanoseconds.    |
  |             | *eg.* inputting ``10.5`` is 10 seconds and 500000000   |
  | Latency     | nanoseconds.                                           |
  |             | Additionally, the string ``"Infinite"`` is also valid  |
  | Lifespan    | input, signifying the 'infinite' time period           |
  |             | (2147483647.2147483647)                                |
  +-------------+--------------------------------------------------------+

  The string value written to the *Replacement Value* column will be the new 
  value of the QoS policy. Optionally, values in the *Original Value* column 
  are taken as the filter to conditionally apply the transformation; 
  *e.g.* for a Partition transformation with *Replacement Value* ``"p2"`` 
  and *Original Value* ``"p1"``, all samples with partition ``"p1"`` will be 
  transformed to partition ``"p2"``. 
  
  |caution| Note that the original value needs to be an *exact* match; 
  wildcards and expressions are *not* supported.


**Time Range Copy and Paste**

The time range record values can be copied from the storage editor and 
pasted into the scenario editor. Please refer to 
`Selected Row(s) Menu`_ for details of how to copy a time range.

To paste the record time range, Add a new time range. Select the time 
range in the Time Ranges table, and right-click and then choose Paste 
Record Time Range. 

.. _`Paste Record Time Range`:

.. centered:: **Paste Record Time Range**

.. image:: /images/047_PasteRecordTimeRange.png
   :width: 100mm
   :align: center
   :alt: Paste Record Time Range



Remove Record
-------------
The remove record command is used to remove a record interest from a 
storage.

.. _`Remove Record parameters`:

.. centered:: **Remove Record parameters**

.. image:: /images/048_RemoveRecordParameters.png
   :width: 100mm
   :align: center
   :alt: Remove Record parameters

The parameters are:

*Storage name* – 
  The name of the storage of which record-interest is 
  removed.

*Partition/Topic Sets* – 
  One or more partition/topic sets can be added to define 
  the record interest(s) to be removed.

  |caution|
    Note that the partition/topic sets need to match those used 
    previously in an add record command.

Remove Replay
-------------

The remove replay command is used to remove a replay interest from a 
storage.

.. _`Remove Replay parameters`:

.. centered:: **Remove Replay parameters**

.. image:: /images/049_RemoveReplayParameters.png
   :width: 100mm
   :align: center
   :alt: Remove Replay parameters


The parameters are:

*Storage name* – 
  The name of the storage of which replay-interest is 
  removed.

*Partition/Topic Sets* – 
  One or more partition/topic sets can be added to define 
  the replay interest(s) to be removed.

  |caution| Note that the partition/topic sets need to match those 
  used previously in an add replay command.
  
*QoS Transformations* - 
  Specifying QoS transformations is optional.

  The QoS transformations to remove from the replay interest. If 
  specified, a replay interest will no longer apply the transformation. 
  
  |caution| If any replay interest is to be removed completely, 
  the set of transformations should match *exactly* the set included in 
  add_replay command(s) responsible for adding the interest.

*Time Ranges* – 
  Specifying time ranges is optional.

  The time ranges to remove. Similar to the add replay command, this 
  parameter is optional. If this parameter is specified, only the interest 
  that exactly matches the time ranges is removed. As a shortcut, if no 
  time ranges are specified, any interest that matches the interest 
  expressions in the remove replay command is removed regardless of the 
  time ranges attached to that interest.


Set Replay Speed
----------------

The set replay speed command is used to change the replay speed of a 
storage.

.. _`Replay Speed parameters`:

.. centered:: **Replay Speed parameters**

.. image:: /images/050_ReplaySpeedParameters.png
   :width: 90mm
   :align: center
   :alt: Replay Speed parameters


The parameters are:

*Storage name* – 
  The name of the storage of which to change the replay 
  speed. If the storage cannot be resolved the command is ignored.

*Replay speed* – 
  Floating-point value containing the new replay-speed. 
  The following values have a special meaning:

  ``-1`` : *Maximum speed*

           Delays between samples are skipped and the samples 
           from the storage are inserted into DDS as fast 
           as possible.

  ``1``  : *Same speed*

           Replays samples with the same timing characteristics 
           as when originally recorded.

  ``0``  : *Pause the storage*

           No samples are replayed until the speed is increased. 


  The default replay speed is ``1`` (samples are replayed with the 
  same timing characteristics as when originally recorded).

Truncate
--------

This command can be used to clear a storage. 

When recording samples to an existing storage, the data is *appended*. 
If instead the required behaviour is to *overwrite* the storage, the 
truncate command can be used to remove the data recorded to the 
storage during previous sessions.

|caution|
  Note that the truncate command can only be executed if the storage 
  isn’t busy recording and/or replaying data. Thus is may be neccessary 
  to first publish remove record/replay commands, in order to remove all 
  interest from a storage so that it gets closed by the RnR service, 
  before the truncate command can be successfully processed. The *Storage 
  Status* view can be used to verify the state of the storage.

.. _`Truncate parameter`:

.. centered:: **Truncate parameter**

.. image:: /images/051_TruncateParameter.png
   :width: 90mm
   :align: center
   :alt: Truncate parameter

The parameter is:

*Storage name* – 
  The name of the storage to clear.

Start
-----

The start command instructs the service to start processing ``"scenario 
name"``. The service will publish a status update that changes the state 
of the scenario to ``RUNNING``. If the target scenario is already known to 
the service, and is in the suspended state, a start command causes the 
scenario to resume processing commands, changing the scenario status 
from ``SUSPENDED`` to ``RUNNING``.

.. _`Start parameter`:

.. centered:: **Start parameter**

.. image:: /images/052_StartParameter.png
   :width: 90mm
   :align: center
   :alt: Start parameter

The parameter is:

*Scenario name* – 
  The name of the scenario to start.

Stop
----

The stop command stops the execution of a scenario, including any 
recording and/or replaying that was defined as part of that scenario.

.. _`Stop parameter`:

.. centered:: **Stop parameter**

.. image:: /images/053_StopParameter.png
   :width: 90mm
   :align: center
   :alt: Stop parameter

The parameter is:

*Scenario name* – 
  The name of the scenario to stop.

|caution|
  It is important to understand that a scenario, once stopped, cannot 
  be started again.

  However, it is possible to start a new scenario with the same name as 
  the stopped scenario. If any commands of the original scenario were 
  published as transient data they will be delivered to and processed by 
  the new scenario, giving the impression that the scenario has been 
  re-started.

Suspend
-------

The suspend command suspends the processing of commands by the scenario. 
This enables applications to submit a number of commands to a scenario, 
without any immediate effects. When the scenario is resumed all new 
commands are processed as if they were published in a batch although in 
reality they may have been published with varying intervals.

.. _`Suspend parameter`:

.. centered:: **Suspend parameter**

.. image:: /images/054_SuspendParameter.png
   :width: 90mm
   :align: center
   :alt: Suspend parameter

The parameter is:

*Scenario name* – 
  The name of the scenario to suspend.


CONFIG
------

The config command is used to modify the runtime configuration of an RnR 
service.

  **config** : Sequence of ``KeyValue`` objects.

A config command can be used to add a storage to the service or modify 
properties of an existing storage. Storages can also be configured in 
the OpenSplice configuration file, but config commands provide the 
opportunity to create and configure storages dynamically.

A single config command can apply to multiple storages, if the config 
sequence consists of multiple elements. The key of the ``KeyValue`` object 
should always be ``‘Storage’``.

.. _`CONFIG command`:

.. centered:: **CONFIG command**

.. image:: /images/055_CONFIGcommand.png
   :width: 90mm
   :align: center
   :alt: CONFIG command

One or more storage configurations can be specified in the CONFIG 
command.

*Name* – 
  The name used to identify the storage in Record and Replay 
  commands.

*Filename* – 
  The file used to store XML data. The filename may include a 
  relative or absolute path. If a path is omitted, the storage file
  is created in the current working directory.

*Kind* – 
  This attribute sets the format of the storage file. The data 
  can either be stored in XML or CDR format. 
  
  |caution| 
    Note that when changing the Kind of an existing storage 
    it is recommended that a ``TRUNCATE`` command be issued first so 
    that the storage file does not contain data of both formats.

*Max file size* - 
  This optional attribute specifies a maximum storage size for 
  individual files. Exceeding a single file size rolls the storage 
  over to a new numbered index file. No value or ``0`` indicates 
  unlimited file size. Valid input is any positive integer not greater 
  than ``2^64 -1`` (in bytes). The value can have a suffix to indicate
  magnitude: ``K``(ilobyte), ``M``(egabyte) or ``G``(igabtye); for 
  example, ``10M`` results in ``10485760`` bytes. The suffix is 
  not case sensitive.

*Statistics* – 
  This optional attribute specifies whether statistics 
  should be maintained for this storage.

*Interval* – 
  This attribute specifies the publication interval of the 
  statistics belonging to this storage, in a Record and Replay storage 
  statistics topic. The publish interval is a value in seconds but may 
  also be set to ``-1``, which means that the statistics are published when 
  the storage is closed. Note that a value of ``0`` means that statistics 
  are never published for this storage.

*Reset* – 
  This attribute enables you to reset the current values of 
  statistics belonging to the storage. Note that this only makes sense in 
  a configuration command for an existing storage, since new storages 
  created from the OpenSplice configuration file always start out with 
  empty statistics.

BuiltinScenario
===============

OpenSplice BuiltInScenario
--------------------------

Since commands are targeted at a service and a scenario, the service 
must start an initial scenario. If it did not, there wouldn’t be 
anything to address commands to.

During startup, the service starts this initial scenario, called the 
``BuiltinScenario``. This is a special scenario that is always running while 
the service is operational. It serves as the starting hook for any new 
scenarios. To run a new scenario, a start command must be published for 
the ``BuiltinScenario``. Like any scenario, the ``BuiltinScenario`` can also 
process other commands like record and/or replay commands.

|caution| 
  Note that the ``BuiltinScenario`` cannot be stopped.

Since one can assume that the ``BuiltinScenario`` is always available and 
running, it is a safe choice to address config and control commands to 
the BuiltinScenario. Especially in a dynamic and distributed 
environment, in which DDS is regularly used, this can be helpful when 
interacting with the service through scripts or perhaps when injecting 
commands stored in a persistent store.

BuiltinScenario in RnR Manager
------------------------------

The scenario editor disables the start and stop functionality for the 
``BuiltinScenario``.

*Start* and *Stop* actions are also disabled in the *Scenario Status* view.

Storage Data
************

The OpenSplice Record and Replay service enables the recording and 
replaying of data. The service records the data to storage xml files. 
These xml files can get quite large, and can be difficult to query and 
read.

RnR Manager provides a Storage Editor that makes it easier to query, 
view and update this storage data.

On import, the xml storage files are converted to a database file. The 
database can then be queried and updated. Updated data can be exported 
back into the OpenSplice Record and Replay service storage xml format if 
required.

Workspace - Imported Storages
=============================

The starting point for viewing storage data is the *Workspace* view.

To import storage data, navigate to the *Workspace* view and select the 
*Imported Storages* folder. Right-click to display a menu with options for 
manipulating storage database files.

.. _`Imported Storages view`:

.. centered:: **Imported Storages view**

.. image:: /images/056_ImportedStoragesView.png
   :width: 90mm
   :align: center
   :alt: Imported Storages view


Database File Management
========================

The *Imported Storages* folder displays the database files that have been 
imported. The files can be managed by using the menu options provided in 
the view. These menu options are accessible by right-clicking on the 
items in the view.

.. _`Imported Storages menu`:

.. centered:: **Imported Storages menu**

.. image:: /images/057_ImportedStoragesMenu.png
   :width: 90mm
   :align: center
   :alt: Imported Storages menu


These database files are created in a workspace project maintained by 
the RnR Manager, which provides facilities to open, copy, paste, delete, 
and create new folders within this workspace.

Import Storage
==============
To import OpenSplice record and replay storage data, open the *Import 
Storage* dialog by choosing the *Import Storage...* option from the 
right-click menu. (A folder must be selected in the view.)

.. _`Import Storage`:

.. centered:: **Import Storage**

.. image:: /images/058_ImportStorage.png
   :width: 100mm
   :align: center
   :alt: Import Storage


To import:

**Step 1**  

  Browse to and select your local storage file.

  If storage file names were configured with a relative path, the 
  RnR service creates the storage files relative to its process working 
  directory.

  Using the file chooser, select the ``.meta`` file associated with 
  the storage. The storage data file is automatically resolved.


**Step 2**

  Specify the database file name. The file will be created in the 
  selected folder.

**Step 3**  

  If necessary, decrease the batch size. The default batch size 
  is ``5000``.

  A batch counter is used to limit the number of samples loaded into 
  memory. If the import is unsuccessful, and/or ‘out of memory’ 
  exceptions are occurring, reduce the batch size number.

Export
======

To export OpenSplice record and replay storage data, open the *Export 
Storage* dialog by choosing the *Export Storage...* option from the 
right-click menu. (A database file must be selected in the view.)

On export a new OpenSplice storage is created and the sample data 
contained in the imported database file will be written to this storage.

Storage Parameters
------------------

The storage parameters are specified on the first page of the Export 
Storage wizard.

.. _`Export Storage - Create Storage`:

.. centered:: **Export Storage - Create Storage**

.. image:: /images/060_Export_CreateStorage.png
   :width: 100mm
   :align: center
   :alt: Export Storage - Create Storage

If RnR Manager is currently connected to a DDS domain, the storage 
attributes are editable, and clicking on the *Finish* button configures 
a new storage with the specified storage attributes. If RnR Manager is 
not connected, only the file name attribute is editable, and clicking 
*Finish* only does the data export to file. See `Storage Attributes`_
for further information.

If necessary, decrease the batch size. The default batch size 
is ``10000``.

A batch counter is used to limit the number of samples loaded into 
memory. If the export is unsuccessful, and/or ‘out of memory’ 
exceptions are occurring, reduce the batch size number.

Sample Set Parameters
---------------------

The second page of the wizard enables you to export a subset of the 
sample data contained in the database. You can also edit the actual 
database file to contain the appropriate subset using the storage 
editor. The ability to specify subset parameters is an additional 
feature of export. 

All unique partition/topic sets of the database file are displayed. You 
can choose to exclude any of these sets by unchecking it the list.

Additionally you can define a time range for the export. Any data 
samples that do not fall within the time range are excluded.

.. _`Export Storage Samples`:

.. centered:: **Export Storage Samples**

.. image:: /images/061_Export_StorageSamples.png
   :width: 100mm
   :align: center
   :alt: Export Storage Samples




Storage Editor
**************

Once a storage has been imported, the database file can be queried and 
updated using the *Storage Editor*.

The storage editor is opened by double-clicking on the database file in 
the *Imported Storages* folder in the *Workspace* view. It can also be 
opened automatically on import.

The editor is comprised of two pages. The first page is used to define 
the query parameters. You can specify query parameters to view subsets 
of the sample data. The second page displays the sample data that meet 
the query parameter criteria.

.. _`Storage Editor`:

.. centered:: **Storage Editor**

.. image:: /images/062_StorageEditor.png
   :width: 140mm
   :align: center
   :alt: Storage Editor


Query Parameters
================

Field Selection
---------------

This section is used to select the data fields (columns) that you would 
like to see displayed in the data page table.

The *Record and Replay* node lists the fields that are written to each 
record and replay sample.

The *Message* node lists the fields that are written with each sample.

The other nodes contain the fields that are specific to each MetaType or 
topic.

Where Clause
------------

The *where* section can be used to defined SQL-like ``where`` conditions.

This section is optional. If no entries are added, no additional 
conditions are added when running the query and populating the data page.

**Operator**

To define a *where* condition an operator must be added. The operator can 
be an ``AND`` or an ``OR``. 

To add an operator, click on the *Add Operator* button. By default, 
``AND`` is selected. To add ``OR``, select the cell, then choose the 
``OR`` item in the dropdown menu.

.. _`Add Operator`:

.. centered:: **Add Operator**

.. image:: /images/063_WHEREoperator.png
   :width: 90mm
   :align: center
   :alt: Add Operator

Each operator must have at least one child clause for the query to be 
valid. If this condition is not met, a red icon is displayed to indicate 
a problem. The *Run Query* button will not be enabled unless all 
validation problems on the query page are resolved.

.. _`Invalid AND`:

.. centered:: **Invalid AND**

.. image:: /images/064_InvalidAND.png
   :width: 60mm
   :align: center
   :alt: Invalid AND


**Clause**

To add *where* selection criteria, add one or more clauses to your 
operator. Each clause contains a field, an operator and a value.

*Field* – 
  A database field or metaType field. Available fields are 
  provided in a dropdown list.

  .. _`Clause field`:

  .. centered:: **Clause field**

  .. image:: /images/065_WhereClauseField.png
     :width: 70mm
     :align: center
     :alt: Clause field



*Clause Operator* – 
  Various comparison operators are available for selection in a 
  dropdown list.


  .. _`Clause Operator`:

  .. centered:: **Clause Operator**

  .. image:: /images/066_WhereClauseOperator.png
     :width: 70mm
     :align: center
     :alt: Clause Operator



*Value* – 
  The expected value of the database field.

Each clause must have a defined field and value. If this condition is 
not met, a red icon is displayed to indicate a problem. The *Run Query* 
button will not be enabled unless all validation problems on the query 
page are resolved.

.. _`Invalid Clause`:

.. centered:: **Invalid Clause**

.. image:: /images/067_InvalidClause.png
   :width: 90mm
   :align: center
   :alt: Invalid Clause



.. _`Valid Condition`:

.. centered:: **Valid Condition**

.. image:: /images/068_ValidWHERE.png
   :width: 90mm
   :align: center
   :alt: Valid Condition



Time Range (message.writeTime)
------------------------------

By default the time range is set to all times. A start and/or end time 
can be defined to query samples within a specific time range. 

Load
----

Many of the storage files have a large amount of data. For larger files, 
it is not always possible to load all of the data into memory. For these 
cases, samples can be incrementally loaded into memory.

The number of samples to load at a time is specified in the ``Load #`` field.

You can then use the *Next* and *Previous* buttons on the data page to view 
the samples incrementally.

|caution|
  **Note**: Any edits made to loaded data will **not** be retained 
  when the *Next* and *Previous* buttons are clicked. Such edits must 
  be saved explicitly prior to loading next or previous samples.

Order By
--------

The data display can be ordered by data fields. By default, the order is 
done using the ``message.writeTime`` fields. You can add any data field to 
the order by criteria.

Data
----

The data page contains one table that displays the samples that meet the 
query criteria. Each row is one sample.

The columns are resizable. The columns can also be re-ordered by 
dragging them in the table.

To get the full name of the column, hover on the column header with the 
mouse.

Values can be edited in the table on a *per cell* basis.

To change the data that is displayed, specify different query parameters 
on the query page, and click on the *Run Query* button.

.. _`Data page`:

.. centered:: **Data page**

.. image:: /images/069_DataPage.png
   :width: 145mm
   :align: center
   :alt: Data page


Selected Row(s) Menu
--------------------

Actions are available for the table’s selected row(s). These actions 
are accessed by using right-click.

.. _`Data table actions`:

.. centered:: **Data table actions**

.. image:: /images/070_DataTableActions.png
   :width: 100mm
   :align: center
   :alt: Data table actions


**Multi Edit...**

The table supports editing a selected row’s field value.

However, if a batch change is required, it is not possible to do it in 
the table itself. The *Multi Edit* dialog provides support for batch 
changes. 

**Step 1**
  
  Select the rows you would like to edit in the table. 

**Step 2**
  
  Right-click and choose the *Multi Edit...* action.

**Step 3**

  Check the field(s) you would like to batch update.

**Step 4**

  Specify the value(s) to be applied to all the selected rows.


.. _`Multi Edit dialog`:

.. centered:: **Multi Edit dialog**

.. image:: /images/071_MultiEditDialog.png
   :width: 100mm
   :align: center
   :alt: Multi Edit dialog


**Copy Record Time Range** 

This action is enabled when more than one row is selected. It is used in 
conjunction with the scenario editor.

The record time of the first and last selected row are saved. These 
values can be pasted into the scenario editor for a replay command. 
Please refer to `Add Replay`_ for details of how to paste the values 
into the scenario editor.

**Delete** 

The delete action deletes all selected rows.


Next and Previous
-----------------

The *Next* and *Previous* buttons are enabled when the samples are being 
loaded incrementally.

Please see `Load`_ for details of load functionality.


.. _`Next and Previous buttons`:

.. centered:: **Next and Previous buttons**

.. image:: /images/072_Next_Previous_buttons.png
   :width: 20mm
   :align: center
   :alt: Next and Previous buttons



Undo, Save and Save As...
-------------------------

.. _`Undo, Save, and Save As... buttons`:

.. centered:: **Undo, Save, and Save As... buttons**

.. image:: /images/073_Undo_etc_buttons.png
   :width: 35mm
   :align: center
   :alt: Undo, Save, and Save As... buttons


The buttons are:

*Undo* — 
  Undo all changes to the database file since the last save.

*Save* — 
  Save all changes to the database file.

*Save As...* — 
  Save the currently loaded samples into a new database file.


.. end


.. |connectedbutton| image:: ./images/013_DomainConnected_button.png
            :height: 4mm


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

         
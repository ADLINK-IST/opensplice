.. _`Scenarios`:


#########
Scenarios
#########

*The actions of a Record and Replay service are organized in* 
**scenarios**.

A *scenario* is an instance of the scenario topic, a group of commands 
sharing the same ``scenarioName``. Each service subscribes to the 
command topic and uses a content filter to only process commands with an 
``rnrId`` matching the service name (or ``*``).

It is possible to create an intricate nesting of scenarios by defining 
a scenario that includes control commands targeting other scenarios.

Different versions of the scenario topic
****************************************

Starting with Vortex OpenSplice V6.5.2, the Record and Replay service 
provides two versions of the scenario topic: ``rr_scenario`` and 
``rr_scenario_v2``. The new ``rr_scenario_v2`` topic is contained 
in the ``RnR_V2`` IDL module, please see
:ref:`the RnR Topic API IDL specification <Appendix A>`
in the :ref:`Appendix <Appendix A>` for details of the changes 
between the original and version 2 of the data-model. 
Both versions will co-exist until a major version upgrade 
allows the merging of all features into a single module and topic.
This will probably coincide with a migration of the data-model to 
Google Protocol Buffers (*GPB*) or a different extensible type scheme 
supported by Vortex OpenSplice at that time.

The original topic is still supported for backwards compatibility with 
applications developed for a previous version of OpenSplice or Record 
and Replay scenarios stored for re-use, *i.e.* in a persistent store. 
Also, because of the nature of the RnR service, the topic definitions of 
RnR may have been introduced in production environments that cannot 
easily be upgraded and/or restarted to replace the old topics with new 
ones. A topic mis-match would prevent a new version of RnR from starting 
on any node attached to the same domain. In those circumstances a new 
topic using new type-names is the only viable approach to support new 
RnR features. Transformations (partition, QoS) of data during replay are 
only available on the ``rr_scenario_v2`` topic because these new 
features require an extension of the ``ADD_REPLAY`` and ``REMOVE_REPLAY`` 
commands.
 
The  next chapter describes all command-types, highlighting differences 
between the original and version 2.

Since all original features are also available using the 
``rr_scenario_v2`` topic, it is not recommended to mix usage of the 
two topics in a single  application. The service does support scenarios 
that use both original and V2 commands but order preservation cannot 
be guaranteed, since a scenario is no longer contained in a single 
instance but is in two instances on two different topics, necessitating 
two individual readers. In practice this can only introduce order reversal 
of commands when both ``rr_scenario`` and ``rr_scenario_v2`` commands 
arrive at the same time.


BuiltinScenario
***************

Since commands are targeted at a service *and* a scenario, the service 
must start an initial scenario. If not, there wouldn’t be anything to 
address commands to.

|info|

During startup, the service starts this initial scenario, called the
``BuiltinScenario``. This is a special scenario that is always running 
while the service is operational. It serves as the starting hook for any 
new scenarios. To run a new scenario, a *start* command must be 
published for the ``BuiltinScenario``. Like any scenario, the
``BuiltinScenario`` can also process other commands like record and/or 
replay commands.

|caution| Note that the ``BuiltinScenario`` can not be stopped.

Since one can assume that the ``BuiltinScenario`` is always available 
and running, it is a safe choice to address config and control commands 
to the ``BuiltinScenario``. In a dynamic and distributed environment, in 
which DDS is regularly used, this can be especially helpful when 
interacting with the service through scripts or perhaps when injecting 
commands stored in a persistent store.


Command durability
******************

The command subscriber of the service is capable to read commands of any
durability (``VOLATILE``, ``TRANSIENT``, ``PERSISTENT``). If commands 
are published with a *transient* and/or *persistent* durability, it is 
important to understand that these commands are managed by the 
middleware in addition to the service. Immediately after a scenario is 
started, any commands still managed by the middleware in transient or 
persistent stores are delivered to the service and processed as part of 
the scenario.

|caution| This is of special importance when ‘re-starting’ scenarios. 
   Note that a scenario, strictly, is *not* restarted. It is removed and
   a *new scenario* with an identical name is created. If any of the 
   commands of the original scenario were published (whether transient 
   or persistent), these are delivered and processed again by the 
   *new* scenario.

Since transient and persistent commands exist in the middleware and are 
not stored or processed by the service as long as the corresponding 
scenario isn’t started, the start command for a scenario does not have 
to be published before the scenario is defined, as one might assume. 
By changing the durability of different commands compromising a scenario,
advanced use cases are possible using relatively simple scenarios.



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

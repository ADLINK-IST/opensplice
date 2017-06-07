.. _`Known Issues`:


############
Known Issues
############

*This section summarizes known issues and limitations of the 
current release of the Record and Replay Service. Please see the* 
Release Notes *supplied with the product for additional information.*


Ability to create topics
************************

The service does not yet have the ability to implicitly create topics 
for the data it replays. To bypass this limitation, the ``DCPSTopic`` 
built-in topic can be included in the recording (and replaying) of data.

The following interest-expression should be used:

  ``__BUILT-IN PARTITION__.DCPSTopic``

To ensure that a topic is re-created at replay before any data belonging 
to that topic gets replayed, the ``DCPSTopic`` expression should be the 
*first* interest-expression that is added to a particular storage using 
the add record/replay commands.



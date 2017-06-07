.. _`Troubleshooting`:


###############
Troubleshooting
###############

*This section provides information for the diagnosis of common 
(security-related) issues, error messages, and configuration 
problems.*


Known Issues
************

This section lists known issues and their resolution.

+ **The networking process does not start up on Windows.** 

  Check for the startup command ``snetworking.exe`` (*cf.* 
  :ref:`Customizing OpenSplice Configuration <Customizing OpenSplice Configuration>` 
  .)

+ **OpenSplice does not start up, although the used 
  configuration is correct.** 

  Make sure that no shared memory file exists from a previous 
  run. This issue shows up mostly on Windows. Remove shared memory 
  files located in ``%TEMP%``.


.. _`How to Diagnose Problems - Logging`:

How to Diagnose Problems - Logging
**********************************

If you are experiencing problems take a look at the OpenSplice 
log files. 

+ Error reports are written to the ``ospl-error.log`` file 
  located in the start-up directory of the OpenSplice daemon, by 
  default. 

+ Information and warning reports are written to the 
  ``ospl-info.log`` file, by default. 

Generally, you do not need to worry about the ``ospl-info.log`` 
file, but in some cases, warning messages may help to understand 
the error message contained in the ``ospl-error.log`` file.


Error Messages
==============

Below are described security-related error messages and their 
resolution. Error messages are written to the ``ospl-error.log`` 
file.

+ **Partition x (<partition_name>)** - 
  undefined security profile ``<profile_name>``.

  This message indicates a missing security profile where **x** 
  is an internal partition identifier to which a security profile 
  named ``<profile_name>`` has been assigned in the configuration 
  file. Check if the ``<profile_name>`` security profile has been 
  defined within the security element of the configuration file.

+ **Failed to initialize the security module.**

  The Security XML configuration is faulty and some information 
  is missing or incorrect. Generally, this message comes after one 
  or more messages reporting missing or incorrect configuration of 
  XML elements. Check the ``ospl-info.log`` security-related messages 
  for additional information.

+ **Partition x (<partition_name>)** - 
  security profile ``<profile_name>`` requires cipher key 
  of ``<bits_nb>`` bits.

  The partition **x** has been assigned a faulty security profile 
  with a missing ``CipherKey`` attribute. Note that:
 
  - Null cipher requires no cipher key
 
  - Blowfish cipher requires a key of 128 bits (32 hex-characters)
 
  - AES128 cipher requires a key of 128 bits (32 hex-characters)
 
  - AES192 cipher requires a key of 192 bits (48 hex-characters)
 
  - AES256 cipher requires a key of 256 bits (64 hex-characters)

+ **Partition x (<partition_name>)** - 
  security profile ``<profile_name>`` with invalid cipher ``<cipher_name>``. 
  
  The partition **x** has been assigned to a faulty security profile 
  with an invalid cipher. Check that the ``<cipher_name>`` identifier 
  is correctly spelled and supported.

  |info|
  Note that only aes128, aes192, aes256, blowfish, rsa-aes128, 
  rsa-192, rsa-256, rsa-blowfish, rsa-null and NULL ciphers are 
  currently supported.

+ **Dropping traffic of partition x due to insufficient cipher 
  key, until re-keying has been done.**

  Partition **x** traffic is temporarily blocked because of faulty 
  cipher key.

+ **Receiving message blocked, bad partition encoding, verify if 
  sending node has security feature enabled.**

  A received message cannot be assigned to a valid network 
  partition, the message is blocked and not delegated to the Data 
  Reader. This may be caused by the security feature not being 
  activated on all participating OpenSplice nodes in the domain. 
  Make sure that the security feature is enabled in all OSPL 
  configuration files (``Security[@enabled=true]``). Note that the 
  network partition is still able to receive data samples from 
  nodes where the OSPL configuration matches the local network 
  partition configuration.

+ **Sending message blocked, bad partition x.**

  Message sending on partition **x** is blocked because of faulty 
  security profile definition. Check the ``ospl-info.log`` for 
  security-related warning messages indicating missing elements.

Warning Messages
================

This section describes security-related warning messages and 
their resolution. Warning messages are written to the 
``ospl-info.log`` file. Note that warning messages can be ignored in 
the absence of errors.

+ **Name attribute of security profile undefined, or empty string.**

  The ``Name`` attribute of one or more ``SecurityProfile`` XML elements 
  is missing in the configuration file.

+ **Cipher attribute of security profile <profile_name> undefined.**

  The ``Cipher`` attribute of one or more ``SecurityProfile`` XML elements 
  is missing in the configuration file.

+ **CipherKey attribute of security profile <profile_name> 
  not defined.**

  The ``CipherKey`` attribute of one or more ``SecurityProfile`` XML 
  elements is missing in the configuration file. Note that the 
  ``CipherKey`` attribute is required only for non-NULL ciphers.

+ **SecurityProfile <profile_name> has invalid cipher key 
  <cipher_key>, check length and encoding.**

  Check the ``<profile_name>`` security profile definition for key 
  length and hexadecimal encoding correctness.

+ **Security profile <profile_name> defines unknown 
  cipher <cipher_name>.**

  Check the ``<cipher_name>`` identifier to see that it is correctly 
  spelled and supported. Note that only aes128, aes192, aes256, blowfish, 
  rsa-aes128, rsa-192, rsa-256, rsa-blowfish, rsa-null and NULL 
  ciphers are currently supported.


.. EoF


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

         
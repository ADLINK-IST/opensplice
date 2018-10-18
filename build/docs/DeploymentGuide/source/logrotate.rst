.. _`Logrotate`:


#########
Logrotate
#########

*The OpenSplice middleware can produce several trace and log files
depending on the applied configuration settings. Log files and in
particular trace files can become very large over time and run into
resource limitations. It is advised to use the logrotate function
to manage resource consumption of log and trace files. The logrotate
function is standard available on linux distributions and for windows
an opensource version is available on sourceforge (LogRotateWin).**

.. _`Description`:

Description
***********

logrotate is designed to ease administration of systems that generate
large numbers of log files. It allows automatic rotation, compression,
removal, and mailing of log files. Each log file may be handled daily,
weekly, monthly, or when it grows too large.
Normally, logrotate is run as a daily cron job. It will not modify a
log multiple times in one day unless the criterion for that log is
based on the log's size and logrotate is being run multiple times each
day, or unless the -f or --forceoption is used. For example, logrotate
can also be run by a supervisory process at any time to process log
files specified in the config_file.
For a more detailed description of logrotate see the linux man pages.

.. _`Configuration file`:

Configuration file
******************

logrotate reads everything about the log files it should be handling
from the series of configuration files specified on the command line.
Each configuration file can set global options (local definitions
override global ones, and later definitions override earlier ones)
and specify log files to rotate.

.. _`Example configuration`:

Example configuration
*********************

For OpenSplice the following example config_file arguments are advised for logrotate:

.. code-block:: bash

   # sample logrotate configuration file
 
   # The copytruncate option specifies that logfiles are first copied and
   # then truncate. This option is required for OpenSplice instead of using
   # the create option to avoid closing the file descriptors used by
   # OpenSplice.
   #
   copytruncate

   # The compress option is used to compress the logfile copies.
   compress

   # The following options specify that the log files in the current directory
   # will be rotates when the file size exceeds 100k and that at most the 5
   # most recent rotated files are maintained.
   "./\*.log" {
       rotate 5
       size 100k
   }

.. EoF


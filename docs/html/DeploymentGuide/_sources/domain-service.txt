.. _`The Domain Service`:

##################
The Domain Service
##################

*The Domain Service is responsible for creating and initialising the database which is
used by the administration to manage the DDS data.*

In the *single process* architecture the Domain Service is started as a new thread
within the DDS application. This is done implicitly when the application invokes
the DDS ``create_participant`` operation and no such service currently exists
within the process. Without an database size configured the Domain Service 
creates the DDS database within the heap memory of the process and so is limited 
only to the maximal heap that the operating system supports. 
To be able to manage the maximum database size a database size can also be given 
in the *single process* mode. Then the Domain Service creates the DDS database 
within the heap memory of the process with the given size and will use it's own
memory manager in this specific allocated memory.

In the *shared memory* architecture, the user is responsible for managing the DDS
administration separately from the DDS application. In this mode, the Domain
Service is started as a separate process; it then creates and initialises the database by
allocating a particular amount of shared memory as dictated by the configuration.
Without this administration, no other service or application is able to participate in
the DDS Domain.

In either deployment mode, once the database has been initialised, the Domain
Service starts the set of pluggable services. In single process mode these services
will be started as threads within the existing process, while in shared memory mode
the services will be represented by new separate processes that can interface with
the shared memory segment.

When a shutdown of the OpenSplice Domain Service is requested in shared memory
mode, it will react by announcing the shutdown using the shared administration.
Applications will not be able to use DDS functionality anymore and services are
requested to terminate elegantly. Once this has succeeded, the Domain Service will
destroy the shared administration and finally terminate itself.

The exact fulfilment of these responsibilities is determined by the configuration
of the Domain Service. There are detailed descriptions of all of the available 
configuration  parameters and their purpose in the :ref:`Configuration <Configuration>`
section


.. EoF

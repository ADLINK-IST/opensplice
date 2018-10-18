.. _`Reader Block`:



############
Reader Block
############

The Reader block represents a DDS data reader entity.

In DDS - "To access the received data, the application must use a typed DataReader attached to the subscriber."


.. figure:: images/reader_block.png
        :alt: DDS Reader Block



=========== ========= ============= =========================== ====================
  Port Type  Optional Name          Description                 Output consumed by
=========== ========= ============= =========================== ====================
Input       yes       psub          DDS Subscriber
                                    entity instance
Input       no        topic         DDS Topic entity instance
Output      yes       status        0 for successful reader
                                    creation
Output      no        data          BUS                         user
Output      yes       info          BUS                         user
Output      yes       samples read  Number of samples read      user
=========== ========= ============= =========================== ====================

.. raw:: latex

    \newpage

Reader Block Parameters
***********************


.. figure:: images/reader_block_parameters.png
        :alt: Reader Block Parameters

Data Tab
========
The **Data** tab is used to set:

- Bus Type

     The output data type (BUS) for the **data** output port

- Mode: take or read

     Specify whether the reader block is accessing the samples using DDS take or DDS read.

- Sample Time

     "The sample time of a block is a parameter that indicates when, during simulation, the block produces outputs and if appropriate, updates its internal state." -Simulink documentation

     Default is -1, meaning it will inherit the Simulink sample time from inputs or the model.
     Valid values: -1 and Numeric > 0

- Bus Width

     The bus width is the maximum number of samples that can be read or take(n) per block step.
     Valid values for the bus width are:  integers >= 1.

.. raw:: latex

    \newpage

- Wait for

     Checking the Historical Data field in the Wait for section specifies that the Reader will wait for historical data to arrive. The Timeout field is for setting time period (in seconds) determining how long the Reader should wait for the historical data.
     If the timeout is reached, then any remaining historical data may be interleaved with new data.

     The Data Available field is for specifying whether the Reader should read only if the data is available. The following Timeouts field determines how long the Reader should wait for the availability of data.
     If the timeout is reached, then the block returns no data and the simulation continues.

Ports Tab
=========
The **Ports** tab allows the user to toggle on or off optional ports.

QoS Tab
=======
The **QoS** tab is used to set the QoS profile.   By default, the OSPL default profile is used.

In DDS - The Data-Distribution Service (DDS) relies on the usage of QoS.  A QoS (Quality of Service) is a set of characteristics that controls some aspect of the behavior of the DDS Service.

Each DDS block has an associated QoS profile.   By default, the OSPL default profile is used.  An XML file that specifies QoS profiles can be used to set the QoS of a DDS block.

.. raw:: latex

    \newpage

Filters Tab
===========

The filters tab allows for the filtering of incoming samples.
The filtering can happen based on a query and/or on a sample read condition(s).

**Query**

Expression: The expression is a SQL condition.

Parameters: N parameters in the format {'a', 'b'}
Each parameter element must be a char array (string).

*Note: Query expressions are only validated at runtime.*


**Read Condition**

The read conditions specified will filter the samples that are read or take(n).

Example: For a reader, the Sample State has **Read** selected and  **Not Read** deselected.

Only samples with a Sample State **Read** will be processed with read or take.
Any samples with the **Not Read** sample state will not be read or take(n).

*Note:  At least one read condition must be selected for each category of Sample State, View State, or Instance State.
If not, an error will be thrown when a diagram simulation is run.*

.. figure:: images/reader_filters_tab.png
        :alt: Reader Block Parameters : Filters

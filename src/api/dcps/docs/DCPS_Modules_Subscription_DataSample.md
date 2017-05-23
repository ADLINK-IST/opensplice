The DataSample      {#DCPS_Modules_Subscription_DataSample}
===============

Data is made available to the application by the following operations on DataReader objects: read, read_w_condition, take, and take_w_condition. The general semantics of the “read” operations is that the application only gets access to the corresponding data; the data remains the middleware’s responsibility and can be read again. The semantics of the “take” operation is that the application takes full responsibility for the data; that data will no longer be accessible to the DataReader. Consequently, it is possible for a DataReader to access the same sample multiple times but only if all previous accesses were read operations.

Each of these operations returns an ordered collection of Data values and associated SampleInfo objects. Each data value represents an atom of data information (i.e., a value for one instance). This collection may contain samples related to the same or different instances (identified by the key). Multiple samples can refer to the same instance if the settings of the HISTORY QoS allow for it.


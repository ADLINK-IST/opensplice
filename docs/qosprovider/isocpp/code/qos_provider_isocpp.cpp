#include <stdio.h>
#include <stdlib.h>
#include <dds/dds.hpp>

int main()
{
    try
    {
        dds::core::QosProvider provider("file://path/to/file.xml", "FooQosProfile");
        // As default QoS profile is "FooQosProfile", requesting
        // "TransientKeepLast" in this case is equivalent to requesting
        // "::FooQosProfile::TransientKeepLast".
        dds::sub::qos::DataReaderQos drQos = provider.datareader_qos("TransientKeepLast");

        // As default QoS profile is "FooQosProfile", requesting
        // "Transient" would have been equivalent to requesting
        // "::FooQosProfile::Transient".
        dds::pub::qos::DataWriterQos dwQos = provider.datawriter_qos("::FooQosProfile::Transient");

        // As default QoS profile is "FooQosProfile" it is necessary
        // to use the fully-qualified name to get access to QoS-ses from
        // the "BarQosProfile".
        dds::pub::qos::DataWriterQos dwBarQos = provider.datawriter_qos("::BarQosProfile::Persistent");

        // Create DDS DataReader with drQos DataReaderQos,
        // DDS DataWriter with dwQos DataWriterQos and
        // DDS DataWriter with dwBarQos DataWriterQos
    }
    catch(...)
    {
        printf ("Initialization of QosProvider failed.");
    }
}

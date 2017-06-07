#include <stdio.h>
#include <stdlib.h>
#include "dds_dcps.h"

int
main (void)
{
    DDS_QosProvider provider = NULL;
    DDS_DataReaderQos *drQos = NULL;
    DDS_DataWriterQos *dwQos = NULL;
    DDS_DataWriterQos *dwBarQos = NULL;
    DDS_ReturnCode_t result;

    provider = DDS_QosProvider__alloc ("file://path/to/file.xml", "FooQosProfile");
    if (provider == NULL) {
        printf ("Out of memory or syntax error in XML file");
        exit (EXIT_FAILURE);
    } else {
        // As default QoS profile is "FooQosProfile", requesting
        // "TransientKeepLast" in this case is equivalent to requesting
        // "::FooQosProfile::TransientKeepLast".
        result = DDS_QosProvider_get_datareader_qos (provider, drQos, "TransientKeepLast");
        if(result != DDS_RETCODE_OK){
         printf ("Unable to resolve ReaderQos.");
         exit (EXIT_FAILURE);
        }
        // As default QoS profile is "FooQosProfile", requesting
        // "Transient" would have been equivalent to requesting
        // "::FooQosProfile::Transient".
        result = DDS_QosProvider_get_datawriter_qos (provider, dwQos, "::FooQosProfile::Transient");
        if(result != DDS_RETCODE_OK){
         printf ("Unable to resolve WriterQos.");
         exit (EXIT_FAILURE);
        }
        // As default QoS profile is "FooQosProfile" it is necessary
        // to use the fully-qualified name to get access to QoS-ses from
        // the "BarQosProfile".
        result = DDS_QosProvider_get_datawriter_qos (provider, dwBarQos, "::BarQosProfile::Persistent");
        if(result != DDS_RETCODE_OK){
         printf ("Unable to resolve WriterQos.");
         exit (EXIT_FAILURE);
        }
        // Create DDS DataReader with drQos DataReaderQos,
        // DDS DataWriter with dwQos DataWriterQos and
        // DDS DataWriter with dwBarQos DataWriterQos
        return result;
    }
}

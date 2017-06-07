#include <stdio.h>
#include <stdlib.h>
#include "ccpp_dds_dcps.h"
#include "QosProvider.h"

using namespace DDS;

int
main (void)
{
    QosProvider_var provider = NULL;
    DataReaderQos drQos;
    DataWriterQos dwQos;
    DataWriterQos dwBarQos;
    ReturnCode_t result;

    provider = new QosProvider ("file://path/to/file.xml", "FooQosProfile");
    if (provider == NULL) {
        printf ("Out of memory or syntax error in XML file");
        exit (EXIT_FAILURE);
    } else {
        // As default QoS profile is "FooQosProfile", requesting
        // "TransientKeepLast" in this case is equivalent to requesting
        // "::FooQosProfile::TransientKeepLast".
        result = provider->get_datareader_qos (drQos, "TransientKeepLast");
        if(result != RETCODE_OK){
         printf ("Unable to resolve ReaderQos.");
         exit (EXIT_FAILURE);
        }
        // As default QoS profile is "FooQosProfile", requesting
        // "Transient" would have been equivalent to requesting
        // "::FooQosProfile::Transient".
        result = provider->get_datawriter_qos (dwQos, "::FooQosProfile::Transient");
        if(result != RETCODE_OK){
         printf ("Unable to resolve WriterQos.");
         exit (EXIT_FAILURE);
        }
        // As default QoS profile is "FooQosProfile" it is necessary
        // to use the fully-qualified name to get access to QoS-ses from
        // the "BarQosProfile".
        result = provider->get_datawriter_qos (dwBarQos, "::BarQosProfile::Persistent");
        if(result != RETCODE_OK){
         printf ("Unable to resolve WriterQos.");
         exit (EXIT_FAILURE);
        }
        // Create DDS DataReader with drQos DataReaderQos,
        // DDS DataWriter with dwQos DataWriterQos and
        // DDS DataWriter with dwBarQos DataWriterQos
        return result;
    }
}

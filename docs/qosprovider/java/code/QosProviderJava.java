import DDS.*;

public class QosProviderJava {

    public static void main(String[] args) {
        QosProvider provider = null;
        DataReaderQosHolder drQosHolder = new DataReaderQosHolder();
        DataWriterQosHolder dwQosHolder = new DataWriterQosHolder();
        DataWriterQosHolder dwBarQosHolder = new DataWriterQosHolder();
        int result;

        try {
            provider = new QosProvider("file://path/to/file.xml","FooQosProfile");
            // As default QoS profile is "FooQosProfile", requesting
            // "TransientKeepLast" in this case is equivalent to requesting
            // "::FooQosProfile::TransientKeepLast".
            result = provider.get_datareader_qos(drQosHolder, "TransientKeepLast");
            if(result != RETCODE_OK.value){
                System.err.println("Unable to resolve ReaderQos.");
                System.exit(1);
            }
            // As default QoS profile is "FooQosProfile", requesting
            // "Transient" would have been equivalent to requesting
            // "::FooQosProfile::Transient".
            result = provider.get_datawriter_qos(dwQosHolder, "::FooQosProfile::Transient");
            if(result != RETCODE_OK.value){
                System.err.println("Unable to resolve WriterQos.");
                System.exit(1);
            }
            // As default QoS profile is "FooQosProfile" it is necessary
            // to use the fully-qualified name to get access to QoS-ses from
            // the "BarQosProfile".
            result = provider.get_datawriter_qos(dwBarQosHolder, "::BarQosProfile::Persistent");
            if(result != RETCODE_OK.value){
                System.err.println("Unable to resolve WriterQos.");
                System.exit(1);
            }
            // Create DDS DataReader with drQosHolder.value DataReaderQos,
            // DDS DataWriter with dwQosHolder.value DataWriterQos and
            // DDS DataWriter with dwBarQosHolder.value DataWriterQos
        } catch(NullPointerException npe){
            System.err.println("Initialization of QosProvider failed.");
        }
        return;
    }
}
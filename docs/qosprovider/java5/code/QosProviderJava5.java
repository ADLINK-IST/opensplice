import org.omg.dds.core.QosProvider;
import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.domain.DomainParticipantFactory;
import org.omg.dds.sub.DataReaderQos;
import org.omg.dds.pub.DataWriterQos;

public class QosProviderJava5 {

    public static void main(String[] args) {

        ServiceEnvironment       env = null;
        DomainParticipantFactory dpf = null;
        QosProvider provider = null;
        DataReaderQos drQos = null;
        DataWriterQos dwQos = null;
        DataWriterQos dwBarQos = null;
        int result;

        try {
            System.setProperty(ServiceEnvironment.IMPLEMENTATION_CLASS_NAME_PROPERTY, "org.opensplice.dds.core.OsplServiceEnvironment");
            env = ServiceEnvironment.createInstance(QosProviderJava5.class.getClassLoader());
            dpf = DomainParticipantFactory.getInstance(env);
            provider = QosProvider.newQosProvider("file://path/to/file.xml", "FooQosProfile", env);

            // As default QoS profile is "FooQosProfile", requesting
            // "TransientKeepLast" in this case is equivalent to requesting
            // "::FooQosProfile::TransientKeepLast".
            drQos = provider.getDataReaderQos("TransientKeepLast");
            if(drQos == null){
                System.err.println("Unable to resolve ReaderQos.");
                System.exit(1);
            }
            // As default QoS profile is "FooQosProfile", requesting
            // "Transient" would have been equivalent to requesting
            // "::FooQosProfile::Transient".
            dwQos = provider.getDataWriterQos("::FooQosProfile::Transient");
            if(dwQos == null){
                System.err.println("Unable to resolve WriterQos.");
                System.exit(1);
            }
            // As default QoS profile is "FooQosProfile" it is necessary
            // to use the fully-qualified name to get access to QoS-ses from
            // the "BarQosProfile".
            dwBarQos = provider.getDataWriterQos("::BarQosProfile::Persistent");
            if(dwBarQos == null){
                System.err.println("Unable to resolve WriterQos.");
                System.exit(1);
            }
            // Create DDS DataReader with drQos DataReaderQos,
            // DDS DataWriter with dwQos DataWriterQos and
            // DDS DataWriter with dwBarQos DataWriterQos
        } catch(Exception e){
            System.err.println("Initialization of QosProvider failed.");
        }
        return;
    }
}
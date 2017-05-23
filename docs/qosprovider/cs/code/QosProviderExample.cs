using System;

public class QosProviderExample
{
    public static void Main (string[] args)
    {
        DDS.QosProvider provider = null;
        DDS.DataReaderQos drQos = new DDS.DataReaderQos ();
        DDS.DataWriterQos dwQos = new DDS.DataWriterQos ();
        DDS.DataWriterQos dwBarQos = new DDS.DataWriterQos ();
        DDS.ReturnCode result;

        provider = new DDS.QosProvider ("file://path/to/file.xml","FooQosProfile");
        if (provider != null) {
            // As default QoS profile is "FooQosProfile", requesting
            // "TransientKeepLast" in this case is equivalent to requesting
            // "::FooQosProfile::TransientKeepLast".
            result = provider.GetDataReaderQos (ref drQos, "TransientKeepLast");
            if (result != 0) {
                Console.WriteLine("Unable to resolve ReaderQos.");
                System.Environment.Exit(1);
            }
            // As default QoS profile is "FooQosProfile", requesting
            // "Transient" would have been equivalent to requesting
            // "::FooQosProfile::Transient".
            result = provider.GetDataWriterQos (ref dwQos, "::FooQosProfile::Transient");
            if (result != 0) {
                Console.WriteLine("Unable to resolve WriterQos.");
                System.Environment.Exit(1);
            }
            // As default QoS profile is "FooQosProfile" it is necessary
            // to use the fully-qualified name to get access to QoS-ses from
            // the "BarQosProfile".
            result = provider.GetDataWriterQos (ref dwQos, "::BarQosProfile::Persistent");
            if (result != 0) {
                Console.WriteLine("Unable to resolve WriterQos.");
                System.Environment.Exit(1);
            }
        } else {
            Console.WriteLine("Initialization of QosProvider failed.");
        }
    }
}


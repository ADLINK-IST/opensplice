public class $(type-name)TypeSupportImpl extends org.opensplice.dds.dcps.FooTypeSupportImpl
{
    private static java.lang.String idl_type_name = "$(scoped-meta-type-name)";
    private static java.lang.String idl_key_list = "$(key-list)";
    private static java.lang.String dataReaderClass = "$(java-class-name)DataReader";
    private static java.lang.String dataReaderViewClass = "$(java-class-name)DataReaderView";
    private static java.lang.String dataWriterClass = "$(java-class-name)DataWriter";
    private static java.lang.String type_description = "$(type-description)";

    private long copyCache;

    public void $(type-name)TypeSupport()
    {
        super.Alloc(
                this,
                idl_type_name,
                idl_key_list,
                type_description);
    }

    public int register_type(
            DDS.DomainParticipant participant,
            java.lang.String type_name)
    {
        return super.registerType(
                this,
                participant,
                type_name);
    }

}

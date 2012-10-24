import org.opensplice.dds.dcps.Utilities;

public class $(type-name)TypeSupport extends org.opensplice.dds.dcps.TypeSupportImpl implements $(type-name)TypeSupportOperations
{
    private static java.lang.String idl_type_name = "$(scoped-meta-type-name)";
    private static java.lang.String idl_key_list = "$(key-list)";

    private long copyCache;

    public $(type-name)TypeSupport()
    {
        super("$(java-class-name)DataReaderImpl",
            "$(java-class-name)DataReaderViewImpl",
            "$(java-class-name)DataWriterImpl",
            "(L$(java-class-name)TypeSupport;)V",
            "$(java-org-package-name)",
            "$(java-tgt-package-name)");

        int success = 0;

        try {
            success = org.opensplice.dds.dcps.FooTypeSupportImpl.Alloc(
                    this,
                    idl_type_name,
                    idl_key_list,
                    $(scoped-type-name)MetaHolder.metaDescriptor);
        } catch (UnsatisfiedLinkError ule) {
            /*
             * JNI library is not loaded if no instance of the
             * DomainParticipantFactory exists.
             */
            DDS.DomainParticipantFactory f = DDS.DomainParticipantFactory.get_instance();

            if (f != null) {
                success = org.opensplice.dds.dcps.FooTypeSupportImpl.Alloc(
                        this,
                        idl_type_name,
                        idl_key_list,
                        $(scoped-type-name)MetaHolder.metaDescriptor);
            }
        }
        if (success == 0) {
            throw Utilities.createException(
                    Utilities.EXCEPTION_TYPE_NO_MEMORY,
                    "Could not allocate $(type-name)TypeSupport." );
        }
    }

    protected void finalize() throws Throwable
    {
    	try {
    		org.opensplice.dds.dcps.FooTypeSupportImpl.Free(this);
    	}
        catch(Throwable t){
    	}
    	finally{
    	    super.finalize();
    	}
		
    }

    public long get_copyCache()
    {
        return copyCache;
    }

    public int register_type(
            DDS.DomainParticipant participant,
            java.lang.String type_name)
    {
        return org.opensplice.dds.dcps.FooTypeSupportImpl.registerType(
                this,
                participant,
                type_name);
    }

    public String get_type_name()
    {
        return idl_type_name;
    }

}

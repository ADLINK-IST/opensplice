public class $(type-name)TypeSupport extends org.opensplice.dds.dcps.TypeSupportImpl implements DDS.TypeSupportOperations
{
    private static final long serialVersionUID = 1L;

    private long copyCache;

    public $(type-name)TypeSupport()
    {
        super("$(scoped-meta-type-name)",
              "$(internal-type-name)",
              "$(key-list)",
              $(java-package-redirects),
              $(scoped-type-name)MetaHolder.metaDescriptor);
    }

    @Override
    protected DDS.DataWriter create_datawriter ()
    {
        return new $(type-name)DataWriterImpl(this);
    }

    @Override
    protected DDS.DataReader create_datareader ()
    {
        return new $(type-name)DataReaderImpl(this);
    }

    @Override
    protected DDS.DataReaderView create_dataview ()
    {
        return new $(type-name)DataReaderViewImpl(this);
    }
}

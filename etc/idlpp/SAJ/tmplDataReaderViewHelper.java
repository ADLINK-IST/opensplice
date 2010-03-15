import org.opensplice.dds.dcps.Utilities;

public final class $(type-name)DataReaderViewHelper
{

    public static $(scoped-type-name)DataReaderView narrow(java.lang.Object obj)
    {
        if (obj == null) {
            return null;
        } else if (obj instanceof $(scoped-type-name)DataReaderView) {
            return ($(scoped-type-name)DataReaderView)obj;
        } else {
            throw Utilities.createException(Utilities.EXCEPTION_TYPE_BAD_PARAM, null);
        }
    }

    public static $(scoped-type-name)DataReaderView unchecked_narrow(java.lang.Object obj)
    {
        if (obj == null) {
            return null;
        } else if (obj instanceof $(scoped-type-name)DataReaderView) {
            return ($(scoped-type-name)DataReaderView)obj;
        } else {
            throw Utilities.createException(Utilities.EXCEPTION_TYPE_BAD_PARAM, null);
        }
    }

}

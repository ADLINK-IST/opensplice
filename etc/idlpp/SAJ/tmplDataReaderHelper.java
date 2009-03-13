import org.opensplice.dds.dcps.Utilities;

public final class $(type-name)DataReaderHelper
{

    public static $(scoped-type-name)DataReader narrow(java.lang.Object obj)
    {
        if (obj == null) {
            return null;
        } else if (obj instanceof $(scoped-type-name)DataReader) {
            return ($(scoped-type-name)DataReader)obj;
        } else {
            throw Utilities.createException(Utilities.EXCEPTION_TYPE_BAD_PARAM, null);
        }
    }

    public static $(scoped-type-name)DataReader unchecked_narrow(java.lang.Object obj)
    {
        if (obj == null) {
            return null;
        } else if (obj instanceof $(scoped-type-name)DataReader) {
            return ($(scoped-type-name)DataReader)obj;
        } else {
            throw Utilities.createException(Utilities.EXCEPTION_TYPE_BAD_PARAM, null);
        }
    }

}

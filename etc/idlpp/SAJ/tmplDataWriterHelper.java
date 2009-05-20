import org.opensplice.dds.dcps.Utilities;

public final class $(type-name)DataWriterHelper
{

    public static $(scoped-type-name)DataWriter narrow(java.lang.Object obj)
    {
        if (obj == null) {
            return null;
        } else if (obj instanceof $(scoped-type-name)DataWriter) {
            return ($(scoped-type-name)DataWriter)obj;
        } else {
            throw Utilities.createException(Utilities.EXCEPTION_TYPE_BAD_PARAM, null);
        }
    }

    public static $(scoped-type-name)DataWriter unchecked_narrow(java.lang.Object obj)
    {
        if (obj == null) {
            return null;
        } else if (obj instanceof $(scoped-type-name)DataWriter) {
            return ($(scoped-type-name)DataWriter)obj;
        } else {
            throw Utilities.createException(Utilities.EXCEPTION_TYPE_BAD_PARAM, null);
        }
    }

}

import org.opensplice.dds.dcps.Utilities;

public final class $(type-name)TypeSupportHelper
{

    public static $(scoped-type-name)TypeSupport narrow(java.lang.Object obj)
    {
        if (obj == null) {
            return null;
        } else if (obj instanceof $(scoped-type-name)TypeSupport) {
            return ($(scoped-type-name)TypeSupport)obj;
        } else {
            throw Utilities.createException(Utilities.EXCEPTION_TYPE_BAD_PARAM, null);
        }
    }

    public static $(scoped-type-name)TypeSupport unchecked_narrow(java.lang.Object obj)
    {
        if (obj == null) {
            return null;
        } else if (obj instanceof $(scoped-type-name)TypeSupport) {
            return ($(scoped-type-name)TypeSupport)obj;
        } else {
            throw Utilities.createException(Utilities.EXCEPTION_TYPE_BAD_PARAM, null);
        }
    }

}

package DDS;

import org.opensplice.dds.dcps.Utilities;

public final class PublicationBuiltinTopicDataTypeSupportHelper
{

    public static DDS.PublicationBuiltinTopicDataTypeSupport narrow(java.lang.Object obj)
    {
        if (obj == null) {
            return null;
        } else if (obj instanceof DDS.PublicationBuiltinTopicDataTypeSupport) {
            return (DDS.PublicationBuiltinTopicDataTypeSupport)obj;
        } else {
            throw Utilities.createException(Utilities.EXCEPTION_TYPE_BAD_PARAM, null);
        }
    }

    public static DDS.PublicationBuiltinTopicDataTypeSupport unchecked_narrow(java.lang.Object obj)
    {
        if (obj == null) {
            return null;
        } else if (obj instanceof DDS.PublicationBuiltinTopicDataTypeSupport) {
            return (DDS.PublicationBuiltinTopicDataTypeSupport)obj;
        } else {
            throw Utilities.createException(Utilities.EXCEPTION_TYPE_BAD_PARAM, null);
        }
    }

}

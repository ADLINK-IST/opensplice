package DDS;

import org.opensplice.dds.dcps.Utilities;

public final class PublicationBuiltinTopicDataDataReaderHelper
{

    public static DDS.PublicationBuiltinTopicDataDataReader narrow(java.lang.Object obj)
    {
        if (obj == null) {
            return null;
        } else if (obj instanceof DDS.PublicationBuiltinTopicDataDataReader) {
            return (DDS.PublicationBuiltinTopicDataDataReader)obj;
        } else {
            throw Utilities.createException(Utilities.EXCEPTION_TYPE_BAD_PARAM, null);
        }
    }

    public static DDS.PublicationBuiltinTopicDataDataReader unchecked_narrow(java.lang.Object obj)
    {
        if (obj == null) {
            return null;
        } else if (obj instanceof DDS.PublicationBuiltinTopicDataDataReader) {
            return (DDS.PublicationBuiltinTopicDataDataReader)obj;
        } else {
            throw Utilities.createException(Utilities.EXCEPTION_TYPE_BAD_PARAM, null);
        }
    }

}

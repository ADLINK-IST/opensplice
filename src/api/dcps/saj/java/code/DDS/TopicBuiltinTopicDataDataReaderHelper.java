package DDS;

import org.opensplice.dds.dcps.Utilities;

public final class TopicBuiltinTopicDataDataReaderHelper
{

    public static DDS.TopicBuiltinTopicDataDataReader narrow(java.lang.Object obj)
    {
        if (obj == null) {
            return null;
        } else if (obj instanceof DDS.TopicBuiltinTopicDataDataReader) {
            return (DDS.TopicBuiltinTopicDataDataReader)obj;
        } else {
            throw Utilities.createException(Utilities.EXCEPTION_TYPE_BAD_PARAM, null);
        }
    }

    public static DDS.TopicBuiltinTopicDataDataReader unchecked_narrow(java.lang.Object obj)
    {
        if (obj == null) {
            return null;
        } else if (obj instanceof DDS.TopicBuiltinTopicDataDataReader) {
            return (DDS.TopicBuiltinTopicDataDataReader)obj;
        } else {
            throw Utilities.createException(Utilities.EXCEPTION_TYPE_BAD_PARAM, null);
        }
    }

}

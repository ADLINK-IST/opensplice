package DDS;

import org.opensplice.dds.dcps.Utilities;

public final class TopicBuiltinTopicDataTypeSupportHelper
{

    public static DDS.TopicBuiltinTopicDataTypeSupport narrow(java.lang.Object obj)
    {
        if (obj == null) {
            return null;
        } else if (obj instanceof DDS.TopicBuiltinTopicDataTypeSupport) {
            return (DDS.TopicBuiltinTopicDataTypeSupport)obj;
        } else {
            throw Utilities.createException(Utilities.EXCEPTION_TYPE_BAD_PARAM, null);
        }
    }

    public static DDS.TopicBuiltinTopicDataTypeSupport unchecked_narrow(java.lang.Object obj)
    {
        if (obj == null) {
            return null;
        } else if (obj instanceof DDS.TopicBuiltinTopicDataTypeSupport) {
            return (DDS.TopicBuiltinTopicDataTypeSupport)obj;
        } else {
            throw Utilities.createException(Utilities.EXCEPTION_TYPE_BAD_PARAM, null);
        }
    }

}

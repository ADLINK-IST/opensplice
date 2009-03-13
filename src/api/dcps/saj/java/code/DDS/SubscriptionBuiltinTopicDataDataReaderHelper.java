package DDS;

import org.opensplice.dds.dcps.Utilities;

public final class SubscriptionBuiltinTopicDataDataReaderHelper
{

    public static DDS.SubscriptionBuiltinTopicDataDataReader narrow(java.lang.Object obj)
    {
        if (obj == null) {
            return null;
        } else if (obj instanceof DDS.SubscriptionBuiltinTopicDataDataReader) {
            return (DDS.SubscriptionBuiltinTopicDataDataReader)obj;
        } else {
            throw Utilities.createException(Utilities.EXCEPTION_TYPE_BAD_PARAM, null);
        }
    }

    public static DDS.SubscriptionBuiltinTopicDataDataReader unchecked_narrow(java.lang.Object obj)
    {
        if (obj == null) {
            return null;
        } else if (obj instanceof DDS.SubscriptionBuiltinTopicDataDataReader) {
            return (DDS.SubscriptionBuiltinTopicDataDataReader)obj;
        } else {
            throw Utilities.createException(Utilities.EXCEPTION_TYPE_BAD_PARAM, null);
        }
    }

}

package DDS;

import org.opensplice.dds.dcps.Utilities;

public final class SubscriptionBuiltinTopicDataTypeSupportHelper
{

    public static DDS.SubscriptionBuiltinTopicDataTypeSupport narrow(java.lang.Object obj)
    {
        if (obj == null) {
            return null;
        } else if (obj instanceof DDS.SubscriptionBuiltinTopicDataTypeSupport) {
            return (DDS.SubscriptionBuiltinTopicDataTypeSupport)obj;
        } else {
            throw Utilities.createException(Utilities.EXCEPTION_TYPE_BAD_PARAM, null);
        }
    }

    public static DDS.SubscriptionBuiltinTopicDataTypeSupport unchecked_narrow(java.lang.Object obj)
    {
        if (obj == null) {
            return null;
        } else if (obj instanceof DDS.SubscriptionBuiltinTopicDataTypeSupport) {
            return (DDS.SubscriptionBuiltinTopicDataTypeSupport)obj;
        } else {
            throw Utilities.createException(Utilities.EXCEPTION_TYPE_BAD_PARAM, null);
        }
    }

}

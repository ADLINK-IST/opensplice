package DDS;

import org.opensplice.dds.dcps.Utilities;

public final class ParticipantBuiltinTopicDataTypeSupportHelper
{

    public static DDS.ParticipantBuiltinTopicDataTypeSupport narrow(java.lang.Object obj)
    {
        if (obj == null) {
            return null;
        } else if (obj instanceof DDS.ParticipantBuiltinTopicDataTypeSupport) {
            return (DDS.ParticipantBuiltinTopicDataTypeSupport)obj;
        } else {
            throw Utilities.createException(Utilities.EXCEPTION_TYPE_BAD_PARAM, null);
        }
    }

    public static DDS.ParticipantBuiltinTopicDataTypeSupport unchecked_narrow(java.lang.Object obj)
    {
        if (obj == null) {
            return null;
        } else if (obj instanceof DDS.ParticipantBuiltinTopicDataTypeSupport) {
            return (DDS.ParticipantBuiltinTopicDataTypeSupport)obj;
        } else {
            throw Utilities.createException(Utilities.EXCEPTION_TYPE_BAD_PARAM, null);
        }
    }

}

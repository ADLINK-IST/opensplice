/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

package DDS;

import org.opensplice.dds.dcps.Utilities;

public final class ParticipantBuiltinTopicDataDataWriterHelper
{

    public static DDS.ParticipantBuiltinTopicDataDataWriter narrow(java.lang.Object obj)
    {
        if (obj == null) {
            return null;
        } else if (obj instanceof DDS.ParticipantBuiltinTopicDataDataWriter) {
            return (DDS.ParticipantBuiltinTopicDataDataWriter)obj;
        } else {
            throw Utilities.createException(Utilities.EXCEPTION_TYPE_BAD_PARAM, null);
        }
    }

    public static DDS.ParticipantBuiltinTopicDataDataWriter unchecked_narrow(java.lang.Object obj)
    {
        if (obj == null) {
            return null;
        } else if (obj instanceof DDS.ParticipantBuiltinTopicDataDataWriter) {
            return (DDS.ParticipantBuiltinTopicDataDataWriter)obj;
        } else {
            throw Utilities.createException(Utilities.EXCEPTION_TYPE_BAD_PARAM, null);
        }
    }

}

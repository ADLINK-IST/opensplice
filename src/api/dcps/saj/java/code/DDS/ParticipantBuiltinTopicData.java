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

public final class ParticipantBuiltinTopicData {

    public int[] key = new int[3];
    public DDS.UserDataQosPolicy user_data = new DDS.UserDataQosPolicy();

    public ParticipantBuiltinTopicData() {
    }

    public ParticipantBuiltinTopicData(
        int[] _key,
        DDS.UserDataQosPolicy _user_data)
    {
        key = _key;
        user_data = _user_data;
    }

}

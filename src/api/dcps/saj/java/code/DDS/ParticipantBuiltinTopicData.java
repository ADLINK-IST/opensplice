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

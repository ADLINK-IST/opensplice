package DDS;

public final class SubscriptionKeyQosPolicy {

    public boolean use_key_list;
    public java.lang.String[] key_list = new java.lang.String[0];

    public SubscriptionKeyQosPolicy() {
    }

    public SubscriptionKeyQosPolicy(boolean _use_key_list, java.lang.String[] _key_list)
    {
        use_key_list = _use_key_list;
        key_list = _key_list;
    }

}

package DDS;

public final class ViewKeyQosPolicy {

    public boolean use_key_list;
    public java.lang.String[] key_list = new java.lang.String[0];

    public ViewKeyQosPolicy() {
    }

    public ViewKeyQosPolicy(boolean _use_key_list, java.lang.String[] _key_list)
    {
        use_key_list = _use_key_list;
        key_list = _key_list;
    }

}

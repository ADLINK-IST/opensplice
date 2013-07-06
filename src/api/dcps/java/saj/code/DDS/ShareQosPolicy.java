package DDS;

public final class ShareQosPolicy {

    public boolean enable;
    public java.lang.String name = "";

    public ShareQosPolicy() {
    }

    public ShareQosPolicy(java.lang.String _name, boolean _enable)
    {
        enable = _enable;
        name = _name;
    }

}

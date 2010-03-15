package DDS;

public final class ShareQosPolicy {

    public boolean enable;
    public java.lang.String name = "";

    public ShareQosPolicy() {
    }

    public ShareQosPolicy(
        boolean _enable,
        java.lang.String _name)
    {
        enable = _enable;
        name = _name;
    }

}

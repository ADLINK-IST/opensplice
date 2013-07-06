package DDS;

public final class ShareQosPolicyHolder
{

    public DDS.ShareQosPolicy value = null;

    public ShareQosPolicyHolder () { }

    public ShareQosPolicyHolder (DDS.ShareQosPolicy initialValue)
    {
        value = initialValue;
    }

}

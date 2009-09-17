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

public class SubscriptionBuiltinTopicDataTypeSupport extends org.opensplice.dds.dcps.TypeSupportImpl implements SubscriptionBuiltinTopicDataTypeSupportOperations
{
    private static java.lang.String idl_type_name = "DDS::SubscriptionBuiltinTopicData";
    private static java.lang.String idl_key_list = "key";

    private long copyCache;

    public SubscriptionBuiltinTopicDataTypeSupport()
    {
        super("DDS/SubscriptionBuiltinTopicDataDataReaderImpl",
            "DDS/SubscriptionBuiltinTopicDataDataWriterImpl",
            "(LDDS/SubscriptionBuiltinTopicDataTypeSupport;)V");

        int success = 0;

        try {
            success = org.opensplice.dds.dcps.FooTypeSupportImpl.Alloc(
                    this,
                    idl_type_name,
                    idl_key_list,
                    DDS.SubscriptionBuiltinTopicDataMetaHolder.metaDescriptor);
        } catch (UnsatisfiedLinkError ule) {
            /*
             * JNI library is not loaded if no instance of the
             * DomainParticipantFactory exists.
             */
            DDS.DomainParticipantFactory f = DDS.DomainParticipantFactory.get_instance();
            
            if (f != null) {
                success = org.opensplice.dds.dcps.FooTypeSupportImpl.Alloc(
                        this,
                        idl_type_name,
                        idl_key_list,
                        DDS.SubscriptionBuiltinTopicDataMetaHolder.metaDescriptor);
            }
        }
        if (success == 0) {
            throw Utilities.createException(
                    Utilities.EXCEPTION_TYPE_NO_MEMORY,
                    "Could not allocate SubscriptionBuiltinTopicDataTypeSupport." );
        }
    }

    protected void finalize()
    {
        org.opensplice.dds.dcps.FooTypeSupportImpl.Free(this);
    }

    public long get_copyCache()
    {
        return copyCache;
    }

    public int register_type(
            DDS.DomainParticipant participant,
            java.lang.String type_name)
    {
        return org.opensplice.dds.dcps.FooTypeSupportImpl.registerType(
                this,
                participant,
                type_name);
    }

    public String get_type_name()
    {
        return idl_type_name;
    }

}

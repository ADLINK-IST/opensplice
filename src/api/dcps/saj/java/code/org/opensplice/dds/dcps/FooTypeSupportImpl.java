/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

package org.opensplice.dds.dcps;

public abstract class FooTypeSupportImpl extends org.opensplice.dds.dcps.TypeSupportImpl
{
    public FooTypeSupportImpl(){
        super("org/opensplice/dds/dcps/FooDataReaderImpl",
                "org/opensplice/dds/dcps/FooDataReaderViewImpl",
              "org/opensplice/dds/dcps/FooDataWriterImpl",
	      "(Lorg/opensplice/dds/dcps/FooTypeSupport;)V",
            null,
            null);
    }

    private native static int jniAlloc (
	Object TypeSupport,
	java.lang.String type_name,
	java.lang.String key_list,
	java.lang.String[] type_descriptor);

    private native static int jniFree (
	Object TypeSupport);

    private native static int jniRegisterType (
	    Object TypeSupport,
	    DDS.DomainParticipant participant,
	    java.lang.String type_alias,
        java.lang.String org_pname,
        java.lang.String tgt_pname);

    public static int Alloc (
	Object TypeSupport,
	java.lang.String type_name,
        java.lang.String key_list,
        java.lang.String[] type_descriptor)
    {
        return
	    jniAlloc (
	        TypeSupport,
		type_name,
		key_list,
		type_descriptor);
    }

    public static int Free (
	Object TypeSupport)
    {
        return
	    jniFree (
	        TypeSupport);
    }

    public static int registerType (
	Object TypeSupport,
	DDS.DomainParticipant participant,
	java.lang.String type_alias)
    {
        return
	    jniRegisterType (
	        TypeSupport,
		participant,
		type_alias,
        ((org.opensplice.dds.dcps.TypeSupportImpl)TypeSupport).getOrgPName(),
        ((org.opensplice.dds.dcps.TypeSupportImpl)TypeSupport).getTgtPName());
    }

}

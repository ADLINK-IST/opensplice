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

/**
 * Implementation of the {@link DDS.TypeSupport} interface.
 */
public abstract class TypeSupportImpl extends SajSuperClass implements DDS.TypeSupport {
    private String dataReaderClass = null;
    private String dataReaderViewClass = null;
    private String dataWriterClass = null;
    private String constructorSignature = null;
    private String orgPName = null;
    private String tgtPName = null;

    public TypeSupportImpl( String dataReaderClass,
                            String dataReaderViewClass,
                            String dataWriterClass,
                            String constructorSignature,
                            String orgPName,
                            String tgtPName)
    {
        this.dataReaderClass = dataReaderClass;
        this.dataReaderViewClass = dataReaderViewClass;
        this.dataWriterClass = dataWriterClass;
        this.constructorSignature = constructorSignature;
        this.orgPName = orgPName;
        this.tgtPName = tgtPName;
    }

    public String get_type_name() {
        return this.jniGetTypeName(this);
    }

    private native static String jniGetTypeName (Object TypeSupport);

    protected String getOrgPName()
    {
        return this.orgPName;
    }

    protected String getTgtPName()
    {
        return this.tgtPName;
    }
}

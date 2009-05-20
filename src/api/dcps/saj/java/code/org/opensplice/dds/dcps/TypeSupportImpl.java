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


package org.opensplice.dds.dcps;

/** 
 * Implementation of the {@link DDS.TypeSupport} interface. 
 */ 
public abstract class TypeSupportImpl extends SajSuperClass implements DDS.TypeSupport {
    private String dataReaderClass = null;
    private String dataWriterClass = null;
    private String constructorSignature = null;
    
    public TypeSupportImpl( String dataReaderClass, 
                            String dataWriterClass, 
                            String constructorSignature)
    {
        this.dataReaderClass = dataReaderClass;
        this.dataWriterClass = dataWriterClass;
        this.constructorSignature = constructorSignature;
    }
    
    public String get_type_name() {
        return this.jniGetTypeName(this);
    }
    
    private native static String jniGetTypeName (Object TypeSupport);
}

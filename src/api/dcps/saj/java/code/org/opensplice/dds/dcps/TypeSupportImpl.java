

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

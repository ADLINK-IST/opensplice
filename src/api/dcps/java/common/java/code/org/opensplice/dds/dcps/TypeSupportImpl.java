/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

package org.opensplice.dds.dcps;

/**
 * Implementation of the {@link DDS.TypeSupport} interface.
 */
public abstract class TypeSupportImpl extends TypeSupportBase implements DDS.TypeSupport {
    private static final long serialVersionUID = -2717096586847665623L;
    private java.lang.String packageRedirects = null;
    private java.lang.String idlTypeName = null;
    private java.lang.String internalTypeName = null;
    private java.lang.String idlKeyList = null;
    private java.lang.String descriptor = null;
    private long jniCopyCache = 0;
    private short dataRepresentationId = DDS.OSPL_REPRESENTATION.value;
    private byte[] typeHash = null;
    private byte[] metaData = null;
    private byte[] extentions = null;

    protected int denit () { return super.deinit(); }

    public TypeSupportImpl( String idlTypeName,
                            String internalTypeName,
                            String idlKeyList,
                            String packageRedirects,
                            String[] typeDescriptor)
    {
        StringBuffer buf = new StringBuffer();
        int size = typeDescriptor.length;

        this.idlTypeName = idlTypeName;
        if (internalTypeName.length() == 0) {
            this.internalTypeName = idlTypeName;
        } else {
            this.internalTypeName = internalTypeName;
        }
        this.idlKeyList = idlKeyList;
        this.packageRedirects = packageRedirects;
        for (int i = 0; i<size; i++) {
            buf.append(typeDescriptor[i]);
        }
        this.descriptor = buf.toString();
    }

    @Override
    public String get_type_name() {
        return idlTypeName;
    }

    protected String get_internal_type_name() {
        return internalTypeName;
    }

    public String get_key_list() {
        return idlKeyList;
    }

    protected String get_package_redirects() {
        return this.packageRedirects;
    }

    protected String get_descriptor() {
        return this.descriptor;
    }

    @Override
    public int register_type (
        DDS.DomainParticipant participant,
        java.lang.String type_alias)
    {
        DomainParticipantImpl dp = (DomainParticipantImpl)participant;
        int result = DDS.RETCODE_BAD_PARAMETER.value;

        ReportStack.start();

        if (participant == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "participant 'null' is invalid.");
        } else {
            if (type_alias == null) {
                type_alias = idlTypeName;
            }
            result = dp.register_type(this, type_alias);
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    public long get_copyCache()
    {
        return jniCopyCache;
    }

    protected void set_copyCache(long copyCache)
    {
        this.jniCopyCache = copyCache;
    }

    public short get_data_representation_id()
    {
        return dataRepresentationId;
    }

    public void set_data_representation_id(short dataRepresentationId)
    {
        this.dataRepresentationId = dataRepresentationId;
    }

    public byte[] get_type_hash()
    {
        return typeHash;
    }

    public void set_type_hash(final byte[] typeHash)
    {
        this.typeHash = typeHash;
    }

    public byte[] get_meta_data()
    {
        return metaData;
    }

    public void set_meta_data(final byte[] metaData)
    {
        this.metaData = metaData;
    }

    public byte[] get_extentions()
    {
        return extentions;
    }

    public void set_extentions(final byte[] extentions)
    {
        this.extentions = extentions;
    }

    abstract protected DDS.DataWriter create_datawriter ();
    abstract protected DDS.DataReader create_datareader ();
    abstract protected DDS.DataReaderView create_dataview ();

}

/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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

package org.opensplice.cmdataadapter;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.StringTokenizer;

import org.opensplice.cm.CMException;
import org.opensplice.cm.DataTypeUnsupportedException;
import org.opensplice.cm.Topic;
import org.opensplice.cm.data.Sample;
import org.opensplice.cm.data.State;
import org.opensplice.cm.data.UserData;
import org.opensplice.cm.meta.MetaType;
import org.opensplice.cmdataadapter.protobuf.ProtobufDataAdapterFactory;


/**
 * TypeInfo represents a set of type evolutions for a single data type
 * registered in the system. It is meant to abstract access to DDS data type
 * descriptors (MetaType) so that it keeps track of different evolutions of the
 * same data type in a manner that is transparent to the caller.
 */
public class TypeInfo {

    /** The data representation id value for OpenSplice-internal type descriptors. */
    public static final short OSPL_DATA_ID = 1024;
    /** The data representation id value for Google Protocol Buffers type descriptors. */
    public static final short GPB_DATA_ID = 1025;

//    public static final short XCDR_DATA_ID = 0; // IDL X-Types id's, not implemented yet in ospl.
//    public static final short XML_DATA_ID = 1;

    /** The list of types. */
    private static List<TypeInfo> types = Collections.synchronizedList(new ArrayList<TypeInfo>());

    /** The type name. This is unique per known type. */
    private String name;
    /**
     * The data_representation_id. The value of the id determines the type of
     * meta data TypeEvolutions will contain.
     */
    private short data_representation_id;

    /**
     * The list of evolutions associated with this data type. It is ordered from
     * most recent to least recent.
     */
    private List<TypeEvolution> typeEvolutions;

    /** The original, unadapted DDS type, obtained from Topic. */
    private MetaType bareMetaType;

    /** The key fields defined for this type in the DDS IDL. */
    private String[] keyFields;

    /** The key fields defined for this type in the adapted type. */
    private String[] adaptedKeyFields;

    private TypeInfo (String typeName) {
        name = typeName;
        data_representation_id = -1;
        keyFields = new String[0];
        adaptedKeyFields = new String[0];
        typeEvolutions = new ArrayList<TypeEvolution>();
        types.add(this);
    }

    /**
     * A TypeEvolution is a representation of a single concrete type descriptor
     * associated with this TypeInfo. Each TypeEvolution corresponds to a single
     * received DCPSType instance.
     */
    public static class TypeEvolution implements Comparable<TypeEvolution> {

        private final TypeInfo type;
        private final String type_hash;
        private final byte[] meta_data;
        private final byte[] extensions;
        private final ComparableTime writeTime;
        private MetaType convertedType;

        private TypeEvolution (Sample dcpsTypeSample, TypeInfo typeInfo) {
            this.type = typeInfo;
            UserData dcpsTypeData = dcpsTypeSample.getMessage().getUserData();

            type_hash = dcpsTypeData.getFieldValue("type_hash.msb") +
                    dcpsTypeData.getFieldValue("type_hash.lsb");
            meta_data = CmDataUtil.getByteSequence(dcpsTypeData, "meta_data");
            extensions = CmDataUtil.getByteSequence(dcpsTypeData, "extensions");
            writeTime = new ComparableTime((int)dcpsTypeSample.getMessage().getWriteTimeSec(),
                    (int)dcpsTypeSample.getMessage().getWriteTimeNanoSec());
        }

        /**
         * Get the TypeInfo.
         * @return the typeinfo
         */
        public TypeInfo getTypeInfo () {
            return type;
        }

        /**
         * Get the type hash of the type.
         * @return the typegetMetaType hash of the type
         */
        public String getTypeHash () {
            return type_hash;
        }

        /**
         * Get the meta data of the type.
         * @return the meta data of the type
         */
        public byte[] getMetaData () {
            return meta_data;
        }

        /**
         * Get the extensions of the type.
         * @return the extensions of the type
         */
        public byte[] getExtensions () {
            return extensions;
        }

        /**
         * Get the Time at which the type was registered in the system.
         * @return the Time of type registration
         */
        public ComparableTime getWriteTime () {
            return writeTime;
        }

        /**
         * Get the MetaType for this evolution of the data type.
         * @return the converted MetaType that represents this TypeEvolution
         * @throws CmDataException
         *          If an error occurs while adapting the data.
         */
        public MetaType getMetaType () throws CmDataException {
            if (convertedType == null) {
                switch (type.getDataRepresentationId()) {
                case GPB_DATA_ID:
                    if (ProtobufDataAdapterFactory.getInstance().isEnabled()) {
                        convertedType = ProtobufDataAdapterFactory.getInstance()
                                .constructMetaType(this);
                    } else {
                        convertedType = type.bareMetaType;
                    }
                case OSPL_DATA_ID:
                default:
                }
            }
            return convertedType;
        }

        /**
         * Adapt the UserData contained in a DDS Sample obtained from a DataReader
         * into a readable format.
         *
         * @param readSample
         *          the DDS sample whose contained UserData needs to be adapted.
         * @return The same Sample object that was provided in the readSample parameter,
         *         but with its contained UserData object replaced with the adapted UserData.
         * @throws CmDataException
         *          If an error occurs while adapting the data.
         */
        public Sample adaptDataForRead(Sample readSample) throws CmDataException {
            if (type.getDataRepresentationId() == TypeInfo.GPB_DATA_ID
                    && ProtobufDataAdapterFactory.getInstance().isEnabled()) {
                /*
                 * If we're dealing with a Google Protocol Buffer sample, then
                 * convert and replace the UserData.
                 */
                readSample.getMessage().setUserData(
                        ProtobufDataAdapterFactory.getInstance()
                                .convertUserData(
                                        readSample.getMessage().getUserData(),
                                        this,
                                        readSample.getMessage().getNodeState()
                                                .test(State.WRITE)));
            }
            return readSample;
        }

        /**
         * Adapt UserData into a format suitable to be passed into a DDS Writer.
         *
         * @param writeData
         *          the UserData that needs to be adapted.
         * @return A new UserData object with the data fields adapted.
         * @throws CmDataException
         *          If an error occurs while adapting the data.
         */
        public UserData adaptDataForWrite(UserData writeData) throws CmDataException {
            if (type.getDataRepresentationId() == TypeInfo.GPB_DATA_ID
                    && ProtobufDataAdapterFactory.getInstance().isEnabled()) {
                /*
                 * If we're dealing with a Google Protocol Buffer sample, then
                 * convert and replace the UserData.
                 */
                UserData convertedData = ProtobufDataAdapterFactory
                        .getInstance().buildProtobufUserData(writeData, this,
                                true);
                return convertedData;
            }
            return writeData;
        }

        /**
         * Adapt UserData into a format suitable to be passed into a DDS Writer. When adapting
         * for dispose, only the key fields are adapted, the rest of the data is discarded.
         *
         * @param writeData
         *          the UserData that needs to be adapted.
         * @return A new UserData object with the data fields adapted.
         * @throws CmDataException
         *          If an error occurs while adapting the data.
         */
        public UserData adaptDataForDispose(UserData writeData) throws CmDataException {
            if (type.getDataRepresentationId() == TypeInfo.GPB_DATA_ID
                    && ProtobufDataAdapterFactory.getInstance().isEnabled()) {
                /*
                 * If we're dealing with a Google Protocol Buffer sample, then
                 * convert and replace the UserData.
                 */
                UserData convertedData = ProtobufDataAdapterFactory
                        .getInstance().buildProtobufUserData(writeData, this,
                                false);
                return convertedData;
            }
            return writeData;
        }

        /**
         * Compares this TypeEvolution with another. Comparison is done by
         * comparing the original Sample write time of each TypeEvolution
         *
         * @return The negated value of
         *         {@link ComparableTime#compareTo(ComparableTime)}. The value is
         *         negated for sorting purposes so that the most recent type
         *         evolution is first.
         */
        @Override
        public int compareTo (TypeEvolution o) {
            if (equals(o))
            {
                return 0;
            }
            return writeTime.compareTo(o.getWriteTime()) * -1;
        }

        @Override
        public boolean equals (Object o) {
            if (o instanceof TypeEvolution) {
                return type_hash.equals(((TypeEvolution) o).getTypeHash());
            }
            return false;
        }

        @Override
        public int hashCode () {
            return type_hash.hashCode();
        }
    }

    /**
     * Set the attributes of a type based on a received DCPSType. This adds a new TypeEvolution for
     * this TypeInfo.
     * @param dcpsTypeSample
     *            the Sample of the DCPSType
     */
    public void setData (Sample dcpsTypeSample) {
        UserData dcpsTypeData = dcpsTypeSample.getMessage().getUserData();
        name = dcpsTypeData.getFieldValue("name");
        data_representation_id = Short.parseShort(
                dcpsTypeData.getFieldValue("data_representation_id"));
        synchronized (typeEvolutions) {
            // Check for already existing evolution with type_hash
            String typeHash = dcpsTypeData.getFieldValue("type_hash.msb")
                    + dcpsTypeData.getFieldValue("type_hash.lsb");
            for (TypeEvolution typeEvo : typeEvolutions) {
                if (typeEvo.type_hash.equals(typeHash)) {
                    return;
                }
            }
            typeEvolutions.add(new TypeEvolution(dcpsTypeSample, this));
            Collections.sort(typeEvolutions);
        }
    }

    /**
     * Get the name of the type.
     * @return the name of the type
     */
    public String getTypeName () {
        return name;
    }

    /**
     * Get the data_representation_id of the type.
     * @return the data_representation_id of the type
     */
    public short getDataRepresentationId () {
        return data_representation_id;
    }

    /**
     * Get the most recent evolution of the type, according to the original
     * sample write time.
     *
     * @return the most recent TypeEvolution object, or null if no evolutions
     *         exist.
     */
    public TypeEvolution getMostRecentEvolution () {
        synchronized (typeEvolutions) {
            if (hasEvolutions()) {
                return typeEvolutions.get(0);
            }
        }
        return null;
    }

    /**
     * Get the List of all known evolutions of this data type.
     *
     * @return a copy of the list of known TypeEvolutions to exist in
     *         the system
     */
    public List<TypeEvolution> getAllTypeEvolutions () {
        synchronized (typeEvolutions) {
            return new ArrayList<TypeEvolution>(typeEvolutions);
        }
    }

    /**
     * Determines if this TypeInfo has any known data evolutions.
     *
     * @return true if there exists the number of TypeEvolutions for this
     *         TypeInfo is non-zero.
     */
    public boolean hasEvolutions () {
        synchronized (typeEvolutions) {
            return !typeEvolutions.isEmpty();
        }
    }

    /**
     * Gets the MetaType associated with this TypeInfo. If this TypeInfo has
     * TypeEvolutions, then it will return the MetaType obtained from the most recently
     * registered TypeEvolution's decoded meta data. Otherwise, it will return the original
     * Topic MetaType.
     * @return the MetaType for the most recent TypeEvolution, or if no evolutions, the bare MetaType
     * @throws CmDataException Thrown when retrieving the adapted MetaType fails.
     */
    public MetaType getMetaType () throws CmDataException {
        synchronized (typeEvolutions) {
            if (hasEvolutions()) {
                return getMostRecentEvolution().getMetaType();
            } 
        }
        return bareMetaType;
    }

    /**
     * Gets the MetaType associated with this TypeInfo's TypeEvolution. It will return
     * the MetaType obtained from the TypeEvolution's decoded meta data. Otherwise,
     * it will return the original Topic MetaType.
     * @param typeEvolution
     *         the DDS sample whose contained UserData needs to be adapted.
     * @return the MetaType for this type
     * @throws CmDataException Thrown when retrieving the adapted MetaType fails.
     */
    public MetaType getMetaType (TypeEvolution typeEvolution) throws CmDataException {
        if (typeEvolution != null) {
            return typeEvolution.getMetaType();
        }
        return bareMetaType;
    }

    /**
     * Gets the original MetaType associated with this TypeInfo.
     * @return the MetaType for this type
     */
    public MetaType getBareMetaType () {
        return bareMetaType;
    }

    /**
     * Get the list of key fields defined for this type. If this type is a
     * Google Protocol Buffer type, then the keys are acquired from the proto
     * definition, rather than from the IDL definition.
     *
     * @return the list of key fields
     * @throws CmDataException Thrown when adapting the key fields from the type meta data fails.
     */
    public String[] getKeys() throws CmDataException {
        if (adaptedKeyFields.length != 0) {
            return Arrays.copyOf(adaptedKeyFields, adaptedKeyFields.length);
        } else {
            TypeEvolution typeEvo = null;
            synchronized (typeEvolutions) {
                typeEvo = getMostRecentEvolution();
            }
            if (typeEvo != null) {
                switch (getDataRepresentationId()) {
                case GPB_DATA_ID:
                    if (ProtobufDataAdapterFactory.getInstance().isEnabled()) {
                        adaptedKeyFields = ProtobufDataAdapterFactory
                                .getInstance().getKeys(typeEvo);
                        return Arrays.copyOf(adaptedKeyFields, adaptedKeyFields.length);
                    }
                case OSPL_DATA_ID:
                default:
                }
            }
        }
        return Arrays.copyOf(keyFields, keyFields.length);
    }

    /**
     * Set the original MetaType and keyfields of the TypeInfo, obtained from the CM Topic instance.
     * @param topic the CM Topic instance
     * @throws CmDataException Thrown when retrieving the MetaType from the topic parameter fails.
     */
    public void setBareTopicType (Topic topic) throws CmDataException {
        if (bareMetaType == null) {
            try {
                bareMetaType = topic.getDataType();
            } catch (DataTypeUnsupportedException e) {
                throw new CmDataException("Error occurred while getting Topic type.", e);
            } catch (CMException e) {
                throw new CmDataException("Error occurred while getting Topic type.", e);
            }
            String keys = topic.getKeyList() != null ? topic.getKeyList() : "";
            StringTokenizer tok = new StringTokenizer(keys, ",");
            int keyListSize = tok.countTokens();
            keyFields = new String[keyListSize];
            for (int i = 0; i < keyListSize; i++) {
                keyFields[i] = tok.nextToken();
            }
        }
    }

    /**
     * Set the original MetaType and keyfields of the TypeInfo.
     * @param bareType the original MetaType instance obtained from CM API Topic
     * @param keyFields the original key fields obtained from CM API Topic
     */
    public void setBareTopicType (MetaType bareMetaType, String[] keyFields) {
        this.bareMetaType = bareMetaType;
        this.keyFields = keyFields;
    }

    /**
     * Adapt the UserData contained in a DDS Sample obtained from a DataReader
     * into a readable format.
     *
     * @param typeEvolution
     *          the type evolution to use when adapting the data. Can be null.
     * @param readSample
     *          the DDS sample whose contained UserData needs to be adapted.
     * @return The same Sample object that was provided in the readSample parameter,
     *         but with its contained UserData object replaced with the adapted UserData.
     *         If the provided typeEvolution parameter was null, readSample is returned unaltered.
     * @throws CmDataException
     *          If an error occurs while adapting the data.
     */
    public Sample adaptDataForRead(TypeEvolution typeEvolution, Sample readSample)
            throws CmDataException {
        if (typeEvolution != null) {
            return typeEvolution.adaptDataForRead(readSample);
        }
        return readSample;
    }

    /**
     * Adapt UserData into a format suitable to be passed into a DDS Writer.
     *
     * @param typeEvolution
     *          the type evolution to use when adapting the data. Can be null.
     * @param writeData
     *          the UserData that needs to be adapted.
     * @return A new UserData object with the data fields adapted. If the provided
     *         typeEvolution parameter was null, writeData is returned unaltered.
     * @throws CmDataException
     *          If an error occurs while adapting the data.
     */
    public UserData adaptDataForWrite(TypeEvolution typeEvolution, UserData writeData)
            throws CmDataException {
        if (typeEvolution != null) {
            return typeEvolution.adaptDataForWrite(writeData);
        }
        return writeData;
    }

    /**
     * Adapt UserData into a format suitable to be passed into a DDS Writer. When adapting
     * for dispose, only the key fields are adapted, the rest of the data is discarded.
     *
     * @param typeEvolution
     *          the type evolution to use when adapting the data. Can be null.
     * @param writeData
     *          the UserData that needs to be adapted.
     * @return A new UserData object with the data fields adapted. If the provided
     *         typeEvolution parameter was null, writeData is returned unaltered.
     * @throws CmDataException
     *          If an error occurs while adapting the data.
     */
    public UserData adaptDataForDispose(TypeEvolution typeEvolution, UserData writeData)
            throws CmDataException {
        if (typeEvolution != null) {
            return typeEvolution.adaptDataForDispose(writeData);
        }
        return writeData;
    }

    /**
     * Get the type by type name.
     *
     * @param typeName
     *            the name of the type to find
     * @return the TypeInfo for type name, or if not found, a new and
     *         empty TypeInfo.
     */
    public static TypeInfo getTypeInfoByName (String typeName) {
        synchronized (types) {
            for (TypeInfo typeInfo : types) {
                if (typeInfo.getTypeName().equals(typeName)) {
                    return typeInfo;
                }
            }
        }
        return new TypeInfo(typeName);
    }

    /**
     * Get the type by Topic.
     *
     * @param topic
     *            the Topic with which to get or create the TypeInfo with.
     * @return the TypeInfo for type name, or if not found either a new TypeInfo
     *         object initialized with the Topic
     * @throws CmDataException
     *            If an exception is thrown when trying to get the MetaType from the Topic.
     */
    public static TypeInfo getTypeInfoByTopic (Topic topic) throws CmDataException {
        TypeInfo typeInfo = getTypeInfoByName(topic.getTypeName());
        typeInfo.setBareTopicType(topic);
        return typeInfo;
    }
}

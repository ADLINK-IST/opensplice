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
package org.opensplice.cmdataadapter.protobuf;

import org.opensplice.cm.data.UserData;
import org.opensplice.cm.meta.MetaType;
import org.opensplice.cmdataadapter.CmDataException;
import org.opensplice.cmdataadapter.TypeInfo.TypeEvolution;

/**
 * The ProtobufDataAdapter is a utility class that is meant for
 * transparently translating UserData and MetaType objects from DDS Topics
 * encoded with Google Protocol Buffers.
 */
public interface ProtobufDataAdapter {

    /**
     * Convert a UserData object constructed by Tester, to a UserData object
     * with all fields serialized to the protobuf byte data, such that the
     * UserData is suitable to be written out to the protobuf Topic.
     * 
     * @param userData
     *            The fully formed UserData object that was constructed by
     *            Tester.
     * @param typeEvolution
     *            A TypeEvolution object that contains the meta data necessary
     *            to construct a MetaType instance. It must have a data
     *            representation id of {@link TypeInfo#GPB_DATA_ID}.
     * @return A new, converted UserData object, that has the key field and
     *         filterable fields as regular fields, and the rest of the data as
     *         a byte sequence. Suitable for passing to the DataWriter.
     */
    public UserData buildProtobufUserData (UserData userData, TypeEvolution typeEvolution, boolean validData) throws CmDataException;

    /**
     * Convert a UserData object read in from a Google Protocol Buffer Topic, to
     * a UserData object with all fields deserialized from the protobuf byte
     * data, such that the UserData appears to have been read in from a normal
     * DDS Topic.
     *
     * @param userData
     *            The object that was contained in the Sample read in by the
     *            DataReader of a Google Protocol Buffer Topic.
     * @param typeEvolution
     *            A TypeEvolution object that contains the meta data necessary to
     *            construct a MetaType instance. It must have a data
     *            representation id of {@link TypeInfo#GPB_DATA_ID}.
     * @return A new, converted UserData object, that has all field names and
     *         field values contained in it, derived from deserializing the
     *         Google Protocol Buffer byte sequence from the input UserData.
     */
    public UserData convertUserData (UserData userData, TypeEvolution typeEvolution, boolean validData) throws CmDataException;

    /**
     * Get the key fields for this protobuf type.
     *
     * @param typeEvolution
     *            A TypeEvolution object that contains the meta data necessary
     *            to retrieve the protobuf key fields. It must have a data
     *            representation id of {@link TypeInfo#GPB_DATA_ID}.
     * @return a String array containing the key field names for this protobuf
     *         type
     */
    public String[] getKeys (TypeEvolution typeEvolution) throws CmDataException;

    /**
     * Construct a MetaType object based on the byte sequence meta data
     * contained in the supplied TypeEvolution parameter, such that the MetaType
     * appears to represent a normal DDS Topic type.
     *
     * @param typeEvolution
     *            A TypeEvolution object that contains the meta data necessary to
     *            construct a MetaType instance. It must have a data
     *            representation id of {@link TypeInfo#GPB_DATA_ID}.
     * @return A new MetaType object, that describes the data types that was in
     *         the protobuf meta data.
     */
    public MetaType constructMetaType (TypeEvolution typeEvolution) throws CmDataException;

    /**
     * Construct a description of the data type based on the proto information
     * in the Protobuf Descriptor.
     *
     * @param typeEvolution
     *            A TypeEvolution object that contains the meta data necessary.
     *            It must have a data representation id of
     *            {@link TypeInfo#GPB_DATA_ID}.
     * @return The String containing the proto information.
     */
    public String getDescriptorProto (TypeEvolution typeEvolution) throws CmDataException;

    /**
     * Find the extra protobuf specific properties for the requested field.
     *
     * @param fullFieldName
     *            The field name for which to find the default value for. It is
     *            expected to be of the CM UserData field name format. eg.
     *            "collectionStructs[0].primitive"
     * @param typeEvolution
     *            A TypeEvolution object that contains the meta data necessary to
     *            acquire the values. It must have a data representation id of
     *            {@link TypeInfo#GPB_DATA_ID}.
     *
     * @return The {@link ProtobufDataAdapter#ProtobufFieldProperties} object that contains the field properties.
     */
    public ProtobufFieldProperties getFieldProperties (String fullFieldName, TypeEvolution typeEvolution) throws CmDataException;

    /**
     * This method checks whether the protobuf feature is enabled.
     *
     * If this build of Tester was not included with the protobuf dependency,
     * then the protobuf feature is disabled, and any calls to the other
     * interface methods will throw a ProtobufDataAdapterException.
     *
     * @return true, if protobuf dependencies are included, thus the
     *         implementing class is fully implemented. false, if protobuf
     *         dependencies are not included, thus the implementing class
     *         remains unimplemented.
     */
    public boolean isEnabled ();
}

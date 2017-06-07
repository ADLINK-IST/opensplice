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
package org.opensplice.cm.transform;

import org.opensplice.cm.Storage.Result;
import org.opensplice.cm.meta.MetaType;
import org.opensplice.cm.DataTypeUnsupportedException;
import org.opensplice.cm.data.UserData;

/**
 * Interface that must be implemented by each class that offers facilities
 * for deserializing a Status and that wants be supported by the
 * DataTransformerFactory.
 *
 * @date Oct 13, 2004
 */
public interface StorageDeserializer {
    public Object deserializeStorage(Object serialized) throws TransformationException;
    public Result deserializeStorageResult(Object serialized) throws TransformationException;
    public Result deserializeOpenResult_Result(Object serialized) throws TransformationException;
    public Object deserializeOpenResult_Storage(Object serialized) throws TransformationException;
    public Result deserializeReadResult_Result(Object serialized) throws TransformationException;
    public String deserializeReadResult_DataTypeName(Object serialized) throws TransformationException;
    public UserData deserializeReadResult_Data(Object serialized, MetaType type) throws TransformationException;
    public MetaType deserializeGetTypeResult_Metadata(Object serialized) throws TransformationException, DataTypeUnsupportedException;
}

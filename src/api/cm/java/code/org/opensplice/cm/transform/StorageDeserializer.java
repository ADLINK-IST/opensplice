/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
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

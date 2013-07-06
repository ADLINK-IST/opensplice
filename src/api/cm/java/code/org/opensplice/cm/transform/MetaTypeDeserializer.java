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

import org.opensplice.cm.DataTypeUnsupportedException;
import org.opensplice.cm.meta.*;

/**
 * Interface that must be implemented by each class that offers facilities
 * for deserializing a MetaType and that wants be supported by the 
 * DataTransformerFactory.
 * 
 * @date May 25, 2004
 */
public interface MetaTypeDeserializer{
    /**
     * Deserializes the supplied type from a serialized representation to a
     * MetaType object.
     * 
     * @param type The object tot deserialize.
     * @return The deserialized type.
     */
    public MetaType deserializeMetaType(Object type) throws TransformationException, DataTypeUnsupportedException;
}

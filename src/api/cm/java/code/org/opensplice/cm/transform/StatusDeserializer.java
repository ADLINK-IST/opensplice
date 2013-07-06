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

import org.opensplice.cm.status.Status;

/**
 * Interface that must be implemented by each class that offers facilities
 * for deserializing a Status and that wants be supported by the 
 * DataTransformerFactory.
 *  
 * @date Oct 13, 2004 
 */
public interface StatusDeserializer {
    /**
     * Deserializes the supplied object from a serialized representation to a
     * Status object.
     *  
     * @param serialized The serialized representation of a Status.
     * @return The Java Status object that matches the supplied serialized data. 
     */
    public Status deserializeStatus(Object serialized) throws TransformationException;
}

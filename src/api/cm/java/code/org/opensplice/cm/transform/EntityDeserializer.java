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

import org.opensplice.cm.Entity;



/**
 * Interface that must be implemented by all concrete entity deserializers that 
 * want to be supported by the DataTransformerFactory. Deserializers that 
 * implement this interface must be capable of transforming data with a 
 * specific representation to a Java Entity object.
 * 
 * @date Aug 27, 2004 
 */
public interface EntityDeserializer {
    
    /**
     * Deserializes the supplied object into an Entity object.
     * 
     * @param serialized The serialized data.
     * @return The deserialized entity.
     */
    public Entity deserializeEntity(Object serialized) throws TransformationException;
    
    /**
     * Same as the deserializeEntity except it deserializes a list of serialized 
     * entities into an array of Entity objects.
     *  
     * @param serialized The list of serialized entities.
     * @return An array of deserialized Entity objects.
     */
    public Entity[] deserializeEntityList(Object serialized) throws TransformationException;
}

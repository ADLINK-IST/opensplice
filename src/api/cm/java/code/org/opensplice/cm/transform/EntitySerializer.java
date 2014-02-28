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
 * Interface that must be implemented by all concrete entity serializers that 
 * want to be supported by the DataTransformerFactory. Serializers that
 * implement this interface must be capable of transforming a Java Entity 
 * object to a serialized representation.
 *  
 * @date Sep 17, 2004
 */
public interface EntitySerializer {
    
    /**
     * Serializes the supplied Entity into a serialized representation.
     * 
     * @param e The entity that needs to be serialized
     * @return The serialized entity.
     */
    public String serializeEntity(Entity e) throws TransformationException;
    
    /**
     * Serializes the supplied Entities into a serialized representation.
     * 
     * @param e The entities that needs to be serialized
     * @return The serialized entities.
     */
    public String serializeEntities(Entity[] e) throws TransformationException;
}

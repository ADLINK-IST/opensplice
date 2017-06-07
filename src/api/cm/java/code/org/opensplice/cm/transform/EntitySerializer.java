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

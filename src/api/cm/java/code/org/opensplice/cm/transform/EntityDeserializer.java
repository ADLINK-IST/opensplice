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

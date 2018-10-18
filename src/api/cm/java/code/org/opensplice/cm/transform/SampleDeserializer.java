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

import org.opensplice.cm.data.Sample;
import org.opensplice.cm.meta.MetaType;

/**
 * Interface that must be implemented by each class that offers facilities
 * for deserializing a Sample and that wants be supported by the 
 * DataTransformerFactory.
 * 
 * @date May 14, 2004
 */
public interface SampleDeserializer {
    /**
     * Deserializes the supplied serialized Sample into a Sample object.
     * 
     * @param serializedSample The serialized Sample.
     * @return The deserialized Sample object.
     */
    public Sample deserializeSample(Object serializedSample) throws TransformationException;
    
    /**
     * Deserializes the supplied serialized Sample into a Sample object 
     * according to the supplied type.
     * 
     * @param serializedSample The serialized Sample.
     * @param type The type of the UserData whithin the Sample.
     * @return The deserialized Sample object.
     */
    public Sample deserializeSample(Object serializedSample, MetaType type) throws TransformationException;
}

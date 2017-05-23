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

import org.opensplice.cm.data.Sample;

/**
 * Interface that must be implemented by each class that offers facilities
 * for erializing a Sample and that wants be supported by the 
 * DataTransformerFactory.
 * 
 * @date Mar 31, 2005 
 */
public interface SampleSerializer {
    /**
     * Serializes the supplied Sample to its serialized representation.
     * 
     * @param sampl The sample to serialize.
     * @throws TransformationException Thrown when the supplied Sample is null.
     * @return The serialized String representation of the supplied Sample.
     */
    public String serializeSample(Sample sample) throws TransformationException;
}

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

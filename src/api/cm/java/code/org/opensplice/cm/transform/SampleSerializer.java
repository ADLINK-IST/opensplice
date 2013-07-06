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

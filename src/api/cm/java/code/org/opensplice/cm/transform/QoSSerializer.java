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

import org.opensplice.cm.qos.QoS;

/**
 * Interface that must be implemented by all concrete QoS serializers that 
 * want to be supported by the DataTransformerFactory. Serializers that
 * implement this interface must be capable of transforming a Java QoS 
 * representation into a serialized String representation.
 * 
 * @date Feb 1, 2005 
 */
public interface QoSSerializer {
    /**
     * Serializes the supplied QoS to its serialized representation.
     * 
     * @param qos The quality of service to serialize.
     * @throws TransformationException Thrown when the supplied QoS is null.
     * @return The serialized String representation of the supplied QoS.
     */
    public String serializeQoS(QoS qos) throws TransformationException;
}

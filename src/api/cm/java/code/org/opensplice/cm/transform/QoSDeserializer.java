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
 * Interface that must be implemented by all concrete QoS deserializers that 
 * want to be supported by the DataTransformerFactory. Deserializers that
 * implement this interface must be capable of transforming a serialized 
 * representation of an object to a Java QoS representation. 
 * 
 * @date Jan 10, 2005 
 */
public interface QoSDeserializer {
    
    /**
     * Deserializes the supplied QoS from a serialized representation to a
     * QoS object.
     * 
     * @param qos The object tot deserialize.
     * @return The deserialized QoS.
     */
    public QoS deserializeQoS(Object qos) throws TransformationException;
}

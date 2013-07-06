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
package org.opensplice.cm.qos;

import org.opensplice.cm.transform.DataTransformerFactory;
import org.opensplice.cm.transform.QoSSerializer;
import org.opensplice.cm.transform.TransformationException;

/**
 * Abstract base class for Quality of Service of a specific Entity. It has been
 * defined abstract because only descendants of this class may actually exist.
 * This class provides a generalization for a Quality of Service on Entity 
 * level. 
 * 
 * @date Jan 10, 2005 
 */
public abstract class QoS {
    @Override
    public boolean equals(Object obj){
        if(obj instanceof QoS){
            QoSSerializer serializer = DataTransformerFactory.getQoSSerializer(DataTransformerFactory.XML);
            String xmlQos, xmlQos2;
            
            try {
                xmlQos = serializer.serializeQoS(this);
                xmlQos2 = serializer.serializeQoS((QoS)obj);
                
                if(xmlQos.equals(xmlQos2)){
                    return true;
                }
            } catch (TransformationException e) {}
        }
        return false;
    }

    @Override
    public int hashCode() {
        return 0;
    }
}

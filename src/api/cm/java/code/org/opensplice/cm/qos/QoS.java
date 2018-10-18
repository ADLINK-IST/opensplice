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

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
 */
package org.opensplice.cm.transform.xml;

import org.opensplice.cm.qos.QoS;
import org.opensplice.cm.transform.QoSSerializer;
import org.opensplice.cm.transform.TransformationException;

public class QoSSerializerXMLPreV6_6 implements QoSSerializer {
    private QoSSerializerXML serializer;

    public QoSSerializerXMLPreV6_6() {
        this.serializer = new QoSSerializerXML();
    }

    @Override
    public String serializeQoS(QoS qos) throws TransformationException {
        return serializer.serializeQoSLegacyMode(qos);
    }
}

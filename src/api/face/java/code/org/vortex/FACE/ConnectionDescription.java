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
package org.vortex.FACE;

import java.io.File;
import java.io.IOException;
import java.util.HashSet;

import javax.xml.XMLConstants;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamSource;
import javax.xml.validation.Schema;
import javax.xml.validation.SchemaFactory;
import javax.xml.validation.Validator;

import org.omg.dds.core.QosProvider;
import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.domain.DomainParticipantQos;
import org.omg.dds.pub.DataWriterQos;
import org.omg.dds.pub.PublisherQos;
import org.omg.dds.sub.DataReaderQos;
import org.omg.dds.sub.SubscriberQos;
import org.omg.dds.topic.TopicQos;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

import FACE.CONNECTION_DIRECTION_TYPE;

public class ConnectionDescription {
    /* file in the root for the ddsface jar */
    private static final String SCHEMA_PATH = "vortex_face.xsd";
    private String name;
    private long id;
    private int domainId;
    private CONNECTION_DIRECTION_TYPE direction;
    private long platformViewGuid;
    private long refreshPeriod;
    private String topicName;
    private String typeName;
    private ServiceEnvironment env;
    private HashSet<ConnectionDescription> configurations = new HashSet<ConnectionDescription>();
    private QosConfiguration qos = null;


    public ConnectionDescription(ServiceEnvironment env, String configurationUri)
            throws ConfigurationException {

        Schema schema = null;
        try {
          String language = XMLConstants.W3C_XML_SCHEMA_NS_URI;
          SchemaFactory factory = SchemaFactory.newInstance(language);
          schema = factory.newSchema(new StreamSource(this.getClass().getClassLoader().getResourceAsStream(SCHEMA_PATH)));
        } catch (Exception e) {
            throw new ConfigurationException(e.getMessage());
        }

        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        factory.setSchema(schema);
        factory.setNamespaceAware(true);
        DocumentBuilder builder;
        String name;
        String type;
        long id;
        int domainId;
        CONNECTION_DIRECTION_TYPE direction = null;
        long platformViewGuid;
        long refreshPeriod;
        String topicName;
        String typeName;

        try {
            builder = factory.newDocumentBuilder();

            Document document = builder.parse(new File(configurationUri));
            Validator validator = schema.newValidator();
            validator.validate(new DOMSource(document));

            NodeList nList = document.getElementsByTagName("connection");
            for (int temp = 0; temp < nList.getLength(); temp++) {
                Node nNode = nList.item(temp);
                if (nNode.getNodeType() == Node.ELEMENT_NODE) {
                    Element eElement = (Element) nNode;
                    type = this.getStringConfig(eElement, "type");

                    if (!"DDS".equals(type)) {
                        throw new ConfigurationException("Only 'DDS' type supported (found '" + type + "')");
                    }

                    String dir = this.getStringConfig(eElement, "direction");

                    if ("SOURCE".equals(dir)) {
                        direction = CONNECTION_DIRECTION_TYPE.SOURCE;
                    } else if ("DESTINATION".equals(dir)) {
                        direction = CONNECTION_DIRECTION_TYPE.DESTINATION;
                    } else {
                        throw new ConfigurationException(
                                "Only SOURCE and DESTINATION connections are supported (found: '"
                                        + dir + "').");
                    }
                    NodeList qosList = eElement.getElementsByTagName("qos");
                    QosConfiguration qos = null;
                    if (qosList.getLength() == 0) {
                        qos = new QosConfiguration(env, null, null, null, null, null, null, null, null);
                    } else {
                        Element qosElement = (Element) qosList.item(0);
                        String qosUri = this.getStringConfig(qosElement, "uri");
                        String qosProfile = this.getStringConfig(qosElement, "profile");
                        String domainparticipant_qos_id = this.getStringConfig(qosElement, "domainparticipant_qos_id", true);
                        String topic_qos_id= this.getStringConfig(qosElement, "topic_qos_id", true);
                        String publisher_qos_id = this.getStringConfig(qosElement, "publisher_qos_id", true);
                        String datawriter_qos_id = this.getStringConfig(qosElement, "datawriter_qos_id", true);
                        String subscriber_qos_id = this.getStringConfig(qosElement, "subscriber_qos_id", true);
                        String datareader_qos_id = this.getStringConfig(qosElement, "datareader_qos_id", true);
                        qos = new QosConfiguration(env, qosUri, qosProfile, domainparticipant_qos_id, topic_qos_id, publisher_qos_id, datawriter_qos_id, subscriber_qos_id, datareader_qos_id);
                    }
                    name = this.getStringConfig(eElement, "name");
                    id = this.hashCode();
                    platformViewGuid = this.getLongConfig(eElement, "platform_view_guid");
                    refreshPeriod = this.getLongConfig(eElement, "refresh_period");
                    domainId = this.getIntConfig(eElement, "domain_id", DDS.DOMAIN_ID_DEFAULT.value);
                    topicName = this.getStringConfig(eElement, "topic_name");
                    typeName = this.getStringConfig(eElement, "type_name");
                    ConnectionDescription config = new ConnectionDescription(env,name,id,domainId,qos,direction,platformViewGuid,refreshPeriod,topicName,typeName);
                    configurations.add(config);
                }
            }
        } catch (ParserConfigurationException e) {
            throw new ConfigurationException(e.getMessage());
        } catch (SAXException e) {
            throw new ConfigurationException(e.getMessage());
        } catch (IOException e) {
            throw new ConfigurationException(e.getMessage());
        }
    }

    private long getLongConfig(Element element, String name)
            throws ConfigurationException {
        String value = this.getStringConfig(element, name);
        try {
            return Long.parseLong(value);
        } catch (NumberFormatException e) {
            throw new ConfigurationException(e.getMessage());
        }
    }

    private int getIntConfig(Element element, String name, int defaultValue)
            throws ConfigurationException {
        String value = this.getStringConfig(element, name, true);
        if (value == null) {
            return defaultValue;
        } else {
            try {
                return Integer.parseInt(value);
            } catch (NumberFormatException e) {
                throw new ConfigurationException(e.getMessage());
            }
        }
    }

    private String getStringConfig(Element element, String name, boolean optional)
            throws ConfigurationException {
        String result = null;
        NodeList list = element.getElementsByTagName(name);

        if (list.getLength() != 1 && !optional) {
            throw new ConfigurationException("No element '" + name + "' found.");
        } else {
            if (list.getLength() == 1) {
                Node child = list.item(0).getFirstChild();

                if (child == null) {
                    throw new ConfigurationException("Element '" + name
                            + "' has no content.");
                }
                result = child.getNodeValue();
            }
        }
        return result;
    }

    private String getStringConfig(Element element, String name)
            throws ConfigurationException {
        NodeList list = element.getElementsByTagName(name);

        if (list.getLength() != 1) {
            throw new ConfigurationException("No element '" + name + "' found.");
        }
        Node child = list.item(0).getFirstChild();

        if (child == null) {
            throw new ConfigurationException("Element '" + name
                    + "' has no content.");
        }
        return child.getNodeValue();

    }

    public ConnectionDescription(ServiceEnvironment env, String name, long id,
            int domainId, QosConfiguration qos,
            CONNECTION_DIRECTION_TYPE direction, long platformViewGuid,
            long refreshPeriod, String topicName, String typeName) {
        this.name = name;
        this.id = id;
        this.domainId = domainId;
        this.direction = direction;
        this.platformViewGuid = platformViewGuid;
        this.refreshPeriod = refreshPeriod;
        this.topicName = topicName;
        this.typeName = typeName;
        this.qos = qos;
        this.env = env;
    }

    public QosConfiguration getQos() {
        return qos;
    }

    public void setQos(QosConfiguration qos) {
        this.qos = qos;
    }

    public HashSet<ConnectionDescription> getConfigurations() {
        return configurations;
    }

    public ServiceEnvironment getEnvironment() {
        return this.env;
    }

    public String getName() {
        return this.name;
    }

    public long getId() {
        return this.id;
    }

    public DomainParticipantQos getDomainParticipantQos() {
        if (this.getQos().getQP() == null) {
            return null;
        }
        return this.getQos().getDomainparticipant_qos_id() != null
                ? this.getQos().getQP().getDomainParticipantQos(this.getQos().getDomainparticipant_qos_id())
                : this.getQos().getQP().getDomainParticipantQos();
    }

    public PublisherQos getPublisherQos() {
        if (this.getQos().getQP() == null) {
            return null;
        }
        return this.getQos().getQP().getPublisherQos(this.getQos().getPublisher_qos_id());
    }

    public SubscriberQos getSubscriberQos() {
        if (this.getQos().getQP() == null) {
            return null;
        }
        return this.getQos().getSubscriber_qos_id() != null
                ? this.getQos().getQP().getSubscriberQos(this.getQos().getSubscriber_qos_id())
                : this.getQos().getQP().getSubscriberQos();
    }

    public TopicQos getTopicQos() {
        if (this.getQos().getQP() == null) {
            return null;
        }
        return this.getQos().getTopic_qos_id() != null ? this.getQos().getQP().getTopicQos(this.getQos().getTopic_qos_id())
                : this.getQos().getQP().getTopicQos();
    }

    public DataWriterQos getDataWriterQos() {
        if (this.getQos().getQP() == null) {
            return null;
        }
        return this.getQos().getDatawriter_qos_id() != null
                ? this.getQos().getQP().getDataWriterQos(this.getQos().getDatawriter_qos_id())
                : this.getQos().getQP().getDataWriterQos();
    }

    public DataReaderQos getDataReaderQos() {
        if (this.getQos().getQP() == null) {
            return null;
        }
        return this.getQos().getDatareader_qos_id() != null
                ? this.getQos().getQP().getDataReaderQos(this.getQos().getDatareader_qos_id())
                : this.getQos().getQP().getDataReaderQos();
    }

    public int getDomainId() {
        return this.domainId;
    }

    public CONNECTION_DIRECTION_TYPE getDirection() {
        return this.direction;
    }

    public long getPlatformViewGuid() {
        return this.platformViewGuid;
    }

    public long getRefreshPeriod() {
        return this.refreshPeriod;
    }

    public String getTopicName() {
        return this.topicName;
    }

    public String getTypeName() {
        return this.typeName;
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 1;
        result = prime * result + ((configurations == null) ? 0 : configurations.hashCode());
        result = prime * result + ((direction == null) ? 0 : direction.hashCode());
        result = prime * result + domainId;
        result = prime * result + ((env == null) ? 0 : env.hashCode());
        result = prime * result + (int) (id ^ (id >>> 32));
        result = prime * result + ((name == null) ? 0 : name.hashCode());
        result = prime * result + (int) (platformViewGuid ^ (platformViewGuid >>> 32));
        result = prime * result + ((qos == null) ? 0 : qos.hashCode());
        result = prime * result + (int) (refreshPeriod ^ (refreshPeriod >>> 32));
        result = prime * result + ((topicName == null) ? 0 : topicName.hashCode());
        result = prime * result + ((typeName == null) ? 0 : typeName.hashCode());
        return result;
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj)
            return true;
        if (obj == null)
            return false;
        if (getClass() != obj.getClass())
            return false;
        ConnectionDescription other = (ConnectionDescription) obj;
        if (configurations == null) {
            if (other.configurations != null)
                return false;
        } else if (!configurations.equals(other.configurations))
            return false;
        if (direction == null) {
            if (other.direction != null)
                return false;
        } else if (!direction.equals(other.direction))
            return false;
        if (domainId != other.domainId)
            return false;
        if (env == null) {
            if (other.env != null)
                return false;
        } else if (!env.equals(other.env))
            return false;
        if (id != other.id)
            return false;
        if (name == null) {
            if (other.name != null)
                return false;
        } else if (!name.equals(other.name))
            return false;
        if (platformViewGuid != other.platformViewGuid)
            return false;
        if (qos == null) {
            if (other.qos != null)
                return false;
        } else if (!qos.equals(other.qos))
            return false;
        if (refreshPeriod != other.refreshPeriod)
            return false;
        if (topicName == null) {
            if (other.topicName != null)
                return false;
        } else if (!topicName.equals(other.topicName))
            return false;
        if (typeName == null) {
            if (other.typeName != null)
                return false;
        } else if (!typeName.equals(other.typeName))
            return false;
        return true;
    }

    public static class QosConfiguration {

        private String uri;
        private String profile;
        private String domainparticipant_qos_id;
        private String topic_qos_id;
        private String publisher_qos_id;
        private String datawriter_qos_id;
        private String subscriber_qos_id;
        private String datareader_qos_id;
        private QosProvider qp;

        public QosConfiguration(ServiceEnvironment env, String uri, String profile, String domainparticipant_qos_id, String topic_qos_id,
                String publisher_qos_id, String datawriter_qos_id, String subscriber_qos_id, String datareader_qos_id) {
            this.uri = uri;
            this.profile = profile;
            this.domainparticipant_qos_id = domainparticipant_qos_id;
            this.topic_qos_id = topic_qos_id;
            this.publisher_qos_id = publisher_qos_id;
            this.datawriter_qos_id = datawriter_qos_id;
            this.subscriber_qos_id = subscriber_qos_id;
            this.datareader_qos_id = datareader_qos_id;
            if (uri != null && profile != null) {
                this.qp = QosProvider.newQosProvider(uri, profile, env);
            } else {
                this.qp = null;
            }
        }
        public QosProvider getQP() {
            return qp;
        }
        public String getUri() {
            return uri;
        }
        public void setUri(String uri) {
            this.uri = uri;
        }
        public String getProfile() {
            return profile;
        }
        public void setProfile(String profile) {
            this.profile = profile;
        }
        public String getDomainparticipant_qos_id() {
            return domainparticipant_qos_id;
        }
        public void setDomainparticipant_qos_id(String domainparticipant_qos_id) {
            this.domainparticipant_qos_id = domainparticipant_qos_id;
        }
        public String getTopic_qos_id() {
            return topic_qos_id;
        }
        public void setTopic_qos_id(String topic_qos_id) {
            this.topic_qos_id = topic_qos_id;
        }
        public String getPublisher_qos_id() {
            return publisher_qos_id;
        }
        public void setPublisher_qos_id(String publisher_qos_id) {
            this.publisher_qos_id = publisher_qos_id;
        }
        public String getDatawriter_qos_id() {
            return datawriter_qos_id;
        }
        public void setDatawriter_qos_id(String datawriter_qos_id) {
            this.datawriter_qos_id = datawriter_qos_id;
        }
        public String getSubscriber_qos_id() {
            return subscriber_qos_id;
        }
        public void setSubscriber_qos_id(String subscriber_qos_id) {
            this.subscriber_qos_id = subscriber_qos_id;
        }
        public String getDatareader_qos_id() {
            return datareader_qos_id;
        }
        public void setDatareader_qos_id(String datareader_qos_id) {
            this.datareader_qos_id = datareader_qos_id;
        }

        @Override
        public int hashCode() {
            final int prime = 31;
            int result = 1;
            result = prime * result + ((datareader_qos_id == null) ? 0 : datareader_qos_id.hashCode());
            result = prime * result + ((datawriter_qos_id == null) ? 0 : datawriter_qos_id.hashCode());
            result = prime * result + ((domainparticipant_qos_id == null) ? 0 : domainparticipant_qos_id.hashCode());
            result = prime * result + ((profile == null) ? 0 : profile.hashCode());
            result = prime * result + ((publisher_qos_id == null) ? 0 : publisher_qos_id.hashCode());
            result = prime * result + ((subscriber_qos_id == null) ? 0 : subscriber_qos_id.hashCode());
            result = prime * result + ((topic_qos_id == null) ? 0 : topic_qos_id.hashCode());
            result = prime * result + ((uri == null) ? 0 : uri.hashCode());
            result = prime * result + ((qp == null) ? 0 : qp.hashCode());
            return result;
        }
        @Override
        public boolean equals(Object obj) {
            if (this == obj)
                return true;
            if (obj == null)
                return false;
            if (getClass() != obj.getClass())
                return false;
            QosConfiguration other = (QosConfiguration) obj;
            if (datareader_qos_id == null) {
                if (other.datareader_qos_id != null)
                    return false;
            } else if (!datareader_qos_id.equals(other.datareader_qos_id))
                return false;
            if (datawriter_qos_id == null) {
                if (other.datawriter_qos_id != null)
                    return false;
            } else if (!datawriter_qos_id.equals(other.datawriter_qos_id))
                return false;
            if (domainparticipant_qos_id == null) {
                if (other.domainparticipant_qos_id != null)
                    return false;
            } else if (!domainparticipant_qos_id.equals(other.domainparticipant_qos_id))
                return false;
            if (profile == null) {
                if (other.profile != null)
                    return false;
            } else if (!profile.equals(other.profile))
                return false;
            if (publisher_qos_id == null) {
                if (other.publisher_qos_id != null)
                    return false;
            } else if (!publisher_qos_id.equals(other.publisher_qos_id))
                return false;
            if (subscriber_qos_id == null) {
                if (other.subscriber_qos_id != null)
                    return false;
            } else if (!subscriber_qos_id.equals(other.subscriber_qos_id))
                return false;
            if (topic_qos_id == null) {
                if (other.topic_qos_id != null)
                    return false;
            } else if (!topic_qos_id.equals(other.topic_qos_id))
                return false;
            if (uri == null) {
                if (other.uri != null)
                    return false;
            } else if (!uri.equals(other.uri))
                return false;
            if (qp == null) {
                if (other.qp != null)
                    return false;
            } else if (!qp.equals(other.qp))
                return false;
            return true;
        }

        @Override
        public String toString() {
            return "QosConfiguration [uri=" + uri + ", profile=" + profile + ", domainparticipant_qos_id=" + domainparticipant_qos_id
                    + ", topic_qos_id=" + topic_qos_id + ", publisher_qos_id=" + publisher_qos_id
                    + ", datawriter_qos_id=" + datawriter_qos_id + ", subscriber_qos_id=" + subscriber_qos_id
                    + ", datareader_qos_id=" + datareader_qos_id + "]";
        }
    }

}

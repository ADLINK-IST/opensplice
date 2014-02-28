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
/**
 * Contains a concrete implementation for (de)serializing data (XML).
 * This package provides (de)serialization from and to the XML format.
 */
package org.opensplice.cm.transform.xml;

import java.io.IOException;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.logging.Level;
import java.util.logging.Logger;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.opensplice.cm.Entity;
import org.opensplice.cm.ServiceStateKind;
import org.opensplice.cm.com.Communicator;
import org.opensplice.cm.data.State;
import org.opensplice.cm.impl.DataReaderImpl;
import org.opensplice.cm.impl.EntityImpl;
import org.opensplice.cm.impl.GroupQueueImpl;
import org.opensplice.cm.impl.NetworkReaderImpl;
import org.opensplice.cm.impl.ParticipantImpl;
import org.opensplice.cm.impl.PartitionImpl;
import org.opensplice.cm.impl.PublisherImpl;
import org.opensplice.cm.impl.QueryImpl;
import org.opensplice.cm.impl.QueueImpl;
import org.opensplice.cm.impl.ServiceImpl;
import org.opensplice.cm.impl.ServiceStateImpl;
import org.opensplice.cm.impl.SubscriberImpl;
import org.opensplice.cm.impl.TopicImpl;
import org.opensplice.cm.impl.WaitsetImpl;
import org.opensplice.cm.impl.WriterImpl;
import org.opensplice.cm.transform.EntityDeserializer;
import org.opensplice.cm.transform.TransformationException;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

/**
 * The XML implementation of an EntityDeserializer. It is capable of 
 * transforming a serialized XML representation to an Entity object.
 * 
 * @date Aug 27, 2004 
 */
public class EntityDeserializerXML implements EntityDeserializer {
    /**
     * Creates a new EntityDeserializerXML object which is a concrete descendant
     * of the EntityDeserializer interface.
     * 
     * @throws ParserConfigurationException Thrown when no DOM parser is
     *                                      available in the standard Java API.
     */
    public EntityDeserializerXML(Communicator communicator) throws ParserConfigurationException{
        this.communicator = communicator;
        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        factory.setValidating(false);
        factory.setNamespaceAware(false);
        builder = factory.newDocumentBuilder(); 
        logger = Logger.getLogger("org.opensplice.api.cm.transform.xml");
    }
    
    public Entity deserializeEntity(Object serialized)  throws TransformationException{
        if(serialized == null){
            throw new TransformationException("Supplied Entity is not valid.");
        }
        Entity entity = null;
        Document document;
        
        if(serialized instanceof String){
            String xmlEntity = (String)serialized;
            /*
            logger.logp(Level.FINEST, "EntityDeserializerXML", "deserializeEntity", 
                                       "Deserializing entity from string:\n'" + 
                                       xmlEntity + "'");
            */       
            try {
                document = builder.parse(new InputSource(new StringReader(xmlEntity)));
            }
            catch (SAXException se) {
                logger.logp(Level.SEVERE,  "EntityDeserializerXML", 
                                           "deserializeEntity", 
                                           "SAXException occurred, Entity could not be deserialized");
                throw new TransformationException(se.getMessage());
            }
            catch (IOException ie) {
                logger.logp(Level.SEVERE,  "EntityDeserializerXML", 
                                           "deserializeEntity", 
                                           "IOException occurred, Entity could not be deserialized");
                throw new TransformationException(ie.getMessage());
            }
            
            if(document == null){
                logger.logp(Level.SEVERE,  "EntityDeserializerXML", 
                        "deserializeEntity", 
                        "Received NULL document");
                throw new TransformationException("Supplied Entity is not valid");
            }
            Element rootElement = document.getDocumentElement();
            entity = this.buildEntity(rootElement);
        }
        return entity;
    }
    
    public Entity[] deserializeEntityList(Object serialized) throws TransformationException{
        if(serialized == null){
            throw new TransformationException("Supplied Entity list is not valid");
        }
        Entity[] entities = new Entity[0];
        
        if(serialized instanceof String){
            Document document;
            ArrayList entityList = new ArrayList();
            Element entityElement;
            
            String xmlEntities = (String)serialized;
            /*
            logger.logp(Level.FINEST, "EntityDeserializerXML", "deserializeEntityList", 
                                       "Deserializing entity from string:\n" + 
                                       xmlEntities);
            */       
            try {
                document = builder.parse(new InputSource(new StringReader(xmlEntities)));
            }
            catch (SAXException se) {
                logger.logp(Level.SEVERE,  "EntityDeserializerXML", 
                                           "deserializeEntityList", 
                                           "SAXException occurred, Entity could not be deserialized");
                throw new TransformationException(se.getMessage());
            }
            catch (IOException ie) {
                logger.logp(Level.SEVERE,  "EntityDeserializerXML", 
                                           "deserializeEntityList", 
                                           "IOException occurred, Entity could not be deserialized");
                throw new TransformationException(ie.getMessage());
            }
            if(document == null){
                throw new TransformationException("Supplied Entity list is not valid");
            }
            Element rootElement = document.getDocumentElement();
            NodeList entityNodes = rootElement.getChildNodes();
            
            for(int i=0; i<entityNodes.getLength(); i++){
                entityElement = (Element)(entityNodes.item(i));
                entityList.add(this.buildEntity(entityElement));
            }
            entities = (Entity[])entityList.toArray(entities);            
        }
        return entities;
    }
    
    private Entity buildEntity(Element el){
        if(el == null){
            return null;
        }
        
        NodeList list = el.getElementsByTagName("kind");
        String kind = null;
        EntityImpl result = null;
        
        if(list.getLength() > 0){
            Element elmt = (Element)(list.item(0));
            kind = elmt.getFirstChild().getNodeValue();
        }
        
        long index = this.resolveIndex(el);
        long serial = this.resolveSerial(el);
        String pointer = this.resolvePointer(el);
        String name = this.resolveName(el);
        boolean enabled = this.resolveEnabled(el);
        
        
        if("TOPIC".equals(kind)){
            NodeList topicList = el.getElementsByTagName("typename");
            String typeName = null;

            if(topicList.getLength() > 0){
                Element typeEl = (Element)(topicList.item(0));
                typeName = typeEl.getFirstChild().getNodeValue();
            }
            result = new TopicImpl(getCommunicator(), index, serial, pointer, name, this.resolveKeyList(el), typeName);
        } else if("QUERY".equals(kind)){
            NodeList queryList = el.getElementsByTagName("expression");
            String expression = null;
            Element typeEl;
            
            if(queryList.getLength() > 0){
                typeEl = (Element)(queryList.item(0));
                
                if(typeEl.getFirstChild() != null){
                    expression = typeEl.getFirstChild().getNodeValue();
                }
            }
            
            queryList = el.getElementsByTagName("params");
            String params = null;

            if(queryList.getLength() > 0){
                typeEl = (Element)(queryList.item(0));
                
                if(typeEl.getFirstChild() != null){
                    params = typeEl.getFirstChild().getNodeValue();
                }
            }
            queryList = el.getElementsByTagName("instanceState");
            int instanceState = 0;

            if(queryList.getLength() > 0){
                typeEl = (Element)(queryList.item(0));
                
                if(typeEl.getFirstChild() != null){
                    instanceState = Integer.parseInt(typeEl.getFirstChild().getNodeValue());
                }
            }
            queryList = el.getElementsByTagName("sampleState");
            int sampleState = 0;

            if(queryList.getLength() > 0){
                typeEl = (Element)(queryList.item(0));
                
                if(typeEl.getFirstChild() != null){
                    sampleState = Integer.parseInt(typeEl.getFirstChild().getNodeValue());
                }
            }
            queryList = el.getElementsByTagName("viewState");
            int viewState = 0;

            if(queryList.getLength() > 0){
                typeEl = (Element)(queryList.item(0));
                
                if(typeEl.getFirstChild() != null){
                    viewState = Integer.parseInt(typeEl.getFirstChild().getNodeValue());
                }
            }
            result = new QueryImpl(getCommunicator(), index, serial, pointer, name, expression, params,
                        new State(instanceState), new State(sampleState), new State(viewState));
        } else if("DOMAIN".equals(kind)){
            result = new PartitionImpl(getCommunicator(), index, serial, pointer, name);
        } else if("DATAREADER".equals(kind)){
            result = new DataReaderImpl(getCommunicator(), index, serial, pointer, name);
        } else if("QUEUE".equals(kind)){
            result = new QueueImpl(getCommunicator(), index, serial, pointer, name);
        } else if("GROUPQUEUE".equals(kind)){
            result = new GroupQueueImpl(getCommunicator(), index, serial, pointer, name);
        } else if("NETWORKREADER".equals(kind)){
            result = new NetworkReaderImpl(getCommunicator(), index, serial, pointer, name);
        } else if("WRITER".equals(kind)){
            result = new WriterImpl(getCommunicator(), index, serial, pointer, name);
        } else if("SUBSCRIBER".equals(kind)){
            result = new SubscriberImpl(getCommunicator(), index, serial, pointer, name);
        } else if("PUBLISHER".equals(kind)){
            result = new PublisherImpl(getCommunicator(), index, serial, pointer, name);
        } else if("PARTICIPANT".equals(kind)){
            result = new ParticipantImpl(getCommunicator(), index, serial, pointer, name);
        } else if("SERVICE".equals(kind)){
            result = new ServiceImpl(getCommunicator(), index, serial, pointer, name);
        } else if("SERVICESTATE".equals(kind)){
            NodeList stateList = el.getElementsByTagName("statename");
            String stateName = null;
            String stateKind = null;
            ServiceStateKind ssk = ServiceStateKind.NONE;

            if(stateList.getLength() > 0){
                Element stateEl = (Element)(stateList.item(0));
                stateName = stateEl.getFirstChild().getNodeValue();
            }
            stateList = el.getElementsByTagName("state");
            
            if(stateList.getLength() > 0){
                Element stateEl = (Element)(stateList.item(0));
                stateKind = stateEl.getFirstChild().getNodeValue();
            }
            
            if("INITIALISING".equals(stateKind)){
                ssk = ServiceStateKind.INITIALISING;
            } else if("OPERATIONAL".equals(stateKind)){
                ssk = ServiceStateKind.OPERATIONAL;
            } else if("INCOMPATIBLE_CONFIGURATION".equals(stateKind)){
                ssk = ServiceStateKind.INCOMPATIBLE_CONFIGURATION;
            } else if("TERMINATING".equals(stateKind)){
                ssk = ServiceStateKind.TERMINATING;
            } else if("TERMINATED".equals(stateKind)){
                ssk = ServiceStateKind.TERMINATED;
            } else if("DIED".equals(stateKind)){
                ssk = ServiceStateKind.DIED;
            }
            result = new ServiceStateImpl(getCommunicator(), index, serial, pointer, name, stateName, ssk);
        } else if("WAITSET".equals(kind)){
            result = new WaitsetImpl(getCommunicator(), index, serial, pointer, name);
        } else{
            assert false: "Deserialize entity XML: received unknown entity: " + kind;
            logger.logp(Level.SEVERE,  "EntityDeserializerXML", 
                       "buildEntity", 
                       "Unknown Entity kind found: " + kind);
        }
        if(result != null){
            result.setEnabled(enabled);
        }
        return result;
    }
    
    private String resolveKeyList(Element entityElement){
        NodeList list = entityElement.getElementsByTagName("keyList");
        String result = null;
        
        if(list.getLength() > 0){
            Element el = (Element)(list.item(0));
            Node value = el.getFirstChild();
            
            if(value != null){
                result = value.getNodeValue();
            }
        }
        return result;
    }
    
    private String resolvePointer(Element entityElement){
        NodeList list = entityElement.getElementsByTagName("pointer");
        String result = null;
        
        if(list.getLength() > 0){
            Element el = (Element)(list.item(0));
            result = el.getFirstChild().getNodeValue();
        }
        return result;
    }
    
    private String resolveName(Element entityElement){
        NodeList list = entityElement.getElementsByTagName("name");
        String result = null;
        
        if(list.getLength() > 0){
            Element el = (Element)(list.item(0));
            Node first = el.getFirstChild();
            
            if(first != null){
                result = first.getNodeValue();
                
                if(result.equals("NULL")){
                    result = null;
                }
            } else {
                result = "";
            }
        }
        return result;
    }
    
    private long resolveIndex(Element entityElement){
        NodeList list = entityElement.getElementsByTagName("handle_index");
        long result = -1;
        
        if(list.getLength() > 0){
            Element el = (Element)(list.item(0));
            result = Long.parseLong(el.getFirstChild().getNodeValue());
        }
        return result;
    }
    
    private long resolveSerial(Element entityElement){
        NodeList list = entityElement.getElementsByTagName("handle_serial");
        long result = -1;
        
        if(list.getLength() > 0){
            Element el = (Element)(list.item(0));
            result = Long.parseLong(el.getFirstChild().getNodeValue());
        }
        return result;
    }
    
    private boolean resolveEnabled(Element entityElement){
        NodeList list = entityElement.getElementsByTagName("enabled");
        boolean result = true;
        
        if(list.getLength() > 0){
            Element el = (Element)(list.item(0));
            result = Boolean.valueOf(el.getFirstChild().getNodeValue()).booleanValue();
        }
        return result;
    }

    protected Communicator getCommunicator() {
      return communicator;
    }
    
    /**
     * Builder that is capable of creating a DOM tree from an XML string.
     */
    private DocumentBuilder builder;
    
    /**
     * Logging facilities for the deserializer.
     */
    private Logger logger;

    private Communicator communicator;
}

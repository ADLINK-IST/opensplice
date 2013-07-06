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
package org.opensplice.cm.transform.xml;

import java.io.IOException;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.logging.Level;
import java.util.logging.Logger;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import org.opensplice.cm.status.*;
import org.opensplice.cm.transform.StatusDeserializer;
import org.opensplice.cm.transform.TransformationException;

/**
 * 
 * 
 * @date Oct 13, 2004 
 */
public class StatusDeserializerXML implements StatusDeserializer{
    private DocumentBuilder builder;
    private Logger logger;
    
    public StatusDeserializerXML() throws ParserConfigurationException{
        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        factory.setValidating(false);
        factory.setNamespaceAware(false);
        builder = factory.newDocumentBuilder(); 
        logger = Logger.getLogger("org.opensplice.api.cm.transform.xml");
    }
    
    public Status deserializeStatus(Object serialized) throws TransformationException {
        if(serialized == null){
            throw new TransformationException("Supplied Status is not valid.");
        }
        Status status = null;
        Document document;
        
        if(serialized instanceof String){
            String xmlStatus = (String)serialized;
            /*
            logger.logp(Level.SEVERE, "StatusDeserializerXML", "deserializeStatusses", 
                                       "Deserializing statusses from string:\n'" + 
                                       xmlStatus + "'");
            */     
            try {
                document = builder.parse(new InputSource(new StringReader(xmlStatus)));
            }
            catch (SAXException se) {
                logger.logp(Level.SEVERE,  "StatusDeserializerXML", 
                                           "deserializeStatusses", 
                                           "SAXException occurred, Statusses could not be deserialized");
                throw new TransformationException(se.getMessage());
            }
            catch (IOException ie) {
                logger.logp(Level.SEVERE,  "StatusDeserializerXML", 
                                           "deserializeStatusses", 
                                           "IOException occurred, Statusses could not be deserialized");
                throw new TransformationException(ie.getMessage());
            }
            if(document == null){
                throw new TransformationException("Supplied Status is not valid.");
            }
            
            Element rootElement = document.getDocumentElement();
            status = this.buildStatus(rootElement);
        }
        return status;
    }

    private Status buildStatus(Element rootElement){
        String kind;
        String state = null;
        Status s;
        NodeList stateElements;
        
        s = null;
        kind = rootElement.getFirstChild().getFirstChild().getNodeValue();
        stateElements = rootElement.getElementsByTagName("state");
        
        if(stateElements.getLength() > 0){
            state = stateElements.item(0).getFirstChild().getNodeValue();
        }
        
        if("K_DOMAINSTATUS".equals(kind)){
            s = this.buildPartitionStatus(rootElement, state);
        } else if("K_TOPICSTATUS".equals(kind)){
            s = this.buildTopicStatus(rootElement, state);
        } else if("K_SUBSCRIBERSTATUS".equals(kind)){
            s = this.buildSubscriberStatus(rootElement, state);
        } else if("K_READERSTATUS".equals(kind)){
            s = this.buildReaderStatus(rootElement, state);
        } else if("K_WRITERSTATUS".equals(kind)){
            s = this.buildWriterStatus(rootElement, state);
        } else {
            logger.logp(Level.SEVERE,  "StatusDeserializerXML", 
                    "buildStatusses", 
                    "Found unknown status: " + kind);
        }
        return s;
    }
    
    private PartitionStatus buildPartitionStatus(Element el, String state){
        GroupsChangedInfo s = null;
        PartitionStatus result = null;
        Element statusElement = null;
        
        NodeList elements = el.getElementsByTagName("groupsChanged");
        
        if(elements.getLength() > 0){
            statusElement = (Element)(elements.item(0));
            s = this.buildGroupsChangedInfo(statusElement);
            result = new PartitionStatus(state, s);
        }
        return result;
    }
    
    private TopicStatus buildTopicStatus(Element el, String state){
        InconsistentTopicInfo s = null;
        TopicStatus result = null;
        Element statusElement = null;
        
        NodeList elements = el.getElementsByTagName("inconsistentTopic");
        
        if(elements.getLength() > 0){
            statusElement = (Element)(elements.item(0));
            s = this.buildInconsistentTopicInfo(statusElement);
            result = new TopicStatus(state, s);
        }
        return result;
    }
    
    private SubscriberStatus buildSubscriberStatus(Element el, String state){
        SubscriberStatus result = null;
        Element statusElement = null;
        NodeList elements;
        SampleLostInfo sl = null;
        
        elements = el.getElementsByTagName("sampleLost");
        
        if(elements.getLength() > 0){
            statusElement = (Element)(elements.item(0));
            sl = this.buildSampleLostInfo(statusElement);
        }
        result = new SubscriberStatus(state, sl);
        
        return result;
    }
    
    private WriterStatus buildWriterStatus(Element el, String state){
        LivelinessLostInfo ll = null;
        DeadlineMissedInfo dm = null;
        IncompatibleQosInfo iq = null;
        TopicMatchInfo ti = null;
        WriterStatus result = null;
        Element statusElement = null;
        
        NodeList elements = el.getElementsByTagName("livelinessLost");
        
        if(elements.getLength() > 0){
            statusElement = (Element)(elements.item(0));
            ll = this.buildLivelinessLostInfo(statusElement);
        }
        elements = el.getElementsByTagName("deadlineMissed");
        
        if(elements.getLength() > 0){
            statusElement = (Element)(elements.item(0));
            dm = this.buildDeadlineMissedInfo(statusElement);
        }
        elements = el.getElementsByTagName("incompatibleQos");
        
        if(elements.getLength() > 0){
            statusElement = (Element)(elements.item(0));
            iq = this.buildIncompatibleQosInfo(statusElement);
        }
        
        elements = el.getElementsByTagName("publicationMatch");
        
        if(elements.getLength() > 0){
            statusElement = (Element)(elements.item(0));
            ti = this.buildTopicMatchInfo(statusElement);
        }
        result = new WriterStatus(state, ll, dm, iq, ti);
        
        return result;
    }
    
    private ReaderStatus buildReaderStatus(Element el, String state){
        LivelinessChangedInfo lc = null;
        SampleRejectedInfo sr = null;
        DeadlineMissedInfo dm = null;
        IncompatibleQosInfo iq = null;
        SampleLostInfo sl = null;
        TopicMatchInfo tm = null;
        
        ReaderStatus result = null;
        Element statusElement = null;
        
        NodeList elements = el.getElementsByTagName("livelinessChanged");
                                                     
        if(elements.getLength() > 0){
            statusElement = (Element)(elements.item(0));
            lc = this.buildLivelinessChangedInfo(statusElement);
        }
        elements = el.getElementsByTagName("sampleRejected");
        
        if(elements.getLength() > 0){
            statusElement = (Element)(elements.item(0));
            sr = this.buildSampleRejectedInfo(statusElement);
        }
        elements = el.getElementsByTagName("deadlineMissed");
        
        if(elements.getLength() > 0){
            statusElement = (Element)(elements.item(0));
            dm = this.buildDeadlineMissedInfo(statusElement);
        }
        elements = el.getElementsByTagName("incompatibleQos");
        
        if(elements.getLength() > 0){
            statusElement = (Element)(elements.item(0));
            iq = this.buildIncompatibleQosInfo(statusElement);
        }
        elements = el.getElementsByTagName("sampleLost");
        
        if(elements.getLength() > 0){
            statusElement = (Element)(elements.item(0));
            sl = this.buildSampleLostInfo(statusElement);
        }
        elements = el.getElementsByTagName("subscriptionMatch");

        if(elements.getLength() > 0){
            statusElement = (Element)(elements.item(0));
            tm = this.buildTopicMatchInfo(statusElement);
        }
        result = new ReaderStatus(state, lc, sr, dm, iq, sl, tm);
        
        return result;
    }
    
    private GroupsChangedInfo buildGroupsChangedInfo(Element el){
        long totalCount = 0;
        long totalChanged = 0;
        
        if("NULL".equals(el.getFirstChild().getNodeValue()))
        {
            return null;
        }
        NodeList members = el.getChildNodes();
        String name;
        
        for(int i=0; i<members.getLength(); i++){
            name = members.item(i).getNodeName();
            
            if("totalCount".equals(name)){
                totalCount = Long.parseLong(members.item(i).getFirstChild().getNodeValue());
            } else if("totalChanged".equals(name)){
                totalChanged = Long.parseLong(members.item(i).getFirstChild().getNodeValue());
            } 
        }
        return new GroupsChangedInfo(totalCount, totalChanged);
    }
    
    private SampleLostInfo buildSampleLostInfo(Element el){
        long totalCount = 0;
        long totalChanged = 0;
        
        if("NULL".equals(el.getFirstChild().getNodeValue()))
        {
            return null;
        }
        NodeList members = el.getChildNodes();
        
        for(int i=0; i<members.getLength(); i++){
            if("totalCount".equals(members.item(i).getNodeName())){
                totalCount = Long.parseLong(members.item(i).getFirstChild().getNodeValue());
            } else if("totalChanged".equals(members.item(i).getNodeName())){
                totalChanged = Long.parseLong(members.item(i).getFirstChild().getNodeValue());
            } 
        }
        return new SampleLostInfo(totalCount, totalChanged);
    }
    
    private InconsistentTopicInfo buildInconsistentTopicInfo(Element el){
        long totalCount = 0;
        long totalChanged = 0;
        
        if("NULL".equals(el.getFirstChild().getNodeValue()))
        {
            return null;
        }
        NodeList members = el.getChildNodes();
        
        for(int i=0; i<members.getLength(); i++){
            if("totalCount".equals(members.item(i).getNodeName())){
                totalCount = Long.parseLong(members.item(i).getFirstChild().getNodeValue());
            } else if("totalChanged".equals(members.item(i).getNodeName())){
                totalChanged = Long.parseLong(members.item(i).getFirstChild().getNodeValue());
            } 
        }
        return new InconsistentTopicInfo(totalCount, totalChanged);
    }
    
    private SampleRejectedInfo buildSampleRejectedInfo(Element el){
        long totalCount = 0;
        long totalChanged = 0;
        SampleRejectedKind lastReason = null;
        String lastInstanceHandle = null;
        
        if("NULL".equals(el.getFirstChild().getNodeValue()))
        {
            return null;
        }
        NodeList members = el.getChildNodes();
        String name;
        
        for(int i=0; i<members.getLength(); i++){
            name = members.item(i).getNodeName();
            if("totalCount".equals(name)){
                totalCount = Long.parseLong(members.item(i).getFirstChild().getNodeValue());
            } else if("totalChanged".equals(name)){
                totalChanged = Long.parseLong(members.item(i).getFirstChild().getNodeValue());
            } else if("lastReason".equals(name)){
                lastReason = SampleRejectedKind.from_string(members.item(i).getFirstChild().getNodeValue());
            } else if("instanceHandle".equals(name)){
                if("NULL".equals(members.item(i).getFirstChild().getNodeValue()))
                {
                    lastInstanceHandle = "NULL";
                } else {
                    NodeList members2 = members.item(i).getChildNodes();
                    String name2;
                    String index = null;
                    String serial = null;
                    
                    for(int j=0; j<members2.getLength(); j++){
                        name2 = members2.item(j).getNodeName();
                        
                        if("index".equals(name2)){
                            index = members2.item(j).getFirstChild().getNodeValue();
                        } else if("serial".equals(name2)){
                            serial = members2.item(j).getFirstChild().getNodeValue();
                        }
                    }
                    lastInstanceHandle = index + ", " + serial;
                }
            }
        }
        return new SampleRejectedInfo(totalCount, totalChanged, lastReason, lastInstanceHandle);
    }
    
    private TopicMatchInfo buildTopicMatchInfo(Element el){
        long totalCount = 0;
        long totalChanged = 0;
        long currentCount = 0;
        long currentCountChanged = 0;
        
        String lastHandle = null;
        String name;
        
        if("NULL".equals(el.getFirstChild().getNodeValue()))
        {
            return null;
        }
        NodeList members = el.getChildNodes();
        
        for(int i=0; i<members.getLength(); i++){
            name = members.item(i).getNodeName();
            
            if("totalCount".equals(name)){
                totalCount = Long.parseLong(members.item(i).getFirstChild().getNodeValue());
            } else if("totalChanged".equals(name)){
                totalChanged = Long.parseLong(members.item(i).getFirstChild().getNodeValue());
            } else if("currentCount".equals(name)){
                currentCount = Long.parseLong(members.item(i).getFirstChild().getNodeValue());
            } else if("currentChanged".equals(name)){
                currentCountChanged = Long.parseLong(members.item(i).getFirstChild().getNodeValue());
            } else if("instanceHandle".equals(name)){
                lastHandle = members.item(i).getFirstChild().getNodeValue();
            }
        }
        return new TopicMatchInfo(totalCount, totalChanged, currentCount, currentCountChanged, lastHandle);
    }
    
    private LivelinessChangedInfo buildLivelinessChangedInfo(Element el){
        long activeCount = 0;
        long inactiveCount = 0;
        long activeCountChange = 0;
        long inactiveCountChange = 0;
        String instanceHandle = null;
        
        if("NULL".equals(el.getFirstChild().getNodeValue()))
        {
            return null;
        }
        NodeList members = el.getChildNodes();
        String name;
        
        for(int i=0; i<members.getLength(); i++){
            name = members.item(i).getNodeName();
            
            if("activeCount".equals(name)){
                activeCount = Long.parseLong(members.item(i).getFirstChild().getNodeValue());
            } else if("inactiveCount".equals(name)){
                inactiveCount = Long.parseLong(members.item(i).getFirstChild().getNodeValue());
            } else if("activeChanged".equals(name)){
                activeCountChange = Long.parseLong(members.item(i).getFirstChild().getNodeValue());
            } else if("inactiveChanged".equals(name)){
                inactiveCountChange = Long.parseLong(members.item(i).getFirstChild().getNodeValue());
            }  else if("instanceHandle".equals(name)){
                if("NULL".equals(members.item(i).getFirstChild().getNodeValue()))
                {
                    instanceHandle = "NULL";
                } else {
                    NodeList members2 = members.item(i).getChildNodes();
                    String name2;
                    String index = null;
                    String serial = null;
                    
                    for(int j=0; j<members2.getLength(); j++){
                        name2 = members2.item(j).getNodeName();
                        
                        if("index".equals(name2)){
                            index = members2.item(j).getFirstChild().getNodeValue();
                        } else if("serial".equals(name2)){
                            serial = members2.item(j).getFirstChild().getNodeValue();
                        }
                    }
                    instanceHandle = index + ", " + serial;
                }
            }
        }
        return new LivelinessChangedInfo(activeCount, inactiveCount, activeCountChange, inactiveCountChange, instanceHandle);
    }
    
    private DeadlineMissedInfo buildDeadlineMissedInfo(Element el){
        long totalCount = 0;
        long totalChanged = 0;
        String lastInstanceHandle = null;
        
        if("NULL".equals(el.getFirstChild().getNodeValue()))
        {
            return null;
        }
        NodeList members = el.getChildNodes();
        String name;
        
        for(int i=0; i<members.getLength(); i++){
            name = members.item(i).getNodeName();
            
            if("totalCount".equals(name)){
                totalCount = Long.parseLong(members.item(i).getFirstChild().getNodeValue());
            } else if("totalChanged".equals(name)){
                totalChanged = Long.parseLong(members.item(i).getFirstChild().getNodeValue());
            } else if("instanceHandle".equals(name)){
                if("NULL".equals(members.item(i).getFirstChild().getNodeValue()))
                {
                    lastInstanceHandle = "NULL";
                } else {
                    NodeList members2 = members.item(i).getChildNodes();
                    String name2;
                    String index = null;
                    String serial = null;
                    
                    for(int j=0; j<members2.getLength(); j++){
                        name2 = members2.item(j).getNodeName();
                        
                        if("index".equals(name2)){
                            index = members2.item(j).getFirstChild().getNodeValue();
                        } else if("serial".equals(name2)){
                            serial = members2.item(j).getFirstChild().getNodeValue();
                        }
                    }
                    lastInstanceHandle = index + ", " + serial;
                }
            }
        }
        return new DeadlineMissedInfo(totalCount, totalChanged, lastInstanceHandle);
    }
        
    private LivelinessLostInfo buildLivelinessLostInfo(Element el){
        long totalCount = 0;
        long totalChanged = 0;
        
        if("NULL".equals(el.getFirstChild().getNodeValue()))
        {
            return null;
        }
        NodeList members = el.getChildNodes();
        
        for(int i=0; i<members.getLength(); i++){
            if("totalCount".equals(members.item(i).getNodeName())){
                totalCount = Long.parseLong(members.item(i).getFirstChild().getNodeValue());
            } else if("totalChanged".equals(members.item(i).getNodeName())){
                totalChanged = Long.parseLong(members.item(i).getFirstChild().getNodeValue());
            } 
        }
        return new LivelinessLostInfo(totalCount, totalChanged);
    }
    
    private IncompatibleQosInfo buildIncompatibleQosInfo(Element el){
        long totalCount = 0;
        long totalChanged = 0;
        long lastPolicyId = 0;
        ArrayList qpc = new ArrayList();
        
        if("NULL".equals(el.getFirstChild().getNodeValue()))
        {
            return null;
        }
        NodeList members = el.getChildNodes();
        String name;
        
        for(int i=0; i<members.getLength(); i++){
            name = members.item(i).getNodeName();
            
            if("totalCount".equals(name)){
                totalCount = Long.parseLong(members.item(i).getFirstChild().getNodeValue());
            } else if("totalChanged".equals(name)){
                totalChanged = Long.parseLong(members.item(i).getFirstChild().getNodeValue());
            } else if("lastPolicyId".equals(name)){
                lastPolicyId = Long.parseLong(members.item(i).getFirstChild().getNodeValue());
            } else if("policyCount".equals(name)){
                if("NULL".equals(members.item(i).getFirstChild().getNodeValue())){
                    
                } else {
                    NodeList elements = members.item(i).getChildNodes();
                    
                    for(int j=0; j<elements.getLength(); j++){
                        if("element".equals(elements.item(j).getNodeName())){
                            qpc.add(new Long(elements.item(j).getFirstChild().getNodeValue()));
                        }
                    }
                }
            }
        }
        IncompatibleQosInfo s = new IncompatibleQosInfo(totalCount, totalChanged);
        
        for(int i=0; i<qpc.size(); i++){
            s.addPolicy(((Long)qpc.get(i)).longValue());
        }
        s.setLastPolicyId(lastPolicyId);
        return s;
    }
    /*
    private QosPolicyCount buildQosPolicyCount(Element el){
        long policyId = 0;
        long count = 0;
        
        if("NULL".equals(el.getFirstChild().getNodeValue()))
        {
            return null;
        }
        NodeList members = el.getChildNodes();
        
        for(int i=0; i<members.getLength(); i++){
            if("policyId".equals(members.item(i).getNodeName())){
                policyId = Long.parseLong(members.item(i).getFirstChild().getNodeValue());
            } else if("count".equals(members.item(i).getNodeName())){
                count = Long.parseLong(members.item(i).getFirstChild().getNodeValue());
            } 
        }
        return new QosPolicyCount(policyId, count);
    }
    */
}

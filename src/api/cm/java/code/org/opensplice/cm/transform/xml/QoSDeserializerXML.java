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
import java.util.logging.Level;
import java.util.logging.Logger;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.opensplice.cm.Time;
import org.opensplice.cm.qos.DeadlinePolicy;
import org.opensplice.cm.qos.DurabilityKind;
import org.opensplice.cm.qos.DurabilityPolicy;
import org.opensplice.cm.qos.DurabilityServicePolicy;
import org.opensplice.cm.qos.EntityFactoryPolicy;
import org.opensplice.cm.qos.GroupDataPolicy;
import org.opensplice.cm.qos.HistoryPolicy;
import org.opensplice.cm.qos.HistoryQosKind;
import org.opensplice.cm.qos.LatencyPolicy;
import org.opensplice.cm.qos.LifespanPolicy;
import org.opensplice.cm.qos.LivelinessKind;
import org.opensplice.cm.qos.LivelinessPolicy;
import org.opensplice.cm.qos.OrderbyKind;
import org.opensplice.cm.qos.OrderbyPolicy;
import org.opensplice.cm.qos.OwnershipKind;
import org.opensplice.cm.qos.OwnershipPolicy;
import org.opensplice.cm.qos.PacingPolicy;
import org.opensplice.cm.qos.ParticipantQoS;
import org.opensplice.cm.qos.PresentationKind;
import org.opensplice.cm.qos.PresentationPolicy;
import org.opensplice.cm.qos.PublisherQoS;
import org.opensplice.cm.qos.QoS;
import org.opensplice.cm.qos.ReaderLifecyclePolicy;
import org.opensplice.cm.qos.ReaderLifespanPolicy;
import org.opensplice.cm.qos.ReaderQoS;
import org.opensplice.cm.qos.ReliabilityKind;
import org.opensplice.cm.qos.ReliabilityPolicy;
import org.opensplice.cm.qos.ResourcePolicy;
import org.opensplice.cm.qos.ScheduleKind;
import org.opensplice.cm.qos.SchedulePolicy;
import org.opensplice.cm.qos.SchedulePriorityKind;
import org.opensplice.cm.qos.SharePolicy;
import org.opensplice.cm.qos.StrengthPolicy;
import org.opensplice.cm.qos.SubscriberQoS;
import org.opensplice.cm.qos.TopicDataPolicy;
import org.opensplice.cm.qos.TopicQoS;
import org.opensplice.cm.qos.TransportPolicy;
import org.opensplice.cm.qos.UserDataPolicy;
import org.opensplice.cm.qos.UserKeyPolicy;
import org.opensplice.cm.qos.WriterLifecyclePolicy;
import org.opensplice.cm.qos.WriterQoS;
import org.opensplice.cm.transform.QoSDeserializer;
import org.opensplice.cm.transform.TransformationException;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

/**
 * The XML implementation of an QoSDeserializer. It is capable of 
 * transforming a serialized XML representation to an QoS object.
 * 
 * @date Jan 10, 2005 
 */
public class QoSDeserializerXML implements QoSDeserializer {
    private DocumentBuilder builder = null;
    private Logger logger = null;
    
    /**
     * Creates a new QoSDeserializerXML object which is a concrete descendant
     * of the QoSDeserializer interface.
     * 
     * @throws ParserConfigurationException Thrown when no DOM parser is
     *                                      available in the standard Java API.
     */
    public QoSDeserializerXML() throws ParserConfigurationException{
        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        factory.setValidating(false);
        factory.setNamespaceAware(false);
        builder = factory.newDocumentBuilder(); 
        logger = Logger.getLogger("com.thales.splice.api.cm.transform.xml");
    }
    
    @Override
    public QoS deserializeQoS(Object qos) throws TransformationException{
        QoS result = null;
        Document document = null;
        
        if(qos == null){
            throw new TransformationException("Supplied QoS is not valid.");
        }
        
        if(qos instanceof String){
            try {
                document = builder.parse(new InputSource(new StringReader((String)qos)));
            }
            catch (SAXException se) {
                logger.logp(Level.SEVERE,  "QoSDeserializerXML", 
                                           "deserializeQoS", 
                                           "SAXException occurred, QoS could not be deserialized");
                throw new TransformationException(se.getMessage());
            }
            catch (IOException ie) {
                logger.logp(Level.SEVERE,  "QoSDeserializerXML", 
                                           "deserializeQoS", 
                                           "IOException occurred, QoS could not be deserialized");
                throw new TransformationException(ie.getMessage());
            }
            
            if(document == null){
                logger.logp(Level.SEVERE,  "QoSDeserializerXML", 
                        "deserializeQoS", 
                        "Received NULL document");
                throw new TransformationException("Supplied QoS is not valid.");
            }
            Element rootElement = document.getDocumentElement();
            result = this.buildQoS(rootElement);
        } else if(qos instanceof Element){
            result = this.buildQoS((Element)qos);
        } else {
            throw new TransformationException("Supplied object type not supported. (" + qos.getClass().getName() + ")");
        }
        return result;
    }
    
    private QoS buildQoS(Element e){
        String kind = null;
        QoS result = null;
        
        if(e == null){
            return null;
        }
        Node kindNode = this.getChildNode(e, "kind");
        
        if(kindNode != null){
            Element elmt = (Element)(kindNode);
            Node n = elmt.getFirstChild();
            
            if(n != null){
                kind = n.getNodeValue();
            }
        }
        
        if("V_PARTICIPANT_QOS".equals(kind)){
            result = this.parseParticipantQoS(e);
        } else if("V_TOPIC_QOS".equals(kind)){
            result = this.parseTopicQoS(e);
        } else if("V_WRITER_QOS".equals(kind)){
            result = this.parseWriterQoS(e);
        } else if("V_READER_QOS".equals(kind)){
            result = this.parseReaderQoS(e);
        } else if("V_PUBLISHER_QOS".equals(kind)){
            result = this.parsePublisherQos(e);
        } else if("V_SUBSCRIBER_QOS".equals(kind)){
            result = this.parseSubscriberQoS(e);
        } else{
            logger.logp(Level.SEVERE,  "QoSDeserializerXML", 
                   "buildQoS", 
                   "Unknown QoS kind found: " + kind);
            assert false: "Deserialize QoS XML: received unknown QoS: " + kind;
        }
        return result;
    }
    
    private ParticipantQoS parseParticipantQoS(Element e){
        NodeList list = e.getChildNodes();
        String name;
        Element child;
        EntityFactoryPolicy efp = null;
        UserDataPolicy udp = null;
        SchedulePolicy scp = null;
        
        for(int i=0; i<list.getLength(); i++){
            child = (Element)list.item(i);
            name = child.getNodeName();
            
            if("entityFactory".equals(name)){
                efp = this.parseEntityFactory(child);
            } else if("userData".equals(name)){
                udp = this.parseUserData(child);
            } else if("watchdogScheduling".equals(name)){
                scp = this.parseSchedule(child);
            }
        }
        return new ParticipantQoS(efp, udp, scp);
    }
    
    private TopicQoS parseTopicQoS(Element e){
        NodeList list = e.getChildNodes();
        String name;
        Element child;
        TopicQoS qos = null;
        TopicDataPolicy tdp = null;
        DurabilityPolicy drp = null;
        DurabilityServicePolicy dsp = null;
        DeadlinePolicy dlp = null;
        LatencyPolicy lcp = null;
        LivelinessPolicy llp = null;
        ReliabilityPolicy rbp = null;
        OrderbyPolicy obp = null;
        HistoryPolicy hsp = null;
        ResourcePolicy rsp = null;
        TransportPolicy trp = null;
        LifespanPolicy lsp = null;
        OwnershipPolicy osp = null;
        
        for(int i=0; i<list.getLength(); i++){
            if(list.item(i) instanceof Element){
                child = (Element)list.item(i);
                name = child.getNodeName();
                
                if("topicData".equals(name)){
                    tdp = this.parseTopicData(child);
                } else if("durability".equals(name)){
                    drp = this.parseDurability(child);
                } else if("durabilityService".equals(name)){
                    dsp = this.parseDurabilityService(child);
                } else if("deadline".equals(name)){
                    dlp = this.parseDeadline(child);
                } else if("latency".equals(name)){
                    lcp = this.parseLatency(child);
                } else if("liveliness".equals(name)){
                    llp = this.parseLiveliness(child);
                } else if("reliability".equals(name)){
                    rbp = this.parseReliability(child);
                } else if("orderby".equals(name)){
                    obp = this.parseOrderby(child);
                } else if("history".equals(name)){
                    hsp = this.parseHistory(child);
                } else if("resource".equals(name)){
                    rsp = this.parseResource(child);
                } else if("transport".equals(name)){
                    trp = this.parseTransport(child);
                } else if("lifespan".equals(name)){
                    lsp = this.parseLifespan(child);
                } else if("ownership".equals(name)){
                    osp = this.parseOwnership(child);
                }
            }
        }
        qos = new TopicQoS(tdp, drp, dsp, dlp, lcp, llp, rbp, obp, hsp, rsp, trp, lsp, osp);
        return qos;
    }
    
    private WriterQoS parseWriterQoS(Element e){
        NodeList list = e.getChildNodes();
        String name;
        Element child;
        WriterQoS qos = null;
        DurabilityPolicy drp = null;
        DeadlinePolicy dlp = null;
        LatencyPolicy lcp = null;
        LivelinessPolicy llp = null;
        ReliabilityPolicy rbp = null;
        OrderbyPolicy obp = null;
        HistoryPolicy hsp = null;
        ResourcePolicy rsp = null;
        TransportPolicy trp = null;
        LifespanPolicy lsp = null;
        UserDataPolicy udp = null;
        OwnershipPolicy osp = null;
        StrengthPolicy stp = null;
        WriterLifecyclePolicy wlp = null;
        
        for(int i=0; i<list.getLength(); i++){
            if(list.item(i) instanceof Element){
                child = (Element)list.item(i);
                name = child.getNodeName();
                
                if("durability".equals(name)){
                    drp = this.parseDurability(child);
                } else if("deadline".equals(name)){
                    dlp = this.parseDeadline(child);
                } else if("latency".equals(name)){
                    lcp = this.parseLatency(child);
                } else if("liveliness".equals(name)){
                    llp = this.parseLiveliness(child);
                } else if("reliability".equals(name)){
                    rbp = this.parseReliability(child);
                } else if("orderby".equals(name)){
                    obp = this.parseOrderby(child);
                } else if("history".equals(name)){
                    hsp = this.parseHistory(child);
                } else if("resource".equals(name)){
                    rsp = this.parseResource(child);
                } else if("transport".equals(name)){
                    trp = this.parseTransport(child);
                } else if("lifespan".equals(name)){
                    lsp = this.parseLifespan(child);
                } else if("userData".equals(name)){
                    udp = this.parseUserData(child);
                } else if("ownership".equals(name)){
                    osp = this.parseOwnership(child);
                } else if("strength".equals(name)){
                    stp = this.parseStrenght(child);
                } else if("lifecycle".equals(name)){
                    wlp = this.parseWriterLifecycle(child);
                }
            }
        }
        qos = new WriterQoS(drp, dlp, lcp, llp, rbp, obp, hsp, rsp, trp, lsp, udp, osp, stp, wlp);
        return qos;
    }
    
    private ReaderQoS parseReaderQoS(Element e){
        NodeList list = e.getChildNodes();
        String name;
        Element child;
        ReaderQoS qos = null;
        DurabilityPolicy drp = null;
        DeadlinePolicy dlp = null;
        LatencyPolicy lcp = null;
        LivelinessPolicy llp = null;
        ReliabilityPolicy rbp = null;
        OrderbyPolicy obp = null;
        HistoryPolicy hsp = null;
        ResourcePolicy rsp = null;
        UserDataPolicy udp = null;
        PacingPolicy pcp = null;
        OwnershipPolicy osp = null;
        ReaderLifecyclePolicy rlp = null;
        ReaderLifespanPolicy rlsp = null;
        SharePolicy sp = null;
        UserKeyPolicy uk = null;
        
        for(int i=0; i<list.getLength(); i++){
            if(list.item(i) instanceof Element){
                child = (Element)list.item(i);
                name = child.getNodeName();
                
                if("durability".equals(name)){
                    drp = this.parseDurability(child);
                } else if("deadline".equals(name)){
                    dlp = this.parseDeadline(child);
                } else if("latency".equals(name)){
                    lcp = this.parseLatency(child);
                } else if("liveliness".equals(name)){
                    llp = this.parseLiveliness(child);
                } else if("reliability".equals(name)){
                    rbp = this.parseReliability(child);
                } else if("orderby".equals(name)){
                    obp = this.parseOrderby(child);
                } else if("history".equals(name)){
                    hsp = this.parseHistory(child);
                } else if("resource".equals(name)){
                    rsp = this.parseResource(child);
                } else if("userData".equals(name)){
                    udp = this.parseUserData(child);
                } else if("ownership".equals(name)){
                    osp = this.parseOwnership(child);
                } else if("pacing".equals(name)){
                    pcp = this.parsePacing(child);
                } else if("lifecycle".equals(name)){
                    rlp = this.parseReaderLifecycle(child);
                } else if("lifespan".equals(name)){
                    rlsp = this.parseReaderLifespan(child);
                } else if("share".equals(name)){
                    sp = this.parseShare(child);
                } else if("userKey".equals(name)){
                    uk = this.parseUserKey(child);
                }
            }
        }
        qos = new ReaderQoS(drp, dlp, lcp, llp, rbp, obp, hsp, rsp, udp, osp, pcp, rlp, rlsp, sp, uk);
        return qos;
    }
    
    private PublisherQoS parsePublisherQos(Element e){
        NodeList list = e.getChildNodes();
        String name;
        Element child;
        PresentationPolicy prp = null;
        String partition = null;
        GroupDataPolicy gdp = null;
        EntityFactoryPolicy efp = null;
        
        for(int i=0; i<list.getLength(); i++){
            if(list.item(i) instanceof Element){
                child = (Element)list.item(i);
                name = child.getNodeName();
                
                if("presentation".equals(name)){
                    prp = this.parsePresentation(child);
                } else if("partition".equals(name)){
                    partition = this.parsePartition(child);
                } else if("groupData".equals(name)){
                    gdp = this.parseGroupData(child);
                } else if("entityFactory".equals(name)){
                    efp = this.parseEntityFactory(child);
                }
            }
        }
        return new PublisherQoS(prp, partition, gdp, efp);
    }
    
    private SubscriberQoS parseSubscriberQoS(Element e){
        NodeList list = e.getChildNodes();
        String name;
        Element child;
        PresentationPolicy prp = null;
        String partition = null;
        GroupDataPolicy gdp = null;
        EntityFactoryPolicy efp = null;
        SharePolicy sp = null;
        
        for(int i=0; i<list.getLength(); i++){
            if(list.item(i) instanceof Element){
                child = (Element)list.item(i);
                name = child.getNodeName();
                
                if("presentation".equals(name)){
                    prp = this.parsePresentation(child);
                } else if("partition".equals(name)){
                    partition = this.parsePartition(child);
                } else if("groupData".equals(name)){
                    gdp = this.parseGroupData(child);
                } else if("entityFactory".equals(name)){
                    efp = this.parseEntityFactory(child);
                } else if("share".equals(name)){
                    sp = this.parseShare(child);
                }
            }
        }
        return new SubscriberQoS(prp, partition, gdp, efp, sp);
    }
    /*
    private ViewQoS parseViewQoS(Element e){
        String name;
        Element child;
        String keyPolicy = null;
        NodeList list = e.getChildNodes();
        
        for(int i=0; i<list.getLength(); i++){
            if(list.item(i) instanceof Element){
                child = (Element)list.item(i);
                name = child.getNodeName();
                
                if("keyList".equals(name)){
                    keyPolicy = this.parseKeyPolicy(child);
                }
            }
        }
        return new ViewQoS(keyPolicy);
    }
    */
    private EntityFactoryPolicy parseEntityFactory(Element e){
        Node child = this.getChildNode(e, "autoenable_created_entities");
        EntityFactoryPolicy result = null;
        
        if(child != null){
            Element el = (Element)child;
            String value = el.getFirstChild().getNodeValue();
            boolean b = false;
            
            if("TRUE".equals(value)){
               b = true;
            }
            result = new EntityFactoryPolicy(b);
        }
        return result;
    }
    
    private UserDataPolicy parseUserData(Element e){
        NodeList list;
        UserDataPolicy result;
        int size = -1;
        byte[] value;
        
        Node child = this.getChildNode(e, "size");
        
        if(child != null){
            Element el = (Element)child;
            size = Integer.parseInt(el.getFirstChild().getNodeValue());
        }
        
        if(size == -1){
            result = null;
        } else if(size > 0){
            value = new byte[size];
            list = e.getElementsByTagName("value");
            
            if(list.getLength() > 0){
                list = ((Element)list.item(0)).getElementsByTagName("element");
                
                for(int i=0; i<value.length; i++){
                    value[i] = Byte.parseByte(
                                list.item(i).getFirstChild().getNodeValue());
                }
            }
            result = new UserDataPolicy(value);
        } else {
            result = new UserDataPolicy(null);
        }
        return result;
    }
    
    private TopicDataPolicy parseTopicData(Element e){
        NodeList list;
        TopicDataPolicy result;
        int size = -1;
        byte[] value;
        Node child = this.getChildNode(e, "size");
        
        if(child != null){
            Element el = (Element)child;
            size = Integer.parseInt(el.getFirstChild().getNodeValue());
        }
        
        if(size == -1){
            result = null;
        } else if(size > 0){
            value = new byte[size];
            list = e.getElementsByTagName("value");
            
            if(list.getLength() > 0){
                list = ((Element)list.item(0)).getElementsByTagName("element");
                
                for(int i=0; i<value.length; i++){
                    value[i] = Byte.parseByte(
                                list.item(i).getFirstChild().getNodeValue());
                }
            }
            result = new TopicDataPolicy(value);
        } else {
            result = new TopicDataPolicy(null);
        }
        return result;
    }
    
    private DurabilityPolicy parseDurability(Element e){
        DurabilityPolicy result = null;
        DurabilityKind kind = null;
        Node child = this.getChildNode(e, "kind");
        
        if(child != null){
            Element el = (Element)child;
            String value = el.getFirstChild().getNodeValue();
            kind = DurabilityKind.from_string(value);
            
            if(kind != null){
                result = new DurabilityPolicy(kind);
            }
        }
        return result;
    }
    
    private DurabilityServicePolicy parseDurabilityService(Element e){
        DurabilityServicePolicy result = null;
        NodeList list = e.getChildNodes();
        Node n;
        int max_samples = -1;
        int max_instances = -1;
        int max_samples_per_instance = -1;
        int history_depth = 1;
        HistoryQosKind kind = null;
        Time service_cleanup_delay = null;
        
        for(int i=0; i< list.getLength(); i++){
            n = list.item(i);
            
            if("max_samples".equals(n.getNodeName())){
                max_samples = Integer.parseInt(n.getFirstChild().getNodeValue());
            } else if("max_instances".equals(n.getNodeName())){
                max_instances = Integer.parseInt(n.getFirstChild().getNodeValue());
            } else if("max_samples_per_instance".equals(n.getNodeName())){
                max_samples_per_instance = 
                            Integer.parseInt(n.getFirstChild().getNodeValue());
            } else if("history_depth".equals(n.getNodeName())){
                history_depth = Integer.parseInt(n.getFirstChild().getNodeValue());
            } else if("history_kind".equals(n.getNodeName())){
                kind = HistoryQosKind.from_string(n.getFirstChild().getNodeValue());  
            }
        }
        service_cleanup_delay = this.parseTime("service_cleanup_delay", e);
        
        result = new DurabilityServicePolicy(service_cleanup_delay, kind,
                history_depth, max_samples, max_instances, max_samples_per_instance);
        
        return result;
    }
    
    private DeadlinePolicy parseDeadline(Element e){
        return new DeadlinePolicy(this.parseTime("period", e));
    }
    
    private LatencyPolicy parseLatency(Element e){
        return new LatencyPolicy(this.parseTime("duration", e));
    }
    
    private LivelinessPolicy parseLiveliness(Element e){
        LivelinessPolicy result = null;
        LivelinessKind kind = null;
        Time t = null;
        
        Node child = this.getChildNode(e, "kind");
        
        if(child != null){
            Element el = (Element)child;
            kind = LivelinessKind.from_string(el.getFirstChild().getNodeValue());
            t = this.parseTime("lease_duration", e);
            
            result = new LivelinessPolicy(kind, t);
        }
        
        return result;
    }
    
    private ReliabilityPolicy parseReliability(Element e){
        ReliabilityPolicy result = null;
        ReliabilityKind kind = null;
        Time t = null;
        Boolean synchronous = false;
        Node child = this.getChildNode(e, "kind");
        
        if(child != null){
            Element el = (Element)child;
            kind = ReliabilityKind.from_string(el.getFirstChild().getNodeValue());
            t = this.parseTime("max_blocking_time", e);
            child = this.getChildNode(e, "synchronous");
            if (child != null) {
                synchronous = new Boolean(child.getFirstChild().getNodeValue());
            }
            result = new ReliabilityPolicy(kind, t, synchronous);
        }
        return result;
    }
    
    private OrderbyPolicy parseOrderby(Element e){
        OrderbyPolicy result = null;
        OrderbyKind kind = null;
        Node child = this.getChildNode(e, "kind");
        
        if(child != null){
            Element el = (Element)child;
            kind = OrderbyKind.from_string(el.getFirstChild().getNodeValue());
                        
            result = new OrderbyPolicy(kind);
        }
        return result;
    }
    
    private HistoryPolicy parseHistory(Element e){
        HistoryPolicy result = null;
        HistoryQosKind kind = null;
        NodeList list = e.getChildNodes();
        Node n;
        int depth = 0;
        
        for(int i=0; i< list.getLength(); i++){
            n = list.item(i);
            
            if("kind".equals(n.getNodeName())){
                kind = HistoryQosKind.from_string(n.getFirstChild().getNodeValue());
            } else if("depth".equals(n.getNodeName())){
                depth = Integer.parseInt(n.getFirstChild().getNodeValue());
            }
        }
        result = new HistoryPolicy(kind, depth);
        
        return result;
    }
    
    private ResourcePolicy parseResource(Element e){
        ResourcePolicy result = null;
        NodeList list = e.getChildNodes();
        Node n;
        int max_samples = -1;
        int max_instances = -1;
        int max_samples_per_instance = -1;
        
        for(int i=0; i< list.getLength(); i++){
            n = list.item(i);
            
            if("max_samples".equals(n.getNodeName())){
                max_samples = Integer.parseInt(n.getFirstChild().getNodeValue());
            } else if("max_instances".equals(n.getNodeName())){
                max_instances = Integer.parseInt(n.getFirstChild().getNodeValue());
            } else if("max_samples_per_instance".equals(n.getNodeName())){
                max_samples_per_instance = 
                            Integer.parseInt(n.getFirstChild().getNodeValue());
            }
        }
        result = new ResourcePolicy(max_samples, max_instances, max_samples_per_instance);
        
        return result;
    }
    
    private TransportPolicy parseTransport(Element e){
        TransportPolicy result = null;
        int value;
        Node n = this.getChildNode(e, "value");
        
        if(n != null){
            value = Integer.parseInt(n.getFirstChild().getNodeValue());
            result = new TransportPolicy(value);
        }
        return result;
    }
    
    private LifespanPolicy parseLifespan(Element e){
        return new LifespanPolicy(this.parseTime("duration", e));
    }
    
    private StrengthPolicy parseStrenght(Element e){
        StrengthPolicy result = null;
        int value;
        Node n = this.getFirstChildElement(e);
        
        if(n != null){
            value = Integer.parseInt(n.getFirstChild().getNodeValue());
            result = new StrengthPolicy(value);
        }
        return result;
    }
    
    private WriterLifecyclePolicy parseWriterLifecycle(Element e){
        WriterLifecyclePolicy result = null;
        Node n = this.getFirstChildElement(e);
        
        if(n != null){
            String value = n.getFirstChild().getNodeValue();
            boolean b = false;
            
            if("TRUE".equals(value)){
               b = true;
            }
            result = new WriterLifecyclePolicy(b, this.parseTime("autopurge_suspended_samples_delay", e), this.parseTime("autounregister_instance_delay", e));
        }
        return result;
    }
    
    private OwnershipPolicy parseOwnership(Element e){
        OwnershipPolicy result = null;
        OwnershipKind kind = null;
        Node child = this.getChildNode(e, "kind");
        
        if(child != null){
            Element el = (Element)child;
            kind = OwnershipKind.from_string(el.getFirstChild().getNodeValue());
                        
            result = new OwnershipPolicy(kind);
        }
        return result;
    }
    
    private PacingPolicy parsePacing(Element e){
        return new PacingPolicy(this.parseTime("minSeperation", e));
    }
    
    private ReaderLifecyclePolicy parseReaderLifecycle(Element e){
        boolean enable_invalid_samples;
        Node node = this.getChildNode(e, "enable_invalid_samples");
        
        if(node != null){
            String value = node.getFirstChild().getNodeValue();
            
            if("TRUE".equals(value)){
                enable_invalid_samples = true;
            } else {
                enable_invalid_samples = false;
            }
        } else {
            enable_invalid_samples = false;
        }
        return new ReaderLifecyclePolicy(
                this.parseTime("autopurge_nowriter_samples_delay", e),
                this.parseTime("autopurge_disposed_samples_delay", e),
                enable_invalid_samples);
    }
    
    private ReaderLifespanPolicy parseReaderLifespan(Element e){
        ReaderLifespanPolicy result = null;
        Node n = this.getFirstChildElement(e);
        
        if(n != null){
            String value = n.getFirstChild().getNodeValue();
            boolean b = false;
            
            if("TRUE".equals(value)){
               b = true;
            }
            result = new ReaderLifespanPolicy(b, this.parseTime("duration", e));
        }
        return result;
    }
    
    private SharePolicy parseShare(Element e){
        SharePolicy result = null;        
        Node n;
        String name = null;
        boolean enable = false;
        
        NodeList list = e.getChildNodes();
        
        for(int i=0; i< list.getLength(); i++){
            n = list.item(i);

            if("name".equals(n.getNodeName())){
                name = n.getFirstChild().getNodeValue();
                
                if("<NULL>".equals(name) || "&lt;NULL&gt;".equals(name)){
                    name = null;
                }
            } else if("enable".equals(n.getNodeName())){
                enable = new Boolean(n.getFirstChild().getNodeValue()).booleanValue();
            }
        }
        result = new SharePolicy(name, enable);
        
        return result;
    }
    
    private UserKeyPolicy parseUserKey(Element e){
        UserKeyPolicy result = null;        
        Node n, expressionNode;
        String expression = null;
        boolean enable = false;
        
        NodeList list = e.getChildNodes();
        
        for(int i=0; i< list.getLength(); i++){
            n = list.item(i);

            if("expression".equals(n.getNodeName())){
                expressionNode = n.getFirstChild();
                
                if(expressionNode != null) {
                    expression = expressionNode.getNodeValue();
                
                    if("<NULL>".equals(expression) || "&lt;NULL&gt;".equals(expression)){
                        expression = null;
                    }
                } else {
                    expression = "";
                }
            } else if("enable".equals(n.getNodeName())){
                enable = new Boolean(n.getFirstChild().getNodeValue()).booleanValue();
            }
        }
        result = new UserKeyPolicy(enable, expression);
        
        return result;
    }
    
    private PresentationPolicy parsePresentation(Element e){
        PresentationPolicy result = null;
        NodeList list = e.getChildNodes();
        Node n;
        PresentationKind access_scope = null;
        boolean coherent_access = false;
        boolean ordered_access = false;
        
        for(int i=0; i< list.getLength(); i++){
            n = list.item(i);
            
            if("access_scope".equals(n.getNodeName())){
                access_scope = PresentationKind.from_string(
                                        n.getFirstChild().getNodeValue());
            } else if("coherent_access".equals(n.getNodeName())){
                 coherent_access = new Boolean(
                             n.getFirstChild().getNodeValue()).booleanValue();
            } else if("ordered_access".equals(n.getNodeName())){
                ordered_access = new Boolean(
                            n.getFirstChild().getNodeValue()).booleanValue();
            }
        }
        result = new PresentationPolicy(access_scope, coherent_access, ordered_access);
        
        return result;
    }
    
    private SchedulePolicy parseSchedule(Element e){
        SchedulePolicy result = null;
        Node n;
        ScheduleKind kind = ScheduleKind.DEFAULT;
        SchedulePriorityKind priorityKind = SchedulePriorityKind.RELATIVE;
        int priority = -1;
        
        NodeList list = e.getChildNodes();
        
        for(int i=0; i< list.getLength(); i++){
            n = list.item(i);
            
            if("kind".equals(n.getNodeName())){
                kind = ScheduleKind.from_string(n.getFirstChild().getNodeValue());
            } else if("priorityKind".equals(n.getNodeName())){
                priorityKind = SchedulePriorityKind.from_string(n.getFirstChild().getNodeValue());
            } else if("priority".equals(n.getNodeName())){
                priority = Integer.parseInt(n.getFirstChild().getNodeValue());
            }
        }
        result = new SchedulePolicy(kind, priorityKind, priority);
        
        return result;
    }
    
    private String parsePartition(Element e){
        String result;
        Node child = e.getFirstChild();
        
        if(child == null){
            result = null;
        } else if (child.getNodeValue().equals("&lt;NULL&gt;")) {
            result = null;
        } else {
            result = child.getNodeValue();
        }
        return result;
    }
    
    private GroupDataPolicy parseGroupData(Element e){
        NodeList list;
        GroupDataPolicy result;
        int size = -2;
        byte[] value;
        Node child = this.getChildNode(e, "size");
        
        if(child != null){
            Element el = (Element)child;
            size = Integer.parseInt(el.getFirstChild().getNodeValue());
        }
        
        if(size == -2){
            result = null;
        } else if(size > 0){
            value = new byte[size];
            list = e.getElementsByTagName("value");
            
            if(list.getLength() > 0){
                list = ((Element)list.item(0)).getElementsByTagName("element");
                
                for(int i=0; i<value.length; i++){
                    value[i] = Byte.parseByte(
                                list.item(i).getFirstChild().getNodeValue());
                }
            }
            result = new GroupDataPolicy(value);
        } else {
            result = new GroupDataPolicy(null);
        }
        return result;
    }
    /*
    private String parseKeyPolicy(Element e){
        Node n = this.getChildNode(e, "expression");
        String result = null;
        
        if(n != null){
            result = n.getFirstChild().getNodeValue();
        }
        return result;
    }
    */
    private Time parseTime(String name, Element e){
        NodeList list = e.getElementsByTagName(name);
        Node n;
        int sec = 0, nsec = 0;
        Time result = null;
        
        Node node = this.getChildNode(e, name);
        
        if(node != null){
            Element child = (Element)node;
            list = child.getChildNodes();
            
            for(int i=0; i<list.getLength(); i++){
               n = list.item(i);
               
               if("seconds".equals(n.getNodeName())){
                   sec = Integer.parseInt(n.getFirstChild().getNodeValue());
               } else if("nanoseconds".equals(n.getNodeName())){
                   nsec = Integer.parseInt(n.getFirstChild().getNodeValue());
               }
            }
            result = new Time(sec, nsec);
        }
        return result;
    }
    
    private Node getChildNode(Node node, String child){
        NodeList members = node.getChildNodes();
        Node childNode = null;
        
        if(child != null){
            for(int i=0; (i<members.getLength()) && (childNode == null); i++){
                if(child.equals(members.item(i).getNodeName())){
                    childNode = members.item(i);
                }
            }
        }
        return childNode;
    }
    
    private Element getFirstChildElement(Node parent){
        NodeList members = parent.getChildNodes();
        Element childNode = null;
        
        for(int i=0; (i<members.getLength()) && (childNode == null); i++){
            if(members.item(i) instanceof Element){
                childNode = (Element)members.item(i);
            }
        }
        return childNode;
    }
    /*
    private Element getLastChildElement(Node parent){
        NodeList members = parent.getChildNodes();
        Element childNode = null;
        
        for(int i=0; i<members.getLength(); i++){
            if(members.item(i) instanceof Element){
                childNode = (Element)members.item(i);
            }
        }
        return childNode;
    }
    */
}

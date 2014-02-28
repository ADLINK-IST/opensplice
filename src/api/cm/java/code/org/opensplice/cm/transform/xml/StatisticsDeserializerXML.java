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

import org.opensplice.cm.Entity;
import org.opensplice.cm.Time;
import org.opensplice.cm.statistics.AbstractValue;
import org.opensplice.cm.statistics.AvgValue;
import org.opensplice.cm.statistics.FullCounter;
import org.opensplice.cm.statistics.Statistics;
import org.opensplice.cm.statistics.StringValue;
import org.opensplice.cm.statistics.TimedValue;
import org.opensplice.cm.statistics.Value;
import org.opensplice.cm.transform.StatisticsDeserializer;
import org.opensplice.cm.transform.TransformationException;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

/**
 *
 *
 * @date May 12, 2005
 */
public class StatisticsDeserializerXML implements StatisticsDeserializer{
    private final DocumentBuilder builder;
    private final Logger logger;

    public StatisticsDeserializerXML() throws ParserConfigurationException{
        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        factory.setValidating(false);
        factory.setNamespaceAware(false);
        builder = factory.newDocumentBuilder();
        logger = Logger.getLogger("org.opensplice.api.cm.transform.xml");
    }

    @Override
    public Statistics deserializeStatistics(Object serialized, Entity entity) throws TransformationException {
        Document document;
        Statistics statistics = null;

        if(serialized == null){
            throw new TransformationException("No statistics supplied.");
        }

        if(serialized instanceof String){
            String xmlStatistics = (String)serialized;
            logger.logp(Level.FINEST,  "StatisticsDeserializerXML",
                    "deserializeStatistics",
                    xmlStatistics);

            try {
                document = builder.parse(new InputSource(new StringReader(xmlStatistics)));
            }
            catch (SAXException se) {
                logger.logp(Level.SEVERE,  "StatisticsDeserializerXML",
                                           "deserializeStatistics",
                                           "SAXException occurred, Statistics could not be deserialized");
                throw new TransformationException(se.getMessage());
            }
            catch (IOException ie) {
                logger.logp(Level.SEVERE,  "StatisticsDeserializerXML",
                                           "deserializeStatistics",
                                           "IOException occurred, Statistics could not be deserialized");
                throw new TransformationException(ie.getMessage());
            }
            if(document == null){
                throw new TransformationException("Supplied Statistics is not valid.");
            }

            Element rootElement = document.getDocumentElement();
            statistics = this.buildStatistics(rootElement, entity);
        }
        return statistics;
    }
    
    
	@Override
	public Statistics[] deserializeStatistics(Object serialized, Entity[] entities) throws TransformationException {
	    Document document;
	    Statistics[] statistics = null;
	
	    if(serialized == null){
	        throw new TransformationException("No statistics supplied.");
	    }
	
	    if(serialized instanceof String){
	        String xmlStatistics = (String)serialized;
	        logger.logp(Level.FINEST,  "StatisticsDeserializerXML",
	                "deserializeStatistics",
	                xmlStatistics);
	
	        try {
	            document = builder.parse(new InputSource(new StringReader(xmlStatistics)));
	        }
	        catch (SAXException se) {
	            logger.logp(Level.SEVERE,  "StatisticsDeserializerXML",
	                                       "deserializeStatistics",
	                                       "SAXException occurred, Statistics could not be deserialized");
	            throw new TransformationException(se.getMessage());
	        }
	        catch (IOException ie) {
	            logger.logp(Level.SEVERE,  "StatisticsDeserializerXML",
	                                       "deserializeStatistics",
	                                       "IOException occurred, Statistics could not be deserialized");
	            throw new TransformationException(ie.getMessage());
	        }
	        if(document == null){
	            throw new TransformationException("Supplied Statistics is not valid.");
	        }
	
	        Element rootElement = document.getDocumentElement();	        
	        NodeList statsXML = rootElement.getElementsByTagName("object");
	               	        		
	        statistics = new Statistics[entities.length]; 
	        for(int i = 0 ; i < entities.length ; i++){
	        	if(entities[i] == null){
	        		throw new TransformationException("Supplied entities not valid");
	        	}
				statistics[i] = this.buildStatistics((Element) statsXML.item(i), entities[i]);
	        }
	    }
	    return statistics;
	}

    private Statistics buildStatistics(Element el, Entity entity) throws TransformationException{
        Statistics statistics;
        statistics = new Statistics(entity, new Time(0,0));
        return buildStatisticsTree(el, entity, statistics, "");
    }

    private Statistics buildStatisticsStringArray(Element el, Entity entity, Statistics statistics, String pref)
            throws TransformationException {
        Statistics stat = statistics;
        NodeList list = el.getChildNodes();
        Node child;
        String prefix = "";
        for (int i = 0; i < list.getLength(); i++) {
            child = list.item(i);
            if (child instanceof Element) {
                if ("element".equals(child.getNodeName())) {
                    /* ignore the size element which is always at position 0 */
                    prefix = pref + "[" + (i - 1) + "]";
                    stat.addString(this.buildStringValue((Element) child), prefix);
                }
            }
        }
        return stat;
    }

    private Statistics buildStatisticsTree(Element el, Entity entity, Statistics statistics, String pref) throws TransformationException{
    	Statistics stat = statistics;
    	NodeList list = el.getChildNodes();
        Node child;
        String prefix = "";
        String name ="";
        for(int i=0; i<list.getLength(); i++){
            child = list.item(i);
            if(child instanceof Element){
                if ("lastReset".equals(child.getNodeName())) {
                    stat.setLastReset(this.parseTime((Element) child));
                } else if (("channels".equals(child.getNodeName())) || ("queues".equals(child.getNodeName()))
                        || ("scenarios".equals(child.getNodeName())) || ("storages".equals(child.getNodeName()))) {
                    prefix = pref + child.getNodeName() + ".";
                    stat = buildStatisticsTree((Element) child, entity, stat, prefix);
                } else if ("element".equals(child.getNodeName())) {
                    if (!name.equalsIgnoreCase("")) {
                        prefix = pref + "[" + name + "].";
                    } else {
                        prefix = pref + "[" + i + "].";
                    }
                    stat = buildStatisticsTree((Element) child, entity, stat, prefix);
                } else if ("numberOfSamplesWaiting".equals(child.getNodeName())) {
                    stat.addCounter(buildFullCounter((Element) child), pref);
                } else if (("topicsRecorded".equals(child.getNodeName()))
                        || ("topicsReplayed".equals(child.getNodeName()))) {
                    prefix = pref + child.getNodeName();
                    stat = buildStatisticsStringArray((Element) child, entity, stat, prefix);
                } else if ("name".equals(child.getNodeName())) {
                    name = buildStringValue((Element) child).getValue();
                    String tmp = pref.substring(0, pref.indexOf("[")) + "[" + name + "].";
                    pref = tmp;
                } else {
                    stat.addCounter(this.buildCounter((Element) child), pref);
                }
            }
        }
        return stat;
    }

    private AbstractValue buildCounter(Element el) throws TransformationException{
        Node child;
        String childName;
        AbstractValue result = null;
        NodeList children = el.getChildNodes();

        if(children.getLength() == 4){
            result = this.buildFullCounter(el);
        } else {
        	child = children.item(0);
            childName = child.getNodeName();

            if("count".equals(childName)){
               result = this.buildAvgValue(el);
            } else if("lastUpdate".equals(childName)){
               result = this.buildTimedValue(el);
            } else {
        	   result = this.buildValue(el);
            }
        }
        return result;
    }

    private Value buildValue(Element el) throws TransformationException{
        long longValue;
        String name;
        Value result = null;
        Node value = el.getFirstChild();

        if(value != null){
            try{
                longValue = Long.parseLong(value.getNodeValue());
            } catch(NumberFormatException nfe){
                logger.logp(Level.SEVERE,  "StatisticsDeserializerXML",
                        "buildValue",
                        "NumberFormatException occurred, Statistics could not be deserialized: " + value);
                throw new TransformationException(nfe.getMessage());
            }
            name = el.getNodeName();
            result = new Value(name, longValue);
        }
        return result;
    }

    private StringValue buildStringValue(Element el) throws TransformationException{
        String stringValue;
        String name;
        StringValue result = null;
        Node value = el.getFirstChild();

        if(value != null){
            try{
            	stringValue = value.getNodeValue();
            } catch(NumberFormatException nfe){
                logger.logp(Level.SEVERE,  "StatisticsDeserializerXML",
                        "buildValue",
                        "NumberFormatException occurred, Statistics could not be deserialized: " + value);
                throw new TransformationException(nfe.getMessage());
            }
            name = el.getNodeName();
            result = new StringValue(name, stringValue);
        }
        return result;
    }

    private TimedValue buildTimedValue(Element el) throws TransformationException{
        long longValue = 0;
        Time lastUpdate = new Time(0,0);
        String name, childName;
        Node child, value;

        name = el.getNodeName();

        NodeList list = el.getChildNodes();

        for(int i=0; i<list.getLength(); i++){
            child = list.item(i);
            childName = child.getNodeName();

            if("lastUpdate".equals(childName)){
                lastUpdate = this.parseTime((Element)child);
            } else if("value".equals(childName)){
                value = child.getFirstChild();

                if(value != null){
                    try{
                        longValue = Long.parseLong(value.getNodeValue());
                    } catch(NumberFormatException nfe){
                        logger.logp(Level.SEVERE,  "StatisticsDeserializerXML",
                                "buildTimedValue",
                                "NumberFormatException occurred, Statistics could not be deserialized");
                        throw new TransformationException(nfe.getMessage());
                    }
                }
            }
        }
        return new TimedValue(name, longValue, lastUpdate);
    }

    private  AvgValue buildAvgValue(Element el) throws TransformationException{
        float floatValue = 0;
        long count = 0;
        String name, childName;
        Node child, value;

        name = el.getNodeName();
        NodeList list = el.getChildNodes();

        for(int i=0; i<list.getLength(); i++){
            child = list.item(i);
            childName = child.getNodeName();

            if("value".equals(childName)){
                value = child.getFirstChild();

                if(value != null){
                    try{
                        floatValue = Float.parseFloat(value.getNodeValue());
                    } catch(NumberFormatException nfe){
                        logger.logp(Level.SEVERE,  "StatisticsDeserializerXML",
                                "buildAvgValue",
                                "NumberFormatException occurred, Statistics could not be deserialized");

                        throw new TransformationException(nfe.getMessage());
                    }
                }
            } else if("count".equals(childName)){
                value = child.getFirstChild();

                if(value != null){
                    try{
                        count = Long.parseLong(value.getNodeValue());
                    } catch(NumberFormatException nfe){
                        logger.logp(Level.SEVERE,  "StatisticsDeserializerXML",
                                "buildAvgValue",
                                "NumberFormatException occurred, Statistics could not be deserialized");
                        throw new TransformationException(nfe.getMessage());
                    }
                }
            }
        }
        //name = name + countelement;
       return new AvgValue(name, count, floatValue);
    }

    private FullCounter buildFullCounter(Element el) throws TransformationException{
        long longValue = 0;
        TimedValue min = null;
        TimedValue max = null;
        AvgValue avg = null;
        String name, childName;
        Node child, value;
        //AbstractValue value;

        name = el.getNodeName();

        NodeList list = el.getChildNodes();

        for(int i=0; i<list.getLength(); i++){
            child = list.item(i);
            childName = child.getNodeName();

            if("min".equals(childName)){
                min = this.buildTimedValue((Element)child);
            } else if("max".equals(childName)){
                max = this.buildTimedValue((Element)child);
            } else if("avg".equals(childName)){
                avg = this.buildAvgValue((Element)child);
            } else if("value".equals(childName)){
                value = child.getFirstChild();

                if(value != null){
                    try{
                        longValue = Long.parseLong(value.getNodeValue());
                    } catch(NumberFormatException nfe){
                        logger.logp(Level.SEVERE,  "StatisticsDeserializerXML",
                                "buildTimedValue",
                                "NumberFormatException occurred, Statistics could not be deserialized");
                        throw new TransformationException(nfe.getMessage());
                    }
                }
            }
        }
        //name = name + countelement;
        return new FullCounter(name, longValue, min, max, avg);
    }


    /*
    private Counter buildCounterOld(Element el){
        Counter result  = null;
        String name     = el.getNodeName();
        String unit     = null;
        long value      = -1;
        long max        = -1;
        long min        = -1;
        long avg        = -1;
        long count      = -1;
        Time lastTime   = null;
        Time maxTime    = null;
        Time minTime    = null;

        NodeList children = el.getChildNodes();
        Node child;
        String childName;
        int counterType = 0;

        for(int i=0; i<children.getLength(); i++){
            child = children.item(i);
            childName = child.getNodeName();

            if("unit".equals(childName)){
                unit = child.getFirstChild().getNodeValue();
                if (counterType == 0) {
                    counterType = 1;
                }
            } else if("value".equals(childName)){
                value = Long.parseLong(child.getFirstChild().getNodeValue());
                if (counterType == 0) {
                    counterType = 1;
                }
            } else if("lastUpdate".equals(childName)){
                lastTime = this.parseTime((Element)child);
                if (counterType == 0) {
                    counterType = 1;
                }
            } else if("max".equals(childName)){
                max = Long.parseLong(child.getFirstChild().getNodeValue());
                if (counterType<2) {
                    counterType = 2;
                }
            } else if("timeMax".equals(childName)){
                maxTime = this.parseTime((Element)child);
                if (counterType<2) {
                    counterType = 2;
                }
            } else if("min".equals(childName)){
                min = Long.parseLong(child.getFirstChild().getNodeValue());
                counterType = 3;
            } else if("timeMin".equals(childName)){
                minTime = this.parseTime((Element)child);
                counterType = 3;
            } else if("avg".equals(childName)){
                avg = Long.parseLong(child.getFirstChild().getNodeValue());
                counterType = 3;
            } else if("count".equals(childName)){
                count = Long.parseLong(child.getFirstChild().getNodeValue());
                counterType = 3;
            }
        }

        switch (counterType) {
            case 1:
                result = new Counter(name, unit, value, lastTime);
                break;
            case 2:
                result = new MaxCounter(name, unit, value, lastTime, max, maxTime);
                break;
            case 3:
                result = new FullCounter(name, unit, value, lastTime, max, maxTime, min, minTime, avg, count);
                break;
            default:
               logger.logp(Level.SEVERE,  "StatisticsDeserializerXML", "buildCounter", "Statistics not valid");
               break;
        }
        return result;
    }
    */
    private Time parseTime(Element el){
        int sec;
        int nsec;

        sec = Integer.parseInt(el.getFirstChild().getFirstChild().getNodeValue());
        nsec = Integer.parseInt(el.getFirstChild().getNextSibling().getFirstChild().getNodeValue());

        return new Time(sec, nsec);
    }
}

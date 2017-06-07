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
 *
 */
package org.opensplice.cm.transform.xml;

import java.io.IOException;
import java.io.StringReader;
import java.util.logging.Level;
import java.util.logging.Logger;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;

import org.w3c.dom.Element;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import org.opensplice.cm.data.Sample;
import org.opensplice.cm.meta.MetaType;
import org.opensplice.cm.transform.SampleDeserializer;
import org.opensplice.cm.transform.TransformationException;

/**
 * The XML implementation of an SampleDeserializer. It is capable of 
 * transforming a serialized XML representation into an Entity object.
 * For parsing SAX is used.
 * 
 * @date May 14, 2004
 */
public class SampleDeserializerXML implements SampleDeserializer{
    /**
     * Creates a new deserializer, that is capable of transforming an XML Sample
     * to a Java representation. This is a concrete descendant of the 
     * SampleDeserializer interface.
     * 
     * @param type The type of the Sample.
     * @throws ParserConfigurationException Thrown if the parser could not be
     *                                      configured.
     * @throws SAXException Thrown if the parser could not be initialized.
     */
    public SampleDeserializerXML(MetaType type) throws ParserConfigurationException, SAXException{
        handler = new SampleHandler(type);
        factory = SAXParserFactory.newInstance();
        parser = factory.newSAXParser();
        es = new ElementSerializerXML();
        logger = Logger.getLogger("org.opensplice.api.cm.transform.xml");
    }
    
    public SampleDeserializerXML() throws ParserConfigurationException, SAXException{
        factory = SAXParserFactory.newInstance();
        parser = factory.newSAXParser();
        handler = null;
        es = new ElementSerializerXML();
        logger = Logger.getLogger("org.opensplice.api.cm.transform.xml");
    }
    
    @Override
    public synchronized Sample deserializeSample(Object serializedSample, MetaType type) throws TransformationException{
        handler = new SampleHandler(type);
        return this.deserializeSample(serializedSample);
    }
    
    @Override
    public synchronized Sample deserializeSample(Object serializedSample) throws TransformationException{
        if(handler == null){
            throw new TransformationException("Supplied Sample is not valid.");
        }
        Sample sample = null;
        
        if(serializedSample instanceof String){
            sample = this.deserializeSample((String)serializedSample);
        } else if(serializedSample instanceof Element){
            String xmlSample = es.serializeElement((Element)serializedSample);
            sample = this.deserializeSample(xmlSample);
        }
        return sample;
    }
    
    private Sample deserializeSample(String serializedSample) throws TransformationException{
        Sample sample = null;
        
        try {
            parser.parse(new InputSource(
                            new StringReader(serializedSample)), 
                            handler);
            sample = handler.getSample();
        }
        catch (SAXException e) {
            logger.logp(Level.SEVERE, "SampleDeserializerXML", "deserializeSample", 
                        "SAXException: " + e.getMessage() + 
                        "\nInput string:" + serializedSample);
            throw new TransformationException(e.getMessage());
        }
        catch (IOException e) {
            logger.logp(Level.SEVERE, "SampleDeserializerXML", "deserializeSample", 
                        "IOException: " + e.getMessage() + 
                        "\nInput string:" + serializedSample);
            throw new TransformationException(e.getMessage());
        }
        return sample;
    }
    
    /**
     * Factory that provides the SAX parser.
     */
    private SAXParserFactory factory;
    
    /**
     * Handler that is called when SAX parser events occur.
     */
    private SampleHandler handler;
    
    /**
     * The SAX parser that is user to parse the input.
     */
    private SAXParser parser;
    
    private ElementSerializerXML es;
    
    /**
     * Logging facilities for the parser.
     */
    private Logger logger;
}

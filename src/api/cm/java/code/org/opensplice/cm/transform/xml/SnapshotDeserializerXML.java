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

import org.opensplice.cm.com.Communicator;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import org.opensplice.cm.Reader;
import org.opensplice.cm.ReaderSnapshot;
import org.opensplice.cm.Writer;
import org.opensplice.cm.WriterSnapshot;
import org.opensplice.cm.impl.ReaderSnapshotImpl;
import org.opensplice.cm.impl.WriterSnapshotImpl;
import org.opensplice.cm.transform.SnapshotDeserializer;
import org.opensplice.cm.transform.TransformationException;

/**
 * The XML implementation of an SnapshotDeserializer. It is capable of 
 * transforming a serialized XML representation to an Snapshot object.
 * 
 * @date Oct 20, 2004 
 */
public class SnapshotDeserializerXML implements SnapshotDeserializer {
    /**
     * Creates a new deserializer, that is capable of transforming an XML
     * Snapshot to a Java representation. This is a concrete descendant of the 
     * SnapshotDeserializer interface.  
     *
     * @throws ParserConfigurationException Thrown if the parser could not be
     *                                      configured.
     */
    public SnapshotDeserializerXML(Communicator communicator) throws ParserConfigurationException {
        this.communicator = communicator;
        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        factory.setValidating(false);
        factory.setNamespaceAware(false);
        builder = factory.newDocumentBuilder(); 
        logger = Logger.getLogger("org.opensplice.api.cm.transform.xml");
    }
    
    public WriterSnapshot deserializeWriterSnapshot(Object serialized, Writer writer) throws TransformationException{
        if((serialized == null) || (writer == null)){
            throw new TransformationException("Supplied Snapshot or Writer is not valid.");
        }
        Document document;
        WriterSnapshot result = null;
        
        if(serialized instanceof String){
            String xmlEntity = (String)serialized;
            /*
            logger.logp(Level.FINEST, "SnapshotDeserializerXML", "deserializeWriterSnapshot", 
                                       "Deserializing snapshot from string:\n'" + 
                                       xmlEntity + "'");
            */       
            try {
                document = builder.parse(new InputSource(new StringReader(xmlEntity)));
            }
            catch (SAXException se) {
                logger.logp(Level.SEVERE,   "SnapshotDeserializerXML", 
                            "deserializeWriterSnapshot",
                            "SAXException occurred, snapshot could not be deserialized");
                throw new TransformationException(se.getMessage());
            }
            catch (IOException ie) {
                logger.logp(Level.SEVERE,   "SnapshotDeserializerXML", 
                        "deserializeReaderSnapshot",
                        "IOException occurred, snapshot could not be deserialized");
                throw new TransformationException(ie.getMessage());
            }
            
            if(document == null){
                throw new TransformationException("Supplied Snapshot is not valid.");
            }
            Element el = document.getDocumentElement();
            
            if("writerSnapshot".equals(el.getNodeName())){
                String id = el.getFirstChild().getFirstChild().getNodeValue();
                result = new WriterSnapshotImpl(getCommunicator(), id, writer);
            }
        }
        return result;
    }
    
    public ReaderSnapshot deserializeReaderSnapshot(Object serialized, Reader reader) throws TransformationException{
        if((serialized == null) || (reader == null)){
            throw new TransformationException("Supplied Snapshot or Reader is not valid.");
        }
        Document document;
        ReaderSnapshot result = null;
        
        if(serialized instanceof String){
            String xmlEntity = (String)serialized;
            /*
            logger.logp(Level.FINEST, "SnapshotDeserializerXML", "deserializeReaderSnapshot", 
                                       "Deserializing snapshot from string:\n'" + 
                                       xmlEntity + "'");       
            */
            try {
                document = builder.parse(new InputSource(new StringReader(xmlEntity)));
            }
            catch (SAXException se) {
                logger.logp(Level.SEVERE,   "SnapshotDeserializerXML", 
                            "deserializeReaderSnapshot",
                            "SAXException occurred, snapshot could not be deserialized");
                throw new TransformationException(se.getMessage());
            }
            catch (IOException ie) {
                logger.logp(Level.SEVERE,   "SnapshotDeserializerXML", 
                        "deserializeReaderSnapshot",
                        "IOException occurred, snapshot could not be deserialized");
                throw new TransformationException(ie.getMessage());
            }
            if(document == null){
                throw new TransformationException("Supplied Snapshot is not valid.");
            }
            
            Element el = document.getDocumentElement();
            
            if("readerSnapshot".equals(el.getNodeName())){
                String id = el.getFirstChild().getFirstChild().getNodeValue();
                result = new ReaderSnapshotImpl(getCommunicator(), id, reader);
            }
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

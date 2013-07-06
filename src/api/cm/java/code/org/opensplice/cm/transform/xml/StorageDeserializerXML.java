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
import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Attr;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;


import org.opensplice.cm.Storage;
import org.opensplice.cm.Storage.Result;
import org.opensplice.cm.data.UserData;
import org.opensplice.cm.impl.StorageImpl;
import org.opensplice.cm.meta.MetaType;
import org.opensplice.cm.transform.MetaTypeDeserializer;
import org.opensplice.cm.transform.StorageDeserializer;
import org.opensplice.cm.transform.TransformationException;
import org.opensplice.cm.DataTypeUnsupportedException;

/**
 *
 *
 * @date Oct 13, 2004
 */
public class StorageDeserializerXML implements StorageDeserializer{
    private static final String STORAGE_STRING = "rr_storage";
    private static final String STORAGERESULT_STRING = "rr_storageResult";
    private static final String STORAGEOPENRESULT_STRING = "rr_storageOpenResult";
    private static final String STORAGEREADRESULT_STRING = "rr_storageReadResult";
    private static final String STORAGEREADDATAXML_STRING = "rr_storageReadDataXML";
    private static final String STORAGEGETTYPERESULT_STRING = "rr_storageGetTypeResult";
    private static final String STORAGEGETTYPEMETADATA_STRING = "rr_storageType";
    private DocumentBuilder builder;
    private Logger logger;
    private MetaTypeDeserializer metaDeserializer;
    private ElementSerializerXML es;
    private SAXParserFactory saxFactory;


    public StorageDeserializerXML() throws ParserConfigurationException{
        DocumentBuilderFactory docFactory = DocumentBuilderFactory.newInstance();
        builder = docFactory.newDocumentBuilder();
        saxFactory = SAXParserFactory.newInstance();
        logger = Logger.getLogger("org.opensplice.api.cm.transform.xml");
        metaDeserializer = new MetaTypeDeserializerXML();
        es = new ElementSerializerXML();
    }

    public Object deserializeStorage(Object serialized) throws TransformationException {
        if(serialized == null){
            throw new TransformationException("Supplied Storage is not valid.");
        }
        Object storage = null;
        Document document;

        if(serialized instanceof String){
            String xmlStorage = (String)serialized;
            try {
                document = builder.parse(new InputSource(new StringReader(xmlStorage)));
            }
            catch (SAXException se) {
                logger.logp(Level.SEVERE,  "StorageDeserializerXML",
                                           "deserializeStorage",
                                           "SAXException occurred, Storage could not be deserialized");
                throw new TransformationException(se.getMessage());
            }
            catch (IOException ie) {
                logger.logp(Level.SEVERE,  "StorageDeserializerXML",
                                           "deserializeStorage",
                                           "IOException occurred, Storage could not be deserialized");
                throw new TransformationException(ie.getMessage());
            }
            if(document == null){
                throw new TransformationException("Supplied Storage is not valid.");
            }

            Element rootElement = document.getDocumentElement();
            storage = buildStorage(rootElement);
        }
        return storage;
    }

    private static Object buildStorage(Element rootElement){
        Object storage = null;

        if(STORAGE_STRING.equals(rootElement.getNodeName())){
            Node ptr = rootElement.getFirstChild();
            storage = ptr.getNodeValue();
        }

        return storage;
    }

    private static Result buildStorageResult(Element rootElement) throws TransformationException{
        Result r;

        if(STORAGERESULT_STRING.equals(rootElement.getNodeName())){
            String result = rootElement.getFirstChild().getNodeValue().toUpperCase();

            if(result.equals(Result.SUCCESS.name())){
                r = Result.SUCCESS;
            } else if (result.equals(Result.INVALID.name())){
                r = Result.INVALID;
            } else if (result.equals(Result.FAILED.name())){
                r = Result.FAILED;
            } else if (result.equals(Result.ERROR.name())){
                r = Result.ERROR;
            } else if (result.equals(Result.OUTOFRESOURCES.name())){
                r = Result.OUTOFRESOURCES;
            } else if (result.equals(Result.BUSY.name())) {
                r = Result.BUSY;
            } else if (result.equals(Result.NOTIMPLEMENTED.name())){
                r = Result.NOTIMPLEMENTED;
            } else {
                throw new TransformationException("Supplied XML StorageResult contains an unknown result.");
            }
        } else {
            throw new TransformationException("Supplied XML StorageResult doesn't start with tag <" + STORAGERESULT_STRING + ">.");
        }

        return r;
    }

    public Result deserializeStorageResult(Object serialized) throws TransformationException {
        if(serialized == null){
            throw new TransformationException("Supplied StorageResult is not valid.");
        }
        Result result = Result.ERROR;
        Document document;

        if(serialized instanceof String){
            String xmlStorageResult = (String)serialized;
            try {
                document = builder.parse(new InputSource(new StringReader(xmlStorageResult)));
            }
            catch (SAXException se) {
                logger.logp(Level.SEVERE,  "StorageDeserializerXML",
                                           "deserializeStorageResult",
                                           "SAXException occurred, StorageResult could not be deserialized");
                throw new TransformationException(se.getMessage());
            }
            catch (IOException ie) {
                logger.logp(Level.SEVERE,  "StorageDeserializerXML",
                                           "deserializeStorageResult",
                                           "IOException occurred, StorageResult could not be deserialized");
                throw new TransformationException(ie.getMessage());
            }
            if(document == null){
                throw new TransformationException("Supplied StorageResult is not valid.");
            }

            Element rootElement = document.getDocumentElement();

            result = buildStorageResult(rootElement);
        }
        return result;
    }

    public Result deserializeOpenResult_Result(Object serialized) throws TransformationException {
        if(serialized == null){
            throw new TransformationException("Supplied StorageOpenResult is not valid.");
        }
        Result result = Result.ERROR;
        Document document;

        if(serialized instanceof String){
            String xmlStorageOpenResult = (String)serialized;
            try {
                document = builder.parse(new InputSource(new StringReader(xmlStorageOpenResult)));
            }
            catch (SAXException se) {
                logger.logp(Level.SEVERE,  "StorageDeserializerXML",
                                           "deserializeStorageOpenResult_Result",
                                           "SAXException occurred, StorageOpenResult could not be deserialized");
                throw new TransformationException(se.getMessage());
            }
            catch (IOException ie) {
                logger.logp(Level.SEVERE,  "StorageDeserializerXML",
                                           "deserializeStorageOpenResult_Result",
                                           "IOException occurred, StorageOpenResult could not be deserialized");
                throw new TransformationException(ie.getMessage());
            }
            if(document == null){
                throw new TransformationException("Supplied StorageOpenResult is not valid.");
            }

            Element rootElement = document.getDocumentElement();

            if(STORAGEOPENRESULT_STRING.equals(rootElement.getNodeName())){
                NodeList children;

                children = rootElement.getElementsByTagName(STORAGERESULT_STRING);
                assert(children.getLength() == 1);
                result = buildStorageResult((Element)children.item(0));
            }

        }
        return result;
    }

    public Object deserializeOpenResult_Storage(Object serialized) throws TransformationException {
        if(serialized == null){
            throw new TransformationException("Supplied StorageOpenResult is not valid.");
        }
        Object storage = null;
        Document document;

        if(serialized instanceof String){
            String xmlStorageOpenResult = (String)serialized;
            try {
                document = builder.parse(new InputSource(new StringReader(xmlStorageOpenResult)));
            }
            catch (SAXException se) {
                logger.logp(Level.SEVERE,  "StorageDeserializerXML",
                                           "deserializeStorageOpenResult_Storage",
                                           "SAXException occurred, StorageOpenResult could not be deserialized");
                throw new TransformationException(se.getMessage());
            }
            catch (IOException ie) {
                logger.logp(Level.SEVERE,  "StorageDeserializerXML",
                                           "deserializeStorageOpenResult_Storage",
                                           "IOException occurred, StorageOpenResult could not be deserialized");
                throw new TransformationException(ie.getMessage());
            }
            if(document == null){
                throw new TransformationException("Supplied StorageOpenResult is not valid.");
            }

            Element rootElement = document.getDocumentElement();

            if(STORAGEOPENRESULT_STRING.equals(rootElement.getNodeName())){
                NodeList children;

                children = rootElement.getElementsByTagName(STORAGE_STRING);
                assert(children.getLength() == 1);
                storage = buildStorage((Element)children.item(0));
            }

        }
        return storage;
    }

    public Result deserializeReadResult_Result(Object serialized) throws TransformationException {
        if(serialized == null){
            throw new TransformationException("Supplied StorageReadResult is not valid.");
        }
        Result result = Result.ERROR;
        Document document;

        if(serialized instanceof String){
            String xmlStorageReadResult = (String)serialized;
            try {
                document = builder.parse(new InputSource(new StringReader(xmlStorageReadResult)));
            }
            catch (SAXException se) {
                logger.logp(Level.SEVERE,  "StorageDeserializerXML",
                                           "deserializeStorageReadResult_Result",
                                           "SAXException occurred, StorageReadResult could not be deserialized");
                throw new TransformationException(se.getMessage());
            }
            catch (IOException ie) {
                logger.logp(Level.SEVERE,  "StorageDeserializerXML",
                                           "deserializeStorageReadResult_Result",
                                           "IOException occurred, StorageReadResult could not be deserialized");
                throw new TransformationException(ie.getMessage());
            }
            if(document == null){
                throw new TransformationException("Supplied StorageReadResult is not valid.");
            }

            Element rootElement = document.getDocumentElement();

            if(STORAGEREADRESULT_STRING.equals(rootElement.getNodeName())){
                NodeList children;

                children = rootElement.getElementsByTagName(STORAGERESULT_STRING);
                assert(children.getLength() == 1);
                result = buildStorageResult((Element)children.item(0));
            }

        }
        return result;
    }

    public UserData deserializeReadResult_Data(Object serialized, MetaType type) throws TransformationException {
        if(serialized == null){
            throw new TransformationException("Supplied StorageReadResult is not valid.");
        }
        UserData result = null;
        Document document;

        if(serialized instanceof String){
            String xmlStorageReadResult = (String)serialized;
            try {
                document = builder.parse(new InputSource(new StringReader(xmlStorageReadResult)));
            }
            catch (SAXException se) {
                logger.logp(Level.SEVERE,  "StorageDeserializerXML",
                                           "deserializeStorageReadResult_Data",
                                           "SAXException occurred, StorageReadResult could not be deserialized");
                throw new TransformationException(se.getMessage());
            }
            catch (IOException ie) {
                logger.logp(Level.SEVERE,  "StorageDeserializerXML",
                                           "deserializeStorageReadResult_Data",
                                           "IOException occurred, StorageReadResult could not be deserialized");
                throw new TransformationException(ie.getMessage());
            }
            if(document == null){
                throw new TransformationException("Supplied StorageReadResult is not valid.");
            }

            Element rootElement = document.getDocumentElement();

            if(STORAGEREADRESULT_STRING.equals(rootElement.getNodeName())){
                NodeList children;
                String xmlData;
                SAXParser parser;
                UserDataHandler udh = new UserDataHandler(type);

                children = rootElement.getElementsByTagName(STORAGEREADDATAXML_STRING);
                assert(children.getLength() == 1);
                /* TODO: xmlData will not contain the typeName-attribute anymore,
                 * since serializeElement doesn't do attributes.
                 * TODO: remove this parse (DOM), serialize to XML from DOM,
                 * parse (SAX) detour...*/
                xmlData = es.serializeElement((Element)children.item(0).getFirstChild());

                try {
                    parser = saxFactory.newSAXParser();

                    parser.parse(new InputSource(
                                    new StringReader(xmlData)),
                                    udh);
                    result = udh.getUserData();
                }
                catch (ParserConfigurationException e){
                    logger.logp(Level.SEVERE, "StorageDeserializerXML", "deserializeReadResult_Data",
                            "ParserConfigurationException: " + e.getMessage() );
                    throw new TransformationException(e.getMessage());
                }
                catch (SAXException e) {
                    logger.logp(Level.SEVERE, "StorageDeserializerXML", "deserializeReadResult_Data",
                                "SAXException: " + e.getMessage() +
                                "\nInput string:" + xmlData);
                    throw new TransformationException(e.getMessage());
                }
                catch (IOException e) {
                    logger.logp(Level.SEVERE, "StorageDeserializerXML", "deserializeReadResult_Data",
                                "IOException: " + e.getMessage() +
                                "\nInput string:" + xmlData);
                    throw new TransformationException(e.getMessage());
                }
            }
        }
        return result;
    }

    public String deserializeReadResult_DataTypeName(Object serialized) throws TransformationException
    {
        if(serialized == null){
            throw new TransformationException("Supplied StorageReadResult is not valid.");
        }
        String result = null;
        Document document;

        if(serialized instanceof String){
            String xmlStorageReadResult = (String)serialized;
            try {
                document = builder.parse(new InputSource(new StringReader(xmlStorageReadResult)));
            }
            catch (SAXException se) {
                logger.logp(Level.SEVERE,  "StorageDeserializerXML",
                                           "deserializeReadResult_DataTypeName",
                                           "SAXException occurred, StorageReadResult could not be deserialized");
                throw new TransformationException(se.getMessage());
            }
            catch (IOException ie) {
                logger.logp(Level.SEVERE,  "StorageDeserializerXML",
                                           "deserializeReadResult_DataTypeName",
                                           "IOException occurred, StorageReadResult could not be deserialized");
                throw new TransformationException(ie.getMessage());
            }
            if(document == null){
                throw new TransformationException("Supplied StorageReadResult is not valid.");
            }

            Element rootElement = document.getDocumentElement();

            if(STORAGEREADRESULT_STRING.equals(rootElement.getNodeName())){
                NodeList children;
                Element objElem;

                children = rootElement.getElementsByTagName(STORAGEREADDATAXML_STRING);

                /* The child of the STORAGEREADDATAXML_STRING element should have a
                 * 'type' attribute. Typically the child element is named 'object'. */
                if(children.item(0).hasChildNodes()){
                    objElem = (Element)children.item(0).getFirstChild();
                    result = objElem.getAttribute("type");
                }
            }

        }
        return result;
    }

    public MetaType deserializeGetTypeResult_Metadata(Object serialized) throws TransformationException, DataTypeUnsupportedException {
        if(serialized == null){
            throw new TransformationException("Supplied GetTypeResult is not valid.");
        }
        MetaType result = null;
        Document document;

        if(serialized instanceof String){
            String xmlStorageGetTypeResult = (String)serialized;
            try {
                document = builder.parse(new InputSource(new StringReader(xmlStorageGetTypeResult)));
            }
            catch (SAXException se) {
                logger.logp(Level.SEVERE,  "StorageDeserializerXML",
                                           "deserializeGetTypeResult_Metadata",
                                           "SAXException occurred, GetTypeResult could not be deserialized");
                throw new TransformationException(se.getMessage());
            }
            catch (IOException ie) {
                logger.logp(Level.SEVERE,  "StorageDeserializerXML",
                                           "deserializeGetTypeResult_Metadata",
                                           "IOException occurred, GetTypeResult could not be deserialized");
                throw new TransformationException(ie.getMessage());
            }
            if(document == null){
                throw new TransformationException("Supplied StorageReadResult is not valid.");
            }

            Element rootElement = document.getDocumentElement();

            if(STORAGEGETTYPERESULT_STRING.equals(rootElement.getNodeName())){
                NodeList children;

                children = rootElement.getElementsByTagName(STORAGEGETTYPEMETADATA_STRING);
                assert(children.getLength() == 1);
                children = ((Element)children.item(0)).getChildNodes();
                assert(children.getLength() == 1);
                result = metaDeserializer.deserializeMetaType(children.item(0));
            }
        }
        return result;
    }
}

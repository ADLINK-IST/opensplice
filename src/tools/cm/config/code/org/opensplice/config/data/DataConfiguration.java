/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
package org.opensplice.config.data;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.StringWriter;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.w3c.dom.Attr;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.w3c.dom.Text;
import org.xml.sax.ErrorHandler;
import org.xml.sax.SAXException;
import org.xml.sax.SAXParseException;

import org.opensplice.config.meta.MetaAttribute;
import org.opensplice.config.meta.MetaConfiguration;
import org.opensplice.config.meta.MetaElement;
import org.opensplice.config.meta.MetaNode;
import org.opensplice.config.meta.MetaValue;

public class DataConfiguration {
    private DataElement rootElement;
    private ArrayList<DataElement> services;
    private Document document;
    private MetaConfiguration metadata;
    private Set<DataConfigurationListener> listeners;
    private File file;
    private boolean fileUpToDate;
       
    
    public DataConfiguration(MetaConfiguration metadata, File file, boolean repair) throws DataException {
        try {
            this.metadata     = metadata;
            this.rootElement  = null;
            this.services     = new ArrayList<DataElement>();
            
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            DocumentBuilder builder = factory.newDocumentBuilder();
            
            builder.setErrorHandler(new ErrorHandler(){
                public void warning(SAXParseException exception) throws SAXException {
                    
                }

                public void error(SAXParseException exception) throws SAXException {
                    System.err.println("Parse error at line: " + exception.getLineNumber() +
                                        " column: " + exception.getColumnNumber() + ".");
                    
                }

                public void fatalError(SAXParseException exception) throws SAXException {
                    System.err.println("Parse error at line: " + exception.getLineNumber() +
                                        " column: " + exception.getColumnNumber() + ".");
                }
            });
            this.document     = builder.parse(file);
            this.listeners    = Collections.synchronizedSet(new HashSet<DataConfigurationListener>());
            this.fileUpToDate = true;
            this.initExisting(repair);
            this.file         = file;
        } catch (SAXException se) {
            throw new DataException(se.getMessage());
        } catch (IOException ie) {
            throw new DataException(ie.getMessage());
        } catch (ParserConfigurationException pe) {
            throw new DataException(pe.getMessage());
        }
    }
    
    public DataConfiguration(File file, boolean repair) throws DataException {
        this(MetaConfiguration.getInstance(), file, repair);
    }
    
    public DataConfiguration(MetaConfiguration metadata) throws DataException {
        try {
            this.metadata     = metadata;
            this.rootElement  = null;
            this.services     = new ArrayList<DataElement>();
            this.listeners    = Collections.synchronizedSet(new HashSet<DataConfigurationListener>());
            this.fileUpToDate = false;
            this.file         = null;
            this.initDocument();
            this.init();
        } catch (NullPointerException npe){
            throw new DataException(npe.getMessage());
        }
    }
    
    public DataConfiguration() throws DataException {
        this(MetaConfiguration.getInstance());
    }
    
    public Document getDocument() {
        return this.document;
    }

    public MetaConfiguration getMetadata() {
        return this.metadata;
    }

    public DataElement getRootElement() {
        return this.rootElement;
    }

    public DataElement[] getServices() {
        return this.services.toArray(new DataElement[this.services.size()]);
    }
    
    public void addDataConfigurationListener(DataConfigurationListener listener){
        synchronized(this.listeners){
            if(this.listeners.contains(listener)){
                assert false;
            }
                
            this.listeners.add(listener);
        }
    }
    
    public void removeDataConfigurationListener(DataConfigurationListener listener){
        synchronized(listeners){
            listeners.remove(listener);
        }
    }
    
    public void addNode(DataElement parent, DataNode child) throws DataException {
        if((parent.getOwner().equals(this))){
            parent.addChild(child);
            
            if(parent.equals(this.rootElement)){
                if(child instanceof DataElement){
                    this.services.add((DataElement)child);
                }
            }
            this.fileUpToDate = false;
            this.notifyNodeAdded(parent, child);
        } else {
            throw new DataException("Parent node not in configuration.");
        }
    }
    
    public void addNode(DataElement parent, MetaNode child) throws DataException {
        if((parent.getOwner().equals(this))){
            DataNode added = this.createDataForMeta(parent, child);
            
            if(parent.equals(this.rootElement)){
                if(added instanceof DataElement){
                    this.services.add((DataElement)added);
                }
            }
            this.fileUpToDate = false;
            this.notifyNodeAdded(parent, added);
        } else {
            throw new DataException("Parent node not in configuration.");
        }
    }
    
    public void removeNode(DataNode child) throws DataException {
        if(child == null){
            throw new DataException("Invalid node.");
        } 
        DataElement parent = (DataElement)child.getParent();
        
        if(parent == null){
            if((child.getOwner().equals(this))){
                parent = this.rootElement;
                parent.removeChild(child);
                
                if(child instanceof DataElement){
                    this.services.remove((DataElement)child);
                }
                this.fileUpToDate = false;
                this.notifyNodeRemoved(parent, child);
            } else {
                throw new DataException("Node not in configuration.");
            }
        } else if(parent.getOwner().equals(this)){
            parent.removeChild(child);
            
            if(parent.equals(this.rootElement)){
                if(child instanceof DataElement){
                    this.services.remove((DataElement)child);
                }
            }
            this.fileUpToDate = false;
            this.notifyNodeRemoved(parent, child);
        } else {
            throw new DataException("Parent and/or child node not in configuration.");
        }
    }
    
    public void setFile(File file) throws DataException{
        if(file == null){
            throw new DataException("Invalid file pointer provided");
        }
        this.file = file;
    }
    
    public File getFile(){
        return this.file;
    }
    
    public boolean isUpToDate(){
        return this.fileUpToDate;
    }
    
    public void store(boolean replaceOld) throws DataException{
        FileWriter writer;
        if(this.file == null){
            throw new DataException("No file specified.");
        }
        if((this.file.exists()) && (replaceOld == false)){
            throw new DataException("File already exists.");
        }
        if(!this.file.exists()){
            try {
                this.file.createNewFile();
            } catch (IOException e) {
                throw new DataException("Cannot create file: " + file.getAbsolutePath());
            }
        } else if(!file.canWrite()){
            throw new DataException("Cannot write to: " + file.getAbsolutePath());
        }
        try {
            writer = new FileWriter(this.file, false);
            writer.write(this.toString());
            writer.close();
        } catch (IOException e) {
            throw new DataException(e.getMessage());
        }
        this.fileUpToDate = true;
    }
    
    public void setValue(DataValue dataValue, Object value) throws DataException {
        Object oldValue;
        
        if(dataValue.getOwner().equals(this)){
            oldValue = dataValue.getValue();
            dataValue.setValue(value);
            this.fileUpToDate = false;
            this.notifyValueChanged(dataValue, oldValue, value);
        } else {
            throw new DataException("Parent and/or child node not in configuration.");
        }
    }
    
    public String toString(){
        String result = null;
        
        try{
            TransformerFactory tFactory = TransformerFactory.newInstance();
            Transformer transformer = tFactory.newTransformer();
            transformer.setOutputProperty(OutputKeys.OMIT_XML_DECLARATION, "yes");
            transformer.setOutputProperty(OutputKeys.METHOD, "xml");
            transformer.setOutputProperty(OutputKeys.INDENT, "no");
            
            this.beautify(this.document.getDocumentElement());
                        
            DOMSource source = new DOMSource(this.document);
            StringWriter writer = new StringWriter();
            StreamResult sr = new StreamResult(writer);
            transformer.transform(source, sr);
            writer.flush();
            result = writer.toString();
            
            int indent = 0;
            String tab = "   ";
            StringWriter sw = new StringWriter();
            char[] characters = result.toCharArray();
            char c, prevC;
            prevC = '-';
            boolean inTag = false;
            boolean inCdata = false;
            boolean inClosingTag = false;
            
            for(int i=0; i<characters.length; i++){
                c = characters[i];
                sw.append(c);
                
                if(c == '\n'){
                    if((characters.length > i + 2) && (!inCdata)){
                        if((characters[i+1] == '<') && (characters[i+2] == '!')){
                            /*Write no tabs.*/
                        } else if((characters[i+1] == '<') && (characters[i+2] == '/')){
                            for(int j=0; j<indent-1; j++){
                                sw.write(tab);
                            }
                        } else {
                            for(int j=0; j<indent; j++){
                                sw.write(tab);
                            }
                        }
                    } else if(inCdata){
                        /*Write no tabs.*/
                    } else {
                        for(int j=0; j<indent; j++){
                            sw.write(tab);
                        }
                    }
                } else if(c == '<'){
                    if(characters[i+1] == '/'){
                        inClosingTag = true;
                        indent--;
                    } else if(characters[i+1] == '!'){
                        inCdata = true;
                    } else if(characters[i+1] != '/'){
                        inClosingTag = false;
                        indent++;
                    }
                    inTag = true;
                } else if(c == '>'){
                    if(inTag){
                        if(prevC == ']' || prevC == '-'){
                            inCdata = false;
                        } else if((inCdata == false) && (prevC == '/')){
                            indent--;
                            /*added*/
                            inClosingTag = true;
                            /*end added*/
                        }
                        inTag = false;
                        
                        if(characters.length > (i+2)){
                            if(characters[i+1] == '<'){
                                if(characters[i+2] != '/'){
                                    sw.append('\n');
                                    
                                    for(int j=0; j<indent; j++){
                                        sw.write(tab);
                                    }
                                } 
                                /*added*/
                                else if(inClosingTag){
                                    sw.append("\n");
                                    for(int j=1; j<indent; j++){
                                        sw.write(tab);
                                    }
                                }
                                /*end added*/
                            }
                        }
                    }
                }
                prevC = c;
            }
            sw.append("\n\n");
            result = sw.toString();
        } catch (TransformerConfigurationException tce) {
            System.err.println(tce.getMessage());
        } catch (TransformerException te) {
            System.err.println(te.getMessage());
        }
        return result;
    }
  
    private DataNode createDataForMeta(DataElement parent, MetaNode metaNode) throws DataException {
        DataNode node;
        Document dom;
        Node domNode;
        Text text;
        DataNode result = null;
        
        dom = this.getDocument();
        
        if(metaNode instanceof MetaElement){
            domNode = dom.createElement(((MetaElement)metaNode).getName());
            node = new DataElement((MetaElement)metaNode, (Element)domNode);
            result = parent.addChild(node);
            
            for(MetaNode mn: ((MetaElement)metaNode).getChildren()){
                if(mn instanceof MetaAttribute){
                    if(((MetaAttribute)mn).isRequired()){
                       this.createDataForMeta((DataElement)node, mn); 
                    }
                } else if(mn instanceof MetaElement){
                    for(int i=0; i<((MetaElement)mn).getMinOccurrences(); i++){
                        this.createDataForMeta((DataElement)node, mn);
                    }
                } else if(mn instanceof MetaValue){
                    text = dom.createTextNode(((MetaValue)mn).getDefaultValue().toString());
                    DataValue dv = new DataValue((MetaValue)mn, text);
                    ((DataElement)node).addChild(dv);
                } else {
                    assert false;
                }
            }
        } else if(metaNode instanceof MetaAttribute){
            domNode = dom.createAttribute(((MetaAttribute)metaNode).getName());
            node = new DataAttribute((MetaAttribute)metaNode, (Attr)domNode);
            result = parent.addChild(node);
        } else if(metaNode instanceof MetaValue){
            domNode = dom.createTextNode(((MetaValue)metaNode).getDefaultValue().toString());
            node = new DataValue((MetaValue)metaNode, (Text)domNode);
            result = parent.addChild(node);
        } else {
            throw new DataException("Unknown meta type: " + metaNode.getClass());
        }
        return result;
    }
    
    private void initDocument() throws DataException {
        try {
            this.document    = DocumentBuilderFactory.newInstance().newDocumentBuilder().newDocument();
        } catch (ParserConfigurationException e) {
            throw new DataException(e.getMessage());
        } catch (NullPointerException npe){
            throw new DataException(npe.getMessage());
        } 
    }
    
    private void initExisting(boolean repair) throws DataException{
        Element domElement;
        MetaElement metaElement;
                
        metaElement = this.metadata.getRootElement();
        domElement  = this.document.getDocumentElement();
        
        if((domElement != null) && (metaElement.getName().equals(domElement.getNodeName()))){
            this.rootElement = new DataElement(metaElement, domElement);
            this.rootElement.setOwner(this);
            this.initExistingDataElement(metaElement, this.rootElement, domElement, repair);
            
            for(DataNode dn: this.rootElement.getChildren()){
                if(dn instanceof DataElement){
                    this.services.add((DataElement)dn);
                }
            }
        } else if(repair && (domElement == null)){
            this.initDocument();
            this.init();
        } else if(repair){
            this.document.renameNode(domElement, null, metaElement.getName());
            this.initExisting(repair);
        } else {
            if(domElement != null){
                throw new DataException("RootElement should be: '" + 
                        metaElement.getName() + "', but is '" + 
                        domElement.getNodeName() + "'.");
            } else {
                throw new DataException("RootElement should be: '" + 
                        metaElement.getName() + "', but is 'null'.");
            }
        }
    }
    
    private void initExistingDataElement(MetaElement metadata, DataElement parent, Element element, boolean repair) throws DataException {
        int occurrences;
        NodeList children;
        NamedNodeMap attributes;
        Node child;
        Element childElement;
        Text childText;
        Attr childAttribute;
        MetaElement childMetaElement;
        MetaValue childMetaValue;
        MetaAttribute childMetaAttribute;
        DataElement childDataElement;
        DataValue childDataValue;
        DataAttribute childDataAttribute;
        
        children = element.getChildNodes();
        int count = children.getLength();
                
        for(int i=0; i<count; i++){
            child = children.item(i);
            
            if(child instanceof Element){
                childElement = (Element)child;
                childMetaElement = this.findChildElement(metadata, childElement.getNodeName());
                
                if(childMetaElement != null){
                    childDataElement = new DataElement(childMetaElement, childElement);
                    try {
                        parent.addChild(childDataElement, false);
                    } catch (DataException de) {
                        if(!repair){
                            throw de;
                        }
                    }
                    childDataElement.setOwner(this);
                    this.initExistingDataElement(childMetaElement, childDataElement, childElement, repair);
                }
            } else if(child instanceof Text){
                childText = (Text)child;
                childMetaValue = this.findChildValue(metadata);
                
                if(childMetaValue != null){
                    try {
                        childDataValue = new DataValue(childMetaValue, childText);
                    } catch (DataException de) {
                       if(repair){
                           childText = this.document.createTextNode(childMetaValue.getDefaultValue().toString());
                           childDataValue = new DataValue(childMetaValue, childText);
                       } else {
                           throw de;
                       }
                    }
                    try {
                        parent.addChild(childDataValue, false);
                    } catch (DataException de) {
                        if(!repair){
                            throw de;
                        }
                    }
                }
            }
        }
        attributes = element.getAttributes();
        
        for(int i=0; i<attributes.getLength(); i++){
            child = attributes.item(i);
            
            if(child instanceof Attr){
                childAttribute = (Attr)child;
                childMetaAttribute = this.findChildAttribute(metadata, childAttribute.getName());
                
                if(childMetaAttribute != null){
                    try {
                        childDataAttribute = new DataAttribute(childMetaAttribute, childAttribute, childAttribute.getNodeValue());
                    } catch (DataException de) {
                        if(repair){
                            childDataAttribute = new DataAttribute(childMetaAttribute, childAttribute, childMetaAttribute.getValue().getDefaultValue());
                        } else {
                            throw de;
                        }
                    }
                    try {
                        parent.addChild(childDataAttribute, false);
                    } catch (DataException de) {
                        if(!repair){
                            throw de;
                        } else {
                            /*Ignore it.*/
                        }
                    }
                }
            }
        }        
        /*All data has been added as far as it goes. Now check if data is missing*/
        for(MetaNode mn: metadata.getChildren()){
            if(mn instanceof MetaElement){
                childMetaElement = (MetaElement)mn;
                occurrences = this.countChildElementOccurrences((Element)parent.getNode(), childMetaElement.getName());
                                
                if(occurrences < childMetaElement.getMinOccurrences()){
                    if(repair){
                        while(occurrences < childMetaElement.getMinOccurrences()){
                            childElement = this.document.createElement(childMetaElement.getName());
                            element.appendChild(childElement);
                            childDataElement = new DataElement(childMetaElement, childElement);
                            parent.addChild(childDataElement);
                            this.initDataElement(childDataElement);
                            occurrences++;
                        }
                        occurrences = 0;
                    } else {
                        throw new DataException("Found only " + occurrences + 
                                " occurrences for element '" + 
                                childMetaElement.getName() +
                                "', but expected at least " + 
                                childMetaElement.getMinOccurrences());
                    }
                }
            } else if(mn instanceof MetaAttribute){
                childMetaAttribute = (MetaAttribute)mn;
                
                if(childMetaAttribute.isRequired()){
                    if(!element.hasAttribute(childMetaAttribute.getName())){
                        childAttribute = this.document.createAttribute(childMetaAttribute.getName());
                        childDataAttribute = new DataAttribute(childMetaAttribute, 
                                childAttribute, 
                                childMetaAttribute.getValue().getDefaultValue());
                        parent.addChild(childDataAttribute);
                    }
                }
            } else if(mn instanceof MetaValue){
                childMetaValue = (MetaValue)mn;
                occurrences = this.countChildElementOccurrences((Element)parent.getNode(), null);
                
                if(occurrences == 0){
                    childText = this.document.createTextNode(childMetaValue.getDefaultValue().toString());                    
                    childDataValue = new DataValue(childMetaValue, childText);
                    parent.addChild(childDataValue);
                }
            }
        }
        assert parent.getOwner() != null: "Owner == null";
        
        for(DataNode dn: parent.getChildren()){
            assert dn.getOwner() != null: "Owner == null (2)";
        }
    }
    
    private void init() throws DataException{
        Element domElement, de;
        MetaElement metaElement;
        DataElement dataElement;
        int occurrences = 0;
        
        metaElement = this.metadata.getRootElement();
        domElement  = this.document.createElement(metaElement.getName());
        
        this.document.appendChild(domElement);
        domElement.setAttribute("version", Double.toString(this.metadata.getVersion()));
        
        this.rootElement = new DataElement(metaElement, domElement);
        this.rootElement.setOwner(this);
        //this.initDataElement(this.rootElement);
        
        for(MetaElement me: this.metadata.getServices()){
            while(occurrences < me.getMinOccurrences()){
                de = this.document.createElement(me.getName());
                domElement.appendChild(de);
                dataElement = new DataElement(me, de);
                this.rootElement.addChild(dataElement);
                this.initDataElement(dataElement);
                this.services.add(dataElement);
                occurrences++;
            }
            occurrences = 0;
        }
    }
    
    private void initDataElement(DataElement data) throws DataException{
        MetaAttribute ma;
        MetaElement me;
        MetaValue mv;
        DataAttribute da;
        DataElement de;
        DataValue dv;
        Attr attribute;
        Element element;
        Text text;
        int occurrences;
        
        MetaElement meta = (MetaElement)data.getMetadata();
        occurrences = 0;
        
        for(MetaNode mn: meta.getChildren()){
            if(mn instanceof MetaAttribute){
                ma = (MetaAttribute)mn;
                
                if(ma.isRequired()){
                    attribute = this.document.createAttribute(ma.getName());
                    da        = new DataAttribute(ma, attribute, ma.getValue().getDefaultValue());
                    data.addChild(da);
                }
            } else if(mn instanceof MetaElement){
                me = (MetaElement)mn;
                
                while(occurrences < me.getMinOccurrences()){
                    element = this.document.createElement(me.getName());
                    de      = new DataElement(me, element);
                    data.addChild(de);
                    this.initDataElement(de);
                    occurrences++;
                }
                occurrences = 0;
            } else if(mn instanceof MetaValue){
                mv   = (MetaValue)mn;
                text = this.document.createTextNode(mv.getDefaultValue().toString());
                dv   = new DataValue(mv, text);
                data.addChild(dv);
            }
        }
    }
    
    private MetaElement findChildElement(MetaElement element, String name){
        MetaElement child = null;
        MetaNode[] children = element.getChildren();
        
        for(int i=0; (i<children.length) && (child==null); i++){
            if(children[i] instanceof MetaElement){
                if(((MetaElement)children[i]).getName().equals(name)){
                    child = (MetaElement)children[i];
                }
            }
        }
        return child;
    }
    
    private MetaValue findChildValue(MetaElement element){
        MetaValue child = null;
        MetaNode[] children = element.getChildren();
        
        for(int i=0; (i<children.length) && (child==null); i++){
            if(children[i] instanceof MetaValue){
                child = (MetaValue)children[i];
            }
        }
        return child;
    }
    
    private MetaAttribute findChildAttribute(MetaElement element, String name){
        MetaAttribute child = null;
        MetaNode[] children = element.getChildren();
        
        for(int i=0; (i<children.length) && (child==null); i++){
            if(children[i] instanceof MetaAttribute){
                if(((MetaAttribute)children[i]).getName().equals(name)){
                    child = (MetaAttribute)children[i];
                }
            }
        }
        return child;
    }
    
    private int countChildElementOccurrences(Element element, String childName){
        int occurrences = 0;
        NodeList children = element.getChildNodes();
        
        for(int i=0; i<children.getLength(); i++){
            if(childName == null){
                if(children.item(i).getNodeName().equals("#text")){
                    occurrences++;
                }
            } else if(childName.equals(children.item(i).getNodeName())){
                occurrences++;
            }
        }
        return occurrences;
    }
    
    private void notifyNodeAdded(DataElement parent, DataNode addedNode){
        synchronized(this.listeners){
            for(DataConfigurationListener listener: this.listeners){
                listener.nodeAdded(parent, addedNode);
            }
        }
    }
    
    private void notifyNodeRemoved(DataElement parent, DataNode removedNode){
        synchronized(this.listeners){
            for(DataConfigurationListener listener: this.listeners){
                listener.nodeRemoved(parent, removedNode);
            }
        }
    }
    
    private void notifyValueChanged(DataValue data, Object oldValue, Object newValue){
        synchronized(this.listeners){
            for(DataConfigurationListener listener: this.listeners){
                listener.valueChanged(data, oldValue, newValue);
            }
        }
    }
    
    private void beautify(Node node){
        NodeList children;
        String value;
        
        if(node instanceof Element){
            children = ((Element)node).getChildNodes();
            
            for(int i=0; i<children.getLength(); i++){
                this.beautify(children.item(i));
            }
        } else if(node instanceof Text){
            value = node.getNodeValue();
            node.setNodeValue(value.trim());
        }
        return;
    }
}

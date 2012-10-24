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
 package org.opensplice.cm.transform.xml;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.ListIterator;
import java.util.StringTokenizer;
import java.util.logging.Level;
import java.util.logging.Logger;

import org.xml.sax.Attributes;
import org.xml.sax.Locator;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;

import org.opensplice.cm.Time;
import org.opensplice.cm.data.GID;
import org.opensplice.cm.data.Message;
import org.opensplice.cm.data.Sample;
import org.opensplice.cm.data.State;
import org.opensplice.cm.data.UserData;
import org.opensplice.cm.meta.MetaClass;
import org.opensplice.cm.meta.MetaCollection;
import org.opensplice.cm.meta.MetaEnum;
import org.opensplice.cm.meta.MetaField;
import org.opensplice.cm.meta.MetaPrimitive;
import org.opensplice.cm.meta.MetaStruct;
import org.opensplice.cm.meta.MetaType;
import org.opensplice.cm.meta.MetaUnion;
import org.opensplice.cm.meta.MetaUnionCase;
import org.opensplice.cm.qos.MessageQoS;
import org.opensplice.cm.qos.ReliabilityKind;
import org.opensplice.cm.qos.ReliabilityPolicy;

/**
 * Offers facilities to deserialize a Sample. It is being triggered by
 * the SAX parser. 
 * 
 * @date May 13, 2004
 */
public class SampleHandler extends DefaultHandler {
    /**
     * Constructs a new SampleHandler that is able to deserialize a Sample from
     * XML to a Java representation.
     * 
     * @param type The type of the userdata in the Sample.
     */    
    public SampleHandler(MetaType type){
        userDataType = type;
        sample = null;
        message = null;
        userData = null;
        logger = Logger.getLogger("org.opensplice.api.cm.transform.xml");
    }
    
    /**
     * Adds a field to the current scope.
     * 
     * @param field The field to add to the scope.
     */
    private void scopeAdd(String field){
        fieldScope.add(field);        
    }
    
    /**
     * Removes the last field from the scope.
     */
    private void scopeRemoveLast(){
        fieldScope.removeLast();
    }
    
    /**
     * Provides access to the current scope.
     * 
     * @return The current scope. Fieldnames are '.' separated.
     */
    private String scopeGet(){
        String result = null;
        boolean first = true;
        ListIterator li = fieldScope.listIterator();
        
        while(li.hasNext()){
            if(first){
                result = (String)(li.next());
                first = false;
            }
            else{
                result += "." + (String)(li.next());
            }
        }
        return result;
    }
     
    /**
     * Provides the current index to address the supplied collection.
     * 
     @verbatim
      Example:
       
      IDL:
      long arr[2][3];
      
      Result:
      The first time "[0][0]" will be returned, the second time "[0][1]" and so on.
     @endverbatim
     *  
     * @param fieldName The name of the collection.
     * @param depth The depth of the index of the collection to look up. This is 
     *              used for collections within collections.
     * @return A String representation of the current indices of the collection.
     */ 
    private String getIndexFieldElement(String fieldName, int depth){
        ArrayList indices = (ArrayList)(fieldElementIndices.get(fieldName));
        String result = "";
        MetaCollection colType = (MetaCollection)(userDataType.getField(fieldName));        
        
        if(indices == null){
            ArrayList newList = new ArrayList(depth);
        
            for(int i=0; i<depth; i++){
                newList.add("0");
                result += "[0]";
            }
            fieldElementIndices.put(fieldName, newList);
        } 
        else{
            int index, subDepth;
            String strIndex;
            boolean prevAdd = true;     
            boolean isAdded = false;    //true if one index has already been added with 1
        
            for(int i=depth-1; i>=0; i--){
                index = Integer.parseInt((String)(indices.get(i)));
                subDepth = colType.getRecursiveMaxSize(i);
                
                if(index == (subDepth - 1 ) && (!isAdded)){
                    strIndex = Integer.toString(0);
                    prevAdd = true;
                }
                else if(prevAdd && (!isAdded)){
                    strIndex = Integer.toString(index + 1);
                    isAdded = true;
                    prevAdd = false;
                }
                else{
                    strIndex = Integer.toString(index);
                }
                indices.set(i, strIndex);
                result += "[" + strIndex + "]";
            }
        }
        return result;
    }
        
    /**
     * Triggered by the parser to notify that the end of the XML input
     * has been reached.
     * 
     * @see org.xml.sax.ContentHandler#endDocument()
     */         
    public void endDocument() throws SAXException {
        Sample s = sample;
        UserData ud;
        
        for(int i=0; i<history.size(); i++){
            s.setPrevious((Sample)history.get(i));
            s = s.getPrevious();
            ud = s.getMessage().getUserData();
            fillUserData(ud, (ArrayList)flatDataHistory.get(i));
            
        }
        ud = s.getMessage().getUserData();
        fillUserData(ud, flatData);
    }
    
    private void fillUserData(UserData data, ArrayList flatElements){
        MetaField[] fields = data.getUserDataType().getFields();
        MetaField field;
     
        for(int i=0; i<fields.length; i++){
            field = fields[i];
            this.fillType(field.getName(), field, data, flatElements);    
        }
    }
    private void fillType(String nestedFieldName, MetaField field, UserData data, ArrayList flatElements){
        if(field instanceof MetaPrimitive){         //Primitive
            this.fillPrimitive(nestedFieldName, data, flatElements);
        } else if(field instanceof MetaEnum){       //Enumeration
            this.fillEnum(nestedFieldName, data, flatElements);
        } else if(field instanceof MetaUnion){      //Union
            this.fillUnion(nestedFieldName, (MetaUnion)field, data, flatElements);
        } else if(field instanceof MetaStruct){     //Structure
            this.fillStruct(nestedFieldName, (MetaStruct)field, data, flatElements);
        } else if(field instanceof MetaClass){      //Class
            this.fillClass(nestedFieldName, (MetaClass)field, data, flatElements);
        } else if(field instanceof MetaCollection){ //Collection
            this.fillCollection(nestedFieldName, (MetaCollection)field, data, flatElements);
        }
    }
    
    private void fillCollection(String nestedFieldName, MetaCollection field, UserData data, ArrayList elements){
        int size;
        String typeName = field.getTypeName();
        FlatElement fe;
                
        if( (typeName.equals("c_string")) ||
            (typeName.equals("c_wstring")) ||
            (typeName.startsWith("C_STRING<")) ||
            (typeName.startsWith("C_WSTRING<")))
        {
            fe = this.getFirstMatchingElement(elements, nestedFieldName);
            
            if(fe != null){
                data.setData(nestedFieldName, fe.flatValue);
            }
        } else { //Not a String collection.
            size = field.getMaxSize();
            
            if(size == 0){
                boolean found = true;
                String fieldName;
                
                for(int i=0; found; i++){
                    fieldName = getCollectionFieldName(nestedFieldName, i);
                    fe = this.getFirstMatchingElement(elements, fieldName);
                   
                    if(fe != null){
                        if(!fe.flatValue.equals("<NULL>")){
                            data.setData(fieldName, fe.flatValue);
                        } else {
                            found = false;
                        }
                    } else {
                        found = false;
                    }
                }
            } else {
                for(int i=0; i<size; i++){
                    this.fillType(getCollectionFieldName(nestedFieldName, i), field.getSubType(), data, elements);
                }
            }
        }
    }
    
    private String getCollectionFieldName(String name, int colIndex){
        int index;
        String result = name;
        
        if(name.endsWith("]")){
            index = name.lastIndexOf('[');
            result = name.substring(0, index);
            index = result.lastIndexOf('[');
            
            while((index != -1) && (result.endsWith("]"))){
                result = result.substring(0, index);
                index = result.lastIndexOf('[');
            }
            index = result.length();
            result += "[" + colIndex + "]";
            result += name.substring(index);
        } else {
            result += "[" + colIndex + "]";
        }
        
        return result;
    }
    
    private void fillClass(String nestedFieldName, MetaClass field, UserData data, ArrayList elements){
        MetaField[] fields = field.getFields();
        
        for(int i=0; i<fields.length; i++){
            this.fillType(nestedFieldName + "." + fields[i].getName(), fields[i], data, elements);
        }
    }
    
    private void fillStruct(String nestedFieldName, MetaStruct field, UserData data, ArrayList elements){
        MetaField[] fields = field.getFields();
        
        for(int i=0; i<fields.length; i++){
            this.fillType(nestedFieldName + "." + fields[i].getName(), fields[i], data, elements);
        }
    }
    
    private void fillPrimitive(String nestedFieldName, UserData data, ArrayList elements){
        FlatElement element;
        element = getFirstMatchingElement(elements, nestedFieldName);
        
        if(element != null){
            data.setData(nestedFieldName, element.flatValue);
        } else {
            logger.logp(Level.SEVERE, "SampleHandler", "fillPrimitive", 
                    "Could not find primitive: " + nestedFieldName);
        }
    }
    
    private void fillEnum(String nestedFieldName, UserData data, ArrayList elements){
        FlatElement element;
        element = getFirstMatchingElement(elements, nestedFieldName);
        
        if(element != null){
            data.setData(nestedFieldName, element.flatValue);
        } else {
            logger.logp(Level.SEVERE, "SampleHandler", "fillEnum", 
                    "Could not find enum: " + nestedFieldName);
        }
    }
    
    private void fillUnion(String nestedFieldName, MetaUnion field, UserData data, ArrayList elements){
        FlatElement element;
        element = getFirstMatchingElement(elements, nestedFieldName + ".switch");
        
        if(element != null){
            data.setData(nestedFieldName + ".switch", element.flatValue);
            MetaUnionCase c = field.getCase(element.flatValue);
            this.fillType(nestedFieldName + "." + c.getField().getName(), c.getField(), data, elements);
            
        } else {
            logger.logp(Level.SEVERE, "SampleHandler", "fillUnion", 
                    "Could not find union case: " + nestedFieldName + ".switch");
        }
    }
    
    private FlatElement getFirstMatchingElement(ArrayList elements, String name){
        FlatElement element, result;
        int i, index;
        StringTokenizer st;
        String elementName, token;
        boolean proceed;
        char[] ta;
        
        result = null;
        
        for(i=0; (i<elements.size()) && (result == null); i++){
            element = (FlatElement)elements.get(i);
            
            if(element.flatName.equals(name)){
                result = element;
            } else {
                st = new StringTokenizer(name, ".");
                elementName = element.flatName;
                proceed = true;
                
                while(st.hasMoreTokens() && proceed){
                    token = st.nextToken();
                    
                    if(elementName.startsWith(token) && (!elementName.endsWith(".size"))){
                        if(elementName.equals(token) && (!st.hasMoreTokens())){
                            result = element;
                            proceed = false;
                        } else {
                            elementName = elementName.substring(elementName.indexOf('.') + 1);
                        }
                        
                    } else {
                        index = token.indexOf('[');
                        
                        if((index != -1) && (!elementName.endsWith(".size"))){
                            if(elementName.startsWith(token.substring(0, index))){
                                if((token.indexOf('.') == -1) && ("<NULL>".equals(element.flatValue))){
                                    result = element;
                                    proceed = false;
                                } else if(elementName.length() > (index+1)){
                                    elementName = elementName.substring(index + 1);
                                    token = token.substring(index);
                                    
                                    if(token.startsWith("[")){
                                        ta = token.toCharArray();
                                        
                                        for(int j=0; j<ta.length && proceed; j++){
                                           if(ta[j] == '['){
                                               if(elementName.startsWith("element.")){
                                                   elementName = elementName.substring(8);
                                               } else if(elementName.startsWith("element")){
                                                   if(!st.hasMoreTokens() && proceed && token.lastIndexOf('[') == j){
                                                       result = element;
                                                       proceed = false;
                                                   } else {
                                                       j = ta.length;
                                                   }
                                               } else {
                                                   proceed = false;
                                               }
                                           }
                                        }
                                    } else {
                                        proceed = false;
                                    }
                                } else {
                                    proceed = false;
                                }
                            } else {
                                proceed = false;
                            }
                        } else {
                            proceed = false;
                        }
                    }
                }
            }
        }
        if(result != null){
            for(int j=0; j<i; j++){
                elements.remove(0);
            }
        }
        return result;
    }
    
    /**
     * Triggered by the parser to notify that the parsing has been started.
     * All data will be initialized.
     * 
     * @see org.xml.sax.ContentHandler#startDocument()
     */
    public void startDocument() throws SAXException {
        MessageQoS msgQos = null;
        userData = new UserData(userDataType);
        writerGid = new GID(-1, -1);
        instanceGid = new GID(-1, -1);
        State state = new State(State.WRITE);
        message = new Message(state, -1, -1, writerGid, instanceGid, 0, msgQos, userData);
        sample = new Sample(-1, -1, new State(0),-1, -1, message);
        fieldScope = new LinkedList();
        fieldElementIndices = new HashMap();
        flatData = new ArrayList();
        flatDataHistory = new ArrayList();
        history = new ArrayList();
        dataBuffer = null;
    }
    
    /**
     * Triggered by the parser to notify a start tag. The name of the tag will
     * be added to the current scope.
     * 
     * @see org.xml.sax.ContentHandler#startElement(java.lang.String, java.lang.String, java.lang.String, org.xml.sax.Attributes)
     */
    public void startElement(String namespaceURI, String localName, String qName, Attributes atts) throws SAXException {
        this.scopeAdd(qName);
    }
    
    /**
     * Triggered by the parser to notify an end tag. The name of the tag is
     * being removed from the current scope. Data that is currently in the
     * buffer, is assigned now. After that, the data buffer is cleared.
     * 
     * The following data is supported:
     * - object.insertTime.seconds              -> insert time seconds
     * - object.insertTime.nanoseconds          -> insert time nanoseconds
     * - object.sampleState                     -> sample state
     * - object.previous.*                      -> history
     * - object.message.nodeState               -> node state
     * - object.message.writeTime.seconds       -> write time seconds
     * - object.message.writeTime.nanoseconds   -> write time nanoseconds
     * - object.message.userData.*              -> all userdata fields.
     * - object.message.writerGID.*             -> writer GID.
     * - object.message.instanceGID.*           -> instance GID.
     * - object.message.sampleSequenceNumber    -> message sequence number.
     * 
     * @todo TODO: Implement QoS support.
     * 
     * @see org.xml.sax.ContentHandler#endElement(java.lang.String, java.lang.String, java.lang.String)
     */
    public void endElement(String namespaceURI, String localName, String qName) throws SAXException {
        String s = dataBuffer;
        String scope = this.scopeGet();
        int historyDepth = -1;
        ArrayList flatDH = flatData;
        Sample sam = sample;
        Message mes = message;
        
        if(s == null){
            s = "";
        }
        
        if(s != null){
            if((scope.startsWith("object.previous.")) && (!("NULL".equals(s)))){
                historyDepth++;
                String temp = scope.substring(16);
                
                while(temp.startsWith("previous.")){
                    historyDepth++;
                    temp = temp.substring(9);
                }
                
                while(history.size() < (historyDepth+1)){
                    UserData hUserData = new UserData(userDataType);
                    GID wGid = new GID(-1, -1);
                    GID iGid = new GID(-1, -1);
                    MessageQoS msgQos = null;
                    State state = new State(State.WRITE);
                    Message hMessage = new Message(state, -1, -1, wGid, iGid, 0, msgQos, hUserData);
                    Sample hSample = new Sample(-1, -1, new State(0), -1, -1, hMessage);
                    history.add(hSample);
                    ArrayList hFlatData = new ArrayList();
                    flatDataHistory.add(hFlatData);
                }
                sam = (Sample)(history.get(historyDepth));
                mes = sam.getMessage();
                flatDH = (ArrayList)flatDataHistory.get(historyDepth);
                scope = "object." + temp;
            }
            
            if(scope.startsWith("object.message.userData.")){
                String fieldName = scope.substring(24);
                FlatElement fe = new FlatElement(fieldName, s);
                flatDH.add(fe);
            }
            else if(scope.equals("object.insertTime.seconds")){
                sam.setInsertTimeSec(Long.parseLong(s));
            } 
            else if(scope.equals("object.insertTime.nanoseconds")){
                sam.setInsertTimeNanoSec(Long.parseLong(s));
            }
            else if(scope.equals("object.sampleState")){
                sam.setState(new State(Integer.parseInt(s)));
            }
            else if(scope.equals("object.disposeCount")){
                sam.setDisposeCount(Long.parseLong(s));
            }
            else if(scope.equals("object.noWritersCount")){
                sam.setNoWritersCount(Long.parseLong(s));
            }
            else if(scope.equals("object.message.nodeState")){
                mes.setNodeState(new State(Integer.parseInt(s)));
            }
            else if(scope.equals("object.message.writeTime.seconds")){
                mes.setWriteTimeSec(Long.parseLong(s));
            }
            else if(scope.equals("object.message.writeTime.nanoseconds")){
                mes.setWriteTimeNanoSec(Long.parseLong(s));
            }
            else if(scope.equals("object.message.writerGID.localId")){
                mes.getWriterGid().setLocalId(Long.parseLong(s));
            }
            else if(scope.equals("object.message.writerGID.systemId")){
                mes.getWriterGid().setSystemId(Long.parseLong(s));
            }
            else if(scope.equals("object.message.writerInstanceGID.localId")){
                mes.getInstanceGid().setLocalId(Long.parseLong(s));
            }
            else if(scope.equals("object.message.writerInstanceGID.systemId")){
                mes.getInstanceGid().setSystemId(Long.parseLong(s));
            }
            else if(scope.equals("object.message.sampleSequenceNumber")){
                mes.setSampleSequenceNumber(Long.parseLong(s));
            }
            else if(scope.equals("object.message.qos.size")){
                mes.setQos(new MessageQoS(new int[Integer.parseInt(s)]));
            }
            else if(scope.equals("object.message.qos.element")){
                mes.getQos().addElement(Integer.parseInt(s));
            }
            else{
                logger.logp(Level.FINE, "SampleHandler", "endElement", "UNKNOWN scope: " + scope + " = " + s);
            }
            dataBuffer = null;
        }
        this.scopeRemoveLast();
        
        
        if("previous".equals(qName)){
            if(this.scopeGet().indexOf("userData") == -1){
                fieldElementIndices = new HashMap();
            }
        }
    }
    
    /**
     * Triggered by the parser when data has been found. Data is added to the 
     * data buffer. 
     * 
     * @see org.xml.sax.ContentHandler#characters(char[], int, int)
     */
    public void characters(char[] ch, int start, int length) throws SAXException {
        String s = new String(ch, start, length);
        
        if(dataBuffer == null){
            dataBuffer = s;
        } else {
            dataBuffer += s;
        }
    }
    
    /**
     * Provides access to the parsed Sample.
     * 
     * @return The parsed Sample.
     */
    public Sample getSample(){
        return sample;
    }

    /**
     * Not implemented, because it is not being used.
     * 
     * @see org.xml.sax.ContentHandler#ignorableWhitespace(char[], int, int)
     */
    public void ignorableWhitespace(char[] ch, int start, int length) throws SAXException {}
    
    /**
     * Not implemented, because it is not being used.
     *
     * @see org.xml.sax.ContentHandler#skippedEntity(java.lang.String)
     */
    public void skippedEntity(String name) throws SAXException {}
    
    /**
     * Not implemented, because it is not being used.
     *
     * @see org.xml.sax.ContentHandler#setDocumentLocator(org.xml.sax.Locator)
     */
    public void setDocumentLocator(Locator locator) {}
    
    /**
     * Not implemented, because it is not being used.
     *
     * @see org.xml.sax.ContentHandler#processingInstruction(java.lang.String, java.lang.String)
     */
    public void processingInstruction(String target, String data) throws SAXException {}
    
    /**
     * Not implemented, because it is not being used.
     *
     * @see org.xml.sax.ContentHandler#startPrefixMapping(java.lang.String, java.lang.String)
     */
    public void startPrefixMapping(String prefix, String uri) throws SAXException {}
    
    /**
     * Not implemented, because it is not being used.
     *
     * @see org.xml.sax.ContentHandler#endPrefixMapping(java.lang.String)
     */
    public void endPrefixMapping(String prefix) throws SAXException {}
    
    
    private class FlatElement {
        public String flatName;
        public String flatValue;
        
        
        public FlatElement(String flatName, String flatValue){
            this.flatName = flatName;
            this.flatValue = flatValue;
        }
    }
    
    private ArrayList flatData; /* <FlatElement> */
    
    private ArrayList flatDataHistory; /* <ArrayList<FlatElement>> */
    
    private ArrayList history;
    
    /**
     * The deserialized sample.
     */
    private Sample sample;
    
    /**
     * The deserialized message in the sample.
     */
    private Message message;
    
    /**
     * The deserialized userdata in the message.
     */
    private UserData userData;
    
    /**
     * Used to keep track of the current scope.
     * Nested fields are added to the scope until handled.
     */
    private LinkedList fieldScope;
    
    /**
     * The type of the userdata in the message.
     */
    private MetaType userDataType;
    
    /**
     * Keeps track of a collection index while processing it.
     * (<String fieldName, ArrayList indices>). 
     * 
     * This is done to address the right index of a collection.
     */ 
    private HashMap fieldElementIndices;
    
    /**
     * Logging facilities for the handler.
     */
    private Logger logger;
    
    private GID writerGid;
    
    private GID instanceGid;
    
    private String dataBuffer;
}

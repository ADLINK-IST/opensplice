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

import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.ListIterator;
import java.util.NoSuchElementException;

import org.opensplice.cm.data.UserData;
import org.opensplice.cm.meta.MetaType;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 * Offers facilities to deserialize UserData. It is being triggered by
 * the SAX parser.
 *
 * @date May 13, 2004
 */
public class UserDataHandler extends CommonHandler {
    /**
     * Constructs a new UserDataHandler that is able to deserialize userData
     * from XML to a Java representation.
     *
     * @param type
     *            The type of the userdata in the Sample.
     */
    public UserDataHandler(MetaType type){
        super(type);
        userData = null;
    }

    /**
     * Adds a field to the current scope.
     *
     * @param field The field to add to the scope.
     */
    @Override
    protected void scopeAdd(String field) {
        if(fieldScope == null){
            assert(field.equals("object"));
            /* Filter out the outer 'object' scope */
            fieldScope = new LinkedList<String>();
        } else {
            fieldScope.add(field);
        }
    }

    /**
     * Removes the last field from the scope.
     */
    @Override
    protected void scopeRemoveLast() {
        try {
            fieldScope.removeLast();
        } catch (NoSuchElementException e){
            /* Filter out the top-level 'object' scope */
            fieldScope = null;
        }
    }

    /**
     * Provides access to the current scope.
     *
     * @return The current scope. Fieldnames are '.' separated.
     */
    @Override
    protected String scopeGet() {
        boolean first = true;
        ListIterator<String> li = fieldScope.listIterator();
        StringBuffer buf = new StringBuffer();
        while(li.hasNext()){
            if(first){
                buf.append(li.next());
                first = false;
            }
            else{
                buf.append("." + (li.next()));
            }
        }
        return buf.toString();
    }



    /**
     * Triggered by the parser to notify that the end of the XML input
     * has been reached.
     *
     * @see org.xml.sax.ContentHandler#endDocument()
     */
    @Override
    public void endDocument() throws SAXException {
        fillUserData(userData, flatData);
    }

    /**
     * Triggered by the parser to notify that the parsing has been started.
     * All data will be initialized.
     *
     * @see org.xml.sax.ContentHandler#startDocument()
     */
    @Override
    public void startDocument() throws SAXException {
        userData = new UserData(userDataType);
        fieldScope = null;
        fieldElementIndices = new HashMap<String, ArrayList<String>>();
        flatData = new ArrayList<FlatElement>();
        dataBuffer = null;
    }

    /**
     * Triggered by the parser to notify a start tag. The name of the tag will
     * be added to the current scope.
     *
     * @see org.xml.sax.ContentHandler#startElement(java.lang.String, java.lang.String, java.lang.String, org.xml.sax.Attributes)
     */
    @Override
    public void startElement(String namespaceURI, String localName, String qName, Attributes atts) throws SAXException {
        this.scopeAdd(qName);
    }

    @Override
    public void endElement(String namespaceURI, String localName, String qName) throws SAXException {
        String s = dataBuffer;
        String scope = this.scopeGet();

        if(s == null){
            s = "";
        }

        if(s != null && scope != null){
            FlatElement fe = new FlatElement(scope, s);
            flatData.add(fe);
            dataBuffer = null;
        }
        this.scopeRemoveLast();
    }

    /**
     * Provides access to the parsed UserData.
     *
     * @return The parsed UserData.
     */
    public UserData getUserData(){
        return userData;
    }

    /**
     * The deserialized userdata in the message.
     */
    private UserData userData;

    /**
     * Used to keep track of the current scope.
     * Nested fields are added to the scope until handled.
     */
    private LinkedList<String> fieldScope;


}

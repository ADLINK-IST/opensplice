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
package org.opensplice.cm.com;

import java.io.IOException;
import java.io.OutputStreamWriter;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.Document;
import org.xml.sax.SAXException;

/**
 * Represents a SOAP connection. It is an clientside HTTP connection, which can 
 * send SOAPRequest messages and receives answers as SOAPResponse objects. 
 * 
 * @date Feb 16, 2005 
 */
public class SOAPConnection {
    /**
     * The HTTP connection.
     */
    private HttpURLConnection connection = null;
    
    /**
     * The output writer that writes data 'over' the connection.
     */
    private OutputStreamWriter writer = null;
    
    /**
     * The last used URL. This is used to cache the connection and its
     * reader and writer when multiple requests are sent to the same URL. 
     */
    private String lastURL = null;
    
    /**
     * Builder that is able to create a DOM tree from an XML document.
     */
    private DocumentBuilder builder = null;
    
    /**
     * Constructs a new SOAPConnection.
     *  
     *
     * @throws SOAPException Thrown when the DocumentBuilder could not be 
     *                       initialized.
     */
    public SOAPConnection() throws SOAPException{
        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        factory.setValidating(false);
        factory.setNamespaceAware(false);
        try {
            builder = factory.newDocumentBuilder();
        } catch (ParserConfigurationException e) {
            throw new SOAPException(e.getMessage());
        }
    }
    
    /**
     * Sends the supplied message to the supplied URL and returns the response
     * it received.
     * 
     * @param message The message to send. This message must be an instance of
     *                SOAPRequest. 
     * @param url The URL to send the message to.
     * @return The received response.
     * @throws SOAPException Thrown when:
     *                       - Supplied URL is malformed.
     *                       - Supplied message is invalid.
     *                       - Connection to supplied URL could not be established.
     *                       - Response could not be parsed.
     * @todo TODO: Change 'SOAPMessage message' argument into 'SOAPRequest 
     *       message' and change return type into 'SOAPResponse'.
     */
    public synchronized SOAPMessage call(SOAPMessage message, String url) throws SOAPException{
        boolean create = false;
        SOAPResponse response = null;
        
        try {
            if(lastURL != null){
                if(!lastURL.equals(url)){
                    create = true;
                }
            } else {
                create = true;
            }
            
            if(create){
                URL u = new URL(url);
                connection = (HttpURLConnection) u.openConnection();
                connection.setDoOutput(true);
                connection.connect();
                writer = new OutputStreamWriter(connection.getOutputStream());
                writer.write(((SOAPRequest)message).getString());
                writer.flush();
            } else {
                writer.write(((SOAPRequest)message).getString());
                writer.flush();
            }
            try {
                Document document = builder.parse(connection.getInputStream());
                response = new SOAPResponse(document);
            } catch (SAXException e) {
                throw new SOAPException(e.getMessage());
            }
        } catch (MalformedURLException me) {
            throw new SOAPException("Malformed URL: " + me.getMessage());
        } catch (IOException ie) {
            throw new SOAPException("IOException: " + ie.getMessage());
        } catch(ClassCastException ce){
            throw new SOAPException("Malformed URL.");
        }
        return response;
    }
    
    /**
     * Closes the current connection, if there is one.
     */
    public void close(){
        if(connection != null){
            connection.disconnect();
            connection = null;
            lastURL = null;
            writer = null;
        }
    }
}

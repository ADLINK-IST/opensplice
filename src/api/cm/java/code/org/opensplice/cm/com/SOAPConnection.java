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
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
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
     * Builder that is able to create a DOM tree from an XML document.
     */
    private DocumentBuilder builder = null;

    /**
     * Constructs a new SOAPConnection.
     *
     *
     * @throws SOAPException
     *             Thrown when the DocumentBuilder could not be initialized.
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
     * @param message
     *            The message to send. This message must be an instance of
     *            SOAPRequest.
     * @param url
     *            The URL to send the message to.
     * @return The received response.
     * @throws SOAPException
     *             Thrown when: - Supplied URL is malformed. - Supplied message
     *             is invalid. - Connection to supplied URL could not be
     *             established. - Response could not be parsed.
     * @throws NoSuchMethodException
     * @todo TODO: Change 'SOAPMessage message' argument into 'SOAPRequest
     *       message' and change return type into 'SOAPResponse'.
     */
    public synchronized SOAPMessage call(SOAPMessage message, String url) throws SOAPException {
        SOAPResponse response = null;
        int status = 0;
        try {
            URL u = new URL(url);
            connection = (HttpURLConnection) u.openConnection();
            connection.setDoOutput(true);
            connection.connect();

            writer = new OutputStreamWriter(connection.getOutputStream(), "UTF-8");
            writer.write(((SOAPRequest) message).getString());
            writer.flush();
            try {
                Document document = builder.parse(connection.getInputStream());
                response = new SOAPResponse(document);
            } catch (SAXException e) {
                throw new SOAPException(e.getMessage());
            }
        } catch (MalformedURLException me) {
            throw new SOAPException("Malformed URL: " + me.getMessage());
        } catch (IOException ie) {
            try {
                status = connection.getResponseCode();
            } catch (IOException i) {
                throw new SOAPException("IOException: " + i.getMessage());
            }
            if (status == HttpURLConnection.HTTP_INTERNAL_ERROR) {
                Document doc;
                try {
                    doc = builder.parse(connection.getErrorStream());
                } catch (SAXException e) {
                    throw new SOAPException(e.getMessage());
                } catch (IOException e) {
                    throw new SOAPException("IOException: " + e.getMessage());
                }

                String faultCode = null;
                String faultString = null;

                NodeList nodeList = doc.getElementsByTagName("faultcode");
                if (nodeList != null) {
                    Element nameElement = (Element) nodeList.item(0);
                    if (nameElement != null) {
                        faultCode = nameElement.getFirstChild().getNodeValue().trim();
                    }
                }

                nodeList = doc.getElementsByTagName("faultstring");
                if (nodeList != null) {
                    Element nameElement = (Element) nodeList.item(0);
                    if (nameElement != null) {
                        faultString = nameElement.getFirstChild().getNodeValue().trim();
                    }
                }
                throw new SOAPException("IOException: " + ie.getMessage(), faultCode, faultString);
            } else {
                throw new SOAPException("IOException: " + ie.getMessage());
            }
        } catch(ClassCastException ce){
            throw new SOAPException("Malformed URL.");
        }
        return response;
    }

    /**
     * Closes the current connection, if there is one.
     */
    public synchronized void close() {
        if(connection != null){
            connection.disconnect();
            connection = null;
            writer = null;
        }
    }
}

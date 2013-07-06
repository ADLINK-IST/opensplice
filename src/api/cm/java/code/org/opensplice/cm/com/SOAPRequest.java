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

import java.io.StringWriter;

/**
 * Concrete SOAP message, which represents a SOAP request. SOAP requests can be
 * sent to a SOAP service using the SOAPConnection object. This class should
 * be used like this:
 * 1. Call constructor. (SOAPRequest request = new SOAPRequest())
 * 2. Set the method to call (request.setMethod("myMethod"))
 * 3. Add method parameters (request.addBodyParameter("parameterName", "parameterValue"))
 * 4. Save the changes (request.saveChanges())
 * 5. The message is ready and its string representation can be resolved. 
 *    (request.getString())
 * 
 * Steps 2 and 3 may also be called in a different order.
 * 
 * @date Feb 17, 2005 
 */
class SOAPRequest extends SOAPMessage{
    /**
     * Writer for the SOAP message. This performs better then String
     * concatenation.
     */
    private StringWriter writer = null;
    
    /**
     * The SOAP message.
     */
    private String message = null;
    
    /**
     * The SOAP method to call.
     */
    private String method = null;
    
    private boolean mayModify;
    
    /**
     * Constructs a new SOAP request.
     */
    public SOAPRequest(){
        super();
        mayModify = true;
        writer = new StringWriter();
        writer.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?><SOAP-ENV:Envelope ");
        writer.write("xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" ");
        writer.write("xmlns:SOAP-ENC=\"http://schemas.xmlsoap.org/soap/encoding/\" ");
        writer.write("xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" ");
        writer.write("xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" ");
        writer.write("xmlns:cms=\"http://127.0.0.1/cms.wsdl\">");
        body = "";
        method = "";
    }
    
    /**
     * Sets the method to call. If mayModify is false, this call will have no 
     * effect.
     * 
     * @param method The method to call.
     */
    public void setMethod(String method){
        if(mayModify){
            this.method = method;
        }
    }
    
    /**
     * Adds a body parameter to the request. The following will be added to the
     * request:
     * <name>value</name>
     * If mayModify is false, this call will have no effect.
     * 
     * 
     * @param name The name of the parameter.
     * @param value The value of the parameter.
     */
    public void addBodyParameter(String name, String value){
        if(mayModify){
            if(value == null){
                body += "<" + name + "></" + name + ">";
            } else {
                value = this.encodeString(value);
                body += "<" + name + ">" + value + "</" + name + ">";
            }
        }
    }
    
    /**
     * Saves the request. After calling this routine, the message cannot be
     * modified.
     *
     */
    public void saveChanges(){
        writer.write("<SOAP-ENV:Body><cms:" + method + ">");
        writer.write(body);
        writer.write("</cms:" + method + "></SOAP-ENV:Body></SOAP-ENV:Envelope>");
        writer.flush();
        message = writer.toString();
        mayModify = false;
    }
    
    /**
     * Provides access to the String representation of the message.
     * 
     * @return The String representation of the message.
     */
    public String getString(){
        return message;
    }
    
    private String encodeString(String value){
        char c;
        StringWriter sw = new StringWriter();
        String str;
        
        
        for(int i=0; i<value.length(); i++){
            c = value.charAt(i);
            
            switch(c){
            case '<':
                sw.write("&lt;");
                break;
            case '>':
                sw.write("&gt;");
                break;
            case '"':
                sw.write("&quot;");
                break;
            case '\'':
                sw.write("&apos;");
                break;
            case '&':
                str = value.substring(i);
                
                if(str.startsWith("&amp;")){
                    i += 4;
                    sw.write("&amp;amp;");
                } else if(str.startsWith("&lt;")){
                    i += 3;
                    sw.write("&amp;lt;");
                } else if(str.startsWith("&gt;")){
                    i += 3;
                    sw.write("&amp;gt;");
                } else if(str.startsWith("&quot;")){
                    i += 5;
                    sw.write("&amp;quot;");
                } else if(str.startsWith("&apos;")){
                    i += 5;
                    sw.write("&amp;apos;");
                } else {
                    sw.write("&amp;");
                }
                break;
            default:
                sw.write(c);
                break;
            }
        }
        sw.flush();
        return sw.toString();
    }
}

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
package org.opensplice.cm;

import org.opensplice.cm.com.*;
import org.opensplice.cm.impl.ParticipantImpl;
import org.opensplice.cm.qos.ParticipantQoS;

/**
 * Control & Monitoring factory.
 * 
 * This class takes care of initialisation and detaching of the Control
 * & Monitoring API. The API can only be used when it is initialised. The
 * CMFactory is NOT threadsafe. Synchronization of initialisation and detaching
 * must be taken care of by the user of the API. 
 *
 * All allocated entities are registered here and are freed when the API is 
 * detached.
 */
public class CMFactory {
    /**
     * Changes the communication mode of the C&M API. Currently two modes
     * are supported:
     * - COMMUNICATION_MODE_JNI  : local connection using JNI.
     * - COMMUNICATION_MODE_SOAP : remote connection using SOAP. 
     * 
     * The communication mode may only be changed when the CMFactory is not
     * initialised. 
     * 
     * @param mode The communication mode to set.
     * @throws CMException Thrown when:
     *                     - Communication mode is unknown
     *                     - CMFactory is currently initialised.
     */
    public static void setCommunicationMode(int mode) throws CMException {
        if(isInitialised()){
            throw new CMException("Communication mode can only be changed when not initialised.");
        }
        if(mode == COMMUNICATION_MODE_JNI){
            COMMUNICATION_MODE = mode;
        } else if(mode == COMMUNICATION_MODE_SOAP){
            COMMUNICATION_MODE = mode;
        } else {
            throw new CMException("Supplied communication mode unknown.");
        }
    }
    
    /**
     * Initialises the Control & Monitoring API.
     *  
     * @throws CMException Thrown when the API could not be initialised. This
     *                     happens when the communication libraries cannot be
     *                     loaded or splice is not runnning.
     */
    public static synchronized void initialise() throws CMException{
        if(instance == null){
            instance = new CMFactory("http://127.0.0.1");
        }
    }
    
    /**
     * Initialises the Control & Monitoring API.
     * 
     * @param url The location of the SPLICE-DDS node to initialize. In 
     *            COMMUNICATION_MODE_JNI mode this is ignored.
     * @throws CMException Thrown when the API could not be initialised. This
     *                     happens when the communication libraries cannot be
     *                     loaded or splice is not runnning.
     */
    public static synchronized void initialise(String url) throws CMException{
        if(instance == null){
            instance = new CMFactory(url);
        }
    }
    
    /**
     * Checks whether the Control & Monitoring API is currently initialised.
     * 
     * @return true if it is initialised, false otherwise. 
     */
    public static boolean isInitialised(){
        return (instance != null);
    }
    
    /**
     * Detaches the Control & Monitoring API.
     * All entities that not have been freed are freed before the API is
     * detached.
     * 
     * @throws CMException Thrown when the API was currently not initialised or
     *                     it could not be detached.
     */
    public static synchronized void detach() throws CMException{
        if(instance == null){
            throw new CMException("Not initialised.");
        }
        
        try {
            communicator.detach();
        } catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }
        currentURL = null;
        instance = null;
        communicator = null;
    }
    
    /**
     * Provides access to the domain id of the supplied kernel uri. This 
     * function will disappear in the future.
     * 
     * @return The URL of the node where the API is currently working on.
     * @throws CMException Thrown when the API currently is not initialised.
     */
    public static String getCurrentURL() throws CMException{
        if(instance == null){
            throw new CMException(initMsg);
        }
        return currentURL;
    }
    
    /**
     * Provides access to the communication handler of the Control & Monitoring 
     * API.
     * 
     * @return The communicator of the Control & Monitoring API.
     * @throws CMException Thrown when the Control & Monitoring API currently
     *                     is not initialised.
     */
    public static Communicator getCommunicator() throws CMException{
        if(instance == null){
            throw new CMException(initMsg);
        }
        return communicator;
    }
    
    /**
     * Provides access to the current communication mode.
     * 
     * @return The current communication mode.
     */
    public static int getCommunicationMode(){
        return COMMUNICATION_MODE;
    }
    
    public static Participant createParticipant(String uri, int timeout, String name, ParticipantQoS qos) throws CMException{
        return new ParticipantImpl(uri, timeout, name, qos);
        /*
        Participant participant;
        
        try {
            participant = CMFactory.getCommunicator().participantNew(uri, timeout, name, qos);
        } catch (CommunicationException ce) {
            throw new CMException(ce.getMessage());
        } 
        return participant;
        */
    }
    
    private CMFactory(String url) throws CMException{
        if(COMMUNICATION_MODE == COMMUNICATION_MODE_JNI){
            try {
                communicator = new JniCommunicator();
            } catch (CommunicationException e) {
                throw new CMException("JNI library could not be loaded.");
            }
        } else if(COMMUNICATION_MODE == COMMUNICATION_MODE_SOAP){
            try {
                /*init soap communicator.*/
                communicator = new SOAPCommunicator();
            } catch (CommunicationException ce) {
                throw new CMException("Could not connect to : " + url);
            } catch (NoClassDefFoundError ne){
                throw new CMException("Required Java SOAP extensions not available.");
                
            }
        }
        try {
            communicator.initialise(url);
        } catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }
        currentURL = url;
    }
    
    /**
     * JNI communication mode. The API will communicate with Splice using
     * JNI.
     */
    public static final int COMMUNICATION_MODE_JNI = 0;
    
    /**
     * SOAP communication mode. The API will communicate with Splice using 
     * SOAP.
     */
    public static final int COMMUNICATION_MODE_SOAP = 1;
    
    /**
     * The current communication mode.
     */
    private static int COMMUNICATION_MODE = COMMUNICATION_MODE_JNI;
    
    /**
     * The CMFactory instance.
     */
    private static CMFactory instance  = null;
    
    /**
     * The commmunicator.
     */
    private static Communicator communicator = null;
    
    /**
     * Message that is used to notify that the API is used without being
     * initialized.
     */
    private static final String initMsg = "C&M API not initialised.";
    
    /**
     * The current connected URL.
     */
    private static String currentURL = null;
}

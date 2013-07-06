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
package org.opensplice.cm;

import org.opensplice.cm.com.CommunicationException;
import org.opensplice.cm.com.Communicator;
import org.opensplice.cm.com.JniCommunicator;
import org.opensplice.cm.com.SOAPCommunicator;
import org.opensplice.cm.impl.ParticipantImpl;
import org.opensplice.cm.impl.StorageImpl;
import org.opensplice.cm.qos.ParticipantQoS;

/**
 * Control & Monitoring factory.
 *
 * This class takes care of initialisation and detaching of the Control &
 * Monitoring API. The API can only be used when it is initialised. The
 * CMFactory is NOT threadsafe. Synchronization of initialisation and detaching
 * must be taken care of by the user of the API.
 *
 * All allocated entities are registered here and are freed when the API is
 * detached.
 */
public class CMFactory {

    /**
     * Checks whether the Control & Monitoring API is currently initialised.
     *
     * @return true if it is initialised, false otherwise.
     */
    public static boolean isInitialised() {
        return (instance != null);
    }

    /**
     * Detaches the Control & Monitoring API. All entities that not have been
     * freed are freed before the API is detached.
     *
     * @throws CMException
     *             Thrown when the API was currently not initialised or it could
     *             not be detached.
     */
    public static synchronized void detach() throws CMException {
        if(instance == null){
            throw new CMException("Not initialised.");
        }

        try {
            communicator.detach();
        } catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }
        instance = null;
        communicator = null;
    }

    /**
     * Provides access to the communication handler of the Control & Monitoring
     * API.
     *
     * @return The communicator of the Control & Monitoring API.
     * @throws CMException
     *             Thrown when the Control & Monitoring API currently is not
     *             initialised.
     */
    public static Communicator getCommunicator() throws CMException {
        if(instance == null){
            throw new CMException(initMsg);
        }
        return communicator;
    }

    public static Participant createParticipant(int uri, int timeout, String name, ParticipantQoS qos)
            throws CMException {
    	return createParticipant(Integer.toString(uri),timeout,name,qos);
    }

    /*
     * Initialises the Control & Monitoring API. When there is no instance
     * initiated a new one will be initiated with the current COMMUNICATION_MODE
     * the following COMMUNICATION_MODE are available - COMMUNICATION_MODE_JNI :
     * local connection using JNI. - COMMUNICATION_MODE_SOAP : remote connection
     * using SOAP.
     */
    public synchronized static Participant createParticipant(String uri, int timeout, String name, ParticipantQoS qos)
            throws CMException {
        if (instance == null) {
            if (uri.toLowerCase().startsWith("http://") && !uri.equalsIgnoreCase("http://")) {
                /* soap mode */
                COMMUNICATION_MODE = COMMUNICATION_MODE_SOAP;
            } else {
                /* we are a local */
                COMMUNICATION_MODE = COMMUNICATION_MODE_JNI;
            }
            instance = new CMFactory(uri);

        } else {
            if (uri.toLowerCase().startsWith("http://") && !uri.equalsIgnoreCase("http://")) {
                if (COMMUNICATION_MODE != COMMUNICATION_MODE_SOAP) {
                    throw new CMException("Can not create Participant in SOAP mode because current active mode is JNI");
                }
            } else {
                if (COMMUNICATION_MODE != COMMUNICATION_MODE_JNI) {
                    throw new CMException("Can not create Participant in JNI mode because current active mode is SOAP");
                }
            }
        }
        try {
            matchCMVersions(getLocalVersion(), getVersion());
        } catch (NoSuchMethodError er) {
            return new ParticipantImpl(uri, timeout, name, qos);
        }
        return new ParticipantImpl(uri, timeout, name, qos);
    }

    /**
     * Creates a storage object described by the provided storage attributes.
     */
    public static Storage createStorage() throws CMException {
        return new StorageImpl();
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
    }

    /**
     * Resolves the current remote version of the CM API.
     *
     * @return The current remote version of the CM API.
     * @throws CMException
     *             Thrown when the service has already been freed, or when its
     *             kernel service could not be claimed.
     */
    public static String getVersion() throws CMException {
        String version;

        try {
            version = CMFactory.getCommunicator().getVersion();
        } catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }
        return version;
    }

    /**
     * Resolves the current local version of the CM API.
     *
     * @return The current local version of the CM API.
     */
    public static String getLocalVersion() {
        Package pkgs[];

        pkgs = Package.getPackages();
        String result = "";
        for (int i = 0; i < pkgs.length; i++) {
            if (pkgs[i].getName().equals("org.opensplice.cm")) {
                result = pkgs[i].getImplementationVersion();
                if (result == null) {
                    result = "N.A.";
                }
            }
        }
        return result;
    }

    public static void matchCMVersions(String local, String remote) throws CMException {

        if (!local.equals("N.A.") && !remote.equals("N.A.") && !local.equals(remote)) {
            String[] localVersion = local.split("\\.");
            String[] remoteVersion = remote.split("\\.");
            if (localVersion != null && remoteVersion != null) {
                if (localVersion[0].equals(remoteVersion[0])) {
                    if (!localVersion[1].equals(remoteVersion[1])) {
                        int localMinor = new Integer(localVersion[1]).intValue();
                        int remoteMinor = new Integer(remoteVersion[1]).intValue();
                        if (localMinor > remoteMinor) {
                            throw new CMException("CM API Minor version mismatch, remote version " + remoteVersion[0]
                                    + "." + remoteMinor + " is lower than local " + localVersion[0] + "." + +localMinor
                                    + " version");
                        }
                    }
                } else {
                    throw new CMException("CM API Major version mismatch, local version " + local + " remote version "
                            + remote);
                }
            } else {
                throw new CMException("CM API Major version mismatch, local version " + local + " remote version "
                        + remote);
            }
        }
    }

    /**
     * JNI communication mode. The API will communicate with Splice using
     * JNI.
     */
    public static final int COMMUNICATION_MODE_JNI = 0;

    /**
     * SOAP communication mode. The API will communicate with Splice using SOAP.
     */
    public static final int COMMUNICATION_MODE_SOAP = 1;

    /**
     * The current communication mode.
     */
    private static int COMMUNICATION_MODE = COMMUNICATION_MODE_JNI;

    /**
     * The CMFactory instance.
     */
    private static CMFactory    instance                = null;

    /**
     * The commmunicator.
     */
    private static Communicator communicator = null;

    /**
     * Message that is used to notify that the API is used without being
     * initialized.
     */
    private static final String initMsg = "C&M API not initialised.";

}

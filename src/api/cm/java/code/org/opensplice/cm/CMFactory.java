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

import java.util.HashMap;
import java.util.Set;

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
     * Detaches the Control & Monitoring API. All entities that not have been
     * freed are freed before the API is detached.
     *
     * @throws CMException
     *             Thrown when the API was currently not initialised or it could
     *             not be detached.
     */
    public static synchronized void detach() throws CMException {
        Communicator communicator;
        Set<String> domains = communicators.keySet();

        try {
            for(String domain: domains){
                communicator = communicators.get(domain);
                communicator.detach();
            }
            communicators.clear();
        } catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }
    }

    public static synchronized void detach(String domain) throws CMException {
        Communicator communicator;

        try {
            communicator = communicators.remove(domain);

            if(communicator != null){
                communicator.detach();
            }
        } catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }
    }

    /**
     * Provides access to the communication handler for a given domain.
     * 
     * @return The communicator for the given domain
     * @throws CMException
     *             Thrown when communicator could not be constructed or
     *             initialised.
     */
    private synchronized static Communicator getCommunicator(String domain) throws CMException {
    	Communicator c = communicators.get(domain);

        if(c == null){
        	try {
        		if (domain.toLowerCase().startsWith("http://") && !domain.equalsIgnoreCase("http://")) {
					c = new SOAPCommunicator();
	        	} else {
	        		c = new JniCommunicator();
	        	}
        		c.initialise(domain);
        	} catch (CommunicationException e) {
				throw new CMException(e.getMessage());
			}  catch (NoClassDefFoundError ne){
                throw new CMException("Required Java SOAP extensions not available.");
            }
        	communicators.put(domain, c);
        }
        return c;
    }

    public static Participant createParticipant(int domain, int timeout, String name, ParticipantQoS qos)
            throws CMException {
    	return createParticipant(Integer.toString(domain),timeout,name,qos);
    }


    public static Participant createParticipant(String domain, int timeout, String name, ParticipantQoS qos)
            throws CMException {
        try {
            matchCMVersions(getLocalVersion(), getVersion(domain));
        } catch (NoSuchMethodError er) {
        	/* Ignore and continue */
        }
        return new ParticipantImpl(CMFactory.getCommunicator(domain), domain, timeout, name, qos);
    }

    /**
     * Creates a storage object described by the provided storage attributes.
     */
    public static Storage createStorage(String domain) throws CMException {
        return new StorageImpl(CMFactory.getCommunicator(domain));
    }

    /**
     * Resolves the version of the federation identified by the domain
     * parameter.
     * 
     * @return The version of federation identified by the domain parameter.
     * @throws CMException
     *             Thrown when communication with the federation fails.
     */
    public synchronized static String getVersion(String domain) throws CMException {
        String version;
        Communicator communicator = null;

        if(domain == null){
            throw new CMException("Invalid domain id provided.");
        }

        communicator = communicators.get(domain);
        if(communicator == null){
            try {
                if (domain.toLowerCase().startsWith("http://") && !domain.equalsIgnoreCase("http://")) {
                    communicator = new SOAPCommunicator();
                } else {
                    communicator = new JniCommunicator();
                }
                communicator.initialise(domain);
                version = communicator.getVersion();
            } catch (CommunicationException e) {
                throw new CMException(e.getMessage());
            } finally {
                try {
                    communicator.detach();
                } catch (CommunicationException e) {
                    throw new CMException(e.getMessage());
                }
            }
        } else {
            try {
                version = communicator.getVersion();
            } catch (CommunicationException e) {
                throw new CMException(e.getMessage());
            }
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
     * The commmunicator.
     */
    private static HashMap<String,Communicator> communicators = new HashMap<String,Communicator>();

}
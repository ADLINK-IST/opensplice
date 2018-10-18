/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
package org.opensplice.cm;

import java.util.HashMap;
import java.util.Set;

import org.opensplice.cm.com.CommunicationException;
import org.opensplice.cm.com.Communicator;
import org.opensplice.cm.com.JniCommunicator;
import org.opensplice.cm.com.SOAPCommunicator;
import org.opensplice.cm.com.SOAPConnectionPool;
import org.opensplice.cm.com.SOAPException;
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
        } finally {
            if (communicators.isEmpty()) {
                try {
                    SOAPConnectionPool.getInstance().closeConnections();
                } catch (SOAPException e) {
                    throw new CMException(e.getMessage());
                }
            }
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
        } finally {
            if(communicators.isEmpty()){
                try {
                    SOAPConnectionPool.getInstance().closeConnections();
                } catch (SOAPException e) {
                    throw new CMException(e.getMessage());
                }
            }
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
                    c = new SOAPCommunicator(domain);
                } else {
                    c = new JniCommunicator(domain);
                }
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
            throws CMCompatibilityException, CMException {
       return createParticipant(Integer.toString(domain),timeout,name,qos);
    }


    public static Participant createParticipant(String domain, int timeout, String name, ParticipantQoS qos)
            throws CMCompatibilityException, CMException {
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
                    communicator = new SOAPCommunicator(domain);
                } else {
                    communicator = new JniCommunicator(domain);
                }
                version = communicator.getVersion();
            } catch (CommunicationException e) {
                throw new CMException(e.getMessage());
            } finally {
                try {
                    if (communicator != null) {
                        communicator.detach();
                    }
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

    public static void matchCMVersions(String local, String remote) throws CMCompatibilityException {

        if (!local.equals("N.A.") && !remote.equals("N.A.") && !local.equals(remote)) {
            String[] localVersion = local.replaceFirst("V", "").split("\\.");
            String[] remoteVersion = remote.replaceFirst("V", "").split("\\.");
            int localMajor = new Integer(localVersion[0]);
            int localMinor = new Integer(localVersion[1]);
            int remoteMajor = new Integer(remoteVersion[0]);
            int remoteMinor = new Integer(remoteVersion[1]);
            final String devLabel = "non-ADLINK build";
            boolean isLocalDev = local.contains(devLabel);
            boolean isRemoteDev = local.contains(devLabel);

            double localMajorMinor = new Double(localMajor+"."+localMinor);
            double remoteMajorMinor = new Double(remoteMajor+"."+remoteMinor);
            if(!isLocalDev || !isRemoteDev){
                //Check here that 6.5 is minimum version required to be used before anything else
                if(remoteMajorMinor < 6.5){
                    String message = String.format("Both local and remote CM API versions be must minimum 6.5 to be compatible, local version %.1f remote version %.1f.",
                            localMajorMinor, remoteMajorMinor);
                    throw new CMCompatibilityException(message, localMajor, localMinor, remoteMajor, remoteMinor);
                }
            }
        }
    }

    /**
     * The commmunicator.
     */
    private static HashMap<String,Communicator> communicators = new HashMap<String,Communicator>();

}

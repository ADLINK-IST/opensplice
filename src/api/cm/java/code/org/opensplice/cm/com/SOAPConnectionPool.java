package org.opensplice.cm.com;

import java.util.HashSet;
import java.util.Iterator;

public class SOAPConnectionPool {
    private final HashSet<SOAPConnection> availableConnections;
    private final HashSet<SOAPConnection> usedConnections;
    private static SOAPConnectionPool instance = null;


    public static synchronized SOAPConnectionPool getInstance() throws SOAPException{
        if(instance == null){
            instance = new SOAPConnectionPool();
        }
        return instance;
    }

    private SOAPConnectionPool() throws SOAPException{
        availableConnections = new HashSet<SOAPConnection>();
        usedConnections = new HashSet<SOAPConnection>();
    }

    public synchronized SOAPConnection acquireConnection() throws SOAPException{
        Iterator<SOAPConnection> iter;
        SOAPConnection connection = null;

        if(availableConnections.size() > 0){
            iter = availableConnections.iterator();
            connection = iter.next();
            availableConnections.remove(connection);
            usedConnections.add(connection);
        } else {
            connection = new SOAPConnection();
            usedConnections.add(connection);
        }
        return connection;
    }

    public synchronized void releaseConnection(SOAPConnection connection){
        if(usedConnections.contains(connection)){
            usedConnections.remove(connection);
            availableConnections.add(connection);
        }
    }

    public synchronized void closeConnections() throws SOAPException{
        if(!usedConnections.isEmpty()){
            throw new SOAPException("SOAPConnectionPool.closeConnections(): could not close connections as one or more are still in use.");
        }
        for(SOAPConnection connection:availableConnections){
            connection.close();
        }
        availableConnections.clear();
    }
}

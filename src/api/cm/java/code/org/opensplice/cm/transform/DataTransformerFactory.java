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
/**
 * Contains abstract serializer/deserializer facilities
 * to transform data from Java to a specific representation and the other way
 * around.
 */
package org.opensplice.cm.transform;

import java.util.logging.Level;
import java.util.logging.Logger;

import javax.xml.parsers.ParserConfigurationException;

import org.opensplice.cm.com.Communicator;
import org.xml.sax.SAXException;

import org.opensplice.cm.meta.*;
import org.opensplice.cm.transform.xml.*;

/**
 * Factory that supplies facilities to get access to concrete (de)serializers.
 * Currently only XML is supported.
 * 
 * @date May 14, 2004
 */
public class DataTransformerFactory {
    /**
     * Constructs a deserializer that is capable of deserializing a Sample.
     * 
     * @param type The type of the Sample to deserialize.
     * @param kind The kind of serializer that is needed. Currently only XML 
     *             is supported.
     * @return A deserializer that is capable of deserializing a Sample
     */
    public static SampleDeserializer getSampleDeserializer(MetaType type, int kind){
        SampleDeserializer ss = null;
        
        if(kind == XML){
            try {
                ss = new SampleDeserializerXML(type);
            }
            catch (ParserConfigurationException e) {
                logger.logp(Level.SEVERE, "DataHandlerFactory", "getSampleDeserializer", "ParserConfigurationException: " + e.getMessage());
                System.err.println("Parser could not be intialized.\nUnrecoverable exception. Bailing out...");
                System.exit(0);
            }
            catch (SAXException e) {
                logger.logp(Level.SEVERE, "DataHandlerFactory", "getSampleDeserializer", "SAXException: " + e.getMessage());
                System.err.println("Parser could not be intialized.\nUnrecoverable exception. Bailing out...");
                System.exit(0);
            }
        }
        return ss;
    }
    
    public static SampleSerializer getSampleSerializer(int kind){
        SampleSerializer ss = null;
        
        if(kind == XML){
            ss = new SampleSerializerXML();
        }
        return ss;
    }
    
    /**
     * Constructs a deserializer that is capable of deserializing a Sample. The
     * difference between the deserializer that this function returns and the
     * one returned when supplying a MetaType, lies in the way that the
     * deserializer must be accessed afterwards. When deserializing a Sample 
     * with a deserializer returned by this function, a MetaType must be 
     * supplied for each deserialize action.
     * 
     * @param kind The kind of serializer that is needed. Currently only XML 
     *             is supported.
     * @return A deserializer that is capable of deserializing a Sample
     */
    public static SampleDeserializer getUntypedSampleDeserializer(int kind){
        SampleDeserializer ss = null;
        
        if(kind == XML){
            try {
                ss = new SampleDeserializerXML();
            }
            catch (ParserConfigurationException e) {
                logger.logp(Level.SEVERE, "DataHandlerFactory", "getUntypedSampleDeserializer", "ParserConfigurationException: " + e.getMessage());
                System.err.println("Parser could not be intialized.\nUnrecoverable exception. Bailing out...");
                System.exit(0);
            }
            catch (SAXException e) {
                logger.logp(Level.SEVERE, "DataHandlerFactory", "getUntypedSampleDeserializer", "SAXException: " + e.getMessage());
                System.err.println("Parser could not be intialized.\nUnrecoverable exception. Bailing out...");
                System.exit(0);
            }
        }
        return ss;
    }

    /**
     * Constructs a deserializer that is capable of deserializing a Storage and
     * its related return-/parameter-types.
     *
     * @param kind The kind of serializer that is needed. Currently only XML
     *             is supported.
     * @return A deserializer that is capable of deserializing a Storage
     */
    public static StorageDeserializer getStorageDeserializer(int kind){
        StorageDeserializer sd = null;

        if(kind == XML){
            try {
                sd = new StorageDeserializerXML();
            }
            catch (ParserConfigurationException e) {
                logger.logp(Level.SEVERE, "DataHandlerFactory", "getStorageDeserializer", "ParserConfigurationException: " + e.getMessage());
                System.err.println("Parser could not be intialized.\nUnrecoverable exception. Bailing out...");
                System.exit(0);
            }
        }
        return sd;
    }

    /**
     * Constructs a serializer that is capable of serializing a Storage and
     * its related return-/parameter-types.
     *
     * @param kind The kind of serializer that is needed. Currently only XML
     *             is supported.
     * @return A serializer that is capable of serializing a Storage
     */
    public static StorageSerializer getStorageSerializer(int kind){
        StorageSerializer ss = null;

        if(kind == XML){
            ss = new StorageSerializerXML();
        }
        return ss;
    }

    /**
     * Constructs a deserializer that is capable of deserializing a type.
     * 
     * @param kind The kind of deserializer that is needed. Currently only XML is 
     *             supported.
     * @return A deserializer that is capable of deserializing a MetaType.
     */
    public static MetaTypeDeserializer getMetaTypeDeserializer(int kind){
        MetaTypeDeserializer udts = null;
        
        if(kind == XML){
            try {
                udts = new MetaTypeDeserializerXML();
            }
            catch (ParserConfigurationException e) {
                logger.logp(Level.SEVERE, "DataHandlerFactory", "getSampleDeserializer", "ParserConfigurationException: " + e.getMessage());
                System.err.println("Parser could not be intialized.\nUnrecoverable exception. Bailing out...");
                System.exit(0);
            }
        }
        return udts;
    }
    
    /**
     * Constructs a serializer that is capable of serializing UserData.
     * 
     * @param kind The kind of serializer that is needed. Currently only XML 
     *             is supported.
     * @return A serializer that is capable of serializing UserData.
     */
    public static UserDataSerializer getUserDataSerializer(int kind){
        UserDataSerializer uds = null;
        
        if(kind == XML){
            uds = new UserDataSerializerXML();    
        }
        return uds; 
    }
    
    /**
     * Constructs a deserializer that is capable of deserializing a Entity.
     * 
     * @param kind The kind of deserializer that is needed. Currently only XML
     *             is supported. 
     * @return A deserializer that is capable of deserializing Entity objects.
     */
    public static EntityDeserializer getEntityDeserializer(Communicator communicator, int kind){
        EntityDeserializer eds = null;
        
        if(kind == XML){
            try {
                eds = new EntityDeserializerXML(communicator);
            }
            catch (ParserConfigurationException e) {
                logger.logp(Level.SEVERE, "DataHandlerFactory", "getEntityDeserializer", "ParserConfigurationException: " + e.getMessage());
                System.err.println("Parser could not be intialized.\nUnrecoverable exception. Bailing out...");
                System.exit(0);
            }
        }
        return eds;
    }
    
    /**
     * Constructs a serializer that is capable of serializing a Entity.
     * 
     * @param kind The kind of serializer that is needed. Currently only XML
     *             is supported. 
     * @return A serializer that is capable of serializing Entity objects.
     */
    public static EntitySerializer getEntitySerializer(int kind){
        EntitySerializer es = null;
        
        if(kind == XML){
            es = new EntitySerializerXML();    
        }
        return es; 
    }
    
    /**
     * Constructs a deserializer that is capable of deserializing a Status.
     * 
     * @param kind The kind of serializer that is needed. Currently only XML
     *             is supported.
     * @return A deserializer that is capable of deserializing Status objects.
     */
    public static StatusDeserializer getStatusDeserializer(int kind){
        StatusDeserializer sd = null;
        
        if(kind == XML){
            try {
                sd = new StatusDeserializerXML();
            } catch (ParserConfigurationException e) {
                logger.logp(Level.SEVERE, "DataHandlerFactory", "getStatusDeserializer", "ParserConfigurationException: " + e.getMessage());
                System.err.println("Parser could not be intialized.\nUnrecoverable exception. Bailing out...");
                System.exit(0);
            }
        }
        return sd;
    }
    
    /**
     * Constructs a deserializer that is capable of deserializing a Snapshot.
     * 
     * @param kind The kind of serializer that is needed. Currently only XML
     *             is supported.
     * @return A deserializer that is capable of deserializing Snapshot objects.
     */
    public static SnapshotDeserializer getSnapshotDeserializer(Communicator communicator, int kind){
        SnapshotDeserializer sd = null;
        
        if(kind == XML){
            try {
                sd = new SnapshotDeserializerXML(communicator);
            } catch (ParserConfigurationException e) {
                logger.logp(Level.SEVERE, "DataHandlerFactory", "getSnapshotDeserializer", "ParserConfigurationException: " + e.getMessage());
                System.err.println("Parser could not be intialized.\nUnrecoverable exception. Bailing out...");
                System.exit(0);
            }
        }
        return sd;
    }
    
    /**
     * Constructs a serializer that is capable of serializing a Snapshot.
     * 
     * @param kind The kind of serializer that is needed. Currently only XML
     *             is supported.
     * @return A serializer that is capable of serializing Snapshot objects.
     */
    public static SnapshotSerializer getSnapshotSerializer(int kind){
        SnapshotSerializer ss = null;
        
        if(kind == XML){
            ss = new SnapshotSerializerXML();    
        }
        return ss; 
    }
    
    public static QoSDeserializer getQoSDeserializer(int kind){
        QoSDeserializer qd = null;
        
        if(kind == XML){
            try {
                qd = new QoSDeserializerXML();
            } catch (ParserConfigurationException e) {
                logger.logp(Level.SEVERE, "DataHandlerFactory", "getQoSDeserializer", "ParserConfigurationException: " + e.getMessage());
                System.err.println("Parser could not be intialized.\nUnrecoverable exception. Bailing out...");
                System.exit(0);
            }
        }
        
        return qd;
    }
    
    public static QoSSerializer getQoSSerializer(int kind){
        QoSSerializer qd = null;
        
        if(kind == XML){
            qd = new QoSSerializerXML();
        }
        
        return qd;
    }
    
    public static StatisticsDeserializer getStatisticsDeserializer(int kind){
        StatisticsDeserializer sd = null;
        
        if(kind == XML){
            try {
                sd = new StatisticsDeserializerXML();
            } catch (ParserConfigurationException e) {
                logger.logp(Level.SEVERE, "DataHandlerFactory", "getStatisticsDeserializer", "ParserConfigurationException: " + e.getMessage());
                System.err.println("Parser could not be intialized.\nUnrecoverable exception. Bailing out...");
                System.exit(0);
            }
        }
        return sd;
    }
     
    /**
     * Provides logging facilities.
     */
    private static Logger logger = Logger.getLogger("splice.tooling.monitor.model.data");
    
    /**
     * Transformer kind that specifies that transformation from or to XML is needed.
     */
    public static final int XML = 0;
}

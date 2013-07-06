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
package org.opensplice.common.model.sample;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;

import org.opensplice.cm.CMException;
import org.opensplice.cm.DataTypeUnsupportedException;
import org.opensplice.cm.Entity;
import org.opensplice.cm.EntityFilter;
import org.opensplice.cm.Query;
import org.opensplice.cm.Reader;
import org.opensplice.cm.Snapshot;
import org.opensplice.cm.Topic;
import org.opensplice.cm.data.Sample;
import org.opensplice.cm.qos.TopicQoS;
import org.opensplice.cm.transform.DataTransformerFactory;
import org.opensplice.cm.transform.QoSSerializer;
import org.opensplice.cm.transform.SampleSerializer;
import org.opensplice.cm.transform.TransformationException;
import org.opensplice.common.CommonException;
import org.opensplice.common.SampleModelSizeException;
import org.opensplice.common.model.table.UserDataSingleTableModel;
import org.opensplice.common.model.table.UserDataTableModel;

/**
 * Abstract SampleModel descendant which represents a model that holds
 * a Snapshot. This class has been defined abstract, because only concrete
 * descendants of this class may actually exist. 
 * 
 * @date Nov 17, 2004 
 */
public abstract class SnapshotSampleModel extends SampleModel{
    protected Snapshot snapshot;
    
    /**
     * Constructs the model for the supplied ReaderSnapshot.
     *
     * @param _snapshot The snapshot where data is read/taken from.
     * @throws CMException Thrown when the data type could not be retrieved.
     */
    public SnapshotSampleModel(Snapshot _snapshot) throws CommonException{
        super();
        snapshot = _snapshot;
        
        try {
            userDataModel = new UserDataTableModel(snapshot.getUserDataType());
            singleUserDataModel = new UserDataSingleTableModel(
                                            snapshot.getUserDataType(), false);
        } catch(CMException e){
            throw new CommonException(e.getMessage());
        } catch (DataTypeUnsupportedException e) {
            throw new CommonException(e.getMessage());
        }
    }

    /**
     * Reads a Sample from its reader.
     * @throws SampleModelSizeException
     */
    @Override
    public Sample read() throws CommonException, SampleModelSizeException {
        Sample result = null;
        this.checkSize();
        
        try {
            result = snapshot.read();
        } catch (CMException e) {
            throw new CommonException(e.getMessage());
        } catch (DataTypeUnsupportedException e) {
            throw new CommonException(e.getMessage());
        }
        if(result != null){
            boolean added = this.addSample(result);
            
            if(added && (result != null)){
                this.notifyListeners("data_read");
            }
        } else {
            this.notifyListeners("no_data_available");
        }
        return result;
    }

    /**
     * Takes a Sample from its reader.
     * @throws SampleModelSizeException
     */
    @Override
    public Sample take() throws CommonException, SampleModelSizeException {
        Sample result = null;
        this.checkSize();
        
        try {
            result = snapshot.take();
        } catch (CMException e) {
            throw new CommonException(e.getMessage());
        } catch (DataTypeUnsupportedException e) {
            throw new CommonException(e.getMessage());
        }
        if(result != null){
            boolean added = this.addSample(result);
            
            while(!added && result != null){
                try {
                    result = snapshot.take();
                    
                    if(result != null){
                        added = this.addSample(result);
                    }
                } catch (CMException e) {
                    throw new CommonException(e.getMessage());
                } catch (DataTypeUnsupportedException e) {
                    throw new CommonException(e.getMessage());
                }
            }
            if(result != null){
                this.notifyListeners("data_take");
            } else {
                this.notifyListeners("no_data_available");
            }
        } else {
            this.notifyListeners("no_data_available");
        }
        return result;
    }
    
    @Override
    public Sample readNext() throws CommonException, SampleModelSizeException{
        throw new CommonException("readNext not supported for snapshots.");
    }
    
    /**
     * Provides access to the Snapshot of this model.
     * 
     * @return
     */
    public Snapshot getSnapshot(){
        return snapshot;
    }
    
    protected abstract Topic getTopic() throws CommonException;
    protected abstract String getPartitions() throws CommonException;
    
    @Override
    public synchronized int export(File file) throws CommonException{
        Topic top;
        TopicQoS qos;
        QoSSerializer ser;
        String xmlQos;
        SampleSerializer sampleSer;
        OutputStreamWriter fw = null;
        Sample sample;
        int i = 0;
                
        if(!file.exists()){
            try {
                file.createNewFile();
            } catch (IOException e1) {
                throw new CommonException("Cannot create file.");
            }
        }
        if((!file.canRead()) || !(file.canWrite())){
            throw new CommonException("Cannot open file (insufficient rights).");
        }
        
        try {
            ser = DataTransformerFactory.getQoSSerializer(DataTransformerFactory.XML);
            sampleSer = DataTransformerFactory.getSampleSerializer(DataTransformerFactory.XML);
            
            fw = new OutputStreamWriter(new FileOutputStream(file, true), "UTF-8");
            top = this.getTopic();
            
            if (top != null) {
                qos = (TopicQoS) top.getQoS();
                xmlQos = ser.serializeQoS(qos);

                fw.write("<splice_data><partitions>");
                String parts = this.getPartitions();

                if (parts != null) {
                    fw.write(parts);
                }
                fw.write("</partitions><topic><name>");
                fw.write(top.getName());
                fw.write("</name><typeName>");
                fw.write(top.getTypeName());
                fw.write("</typeName><keyList>");
                
                if (top.getKeyList() != null) {
                    fw.write(top.getKeyList());
                }
                fw.write("</keyList><qos>");
                fw.write(xmlQos);
                fw.write("</qos><metadata>");
                fw.write(this.userDataModel.getUserDataType().toXML());
                fw.write("</metadata></topic><data>");

                do {
                    sample = userDataModel.getDataAt(i);

                    if (sample != null) {
                        fw.write(sampleSer.serializeSample(sample));
                    }
                    i++;
                } while (sample != null);

                fw.write("</data></splice_data>");
            }
            fw.flush();
            fw.close();
        } catch (FileNotFoundException e) {
            throw new CommonException(e.getMessage());
        } catch (IOException ie) {
            throw new CommonException(ie.getMessage());
        } catch (CMException ce) {
            throw new CommonException(ce.getMessage());
        } catch (TransformationException te) {
            throw new CommonException(te.getMessage());
        } finally {
            if (fw != null) {
                try {
                    fw.close();
                } catch (IOException ie) {
                    throw new CommonException(ie.getMessage());
                }
            }
        }
        return i-1;
    }

    /**
     * Provides access to the Topic of the supplied Reader.
     * 
     * @param r The Reader to find the Topic of.
     * @return The Topic of the supplied Reader.
     * @throws CommonException Thrown when the Reader is not available.
     */
    protected Topic getTopic(Reader r) throws CommonException{
        Topic result = null;
        Entity[] entities;
        Entity tmp = null;
        boolean found = false;
        
        if(r instanceof Query){
            try {
                entities = r.getDependantEntities(EntityFilter.READER);
            } catch (CMException e) {
                throw new CommonException(e.getMessage());
            }
            if(entities.length == 0){
                throw new CommonException("Query reader has no source.");
            }
            r = (Reader)entities[0];
            
            for(int i=0; i<entities.length; i++){
                if(entities[i] != r){ 
                    entities[i].free();
                }
            }
        }
        
        try {
            entities = r.getOwnedEntities(EntityFilter.ENTITY);
            
            for(int i=0; i<entities.length && !found; i++){
                tmp = entities[i];
                
                if(tmp instanceof Topic){
                    found = true;
                    result = (Topic)tmp;
                }
            }
            for(int i=0; i<entities.length; i++){
                if(entities[i] != result){ 
                    entities[i].free();
                }
            }
        } catch (CMException e) {
            throw new CommonException(e.getMessage());
        }
        
        return result;
    }
}

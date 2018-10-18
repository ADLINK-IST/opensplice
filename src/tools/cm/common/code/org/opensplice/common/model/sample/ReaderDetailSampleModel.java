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
import org.opensplice.cm.Topic;
import org.opensplice.cm.Writer;
import org.opensplice.cm.data.Sample;
import org.opensplice.cm.data.UserData;
import org.opensplice.cm.meta.MetaType;
import org.opensplice.cm.qos.TopicQoS;
import org.opensplice.cm.transform.DataTransformerFactory;
import org.opensplice.cm.transform.QoSSerializer;
import org.opensplice.cm.transform.SampleSerializer;
import org.opensplice.cm.transform.TransformationException;
import org.opensplice.cmdataadapter.CmDataException;
import org.opensplice.cmdataadapter.TypeInfo.TypeEvolution;
import org.opensplice.common.CommonException;
import org.opensplice.common.SampleModelSizeException;
import org.opensplice.common.model.TypeHandler;
import org.opensplice.common.model.table.UserDataSingleTableModel;
import org.opensplice.common.model.table.UserDataTableModel;

/**
 * Concrete descendant of the SampleModel component. This components
 * administrates the Sample objects read/taken from a Reader entity.
 *
 * @date Oct 27, 2004
 */
public class ReaderDetailSampleModel extends DetailSampleModel{
    /**
     * The reader where data is read/taken from.
     */
    protected Reader reader;


    /**
     * Constructs the model for the supplied Reader.
     *
     * @param _reader The Reader where data is read/taken from.
     * @throws CMException Thrown when the reader is no longer available or the
     *                     data type of its contents cannot be found.
     */
    public ReaderDetailSampleModel(Reader _reader, String structName, Sample sample, TypeEvolution _typeEvolution) throws CommonException{
        super(structName);

        if(_reader == null){
            throw new CommonException("Supplied reader == null.");
        }
        reader = _reader;
        Topic topic = null;
        try {
            topic = getTopic(_reader);
            initModels(topic, structName, _typeEvolution);
        } finally {
            if (topic != null) {
                topic.free();
            }
        }
        this.addSample(sample,structName);
    }
    
    /**
     * Constructs the model for the supplied Reader.
     *
     * @param _reader The Reader where data is read/taken from.
     * @throws CMException Thrown when the reader is no longer available or the
     *                     data type of its contents cannot be found.
     */
    public ReaderDetailSampleModel(Reader _reader, String structName, UserData data, TypeEvolution _typeEvolution) throws CommonException{
        super(structName);

        if(_reader == null){
            throw new CommonException("Supplied reader == null.");
        }
        reader = _reader;
        Topic topic = null;
        try {
            topic = getTopic(_reader);
            initModels(topic, structName, _typeEvolution);
        } finally {
            if (topic != null) {
                topic.free();
            }
        }
        this.addSample(data,structName);
    }

    /**
     * Constructs the model for the supplied Reader.
     *
     * @param _reader The Reader where data is read/taken from.
     * @throws CMException Thrown when the reader is no longer available or the
     *                     data type of its contents cannot be found.
     */
    protected ReaderDetailSampleModel(Writer _writer, String structName, UserData data, TypeEvolution _typeEvolution) throws CommonException{
        super(structName);
        if(_writer == null){
            throw new CommonException("Supplied writer == null.");
        }
        Topic topic = null;
        try {
            topic = getTopic(_writer);
            initModels(topic, structName, _typeEvolution);
        } finally {
            if (topic != null) {
                topic.free();
            }
        }
        this.addSample(data,structName);
    }

    private void initModels(Topic topic, String structName, TypeEvolution tEvo) throws CommonException {
        try {
            typeEvolution = tEvo;
            typeInfo = TypeHandler.getTypeHandler().getTypeInfo(topic);
            MetaType metaType = typeInfo.getMetaType(tEvo);
            userDataModel = new UserDataTableModel(metaType, structName);
            singleUserDataModel = new UserDataSingleTableModel(metaType, false, structName);
            toBeWrittenUserData = new UserData(metaType);
        } catch (CmDataException e) {
            throw new CommonException(e.getMessage());
        }
    }

    public void updateSample(Sample sample, String structName) {
        this.addSample(sample,structName);
    }
    
    public void updateSample(UserData data, String structName) {
        this.addSample(data,structName);
    }

    @Override
    public Sample read() throws CommonException, SampleModelSizeException {
        Sample result = null;
        this.checkSize();

        try {
            result = reader.read();
        } catch (CMException e) {
            throw new CommonException(e.getMessage());
        } catch (DataTypeUnsupportedException de) {
            throw new CommonException(de.getMessage());
        }
        if(result != null){
            boolean added;
            if (structDetail != null) {
                added = this.addSample(result,structDetail);
            } else {
                added = this.addSample(result);
            }

            if (added) {
                this.notifyListeners("data_read");
            }
        } else {
            this.notifyListeners("no_data_available");
        }
        return result;
    }

    @Override
    public Sample take() throws CommonException, SampleModelSizeException {
        Sample result = null;
        this.checkSize();

        try {
            result = reader.take();
        } catch (CMException e) {
            throw new CommonException(e.getMessage());
        } catch (DataTypeUnsupportedException de) {
            throw new CommonException(de.getMessage());
        }
        if(result != null){
            boolean added;
            if (structDetail != null) {
                added = this.addSample(result,structDetail);
            } else {
                added = this.addSample(result);
            }


            while(!added && result != null){
                try {
                    result = reader.take();

                    if(result != null){
                        if (structDetail != null) {
                            added = this.addSample(result,structDetail);
                        } else {
                            added = this.addSample(result);
                        }

                    }
                } catch (CMException e) {
                    throw new CommonException(e.getMessage());
                } catch (DataTypeUnsupportedException e2) {
                    throw new CommonException(e2.getMessage());
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
    public Sample readNext() throws CommonException, SampleModelSizeException {
        Sample result = null;
        this.checkSize();

        if(lastReadSample == null){
            result = this.read();
        } else {
            try {
                result = reader.readNext(lastReadSample.getMessage().getInstanceGid());
            } catch (CMException e) {
                throw new CommonException(e.getMessage());
            } catch (DataTypeUnsupportedException de) {
                throw new CommonException(de.getMessage());
            }
            if(result != null){
                boolean added;
                if (structDetail != null) {
                    added = this.addSample(result,structDetail);
                } else {
                    added = this.addSample(result);
                }


                if (added) {
                    this.notifyListeners("data_read");
                }
            } else {
                this.notifyListeners("no_data_available");
            }
        }
        return result;
    }

    @Override
    public synchronized int export(File file) throws CommonException{
        Topic top;
        TopicQoS qos;
        QoSSerializer ser;
        OutputStreamWriter fw = null;
        String xmlQos;
        Sample sample;
        SampleSerializer sampleSer;
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

            fw = new OutputStreamWriter(new FileOutputStream(file, false), "UTF-8");
            top = this.getTopic(reader);

            qos = (TopicQoS)top.getQoS();
            xmlQos = ser.serializeQoS(qos);

            fw.write("<splice_data><partitions>");
            String parts = this.getPartitions(reader);

            if(parts != null){
                fw.write(parts);
            }
            fw.write("</partitions><topic><name>");
            fw.write(top.getName());
            fw.write("</name><typeName>");
            fw.write(top.getTypeName());
            fw.write("</typeName><keyList>");

            if(top.getKeyList() != null){
                fw.write(top.getKeyList());
            }
            fw.write("</keyList><qos>");
            fw.write(xmlQos);
            fw.write("</qos><metadata>");
            fw.write(this.userDataModel.getUserDataType().toXML());
            fw.write("</metadata></topic><data>");

            do{
                sample = userDataModel.getDataAt(i);

                if(sample != null){
                    fw.write(sampleSer.serializeSample(sample));
                }
                i++;
            } while(sample != null);

            fw.write("</data></splice_data>");
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

    protected Topic getTopic(Entity entity) throws CommonException{
        Topic result = null;
        Entity[] entities;
        Entity tmp = null;
        Reader r = (Reader) reader;
        boolean found = false;

        if(reader instanceof Query){
            try {
                entities = reader.getDependantEntities(EntityFilter.READER);
            } catch (CMException e) {
                throw new CommonException(e.getMessage());
            }
            if(entities.length == 0){
                throw new CommonException("Query reader has no source.");
            }
            r = (Reader)entities[0];

            for(int i=1; i<entities.length; i++){
                entities[i].free();
            }
        }

        try {
            entities = r.getOwnedEntities(EntityFilter.ENTITY);

            if(reader instanceof Query){
                r.free();
            }

            for(int i=0; i<entities.length && !found; i++){
                tmp = entities[i];

                if(tmp instanceof Topic){
                    found = true;
                }
            }
            for(int i=0; i<entities.length; i++){
                if(entities[i] != tmp){
                    entities[i].free();
                }
            }
            if(tmp != null){
                result = (Topic)tmp;

            } else {
                throw new CommonException("Topic could not be retrieved.");
            }

        } catch (CMException e) {
            throw new CommonException(e.getMessage());
        }

        return result;
    }

    /**
     * Provides access to the Reader.
     *
     * @return The Reader where data is read from.
     */
    public Reader getReader(){
        return reader;
    }
}

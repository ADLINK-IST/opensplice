/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
import java.util.ArrayList;

import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.opensplice.cm.CMException;
import org.opensplice.cm.DataTypeUnsupportedException;
import org.opensplice.cm.Participant;
import org.opensplice.cm.Partition;
import org.opensplice.cm.Publisher;
import org.opensplice.cm.Topic;
import org.opensplice.cm.Writer;
import org.opensplice.cm.data.Sample;
import org.opensplice.cm.data.UserData;
import org.opensplice.cm.meta.MetaType;
import org.opensplice.cm.qos.PublisherQoS;
import org.opensplice.cm.qos.QoS;
import org.opensplice.cm.qos.TopicQoS;
import org.opensplice.cm.qos.WriterQoS;
import org.opensplice.cm.transform.DataTransformerFactory;
import org.opensplice.cm.transform.MetaTypeDeserializer;
import org.opensplice.cm.transform.QoSDeserializer;
import org.opensplice.cm.transform.QoSSerializer;
import org.opensplice.cm.transform.SampleDeserializer;
import org.opensplice.cm.transform.SampleSerializer;
import org.opensplice.cm.transform.TransformationException;
import org.opensplice.cmdataadapter.CmDataException;
import org.opensplice.cmdataadapter.TypeInfo;
import org.opensplice.common.CommonException;
import org.opensplice.common.SampleModelSizeException;
import org.opensplice.common.model.table.UserDataSingleTableModel;
import org.opensplice.common.model.table.UserDataTableModel;
import org.opensplice.common.util.Report;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

/**
 * Concrete descendant of the SampleModel that can be used to import metadata
 * and or data from file into a SPLICE-DDS system.
 *
 * @date Apr 5, 2005
 */
public class ImportSampleModel extends SampleModel {
    private Document document = null;
    private String topicName = null;
    private String topicTypeName = null;
    private String topicKeyList = null;
    private TopicQoS topicQoS = null;
    private Topic topic = null;
    private String partitionExpression = null;
    private Partition[]        partitions          = null;
    private ArrayList<Node>    dataNodes           = null;
    private int dataNodesIndex = 0;
    private SampleDeserializer sd = null;
    private Writer writer = null;
    private Publisher publisher = null;
    private Participant participant = null;
    private boolean initialized = false;
    private boolean metaDataWritten = false;

    /**
     * Constructs a new ImportSampleModel from the supplied parameters.
     *
     * @param file
     *            The file where the metadata and data is located in.
     * @param participant
     *            The participant that determines the domain and node of the
     *            SPLICE-DDS system.
     * @param partitionExpression
     *            The partition(s) where the data must be imported in. When null
     *            is provided the model takes the partitions where the data was
     *            exported from.
     * @throws CommonException
     *             Thrown when: - The supplied participant is not valid. - The
     *             supplied file is not valid.
     */
    public ImportSampleModel(File file, Participant participant, String partitionExpression) throws CommonException{
        super();

        if(participant == null){
            throw new CommonException("Supplied participant not valid.");
        }
        this.participant = participant;

        try {
            DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
            document = dbf.newDocumentBuilder().parse(file);
            Element rootElement = document.getDocumentElement();

            if(!("splice_data".equals(rootElement.getNodeName()))){
                throw new CommonException("Input file not valid: '<splice_data>' expected");
            }
            Node topicEl = this.getChildNode(rootElement, "topic");

            if(topicEl == null){
                throw new CommonException("Input file not valid: no topic defined.");
            }

            this.resolveTopicName((Element)topicEl);
            this.resolveTopicTypeName((Element)topicEl);
            this.resolveTopicKeyList((Element)topicEl);
            this.resolveTopicQoS((Element)topicEl);
            if(topicEl instanceof Element){
                this.initializeMetaType((Element)topicEl);
            } else {
                throw new CommonException("Input file not valid: topic definition in file not valid.");
            }

            if(partitionExpression == null){
                this.resolvePartitions(rootElement);
            } else {
                this.partitionExpression = partitionExpression;
            }

            if(this.partitionExpression == null){
                throw new CommonException("No partitions in file and not specified as parameter.");
            }

            sd = DataTransformerFactory
                    .getSampleDeserializer(typeInfo.getBareMetaType(),
                            DataTransformerFactory.XML);
            NodeList dataList = rootElement.getElementsByTagName("data");

            if(dataList.getLength() != 1){
                throw new CommonException("Input file not valid: #data defined != 1");
            }
            Node dataNode = dataList.item(0);
            dataNodes = new ArrayList<Node>();
            NodeList list = dataNode.getChildNodes();

            for(int i=0; i<list.getLength(); i++){
                dataNode = list.item(i);

                if("object".equals(dataNode.getNodeName())){
                    dataNodes.add(dataNode);
                }
            }

        } catch (SAXException se) {
            throw new CommonException("Input file not valid: " + se.getMessage());
        } catch (IOException ie) {
            throw new CommonException("IOException: " + ie.getMessage());
        } catch (ParserConfigurationException pe) {
            throw new CommonException("Could not instantiate parser: " + pe.getMessage());
        }
    }

    /**
     * Reads one Sample from the file.
     *
     * @throws CommonException
     *             Thrown when the sample could not be deserialized.
     * @throws SampleModelSizeException
     *             Thrown when to many samples are in the model.
     */
    @Override
    public Sample read() throws CommonException, SampleModelSizeException {
        Sample result = null;
        this.checkSize();

        if(dataNodes.size() > dataNodesIndex){
            Node sampleNode = dataNodes.get(dataNodesIndex);

            if(sampleNode instanceof Element){
                try {
                    result = sd.deserializeSample(sampleNode);
                } catch (TransformationException te) {
                    throw new CommonException(te.getMessage());
                }
            }
            if(result != null){
                try {
                    result = typeInfo.adaptDataForRead(typeInfo.getMostRecentEvolution(), result);
                } catch (CmDataException e) {
                    throw new CommonException(e.getMessage());
                }
                boolean added = this.addSample(result);

                if (added) {
                    this.notifyListeners("data_read");
                }
            }
        } else {
            this.notifyListeners("no_data_available");
        }
        return result;
    }

    /**
     * Takes one Sample from the file.
     *
     * @throws CommonException
     *             Thrown when the sample could not be deserialized.
     * @throws SampleModelSizeException
     *             Thrown when to many samples are in the model.
     */
    @Override
    public Sample take() throws CommonException, SampleModelSizeException {
        Sample result;
        boolean added = false;

        do{
            result = null;

            if(dataNodes.size() > dataNodesIndex){
                Node sampleNode = dataNodes.get(dataNodesIndex);
                dataNodesIndex++;

                if(sampleNode instanceof Element){
                    try {
                        result = sd.deserializeSample(sampleNode);
                    } catch (TransformationException te) {
                        throw new CommonException(te.getMessage());
                    }
                }
            }
            if(result != null){
                try {
                    result = typeInfo.adaptDataForRead(typeInfo.getMostRecentEvolution(), result);
                } catch (CmDataException e) {
                    throw new CommonException(e.getMessage());
                }
                this.checkSize();
                added = this.addSample(result);
            }
        } while((result != null) && (!added));

        if(added && (result != null)){
            this.notifyListeners("data_take");
        } else {
            this.notifyListeners("no_data_available");
        }
        return result;
    }

    /**
     * Reads one Sample from the file.
     *
     * @throws CommonException
     *             Thrown when the sample could not be deserialized.
     * @throws SampleModelSizeException
     *             Thrown when to many samples are in the model.
     */
    @Override
    public Sample readNext() throws CommonException, SampleModelSizeException {
        Sample result = null;

        result = this.read();

        if(result != null){
            dataNodesIndex++;
        }
        return result;
    }

    @Override
    public int export(File file) throws CommonException {
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
            fw = new OutputStreamWriter(new FileOutputStream(file, true), "UTF-8");

            NodeList topics = document.getDocumentElement().getElementsByTagName("topic");
            Element topicElement = (Element)topics.item(0);
            String name = this.resolveTopicName(topicElement);
            String typeName = this.resolveTopicTypeName(topicElement);
            String keyList = this.resolveTopicKeyList(topicElement);
            TopicQoS qos = this.resolveTopicQoS(topicElement);

            xmlQos = ser.serializeQoS(qos);

            fw.write("<splice_data><partitions>");

            if(partitionExpression != null){
                fw.write(partitionExpression);
            }
            fw.write("</partitions><topic><name>");
            fw.write(name);
            fw.write("</name><typeName>");
            fw.write(typeName);
            fw.write("</typeName><keyList>");

            if(keyList != null){
                fw.write(keyList);
            }
            fw.write(keyList);
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
     * This action writes the metadata to the connected SPLICE-DDS system and
     * creates a Topic for this type. Subsequent calls to this function have no
     * effect.
     *
     * To properly clean up all created entities in the model once it is
     * initialized, the deinitialize() function should be called when the model
     * does not have to be used anymore.
     *
     * @throws CommonException
     *             Thrown when the meta data is already available in the
     *             SPLICE-DDS system, but differs from the meta data in this
     *             model.
     */
    public void initialize() throws CommonException{
        if(!initialized) {
            this.writeMetadata();
            this.init(true);
        }
    }

    /**
     * Cleans up all entities created by the model. Once this function is
     * called, the model cannot be used anymore.
     *
     *
     * @throws CommonException
     *             Thrown when the model could not be deinitialized.
     */
    public void deinitialize() throws CommonException{
        this.init(false);
    }

    /**
     * Writes the data at the selected index in the model, to the SPLICE-DDS
     * system.
     *
     * @param index
     *            The index in the table that holds the data that must be
     *            injected in the system.
     * @throws CommonException
     *             Thrown when: - The model is not initialized (initialize()) -
     *             No data is available at the specified index.
     */
    public void writeAt(int index) throws CommonException {
        if(!initialized){
            throw new CommonException("Not initialized.");
        }

        try {
            Sample sample = userDataModel.getDataAt(index);

            if(sample != null){
                UserData data = sample.getMessage().getUserData();
                try {
                    data = typeInfo.adaptDataForWrite(typeInfo.getMostRecentEvolution(), data);
                } catch (CmDataException e) {
                    throw new CommonException(e.getMessage());
                }
                writer.write(data);
            } else {
                throw new CommonException("No data selected.");
            }
        } catch (CMException ce) {
            throw new CommonException(ce.getMessage());
        }
    }

    /**
     * Diposes the data at the selected index in the model, to the SPLICE-DDS
     * system.
     *
     * @param index
     *            The index in the table that holds the data that must be
     *            injected in the system.
     * @throws CommonException
     *             Thrown when: - The model is not initialized (initialize()) -
     *             No data is available at the specified index.
     */
    public void disposeAt(int index) throws CommonException {
        if(!initialized){
            throw new CommonException("Not initialized.");
        }

        try {
            Sample sample = userDataModel.getDataAt(index);

            if(sample != null){
                UserData data = sample.getMessage().getUserData();
                try {
                    data = typeInfo.adaptDataForDispose(typeInfo.getMostRecentEvolution(), data);
                } catch (CmDataException e) {
                    throw new CommonException(e.getMessage());
                }
                writer.dispose(data);
            } else {
                throw new CommonException("No data selected.");
            }
        } catch (CMException ce) {
            throw new CommonException(ce.getMessage());
        }
    }

    public void writeDisposeAt(int index) throws CommonException {
        if(!initialized){
            throw new CommonException("Not initialized.");
        }

        try {
            Sample sample = userDataModel.getDataAt(index);

            if(sample != null){
                UserData data = sample.getMessage().getUserData();
                try {
                    data = typeInfo.adaptDataForWrite(typeInfo.getMostRecentEvolution(), data);
                } catch (CmDataException e) {
                    throw new CommonException(e.getMessage());
                }
                writer.writeDispose(data);
            } else {
                throw new CommonException("No data selected.");
            }
        } catch (CMException ce) {
            throw new CommonException(ce.getMessage());
        }
    }

    public void registerAt(int index) throws CommonException {
        if(!initialized){
            throw new CommonException("Not initialized.");
        }

        try {
            Sample sample = userDataModel.getDataAt(index);

            if(sample != null){
                UserData data = sample.getMessage().getUserData();
                try {
                    data = typeInfo.adaptDataForDispose(typeInfo.getMostRecentEvolution(), data);
                } catch (CmDataException e) {
                    throw new CommonException(e.getMessage());
                }
                writer.register(data);
            } else {
                throw new CommonException("No data selected.");
            }
        } catch (CMException ce) {
            throw new CommonException(ce.getMessage());
        }
    }

    public void unregisterAt(int index) throws CommonException {
        if(!initialized){
            throw new CommonException("Not initialized.");
        }

        try {
            Sample sample = userDataModel.getDataAt(index);

            if(sample != null){
                UserData data = sample.getMessage().getUserData();
                try {
                    data = typeInfo.adaptDataForDispose(typeInfo.getMostRecentEvolution(), data);
                } catch (CmDataException e) {
                    throw new CommonException(e.getMessage());
                }
                writer.unregister(data);
            } else {
                throw new CommonException("No data selected.");
            }
        } catch (CMException ce) {
            throw new CommonException(ce.getMessage());
        }
    }

    /**
     * Writes all data in the model to the SPLICE-DDS system.
     *
     * @throws CommonException
     *             Thrown when: - The model is not initialized (initialize())
     */
    public void writeAll() throws CommonException {
        if(!initialized){
            throw new CommonException("Not initialized.");
        }

        try {
            int size = userDataModel.getVisibleContentCount();

            for(int i=0; i<size; i++){
                UserData data = userDataModel.getDataAt(i).getMessage().getUserData();
                try {
                    data = typeInfo.adaptDataForWrite(typeInfo.getMostRecentEvolution(), data);
                } catch (CmDataException e) {
                    throw new CommonException(e.getMessage());
                }
                writer.write(data);
            }
        } catch (CMException ce) {
            throw new CommonException(ce.getMessage());
        }
    }

    /**
     * Disposes all data in the model in the SPLICE-DDS system.
     *
     * @throws CommonException
     *             Thrown when: - The model is not initialized (initialize())
     */
    public void disposeAll() throws CommonException {
        if(!initialized){
            throw new CommonException("Not initialized.");
        }

        try {
            int size = userDataModel.getVisibleContentCount();

            for(int i=0; i<size; i++){
                UserData data = userDataModel.getDataAt(i).getMessage().getUserData();
                try {
                    data = typeInfo.adaptDataForDispose(typeInfo.getMostRecentEvolution(), data);
                } catch (CmDataException e) {
                    throw new CommonException(e.getMessage());
                }
                writer.dispose(data);
            }
        } catch (CMException ce) {
            throw new CommonException(ce.getMessage());
        }
    }

    public void writeDisposeAll() throws CommonException {
        if(!initialized){
            throw new CommonException("Not initialized.");
        }

        try {
            int size = userDataModel.getVisibleContentCount();

            for(int i=0; i<size; i++){
                UserData data = userDataModel.getDataAt(i).getMessage().getUserData();
                try {
                    data = typeInfo.adaptDataForWrite(typeInfo.getMostRecentEvolution(), data);
                } catch (CmDataException e) {
                    throw new CommonException(e.getMessage());
                }
                writer.writeDispose(data);
            }
        } catch (CMException ce) {
            throw new CommonException(ce.getMessage());
        }
    }

    public void registerAll() throws CommonException {
        if(!initialized){
            throw new CommonException("Not initialized.");
        }

        try {
            int size = userDataModel.getVisibleContentCount();

            for(int i=0; i<size; i++){
                UserData data = userDataModel.getDataAt(i).getMessage().getUserData();
                try {
                    data = typeInfo.adaptDataForDispose(typeInfo.getMostRecentEvolution(), data);
                } catch (CmDataException e) {
                    throw new CommonException(e.getMessage());
                }
                writer.register(data);
            }
        } catch (CMException ce) {
            throw new CommonException(ce.getMessage());
        }
    }

    public void unregisterAll() throws CommonException {
        if(!initialized){
            throw new CommonException("Not initialized.");
        }

        try {
            int size = userDataModel.getVisibleContentCount();

            for(int i=0; i<size; i++){
                UserData data = userDataModel.getDataAt(i).getMessage().getUserData();
                try {
                    data = typeInfo.adaptDataForDispose(typeInfo.getMostRecentEvolution(), data);
                } catch (CmDataException e) {
                    throw new CommonException(e.getMessage());
                }
                writer.unregister(data);
            }
        } catch (CMException ce) {
            throw new CommonException(ce.getMessage());
        }
    }

    public void writeMetadata() throws CommonException{
        synchronized (this) {
            if(!metaDataWritten){
                if (topic == null) {
                    try {
                        MetaType type = typeInfo.getBareMetaType();
                        participant.registerType(type);
                        this.initializeTopic(type);
                        metaDataWritten = true;
                    } catch (CMException ce) {
                        throw new CommonException(ce.getMessage());
                    }
                }
            }
        }
    }

    private synchronized void init(boolean init) throws CommonException {
        PublisherQoS qos;

        if(init){
            initialized = true;

            if(publisher == null){
                try{
                    qos = PublisherQoS.getDefault();

                    if(partitionExpression != null){
                        ArrayList<Partition> domains = new ArrayList<Partition>();
                        String[] expr = partitionExpression.split(",");
                        for (int i = 0; i < expr.length; i++) {
                            if (expr[i].indexOf("*") == -1) {
                                Partition p = this.lookupPartition(expr[i]);
                                if (p == null) {
                                    p = participant.createPartition(expr[i]);
                                    domains.add(p);
                                } else {
                                    p.free();
                                }
                                qos.addPartition(p.getName());
                            } else {
                                qos.addPartition(expr[i]);
                            }
                        }
                        // remove leading comma if present
                        String thePartition = qos.getPartition();
                        if (thePartition.startsWith(",")) {
                            qos.setPartition(thePartition.substring(1, thePartition.length()));
                        }
                        partitions = domains.toArray(new Partition[domains
                                .size()]);
                    }
                    
                    publisher = participant.createPublisher("import_publisher", qos);

                    WriterQoS wqos = WriterQoS.copyFromTopicQoS((TopicQoS)topic.getQoS());
                    writer = publisher.createWriter("import_writer", topic, wqos);
                } catch(CMException ce){
                    throw new CommonException(ce.getMessage());
                }
            }
        } else {
            initialized = false;

            if(writer != null){
                writer.free();
                writer = null;
            }
            if(publisher != null){
                publisher.free();
                publisher = null;
            }

            synchronized (this) {
                if (topic != null) {
                    topic.free();
                    topic = null;
                }
            }

            if (partitions != null) {
                for (int i = 0; i < partitions.length; i++) {
                    partitions[i].free();
                }
                partitions = null;
            }

        }
    }

    private Partition lookupPartition(String string) throws CommonException {
        Partition result = null;
        try {
            Partition[] parts;

            synchronized (this) {
                parts = participant.resolveAllDomains();
            }
            for (int i = 0; i < parts.length; i++) {
                if (parts[i].getName().equals(string)) {
                    result = parts[i];
                } else {
                    parts[i].free();
                }
            }
        } catch (CMException ce) {
            throw new CommonException(ce.getMessage());
        }
        return result;
    }

    private void initializeTopic(MetaType type) throws CommonException{
        String errorMessage = null;

        try {
            NodeList topicList = document.getDocumentElement().getElementsByTagName("topic");
            Element topicElement = (Element)topicList.item(0);
            String name = this.resolveTopicName(topicElement);
            Topic myTopic = null;
            Topic[] topics = null;
            synchronized (this) {
                topics = participant.findTopic(name);
            }

            if(topics.length > 0){
                myTopic = topics[0];
            }
            String typeName = this.resolveTopicTypeName(topicElement);
            String keyList = this.resolveTopicKeyList(topicElement);
            TopicQoS qos = this.resolveTopicQoS(topicElement);

            if(myTopic == null){
                StringBuilder keyListNew = new StringBuilder();
                String[] keys = keyList.split(",");

                for(int i=0; i<keys.length; i++){
                    if(keyListNew.length() != 0){
                        keyListNew.append(",");
                    }
                    if(keys[i].startsWith("userData.")){
                        keyListNew.append(keys[i].substring(9));
                    } else {
                        keyListNew.append(keys[i]);
                    }
                }
                this.topic = participant.createTopic(name, typeName, keyListNew.toString(), qos);
            } else if(myTopic.getTypeName().equals(typeName)){
                if(myTopic.getKeyList().equals(keyList)){
                    if(type.equals(myTopic.getDataType())){
                        if(myTopic.getQoS().equals(qos)){
                            this.topic = myTopic;
                        } else {
                            errorMessage = "Topic '" + name + "' is already defined with another QoS.";
                        }
                    } else {
                        errorMessage = "Topic '" + name + "' is already defined with another data type.";
                    }
                } else {
                    errorMessage = "Topic '" + name + "' is already defined with another keyList.";
                }
            } else {
                errorMessage = "Topic '" + name + "' is already defined with another typeName.";
            }

            if(errorMessage != null){
                throw new CommonException(errorMessage);
            }
        } catch (CMException ce) {
            throw new CommonException(ce.getMessage());
        } catch (DataTypeUnsupportedException de) {
            throw new CommonException(de.getMessage());
        }
    }

    private void initializeMetaType(Element topicElement) throws CommonException{
        try {
            NodeList metadatas = topicElement.getChildNodes();
            Node metadata = null;
            Node child = null;

            for(int i=0; (i<metadatas.getLength()) && (metadata == null); i++){
                if(metadatas.item(i) instanceof Element){
                    if(metadatas.item(i).getNodeName().equals("metadata")){
                        metadata = metadatas.item(i);
                    }
                }
            }
            if(metadata == null){
                throw new CommonException("Input file not valid: no metadata found.");
            }
            MetaTypeDeserializer md = DataTransformerFactory.getMetaTypeDeserializer(DataTransformerFactory.XML);
            metadatas = metadata.getChildNodes();

            for(int i=0; (i<metadatas.getLength()) && (child == null); i++){
                if(metadatas.item(i) instanceof Element){
                    child = metadatas.item(i);
                }
            }
            MetaType type = md.deserializeMetaType(child);
            typeInfo = TypeInfo.getTypeInfoByName(topicTypeName);

            String[] keys = topicKeyList.split(",");
            String[] newKeys = new String[keys.length];
            for (int i = 0; i < keys.length; i++) {
                if(keys[i].startsWith("userData.")){
                    newKeys[i] = keys[i].substring(9);
                } else {
                    newKeys[i] = keys[i];
                }
            }
            typeInfo.setBareTopicType(type, newKeys);

            userDataModel = new UserDataTableModel(typeInfo.getMetaType());
            singleUserDataModel = new UserDataSingleTableModel(typeInfo.getMetaType(), false);
        } catch (TransformationException te) {
            Report.getInstance().writeErrorLog("TransformationException");
            throw new CommonException(te.getMessage());
        } catch (DataTypeUnsupportedException de) {
            Report.getInstance().writeErrorLog("DataTypeUnsupportedException");
            throw new CommonException(de.getMessage());
        } catch (CmDataException e) {
            Report.getInstance().writeErrorLog("CmDataException");
            throw new CommonException(e.getMessage());
        }
    }

    private String resolveTopicName(Element topicElement) throws CommonException{
        if(topicName == null){
            NodeList names = topicElement.getChildNodes();
            Node name = null;
            Node tmp;

            for(int i=0; (i<names.getLength()) && (name == null); i++){
                tmp = names.item(i);

                if(tmp instanceof Element){
                    if("name".equals(((Element)tmp).getNodeName())){
                        name = tmp;
                    }
                }
            }

            if(name == null){
                throw new CommonException("Topic name not valid.");
            }
            name = name.getFirstChild();

            if(name == null){
                throw new CommonException("Topic name not valid.");
            }
            topicName = name.getNodeValue();
        }
        return topicName;
    }

    private String resolveTopicTypeName(Element topicElement) throws CommonException{
        if(topicTypeName == null){
            NodeList names = topicElement.getChildNodes();
            Node name = null;
            Node tmp;

            for(int i=0; (i<names.getLength()) && (name == null); i++){
                tmp = names.item(i);

                if(tmp instanceof Element){
                    if("typeName".equals(((Element)tmp).getNodeName())){
                        name = tmp;
                    }
                }
            }

            if(name == null){
                throw new CommonException("Topic type name not valid.");
            }
            name = name.getFirstChild();

            if(name == null){
                throw new CommonException("Topic type name not valid.");
            }
            topicTypeName = name.getNodeValue();
        }
        return topicTypeName;
    }

    private String resolveTopicKeyList(Element topicElement) throws CommonException{
        if(topicKeyList == null){
            NodeList names = topicElement.getChildNodes();
            Node name = null;
            Node tmp;

            for(int i=0; (i<names.getLength()) && (name == null); i++){
                tmp = names.item(i);

                if(tmp instanceof Element){
                    if("keyList".equals(((Element)tmp).getNodeName())){
                        name = tmp;
                    }
                }
            }

            if(name == null){
                throw new CommonException("Topic keyList not valid.");
            }
            name = name.getFirstChild();

            if(name == null){
                topicKeyList = "";
            } else {
                topicKeyList = name.getNodeValue();
            }
        }
        return topicKeyList;
    }

    private TopicQoS resolveTopicQoS(Element topicElement) throws CommonException{
        if(topicQoS == null){
            QoS qos;
            QoSDeserializer qd;

            try {
                NodeList names = topicElement.getChildNodes();
                Node name = null;
                Node tmp;

                for(int i=0; (i<names.getLength()) && (name == null); i++){
                    tmp = names.item(i);

                    if(tmp instanceof Element){
                        if("qos".equals(((Element)tmp).getNodeName())){
                            name = tmp;
                        }
                    }
                }

                if(name == null){
                    throw new CommonException("Topic QoS not valid.");
                }
                qd = DataTransformerFactory.getQoSDeserializer(DataTransformerFactory.XML);

                qos = qd.deserializeQoS(this.getChildNode(name, "object"));

                if(qos == null){
                    throw new CommonException("QoS is not a valid Topic QoS");
                }
                if(!(qos instanceof TopicQoS)){
                    throw new CommonException("QoS is not a valid Topic QoS");
                }
            } catch (TransformationException te) {
                throw new CommonException("Topic QoS not valid (parse error): " + te.getMessage() );
            }

            topicQoS = (TopicQoS)qos;
        }
        return topicQoS;
    }

    private String resolvePartitions(Element rootElement) throws CommonException {
        if(partitionExpression == null){
            NodeList names = rootElement.getChildNodes();
            Node name = null;
            Node tmp;

            for(int i=0; (i<names.getLength()) && (name == null); i++){
                tmp = names.item(i);

                if(tmp instanceof Element){
                    if("partitions".equals(((Element)tmp).getNodeName())){
                        name = tmp;
                    }
                }
            }

            if(name == null){
                throw new CommonException("Partitions not valid.");
            }
            name = name.getFirstChild();

            if(name != null){
                partitionExpression = name.getNodeValue();
            }
        }
        return partitionExpression;
    }

    private Node getChildNode(Node node, String child){
        NodeList members = node.getChildNodes();
        Node childNode = null;

        if(child != null){
            for(int i=0; (i<members.getLength()) && (childNode == null); i++){
                if(child.equals(members.item(i).getNodeName())){
                    childNode = members.item(i);
                }
            }
        }
        return childNode;
    }
}

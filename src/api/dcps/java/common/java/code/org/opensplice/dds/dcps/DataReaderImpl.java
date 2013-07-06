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


package org.opensplice.dds.dcps;

import DDS.DataReaderView;
import DDS.DataReaderViewQos;
import DDS.DataReaderViewQosHolder;
import DDS.Property;
import DDS.PropertyHolder;

import java.util.ArrayList;

/**
 * Implementation of the {@link DDS.DataReader} interface.
 */
public class DataReaderImpl extends DataReaderBase implements DDS.DataReader {
    private ArrayList<Thread> workers = new ArrayList<Thread>();

    /**
     * The address of the parallel demarshalling administration for this reader.
     * This field will be set from a JNI context just after construction of the
     * reader. */
    private long parallelDemarshallingContext = 0;

    /**
     * Starts the given number of worker threads to be used in JNI context.
     * @pre There are currently no worker threads started.
     * @param nrOfWorkers the number of worker-threads to start
     * @return the number of actually started threads
     */
    private int startWorkers(int nrOfWorkers){
        int started = 0;
        class Worker implements Runnable {
            public void run(){
                assert parallelDemarshallingContext != 0;
                jniParallelDemarshallingMain(parallelDemarshallingContext);
            }
        }

        assert workers.size() == 0;

        /* Now start new threads */
        if(parallelDemarshallingContext != 0){
            Worker worker = new Worker();
            for(started = 0; started < nrOfWorkers; started++){
                Thread t = new Thread(worker);
                t.setDaemon(true);
                workers.add(started, t);
                t.start();
            }
        }

        return started;
    }

    /**
     * Joins all administered worker threads. The function will return only
     * after all threads finished execution, so it should be ensured (from a
     * JNI environment) that all threads will unblock.
     */
    private void joinWorkers(){
        /* Join all current threads */
        try{
            for(Thread t : workers){
                t.join();
            }
        } catch (InterruptedException ignore){
            /* Ignore interruption */
        }
        workers.clear();
    }


    /**
     * If 1, use CDR-copy */
    private long CDRCopy = 0;
    private org.omg.CORBA.ORB orb = null;
    private java.lang.reflect.Constructor<java.io.InputStream> CDRInputStreamConstructor;
    private java.lang.reflect.Method CDRHelperRead;

    private synchronized boolean CDRCopySetupHelper() {
        if(orb == null) {
          orb = org.omg.CORBA.ORB.init();
        }
        try {
            if(CDRHelperRead == null) {
                String name = this.getClass().getName();
                if(!name.endsWith("DataReaderImpl")) {
                    System.out.println("CDRDeserializeByteBuffer unexpected class name: " + name);
                    return false;
                }
                String helperName = name.substring(0, name.length()-14).concat("Helper");
                Class helperClass = Class.forName(helperName);
                Class isClass = Class.forName("org.jacorb.orb.CDRInputStream");
                CDRInputStreamConstructor = isClass.getConstructor(new Class[]{byte[].class});
                Class absIsClass = Class.forName("org.omg.CORBA.portable.InputStream");
                CDRHelperRead = helperClass.getMethod("read", absIsClass);
            }
            return true;
        } catch(Exception e) {
            e.printStackTrace(System.out);
            return false;
        }
    }

    private Object CDRDeserializeByteBuffer(java.nio.ByteBuffer buf) throws java.lang.ClassNotFoundException, java.lang.NoSuchMethodException, java.lang.IllegalAccessException, java.lang.reflect.InvocationTargetException, java.lang.InstantiationException {
        byte myBuf[] = new byte[buf.capacity()];
        buf.get(myBuf);
        //System.out.printf("CDR buffer size: %d\n", buf.capacity());
        java.io.InputStream in = CDRInputStreamConstructor.newInstance(myBuf);
        Object obj = CDRHelperRead.invoke(null, in);
        try {
            in.close();
        } catch(java.io.IOException e) {
            System.out.println("CDRDeserializeByteBuffer: IOException when closing stream");
            obj = null;
        }
        return obj;
    }

    /* see DDS.DataReaderOperations for javadoc */
    public DDS.ReadCondition create_readcondition (int sample_states, int view_states, int instance_states) {
        return jniCreateReadcondition(sample_states, view_states, instance_states);
    }

    /* see DDS.DataReaderOperations for javadoc */
    public DDS.QueryCondition create_querycondition (int sample_states, int view_states, int instance_states, String query_expression, String[] query_parameters) {
        return jniCreateQuerycondition(sample_states, view_states, instance_states, query_expression, query_parameters);
    }

    /* see DDS.DataReaderOperations for javadoc */
    public int delete_readcondition (DDS.ReadCondition a_condition) {
        return jniDeleteReadcondition(a_condition);
    }

    /* see DDS.DataReaderOperations for javadoc */
    public int delete_contained_entities () {
        return jniDeleteContainedEntities();
    }

    /* see DDS.DataReaderOperations for javadoc */
    public int set_qos (DDS.DataReaderQos qos) {
        return jniSetQos(qos);
    }

    /* see DDS.DataReaderOperations for javadoc */
    public int get_qos (DDS.DataReaderQosHolder qos) {
        return jniGetQos(qos);
    }

    /* see DDS.DataReaderOperations for javadoc */
    public int set_listener (DDS.DataReaderListener a_listener, int mask) {
        return jniSetListener(a_listener, mask);
    }

    /* see DDS.DataReaderOperations for javadoc */
    public DDS.DataReaderListener get_listener () {
        return jniGetListener();
    }

    /* see DDS.DataReaderOperations for javadoc */
    public DDS.TopicDescription get_topicdescription () {
        return jniGetTopicdescription();
    }

    /* see DDS.DataReaderOperations for javadoc */
    public DDS.Subscriber get_subscriber () {
        return jniGetSubscriber();
    }

    /* see DDS.DataReaderOperations for javadoc */
    public int get_sample_rejected_status (DDS.SampleRejectedStatusHolder status) {
        return jniGetSampleRejectedStatus(status);
    }

    /* see DDS.DataReaderOperations for javadoc */
    public int get_liveliness_changed_status (DDS.LivelinessChangedStatusHolder status) {
        return jniGetLivelinessChangedStatus(status);
    }

    /* see DDS.DataReaderOperations for javadoc */
    public int get_requested_deadline_missed_status (DDS.RequestedDeadlineMissedStatusHolder status) {
        return jniGetRequestedDeadlineMissedStatus(status);
    }

    /* see DDS.DataReaderOperations for javadoc */
    public int get_requested_incompatible_qos_status (DDS.RequestedIncompatibleQosStatusHolder status) {
        return jniGetRequestedIncompatibleQosStatus(status);
    }

    /* see DDS.DataReaderOperations for javadoc */
    public int get_subscription_matched_status (DDS.SubscriptionMatchedStatusHolder status) {
        return jniGetSubscriptionMatchedStatus(status);
    }

    /* see DDS.DataReaderOperations for javadoc */
    public int get_sample_lost_status (DDS.SampleLostStatusHolder status) {
        return jniGetSampleLostStatus(status);
    }

    /* see DDS.DataReaderOperations for javadoc */
    public int wait_for_historical_data (DDS.Duration_t max_wait) {
        return jniWaitForHistoricalData(max_wait);
    }

    public int wait_for_historical_data_w_condition(String filter_expression, String[] expression_parameters, DDS.Time_t min_source_timestamp, DDS.Time_t max_source_timestamp, DDS.ResourceLimitsQosPolicy resource_limits, DDS.Duration_t max_wait){
        return jniWaitForHistoricalDataWCondition(filter_expression, expression_parameters, min_source_timestamp, max_source_timestamp, resource_limits, max_wait);
    }

    /* see DDS.DataReaderOperations for javadoc */
    public int get_matched_publications (DDS.InstanceHandleSeqHolder publication_handles) {
        return jniGetMatchedPublications(publication_handles);
    }

    /* see DDS.DataReaderOperations for javadoc */
    public int get_matched_publication_data (DDS.PublicationBuiltinTopicDataHolder publication_data, long publication_handle) {
        return jniGetMatchedPublicationData(publication_data, publication_handle);
    }

    public DDS.DataReaderView create_view(DataReaderViewQos qos){
        return jniCreateView(qos);
    }
    public int delete_view(DataReaderView a_view){
        return jniDeleteView(a_view);
    }

    public int get_default_datareaderview_qos(DataReaderViewQosHolder qos){
        return jniGetDefaultDataReaderViewQos(qos);
    }

    public int set_default_datareaderview_qos(DataReaderViewQos qos){
        return jniSetDefaultDataReaderViewQos(qos);
    }

    /* see DDS.PropertyInterfaceOperations for javadoc */
    public int set_property(Property a_property){
        if (a_property.name != null && a_property.value != null){
            if(a_property.name.equals("CDRCopy") && a_property.value.equals("true")) {
                if(!CDRCopySetupHelper()) {
                    return DDS.RETCODE_ERROR.value;
                }
            }
        }
        return jniSetProperty(a_property);
    }

    /* see DDS.PropertyInterfaceOperations for javadoc */
    public int get_property(PropertyHolder a_property){
        return jniGetProperty(a_property);
    }

    private native DDS.ReadCondition jniCreateReadcondition(int sample_states, int view_states, int instance_states);
    private native DDS.QueryCondition jniCreateQuerycondition(int sample_states, int view_states, int instance_states, String query_expression, String[] query_parameters);
    private native int jniDeleteReadcondition(DDS.ReadCondition a_condition);
    private native int jniDeleteContainedEntities();
    private native int jniSetQos(DDS.DataReaderQos qos);
    private native int jniGetQos(DDS.DataReaderQosHolder qos);
    private native int jniSetListener(DDS.DataReaderListener a_listener, int mask);
    private native DDS.DataReaderListener jniGetListener();
    private native DDS.TopicDescription jniGetTopicdescription();
    private native DDS.Subscriber jniGetSubscriber();
    private native int jniGetSampleRejectedStatus(DDS.SampleRejectedStatusHolder status);
    private native int jniGetLivelinessChangedStatus(DDS.LivelinessChangedStatusHolder status);
    private native int jniGetRequestedDeadlineMissedStatus(DDS.RequestedDeadlineMissedStatusHolder status);
    private native int jniGetRequestedIncompatibleQosStatus(DDS.RequestedIncompatibleQosStatusHolder status);
    private native int jniGetSubscriptionMatchedStatus(DDS.SubscriptionMatchedStatusHolder status);
    private native int jniGetSampleLostStatus(DDS.SampleLostStatusHolder status);
    private native int jniWaitForHistoricalData(DDS.Duration_t max_wait);
    private native int jniWaitForHistoricalDataWCondition(String filter_expression, String[] expression_parameters, DDS.Time_t min_source_timestamp, DDS.Time_t max_source_timestamp, DDS.ResourceLimitsQosPolicy resource_limits, DDS.Duration_t max_wait);
    private native int jniGetMatchedPublications(DDS.InstanceHandleSeqHolder publication_handles);
    private native int jniGetMatchedPublicationData(DDS.PublicationBuiltinTopicDataHolder publication_data, long publication_handle);
    private native DDS.DataReaderView jniCreateView(DDS.DataReaderViewQos qos);
    private native int jniDeleteView(DataReaderView a_view);
    private native int jniGetDefaultDataReaderViewQos(DataReaderViewQosHolder qos);
    private native int jniSetDefaultDataReaderViewQos(DataReaderViewQos qos);
    private native int jniParallelDemarshallingMain(long parDemCtx);
    private native int jniSetProperty(Property a_property);
    private native int jniGetProperty(PropertyHolder a_property);
}

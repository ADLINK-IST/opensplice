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


package org.opensplice.dds.dcps;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;

import DDS.DataReaderView;
import DDS.DataReaderViewQos;
import DDS.DataReaderViewQosHolder;
import DDS.Property;
import DDS.PropertyHolder;

/**
 * Implementation of the {@link DDS.DataReader} interface.
 */
public class DataReaderImpl extends DataReaderBase implements DDS.DataReader {
    private static final long serialVersionUID = 7369562459575330228L;

    private final ArrayList<Thread> workers = new ArrayList<Thread>();

    private DDS.DataReaderViewQos defaultDataReaderViewQos = Utilities.defaultDataReaderViewQos;

    /**
     * The address of the parallel demarshalling administration for this reader.
     * This field will be set from a JNI context when parallel demarshalling is
     * enabled by means of set_property(...) on the reader. */
    private long parallelDemarshallingContext = 0;

    private String name;
    private SubscriberImpl subscriber = null;
    private DDS.TopicDescription description = null;
    private DDS.DataReaderListener listener = null;

    private final Set<DataReaderViewImpl> views = new HashSet<DataReaderViewImpl>();
    private final Set<ReadConditionImpl> conditions = new HashSet<ReadConditionImpl>();

    protected DataReaderImpl() { }

    protected int init (
        SubscriberImpl subscriber,
        String name,
        DDS.TopicDescription description,
        DDS.DataReaderQos qos)
    {
        int result = DDS.RETCODE_OK.value;
        String expression = null;
        DDS.StringSeqHolder parameters = new DDS.StringSeqHolder();

        /* Parameters validated by DDS.Subscriber.create_datareader */

        if (description instanceof TopicImpl) {
            expression = new String(
                "select * from " +
                ((TopicImpl)(description)).get_name());
        } else if (description instanceof ContentFilteredTopicImpl) {
            expression = new String(
                "select * from " +
                ((ContentFilteredTopicImpl)description).get_related_topic().get_name() +
                " where " +
                ((ContentFilteredTopicImpl)description).get_filter_expression());
            ((ContentFilteredTopicImpl)description).get_expression_parameters(parameters);
        } else if (description instanceof MultiTopicImpl) {
            result = DDS.RETCODE_UNSUPPORTED.value;
        } else {
            result = DDS.RETCODE_BAD_PARAMETER.value;
        }

        if (result == DDS.RETCODE_OK.value) {
            long uSubscriber = subscriber.get_user_object();
            if (uSubscriber != 0) {
                /* QoS is guaranteed to be valid by the creating subscriber.
                   DATAREADER_QOS_DEFAULT and DATAREADER_QOS_USE_TOPIC_QOS are
                   properly resolved. */
                this.subscriber = subscriber;
                this.name = name;
                this.description = description;
                long uReader = jniDataReaderNew(uSubscriber, name, expression, parameters.value, qos);

                if (uReader != 0) {
                    ((TopicDescription)description).keep();
                    this.set_user_object(uReader);
                    this.setDomainId(subscriber.getDomainId());
                } else {
                    this.subscriber = null;
                    this.name = null;
                    this.description = null;
                    result = DDS.RETCODE_ERROR.value;
                }
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        return result;
    }

    @Override
    protected int deinit ()
    {
        int result = DDS.RETCODE_OK.value;
        long uReader = 0;

        synchronized (this)
        {
            uReader = this.get_user_object();
            if (uReader != 0) {
                /* currently the datareader ignores outstanding loans.
                 * If it is checked it should also check the set property
                 * ignoreLoansOnDeletion if it should return
                 * RETCODE_PRECONDITION_NOT_MET or not.
                 */
                if (this.conditions.size() != 0) {
                    result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
                    ReportStack.report(result,
                        "DataReader still contains '" + this.conditions.size()
                        + "' Condition entities.");
                } else if (this.views.size() != 0) {
                    result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
                    ReportStack.report(result,
                        "DataReader still contains '" + this.views.size()
                        + "' DataReaderView entities.");
                } else {
                    if (this.listener != null) {
                        /* Should always be done before disable_callbacks(), so
                         * that events that can trigger a listener on an owning
                         * entity are propagated instead of being consumed by
                         * the listener being destroyed. */
                        result = set_listener(this.listener, 0);
                    }
                    if (result == DDS.RETCODE_OK.value) {
                        result = this.disable_callbacks();
                        if (result == DDS.RETCODE_OK.value) {
                            result = ((EntityImpl) this).detach_statuscondition();
                            if (result == DDS.RETCODE_OK.value) {
                                ((TopicDescription) this.description).free();
                                this.name = null;
                                this.subscriber = null;
                                this.description = null;
                                result = jniDataReaderFree(uReader);
                                if (result == DDS.RETCODE_OK.value) {
                                    result = super.deinit();
                                }
                            }
                        }
                    }
                }
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        return result;
    }

    /**
     * Starts the given number of worker threads to be used in JNI context.
     *
     * WARNING: This call should only be invoked from JNI.
     * @pre There are currently no worker threads started.
     * @param nrOfWorkers the number of worker-threads to start
     * @return the number of actually started threads
     */
    private int startWorkers(int nrOfWorkers){
        int started = 0;
        class Worker implements Runnable {
            @Override
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
     *
     * WARNING: This call should only be invoked from JNI.
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
    private final long CDRCopy = 0;
    private org.omg.CORBA.ORB orb = null;
    private java.lang.reflect.Constructor<java.io.InputStream> CDRInputStreamConstructor;
    private java.lang.reflect.Method CDRHelperRead;

    /**
     * Resolves the read method for deserializing the CDR-blob. This
     * initialization is performed from within a JNI context.
     */
    @SuppressWarnings({ "rawtypes", "unchecked" })
    private boolean CDRCopySetupHelper() {
        if(orb == null) {
            orb = org.omg.CORBA.ORB.init();
        }
        try {
            if(CDRHelperRead == null) {
                String name = this.getClass().getName();
                if(!name.endsWith("DataReaderImpl")) {
                    ReportStack.report(DDS.RETCODE_ERROR.value,
                        "CDRDeserializeByteBuffer unexpected class name: " + name);
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

    private Object CDRDeserializeByteBuffer(java.nio.ByteBuffer buf)
        throws java.lang.ClassNotFoundException,
               java.lang.NoSuchMethodException,
               java.lang.IllegalAccessException,
               java.lang.reflect.InvocationTargetException,
               java.lang.InstantiationException
    {
        byte myBuf[] = new byte[buf.capacity()];
        buf.get(myBuf);

        java.io.InputStream in = CDRInputStreamConstructor.newInstance(myBuf);
        Object obj = CDRHelperRead.invoke(null, in);
        try {
            in.close();
        } catch(java.io.IOException e) {
            ReportStack.report(DDS.RETCODE_ERROR.value,
                "CDRDeserializeByteBuffer: IOException when closing stream.");
            obj = null;
        }
        return obj;
    }

    /* see DDS.DataReaderOperations for javadoc */
    @Override
    public DDS.ReadCondition create_readcondition (
        int sample_states,
        int view_states,
        int instance_states)
    {
        int result = DDS.RETCODE_OK.value;
        ReadConditionImpl readCondition = null;
        ReportStack.start();

        if ((result = Utilities.checkSampleStateMask(sample_states))
                 == DDS.RETCODE_OK.value &&
            (result = Utilities.checkViewStateMask(view_states))
                 == DDS.RETCODE_OK.value &&
            (result = Utilities.checkInstanceStateMask(instance_states))
                 == DDS.RETCODE_OK.value)
        {
            readCondition = new ReadConditionImpl();
            synchronized(this)
            {
                result = readCondition.init(
                    this, sample_states, view_states, instance_states, 0);
                if (result == DDS.RETCODE_OK.value) {
                    this.conditions.add(readCondition);
                } else {
                    readCondition = null;
                }
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return readCondition;
    }

    /* see DDS.DataReaderOperations for javadoc */
    @Override
    public DDS.QueryCondition create_querycondition (
        int sample_states,
        int view_states,
        int instance_states,
        String query_expression,
        String[] query_parameters)
    {
        QueryConditionImpl queryCondition = null;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if ((result = Utilities.checkSampleStateMask(sample_states))
                == DDS.RETCODE_OK.value &&
           (result = Utilities.checkViewStateMask(view_states))
                == DDS.RETCODE_OK.value &&
           (result = Utilities.checkInstanceStateMask(instance_states))
                == DDS.RETCODE_OK.value)
        {
            if (query_expression == null) {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                ReportStack.report(result,
                        "query_expression 'null' is invalid.");
            } else {
                queryCondition = new QueryConditionImpl();
                synchronized(this)
                {
                    result = queryCondition.init(this, sample_states, view_states,
                            instance_states, query_expression, query_parameters);

                    if (result == DDS.RETCODE_OK.value) {
                        this.conditions.add(queryCondition);
                    } else {
                        queryCondition = null;
                    }
                }
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return queryCondition;
    }

    /* see DDS.DataReaderOperations for javadoc */
    @Override
    public int delete_readcondition (
        DDS.ReadCondition a_condition)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start ();

        if (this.get_user_object() != 0) {
            if (a_condition != null) {
                synchronized(this)
                {
                    ReadConditionImpl rc = (ReadConditionImpl)a_condition;
                    if (this.conditions.remove(rc)) {
                        result = rc.deinit();
                        if (result != DDS.RETCODE_OK.value) {
                            this.conditions.add(rc);
                        }
                    } else {
                        if (rc.get_user_object() == 0) {
                            result = DDS.RETCODE_ALREADY_DELETED.value;
                        } else {
                            result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
                            ReportStack.report (result, "Condition not created by DataReader.");
                        }
                    }
                }
            } else {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                ReportStack.report (result, "a_condition 'null' is invalid.");
            }
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }

        ReportStack.flush (this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.DataReaderOperations for javadoc */
    @Override
    public int delete_contained_entities ()
    {
        int result, endResult = DDS.RETCODE_OK.value;
        ReportStack.start();

        synchronized(this)
        {
            if (this.get_user_object() != 0) {
                Iterator<ReadConditionImpl> it = this.conditions.iterator();
                while (it.hasNext()) {
                    result = it.next().deinit();
                    if (result == DDS.RETCODE_OK.value || result == DDS.RETCODE_ALREADY_DELETED.value) {
                        /* Use iterator.remove() to remove from the set */
                        it.remove();
                    }
                    if(result != DDS.RETCODE_OK.value){
                        ReportStack.report (result, "Deletion of ReadCondition contained in DataReader failed.");
                        if(endResult == DDS.RETCODE_OK.value) {
                            /* Store first encountered error. */
                            endResult = result;
                        }
                    }
                }

                Iterator<DataReaderViewImpl> iv = this.views.iterator();
                while (iv.hasNext()) {
                    DataReaderViewImpl view = iv.next();
                    result = view.delete_contained_entities();
                    if (result == DDS.RETCODE_OK.value) {
                        result = view.deinit();
                        if (result == DDS.RETCODE_OK.value || result == DDS.RETCODE_ALREADY_DELETED.value) {
                            /* Use iterator.remove() to remove from the set */
                            iv.remove();
                        }
                        if (result != DDS.RETCODE_OK.value) {
                            ReportStack.report (result, "Deletion of DataReaderView contained in DataReader failed.");
                        }
                    } else {
                        if (result == DDS.RETCODE_ALREADY_DELETED.value) {
                            /* Use iterator.remove() to remove from the set */
                            iv.remove();
                        }
                        ReportStack.report (result, "delete_contained_entities failed on DataReaderView contained in DataReader.");
                    }

                    if (endResult == DDS.RETCODE_OK.value ) {
                        /* Store first encountered error. */
                        endResult = result;
                    }
                }
            } else {
                endResult = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, endResult != DDS.RETCODE_OK.value);
        return endResult;
    }

    /* see DDS.DataReaderOperations for javadoc */
    @Override
    public int set_qos (
        DDS.DataReaderQos qos)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (qos == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "qos 'null' is invalid.");
        } else if (qos == DDS.DATAREADER_QOS_DEFAULT.value) {
            DDS.DataReaderQosHolder holder = new DDS.DataReaderQosHolder();
            result = this.subscriber.get_default_datareader_qos(holder);
            qos = holder.value;
        } else if (qos == DDS.DATAREADER_QOS_USE_TOPIC_QOS.value) {
            DDS.DataReaderQosHolder holder = new DDS.DataReaderQosHolder();
            result = this.subscriber.copy_from_topic_qos(holder, DDS.TOPIC_QOS_DEFAULT.value);
            if (result == DDS.RETCODE_OK.value) {
                qos = holder.value;
                result = Utilities.checkQos (qos);
            }
        } else {
            result = Utilities.checkQos (qos);
        }

        if (result == DDS.RETCODE_OK.value) {
            long uReader = this.get_user_object();
            if (uReader != 0) {
                result = jniSetQos(uReader, qos);
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.DataReaderOperations for javadoc */
    @Override
    public int get_qos (
        DDS.DataReaderQosHolder qos)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (qos == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "qos 'null' is invalid.");
        } else {
            long uReader = this.get_user_object();
            if (uReader != 0) {
                result = jniGetQos(uReader, qos);
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.DataReaderOperations for javadoc */
    @Override
    public int set_listener (
        DDS.DataReaderListener a_listener, int mask)
    {
        long uReader = 0;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        uReader = this.get_user_object();
        if (uReader != 0) {
            this.listener = a_listener;
            result = this.set_listener_interest(mask);
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.DataReaderOperations for javadoc */
    @Override
    public DDS.DataReaderListener get_listener ()
    {
        return listener;
    }

    /* see DDS.DataReaderOperations for javadoc */
    @Override
    public DDS.TopicDescription get_topicdescription ()
    {
        return this.description;
    }

    /* see DDS.DataReaderOperations for javadoc */
    @Override
    public DDS.Subscriber get_subscriber ()
    {
        return this.subscriber;
    }

    /* see DDS.DataReaderOperations for javadoc */
    @Override
    public int get_sample_rejected_status (
        DDS.SampleRejectedStatusHolder status)
    {
        int result = DDS.RETCODE_OK.value;
        long uReader;
        ReportStack.start();

        if (status == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "status 'null' is invalid.");
        } else {
            uReader = this.get_user_object();
            if (uReader != 0) {
                result = jniGetSampleRejectedStatus(uReader, status);
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.DataReaderOperations for javadoc */
    @Override
    public int get_liveliness_changed_status (
        DDS.LivelinessChangedStatusHolder status)
    {
        int result = DDS.RETCODE_OK.value;
        long uReader;
        ReportStack.start();

        if (status == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "status 'null' is invalid.");
        } else {
            uReader = this.get_user_object();
            if (uReader != 0) {
                result = jniGetLivelinessChangedStatus(uReader, status);
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.DataReaderOperations for javadoc */
    @Override
    public int get_requested_deadline_missed_status (
        DDS.RequestedDeadlineMissedStatusHolder status)
    {
        int result = DDS.RETCODE_OK.value;
        long uReader;
        ReportStack.start();

        if (status == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "status 'null' is invalid.");
        } else {
            uReader = this.get_user_object();
            if (uReader != 0) {
                result = jniGetRequestedDeadlineMissedStatus(uReader, status);
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.DataReaderOperations for javadoc */
    @Override
    public int get_requested_incompatible_qos_status (
        DDS.RequestedIncompatibleQosStatusHolder status)
    {
        int result = DDS.RETCODE_OK.value;
        long uReader;
        ReportStack.start();

        if (status == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "status 'null' is invalid.");
        } else {
            uReader = this.get_user_object();
            if (uReader != 0) {
                result = jniGetRequestedIncompatibleQosStatus(uReader, status);
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.DataReaderOperations for javadoc */
    @Override
    public int get_subscription_matched_status (
        DDS.SubscriptionMatchedStatusHolder status)
    {
        int result = DDS.RETCODE_OK.value;
        long uReader;
        ReportStack.start();

        if (status == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "status 'null' is invalid.");
        } else {
            uReader = this.get_user_object();
            if (uReader != 0) {
                result = jniGetSubscriptionMatchedStatus(uReader, status);
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.DataReaderOperations for javadoc */
    @Override
    public int get_sample_lost_status (
        DDS.SampleLostStatusHolder status)
    {
        int result = DDS.RETCODE_OK.value;
        long uReader;
        ReportStack.start();

        if (status == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "status 'null' is invalid.");
        } else {
            uReader = this.get_user_object();
            if (uReader != 0) {
                result = jniGetSampleLostStatus(uReader, status);
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.DataReaderOperations for javadoc */
    @Override
    public int wait_for_historical_data (
        DDS.Duration_t max_wait)
    {
        int result = DDS.RETCODE_OK.value;
        long uReader;
        ReportStack.start();

        if ((result = Utilities.checkDuration (max_wait))
                 == DDS.RETCODE_OK.value)
        {
            uReader = this.get_user_object();
            if (uReader != 0) {
                result = jniWaitForHistoricalData(uReader, max_wait);
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value &&
                                result != DDS.RETCODE_TIMEOUT.value);
        return result;
    }

    @Override
    public int wait_for_historical_data_w_condition(
        String filter_expression,
        String[] expression_parameters,
        DDS.Time_t min_source_timestamp,
        DDS.Time_t max_source_timestamp,
        DDS.ResourceLimitsQosPolicy resource_limits,
        DDS.Duration_t max_wait)
    {
        int result = DDS.RETCODE_OK.value;
        long uReader;
        ReportStack.start();

        if ((result = Utilities.checkTime(min_source_timestamp))
                 == DDS.RETCODE_OK.value &&
            (result = Utilities.checkTime(max_source_timestamp))
                 == DDS.RETCODE_OK.value &&
            (result = Utilities.checkPolicy(resource_limits))
                 == DDS.RETCODE_OK.value &&
            (result = Utilities.checkDuration(max_wait))
                 == DDS.RETCODE_OK.value)
        {
            uReader = this.get_user_object();
            if (uReader != 0) {
                result = jniWaitForHistoricalDataWCondition(
                    uReader, filter_expression, expression_parameters,
                    min_source_timestamp, max_source_timestamp,
                    resource_limits, max_wait);
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value &&
                                result != DDS.RETCODE_TIMEOUT.value);
        return result;
    }

    /* see DDS.DataReaderOperations for javadoc */
    @Override
    public int get_matched_publications (
        DDS.InstanceHandleSeqHolder publication_handles)
    {
        int result = DDS.RETCODE_BAD_PARAMETER.value;
        long uReader;
        ReportStack.start();

        if (publication_handles == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "publication_handles 'null' is invalid.");
        } else {
            uReader = this.get_user_object();
            if (uReader != 0) {
                result = jniGetMatchedPublications(uReader, publication_handles);
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.DataReaderOperations for javadoc */
    @Override
    public int get_matched_publication_data (
        DDS.PublicationBuiltinTopicDataHolder publication_data,
        long publication_handle)
    {
        int result = DDS.RETCODE_OK.value;
        long uReader;
        ReportStack.start();

        if (publication_data == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "publication_data 'null' is invalid.");
        } else {
            uReader = this.get_user_object();
            if (uReader != 0) {
                result = jniGetMatchedPublicationData(uReader, publication_data, publication_handle);
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    @Override
    public DDS.DataReaderView create_view(
        DataReaderViewQos qos)
    {
        DataReaderViewImpl view = null;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (qos == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "qos 'null' is invalid.");
        } else if (qos == DDS.DATAREADERVIEW_QOS_DEFAULT.value) {
            qos = this.defaultDataReaderViewQos;
        } else {
            result = Utilities.checkQos(qos);
        }

        if (result == DDS.RETCODE_OK.value) {
            view = (DataReaderViewImpl)((org.opensplice.dds.dcps.TopicDescription)description).create_dataview();
            result = view.init(this, this.name + "_view", qos);
            if (result == DDS.RETCODE_OK.value) {
                synchronized (this)
                {
                    this.views.add(view);
                }
            } else {
                view = null;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return view;
    }

    @Override
    public int delete_view(
        DataReaderView view)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (view == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "view 'null' is invalid.");
        } else {
            synchronized (this)
            {
                DataReaderViewImpl drv = (DataReaderViewImpl)view;
                if (this.views.remove(drv)) {
                    result = drv.deinit();
                    if (result != DDS.RETCODE_OK.value) {
                        this.views.add(drv);
                    }
                } else {
                    result = DDS.RETCODE_BAD_PARAMETER.value;
                    ReportStack.report(result, "DataReaderView not created by DataReader.");
                }
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    @Override
    public int get_default_datareaderview_qos(
        DataReaderViewQosHolder qos)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (qos == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "qos 'null' is invalid.");
        } else {
            qos.value = Utilities.deepCopy(this.defaultDataReaderViewQos);
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    @Override
    public int set_default_datareaderview_qos(
        DataReaderViewQos qos)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (qos == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "qos 'null' is invalid.");
        } else if (qos == DDS.DATAREADERVIEW_QOS_DEFAULT.value) {
            qos = Utilities.defaultDataReaderViewQos;
        } else {
            result = Utilities.checkQos(qos);
        }

        if (result == DDS.RETCODE_OK.value) {
            qos = Utilities.deepCopy(qos);
            if (this.get_user_object() != 0) {
                this.defaultDataReaderViewQos = qos;
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.PropertyInterfaceOperations for javadoc */
    @Override
    public int set_property(Property a_property)
    {
        int result = DDS.RETCODE_OK.value;
        long uReader;
        ReportStack.start();

        result = checkProperty(a_property);

        if (result == DDS.RETCODE_OK.value) {
            uReader = this.get_user_object();
            if (uReader != 0) { /* Should be an assert... */
                /* Pass non pure-Java properties down the API */
                result = jniSetProperty(uReader, a_property);
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.PropertyInterfaceOperations for javadoc */
    @Override
    public int get_property(PropertyHolder a_property)
    {
        int result = DDS.RETCODE_OK.value;
        long uReader;
        ReportStack.start();

        uReader = this.get_user_object();
        if (uReader != 0) {
            result = jniGetProperty(uReader, a_property);
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    protected int read_instance_handles(DDS.InstanceHandleSeqHolder handles)
    {
        int result = DDS.RETCODE_OK.value;
        long uReader;
        ReportStack.start();

        uReader = this.get_user_object();
        if (uReader != 0) {
            result = jniReadInstanceHandles(uReader, handles);
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    @Override
    protected int notify(Event e)
    {
        int result = DDS.RETCODE_OK.value;

        DDS.DataReaderListener drl = this.listener;
        if (drl == null)
            return result;

        switch (e.kind) {
            case DDS.DATA_AVAILABLE_STATUS.value:
                drl.on_data_available(this);
                break;
            case DDS.REQUESTED_DEADLINE_MISSED_STATUS.value:
                drl.on_requested_deadline_missed(this, (DDS.RequestedDeadlineMissedStatus) e.status);
                break;
            case DDS.REQUESTED_INCOMPATIBLE_QOS_STATUS.value:
                drl.on_requested_incompatible_qos(this, (DDS.RequestedIncompatibleQosStatus) e.status);
                break;
            case DDS.SAMPLE_REJECTED_STATUS.value:
                drl.on_sample_rejected(this, (DDS.SampleRejectedStatus) e.status);
                break;
            case DDS.LIVELINESS_CHANGED_STATUS.value:
                drl.on_liveliness_changed(this, (DDS.LivelinessChangedStatus) e.status);
                break;
            case DDS.SAMPLE_LOST_STATUS.value:
                drl.on_sample_lost(this, (DDS.SampleLostStatus) e.status);
                break;
            case DDS.SUBSCRIPTION_MATCHED_STATUS.value:
                drl.on_subscription_matched(this, (DDS.SubscriptionMatchedStatus) e.status);
                break;
            default:
                // TODO [jeroenk]: Should result be updated?
                ReportStack.report(
                    DDS.RETCODE_UNSUPPORTED.value,
                    "Received unsupported event kind '" + e.kind + "'.");
                break;
        }

        return result;
    }

    private native long jniDataReaderNew(
                            long uSubscriber,
                            String name,
                            String expression,
                            String[] parameters,
                            DDS.DataReaderQos qos);

    private native int jniDataReaderFree(
                            long uReader);

    private native int jniSetQos(
                            long uReader,
                            DDS.DataReaderQos qos);

    private native int jniGetQos(
                            long uReader,
                            DDS.DataReaderQosHolder qos);

    private native int jniGetSampleRejectedStatus(
                            long uReader,
                            DDS.SampleRejectedStatusHolder status);

    private native int jniGetLivelinessChangedStatus(
                            long uReader,
                            DDS.LivelinessChangedStatusHolder status);

    private native int jniGetRequestedDeadlineMissedStatus(
                            long uReader,
                            DDS.RequestedDeadlineMissedStatusHolder status);

    private native int jniGetRequestedIncompatibleQosStatus(
                            long uReader,
                            DDS.RequestedIncompatibleQosStatusHolder status);

    private native int jniGetSubscriptionMatchedStatus(
                            long uReader,
                            DDS.SubscriptionMatchedStatusHolder status);

    private native int jniGetSampleLostStatus(
                            long uReader,
                            DDS.SampleLostStatusHolder status);

    private native int jniWaitForHistoricalData(
                            long uReader,
                            DDS.Duration_t max_wait);

    private native int jniWaitForHistoricalDataWCondition(
                            long uReader,
                            String filter_expression,
                            String[] expression_parameters,
                            DDS.Time_t min_source_timestamp,
                            DDS.Time_t max_source_timestamp,
                            DDS.ResourceLimitsQosPolicy resource_limits,
                            DDS.Duration_t max_wait);

    private native int jniGetMatchedPublications(
                            long uReader,
                            DDS.InstanceHandleSeqHolder publication_handles);

    private native int jniGetMatchedPublicationData(
                            long uReader,
                            DDS.PublicationBuiltinTopicDataHolder publication_data,
                            long publication_handle);

    private native int jniParallelDemarshallingMain(
                            long parDemCtx);

    private native int jniSetProperty(
                            long uReader,
                            Property a_property);

    private native int jniGetProperty(
                            long uReader,
                            PropertyHolder a_property);

    private native int jniReadInstanceHandles(
                            long uReader,
                            DDS.InstanceHandleSeqHolder handles);
}

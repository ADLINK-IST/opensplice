public class $(type-name)DataReaderImpl extends org.opensplice.dds.dcps.DataReaderImpl implements $(type-name)DataReader
{
    private long copyCache;
    private $(type-name)TypeSupport typeSupport;

    public $(type-name)DataReaderImpl($(scoped-type-name)TypeSupport ts)
    {
        typeSupport = ts;
        copyCache = typeSupport.get_copyCache ();
    }

    public int read(
            $(scoped-type-name)SeqHolder received_data, 
            DDS.SampleInfoSeqHolder info_seq, 
            int max_samples, 
            int sample_states, 
            int view_states, 
            int instance_states)
    {
        return
            org.opensplice.dds.dcps.FooDataReaderImpl.read(
                    this,
                    copyCache,
                    received_data,
                    info_seq, 
                    max_samples, 
                    sample_states, 
                    view_states, 
                    instance_states);
    }

    public int take(
            $(scoped-type-name)SeqHolder received_data, 
            DDS.SampleInfoSeqHolder info_seq, 
            int max_samples, 
            int sample_states, 
            int view_states, 
            int instance_states)
    {
        return
            org.opensplice.dds.dcps.FooDataReaderImpl.take(
                    this,
                    copyCache,
                    received_data, 
                    info_seq, 
                    max_samples, 
                    sample_states, 
                    view_states, 
                    instance_states);
    }

    public int read_w_condition(
            $(scoped-type-name)SeqHolder received_data, 
            DDS.SampleInfoSeqHolder info_seq, 
            int max_samples, 
            DDS.ReadCondition a_condition)
    {
        return
            org.opensplice.dds.dcps.FooDataReaderImpl.readWCondition(
                    this,
                    copyCache,
                    received_data, 
                    info_seq, 
                    max_samples, 
                    a_condition);
    }

    public int take_w_condition(
            $(scoped-type-name)SeqHolder received_data, 
            DDS.SampleInfoSeqHolder info_seq, 
            int max_samples, 
            DDS.ReadCondition a_condition)
    {
        return
            org.opensplice.dds.dcps.FooDataReaderImpl.takeWCondition(
                    this,
                    copyCache,
                    received_data, 
                    info_seq, 
                    max_samples, 
                    a_condition);
    }

    public int read_next_sample(
            $(scoped-actual-type-name)Holder received_data, 
            DDS.SampleInfoHolder sample_info)
    {
        return
            org.opensplice.dds.dcps.FooDataReaderImpl.readNextSample (
                    this,
                    copyCache,
                    received_data, 
                    sample_info);
    }

    public int take_next_sample(
            $(scoped-actual-type-name)Holder received_data, 
            DDS.SampleInfoHolder sample_info)
    {
        return
            org.opensplice.dds.dcps.FooDataReaderImpl.takeNextSample(
                    this,
                    copyCache,
                    received_data, 
                    sample_info);
    }

    public int read_instance(
            $(scoped-type-name)SeqHolder received_data, 
            DDS.SampleInfoSeqHolder info_seq, 
            int max_samples, 
            long a_handle, 
            int sample_states, 
            int view_states, 
            int instance_states)
    {
        return
            org.opensplice.dds.dcps.FooDataReaderImpl.readInstance (
                    this,
                    copyCache,
                    received_data, 
                    info_seq, 
                    max_samples,
                    a_handle, 
                    sample_states, 
                    view_states, 
                    instance_states);
    }

    public int take_instance(
            $(scoped-type-name)SeqHolder received_data, 
            DDS.SampleInfoSeqHolder info_seq, 
            int max_samples, 
            long a_handle, 
            int sample_states, 
            int view_states, 
            int instance_states)
    {
        return
            org.opensplice.dds.dcps.FooDataReaderImpl.takeInstance(
                    this,
                    copyCache,
                    received_data, 
                    info_seq, 
                    max_samples,
                    a_handle, 
                    sample_states, 
                    view_states, 
                    instance_states);
    }

    public int read_next_instance(
            $(scoped-type-name)SeqHolder received_data, 
            DDS.SampleInfoSeqHolder info_seq, 
            int max_samples, 
            long a_handle, 
            int sample_states, 
            int view_states, 
            int instance_states)
    {
        return
            org.opensplice.dds.dcps.FooDataReaderImpl.readNextInstance(
                    this,
                    copyCache,
                    received_data, 
                    info_seq, 
                    max_samples,
                    a_handle, 
                    sample_states, 
                    view_states, 
                    instance_states);
    }

    public int take_next_instance(
            $(scoped-type-name)SeqHolder received_data, 
            DDS.SampleInfoSeqHolder info_seq, 
            int max_samples, 
            long a_handle, 
            int sample_states, 
            int view_states, 
            int instance_states)
    {
        return
            org.opensplice.dds.dcps.FooDataReaderImpl.takeNextInstance(
                    this,
                    copyCache,
                    received_data, 
                    info_seq, 
                    max_samples,
                    a_handle, 
                    sample_states, 
                    view_states, 
                    instance_states);
    }

    public int read_next_instance_w_condition(
            $(scoped-type-name)SeqHolder received_data, 
            DDS.SampleInfoSeqHolder info_seq, 
            int max_samples, 
            long a_handle, 
            DDS.ReadCondition a_condition)
    {
        return
            org.opensplice.dds.dcps.FooDataReaderImpl.readNextInstanceWCondition(
                    this,
                    copyCache,
                    received_data, 
                    info_seq, 
                    max_samples,
                    a_handle, 
                    a_condition);
    }

    public int take_next_instance_w_condition(
            $(scoped-type-name)SeqHolder received_data, 
            DDS.SampleInfoSeqHolder info_seq, 
            int max_samples, 
            long a_handle, 
            DDS.ReadCondition a_condition)
    {
        return
            org.opensplice.dds.dcps.FooDataReaderImpl.takeNextInstanceWCondition (
                    this,
                    copyCache,
                    received_data, 
                    info_seq, 
                    max_samples,
                    a_handle, 
                    a_condition);
    }

    public int return_loan(
            $(scoped-type-name)SeqHolder received_data, 
            DDS.SampleInfoSeqHolder info_seq)
    {
        int result;

        if (received_data != null && info_seq != null) {
            if (received_data.value != null && info_seq.value != null) {
                if (received_data.value.length == info_seq.value.length) {
                    received_data.value = null;
                    info_seq.value = null;
                    result = DDS.RETCODE_OK.value;
                } else {
                    result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
                }
            } else {
                if ((received_data.value == null) && (info_seq.value == null)) {
                    result = DDS.RETCODE_OK.value;
                } else {
                    result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
                }
            }
        } else {
            result = DDS.RETCODE_BAD_PARAMETER.value;
        }
        return result;
    }

    public int get_key_value(
            $(scoped-actual-type-name)Holder key_holder, 
            long handle)
    {
        return
            org.opensplice.dds.dcps.FooDataReaderImpl.getKeyValue (
                    this,
                    copyCache,
                    key_holder,
                    handle);
    }
    
    public long lookup_instance(
	$(scoped-actual-type-name) instance)
    {
        return
            org.opensplice.dds.dcps.FooDataReaderImpl.lookupInstance(
                    this,
                    copyCache,
                    instance);
    }

}

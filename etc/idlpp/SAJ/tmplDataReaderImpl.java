public class $(type-name)DataReaderImpl extends org.opensplice.dds.dcps.FooDataReaderImpl implements $(type-name)DataReader
{
    private static final long serialVersionUID = 1L;

    private long copyCache;
    private $(type-name)TypeSupport typeSupport;

    public $(type-name)DataReaderImpl($(scoped-type-name)TypeSupport ts)
    {
        typeSupport = ts;
        copyCache = typeSupport.get_copyCache ();
    }

    @Override
    synchronized public int read(
            $(scoped-type-name)SeqHolder received_data,
            DDS.SampleInfoSeqHolder info_seq,
            int max_samples,
            int sample_states,
            int view_states,
            int instance_states)
    {
        long uReader = 0;
        int result = DDS.RETCODE_OK.value;
        org.opensplice.dds.dcps.ReportStack.start();

        uReader = this.get_user_object();
        if (uReader != 0) {
            if (received_data == null) {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                org.opensplice.dds.dcps.ReportStack.report(
                    result, "received_data 'null' is invalid.");
            } else if (info_seq == null) {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                org.opensplice.dds.dcps.ReportStack.report(
                    result, "info_seq 'null' is invalid.");
            } else if (max_samples < -1) {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                org.opensplice.dds.dcps.ReportStack.report(
                    result, "max_samples is invalid.");
            } else {
                result = org.opensplice.dds.dcps.FooDataReaderImpl.jniRead(
                        this,
                        uReader,
                        copyCache,
                        received_data,
                        info_seq,
                        max_samples,
                        sample_states,
                        view_states,
                        instance_states);
            }
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }

        org.opensplice.dds.dcps.ReportStack.flush(
            this,
            result != DDS.RETCODE_OK.value &&
            result != DDS.RETCODE_NO_DATA.value);
        return result;
    }

    @Override
    synchronized public int take(
            $(scoped-type-name)SeqHolder received_data,
            DDS.SampleInfoSeqHolder info_seq,
            int max_samples,
            int sample_states,
            int view_states,
            int instance_states)
    {
        long uReader = 0;
        int result = DDS.RETCODE_ALREADY_DELETED.value;
        org.opensplice.dds.dcps.ReportStack.start();

        uReader = this.get_user_object();
        if (uReader != 0) {
            if (received_data == null) {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                org.opensplice.dds.dcps.ReportStack.report(
                    result, "received_data 'null' is invalid.");
            } else if (info_seq == null) {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                org.opensplice.dds.dcps.ReportStack.report(
                    result, "info_seq 'null' is invalid.");
            } else if (max_samples < -1) {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                org.opensplice.dds.dcps.ReportStack.report(
                    result, "max_samples is invalid.");
            } else {
                result = org.opensplice.dds.dcps.FooDataReaderImpl.jniTake(
                        this,
                        uReader,
                        copyCache,
                        received_data,
                        info_seq,
                        max_samples,
                        sample_states,
                        view_states,
                        instance_states);
            }
        }

        org.opensplice.dds.dcps.ReportStack.flush(
            this,
            result != DDS.RETCODE_OK.value &&
            result != DDS.RETCODE_NO_DATA.value);
        return result;
    }

    @Override
    synchronized public int read_w_condition(
            $(scoped-type-name)SeqHolder received_data,
            DDS.SampleInfoSeqHolder info_seq,
            int max_samples,
            DDS.ReadCondition a_condition)
    {
        int result = DDS.RETCODE_OK.value;
        org.opensplice.dds.dcps.ReportStack.start();

        result = this.readWCondition(
                    copyCache,
                    received_data,
                    info_seq,
                    max_samples,
                    a_condition);

        org.opensplice.dds.dcps.ReportStack.flush(
            this,
            result != DDS.RETCODE_OK.value &&
            result != DDS.RETCODE_NO_DATA.value);
        return result;
    }

    @Override
    synchronized public int take_w_condition(
            $(scoped-type-name)SeqHolder received_data,
            DDS.SampleInfoSeqHolder info_seq,
            int max_samples,
            DDS.ReadCondition a_condition)
    {
        int result = DDS.RETCODE_OK.value;
        org.opensplice.dds.dcps.ReportStack.start();

        result = this.takeWCondition(
                    copyCache,
                    received_data,
                    info_seq,
                    max_samples,
                    a_condition);

        org.opensplice.dds.dcps.ReportStack.flush(
            this,
            result != DDS.RETCODE_OK.value &&
            result != DDS.RETCODE_NO_DATA.value);
        return result;
    }

    @Override
    synchronized public int read_next_sample(
            $(scoped-actual-type-name)Holder received_data,
            DDS.SampleInfoHolder sample_info)
    {
        int result = DDS.RETCODE_UNSUPPORTED.value;
        org.opensplice.dds.dcps.ReportStack.start();

        org.opensplice.dds.dcps.ReportStack.report(
            result, "read_next_sample not yet supported.");

        org.opensplice.dds.dcps.ReportStack.flush(
            this,
            result != DDS.RETCODE_OK.value &&
            result != DDS.RETCODE_NO_DATA.value);
        return result;
    /*
        long uReader = 0;
        int result = DDS.RETCODE_ALREADY_DELETED.value;

        uReader = this.get_user_object();
        if (uReader != 0) {
            if (received_data != null && sample_info != null) {
                result = org.opensplice.dds.dcps.FooDataReaderImpl.jniReadNextSample (
                        this,
                        uReader,
                        copyCache,
                        received_data,
                        sample_info);
            } else {
                result = DDS.RETCODE_BAD_PARAMETER.value;
            }
        }
        return result;
    */
    }

    @Override
    synchronized public int take_next_sample(
            $(scoped-actual-type-name)Holder received_data,
            DDS.SampleInfoHolder sample_info)
    {
        int result = DDS.RETCODE_UNSUPPORTED.value;
        org.opensplice.dds.dcps.ReportStack.start();

        org.opensplice.dds.dcps.ReportStack.report(
            result, "take_next_sample not yet supported.");

        org.opensplice.dds.dcps.ReportStack.flush(
            this,
            result != DDS.RETCODE_OK.value &&
            result != DDS.RETCODE_NO_DATA.value);
        return result;
    /*
        long uReader = 0;
        int result = DDS.RETCODE_ALREADY_DELETED.value;

        uReader = this.get_user_object();
        if (uReader != 0) {
            if (received_data != null && sample_info != null) {
                result = org.opensplice.dds.dcps.FooDataReaderImpl.jniTakeNextSample(
                        this,
                        uReader,
                        copyCache,
                        received_data,
                        sample_info);
            } else {
                result = DDS.RETCODE_BAD_PARAMETER.value;
            }
        }
        return result;
    */
    }

    @Override
    synchronized public int read_instance(
            $(scoped-type-name)SeqHolder received_data,
            DDS.SampleInfoSeqHolder info_seq,
            int max_samples,
            long a_handle,
            int sample_states,
            int view_states,
            int instance_states)
    {
        long uReader = 0;
        int result = DDS.RETCODE_ALREADY_DELETED.value;
        boolean handleExpired = false;

        org.opensplice.dds.dcps.ReportStack.start();

        uReader = this.get_user_object();
        if (uReader != 0) {
            if (received_data == null) {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                org.opensplice.dds.dcps.ReportStack.report(
                    result, "received_data 'null' is invalid.");
            } else if (info_seq == null) {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                org.opensplice.dds.dcps.ReportStack.report(
                    result, "info_seq 'null' is invalid.");
            } else if (max_samples < -1) {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                org.opensplice.dds.dcps.ReportStack.report(
                    result, "max_samples is invalid.");
            } else {
                if (a_handle != DDS.HANDLE_NIL.value) {
                    result = org.opensplice.dds.dcps.FooDataReaderImpl.jniReadInstance (
                            this,
                            uReader,
                            copyCache,
                            received_data,
                            info_seq,
                            max_samples,
                            a_handle,
                            sample_states,
                            view_states,
                            instance_states);

                    if (result == DDS.RETCODE_HANDLE_EXPIRED.value) {
                        result = DDS.RETCODE_BAD_PARAMETER.value;
                        handleExpired = true;
                    }
                } else {
                    result = DDS.RETCODE_BAD_PARAMETER.value;
                    org.opensplice.dds.dcps.ReportStack.report(
                        result, "a_handle 'HANDLE_NIL' is invalid.");
                }
            }
        }

        org.opensplice.dds.dcps.ReportStack.flush(
            this,
            result != DDS.RETCODE_OK.value &&
            result != DDS.RETCODE_NO_DATA.value &&
            !handleExpired);
        return result;
    }

    @Override
    synchronized public int take_instance(
            $(scoped-type-name)SeqHolder received_data,
            DDS.SampleInfoSeqHolder info_seq,
            int max_samples,
            long a_handle,
            int sample_states,
            int view_states,
            int instance_states)
    {
        long uReader = 0;
        int result = DDS.RETCODE_ALREADY_DELETED.value;
        boolean handleExpired = false;

        org.opensplice.dds.dcps.ReportStack.start();

        uReader = this.get_user_object();
        if (uReader != 0) {
            if (received_data == null) {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                org.opensplice.dds.dcps.ReportStack.report(
                    result, "received_data 'null' is invalid.");
            } else if (info_seq == null) {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                org.opensplice.dds.dcps.ReportStack.report(
                    result, "info_seq 'null' is invalid.");
            } else if (max_samples < -1) {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                org.opensplice.dds.dcps.ReportStack.report(
                    result, "max_samples is invalid.");
            } else {
                if (a_handle != DDS.HANDLE_NIL.value) {
                    result = org.opensplice.dds.dcps.FooDataReaderImpl.jniTakeInstance(
                            this,
                            uReader,
                            copyCache,
                            received_data,
                            info_seq,
                            max_samples,
                            a_handle,
                            sample_states,
                            view_states,
                            instance_states);

                    if (result == DDS.RETCODE_HANDLE_EXPIRED.value) {
                        result = DDS.RETCODE_BAD_PARAMETER.value;
                        handleExpired = true;
                    }
                } else {
                    result = DDS.RETCODE_BAD_PARAMETER.value;
                    org.opensplice.dds.dcps.ReportStack.report(
                        result, "a_handle 'HANDLE_NIL' is invalid.");
                }
            }
        }

        org.opensplice.dds.dcps.ReportStack.flush(
            this,
            result != DDS.RETCODE_OK.value &&
            result != DDS.RETCODE_NO_DATA.value &&
            !handleExpired);
        return result;
    }

    @Override
    synchronized public int read_next_instance(
            $(scoped-type-name)SeqHolder received_data,
            DDS.SampleInfoSeqHolder info_seq,
            int max_samples,
            long a_handle,
            int sample_states,
            int view_states,
            int instance_states)
    {
        long uReader = 0;
        int result = DDS.RETCODE_ALREADY_DELETED.value;
        boolean handleExpired = false;

        org.opensplice.dds.dcps.ReportStack.start();

        uReader = this.get_user_object();
        if (uReader != 0) {
            if (received_data == null) {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                org.opensplice.dds.dcps.ReportStack.report(
                    result, "received_data 'null' is invalid.");
            } else if (info_seq == null) {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                org.opensplice.dds.dcps.ReportStack.report(
                    result, "info_seq 'null' is invalid.");
            } else if (max_samples < -1) {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                org.opensplice.dds.dcps.ReportStack.report(
                    result, "max_samples is invalid.");
            } else {
                result = org.opensplice.dds.dcps.FooDataReaderImpl.jniReadNextInstance(
                        this,
                        uReader,
                        copyCache,
                        received_data,
                        info_seq,
                        max_samples,
                        a_handle,
                        sample_states,
                        view_states,
                        instance_states);

                if (result == DDS.RETCODE_HANDLE_EXPIRED.value) {
                    result = DDS.RETCODE_BAD_PARAMETER.value;
                    handleExpired = true;
                }
            }
        }

        org.opensplice.dds.dcps.ReportStack.flush(
            this,
            result != DDS.RETCODE_OK.value &&
            result != DDS.RETCODE_NO_DATA.value &&
            !handleExpired);
        return result;
    }

    @Override
    synchronized public int take_next_instance(
            $(scoped-type-name)SeqHolder received_data,
            DDS.SampleInfoSeqHolder info_seq,
            int max_samples,
            long a_handle,
            int sample_states,
            int view_states,
            int instance_states)
    {
        long uReader = 0;
        int result = DDS.RETCODE_ALREADY_DELETED.value;
        boolean handleExpired = false;
        org.opensplice.dds.dcps.ReportStack.start();

        uReader = this.get_user_object();
        if (uReader != 0) {
            if (received_data == null) {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                org.opensplice.dds.dcps.ReportStack.report(
                    result, "received_data 'null' is invalid.");
            } else if (info_seq == null) {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                org.opensplice.dds.dcps.ReportStack.report(
                    result, "info_seq 'null' is invalid.");
            } else if (max_samples < -1) {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                org.opensplice.dds.dcps.ReportStack.report(
                    result, "max_samples is invalid.");
            } else {
                result = org.opensplice.dds.dcps.FooDataReaderImpl.jniTakeNextInstance(
                        this,
                        uReader,
                        copyCache,
                        received_data,
                        info_seq,
                        max_samples,
                        a_handle,
                        sample_states,
                        view_states,
                        instance_states);

                if (result == DDS.RETCODE_HANDLE_EXPIRED.value) {
                    result = DDS.RETCODE_BAD_PARAMETER.value;
                    handleExpired = true;
                }
            }
        }

        org.opensplice.dds.dcps.ReportStack.flush(
            this,
            result != DDS.RETCODE_OK.value &&
            result != DDS.RETCODE_NO_DATA.value &&
            !handleExpired);
        return result;
    }

    @Override
    synchronized public int read_next_instance_w_condition(
            $(scoped-type-name)SeqHolder received_data,
            DDS.SampleInfoSeqHolder info_seq,
            int max_samples,
            long a_handle,
            DDS.ReadCondition a_condition)
    {
        int result;
        boolean handleExpired = false;

        org.opensplice.dds.dcps.ReportStack.start();

        result = this.readNextInstanceWCondition(
                    copyCache,
                    received_data,
                    info_seq,
                    max_samples,
                    a_handle,
                    a_condition);

        if (result == DDS.RETCODE_HANDLE_EXPIRED.value) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            handleExpired = true;
        }

        org.opensplice.dds.dcps.ReportStack.flush(
            this,
            result != DDS.RETCODE_OK.value &&
            result != DDS.RETCODE_NO_DATA.value &&
            !handleExpired);
        return result;
    }

    @Override
    synchronized public int take_next_instance_w_condition(
            $(scoped-type-name)SeqHolder received_data,
            DDS.SampleInfoSeqHolder info_seq,
            int max_samples,
            long a_handle,
            DDS.ReadCondition a_condition)
    {
        int result = DDS.RETCODE_OK.value;
        boolean handleExpired = false;

        org.opensplice.dds.dcps.ReportStack.start();

        result = this.takeNextInstanceWCondition(
                    copyCache,
                    received_data,
                    info_seq,
                    max_samples,
                    a_handle,
                    a_condition);

        if (result == DDS.RETCODE_HANDLE_EXPIRED.value) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            handleExpired = true;
        }

        org.opensplice.dds.dcps.ReportStack.flush(
            this,
            result != DDS.RETCODE_OK.value &&
            result != DDS.RETCODE_NO_DATA.value &&
            !handleExpired);
        return result;
    }

    @Override
    synchronized public int return_loan(
            $(scoped-type-name)SeqHolder received_data,
            DDS.SampleInfoSeqHolder info_seq)
    {
        int result;
        org.opensplice.dds.dcps.ReportStack.start();

        if (get_user_object() != 0) {
            if (received_data == null) {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                org.opensplice.dds.dcps.ReportStack.report(
                    result, "received_data 'null' is invalid.");
            } else if (info_seq == null) {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                org.opensplice.dds.dcps.ReportStack.report(
                    result, "info_seq 'null' is invalid.");
            } else {
                if (received_data.value != null && info_seq.value != null) {
                    if (received_data.value.length == info_seq.value.length) {
                        received_data.value = null;
                        info_seq.value = null;
                        result = DDS.RETCODE_OK.value;
                    } else {
                        result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
                        org.opensplice.dds.dcps.ReportStack.report(
                            result, "Length of received_data.value and info_seq.value are not equal.");
                    }
                } else {
                    if ((received_data.value == null) && (info_seq.value == null)) {
                        result = DDS.RETCODE_OK.value;
                    } else if (received_data.value == null) {
                        result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
                        org.opensplice.dds.dcps.ReportStack.report(
                            result, "received_data.value is 'null' while info_seq.value is not.");
                    } else {
                        result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
                        org.opensplice.dds.dcps.ReportStack.report(
                            result, "info_seq.value is 'null' while received_data.value is not.");
                    }
                }
            }
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }

        org.opensplice.dds.dcps.ReportStack.flush(
            this,
            result != DDS.RETCODE_OK.value &&
            result != DDS.RETCODE_NO_DATA.value);
        return result;
    }

    @Override
    synchronized public int get_key_value(
            $(scoped-actual-type-name)Holder key_holder,
            long handle)
    {
        long uReader = 0;
        int result = DDS.RETCODE_ALREADY_DELETED.value;
        org.opensplice.dds.dcps.ReportStack.start();

        uReader = this.get_user_object();
        if (uReader != 0) {
            if (key_holder == null) {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                org.opensplice.dds.dcps.ReportStack.report(
                    result, "key_holder 'null' is invalid.");
            } else {
                result = org.opensplice.dds.dcps.FooDataReaderImpl.jniGetKeyValue (
                        uReader,
                        copyCache,
                        key_holder,
                        handle);
            }
        }

        org.opensplice.dds.dcps.ReportStack.flush(
            this,
            result != DDS.RETCODE_OK.value &&
            result != DDS.RETCODE_NO_DATA.value);
        return result;
    }

    @Override
    synchronized public long lookup_instance(
        $(scoped-actual-type-name) instance)
    {
        long uReader = 0;
        long handle = DDS.HANDLE_NIL.value;
        org.opensplice.dds.dcps.ReportStack.start();

        uReader = this.get_user_object();
        if (uReader != 0) {
            handle = org.opensplice.dds.dcps.FooDataReaderImpl.jniLookupInstance(
                    uReader,
                    copyCache,
                    instance);
        }

        org.opensplice.dds.dcps.ReportStack.flush(this, false);
        return handle;
    }

    synchronized protected DDS.DataReaderView create_dataview ()
    {
        return new $(type-name)DataReaderViewImpl(typeSupport);
    }
}

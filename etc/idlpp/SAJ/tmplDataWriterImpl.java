public class $(type-name)DataWriterImpl extends org.opensplice.dds.dcps.DataWriterImpl implements $(type-name)DataWriter
{
    private static final long serialVersionUID = 1L;

    private long copyCache;
    private $(type-name)TypeSupport typeSupport;

    public $(type-name)DataWriterImpl($(scoped-type-name)TypeSupport ts)
    {
        typeSupport = ts;
        copyCache = typeSupport.get_copyCache ();
    }

    @Override
    public long register_instance(
            $(scoped-actual-type-name) instance_data)
    {
        long uWriter = 0;
        long handle = DDS.HANDLE_NIL.value;
        org.opensplice.dds.dcps.ReportStack.start();

        uWriter = this.get_user_object();
        if (uWriter != 0) {
            if (instance_data == null) {
                org.opensplice.dds.dcps.ReportStack.report(
                    DDS.RETCODE_BAD_PARAMETER.value, "instance_data 'null' is invalid.");
            } else {
                handle = org.opensplice.dds.dcps.FooDataWriterImpl.jniRegisterInstance(
                    uWriter,
                    copyCache,
                    instance_data,
                    org.opensplice.dds.dcps.Utilities.DDS_TIMESTAMP_CURRENT);
            }
        }

        org.opensplice.dds.dcps.ReportStack.flush(
            this, handle == DDS.HANDLE_NIL.value);
        return handle;
    }

    @Override
    public long register_instance_w_timestamp(
            $(scoped-actual-type-name) instance_data,
            DDS.Time_t source_timestamp)
    {
        int result = DDS.RETCODE_ALREADY_DELETED.value;
        long uWriter = 0;
        long handle = DDS.HANDLE_NIL.value;
        org.opensplice.dds.dcps.ReportStack.start();

        uWriter = this.get_user_object();
        if (uWriter != 0) {
            if (instance_data == null) {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                org.opensplice.dds.dcps.ReportStack.report(
                    result, "instance_data 'null' is invalid.");
            } else {
                result = org.opensplice.dds.dcps.Utilities.checkTime(source_timestamp);
            }

            if (result == DDS.RETCODE_OK.value) {
                handle = org.opensplice.dds.dcps.FooDataWriterImpl.jniRegisterInstance(
                    uWriter,
                    copyCache,
                    instance_data,
                    source_timestamp);
            }
        }

        org.opensplice.dds.dcps.ReportStack.flush(
            this, handle == DDS.HANDLE_NIL.value);
        return handle;
    }

    @Override
    public int unregister_instance(
            $(scoped-actual-type-name) instance_data,
            long handle)
    {
        int result = DDS.RETCODE_ALREADY_DELETED.value;
        long uWriter = 0;
        org.opensplice.dds.dcps.ReportStack.start();

        uWriter = this.get_user_object();
        if (uWriter != 0) {
            result = org.opensplice.dds.dcps.FooDataWriterImpl.jniUnregisterInstance(
                    uWriter,
                    copyCache,
                    instance_data,
                    handle,
                    org.opensplice.dds.dcps.Utilities.DDS_TIMESTAMP_CURRENT);
        }

        org.opensplice.dds.dcps.ReportStack.flush(
            this, (result != DDS.RETCODE_OK.value) && (result != DDS.RETCODE_TIMEOUT.value));
        return result;
    }

    @Override
    public int unregister_instance_w_timestamp(
            $(scoped-actual-type-name) instance_data,
            long handle,
            DDS.Time_t source_timestamp)
    {
        int result = DDS.RETCODE_ALREADY_DELETED.value;
        long uWriter = 0;
        org.opensplice.dds.dcps.ReportStack.start();

        uWriter = this.get_user_object();
        if (uWriter != 0) {
            result = org.opensplice.dds.dcps.Utilities.checkTime(source_timestamp);

            if (result == DDS.RETCODE_OK.value) {
                result = org.opensplice.dds.dcps.FooDataWriterImpl.jniUnregisterInstance(
                        uWriter,
                        copyCache,
                        instance_data,
                        handle,
                        source_timestamp);
            }
        }

        org.opensplice.dds.dcps.ReportStack.flush(
            this, (result != DDS.RETCODE_OK.value) && (result != DDS.RETCODE_TIMEOUT.value));
        return result;
    }

    @Override
    public int write(
            $(scoped-actual-type-name) instance_data,
            long handle)
    {
        int result = DDS.RETCODE_ALREADY_DELETED.value;
        long uWriter = 0;
        org.opensplice.dds.dcps.ReportStack.start();

        uWriter = this.get_user_object();
        if (uWriter != 0) {
            if (instance_data == null) {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                org.opensplice.dds.dcps.ReportStack.report(
                    result, "instance_data 'null' is invalid.");
            } else {
                result = org.opensplice.dds.dcps.FooDataWriterImpl.jniWrite(
                        uWriter,
                        copyCache,
                        instance_data,
                        handle,
                        org.opensplice.dds.dcps.Utilities.DDS_TIMESTAMP_CURRENT);
            }
        }

        org.opensplice.dds.dcps.ReportStack.flush(
            this, (result != DDS.RETCODE_OK.value) && (result != DDS.RETCODE_TIMEOUT.value));
        return result;
    }

    @Override
    public int write_w_timestamp(
            $(scoped-actual-type-name) instance_data,
            long handle,
            DDS.Time_t source_timestamp)
    {
        int result = DDS.RETCODE_ALREADY_DELETED.value;
        long uWriter = 0;
        org.opensplice.dds.dcps.ReportStack.start();

        uWriter = this.get_user_object();
        if (uWriter != 0) {
            if (instance_data == null) {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                org.opensplice.dds.dcps.ReportStack.report(
                    result, "instance_data 'null' is invalid.");
            } else {
                result = org.opensplice.dds.dcps.Utilities.checkTime(source_timestamp);
            }

            if (result == DDS.RETCODE_OK.value) {
                result = org.opensplice.dds.dcps.FooDataWriterImpl.jniWrite(
                        uWriter,
                        copyCache,
                        instance_data,
                        handle,
                        source_timestamp);
            }
        }

        org.opensplice.dds.dcps.ReportStack.flush(
            this, (result != DDS.RETCODE_OK.value) && (result != DDS.RETCODE_TIMEOUT.value));
        return result;
    }

    @Override
    public int dispose(
            $(scoped-actual-type-name) instance_data,
            long instance_handle)
    {
        int result = DDS.RETCODE_ALREADY_DELETED.value;
        long uWriter = 0;
        org.opensplice.dds.dcps.ReportStack.start();

        uWriter = this.get_user_object();
        if (uWriter != 0) {
            result = org.opensplice.dds.dcps.FooDataWriterImpl.jniDispose(
                    uWriter,
                    copyCache,
                    instance_data,
                    instance_handle,
                    org.opensplice.dds.dcps.Utilities.DDS_TIMESTAMP_CURRENT);
        }

        org.opensplice.dds.dcps.ReportStack.flush(
            this, (result != DDS.RETCODE_OK.value) && (result != DDS.RETCODE_TIMEOUT.value));
        return result;
    }

    @Override
    public int dispose_w_timestamp(
            $(scoped-actual-type-name) instance_data,
            long instance_handle,
            DDS.Time_t source_timestamp)
    {
        int result = DDS.RETCODE_OK.value;
        long uWriter = 0;
        org.opensplice.dds.dcps.ReportStack.start();

        uWriter = this.get_user_object();
        if (uWriter != 0) {
            result = org.opensplice.dds.dcps.Utilities.checkTime (source_timestamp);

            if (result == DDS.RETCODE_OK.value) {
                result = org.opensplice.dds.dcps.FooDataWriterImpl.jniDispose(
                        uWriter,
                        copyCache,
                        instance_data,
                        instance_handle,
                        source_timestamp);
            }
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }

        org.opensplice.dds.dcps.ReportStack.flush(
            this, (result != DDS.RETCODE_OK.value) && (result != DDS.RETCODE_TIMEOUT.value));
        return result;
    }

    @Override
    public int writedispose(
            $(scoped-actual-type-name) instance_data,
            long handle)
    {
        int result = DDS.RETCODE_OK.value;
        long uWriter = 0;
        org.opensplice.dds.dcps.ReportStack.start();

        uWriter = this.get_user_object();
        if (uWriter != 0) {
            if (instance_data == null) {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                org.opensplice.dds.dcps.ReportStack.report(
                    result, "instance_data 'null' is invalid.");
            } else {
                result = org.opensplice.dds.dcps.FooDataWriterImpl.jniWritedispose(
                        uWriter,
                        copyCache,
                        instance_data,
                        handle,
                        org.opensplice.dds.dcps.Utilities.DDS_TIMESTAMP_CURRENT);
            }
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }

        org.opensplice.dds.dcps.ReportStack.flush(
            this, (result != DDS.RETCODE_OK.value) && (result != DDS.RETCODE_TIMEOUT.value));
        return result;
    }

    @Override
    public int writedispose_w_timestamp(
            $(scoped-actual-type-name) instance_data,
            long handle,
            DDS.Time_t source_timestamp)
    {
        int result = DDS.RETCODE_ALREADY_DELETED.value;
        long uWriter = 0;
        org.opensplice.dds.dcps.ReportStack.start();

        uWriter = this.get_user_object();
        if (uWriter != 0) {
            if (instance_data == null) {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                org.opensplice.dds.dcps.ReportStack.report(
                    result, "instance_data 'null' is invalid.");
            } else {
                result = org.opensplice.dds.dcps.Utilities.checkTime(source_timestamp);
            }

            if (result == DDS.RETCODE_OK.value) {
                result = org.opensplice.dds.dcps.FooDataWriterImpl.jniWritedispose(
                        uWriter,
                        copyCache,
                        instance_data,
                        handle,
                        source_timestamp);
            }
        }

        org.opensplice.dds.dcps.ReportStack.flush(
            this, (result != DDS.RETCODE_OK.value) && (result != DDS.RETCODE_TIMEOUT.value));
        return result;
    }

    @Override
    public int get_key_value(
            $(scoped-actual-type-name)Holder key_holder,
            long handle)
    {
        int result = DDS.RETCODE_OK.value;
        long uWriter = 0;
        org.opensplice.dds.dcps.ReportStack.start();

        uWriter = this.get_user_object();
        if (uWriter != 0) {
            if (key_holder == null) {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                org.opensplice.dds.dcps.ReportStack.report(
                    result, "key_holder 'null' is invalid.");
            } else {
                result = org.opensplice.dds.dcps.FooDataWriterImpl.jniGetKeyValue(
                    uWriter,
                    copyCache,
                    key_holder,
                    handle);
            }
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }

        org.opensplice.dds.dcps.ReportStack.flush(
            this, result != DDS.RETCODE_OK.value);
        return result;
    }

    @Override
    public long lookup_instance(
            $(scoped-actual-type-name) instance_data)
    {
        int result;
        long uWriter = 0;
        long handle = DDS.HANDLE_NIL.value;
        org.opensplice.dds.dcps.ReportStack.start();

        uWriter = this.get_user_object();
        if (uWriter != 0) {
            if (instance_data == null) {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                org.opensplice.dds.dcps.ReportStack.report(
                    result, "instance_data 'null' is invalid.");
            } else {
                handle = org.opensplice.dds.dcps.FooDataWriterImpl.jniLookupInstance(
                        uWriter,
                        copyCache,
                        instance_data);
            }
        }

        org.opensplice.dds.dcps.ReportStack.flush(
            this, handle == DDS.HANDLE_NIL.value);
        return handle;
    }
}

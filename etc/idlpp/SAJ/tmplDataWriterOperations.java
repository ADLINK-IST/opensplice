public interface $(type-name)DataWriterOperations extends
    DDS.DataWriterOperations
{

    long register_instance(
            $(scoped-actual-type-name) instance_data);

    long register_instance_w_timestamp(
            $(scoped-actual-type-name) instance_data, 
            DDS.Time_t source_timestamp);

    int unregister_instance(
            $(scoped-actual-type-name) instance_data, 
            long handle);

    int unregister_instance_w_timestamp(
            $(scoped-actual-type-name) instance_data, 
            long handle, 
            DDS.Time_t source_timestamp);

    int write(
            $(scoped-actual-type-name) instance_data, 
            long handle);

    int write_w_timestamp(
            $(scoped-actual-type-name) instance_data, 
            long handle, 
            DDS.Time_t source_timestamp);

    int dispose(
            $(scoped-actual-type-name) instance_data, 
            long instance_handle);

    int dispose_w_timestamp(
            $(scoped-actual-type-name) instance_data, 
            long instance_handle, 
            DDS.Time_t source_timestamp);
    
    int writedispose(
            $(scoped-actual-type-name) instance_data, 
            long instance_handle);

    int writedispose_w_timestamp(
            $(scoped-actual-type-name) instance_data, 
            long instance_handle, 
            DDS.Time_t source_timestamp);

    int get_key_value(
            $(scoped-actual-type-name)Holder key_holder, 
            long handle);
    
    long lookup_instance(
            $(scoped-actual-type-name) instance_data);

}

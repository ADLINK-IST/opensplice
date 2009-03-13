public interface $(type-name)DataReaderOperations extends
    DDS.DataReaderOperations
{

    int read(
            $(scoped-type-name)SeqHolder received_data, 
            DDS.SampleInfoSeqHolder info_seq, 
            int max_samples, 
            int sample_states, 
            int view_states, 
            int instance_states);

    int take(
            $(scoped-type-name)SeqHolder received_data, 
            DDS.SampleInfoSeqHolder info_seq, 
            int max_samples, 
            int sample_states, 
            int view_states, 
            int instance_states);

    int read_w_condition(
            $(scoped-type-name)SeqHolder received_data, 
            DDS.SampleInfoSeqHolder info_seq, 
            int max_samples, 
            DDS.ReadCondition a_condition);

    int take_w_condition(
            $(scoped-type-name)SeqHolder received_data, 
            DDS.SampleInfoSeqHolder info_seq, 
            int max_samples, 
            DDS.ReadCondition a_condition);

    int read_next_sample(
            $(scoped-actual-type-name)Holder received_data, 
            DDS.SampleInfoHolder sample_info);

    int take_next_sample(
            $(scoped-actual-type-name)Holder received_data, 
            DDS.SampleInfoHolder sample_info);

    int read_instance(
            $(scoped-type-name)SeqHolder received_data, 
            DDS.SampleInfoSeqHolder info_seq, 
            int max_samples,
            long a_handle, 
            int sample_states, 
            int view_states, 
            int instance_states);

    int take_instance(
            $(scoped-type-name)SeqHolder received_data, 
            DDS.SampleInfoSeqHolder info_seq, 
            int max_samples, 
            long a_handle, 
            int sample_states, 
            int view_states, 
            int instance_states);

    int read_next_instance(
            $(scoped-type-name)SeqHolder received_data, 
            DDS.SampleInfoSeqHolder info_seq, 
            int max_samples, 
            long a_handle, 
            int sample_states, 
            int view_states, 
            int instance_states);

    int take_next_instance(
            $(scoped-type-name)SeqHolder received_data, 
            DDS.SampleInfoSeqHolder info_seq, 
            int max_samples, 
            long a_handle, 
            int sample_states, 
            int view_states, 
            int instance_states);

    int read_next_instance_w_condition(
            $(scoped-type-name)SeqHolder received_data, 
            DDS.SampleInfoSeqHolder info_seq, 
            int max_samples, 
            long a_handle, 
            DDS.ReadCondition a_condition);

    int take_next_instance_w_condition(
            $(scoped-type-name)SeqHolder received_data, 
            DDS.SampleInfoSeqHolder info_seq, 
            int max_samples, 
            long a_handle, 
            DDS.ReadCondition a_condition);

    int return_loan(
            $(scoped-type-name)SeqHolder received_data, 
            DDS.SampleInfoSeqHolder info_seq);

    int get_key_value(
            $(scoped-actual-type-name)Holder key_holder, 
            long handle);
    
    long lookup_instance(
            $(scoped-actual-type-name) instance);

}

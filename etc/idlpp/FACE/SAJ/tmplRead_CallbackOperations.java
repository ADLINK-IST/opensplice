$(package-line)

public interface $(type-name)Read_CallbackOperations {
    void send_event(long transaction_id,
            $(scoped-type-name)Holder message, long message_type_id,
            int message_size, boolean[] waitset,
            FACE.RETURN_CODE_TYPEHolder return_code);
}

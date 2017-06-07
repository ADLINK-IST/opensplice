$(package-line)

import org.vortex.FACE.TS;
import java.util.logging.Level;
import org.vortex.FACE.Holder;
import org.vortex.FACE.Logger;

import FACE.RETURN_CODE_TYPE;

public class $(type-name)TS extends TS {
    public static void Receive_Message(long connection_id, long timeout,
            org.omg.CORBA.LongHolder transaction_id,
            $(scoped-type-name)Holder message,  org.omg.CORBA.LongHolder message_type_id,
            int message_size, FACE.RETURN_CODE_TYPEHolder return_code) {
        if (TS.getInstance().getImpl() == null) {
            return_code.value = FACE.RETURN_CODE_TYPE.NOT_AVAILABLE;
            Logger.getInstance().log("Receive_Message method not available", Level.SEVERE);
            return;
        }
        if (message == null) {
            return_code.value = RETURN_CODE_TYPE.INVALID_PARAM;
            Logger.getInstance().log("Invalid argument message", Level.SEVERE);
            return;
        }
        Holder<$(scoped-type-name)> holder = new Holder<$(scoped-type-name)>();
        TS.getInstance()
                .getImpl()
                .Receive_Message(connection_id, timeout, transaction_id,
                holder, message_type_id, message_size, return_code);
        message.value = holder.value;
    }

    public static void Send_Message(long connection_id, long timeout,
            org.omg.CORBA.LongHolder transaction_id,
            $(scoped-type-name)Holder message,
            long message_type_id,
            org.omg.CORBA.IntHolder message_size,
            FACE.RETURN_CODE_TYPEHolder return_code) {
        if (TS.getInstance().getImpl() == null) {
            return_code.value = FACE.RETURN_CODE_TYPE.NOT_AVAILABLE;
            Logger.getInstance().log("Send_Message method not available", Level.SEVERE);
            return;
        }
        if (message == null) {
            return_code.value = RETURN_CODE_TYPE.INVALID_PARAM;
            Logger.getInstance().log("Invalid argument message", Level.SEVERE);
            return;
        }
        Holder<$(scoped-type-name)> holder = new Holder<$(scoped-type-name)>();
        holder.value = message.value;
        TS.getInstance()
                .getImpl()
                .Send_Message(connection_id, timeout,
                transaction_id, holder, message_type_id,
                message_size, return_code);
    }

    public static void Register_Callback(long connection_id, boolean[] waitset,
            $(scoped-type-name)Read_CallbackHolder data_callback,
            int max_message_size, FACE.RETURN_CODE_TYPEHolder return_code) {
        if (TS.getInstance().getImpl() == null) {
            return_code.value = FACE.RETURN_CODE_TYPE.NOT_AVAILABLE;
            Logger.getInstance().log("Register_Callback method not available", Level.SEVERE);
            return;
        }
        if (data_callback == null) {
            return_code.value = RETURN_CODE_TYPE.INVALID_PARAM;
            Logger.getInstance().log("Invalid argument data_callback", Level.SEVERE);
            return;
        }
        final $(scoped-type-name)Read_CallbackHolder callback = data_callback;

        org.vortex.FACE.Read_CallbackHolder<$(scoped-type-name)> holder = new org.vortex.FACE.Read_CallbackHolder<$(scoped-type-name)>();
        holder.value = new org.vortex.FACE.Read_Callback<$(scoped-type-name)>() {

            @Override
            public void send_event(long transaction_id,
                    Holder<$(type-name)> message, long message_type_id,
                    int message_size, boolean[] waitset,
                    FACE.RETURN_CODE_TYPEHolder return_code) {
                $(scoped-type-name)Holder holder = new $(scoped-type-name)Holder();
                holder.value = message.value;
                callback.value.send_event(transaction_id, holder,
                        message_type_id, message_size, waitset, return_code);
            }
        };
        TS.getInstance()
                .getImpl()
                .Register_Callback(connection_id, waitset,
                holder,
                max_message_size, return_code);
    }
}

$(package-line)

import org.vortex.FACE.TS;
import java.util.logging.Level;
import org.vortex.FACE.Holder;
import org.vortex.FACE.Logger;

import FACE.RETURN_CODE_TYPE;

    /**
     * The Receive_Message Function is used to receive data from another source.
     * This needs to be called on the generated type interface without using the TS Interface.
     *
     * Possible return codes:
     * <ul>
     * <li>NO_ERROR - Successful completion.
     * <li>NO_ACTION - Object target of this operation has already been deleted.
     * <li>INVALID_MODE - An operation was invoked on an inappropriate object or
     * <li>INVALID_PARAM - Illegal parameter value (e.g., connection ID).
     * <li>INVALID_CONFIG - Generic, unspecified error.
     * <li>NOT_AVAILABLE - Unsupported operation.
     * <li>INVALID_MODE
     * <ul>
     * <li>A pre-condition for the operation was not met.
     * <li>Operation invoked on an entity that is not yet enabled.
     * </ul>
     * <li>NO_ACTION - Indicates a transient situation where the operation did
     * </ul>
     * @param connection_id
     *        The connection_id which is used to get the connection where to receive messages on.
     *        This is an input parameter.
     * @param timeout
     *        The timeout in nanoseconds, this is used to determine how long DDS should wait for new messages
     *        to arrive before returning the result.
     *        This is an input parameter.
     * @param transaction_id
     *        The transaction_id, each time a message is read an unique transaction_id is generated for it.
     *        This is an output parameter.
     * @param message
     *        The message that is read by DDS
     *        This is an output parameter.
     * @param message_type_id
     *        The message_type_id for DDS this parameter is not relevant.
     *        This is an output parameter.
     * @param message_size
     *        The message_type_id for DDS this parameter is not relevant.
     *        This is an output parameter.
     * @param return_code
     *        The return_code
     *        This is an output parameter.
     */
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

    /**
     * The Send_Message Function is used to send data to another source.
     * This needs to be called on the generated type interface without using the TS Interface.
     *
     * Possible return codes:
     * <ul>
     * <li>NO_ERROR - Successful completion.
     * <li>NO_ACTION - Object target of this operation has already been deleted.
     * <li>INVALID_MODE - An operation was invoked on an inappropriate object or
     * at an inappropriate time.
     * <li>INVALID_PARAM - Illegal parameter value (e.g., connection ID).
     * <li>INVALID_CONFIG - Generic, unspecified error.
     * <li>NOT_AVAILABLE - Unsupported operation.
     * <li>INVALID_MODE
     * <ul>
     * <li>A pre-condition for the operation was not met.
     * <li>Operation invoked on an entity that is not yet enabled.
     * </ul>
     * <li>INVALID_CONFIG - Service ran out of resources needed to complete the
     * operation.
     * <li>TIMED_OUT - DDS will not return TIMEOUT, but this could be returned
     * by the TSS implementation.
     * </ul>
     *
     * @param connection_id
     *        The connection_id which is used to get the connection where to send messages to.
     *        This is an input parameter.
     * @param timeout
     *        The timeout in nanoseconds, this is used to determine how long DDS at maximum can wait to send the message.
     *        This timeout cannot be greater than max_blocking_time of the supplied DataWriter QoS.
     *        This is an input parameter.
     * @param transaction_id
     *        The transaction_id, each time a message is send an unique transaction_id is generated for it.
     *        This is an output parameter.
     * @param message
     *        The message that is read by DDS
     *        This is an output parameter.
     * @param message_type_id
     *        The message_type_id for DDS this parameter is not relevant.
     *        This is an output parameter.
     * @param message_size
     *        The message_type_id for DDS this parameter is not relevant.
     *        This is an output parameter.
     * @param return_code
     *        The return_code
     *        This is an output parameter.
     */
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

    /**
     * The purpose of Register_Callback is to provide a mechanism to read data without polling.
     * This needs to be called on the generated type interface without using the TS Interface.
     * There can only be one callback registration per connection_id.
     *
     * Possible return codes:
     * <ul>
     * <li>NO_ERROR - Successful completion.
     * <li>NO_ACTION - Callback already registered for specified type.
     * <li>INVALID_PARAM - One or more of the parameters are incorrect (e.g.,
     * invalid connection identification (ID), invalid callback, invalid message
     * size).
     * <li>NOT_AVAILABLE - Callback/routing function not available (e.g.,
     * callback service is not provided in this implementation).
     * <li>INVALID_CONFIG - One or more fields in the configuration data for the
     * connection is invalid (e.g., invalid TSS thread parameters).
     * </ul>
     *
     * <p>
     * <b><i>Data_callback EExample</i></b>
     * <pre>
     * <code>
     *
     * $(type-name)Read_Callback initial = new FaceReadCallback();
     * $(type-name)Read_CallbackHolder data_callback = new $(type-name)Read_CallbackHolder(initial);
     *
     * public class FaceReadCallback implements $(type-name)Read_Callback {
     * {
     *  public void send_event(long transaction_id, DataTypeHolder message, long message_type_id, int message_size,
     *           boolean[] waitset, RETURN_CODE_TYPEHolder return_code) {
     *           do your action here
     *  }
     * }
     * </code>
     * </pre>
     *
     * @param connection_id
     *        The connection_id of the connection that needs to be used for the callback.
     *        This is an input parameter.
     * @param waitset
     *        The waitset for DDS this parameter is not relevant.
     *        This is an input parameter.
     * @param data_callback
     *        The data_callback class in which an action can be set on how to react when receiving data.
     *        This data_callback is the external operation (interface, which must be implemented by the application see example)
     *        that is called by the FACE API when new data is available for this connection.
     *        This is an input parameter.
     * @param max_message_size
     *        The max_message_size for DDS this parameter is not relevant however the max_message_size supplied
     *        needs to be less then the max_message_size of the configured connection.
     *        This is an input parameter.
     * @param return_code the return_code
     *        This is an output parameter.
     */
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

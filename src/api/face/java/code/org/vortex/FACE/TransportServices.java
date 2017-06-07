/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
 */
package org.vortex.FACE;

import java.io.File;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.logging.Level;

import org.omg.CORBA.IntHolder;
import org.omg.CORBA.LongHolder;
import org.omg.CORBA.StringHolder;
import org.omg.dds.core.AlreadyClosedException;
import org.omg.dds.core.IllegalOperationException;
import org.omg.dds.core.NotEnabledException;
import org.omg.dds.core.PreconditionNotMetException;
import org.omg.dds.core.ServiceEnvironment;
import FACE.CONNECTION_DIRECTION_TYPEHolder;
import FACE.MESSAGING_PATTERN_TYPE;
import FACE.RETURN_CODE_TYPE;
import FACE.RETURN_CODE_TYPEHolder;
import FACE.TRANSPORT_CONNECTION_STATUS_TYPEHolder;

public class TransportServices {
    private ServiceEnvironment env;
    private Map<String, ConnectionDescription> connectionDescriptions;
    private Map<Long, Connection<?>> connections;

    TransportServices() {
    }

    public void Initialize(String configuration,
            RETURN_CODE_TYPEHolder return_code) {
        System.setProperty(
                ServiceEnvironment.IMPLEMENTATION_CLASS_NAME_PROPERTY,
                "org.opensplice.dds.core.OsplServiceEnvironment");
        this.env = ServiceEnvironment.createInstance(TransportServices.class.getClassLoader());
        this.connectionDescriptions = Collections
                .synchronizedMap(new HashMap<String, ConnectionDescription>());
        this.connections = Collections
                .synchronizedMap(new HashMap<Long, Connection<?>>());
        if (return_code == null) {
            Logger.getInstance().log("Invalid argument return_code", Level.SEVERE);
            return;
        }
        if (configuration == null) {
            return_code.value = FACE.RETURN_CODE_TYPE.INVALID_PARAM;
            Logger.getInstance().log("Invalid argument configuration", Level.SEVERE);
            return;
        }
        File f = new File(configuration);
        if(f.exists() && !f.isDirectory()) {
            try {
                ConnectionDescription cd = new ConnectionDescription(env,configuration);
                for (ConnectionDescription connection : cd.getConfigurations()) {
                    connectionDescriptions.put(connection.getName(), connection);
                }
                return_code.value = FACE.RETURN_CODE_TYPE.NO_ERROR;
            } catch (Exception e) {
                Logger.getInstance().log(e.getMessage(), Level.SEVERE);
                Logger.getInstance().log(e.toString(), Level.FINEST);
                return_code.value = FACE.RETURN_CODE_TYPE.INVALID_CONFIG;
            }
        } else {
            return_code.value = FACE.RETURN_CODE_TYPE.INVALID_PARAM;
            Logger.getInstance().log("Invalid argument configuration, configuration file (" + f.getAbsolutePath() + ") does not exists", Level.SEVERE);
            return;
        }
    }


    public void Create_Connection(
    /* in */String connection_name,
    /* in */MESSAGING_PATTERN_TYPE pattern,
    /* out */LongHolder connection_id,
    /* out */CONNECTION_DIRECTION_TYPEHolder connection_direction,
    /* out */IntHolder max_message_size,
    /* in */long timeout,
    /* out */RETURN_CODE_TYPEHolder return_code) {
        if (return_code == null) {
            Logger.getInstance().log("Invalid argument return_code", Level.SEVERE);
            return;
        }
        if(connection_name == null ) {
            Logger.getInstance().log("Invalid argument connection_name", Level.SEVERE);
            return_code.value = FACE.RETURN_CODE_TYPE.INVALID_PARAM;
            return;
        }
        if(connection_id == null ) {
            Logger.getInstance().log("Invalid argument connection_id", Level.SEVERE);
            return_code.value = FACE.RETURN_CODE_TYPE.INVALID_PARAM;
            return;
        }
        if (connection_direction == null) {
            Logger.getInstance().log("Invalid argument connection_direction", Level.SEVERE);
            return_code.value = FACE.RETURN_CODE_TYPE.INVALID_PARAM;
            return;
        }
        if (pattern != MESSAGING_PATTERN_TYPE.PUB_SUB) {
            Logger.getInstance().log("Only PUB_SUB message type supported ", Level.SEVERE);
            return_code.value = FACE.RETURN_CODE_TYPE.INVALID_PARAM;
            return;
        }
        synchronized (this.connections) {
            ConnectionDescription description = connectionDescriptions.get(connection_name);
            if (description == null) {
                Logger.getInstance().log("No connection found with name " + connection_name, Level.SEVERE);
                return_code.value = FACE.RETURN_CODE_TYPE.INVALID_CONFIG;
                return;
            }
            Connection<?> connection = this.connections.get(description.getId());
            if (connection != null) {
                return_code.value = FACE.RETURN_CODE_TYPE.NO_ACTION;
                connection_id.value = description.getId();
                connection_direction.value = description.getDirection();
            } else {
                try {
                    connection = Connection.getConnection(description,
                            Class.forName(description.getTypeName()));

                    connection_id.value = description.getId();
                    connection_direction.value = description.getDirection();

                    this.connections.put(connection_id.value, connection);

                    return_code.value = FACE.RETURN_CODE_TYPE.NO_ERROR;
                } catch (ClassNotFoundException e) {
                    Logger.getInstance()
                            .log("Class '"
                                    + description.getTypeName()
                                    + "'  for configured type_name could not be found. Please use fully qualified class name",
                                    Level.SEVERE);
                    return_code.value = FACE.RETURN_CODE_TYPE.INVALID_CONFIG;
                } catch (Exception e) {
                    Logger.getInstance().log(e.getMessage(), Level.SEVERE);
                    Logger.getInstance().log(e.toString(), Level.FINEST);
                    return_code.value = this.getFACEReturnCodeFromException(e);
                }
            }
        }
        return;
    }

    /**
     * For DDS, TS shall keep track of the DomainParticipant and Topic variables
     * for deletion. For SOURCE connection_direction, the Publisher and
     * DataWriter shall be kept track of for deletion. For DESTINATION
     * connection_direction, the Subscriber and DataReader shall be kept track
     * of for deletion.
     *
     * Possible return codes:
     * <ul>
     * <li>NO_ERROR - Successful completion.
     * <li>NO_ACTION - Object target of this operation has already been deleted.
     * <li>INVALID_MODE - An operation was invoked on an inappropriate object or
     * at an inappropriate time.
     * <li>INVALID_PARAM - Connection identification (ID) invalid.
     * <li>NOT_AVAILABLE - Unsupported operation.
     * <li>INVALID_CONFIG - Generic, unspecified error.
     * <li>INVALID_MODE - A pre-condition for the operation was not met. Note:
     * In a FACE implementation, this error may imply an implementation problem
     * since the connection is deleted and should clean up all entities/children
     * associated with the connection.
     * </ul>
     *
     * @param connection_id
     *        The connection_id of the connection that needs to be destroyed.
     *        This is an input parameter.
     * @param return_code
     *        The return_code
     *        This is an output parameter.
     */
    public void Destroy_Connection(long connection_id,
    /* out */RETURN_CODE_TYPEHolder return_code) {
        if (return_code == null) {
            Logger.getInstance().log("Invalid argument return_code", Level.SEVERE);
            return;
        }
        synchronized (this.connections) {
            Connection<?> connection = this.connections.remove(connection_id);
            if (connection == null) {
                Logger.getInstance().log("Invalid argument connection_id could not find a connection for connection_id:" + connection_id, Level.SEVERE);
                return_code.value = RETURN_CODE_TYPE.INVALID_PARAM;
                return;
            }
            try {
                connection.close();
                return_code.value = RETURN_CODE_TYPE.NO_ERROR;
            } catch (Exception e) {
                Logger.getInstance().log(e.getMessage(), Level.SEVERE);
                return_code.value = RETURN_CODE_TYPE.INVALID_MODE;
            }
        }
        return;
    }

    /**
     * The Receive_Message Function is used to receive data from another source.
     * For Java this needs to be called on the generated type interface without using the TS Interface.
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
     * </ul>
     * <ul>
     * <li>A pre-condition for the operation was not met.
     * <li>Operation invoked on an entity that is not yet enabled.
     * </ul>
     *
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
    public <TYPE> void Receive_Message(long connection_id, long timeout,
            LongHolder transaction_id, Holder<TYPE> message, LongHolder message_type_id,
            int message_size, RETURN_CODE_TYPEHolder return_code) {
        if (return_code == null) {
            Logger.getInstance().log("Invalid argument return_code", Level.SEVERE);
            return;
        }
        if (transaction_id == null) {
            Logger.getInstance().log("Invalid argument transaction_id", Level.SEVERE);
            return;
        }
        if (message == null) {
            Logger.getInstance().log("Invalid argument message", Level.SEVERE);
            return;
        }
        synchronized (this.connections) {
            Connection<?> connection = connections.get(connection_id);
            if (connection == null) {
                Logger.getInstance().log("Invalid argument connection_id could not find a connection for connection_id:" + connection_id, Level.SEVERE);
                return_code.value = RETURN_CODE_TYPE.INVALID_PARAM;
                return;
            }

            if (connection.getDirection().value() == FACE.CONNECTION_DIRECTION_TYPE._SOURCE) {
                return_code.value = RETURN_CODE_TYPE.INVALID_MODE;
                Logger.getInstance().log("Invalid connection direction for Receive_Message function", Level.SEVERE);
                return;
            }

            Connection<TYPE> typedConnection = connection.cast();

            if (typedConnection == null) {
                return_code.value = RETURN_CODE_TYPE.INVALID_PARAM;
                return;
            }
            DestinationConnection<TYPE> destConnection = typedConnection.asDestination();

            if (destConnection == null) {
                return_code.value = RETURN_CODE_TYPE.INVALID_PARAM;
                return;
            }
            destConnection.receiveMessage(timeout, transaction_id, message,
                    message_size, return_code);
        }
    }

    /**
     * The Send_Message Function is used to send data to another source.
     * For Java this needs to be called on the generated type interface without using the TS Interface.
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

    public <TYPE> void Send_Message(long connection_id, long timeout,
            LongHolder transaction_id, Holder<TYPE> message, long message_type_id,
            IntHolder message_size, RETURN_CODE_TYPEHolder return_code) {
        if (return_code == null) {
            Logger.getInstance().log("Invalid argument return_code", Level.SEVERE);
            return;
        }
        if (transaction_id == null) {
            Logger.getInstance().log("Invalid argument transaction_id", Level.SEVERE);
            return_code.value = RETURN_CODE_TYPE.INVALID_PARAM;
            return;
        }
        if (message == null) {
            Logger.getInstance().log("Invalid argument message", Level.SEVERE);
            return_code.value = RETURN_CODE_TYPE.INVALID_PARAM;
            return;
        }
        synchronized (this.connections) {
            Connection<?> connection = connections.get(connection_id);

            if (connection == null) {
                Logger.getInstance().log("Invalid argument connection_id could not find a connection for connection_id:" + connection_id, Level.SEVERE);
                return_code.value = RETURN_CODE_TYPE.INVALID_PARAM;
                return;
            }

            if (connection.getDirection().value() == FACE.CONNECTION_DIRECTION_TYPE._DESTINATION) {
                return_code.value = RETURN_CODE_TYPE.INVALID_MODE;
                Logger.getInstance().log("Invalid connection direction for Send_Message function", Level.SEVERE);
                return;
            }

            Connection<TYPE> typedConnection = connection.cast();

            if (typedConnection == null) {
                return_code.value = RETURN_CODE_TYPE.INVALID_PARAM;
                return;
            }
            SourceConnection<TYPE> srcConnection = typedConnection.asSource();

            if (srcConnection == null) {
                return_code.value = RETURN_CODE_TYPE.INVALID_PARAM;
                return;
            }
            return_code.value = srcConnection.sendMessage(message.value, timeout,transaction_id);
        }

    }

    /**
     * The purpose of Register_Callback is to provide a mechanism to read data without polling.
     * For Java this needs to be called on the generated type interface without using the TS Interface.
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
     * @param connection_id
     *        The connection_id of the connection that needs to be used for the callback.
     *        This is an input parameter.
     * @param waitset
     *        The waitset for DDS this parameter is not relevant.
     *        This is an input parameter.
     * @param data_callback
     *        The data_callback class in which an action can be set on how to react when receiving data.
     *        This is an output parameter.
     * @param max_message_size
     *        The max_message_size for DDS this parameter is not relevant however the max_message_size supplied
     *        needs to be less then the max_message_size of the configured connection.
     *        This is an input parameter.
     * @param return_code the return_code
     *        This is an output parameter.
     */
    public <TYPE> void Register_Callback(
    /* in */long connection_id,
    /* in */boolean[] waitset,
    /* inout */Read_CallbackHolder<TYPE> data_callback,
    /* in */int max_message_size,
    /* out */RETURN_CODE_TYPEHolder return_code) {
        if (return_code == null) {
            Logger.getInstance().log("Invalid argument return_code", Level.SEVERE);
            return;
        }
        if (data_callback == null) {
            Logger.getInstance().log("Invalid argument data_callback", Level.SEVERE);
            return_code.value = RETURN_CODE_TYPE.INVALID_PARAM;
            return;
        }
        Connection<?> connection = connections.get(connection_id);

        if (connection == null) {
            Logger.getInstance().log("Invalid argument connection_id could not find a connection for connection_id:" + connection_id, Level.SEVERE);
            return_code.value = RETURN_CODE_TYPE.INVALID_PARAM;
            return;
        }
        if (connection.getDirection().value() == FACE.CONNECTION_DIRECTION_TYPE._SOURCE) {
            return_code.value = RETURN_CODE_TYPE.INVALID_MODE;
            Logger.getInstance().log("Invalid connection direction for Register_Callback function", Level.SEVERE);
            return;
        }

        Connection<TYPE> typedConnection = connection.cast();

        if (typedConnection == null) {
            return_code.value = RETURN_CODE_TYPE.INVALID_PARAM;
            return;
        }
        DestinationConnection<TYPE> destConnection = typedConnection
                .asDestination();

        if (destConnection == null) {
            return_code.value = RETURN_CODE_TYPE.INVALID_PARAM;
            return;
        }
        if (destConnection.getCallback() != null) {
            return_code.value = RETURN_CODE_TYPE.NO_ACTION;
            return;
        }
        return_code.value = destConnection.registerCallback(data_callback,
                max_message_size);
        return;
    }

    public <TYPE> void Unregister_Callback(long connection_id,
            RETURN_CODE_TYPEHolder return_code) {
        if (return_code == null) {
            Logger.getInstance().log("Invalid argument return_code", Level.SEVERE);
            return;
        }
        Connection<?> connection = connections.get(connection_id);

        if (connection == null) {
            Logger.getInstance().log("Invalid argument connection_id could not find a connection for connection_id:" + connection_id, Level.SEVERE);
            return_code.value = RETURN_CODE_TYPE.INVALID_PARAM;
            return;
        }

        if (connection.getDirection().value() == FACE.CONNECTION_DIRECTION_TYPE._SOURCE) {
            return_code.value = RETURN_CODE_TYPE.INVALID_MODE;
            Logger.getInstance().log("Invalid connection direction for Unregister_Callback function", Level.SEVERE);
            return;
        }

        Connection<TYPE> typedConnection = connection.cast();

        if (typedConnection == null) {
            return_code.value = RETURN_CODE_TYPE.INVALID_PARAM;
            return;
        }
        DestinationConnection<TYPE> destConnection = typedConnection
                .asDestination();

        if (destConnection == null) {
            return_code.value = RETURN_CODE_TYPE.INVALID_PARAM;
            return;
        }
        return_code.value = destConnection.unregisterCallback();

        return;

    }

    /**
     * <ul>
     * <li>NO_ERROR - Successful completion.
     * <li>INVALID_PARAM - Connection ID invalid.
     * <li>NOT_AVAILABLE - Implementation not available.
     * </ul>
     */
    public void Get_Connection_Parameters(
    /* inout */StringHolder connection_name,
    /* inout */LongHolder connection_id,
    /* out */TRANSPORT_CONNECTION_STATUS_TYPEHolder connection_status,
    /* out */RETURN_CODE_TYPEHolder return_code) {
        if (return_code == null) {
            Logger.getInstance().log("Invalid argument return_code", Level.SEVERE);
            return;
        }
        if (connection_name == null) {
            Logger.getInstance().log("Invalid argument connection_name", Level.SEVERE);
            return_code.value = RETURN_CODE_TYPE.INVALID_PARAM;
            return;
        }
        if (connection_id == null) {
            Logger.getInstance().log("Invalid argument connection_id", Level.SEVERE);
            return_code.value = RETURN_CODE_TYPE.INVALID_PARAM;
            return;
        }
        if (connection_status == null) {
            Logger.getInstance().log("Invalid argument connection_status", Level.SEVERE);
            return_code.value = RETURN_CODE_TYPE.INVALID_PARAM;
            return;
        }
        synchronized (this.connections) {
            Connection<?> connection = connections.get(connection_id.value);

            if (connection == null) {
                Logger.getInstance().log("Invalid argument connection_id could not find a connection for connection_id:" + connection_id.value, Level.SEVERE);
                return_code.value = RETURN_CODE_TYPE.INVALID_PARAM;
                return;
            }
            connection_name.value = connection.getDescription().getName();
            connection_status.value = connection.getStatus();
            return_code.value = RETURN_CODE_TYPE.NO_ERROR;
        }
        return;
    }

    private RETURN_CODE_TYPE getFACEReturnCodeFromException(Exception e) {
        if (e instanceof AlreadyClosedException
                || e instanceof java.util.concurrent.TimeoutException) {
            return RETURN_CODE_TYPE.NO_ACTION;
        } else if (e instanceof UnsupportedOperationException) {
            return RETURN_CODE_TYPE.NOT_AVAILABLE;
        } else if (e instanceof IllegalArgumentException) {
            return RETURN_CODE_TYPE.INVALID_PARAM;
        } else if (e instanceof NotEnabledException
                || e instanceof PreconditionNotMetException
                || e instanceof IllegalOperationException) {
            return RETURN_CODE_TYPE.INVALID_MODE;
        }
        return RETURN_CODE_TYPE.INVALID_PARAM;

        /*
         * case DDS.RETCODE_ALREADY_DELETED.value: case
         * DDS.RETCODE_NO_DATA.value: return RETURN_CODE_TYPE.NO_ACTION; case
         * DDS.RETCODE_OK.value: return RETURN_CODE_TYPE.NO_ERROR; case
         * DDS.RETCODE_UNSUPPORTED.value: return RETURN_CODE_TYPE.NOT_AVAILABLE;
         * case DDS.RETCODE_BAD_PARAMETER.value: return
         * RETURN_CODE_TYPE.INVALID_PARAM; case DDS.RETCODE_TIMEOUT.value:
         * return RETURN_CODE_TYPE.TIMED_OUT; case
         * DDS.RETCODE_NOT_ENABLED.value: case
         * DDS.RETCODE_PRECONDITION_NOT_MET.value: case
         * DDS.RETCODE_ILLEGAL_OPERATION.value: return
         * RETURN_CODE_TYPE.INVALID_MODE; case DDS.RETCODE_ERROR.value: case
         * DDS.RETCODE_OUT_OF_RESOURCES.value: case
         * DDS.RETCODE_IMMUTABLE_POLICY.value: case
         * DDS.RETCODE_INCONSISTENT_POLICY.value: default: return
         * RETURN_CODE_TYPE.INVALID_CONFIG; }
         */
    }
}

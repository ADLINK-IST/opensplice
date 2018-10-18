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
package org.opensplice.dds.core;

import java.util.concurrent.TimeoutException;

import org.omg.dds.core.Duration;
import org.omg.dds.core.InstanceHandle;
import org.omg.dds.core.Time;

import DDS.ErrorInfo;

public class Utilities {

    public static void checkReturnCode(int retCode,
            OsplServiceEnvironment environment, String message) {
        try {
            checkReturnCode(retCode, environment, message, false);
        } catch (TimeOutExceptionImpl t) {
            throw new DDSExceptionImpl(environment,
                    "Internal error: TimeOutException caught in unexpected location.");
        }
    }

    private static void checkReturnCode(int retCode,
            OsplServiceEnvironment environment, String message,
            boolean withTimeOut) throws TimeOutExceptionImpl {
        switch (retCode) {
        case DDS.RETCODE_PRECONDITION_NOT_MET.value:
            throw new PreconditionNotMetExceptionImpl(environment,
                    getErrorMessage(retCode, message));
        case DDS.RETCODE_OUT_OF_RESOURCES.value:
            throw new OutOfResourcesExceptionImpl(environment, getErrorMessage(
                    retCode, message));
        case DDS.RETCODE_ALREADY_DELETED.value:
            throw new AlreadyClosedExceptionImpl(environment, getErrorMessage(
                    retCode, message));
        case DDS.RETCODE_BAD_PARAMETER.value:
            throw new IllegalArgumentExceptionImpl(environment,
                    getErrorMessage(retCode, message));
        case DDS.RETCODE_ERROR.value:
            throw new DDSExceptionImpl(environment, getErrorMessage(retCode,
                    message));
        case DDS.RETCODE_ILLEGAL_OPERATION.value:
            throw new IllegalOperationExceptionImpl(environment,
                    getErrorMessage(retCode, message));
        case DDS.RETCODE_IMMUTABLE_POLICY.value:
            throw new ImmutablePolicyExceptionImpl(environment,
                    getErrorMessage(retCode, message));
        case DDS.RETCODE_INCONSISTENT_POLICY.value:
            throw new InconsistentPolicyExceptionImpl(environment,
                    getErrorMessage(retCode, message));
        case DDS.RETCODE_NOT_ENABLED.value:
            throw new NotEnabledExceptionImpl(environment, getErrorMessage(
                    retCode, message));
        case DDS.RETCODE_UNSUPPORTED.value:
            throw new UnsupportedOperationExceptionImpl(environment,
                    getErrorMessage(retCode, message));
        case DDS.RETCODE_TIMEOUT.value:
            if (withTimeOut) {
                if (retCode == DDS.RETCODE_TIMEOUT.value) {
                    throw new TimeOutExceptionImpl(environment,
                            getErrorMessage(retCode, message));
                }
            }
        case DDS.RETCODE_OK.value:
        case DDS.RETCODE_NO_DATA.value:
        default:
            break;
        }
    }

    private static String getDetails(DDS.ErrorInfo errorInfo, String message) {
        String result = "";
        DDS.StringHolder messageHolder = new DDS.StringHolder();

        errorInfo.get_message(messageHolder);

        if (messageHolder.value != null) {
            if (message != null) {
                result += message;
                result += "(" + messageHolder.value + ")";
            } else {
                result += messageHolder.value;
            }
        } else if (message != null) {
            result += message;
        }
        errorInfo.get_location(messageHolder);

        if (messageHolder.value != null) {
            result += " at " + messageHolder.value;
        }
        errorInfo.get_source_line(messageHolder);

        if (messageHolder.value != null) {
            result += " (" + messageHolder.value + ")";
        }
        errorInfo.get_stack_trace(messageHolder);

        if (messageHolder.value != null) {
            result += "\nStack trace:\n" + messageHolder.value;
        }
        return result;
    }

    private static String getErrorMessage(int retCode, String message) {
        String output;
        DDS.ErrorInfo errorInfo = new ErrorInfo();
        int result = errorInfo.update();

        switch (result) {
        case DDS.RETCODE_NO_DATA.value:
            output = message;
            break;
        case DDS.RETCODE_OK.value:
            output = getDetails(errorInfo, message);
            break;
        default:
            if (message != null) {
                output = " Unable to get extra error information due to internal error.("
                    + message + ")";
            } else {
                output = " Unable to get extra error information due to internal error.";
            }
            break;
        }
        return output;
    }

    public static String getOsplExceptionStack(Exception ex,
            StackTraceElement[] stack) {
        StringBuffer result = new StringBuffer();
        int startIndex = 0;

        result.append(ex.getClass().getSuperclass().getName() + ": "
                + ex.getMessage() + "\n");

        while ((stack.length > startIndex)
                && (Utilities.class.getName().equals(stack[startIndex]
                        .getClassName()))) {
            startIndex++;
        }
        for (int i = startIndex; i < stack.length; i++) {
            result.append("\tat ");
            result.append(stack[i].getClassName());
            result.append(".");
            result.append(stack[i].getMethodName());
            result.append(" (");
            result.append(stack[i].getFileName());
            result.append(":");
            result.append(stack[i].getLineNumber());
            result.append(")\n");
        }
        return result.toString();
    }

    public static void checkReturnCodeWithTimeout(int retCode,
            OsplServiceEnvironment environment, String message)
            throws TimeoutException {
        checkReturnCode(retCode, environment, message, true);
    }

    public static void throwLastErrorException(
            OsplServiceEnvironment environment) {
        String message;
        int code;
        DDS.ErrorInfo errorInfo = new ErrorInfo();
        int result = errorInfo.update();

        switch (result) {
        case DDS.RETCODE_NO_DATA.value:
            message = "";
            code = DDS.RETCODE_ERROR.value;
            break;
        case DDS.RETCODE_OK.value:
            DDS.ReturnCodeHolder errorHolder = new DDS.ReturnCodeHolder();
            errorInfo.get_code(errorHolder);
            code = errorHolder.value;
            message = getDetails(errorInfo, null);
            break;
        default:
            message = "Unable to get extra error information due to internal error.";
            code = DDS.RETCODE_ERROR.value;
        }

        switch (code) {
        case DDS.RETCODE_PRECONDITION_NOT_MET.value:
            throw new PreconditionNotMetExceptionImpl(environment, message);
        case DDS.RETCODE_OUT_OF_RESOURCES.value:
            throw new OutOfResourcesExceptionImpl(environment, message);
        case DDS.RETCODE_ALREADY_DELETED.value:
            throw new AlreadyClosedExceptionImpl(environment, message);
        case DDS.RETCODE_BAD_PARAMETER.value:
            throw new IllegalArgumentExceptionImpl(environment, message);
        case DDS.RETCODE_ILLEGAL_OPERATION.value:
            throw new IllegalOperationExceptionImpl(environment, message);
        case DDS.RETCODE_IMMUTABLE_POLICY.value:
            throw new ImmutablePolicyExceptionImpl(environment, message);
        case DDS.RETCODE_INCONSISTENT_POLICY.value:
            throw new InconsistentPolicyExceptionImpl(environment, message);
        case DDS.RETCODE_NOT_ENABLED.value:
            throw new NotEnabledExceptionImpl(environment, message);
        case DDS.RETCODE_UNSUPPORTED.value:
            throw new UnsupportedOperationExceptionImpl(environment, message);
        case DDS.RETCODE_ERROR.value:
        default:
            throw new DDSExceptionImpl(environment, message);

        }

    }

    public static DDS.Duration_t convert(OsplServiceEnvironment environment,
            Duration d) {
        if (d == null) {
            throw new IllegalArgumentExceptionImpl(environment,
                    "Illegal Duration provided (null).");
        }
        try {
            return ((DurationImpl) d).convert();
        } catch (ClassCastException e) {
            throw new IllegalArgumentExceptionImpl(environment,
                    "Usage of non-OpenSplice Duration implementation is not supported.");
        }
    }

    public static Duration convert(OsplServiceEnvironment env, DDS.Duration_t d) {
        return new DurationImpl(env, d.sec, d.nanosec);
    }

    public static long convert(OsplServiceEnvironment environment,
            InstanceHandle h) {
        if (h == null) {
            throw new IllegalArgumentExceptionImpl(environment,
                    "Illegal InstanceHandle provided (null).");
        }
        try {
            return ((InstanceHandleImpl) h).getValue();
        } catch (ClassCastException e) {
            throw new IllegalArgumentExceptionImpl(environment,
                    "Usage of non-OpenSplice InstanceHandle implementation is not supported.");
        }
    }

    public static InstanceHandle convert(OsplServiceEnvironment env, long handle) {
        return new InstanceHandleImpl(env, handle);
    }

    public static DDS.Time_t convert(OsplServiceEnvironment environment, Time t) {
        if (t == null) {
            throw new IllegalArgumentExceptionImpl(environment,
                    "Illegal Time provided (null).");
        }
        try {
            return ((ModifiableTimeImpl) t).convert();
        } catch (ClassCastException e) {
            throw new IllegalArgumentExceptionImpl(environment,
                    "Usage of non-OpenSplice Time implementation is not supported.");
        }
    }

    public static Time convert(OsplServiceEnvironment env, DDS.Time_t t) {
        return new TimeImpl(env, t.sec, t.nanosec);
    }
}

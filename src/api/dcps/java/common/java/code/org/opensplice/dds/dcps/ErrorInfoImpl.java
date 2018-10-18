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


package org.opensplice.dds.dcps;

import org.opensplice.dds.dcps.ErrorInfoBase;

/**
 * Implementation of the {@link DDS.Entity} interface.
 */
public class ErrorInfoImpl extends ErrorInfoBase implements DDS.ErrorInfoInterface {

    private static final long serialVersionUID = -2598175768822115853L;
    private boolean valid;
    private int code;
    private String message;
    private String location;
    private String sourceLine;
    private String stackTrace;

    @Override
    public int update() {
        int result = DDS.RETCODE_NO_DATA.value;
        code = DDS.RETCODE_OK.value;
        message = null;
        location = null;
        sourceLine = null;
        stackTrace = null;

        try {
            result = jniUpdate();
        } catch(UnsatisfiedLinkError ule){
            /*
             * JNI library is not loaded if no instance of the
             * DomainParticipantFactory exists.
             */

            DomainParticipantFactoryImpl f = DomainParticipantFactoryImpl.get_instance();

            if(f != null){
                result = jniUpdate();
            }
        }

        valid = (result == DDS.RETCODE_OK.value);

        return result;
    }

    public int get_code (DDS.ReturnCodeHolder code) {
        int result = DDS.RETCODE_OK.value;

        if (code != null) {
            if (this.valid) {
                code.value = this.code;
            } else {
                result = DDS.RETCODE_NO_DATA.value;
            }
        } else {
            result = DDS.RETCODE_BAD_PARAMETER.value;
        }

        return result;
    }

    /**
     * Please do not use these ErrorCode_t values any more.
     * They will be removed in future versions of OpenSplice, and be
     * replaced by the more familiar ReturnCode_t.
     */
    public int get_code (DDS.ErrorCodeHolder code) {
        int result = DDS.RETCODE_OK.value;

        if (code != null) {
            if (this.valid) {
                code.value = this.code;
            } else {
                result = DDS.RETCODE_NO_DATA.value;
            }
        } else {
            result = DDS.RETCODE_BAD_PARAMETER.value;
        }

        return result;
    }

    @Override
    public int get_code(org.omg.CORBA.IntHolder code) {
        int result = DDS.RETCODE_OK.value;

        if (code != null) {
            if (this.valid) {
                code.value = this.code;
            } else {
                result = DDS.RETCODE_NO_DATA.value;
            }
        } else {
            result = DDS.RETCODE_BAD_PARAMETER.value;
        }

        return result;
    }


    public int get_message(DDS.StringHolder message) {
        int result = DDS.RETCODE_OK.value;

        if (message != null) {
            if (this.valid) {
                message.value = this.message;
            } else {
                result = DDS.RETCODE_NO_DATA.value;
            }
        } else {
            result = DDS.RETCODE_BAD_PARAMETER.value;
        }

        return result;
    }

    @Override
    public int get_message(org.omg.CORBA.StringHolder message) {
        int result = DDS.RETCODE_OK.value;

        if (message != null) {
            if (this.valid) {
                message.value = this.message;
            } else {
                result = DDS.RETCODE_NO_DATA.value;
            }
        } else {
            result = DDS.RETCODE_BAD_PARAMETER.value;
        }

        return result;
    }

    public int get_location(DDS.StringHolder location) {
        int result = DDS.RETCODE_OK.value;

        if (location != null) {
            if (this.valid) {
                location.value = this.location;
            } else {
                result = DDS.RETCODE_NO_DATA.value;
            }
        } else {
            result = DDS.RETCODE_BAD_PARAMETER.value;
        }

        return result;
    }

    @Override
    public int get_location(org.omg.CORBA.StringHolder location) {
        int result = DDS.RETCODE_OK.value;

        if (location != null) {
            if (this.valid) {
                location.value = this.location;
            } else {
                result = DDS.RETCODE_NO_DATA.value;
            }
        } else {
            result = DDS.RETCODE_BAD_PARAMETER.value;
        }

        return result;
    }

    public int get_source_line(DDS.StringHolder source_line) {
        int result = DDS.RETCODE_OK.value;

        if (source_line != null) {
            if (this.valid) {
                source_line.value = this.sourceLine;
            } else {
                result = DDS.RETCODE_NO_DATA.value;
            }
        } else {
            result = DDS.RETCODE_BAD_PARAMETER.value;
        }

        return result;
    }

    @Override
    public int get_source_line(org.omg.CORBA.StringHolder source_line) {
        int result = DDS.RETCODE_OK.value;

        if (source_line != null) {
            if (this.valid) {
                source_line.value = this.sourceLine;
            } else {
                result = DDS.RETCODE_NO_DATA.value;
            }
        } else {
            result = DDS.RETCODE_BAD_PARAMETER.value;
        }

        return result;
    }

    public int get_stack_trace(DDS.StringHolder stack_trace) {
        int result = DDS.RETCODE_OK.value;

        if (stack_trace != null) {
            if (this.valid) {
                stack_trace.value = this.stackTrace;
            } else {
                result = DDS.RETCODE_NO_DATA.value;
            }
        } else {
            result = DDS.RETCODE_BAD_PARAMETER.value;
        }

        return result;
    }

    @Override
    public int get_stack_trace(org.omg.CORBA.StringHolder stack_trace) {
        int result = DDS.RETCODE_OK.value;

        if (stack_trace != null) {
            if (this.valid) {
                stack_trace.value = this.stackTrace;
            } else {
                result = DDS.RETCODE_NO_DATA.value;
            }
        } else {
            result = DDS.RETCODE_BAD_PARAMETER.value;
        }

        return result;
    }

    private native int jniUpdate();
}

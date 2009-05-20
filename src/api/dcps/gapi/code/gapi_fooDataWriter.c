/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#include "gapi_fooDataWriter.h"
#include "gapi_dataWriter.h"
#include "gapi_qos.h"
#include "gapi_topic.h"
#include "gapi_kernel.h"
#include "gapi_genericCopyIn.h"
#include "gapi_instanceHandle.h"

#include "os_heap.h"

C_STRUCT(_FooDataWriter) {
    C_EXTENDS(_DataWriter);
};

gapi_instanceHandle_t
gapi_fooDataWriter_register_instance (
    gapi_fooDataWriter _this,
    const gapi_foo *instance_data)
{
    _DataWriter datawriter;
    gapi_instanceHandle_t handle = GAPI_HANDLE_NIL;

    if ( instance_data ) {
        datawriter = gapi_dataWriterClaim(_this, NULL);
        if ( datawriter ) {
            handle = _DataWriterRegisterInstance(datawriter,
                                                 instance_data,
                                                 u_timeGet());
            _EntityRelease(datawriter);
        }
    }
    return handle;
}

gapi_instanceHandle_t
gapi_fooDataWriter_register_instance_w_timestamp (
    gapi_fooDataWriter _this,
    const gapi_foo *instance_data,
    const gapi_time_t *source_timestamp)
{
    _DataWriter datawriter;
    c_time timestamp;
    gapi_returnCode_t result;
    gapi_instanceHandle_t handle = GAPI_HANDLE_NIL;

    if ( instance_data ) {
        datawriter = gapi_dataWriterClaim(_this, NULL);
        if ( datawriter ) {
            result = kernelCopyInTime(source_timestamp, &timestamp);
            if (result == GAPI_RETCODE_OK) {
                handle = _DataWriterRegisterInstance(datawriter,
                                                     instance_data,
                                                     timestamp);
            }
            _EntityRelease(datawriter);
        }
    }
    return handle;
}

gapi_returnCode_t
gapi_fooDataWriter_unregister_instance (
    gapi_fooDataWriter _this,
    const gapi_foo *instance_data,
    const gapi_instanceHandle_t handle)
{
    _DataWriter datawriter;
    gapi_returnCode_t result = GAPI_RETCODE_OK;

    if ( instance_data || handle ) {
        datawriter = gapi_dataWriterClaim(_this, &result);
        if ( datawriter ) {
            result = _DataWriterUnregisterInstance(datawriter,
                                                   instance_data,
                                                   handle,
                                                   u_timeGet());
            _EntityRelease(datawriter);
        }
    } else {
        result = GAPI_RETCODE_PRECONDITION_NOT_MET;
    }
    return result;
}

gapi_returnCode_t
gapi_fooDataWriter_unregister_instance_w_timestamp (
    gapi_fooDataWriter _this,
    const gapi_foo *instance_data,
    const gapi_instanceHandle_t handle,
    const gapi_time_t *source_timestamp)
{
    c_time timestamp;
    _DataWriter datawriter;
    gapi_returnCode_t result = GAPI_RETCODE_OK;

    if ( instance_data || handle ) {
        datawriter = gapi_dataWriterClaim(_this, &result);
        if ( datawriter ) {
            result = kernelCopyInTime(source_timestamp, &timestamp);
            if ( result == GAPI_RETCODE_OK ) {
                result = _DataWriterUnregisterInstance(datawriter,
                                                       instance_data,
                                                       handle,
                                                       timestamp);
            }
            _EntityRelease(datawriter);
        }
    } else {
        result = GAPI_RETCODE_PRECONDITION_NOT_MET;
    }

    return result;
}

gapi_instanceHandle_t
gapi_fooDataWriter_lookup_instance (
    gapi_dataWriter _this,
    const gapi_foo * instance_data)
{
    _DataWriter datawriter;
    u_writer w;
    u_result uResult;
    writerInfo data;
    u_instanceHandle h;
    gapi_instanceHandle_t handle = GAPI_HANDLE_NIL;

    datawriter = gapi_dataWriterClaim(_this, NULL);

    if ( datawriter && instance_data ) {
        data.writer = datawriter;
        data.data = (void *)instance_data;

        w = U_WRITER_GET(datawriter);
        uResult = u_writerLookupInstance(w, &data, &h);

        if(uResult == U_RESULT_OK){
            handle = gapi_instanceHandleFromHandle (h);
        } else {
            handle = GAPI_HANDLE_NIL;
        }
    } else {
        handle = GAPI_HANDLE_NIL;
    }
    _EntityRelease(datawriter);

    return handle;
}

gapi_returnCode_t
gapi_fooDataWriter_write (
    gapi_fooDataWriter _this,
    const gapi_foo *instance_data,
    const gapi_instanceHandle_t handle)
{
    _DataWriter datawriter;
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    writerInfo data;
    u_instanceHandle h;
    u_writer w;
    u_result r;

    if ( instance_data != NULL ) {
        datawriter = gapi_dataWriterClaim(_this, &result);
        if ( datawriter != NULL ) {
            w = U_WRITER_GET(datawriter);
            result = gapi_instanceHandleToHandle(handle,u_entity(w),&h);
            if (result == GAPI_RETCODE_OK) {
                data.writer = datawriter;
                data.data = (void *)instance_data;
                r = u_writerWrite (w,&data,C_TIME_ZERO,h);
                result = kernelResultToApiResult(r);
            }
            _EntityRelease(datawriter);
        }
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }
    return result;
}

gapi_returnCode_t
gapi_fooDataWriter_write_w_timestamp (
    gapi_fooDataWriter _this,
    const gapi_foo *instance_data,
    const gapi_instanceHandle_t handle,
    const gapi_time_t *source_timestamp)
{
    _DataWriter datawriter;
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    writerInfo data;
    u_instanceHandle h;
    u_writer w;

    datawriter = gapi_dataWriterClaim(_this, &result);
    if ( datawriter != NULL ) {
        if ( instance_data != NULL ) {
            u_result r;
            c_time timestamp;

            result = kernelCopyInTime(source_timestamp, &timestamp);
            if ( result == GAPI_RETCODE_OK ) {
                w = U_WRITER_GET(datawriter);
                result = gapi_instanceHandleToHandle(handle,u_entity(w),&h);
                if (result == GAPI_RETCODE_OK) {
                    data.writer = datawriter;
                    data.data = (void *)instance_data;

                    r = u_writerWrite (w,&data,timestamp,h);
                    result = kernelResultToApiResult(r);
                }
            }
        } else {
            result = GAPI_RETCODE_BAD_PARAMETER;
        }
    }
    _EntityRelease(datawriter);

    return result;
}

gapi_returnCode_t
gapi_fooDataWriter_dispose (
    gapi_fooDataWriter _this,
    const gapi_foo *instance_data,
    const gapi_instanceHandle_t handle)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _DataWriter datawriter;
    writerInfo data;
    u_instanceHandle h;
    u_writer w;

    datawriter = gapi_dataWriterClaim(_this, &result);

    if ( datawriter != NULL ) {
        if ( instance_data != NULL ) {
            w = U_WRITER_GET(datawriter);
            result = gapi_instanceHandleToHandle(handle,u_entity(w),&h);
            if (result == GAPI_RETCODE_OK) {
                u_result r;
                data.writer = datawriter;
                data.data = (void *)instance_data;

                r = u_writerDispose (w,&data,C_TIME_ZERO,h);
                result = kernelResultToApiResult(r);
            }
        } else {
            result = GAPI_RETCODE_BAD_PARAMETER;
        }
    }
    _EntityRelease(datawriter);

    return result;
}

gapi_returnCode_t
gapi_fooDataWriter_dispose_w_timestamp (
    gapi_fooDataWriter _this,
    const gapi_foo *instance_data,
    const gapi_instanceHandle_t handle,
    const gapi_time_t *source_timestamp)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _DataWriter datawriter;
    writerInfo data;
    u_instanceHandle h;
    u_writer w;

    datawriter = gapi_dataWriterClaim(_this, &result);
    if ( datawriter != NULL ) {
        if ( instance_data != NULL ) {
            c_time timestamp;
            u_result r;

            result = kernelCopyInTime(source_timestamp, &timestamp);
            if ( result == GAPI_RETCODE_OK ) {
                w = U_WRITER_GET(datawriter);
                result = gapi_instanceHandleToHandle(handle,u_entity(w),&h);
                if (result == GAPI_RETCODE_OK) {
                    data.writer = datawriter;
                    data.data = (void *)instance_data;

                    r = u_writerDispose (w,&data,timestamp,h);
                    result = kernelResultToApiResult(r);
                }
            }
        } else {
            result = GAPI_RETCODE_BAD_PARAMETER;
        }
    }

    _EntityRelease(datawriter);

    return result;
}

gapi_returnCode_t
gapi_fooDataWriter_writedispose (
    gapi_fooDataWriter _this,
    const gapi_foo *instance_data,
    const gapi_instanceHandle_t handle)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _DataWriter datawriter;
    writerInfo data;
    u_instanceHandle h;
    u_writer w;

    datawriter = gapi_dataWriterClaim(_this, &result);

    if ( datawriter != NULL ) {
        if ( instance_data != NULL ) {
            w = U_WRITER_GET(datawriter);
            result = gapi_instanceHandleToHandle(handle,u_entity(w),&h);
            if (result == GAPI_RETCODE_OK) {
               u_result r;
                data.writer = datawriter;
                data.data = (void *)instance_data;

                r = u_writerWriteDispose(w,&data,C_TIME_ZERO,h);
                result = kernelResultToApiResult(r);
            }
        } else {
            result = GAPI_RETCODE_BAD_PARAMETER;
        }
    }

    _EntityRelease(datawriter);

    return result;
}

gapi_returnCode_t
gapi_fooDataWriter_writedispose_w_timestamp (
    gapi_fooDataWriter _this,
    const gapi_foo *instance_data,
    const gapi_instanceHandle_t handle,
    const gapi_time_t *source_timestamp)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _DataWriter datawriter;
    writerInfo data;
    u_instanceHandle h;
    u_writer w;

    datawriter = gapi_dataWriterClaim(_this, &result);

    if ( datawriter != NULL ) {
        if ( instance_data != NULL ) {
            c_time timestamp;
            u_result r;

            result = kernelCopyInTime(source_timestamp, &timestamp);
            if ( result == GAPI_RETCODE_OK ) {
                w = U_WRITER_GET(datawriter);
                result = gapi_instanceHandleToHandle(handle,u_entity(w),&h);
                if (result == GAPI_RETCODE_OK) {
                    data.writer = datawriter;
                    data.data = (void *)instance_data;

                    r = u_writerWriteDispose (w,&data,timestamp,h);
                    result = kernelResultToApiResult(r);
                }
            }
        } else {
            result = GAPI_RETCODE_BAD_PARAMETER;
        }
    }

    _EntityRelease(datawriter);

    return result;
}

gapi_returnCode_t
gapi_fooDataWriter_get_key_value (
    gapi_fooDataWriter _this,
    gapi_foo *key_holder,
    const gapi_instanceHandle_t handle)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _DataWriter datawriter;

    datawriter = gapi_dataWriterClaim(_this, &result);

    if ( datawriter ) {
        if ( (key_holder == NULL) || (handle == GAPI_HANDLE_NIL) ) {
            result = GAPI_RETCODE_BAD_PARAMETER;
        } else {
            result = _DataWriterGetKeyValue(datawriter, key_holder, handle);
        }
    }

    _EntityRelease(datawriter);

    return result;
}






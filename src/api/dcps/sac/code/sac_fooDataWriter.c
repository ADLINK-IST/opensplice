/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

#include "gapi.h"

#include "dds_dcps.h"

DDS_InstanceHandle_t
DDS__FooDataWriter_register_instance (
    DDS_DataWriter this,
    const DDS_sample instance_data
    )
{
    return (DDS_InstanceHandle_t)
	gapi_fooDataWriter_register_instance (
	    (gapi_dataWriter)this,
	    (gapi_foo *)instance_data
	);
}

DDS_InstanceHandle_t
DDS__FooDataWriter_register_instance_w_timestamp (
    DDS_DataWriter this,
    const DDS_sample instance_data,
    const DDS_Time_t *source_timestamp
    )
{
    return (DDS_InstanceHandle_t)
	gapi_fooDataWriter_register_instance_w_timestamp (
	    (gapi_dataWriter)this,
	    (gapi_foo *)instance_data,
	    (const gapi_time_t *)source_timestamp
	);
}

DDS_ReturnCode_t
DDS__FooDataWriter_unregister_instance (
    DDS_DataWriter this,
    const DDS_sample instance_data,
    const DDS_InstanceHandle_t handle
    )
{
    return (DDS_ReturnCode_t)
	gapi_fooDataWriter_unregister_instance (
	    (gapi_dataWriter)this,
	    (gapi_foo *)instance_data,
	    (gapi_instanceHandle_t)handle
	);
}

DDS_ReturnCode_t
DDS__FooDataWriter_unregister_instance_w_timestamp (
    DDS_DataWriter this,
    const DDS_sample instance_data,
    const DDS_InstanceHandle_t handle,
    const DDS_Time_t *source_timestamp
    )
{
    return (DDS_ReturnCode_t)
	gapi_fooDataWriter_unregister_instance_w_timestamp (
	    (gapi_dataWriter)this,
	    (gapi_foo *)instance_data,
	    (gapi_instanceHandle_t)handle,
	    (const gapi_time_t *)source_timestamp
	);
}

DDS_ReturnCode_t
DDS__FooDataWriter_write (
    DDS_DataWriter this,
    const DDS_sample instance_data,
    const DDS_InstanceHandle_t handle
    )
{
    return (DDS_ReturnCode_t)
	gapi_fooDataWriter_write (
	    (gapi_dataWriter)this,
	    (gapi_foo *)instance_data,
	    (gapi_instanceHandle_t)handle
	);
}

DDS_ReturnCode_t
DDS__FooDataWriter_write_w_timestamp (
    DDS_DataWriter this,
    const DDS_sample instance_data,
    const DDS_InstanceHandle_t handle,
    const DDS_Time_t *source_timestamp
    )
{
    return (DDS_ReturnCode_t)
	gapi_fooDataWriter_write_w_timestamp (
	    (gapi_dataWriter)this,
	    (gapi_foo *)instance_data,
	    (gapi_instanceHandle_t)handle,
	    (const gapi_time_t *)source_timestamp
	);
}

DDS_ReturnCode_t
DDS__FooDataWriter_dispose (
    DDS_DataWriter this,
    const DDS_sample instance_data,
    const DDS_InstanceHandle_t instance_handle
    )
{
    return (DDS_ReturnCode_t)
	gapi_fooDataWriter_dispose (
	    (gapi_dataWriter)this,
	    (gapi_foo *)instance_data,
	    (gapi_instanceHandle_t)instance_handle
	);
}

DDS_ReturnCode_t
DDS__FooDataWriter_dispose_w_timestamp (
    DDS_DataWriter this,
    const DDS_sample instance_data,
    const DDS_InstanceHandle_t instance_handle,
    const DDS_Time_t *source_timestamp
    )
{
    return (DDS_ReturnCode_t)
	gapi_fooDataWriter_dispose_w_timestamp (
	    (gapi_dataWriter)this,
	    (gapi_foo *)instance_data,
	    (gapi_instanceHandle_t)instance_handle,
	    (const gapi_time_t *)source_timestamp
	);
}

DDS_ReturnCode_t
DDS__FooDataWriter_writedispose (
    DDS_DataWriter this,
    const DDS_sample instance_data,
    const DDS_InstanceHandle_t instance_handle
    )
{
    return (DDS_ReturnCode_t)
	gapi_fooDataWriter_writedispose (
	    (gapi_dataWriter)this,
	    (gapi_foo *)instance_data,
	    (gapi_instanceHandle_t)instance_handle
	);
}

DDS_ReturnCode_t
DDS__FooDataWriter_writedispose_w_timestamp (
    DDS_DataWriter this,
    const DDS_sample instance_data,
    const DDS_InstanceHandle_t instance_handle,
    const DDS_Time_t *source_timestamp
    )
{
    return (DDS_ReturnCode_t)
	gapi_fooDataWriter_writedispose_w_timestamp (
	    (gapi_dataWriter)this,
	    (gapi_foo *)instance_data,
	    (gapi_instanceHandle_t)instance_handle,
	    (const gapi_time_t *)source_timestamp
	);
}

DDS_ReturnCode_t
DDS__FooDataWriter_get_key_value (
    DDS_DataWriter this,
    DDS_sample key_holder,
    const DDS_InstanceHandle_t handle
    )
{
    return (DDS_ReturnCode_t)
	gapi_fooDataWriter_get_key_value (
	    (gapi_dataWriter)this,
	    (gapi_foo *)key_holder,
	    (gapi_instanceHandle_t)handle
	);
}

DDS_InstanceHandle_t
DDS__FooDataWriter_lookup_instance(
    DDS_DataWriter this,
    const DDS_sample instance_data
    )
{
    return (DDS_InstanceHandle_t)
        gapi_fooDataWriter_lookup_instance (
            (gapi_dataWriter)this,
            (gapi_foo *)instance_data
        );
}


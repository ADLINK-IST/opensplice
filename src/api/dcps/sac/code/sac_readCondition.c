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

/*     SampleStateMask
 *     get_sample_state_mask();
 */
DDS_SampleStateMask
DDS_ReadCondition_get_sample_state_mask (
    DDS_ReadCondition this
    )
{
    return (DDS_SampleStateMask)
	gapi_readCondition_get_sample_state_mask (
	    (gapi_readCondition)this
	);
}

/*     ViewStateMask
 *     get_view_state_mask();
 */
DDS_ViewStateMask
DDS_ReadCondition_get_view_state_mask (
    DDS_ReadCondition this
    )
{
    return (DDS_ViewStateMask)
	gapi_readCondition_get_view_state_mask (
	    (gapi_readCondition)this
	);
}

/*     InstanceStateMask
 *     get_instance_state_mask();
 */
DDS_InstanceStateMask
DDS_ReadCondition_get_instance_state_mask (
    DDS_ReadCondition this
    )
{
    return (DDS_InstanceStateMask)
	gapi_readCondition_get_instance_state_mask (
	    (gapi_readCondition)this
	);
}

/*     DataReader
 *     get_datareader();
 */
DDS_DataReader
DDS_ReadCondition_get_datareader (
    DDS_ReadCondition this
    )
{
    return (DDS_DataReader)
	gapi_readCondition_get_datareader (
	    (gapi_readCondition)this
	);
}

/*     DataReaderView
 *     get_datareaderview();
 */
DDS_DataReaderView
DDS_ReadCondition_get_datareaderview (
    DDS_ReadCondition this
    )
{
    return (DDS_DataReaderView)
	gapi_readCondition_get_datareaderview (
	    (gapi_readCondition)this
	);
}



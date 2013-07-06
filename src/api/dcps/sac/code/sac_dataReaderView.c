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

/*  From Entity
 *     get_statuscondition
 */
DDS_StatusCondition
DDS_DataReaderView_get_statuscondition(
    DDS_DataReaderView _this)
{
    return (DDS_StatusCondition)
        gapi_dataReaderView_get_statuscondition((gapi_dataReaderView)_this);
}

/*  From Entity
 *      DDS_DataReaderView_get_status_changes   
 */
DDS_StatusMask
DDS_DataReaderView_get_status_changes(
    DDS_DataReaderView _this)
{
    return (DDS_StatusMask)
        gapi_dataReaderView_get_status_changes((gapi_dataReaderView)_this);
}

/*     ReturnCode_t
 *     set_qos(
 *         in DataReaderViewQos qos);
 */
DDS_ReturnCode_t
DDS_DataReaderView_set_qos(
    DDS_DataReaderView this,
    const DDS_DataReaderViewQos *qos)
{
    return (DDS_ReturnCode_t)
        gapi_dataReaderView_set_qos(
            (gapi_dataReaderView)this,
            (const gapi_dataReaderViewQos *)qos);
}

/*     ReturnCode_t
 *     get_qos(
 *         inout DataReaderViewQos qos);
 */
DDS_ReturnCode_t
DDS_DataReaderView_get_qos(
    DDS_DataReaderView this,
    DDS_DataReaderViewQos *qos
    )
{
    return (DDS_ReturnCode_t)
        gapi_dataReaderView_get_qos(
           (gapi_dataReaderView)this,
           (gapi_dataReaderViewQos *)qos);
}

/*     DataReader
 *     get_datareader();
 */
DDS_DataReader
DDS_DataReaderView_get_datareader(
    DDS_DataReaderView this)
{
    return (DDS_DataReader)
        gapi_dataReaderView_get_datareader((gapi_dataReaderView)this);
}

/*     ReadCondition
 *     create_readcondition(
 *         in SampleStateMask sample_states,
 *         in ViewStateMask view_states,
 *         in InstanceStateMask instance_states);
 */
DDS_ReadCondition
DDS_DataReaderView_create_readcondition(
    DDS_DataReaderView _this,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states)
{
    return (DDS_ReadCondition)
        gapi_dataReaderView_create_readcondition(
            (gapi_dataReaderView)_this,
            (gapi_sampleStateMask)sample_states,
            (gapi_viewStateMask)view_states,
            (gapi_instanceStateMask)instance_states);
}

/*     QueryCondition
 *     create_querycondition(
 *         in SampleStateMask sample_states,
 *         in ViewStateMask view_states,
 *         in InstanceStateMask instance_states,
 *         in string query_expression,
 *         in StringSeq query_parameters);
 */
DDS_QueryCondition
DDS_DataReaderView_create_querycondition(
    DDS_DataReaderView _this,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states,
    const DDS_char *query_expression,
    const DDS_StringSeq *query_parameters)
{
    return (DDS_QueryCondition)
        gapi_dataReaderView_create_querycondition(
            (gapi_dataReaderView)_this,
            (gapi_sampleStateMask)sample_states,
            (gapi_viewStateMask)view_states,
            (gapi_instanceStateMask)instance_states,
            (gapi_char*)query_expression,
            (gapi_stringSeq*)query_parameters);
}

/*     ReturnCode_t
 *     delete_readcondition(
 *         in ReadCondition a_condition);
 */
DDS_ReturnCode_t
DDS_DataReaderView_delete_readcondition(
    DDS_DataReaderView _this,
    const DDS_ReadCondition a_condition)
{
    return (DDS_ReturnCode_t)
        gapi_dataReaderView_delete_readcondition(
            (gapi_dataReaderView)_this,
            (gapi_readCondition)a_condition);
}

/*     ReturnCode_t
 *     delete_contained_entities();
 */
DDS_ReturnCode_t
DDS_DataReaderView_delete_contained_entities(
    DDS_DataReaderView _this)
{
    return (DDS_ReturnCode_t)
        gapi_dataReaderView_delete_contained_entities(
            (gapi_dataReaderView)_this);
}

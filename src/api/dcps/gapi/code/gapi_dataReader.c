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
#include "gapi_dataReader.h"
#include "gapi_dataReaderView.h"
#include "gapi_qos.h"
#include "gapi_typeSupport.h"
#include "gapi_subscriber.h"
#include "gapi_topicDescription.h"
#include "gapi_contentFilteredTopic.h"
#include "gapi_topic.h"
#include "gapi_contentFilteredTopic.h"
#include "gapi_condition.h"
#include "gapi_domainParticipant.h"
#include "gapi_domainParticipantFactory.h"
#include "gapi_structured.h"
#include "gapi_objManag.h"
#include "gapi_kernel.h"
#include "gapi_genericCopyOut.h"
#include "gapi_expression.h"
#include "gapi_error.h"
#include "gapi_loanRegistry.h"

#include "os_heap.h"
#include "os_stdlib.h"
#include "u_user.h"
#include "v_kernel.h"
#include "v_state.h"
#include "v_event.h"
#include "v_public.h"
#include "kernelModule.h"
#include "v_topic.h"
#include "v_subscriber.h"
#include "v_dataReader.h"
#include "v_dataViewSample.h"

#define MAX_DATASAMPLESEQ_SIZE_ON_STACK  16

void
_DataReaderCopy (
    gapi_dataSampleSeq *samples,
    gapi_readerInfo *info)
{
    unsigned int i, len;
    gapi_fooSeq *data_seq = info->data_buffer;
    gapi_sampleInfoSeq *info_seq = info->info_buffer;
    void *dst;

    if (samples) {
        if (samples->_length > info->max_samples) {
            len = info->max_samples;
        } else {
            len = samples->_length;
        }
        if (len > 0) {
            if (data_seq->_buffer == NULL) {
                if (!info->copy_cache) {
                    data_seq->_buffer = info->alloc_buffer(
                                            len);
                } else {
                    data_seq->_buffer = gapi_copyOutAllocBuffer(
                                            info->copy_cache,
                                            len);
                }
                memset(data_seq->_buffer,0,info->alloc_size*len);
                data_seq->_maximum = len;
                data_seq->_release = FALSE;
                info_seq->_buffer  = gapi_sampleInfoSeq_allocbuf(len);
                info_seq->_maximum = len;
                info_seq->_release = FALSE;
                if (*info->loan_registry == NULL) {
                    *info->loan_registry = (void *)gapi_loanRegistry_new();
                }
                gapi_loanRegistry_register((gapi_loanRegistry)*info->loan_registry,
                                           data_seq->_buffer,
                                           info_seq->_buffer);
            }

            {
                C_STRUCT(gapi_dstInfo) dstInfo;
                dstInfo.copyProgram = info->copy_cache;
                for ( i = 0; i < len; i++ ) {
                    dst = &data_seq->_buffer[i*info->alloc_size];
                    if (info->copy_cache){
                        dstInfo.dst = dst;
                        dstInfo.buf = data_seq->_buffer;
                        info->copy_out (samples->_buffer[i].data, &dstInfo);
                    } else {
                        info->copy_out (samples->_buffer[i].data, dst);
                    }
                    info_seq->_buffer[i] = samples->_buffer[i].info;
                }
            }
        }
        data_seq->_length = len;
        info_seq->_length = len;
        info->num_samples = len;
    }
}

static void
getCopyInfo (
    v_entity e,
    c_voidp argument)
{
    v_topic kt;
    c_type sampleType;
    c_property messageAttr;
    _DataReader dataReader = (_DataReader)argument;

    sampleType = v_dataReaderSampleType(v_dataReader(e));
    kt = v_dataReaderGetTopic(v_dataReader(e));
    dataReader->userdataOffset = v_topicDataOffset(kt);
    c_free(kt);
    messageAttr = c_property(c_metaResolve(c_metaObject(sampleType),"message"));
    dataReader->messageOffset = messageAttr->offset;
    c_free(messageAttr);
}

c_bool
_DataReaderInit (
    _DataReader _this,
    const _Subscriber subscriber,
    const _TopicDescription topicDescription,
    const _TypeSupport typesupport,
    const struct gapi_dataReaderListener *a_listener,
    const gapi_statusMask mask,
    const u_dataReader uReader)
{
    c_bool noError;

    noError = ((_this != NULL) &&
               (topicDescription != NULL) &&
               (typesupport != NULL) &&
               (uReader != NULL));

    if (noError) {
        _EntityInit(_Entity(_this),
                          _Entity(subscriber));

        U_DATAREADER_SET(_this, uReader);
        u_entityAction(u_entity(uReader),getCopyInfo,_this);

        _this->topicDescription = topicDescription;

        if ( a_listener ) {
            _this->_Listener = *a_listener;
        } else {
            memset(&_this->_Listener, 0,
                   sizeof(_this->_Listener));
        }

        _this->readerCopy = _TypeSupportGetReaderCopy(typesupport);
        if (_this->readerCopy == NULL) {
            _this->readerCopy = _DataReaderCopy;
        }
        _this->copy_in = _TypeSupportCopyIn (typesupport);
        _this->copy_out = _TypeSupportCopyOut (typesupport);
        _this->copy_cache = _TypeSupportCopyCache (typesupport);
        _this->allocSize = _TypeSupportTopicAllocSize(typesupport);
        _this->allocBuffer = _TypeSupportTopicAllocBuffer(typesupport);

        _this->reader_mask.sampleStateMask   = (c_long)0;
        _this->reader_mask.viewStateMask     = (c_long)0;
        _this->reader_mask.instanceStateMask = (c_long)0;
    }
    if (noError) {
        _Status status;

        status = _StatusNew(_Entity(_this),
                            STATUS_KIND_DATAREADER,
                            (struct gapi_listener *)a_listener, mask);
        _EntityStatus(_this) = status;
        if (!status) {
            noError = FALSE;
        }
    }
    if (noError) {
        _TopicDescriptionIncUse(topicDescription);
    }
    return noError;
}

_DataReader
_DataReaderNew (
    const _TopicDescription topicDescription,
    const _TypeSupport typesupport,
    const gapi_dataReaderQos *qos,
    const struct gapi_dataReaderListener *a_listener,
    const gapi_statusMask mask,
    const _Subscriber subscriber)
{
    _DataReader _this;
    v_readerQos readerQos;
    u_dataReader uReader;
    gapi_string topicName;
    char dataReaderId[256];
    c_bool noError = TRUE;

    readerQos = u_readerQosNew(NULL);
    if ( readerQos != NULL ) {
        if ( gapi_kernelReaderQosCopyIn(qos, readerQos) ) {
            q_expr expr;
            c_value *params;

            topicName = _TopicDescriptionGetName (topicDescription);
            if (topicName) {
                snprintf (dataReaderId,
                          sizeof(dataReaderId),
                          "%sReader", topicName);
                gapi_free (topicName);
            } else {
                snprintf (dataReaderId,
                          sizeof(dataReaderId),
                          "dataReader");
            }
            expr = _TopicDescriptionGetExpr(topicDescription);
            if (_ObjectGetKind(_Object(topicDescription)) ==
                OBJECT_KIND_CONTENTFILTEREDTOPIC) {
                params = _ContentFilteredTopicParameters(
                             (_ContentFilteredTopic)topicDescription);
            } else {
                params = NULL;
            }
            uReader = u_dataReaderNew(_SubscriberUsubscriber(subscriber),
                                      dataReaderId,
                                      expr,
                                      params,
                                      readerQos,
                                      FALSE);
            q_dispose(expr);
            os_free(params);
            noError = (uReader != NULL);
            if (noError) {
                _this = _DataReaderAlloc();

                if ( _this != NULL ) {
                    noError = _DataReaderInit(_this,
                                              subscriber,
                                              topicDescription,
                                              typesupport,
                                              a_listener,
                                              mask,
                                              uReader);
                    if (!noError) {
                        _EntityDispose(_Entity(_this));
                    }
                }
                if (!noError) {
                    u_dataReaderFree(uReader);
                }
            }
        } else {
            noError = FALSE;
        }
        u_readerQosFree(readerQos);
    } else {
        noError = FALSE;
    }

    if (!noError) {
        _this = NULL;
    }

    return _this;
}

gapi_returnCode_t
_DataReaderFree (
    _DataReader _this)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _Status status;
    u_dataReader r;

    assert(_this);

    /* The following refCount checking and destruction mechanism is not
     * bullet proof and may cause leakage.
     * This is a temporary situation during GAPI redesign and should be
     * resolved when the GAPI redesign is finished.
     */
    r = U_DATAREADER_GET(_this);
    _TopicDescriptionDecUse(_this->topicDescription);

    status = _EntityStatus(_this);

    _StatusSetListener(status, NULL, 0);

    _EntityClaim(status);
    _StatusDeinit(status);

    gapi_loanRegistry_free(_this->loanRegistry);

    /* Following the user layer reader object is deleted after the entity
     * dispose because it will otherwise lead to a double free of the user
     * layer reader.
     * This is caused by the status condition which is attached to an exiting
     * waitset and the fact that a status condition's user object is the user
     * layer reader.
     * This is a hack but besides of that the destruction of the user entity
     * should be part of the entity dispose method.
     * For now this works.
     */
    _EntityDispose(_Entity(_this));
    u_dataReaderFree(r);

    return result;
}

gapi_boolean
_DataReaderPrepareDelete (
    _DataReader   _this,
    gapi_context *context)
{
    gapi_boolean result = TRUE;

    assert(_this);

    if (u_readerQueryCount(U_READER_GET(_this)) > 0) {
        gapi_errorReport(context, GAPI_ERRORCODE_CONTAINS_CONDITIONS);
        result = FALSE;
    }

    if (u_dataReaderViewCount(U_DATAREADER_GET(_this)) > 0) {
        gapi_errorReport(context, GAPI_ERRORCODE_CONTAINS_ENTITIES);
        result = FALSE;
    }

    if ( !gapi_loanRegistry_is_empty(_this->loanRegistry) ) {
        gapi_errorReport(context, GAPI_ERRORCODE_CONTAINS_LOANS);
        result = FALSE;
    }

    return result;
}

u_dataReader
_DataReaderUreader (
    _DataReader _this)
{
    return U_DATAREADER_GET(_this);
}

gapi_dataReaderQos *
_DataReaderGetQos (
    _DataReader dataReader,
    gapi_dataReaderQos *qos)
{
    v_readerQos dataReaderQos;
    u_dataReader uDataReader;
    u_result uResult;

    assert(dataReader);

    uDataReader = U_DATAREADER_GET(dataReader);
    uResult = u_entityQoS(u_entity(uDataReader), (v_qos *)&dataReaderQos);

    if ( uResult == U_RESULT_OK ) {
        gapi_kernelReaderQosCopyOut(dataReaderQos,  qos);
        u_readerQosFree(dataReaderQos);
    }

    return qos;
}

gapi_readCondition
gapi_dataReader_create_readcondition (
    gapi_dataReader _this,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states)
{
    _DataReader datareader;
    _ReadCondition readCondition = NULL;

    datareader = gapi_dataReaderClaim(_this, NULL);

    if ( datareader ) {
        if (_EntityEnabled(datareader) &&
            gapi_stateMasksValid(sample_states, view_states, instance_states) )
        {
            readCondition = _ReadConditionNew (sample_states,
                                               view_states,
                                               instance_states,
                                               datareader,
                                               NULL);
            if ( readCondition != NULL ) {
                gapi_deleteEntityAction deleteAction;
                void *actionArg;

                if ( _ObjectGetDeleteAction(_Object(readCondition),
                                            &deleteAction,
                                            &actionArg) ) {
                    _ObjectSetDeleteAction(_Object(readCondition),
                                           deleteAction,
                                           actionArg);
                }
                _ENTITY_REGISTER_OBJECT(_Entity(datareader), (_Object)readCondition);
            }
        }
        _EntityRelease(datareader);
    }

    return (gapi_readCondition)_EntityRelease(readCondition);
}

gapi_queryCondition
gapi_dataReader_create_querycondition (
    gapi_dataReader _this,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states,
    const gapi_char *query_expression,
    const gapi_stringSeq *query_parameters)
{
    _DataReader datareader;
    gapi_boolean licensed;
    _QueryCondition queryCondition = NULL;

    licensed = _DomainParticipantFactoryIsContentSubscriptionAvailable();

    if(licensed == TRUE){
        datareader = gapi_dataReaderClaim(_this, NULL);

        if ( datareader && _EntityEnabled(datareader) &&
             query_expression && gapi_sequence_is_valid(query_parameters) &&
             gapi_stateMasksValid(sample_states, view_states, instance_states) ) {

            queryCondition = _QueryConditionNew(sample_states,
                                                view_states,
                                                instance_states,
                                                query_expression,
                                                query_parameters,
                                                datareader, NULL);
            if ( queryCondition != NULL ) {
                _ENTITY_REGISTER_OBJECT(_Entity(datareader),
                                        (_Object)queryCondition);
            }
        }
        _EntityRelease(datareader);
    }
    return (gapi_queryCondition)_EntityRelease(queryCondition);
}

gapi_returnCode_t
gapi_dataReader_delete_readcondition (
    gapi_dataReader _this,
    const gapi_readCondition a_condition)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _DataReader datareader;
    _ReadCondition readCondition = NULL;
    c_bool contains;

    if (_this && a_condition) {
        datareader = gapi_dataReaderClaim(_this, &result);
        if (datareader != NULL) {
            readCondition = gapi_readConditionClaim(a_condition, NULL);
            if (readCondition != NULL ) {
                contains = u_readerContainsQuery(U_READER_GET(datareader),
                                                 U_QUERY_GET(readCondition));
                if (contains) {
                    _ReadConditionFree(readCondition);
                } else {
                    result = GAPI_RETCODE_PRECONDITION_NOT_MET;
                    _EntityRelease(readCondition);
                }
            } else {
                result = GAPI_RETCODE_ALREADY_DELETED;
            }
            _EntityRelease(datareader);
        } else {
            result = GAPI_RETCODE_ALREADY_DELETED;
        }
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }
    return result;
}

gapi_returnCode_t
gapi_dataReader_delete_contained_entities (
    gapi_dataReader _this)
{
    gapi_object handle;
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _DataReader datareader;
    gapi_context context;
    _Condition condition = NULL;
    _DataReaderView view = NULL;
    c_iter entities;
    u_entity e;
    u_result ur;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_DELETE_CONTAINED_ENTITIES);

    datareader = gapi_dataReaderClaim(_this, &result);

    if ( datareader != NULL ) {
        if (!gapi_loanRegistry_is_empty(datareader->loanRegistry)) {
            result = GAPI_RETCODE_PRECONDITION_NOT_MET;
        } else {
            entities = u_readerLookupQueries(U_READER_GET(datareader));
            e = c_iterTakeFirst(entities);
            while (e) {
                condition = u_entityGetUserData(e);
                if (condition) {
                    _ObjectReadClaimNotBusy(_Object(condition));
                    _ConditionFree(condition);
                } else {
                    assert(condition);
                    result = GAPI_RETCODE_BAD_PARAMETER;
                }
                e = c_iterTakeFirst(entities);
            }
            c_iterFree(entities);

            entities = u_dataReaderLookupViews(U_DATAREADER_GET(datareader));
            e = c_iterTakeFirst(entities);
            while (e) {
                handle = u_entityGetUserData(e);
                view = _DataReaderView(gapi_conditionClaimNB(handle,&result));
                if (view) {
                    _DataReaderViewFree(view);
                } else {
                    ur = u_dataViewFree(u_dataView(e));
                    if (ur == U_RESULT_OK) {
                        result = GAPI_RETCODE_OK;
                    } else {
                        result = GAPI_RETCODE_BAD_PARAMETER;
                    }
                }
                e = c_iterTakeFirst(entities);
            }
            c_iterFree(entities);
        }
        _EntityRelease(datareader);
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }
    return result;
}

gapi_dataReaderView
gapi_dataReader_create_view (
    gapi_dataReader _this,
    const gapi_dataReaderViewQos *qos)
{
    _DataReader datareader = NULL;
    _DataReaderView view = NULL;
    gapi_dataReaderViewQos *viewQos;
    gapi_context context;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_CREATE_VIEW);

    datareader = gapi_dataReaderClaim(_this, NULL);

    if ( datareader && _EntityEnabled(datareader)) {
        if ( qos == GAPI_DATAVIEW_QOS_DEFAULT ) {
            viewQos = (gapi_dataReaderViewQos *)&datareader->_defDataReaderViewQos;
        } else {
            viewQos = (gapi_dataReaderViewQos *)qos;
        }

        if (gapi_dataReaderViewQosIsConsistent(viewQos,&context) == GAPI_RETCODE_OK) {
            view = _DataReaderViewNew (viewQos, datareader);
            if ( view ) {
                _ENTITY_REGISTER_OBJECT(_Entity(datareader), (_Object)view);
            }
        }
    }

    _EntityRelease(datareader);

    return (gapi_dataReaderView)_EntityRelease(view);
}

gapi_returnCode_t
gapi_dataReader_delete_view (
    gapi_dataReader _this,
    gapi_dataReaderView a_view)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _DataReader datareader;
    _DataReaderView view = NULL;
    c_bool contains;

    gapi_context context;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_DELETE_DATAREADER);

    datareader = gapi_dataReaderClaim(_this, &result);

    if ( datareader != NULL ) {
        view = gapi_dataReaderViewClaim(a_view, NULL);
        if ( view != NULL ) {
            contains = u_dataReaderContainsView(U_DATAREADER_GET(datareader),
                                                U_DATAREADERVIEW_GET(view));
            if (contains) {
                if (_DataReaderViewPrepareDelete(view,&context)) {
                    _DataReaderViewFree(view);
                    view = NULL;
                } else {
                    result = GAPI_RETCODE_PRECONDITION_NOT_MET;
                }
            } else {
                result = GAPI_RETCODE_PRECONDITION_NOT_MET;
            }
            _EntityRelease(view);
        } else {
            result = GAPI_RETCODE_BAD_PARAMETER;
        }
        _EntityRelease(datareader);
    } else {
        result = GAPI_RETCODE_ALREADY_DELETED;
    }
    return result;
}

gapi_returnCode_t
gapi_dataReader_set_qos (
    gapi_dataReader _this,
    const gapi_dataReaderQos *qos)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    u_result uResult;
    _DataReader dataReader;
    v_readerQos dataReaderQos;
    gapi_context context;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_SET_QOS);

    dataReader = gapi_dataReaderClaim(_this, &result);

    if ( dataReader ) {
        if ( qos ) {
            result = gapi_dataReaderQosIsConsistent(qos, &context);
        } else {
            result = GAPI_RETCODE_BAD_PARAMETER;
        }
    }

    if (( result == GAPI_RETCODE_OK )  && (_EntityEnabled(dataReader))){
        gapi_dataReaderQos * existing_qos = gapi_dataReaderQos__alloc();

        result = gapi_dataReaderQosCheckMutability(qos,
                                                   _DataReaderGetQos(dataReader,
                                                                     existing_qos),
                                                   &context);
        gapi_free(existing_qos);
    }

    if ( result == GAPI_RETCODE_OK ) {
        dataReaderQos = u_readerQosNew(NULL);
        if (dataReaderQos) {
            if ( gapi_kernelReaderQosCopyIn(qos, dataReaderQos) ) {
                uResult = u_entitySetQoS(_EntityUEntity(dataReader),
                                         (v_qos)(dataReaderQos) );
                result = kernelResultToApiResult(uResult);
                u_readerQosFree(dataReaderQos);
            } else {
                result = GAPI_RETCODE_OUT_OF_RESOURCES;
            }
        } else {
            result = GAPI_RETCODE_OUT_OF_RESOURCES;
        }
    }

    _EntityRelease(dataReader);

    return result;
}

gapi_returnCode_t

gapi_dataReader_get_qos (
    gapi_dataReader _this,
    gapi_dataReaderQos *qos)
{
    _DataReader dataReader;
    gapi_returnCode_t result;

    dataReader = gapi_dataReaderClaim(_this, &result);
    if ( dataReader && qos ) {
        _DataReaderGetQos(dataReader, qos);
    }
    _EntityRelease(dataReader);
    return result;
}

gapi_returnCode_t
gapi_dataReader_set_listener (
    gapi_dataReader _this,
    const struct gapi_dataReaderListener *a_listener,
    const gapi_statusMask mask)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _DataReader datareader;

    datareader = gapi_dataReaderClaim(_this, &result);

    if ( datareader ) {
        _Status status;

        if ( a_listener ) {
            datareader->_Listener = *a_listener;
        } else {
            memset(&datareader->_Listener, 0, sizeof(datareader->_Listener));
        }

        status = _EntityStatus(datareader);
        if ( _StatusSetListener(status,
                                (struct gapi_listener *)a_listener,
                                mask) )
        {
            result = GAPI_RETCODE_OK;
        }
        _EntityRelease(datareader);
    }
    return result;
}

struct gapi_dataReaderListener
gapi_dataReader_get_listener (
    gapi_dataReader _this)
{
    struct gapi_dataReaderListener listener;
    _DataReader datareader;

    datareader = gapi_dataReaderClaim(_this, NULL);

    if ( datareader != NULL ) {
        listener = datareader->_Listener;
        _EntityRelease(datareader);
    } else {
        memset(&listener, 0, sizeof(listener));
    }
    return listener;
}


gapi_topicDescription
gapi_dataReader_get_topicdescription (
    gapi_dataReader _this)
{
    gapi_topicDescription topicDescription = NULL;
    _DataReader datareader;

    datareader = gapi_dataReaderClaim(_this, NULL);
    if ( datareader != NULL ) {
        topicDescription = (gapi_topicDescription)_EntityHandle(datareader->topicDescription);
        _EntityRelease(datareader);
    }
    return topicDescription;
}

gapi_subscriber
gapi_dataReader_get_subscriber (
    gapi_dataReader _this)
{
    gapi_subscriber subscriber = NULL;
    _DataReader datareader;
    u_subscriber uSubscriber;

    datareader = gapi_dataReaderClaim(_this, NULL);
    if ( datareader != NULL ) {
        uSubscriber = u_dataReaderSubscriber(U_DATAREADER_GET(datareader));
        subscriber = u_entityGetUserData(u_entity(uSubscriber));
        _EntityRelease(datareader);
    }
    return subscriber;
}

static v_result
copy_sample_rejected_status(
    c_voidp info,
    c_voidp arg)
{
    struct v_sampleRejectedInfo *from;
    gapi_sampleRejectedStatus *to;

    from = (struct v_sampleRejectedInfo *)info;
    to = (gapi_sampleRejectedStatus *)arg;

    to->total_count        = from->totalCount;
    to->total_count_change = from->totalChanged;
    switch ( from->lastReason ) {
        case S_NOT_REJECTED:
            to->last_reason = GAPI_NOT_REJECTED;
        break;
        case S_REJECTED_BY_INSTANCES_LIMIT:
            to->last_reason = GAPI_REJECTED_BY_INSTANCES_LIMIT;
        break;
        case S_REJECTED_BY_SAMPLES_LIMIT:
            to->last_reason = GAPI_REJECTED_BY_SAMPLES_LIMIT;
        break;
        case S_REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT:
            to->last_reason = GAPI_REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT;
        break;
    }
    to->last_instance_handle = (gapi_instanceHandle_t)0;

    return V_RESULT_OK;

}

static gapi_returnCode_t
_DataReader_get_sample_rejected_status (
    _DataReader _this,
    gapi_sampleRejectedStatus *status)
{
    u_result uResult;
    uResult = u_readerGetSampleRejectedStatus(
                  u_reader(U_DATAREADER_GET(_this)),
                  TRUE,
                  copy_sample_rejected_status,
                  status);
    return kernelResultToApiResult(uResult);
}

gapi_returnCode_t
gapi_dataReader_get_sample_rejected_status (
    gapi_dataReader _this,
    gapi_sampleRejectedStatus *status)
{
    gapi_returnCode_t result;
    _DataReader datareader;

    datareader = gapi_dataReaderClaim(_this, &result);

    if (datareader != NULL) {
        if (_EntityEnabled(datareader)) {
            result = _DataReader_get_sample_rejected_status (
                         datareader,
                         status);
        } else {
            result=GAPI_RETCODE_NOT_ENABLED;
        }
        _EntityRelease(datareader);
    }

    return result;
}

static v_result
copy_liveliness_changed_status(
    c_voidp info,
    c_voidp arg)
{
    struct v_livelinessChangedInfo *from;
    gapi_livelinessChangedStatus *to;

    from = (struct v_livelinessChangedInfo *)info;
    to = (gapi_livelinessChangedStatus *)arg;

    to->alive_count = from->activeCount;
    to->not_alive_count = from->inactiveCount;
    to->alive_count_change = from->activeChanged;
    to->not_alive_count_change = from->inactiveChanged;

    to->last_publication_handle = u_instanceHandleFromGID(from->instanceHandle);

    return V_RESULT_OK;
}

static gapi_returnCode_t
_DataReader_get_liveliness_changed_status (
    _DataReader _this,
    gapi_livelinessChangedStatus *status)
{
    u_result uResult;
    uResult = u_readerGetLivelinessChangedStatus(
                  u_reader(U_DATAREADER_GET(_this)),
                  TRUE,
                  copy_liveliness_changed_status,
                  status);
    return kernelResultToApiResult(uResult);
}

gapi_returnCode_t
gapi_dataReader_get_liveliness_changed_status (
    gapi_dataReader _this,
    gapi_livelinessChangedStatus *status)
{
    gapi_returnCode_t result;
    _DataReader datareader;

    datareader = gapi_dataReaderClaim(_this, &result);

    if (datareader != NULL) {
        if (_EntityEnabled(datareader)) {
            result = _DataReader_get_liveliness_changed_status (
                         datareader,
                         status);
        } else {
            result=GAPI_RETCODE_NOT_ENABLED;
        }
        _EntityRelease(datareader);
    }
    return result;
}

static v_result
copy_deadline_missed_status(
    c_voidp info,
    c_voidp arg)
{
    struct v_deadlineMissedInfo *from;
    gapi_requestedDeadlineMissedStatus *to;
    v_handleResult result;
    v_object instance;

    from = (struct v_deadlineMissedInfo *)info;
    to = (gapi_requestedDeadlineMissedStatus *)arg;

    to->total_count = from->totalCount;
    to->total_count_change = from->totalChanged;

    result = v_handleClaim(from->instanceHandle, &instance);
    if (result == V_HANDLE_OK) {
        to->last_instance_handle = u_instanceHandleNew(v_public(instance));
        result = v_handleRelease(from->instanceHandle);
    }
    return V_RESULT_OK;
}

static gapi_returnCode_t
_DataReader_get_requested_deadline_missed_status (
    _DataReader _this,
    gapi_requestedDeadlineMissedStatus *status)
{
    u_result uResult;
    uResult = u_readerGetDeadlineMissedStatus(
                  u_reader(U_DATAREADER_GET(_this)),
                  TRUE,
                  copy_deadline_missed_status,
                  status);
    return kernelResultToApiResult(uResult);
}

gapi_returnCode_t
gapi_dataReader_get_requested_deadline_missed_status (
    gapi_dataReader _this,
    gapi_requestedDeadlineMissedStatus *status)
{
    gapi_returnCode_t result;
    _DataReader datareader;

    datareader = gapi_dataReaderClaim(_this, &result);

    if (datareader != NULL) {
        if (_EntityEnabled(datareader)) {
            result = _DataReader_get_requested_deadline_missed_status(
                         datareader,
                         status);
        } else {
            result=GAPI_RETCODE_NOT_ENABLED;
        }
        _EntityRelease(datareader);
    }
    return result;
}

static v_result
copy_incompatible_qos_status(
    c_voidp info,
    c_voidp arg)
{
    unsigned long i;
    unsigned long len;
    v_result result = V_RESULT_PRECONDITION_NOT_MET;
    struct v_incompatibleQosInfo *from;
    gapi_requestedIncompatibleQosStatus *to;

    from = (struct v_incompatibleQosInfo *)info;
    to = (gapi_requestedIncompatibleQosStatus *)arg;

    to->total_count = from->totalCount;
    to->total_count_change = from->totalChanged;
    to->last_policy_id = from->lastPolicyId;
    len = c_arraySize(from->policyCount);

    if ( to->policies._buffer && (len <= to->policies._maximum) ) {
        to->policies._length = len;
        for ( i = 0; i < len; i++ ) {
            to->policies._buffer[i].policy_id = i;
            to->policies._buffer[i].count = ((c_long *)from->policyCount)[i];
        }
        result = V_RESULT_OK;
    }
    return result;
}

static gapi_returnCode_t
_DataReader_get_requested_incompatible_qos_status (
    _DataReader _this,
    gapi_requestedIncompatibleQosStatus *status)
{
    u_result uResult;
    uResult = u_readerGetIncompatibleQosStatus(
                  u_reader(U_DATAREADER_GET(_this)),
                  TRUE,
                  copy_incompatible_qos_status,
                  status);
    return kernelResultToApiResult(uResult);
}

gapi_returnCode_t
gapi_dataReader_get_requested_incompatible_qos_status (
    gapi_dataReader _this,
    gapi_requestedIncompatibleQosStatus *status)
{
    gapi_returnCode_t result;
    _DataReader datareader;

    datareader = gapi_dataReaderClaim(_this, &result);

    if (datareader != NULL) {
        if (_EntityEnabled(datareader)) {
            result = _DataReader_get_requested_incompatible_qos_status(
                          datareader,
                          status);
        } else {
            result=GAPI_RETCODE_NOT_ENABLED;
        }
        _EntityRelease(datareader);
    }
    return result;
}

static v_result
copy_subscription_matched_status(
    c_voidp info,
    c_voidp arg)
{
    struct v_topicMatchInfo *from;
    gapi_subscriptionMatchedStatus *to;

    from = (struct v_topicMatchInfo *)info;
    to = (gapi_subscriptionMatchedStatus *)arg;

    to->total_count = from->totalCount;
    to->total_count_change = from->totalChanged;
    to->current_count = from->currentCount;
    to->current_count_change = from->currentChanged;
    to->last_publication_handle = u_instanceHandleFromGID(from->instanceHandle);
    return V_RESULT_OK;
}

static gapi_returnCode_t
_DataReader_get_subscription_matched_status (
    _DataReader _this,
    gapi_subscriptionMatchedStatus *status)
{
    u_result uResult;
    uResult = u_readerGetSubscriptionMatchStatus(
                  u_reader(U_DATAREADER_GET(_this)),
                  TRUE,
                  copy_subscription_matched_status,
                  status);
    return kernelResultToApiResult(uResult);
}

gapi_returnCode_t
gapi_dataReader_get_subscription_matched_status (
    gapi_dataReader _this,
    gapi_subscriptionMatchedStatus *status)
{
    gapi_returnCode_t result;
    _DataReader datareader;

    datareader = gapi_dataReaderClaim(_this, &result);

    if (datareader != NULL) {
        if (_EntityEnabled(datareader)) {
            result = _DataReader_get_subscription_matched_status (
                         datareader,
                         status);
        } else {
            result=GAPI_RETCODE_NOT_ENABLED;
        }
        _EntityRelease(datareader);
    }
    return result;
}

static v_result
copy_sample_lost_status(
    c_voidp info,
    c_voidp arg)
{
    struct v_sampleLostInfo *from;
    gapi_sampleLostStatus *to;

    from = (struct v_sampleLostInfo *)info;
    to = (gapi_sampleLostStatus *)arg;

    to->total_count = from->totalCount;
    to->total_count_change = from->totalChanged;

    return V_RESULT_OK;
}

static gapi_returnCode_t
_DataReader_get_sample_lost_status (
    _DataReader _this,
    gapi_sampleLostStatus *status)
{
    u_result uResult;
    uResult = u_readerGetSampleLostStatus(
                  u_reader(U_DATAREADER_GET(_this)),
                  TRUE,
                  copy_sample_lost_status,
                  status);
    return kernelResultToApiResult(uResult);
}

gapi_returnCode_t
gapi_dataReader_get_sample_lost_status (
    gapi_dataReader _this,
    gapi_sampleLostStatus *status)
{
    gapi_returnCode_t result;
    _DataReader datareader;

    datareader = gapi_dataReaderClaim(_this, &result);

    if (datareader != NULL) {
        if (_EntityEnabled(datareader)) {
            result = _DataReader_get_sample_lost_status (
                          datareader,
                          status);
        } else {
            result=GAPI_RETCODE_NOT_ENABLED;
        }
        _EntityRelease(datareader);
    }
    return result;
}

gapi_returnCode_t
gapi_dataReader_wait_for_historical_data (
    gapi_dataReader _this,
    const gapi_duration_t *max_wait)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _DataReader datareader;
    c_time  c_time_max_wait;
    u_result  uResult;

    datareader = gapi_dataReaderClaim(_this, &result);

    if (datareader) {
        if ( !max_wait || !gapi_validDuration(max_wait)) {
            result = GAPI_RETCODE_BAD_PARAMETER;
        } else if (!_EntityEnabled(datareader)) {
            result = GAPI_RETCODE_NOT_ENABLED;
        } else {
            kernelCopyInDuration(max_wait, &c_time_max_wait);

            uResult = u_dataReaderWaitForHistoricalData(
                          U_DATAREADER_GET(datareader),
                          c_time_max_wait);
            result = kernelResultToApiResult(uResult);
        }
        _EntityRelease(datareader);
    }
    return result;
}

gapi_returnCode_t
gapi_dataReader_wait_for_historical_data_w_condition (
    gapi_dataReader _this,
    const gapi_char *filter_expression,
    const gapi_stringSeq *filter_parameters,
    const gapi_time_t *min_source_timestamp,
    const gapi_time_t *max_source_timestamp,
    const gapi_resourceLimitsQosPolicy *resource_limits,
    const gapi_duration_t *max_wait)
{
    gapi_returnCode_t result;
    _DataReader datareader;
    c_time  c_time_max_wait,
            c_time_min_source_timestamp,
            c_time_max_source_timestamp;
    u_result  uResult;
    c_ulong length, i;
    c_char** params;
    struct v_resourcePolicy resource;

    datareader = gapi_dataReaderClaim(_this, &result);

    if (datareader) {
        if ( !max_wait || !gapi_validDuration(max_wait)) {
            result = GAPI_RETCODE_BAD_PARAMETER;
        } else if (!_EntityEnabled(datareader)) {
            result = GAPI_RETCODE_NOT_ENABLED;
        } else if(filter_parameters && !gapi_stringSeqValid(filter_parameters)){
            result = GAPI_RETCODE_BAD_PARAMETER;
        } else {
            if(filter_parameters){
                length = filter_parameters->_length;
                params = (c_char**)(os_malloc(length*sizeof(c_char*)));

                for(i=0; i<length; i++){
                    params[i] = filter_parameters->_buffer[i];
                }
            } else {
                params = NULL;
                length = 0;
            }
            kernelCopyInDuration(max_wait, &c_time_max_wait);
            if ( (kernelCopyInTime(min_source_timestamp, &c_time_min_source_timestamp) != GAPI_RETCODE_OK) ||
                 (kernelCopyInTime(max_source_timestamp, &c_time_max_source_timestamp) != GAPI_RETCODE_OK) ) {
                result = GAPI_RETCODE_BAD_PARAMETER;
            } else {
                resource.max_samples = resource_limits->max_samples;
                resource.max_instances = resource_limits->max_instances;
                resource.max_samples_per_instance = resource_limits->max_samples_per_instance;

                uResult= u_dataReaderWaitForHistoricalDataWithCondition(
                        U_DATAREADER_GET(datareader),
                        (c_char*)filter_expression,
                        params,
                        length,
                        c_time_min_source_timestamp,
                        c_time_max_source_timestamp,
                        &resource,
                        c_time_max_wait);

                result = kernelResultToApiResult(uResult);
            }
        }
        _EntityRelease(datareader);
    }
    return result;
}


static v_result
copy_matched_publication(
    c_voidp info,
    c_voidp arg)
{
        struct v_publicationInfo *publicationInfo;
        gapi_instanceHandleSeq *to;
    gapi_instanceHandle_t *tmp_buffer;

    publicationInfo = (struct v_publicationInfo*)info;
    to = (gapi_instanceHandleSeq *)arg;

    if (to->_maximum <= to->_length) {
        tmp_buffer = to->_buffer;
        to->_buffer = gapi_instanceHandleSeq_allocbuf(to->_length + 10);
        to->_maximum = to->_length + 10;
        memcpy(to->_buffer, tmp_buffer, to->_length);
        gapi_free(tmp_buffer);
    }

    to->_buffer[to->_length] = u_instanceHandleFromGID(publicationInfo->key);
    ++to->_length;
    return V_RESULT_OK;
}

static gapi_returnCode_t
_DataReader_get_matched_publications (
    _DataReader _this,
    gapi_instanceHandleSeq *publication_handles)
{
    u_result uResult;
    uResult = u_readerGetMatchedPublications(
                  U_READER_GET(_this),
                  copy_matched_publication,
                  publication_handles);
    return kernelResultToApiResult(uResult);
}

static gapi_returnCode_t
_DataReader_set_notread_threshold (
    _DataReader _this,
    gapi_long threshold)
{
    u_result uResult;
    uResult = u_dataReaderSetNotReadThreshold(
                  U_READER_GET(_this),
                  threshold);
    return kernelResultToApiResult(uResult);
}

gapi_returnCode_t
gapi_dataReader_get_matched_publications (
    gapi_dataReader _this,
    gapi_instanceHandleSeq *publication_handles)
{
    gapi_returnCode_t result;
    _DataReader datareader;

    datareader = gapi_dataReaderClaim(_this, &result);
    if (datareader != NULL) {
        if (_EntityEnabled(datareader)) {
            result = _DataReader_get_matched_publications (
                          datareader,
                          publication_handles);
        } else {
            result=GAPI_RETCODE_NOT_ENABLED;
        }
    }

    _EntityRelease(datareader);

    return result;
}

static gapi_returnCode_t
_DataReader_get_matched_publication_data (
    _DataReader _this,
    gapi_publicationBuiltinTopicData *publication_data,
    const gapi_instanceHandle_t publication_handle)
{
    u_result uResult;
    uResult = u_readerGetMatchedPublicationData(
                  U_READER_GET(_this),
                  publication_handle,
                  gapi_publicationBuiltinTopicData__copyOut,
                  publication_data);
    return kernelResultToApiResult(uResult);
}

gapi_returnCode_t
gapi_dataReader_get_matched_publication_data (
    gapi_dataReader _this,
    gapi_publicationBuiltinTopicData *publication_data,
    const gapi_instanceHandle_t publication_handle)
{
    gapi_returnCode_t result;
    _DataReader datareader;

    datareader = gapi_dataReaderClaim(_this, &result);
    if (datareader != NULL) {
        if (_EntityEnabled(datareader)) {
            result = _DataReader_get_matched_publication_data (
                          datareader,
                          publication_data,
                          publication_handle);
        } else {
            result=GAPI_RETCODE_NOT_ENABLED;
        }
    }

    _EntityRelease(datareader);

    return result;
}

void
_DataReaderTriggerNotify (
    _DataReader _this)
{
    _Status status;
    gapi_listener_DataAvailableListener callback;
    void *listenerData;
    gapi_object handle;

    assert(_this);

    status = _Entity(_this)->status;
    callback     = status->callbackInfo.on_data_available;
    listenerData = status->callbackInfo.listenerData;

    if ( callback && listenerData ) {
        if (u_dataReaderDataAvailableTest(U_DATAREADER_GET(_this))) {
            handle = _EntityHandle(_this);
            _EntitySetBusy(_this);
            _EntityRelease(_this);
            callback(listenerData, handle);
            gapi_objectClearBusy(handle);
            (void)gapi_dataReaderClaim(handle, NULL);
        }
    }
}

gapi_returnCode_t
gapi_dataReader_set_default_datareaderview_qos (
    gapi_dataReader _this,
    const gapi_dataReaderViewQos *qos)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _DataReader dataReader = (_DataReader)_this;
    gapi_context        context;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_SET_DEFAULT_DATAREADERVIEW_QOS);

    dataReader = gapi_dataReaderClaim(_this, &result);

    if ( dataReader ) {
        if ( qos ) {
            result = gapi_dataReaderViewQosIsConsistent(qos, &context);
            if ( result == GAPI_RETCODE_OK ) {
                gapi_dataReaderViewQosCopy (qos, &dataReader->_defDataReaderViewQos);
            }
        } else {
            result = GAPI_RETCODE_BAD_PARAMETER;
        }
        _EntityRelease(dataReader);
    }
    return result;
}

gapi_returnCode_t
gapi_dataReader_get_default_datareaderview_qos (
    gapi_dataReader _this,
    gapi_dataReaderViewQos *qos)
{
    _DataReader datareader;
    gapi_returnCode_t result;

    datareader = gapi_dataReaderClaim(_this, &result);

    if ( datareader) {
        if ( qos ) {
            gapi_dataReaderViewQosCopy (&datareader->_defDataReaderViewQos, qos);
        }
        _EntityRelease(datareader);
    }
    return result;
}

gapi_returnCode_t
gapi_dataReader_set_notread_threshold (
    gapi_dataReader _this,
    gapi_long threshold)
{
    _DataReader datareader;
    gapi_returnCode_t result;

    datareader = gapi_dataReaderClaim(_this, &result);
    if (datareader != NULL) {
        result = _DataReader_set_notread_threshold (
                datareader,
                threshold);
    }

    _EntityRelease(datareader);
    return result;
}

static void
checkDataAvailability(
    v_entity e,
    c_voidp arg)
{
    c_bool *dataAvailable = (c_bool *)arg;

    *dataAvailable = ((v_statusGetMask(e->status) & V_EVENT_DATA_AVAILABLE) != 0);
}

static void
resetDataAvailable(
   v_entity e,
   c_voidp arg)
{
    v_statusReset(e->status, V_EVENT_DATA_AVAILABLE);
}

gapi_returnCode_t
_DataReaderGetKeyValue (
    _DataReader _this,
    void        *instance,
    const gapi_instanceHandle_t handle)
{
    gapi_returnCode_t result;
    u_dataReader reader;
    u_result uResult;

    PREPEND_COPYOUTCACHE(_this->copy_cache, instance, NULL);

    reader = U_DATAREADER_GET(_this);
    uResult = u_dataReaderCopyKeysFromInstanceHandle(reader,
                    (u_instanceHandle)handle,
                    (u_readerAction)_this->copy_out,
                    instance);
    result = kernelResultToApiResult(uResult);

    /* The OpenSplice user-layer may have detected that the instance has been deleted (expired),
     * but the DDS Spec. requires a PRECONDITION_NOT_MET result code if the instance handle is not registered
     */
    if (result == GAPI_RETCODE_ALREADY_DELETED) {
        result = GAPI_RETCODE_PRECONDITION_NOT_MET;
    }

    REMOVE_COPYOUTCACHE(_this->copy_cache, instance);

    return result;
}

void
_DataReaderNotifyListener(
    _DataReader _this,
    gapi_statusMask triggerMask)
{
    _Status status;
    gapi_object source;
    gapi_returnCode_t result;

    if (_this == NULL) {
        OS_REPORT(OS_ERROR,
                  "_DataReaderNotifyListener",0,
                  "Specified DataReader = NULL.");
        return;
    }
    status = _EntityStatus(_this);
    source = _EntityHandle(_this);

    while ( _this && (triggerMask != GAPI_STATUS_KIND_NULL) ) {
        if ( triggerMask & GAPI_DATA_AVAILABLE_STATUS ) {
            /* The behaviour for the triggering of data_on_readers and
             * data_available is described in the DDS specification:
             * first, the middleware tries to trigger the SubscriberListener
             * operation on_data_on_readers with a parameter of the related
             * Subscriber;
             * if this does not succeed (no listener or operation non-enabled),
             * it tries to trigger on_data_available on all the related
             * DataReaderListener objects, with as parameter the related DataReader.
             * This is implemented by the following if else block.
             */
            if (!_StatusNotifyDataOnReaders(status, source)) {
                _StatusNotifyDataAvailable(status, source);
            }
            triggerMask &= ~GAPI_DATA_AVAILABLE_STATUS;
        }
        if ( triggerMask & GAPI_SAMPLE_REJECTED_STATUS ) {
            gapi_sampleRejectedStatus info;

            result = _DataReader_get_sample_rejected_status(_this, &info);
            /* Only allow the callback if there is a change since the last
             * callback, i.e if total_count_change is non zero
             */
            if (result == GAPI_RETCODE_OK && info.total_count_change != 0) {
                _StatusNotifySampleRejected(status, source, &info);
            }
            triggerMask &= ~GAPI_SAMPLE_REJECTED_STATUS;
        }
        if ( triggerMask & GAPI_LIVELINESS_CHANGED_STATUS ) {
            gapi_livelinessChangedStatus info;

            result = _DataReader_get_liveliness_changed_status(_this, &info);
            /* Only allow the callback if there is a change since the last
             * callback, i.e if either alive_count_change or
             * not_alive_count_change are non zero.
             */
            if (result == GAPI_RETCODE_OK &&
                (info.alive_count_change != 0 ||
                 info.not_alive_count_change != 0))
            {
                _StatusNotifyLivelinessChanged(status, source, &info);
            }
            triggerMask &= ~GAPI_LIVELINESS_CHANGED_STATUS;
        }
        if ( triggerMask & GAPI_REQUESTED_DEADLINE_MISSED_STATUS ) {
            gapi_requestedDeadlineMissedStatus info;

            result = _DataReader_get_requested_deadline_missed_status(_this, &info);
            /* Only allow the callback if there is a change since the last
             * callback, i.e if total_count_change is non zero
             */
            if (result == GAPI_RETCODE_OK && info.total_count_change != 0) {
                _StatusNotifyRequestedDeadlineMissed(status, source, &info);
            }
            triggerMask &= ~GAPI_REQUESTED_DEADLINE_MISSED_STATUS;
        }
        if ( triggerMask & GAPI_REQUESTED_INCOMPATIBLE_QOS_STATUS ) {
            gapi_requestedIncompatibleQosStatus info;
            gapi_qosPolicyCount policyCount[MAX_POLICY_COUNT_ID];

            info.policies._maximum = MAX_POLICY_COUNT_ID;
            info.policies._length  = 0;
            info.policies._buffer  = policyCount;

            result = _DataReader_get_requested_incompatible_qos_status(_this, &info);
            /* Only allow the callback if there is a change since the last
             * callback, i.e if total_count_change is non zero
             */
            if (result == GAPI_RETCODE_OK && info.total_count_change != 0) {
                _StatusNotifyRequestedIncompatibleQos(status, source, &info);
            }
            triggerMask &= ~GAPI_REQUESTED_INCOMPATIBLE_QOS_STATUS;
        }
        if ( triggerMask & GAPI_SAMPLE_LOST_STATUS ) {
            gapi_sampleLostStatus info;

            result = _DataReader_get_sample_lost_status (_this, &info);
            /* Only allow the callback if there is a change since the last
             * callback, i.e if total_count_change is non zero
             */
            if (result == GAPI_RETCODE_OK && info.total_count_change != 0) {
                _StatusNotifySampleLost(status, source, &info);
            }
            triggerMask &= ~GAPI_SAMPLE_LOST_STATUS;
        }
        if ( triggerMask & GAPI_SUBSCRIPTION_MATCH_STATUS ) {
            gapi_subscriptionMatchedStatus info;

            result = _DataReader_get_subscription_matched_status (_this, &info);
            /* Only allow the callback if there is a change since the last
             * callback, i.e if total_count_change is non zero
             */
            if (result == GAPI_RETCODE_OK && info.current_count_change != 0) {
                _StatusNotifySubscriptionMatch(status, source, &info);
            }
            triggerMask &= ~GAPI_SUBSCRIPTION_MATCH_STATUS;
        }
    }
}


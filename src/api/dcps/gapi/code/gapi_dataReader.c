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
#include "gapi_dataReaderStatus.h"
#include "gapi_structured.h"
#include "gapi_objManag.h"
#include "gapi_kernel.h"
#include "gapi_genericCopyOut.h"
#include "gapi_expression.h"
#include "gapi_error.h"
#include "gapi_instanceHandle.h"
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
    const u_dataReader uReader,
    const c_bool enable)
{
    _DomainParticipant participant;
    gapi_expression expr;
    c_bool noError;

    noError = ((_this != NULL) &&
               (topicDescription != NULL) &&
               (typesupport != NULL) &&
               (uReader != NULL));

    if (noError) {
        participant = _DomainEntityParticipant(_DomainEntity(subscriber));
        noError = (participant != NULL);
    }

    if (noError) {
        _DomainEntityInit(_DomainEntity(_this),
                          participant,
                          _Entity(subscriber),
                          enable);

        U_DATAREADER_SET(_this, uReader);
        u_entityAction(u_entity(uReader),getCopyInfo,_this);

        _this->subscriber = subscriber;
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

        expr = gapi_createReadExpression(u_entity(uReader),
                                         &_this->reader_mask);
        noError = (expr != NULL);

        if (noError) {
            _this->uQuery = gapi_expressionCreateQuery(expr,
                                                       u_reader(uReader),
                                                       NULL,
                                                       NULL);
            gapi_expressionFree(expr);
            noError = (_this->uQuery != NULL);
        }
    }
    if (noError) {
        _EntityStatus(_this) = _Status(_DataReaderStatusNew(_this,
                                                            a_listener,
                                                            mask));
        noError = (_EntityStatus(_this) != NULL);
    }
    if (noError) {
        _this->conditionSet = gapi_setNew (gapi_objectRefCompare);
        noError = (_this->conditionSet != NULL);
    }
    if (noError) {
        _this->viewSet = gapi_setNew (gapi_objectRefCompare);
        noError = (_this->viewSet != NULL);
    }
    if (noError) {
        _TopicDescriptionIncUse(topicDescription);
    } else {
        gapi_setFree(_this->conditionSet);
        gapi_setFree(_this->viewSet);
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
    u_subscriber uSubscriber;
    u_result uResult;
    v_subscriberQos subscriberQos;
    gapi_string topicName;
    char dataReaderId[256];
    c_bool enable = TRUE;
    c_bool noError = TRUE;

    /* The following code copies the Qos from the subscriber just to 
     * retrieve the auto enable value.
     * Is it really necessary to have it on this level or can it also
     * be handled in the user layer internally?
     * If it is required at this level then a dedicated get enable value
     * would be more efficient than copying the whole qos and freeing it again.
     * In addition if the retrieval of the qos fails the enable is set to
     * TRUE by default without warning, this seems incorrect.
     */
    if (noError) {
        uSubscriber = U_SUBSCRIBER_GET(subscriber);
        uResult = u_entityQoS(u_entity(uSubscriber), (v_qos*)&subscriberQos);
        if ( uResult == U_RESULT_OK ) {
            enable = subscriberQos->entityFactory.autoenable_created_entities;
            u_subscriberQosFree(subscriberQos);
        }
    }

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
                                      enable);
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
                                              uReader,
                                              enable);
                    if (!noError) {
                        _DomainEntityDispose(_DomainEntity(_this));
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

void
_DataReaderFree (
    _DataReader _this)
{
    _DataReaderStatus status;

    assert(_this);

    _TopicDescriptionDecUse(_this->topicDescription);

    status = _DataReaderStatus(_Entity(_this)->status);

    _DataReaderStatusSetListener(status, NULL, 0);

    _DataReaderStatusFree(status);

    _EntityFreeStatusCondition(_Entity(_this));

    u_queryFree(_this->uQuery);
    u_dataReaderFree(U_DATAREADER_GET(_this));

    gapi_setFree(_this->conditionSet);
    _this->conditionSet = NULL;

    gapi_setFree(_this->viewSet);
    _this->viewSet = NULL;

    gapi_loanRegistry_free(_this->loanRegistry);

    _DomainEntityDispose (_DomainEntity(_this));
}

gapi_boolean
_DataReaderPrepareDelete (
    _DataReader   _this,
    gapi_context *context)
{
    gapi_boolean result = TRUE;

    assert(_this);

    if ( !gapi_setIsEmpty(_this->conditionSet) ) {
        gapi_errorReport(context, GAPI_ERRORCODE_CONTAINS_CONDITIONS);
        result = FALSE;
    }

    if ( !gapi_setIsEmpty(_this->viewSet) ) {
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

_Subscriber
_DataReaderSubscriber (
    _DataReader _this)
{
    return _this->subscriber;
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
    uResult = u_entityQoS(u_entity(uDataReader), (v_qos*)&dataReaderQos);

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

    if ( datareader && _Entity(datareader)->enabled &&
        gapi_stateMasksValid(sample_states, view_states, instance_states) ) {
        readCondition = _ReadConditionNew (sample_states,
                                           view_states,
                                           instance_states,
                                           datareader,
                                           NULL);
        if ( readCondition != NULL ) {
            gapi_deleteEntityAction deleteAction;
            void *actionArg;

            gapi_setAdd(datareader->conditionSet, (gapi_object)readCondition);

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

        if ( datareader && _Entity(datareader)->enabled &&
             query_expression && gapi_sequence_is_valid(query_parameters) &&
             gapi_stateMasksValid(sample_states, view_states, instance_states) ) {

            queryCondition = _QueryConditionNew(sample_states,
                                                view_states,
                                                instance_states,
                                                query_expression,
                                                query_parameters,
                                                datareader, NULL);
            if ( queryCondition != NULL ) {
                gapi_setAdd(datareader->conditionSet,
                            (gapi_object)queryCondition);
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

    datareader = gapi_dataReaderClaim(_this, &result);

    if ( datareader != NULL ) {
        readCondition = gapi_readConditionClaim(a_condition, NULL);
        if ( readCondition != NULL ) {
            gapi_setIter iterSet;

            iterSet = gapi_setFind (datareader->conditionSet,
                                    (gapi_object)readCondition);
            if ( gapi_setIterObject(iterSet) != NULL ) {
                gapi_setRemove(datareader->conditionSet,
                               (gapi_object)readCondition);
                _ReadConditionFree(readCondition);
            } else {
                result = GAPI_RETCODE_PRECONDITION_NOT_MET;
                _EntityRelease(readCondition);
            }
            gapi_setIterFree(iterSet);
        } else {
            result = GAPI_RETCODE_BAD_PARAMETER;
        }
    }

    _EntityRelease(datareader);

    return result;
}

gapi_returnCode_t
gapi_dataReader_delete_contained_entities (
    gapi_dataReader _this,
    gapi_deleteEntityAction action,
    void *action_arg)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _DataReader datareader;
    void *userData;
    gapi_context context;
    gapi_setIter iterSet;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_DELETE_CONTAINED_ENTITIES);

    datareader = gapi_dataReaderClaim(_this, &result);

    if ( datareader != NULL ) {
        if (!gapi_loanRegistry_is_empty(datareader->loanRegistry)) {
            result = GAPI_RETCODE_PRECONDITION_NOT_MET;
        }
        iterSet = gapi_setFirst(datareader->conditionSet);

        while ((gapi_setIterObject(iterSet)) && (result == GAPI_RETCODE_OK)) {
            _ReadCondition readCondition = (_ReadCondition)gapi_setIterObject(iterSet);
            _EntityClaim(readCondition);
            userData = _ObjectGetUserData(_Object(readCondition));
            _ReadConditionPrepareDelete(readCondition);
            _ReadConditionFree(readCondition);
            gapi_setIterRemove(iterSet);
            if ( action ) {
                action(userData, action_arg);
            }
        }
        gapi_setIterFree (iterSet);
        iterSet = gapi_setFirst(datareader->viewSet);

        while ((gapi_setIterObject(iterSet)) && (result == GAPI_RETCODE_OK)) {            
            _DataReaderView view = (_DataReaderView)gapi_setIterObject(iterSet);
            _EntityClaim(view);
            _DataReaderViewPrepareDelete(view, &context);
            _DataReaderViewFree(view);
            gapi_setIterRemove(iterSet);
        }
        gapi_setIterFree (iterSet);
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

    if ( datareader && _Entity(datareader)->enabled ) {
        if ( qos == GAPI_DATAVIEW_QOS_DEFAULT ) {
            viewQos = (gapi_dataReaderViewQos *)&gapi_dataReaderViewQosDefault;
        } else {
            viewQos = (gapi_dataReaderViewQos *)qos;
        }

        if (gapi_dataReaderViewQosIsConsistent(viewQos,&context) == GAPI_RETCODE_OK) {
            view = _DataReaderViewNew (viewQos, datareader);
            if ( view ) {
                gapi_setAdd(datareader->viewSet, (gapi_object)view);
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

    gapi_context context;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_DELETE_DATAREADER);

    datareader = gapi_dataReaderClaim(_this, &result);

    if ( datareader != NULL ) {
        view = gapi_dataReaderViewClaim(a_view, NULL);
        if ( view != NULL ) {
            gapi_setIter iterSet = gapi_setFind (datareader->viewSet,
                                                 (gapi_object)view);
            if ( iterSet != NULL ) {
                if ( gapi_setIterObject(iterSet) != NULL ) {
                    if ( _DataReaderViewPrepareDelete(view, &context) ) {
                        gapi_setRemove(datareader->viewSet, (gapi_object)view);
                        _DataReaderViewFree(view);
                        view = NULL;
                    } else {
                        result = GAPI_RETCODE_PRECONDITION_NOT_MET;
                   }
                } else {
                    result = GAPI_RETCODE_PRECONDITION_NOT_MET;
                }
                gapi_setIterFree(iterSet);
            } else {
                result = GAPI_RETCODE_OUT_OF_RESOURCES;
            }
        } else {
            result = GAPI_RETCODE_BAD_PARAMETER;
        }
    }
    _EntityRelease(view);
    _EntityRelease(datareader);

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

    if (( result == GAPI_RETCODE_OK )  && (_Entity(dataReader)->enabled)){
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
        _DataReaderStatus status;

        if ( a_listener ) {
            datareader->_Listener = *a_listener;
        } else {
            memset(&datareader->_Listener, 0, sizeof(datareader->_Listener));
        }

        status = _DataReaderStatus(_EntityStatus(datareader));
        if ( _DataReaderStatusSetListener(status, a_listener, mask) ) {
            result = GAPI_RETCODE_OK;
        }

    }

    _EntityRelease(datareader);

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
    } else {
        memset(&listener, 0, sizeof(listener));
    }

    _EntityRelease(datareader);

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
    }
    _EntityRelease(datareader);

    return topicDescription;
}

gapi_subscriber
gapi_dataReader_get_subscriber (
    gapi_dataReader _this)
{
    gapi_subscriber subscriber = NULL;
    _DataReader datareader;

    datareader = gapi_dataReaderClaim(_this, NULL);
    if ( datareader != NULL ) {
        subscriber = (gapi_subscriber)_EntityHandle(datareader->subscriber);
    }
    _EntityRelease(datareader);

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

gapi_returnCode_t
_DataReader_get_sample_rejected_status (
    _DataReader _this,
    c_bool reset,
    gapi_sampleRejectedStatus *status)
{
    u_result uResult;
    uResult = u_readerGetSampleRejectedStatus(
                  u_reader(U_DATAREADER_GET(_this)),
                  reset,
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
        if (_Entity(datareader)->enabled ) {
            result = _DataReader_get_sample_rejected_status (
                         datareader,
                         TRUE,
                         status);
        } else {
            result=GAPI_RETCODE_NOT_ENABLED;
        }
    }

    _EntityRelease(datareader);

    return result;
}

static v_result
copy_liveliness_changed_status(
    c_voidp info,
    c_voidp arg)
{
    v_handle handle;
    struct v_livelinessChangedInfo *from;
    gapi_livelinessChangedStatus *to;

    from = (struct v_livelinessChangedInfo *)info;
    to = (gapi_livelinessChangedStatus *)arg;

    to->alive_count = from->activeCount;
    to->not_alive_count = from->inactiveCount;
    to->alive_count_change = from->activeChanged;
    to->not_alive_count_change = from->inactiveChanged;

    handle.index  = from->instanceHandle.localId;
    handle.serial = from->instanceHandle.serial;
    to->last_publication_handle = gapi_instanceHandleFromHandle(handle);

    return V_RESULT_OK;
}

gapi_returnCode_t
_DataReader_get_liveliness_changed_status (
    _DataReader _this,
    c_bool reset,
    gapi_livelinessChangedStatus *status)
{
    u_result uResult;
    uResult = u_readerGetLivelinessChangedStatus(
                  u_reader(U_DATAREADER_GET(_this)),
                  reset,
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
        if (_Entity(datareader)->enabled ) {
            result = _DataReader_get_liveliness_changed_status (
                         datareader,
                         TRUE,
                         status);
        } else {
            result=GAPI_RETCODE_NOT_ENABLED;
        }
    }

    _EntityRelease(datareader);

    return result;
}

static v_result
copy_deadline_missed_status(
    c_voidp info,
    c_voidp arg)
{
    v_handle handle;
    struct v_deadlineMissedInfo *from;
    gapi_requestedDeadlineMissedStatus *to;

    from = (struct v_deadlineMissedInfo *)info;
    to = (gapi_requestedDeadlineMissedStatus *)arg;

    to->total_count = from->totalCount;
    to->total_count_change = from->totalChanged;

    handle.index  = from->instanceHandle.localId;
    handle.serial = from->instanceHandle.serial;

    to->last_instance_handle = gapi_instanceHandleFromHandle(handle);

    return V_RESULT_OK;
}

gapi_returnCode_t
_DataReader_get_requested_deadline_missed_status (
    _DataReader _this,
    c_bool reset,
    gapi_requestedDeadlineMissedStatus *status)
{
    u_result uResult;
    uResult = u_readerGetDeadlineMissedStatus(
                  u_reader(U_DATAREADER_GET(_this)),
                  reset,
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
        if (_Entity(datareader)->enabled ) {
            result = _DataReader_get_requested_deadline_missed_status(
                         datareader,
                         TRUE,
                         status);
        } else {
            result=GAPI_RETCODE_NOT_ENABLED;
        }
    }

    _EntityRelease(datareader);

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

gapi_returnCode_t
_DataReader_get_requested_incompatible_qos_status (
    _DataReader _this,
    c_bool reset,
    gapi_requestedIncompatibleQosStatus *status)
{
    u_result uResult;
    uResult = u_readerGetIncompatibleQosStatus(
                  u_reader(U_DATAREADER_GET(_this)),
                  reset,
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
        if (_Entity(datareader)->enabled ) {
            result = _DataReader_get_requested_incompatible_qos_status(
                          datareader,
                          TRUE,
                          status);
        } else {
            result=GAPI_RETCODE_NOT_ENABLED;
        }
    }

    _EntityRelease(datareader);

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
    to->current_count = from->totalCount;
    to->current_count_change = from->totalChanged;
    to->last_publication_handle = gapi_instanceHandle_t(from->instanceHandle);
    return V_RESULT_OK;
}

gapi_returnCode_t
_DataReader_get_subscription_matched_status (
    _DataReader _this,
    c_bool reset,
    gapi_subscriptionMatchedStatus *status)
{
    u_result uResult;
    uResult = u_readerGetSubscriptionMatchStatus(
                  u_reader(U_DATAREADER_GET(_this)),
                  reset,
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
        if (_Entity(datareader)->enabled ) {
            result = _DataReader_get_subscription_matched_status (
                         datareader,
                         TRUE,
                         status);
        } else {
            result=GAPI_RETCODE_NOT_ENABLED;
        }
    }

    _EntityRelease(datareader);

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

gapi_returnCode_t
_DataReader_get_sample_lost_status (
    _DataReader _this,
    c_bool reset,
    gapi_sampleLostStatus *status)
{
    u_result uResult;
    uResult = u_readerGetSampleLostStatus(
                  u_reader(U_DATAREADER_GET(_this)),
                  reset,
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
        if (_Entity(datareader)->enabled ) {
            result = _DataReader_get_sample_lost_status (
                          datareader,
                          TRUE,
                          status);
        } else {
            result=GAPI_RETCODE_NOT_ENABLED;
        }
    }

    _EntityRelease(datareader);

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
        } else if (!_Entity(datareader)->enabled) {
            result = GAPI_RETCODE_NOT_ENABLED;
        } else {
            kernelCopyInDuration(max_wait, &c_time_max_wait);

            uResult = u_dataReaderWaitForHistoricalData(
                          U_DATAREADER_GET(datareader),
                          c_time_max_wait);
            result = kernelResultToApiResult(uResult);
        }
    }

    _EntityRelease(datareader);

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
        } else if (!_Entity(datareader)->enabled) {
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
            kernelCopyInTime(min_source_timestamp, &c_time_min_source_timestamp);
            kernelCopyInTime(max_source_timestamp, &c_time_max_source_timestamp);

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

    return result;
}

gapi_returnCode_t
gapi_dataReader_get_matched_publications (
    gapi_dataReader _this,
    gapi_instanceHandleSeq *publication_handles)
{    
    return GAPI_RETCODE_UNSUPPORTED;
}

gapi_returnCode_t
gapi_dataReader_get_matched_publication_data (
    gapi_dataReader _this,
    gapi_publicationBuiltinTopicData *publication_data,
    const gapi_instanceHandle_t publication_handle)
{    
    return GAPI_RETCODE_UNSUPPORTED;
}

void
_DataReaderSetDeleteAction (
    _DataReader reader,
    gapi_deleteEntityAction action,
    void *argument)
{
     gapi_setIter iter;
    _Condition condition;

     assert(reader);

    _ObjectSetDeleteAction(_Object(reader), action, argument);

    iter = gapi_setFirst(reader->conditionSet);
    if ( iter ) {
        condition = _Condition(gapi_setIterObject(iter));
        while ( condition ) {
            _EntityClaim(condition);
            _ObjectSetDeleteAction(_Object(condition), action, argument);
            _EntityRelease(condition);
            gapi_setIterNext(iter);
            condition = _Condition(gapi_setIterObject(iter));
        }
        gapi_setIterFree(iter);
    }
}

#if 1

gapi_boolean
_DataReaderHasSamplesNotRead (
    _DataReader dataReader)
{
    dataReader->reader_mask.sampleStateMask   = GAPI_NOT_READ_SAMPLE_STATE;
    dataReader->reader_mask.viewStateMask     = 0U;
    dataReader->reader_mask.instanceStateMask = 0U;


    return u_queryTest(dataReader->uQuery);
}

gapi_boolean
_DataReaderContainsSamples (
    _DataReader dataReader,
    const gapi_sampleStateMask   sample_states,
    const gapi_viewStateMask     view_states,
    const gapi_instanceStateMask instance_states)
{
    dataReader->reader_mask.sampleStateMask   = sample_states;
    dataReader->reader_mask.viewStateMask     = view_states;
    dataReader->reader_mask.instanceStateMask = instance_states;

    return u_queryTest(dataReader->uQuery);
}
#endif

static void
_DataReaderNotifyDataAvailable (
    _DataReader _this)
{
    _Status status = _Entity(_this)->status;
    gapi_listener_DataAvailableListener callback;
    void *listenerData;
    gapi_object handle = _EntityHandle(_this);

    callback     = status->callbackInfo.on_data_available; 
    listenerData = status->callbackInfo.listenerData;

    _EntitySetBusy(_this);
    _EntityRelease(_this);
    if ( callback && listenerData ) {
        callback(listenerData, handle);
    }
    gapi_objectClearBusy(handle);
    gapi_dataReaderClaim(handle, NULL);
}


void
_DataReaderTriggerNotify (
    _DataReader dataReader)
{
    assert(dataReader);

    _EntityClaim(dataReader);
    if ( _DataReaderHasSamplesNotRead(dataReader) ) {
        _DataReaderNotifyDataAvailable(dataReader);
    }
    _EntityRelease(dataReader);
}

gapi_returnCode_t
gapi_dataReader_set_default_datareaderview_qos (
    gapi_dataReader _this,
    const gapi_dataReaderViewQos *qos)
{
  return GAPI_RETCODE_UNSUPPORTED;
}

gapi_returnCode_t
gapi_dataReader_get_default_datareaderview_qos (
    gapi_dataReader _this,
    gapi_dataReaderViewQos *qos)
{
  return GAPI_RETCODE_UNSUPPORTED;
}

static c_bool
readerHasDataAvailable (
    v_entity e,
    c_voidp arg)
{
    c_bool *dataAvailable = (c_bool *)arg;
    c_bool result = TRUE;

    if ( (v_statusGetMask(e->status) & V_EVENT_DATA_AVAILABLE) != 0 ) {
        *dataAvailable = TRUE;
        result = FALSE;
    }
    return result;
}

static void
checkDataAvailability(
    v_entity e,
    c_voidp arg)
{
    c_bool *dataAvailable = (c_bool *)arg;

    if ( v_objectKind(e) == K_SUBSCRIBER ) {
        c_setWalk(v_subscriber(e)->readers,
                  (c_action)readerHasDataAvailable, arg);
    } else {
        *dataAvailable = ((v_statusGetMask(e->status) &
                           V_EVENT_DATA_AVAILABLE) != 0);
    }
}

static void
notifyDataOnReaders (
    _DataReader _this,
    gapi_object target)
{
    c_bool dataAvailable;
    u_result result;
    u_entity entity;
    _Subscriber subscriber;
    gapi_object source;

    entity = U_ENTITY_GET(_this);
    assert(entity);
    if (entity) {
        dataAvailable = FALSE;

        result = u_entityAction(entity,
                                checkDataAvailability,
                                &dataAvailable);

        assert(result == U_RESULT_OK);

        if ((result == U_RESULT_OK) && (dataAvailable)) {
            source = _EntityRelease(_this);
            subscriber = _Subscriber(gapi_entityClaim(target,NULL));
            _SubscriberOnDataOnReaders(subscriber);
            _EntityRelease(subscriber);
            gapi_entityClaim(source, NULL);
        }
    }
}

static void
onRequestedDeadlineMissed (
    _DataReader _this)
{
    gapi_requestedDeadlineMissedStatus info;
    gapi_listener_RequestedDeadlineMissedListener callback;
    gapi_returnCode_t result;
    gapi_object target;
    gapi_object source;
    _Entity entity;
    _Status status;
    c_voidp listenerData;

    if ( _this ) {
        result = _DataReader_get_requested_deadline_missed_status(
                     _this, FALSE, &info);            

        if (result == GAPI_RETCODE_OK) {
            status = _Entity(_this)->status;
            source = _EntityHandle(_this);
            target = _StatusFindTarget(status,
                                       GAPI_REQUESTED_DEADLINE_MISSED_STATUS);
            if (target) {
                if ( target != source ) {
                    entity = gapi_entityClaim(target, NULL);
                    status = entity->status;
                } else {
                    entity = NULL;
                }

                callback = status->callbackInfo.on_requested_deadline_missed; 
                listenerData = status->callbackInfo.listenerData;

                _EntitySetBusy(_this);
                _EntityRelease(_this);

                if (entity) {
                    _EntitySetBusy(entity);
                    _EntityRelease(entity);
                    callback(listenerData, source, &info); 
                    gapi_objectClearBusy(target);
                } else {
                    callback(listenerData, source, &info); 
                }

                gapi_objectClearBusy(source);
                gapi_entityClaim(source, NULL);
            }
        }
    }
}

static void
onRequestedIncompatibleQos (
    _DataReader _this)
{
    gapi_requestedIncompatibleQosStatus info;
    gapi_qosPolicyCount policyCount[MAX_POLICY_COUNT_ID];
    gapi_listener_RequestedIncompatibleQosListener callback;
    gapi_returnCode_t result;
    gapi_object target;
    gapi_object source;
    _Entity entity;
    _Status status;
    c_voidp listenerData;
   
    if ( _this ) {
        info.policies._maximum = MAX_POLICY_COUNT_ID;
        info.policies._length  = 0;
        info.policies._buffer  = policyCount;
            
        result = _DataReader_get_requested_incompatible_qos_status(
                     _this, FALSE, &info); 
            
        if (result == GAPI_RETCODE_OK) {
            status = _Entity(_this)->status;
            source = _EntityHandle(_this);
            target = _StatusFindTarget(status,
                                       GAPI_REQUESTED_INCOMPATIBLE_QOS_STATUS);
            if (target) {
                if ( target != source ) {
                    entity = gapi_entityClaim(target, NULL);
                    status = entity->status;
                } else {
                    entity = NULL;
                }

                callback = status->callbackInfo.on_requested_incompatible_qos; 
                listenerData = status->callbackInfo.listenerData;

                _EntitySetBusy(_this);
                _EntityRelease(_this);

                if (entity) {
                    _EntitySetBusy(entity);
                    _EntityRelease(entity);
                    callback(listenerData, source, &info); 
                    gapi_objectClearBusy(target);
                } else {
                    callback(listenerData, source, &info); 
                }

                gapi_objectClearBusy(source);
                gapi_entityClaim(source, NULL);
            }
        }
    }
}

static void
onSampleRejected (
    _DataReader _this)
{
    gapi_listener_SampleRejectedListener callback;
    gapi_sampleRejectedStatus info;
    gapi_returnCode_t result;
    gapi_object source;
    gapi_object target;
    _Entity entity;
    _Status status;
    c_voidp listenerData;
   
    if ( _this ) {
        result = _DataReader_get_sample_rejected_status(
                     _this, FALSE, &info); 

        if (result == GAPI_RETCODE_OK) {
            status = _Entity(_this)->status;
            source = _EntityHandle(_this);
            target = _StatusFindTarget(status,
                                       GAPI_SAMPLE_REJECTED_STATUS);
            if (target) {
                if ( target != source ) {
                    entity = gapi_entityClaim(target, NULL);
                    status = entity->status;
                } else {
                    entity = NULL;
                }

                callback = status->callbackInfo.on_sample_rejected; 
                listenerData = status->callbackInfo.listenerData;

                _EntitySetBusy(_this);
                _EntityRelease(_this);

                if (entity) {
                    _EntitySetBusy(entity);
                    _EntityRelease(entity);
                    callback(listenerData, source, &info); 
                    gapi_objectClearBusy(target);
                } else {
                    callback(listenerData, source, &info); 
                }

                gapi_objectClearBusy(source);
                gapi_entityClaim(source, NULL);
            }
        }
    }
}

static void
onLivelinessChanged (
    _DataReader _this)
{
    gapi_listener_LivelinessChangedListener callback;
    gapi_livelinessChangedStatus info;
    gapi_returnCode_t result;
    gapi_object source;
    gapi_object target;
    _Entity entity;
    _Status status;
    c_voidp listenerData;

    if ( _this ) {
        result = _DataReader_get_liveliness_changed_status(
                     _this, FALSE, &info); 

        if (result == GAPI_RETCODE_OK) {
            status = _Entity(_this)->status;
            source = _EntityHandle(_this);
            target = _StatusFindTarget(status,
                                       GAPI_LIVELINESS_CHANGED_STATUS);
            if (target) {
                if ( target != source ) {
                    entity = gapi_entityClaim(target, NULL);
                    status = entity->status;
                } else {
                    entity = NULL;
                }

                callback = status->callbackInfo.on_liveliness_changed; 
                listenerData = status->callbackInfo.listenerData;

                _EntitySetBusy(_this);
                _EntityRelease(_this);

                if (entity) {
                    _EntitySetBusy(entity);
                    _EntityRelease(entity);
                    callback(listenerData, source, &info); 
                    gapi_objectClearBusy(target);
                } else {
                    callback(listenerData, source, &info); 
                }

                gapi_objectClearBusy(source);
                gapi_entityClaim(source, NULL);
            }
        }
    }
}

/*     void
 *     on_data_available(
 *         in DataReader reader);
 */
static void
onDataAvailable (
    _DataReader _this)
{
    gapi_listener_DataAvailableListener callback;
    gapi_object target;
    gapi_object source;
    _Entity entity;
    _Status status;
    c_voidp listenerData;

    if ( _this ) {
        status = _Entity(_this)->status;
        target = _StatusFindTarget(status,
                                   GAPI_DATA_ON_READERS_STATUS);
        if ( target != NULL ) {
            notifyDataOnReaders(_this, target);
        } else {
            source = _EntityHandle(_this);
            target = _StatusFindTarget(status,
                                       GAPI_DATA_AVAILABLE_STATUS);
            if (target) {
                if ( target != source ) {
                    entity = gapi_entityClaim(target, NULL);
                    status = entity->status;
                } else {
                    entity = NULL;
                }

                callback = status->callbackInfo.on_data_available; 
                listenerData = status->callbackInfo.listenerData;

                _EntitySetBusy(_this);
                _EntityRelease(_this);

                if (entity) {
                    _EntitySetBusy(entity);
                    _EntityRelease(entity);
                    callback(listenerData, source); 
                    gapi_objectClearBusy(target);
                } else {
                    callback(listenerData, source); 
                }

                gapi_objectClearBusy(source);
                gapi_entityClaim(source, NULL);
            }
        }
    }
}

static void
onSubscriptionMatch (
    _DataReader _this) 
{
    gapi_subscriptionMatchedStatus info;
    gapi_listener_SubscriptionMatchedListener callback;
    gapi_returnCode_t result;
    gapi_object target;
    gapi_object source;
    _Status status;
    _Entity entity;
    c_voidp listenerData;
   
    if ( _this ) {
        result = _DataReader_get_subscription_matched_status (
                     _this, FALSE, &info); 
    
        if (result == GAPI_RETCODE_OK) {
            status = _Entity(_this)->status;
            source = _EntityHandle(_this);
            target = _StatusFindTarget(status,
                                       GAPI_LIVELINESS_CHANGED_STATUS);
            if (target) {
                if ( target != source ) {
                    entity = gapi_entityClaim(target, NULL);
                    status = entity->status;
                } else {
                    entity = NULL;
                }

                callback = status->callbackInfo.on_subscription_match; 
                listenerData = status->callbackInfo.listenerData;

                _EntitySetBusy(_this);
                _EntityRelease(_this);

                if (entity) {
                    _EntitySetBusy(entity);
                    _EntityRelease(entity);
                    callback(listenerData, source, &info); 
                    gapi_objectClearBusy(target);
                } else {
                    callback(listenerData, source, &info); 
                }

                gapi_objectClearBusy(source);
                gapi_entityClaim(source, NULL);
            }
        }
    }
}

static void
onSampleLost (
    _DataReader _this)
{
    gapi_sampleLostStatus info;
    gapi_listener_SampleLostListener callback;
    gapi_returnCode_t result;
    gapi_object target;
    gapi_object source;
    _Status status;
    _Entity entity;
    c_voidp listenerData;
   
    if ( _this ) {
        result = _DataReader_get_sample_lost_status (
                     _this, FALSE, &info);

        if (result == GAPI_RETCODE_OK) {
            status = _Entity(_this)->status;
            source = _EntityHandle(_this);
            target = _StatusFindTarget(status,
                                       GAPI_SAMPLE_LOST_STATUS);
            if (target) {
                if ( target != source ) {
                    entity = gapi_entityClaim(target, NULL);
                    status = entity->status;
                } else {
                    entity = NULL;
                }

                callback = status->callbackInfo.on_sample_lost; 
                listenerData = status->callbackInfo.listenerData;

                _EntitySetBusy(_this);
                _EntityRelease(_this);

                if (entity) {
                    _EntitySetBusy(entity);
                    _EntityRelease(entity);
                    callback(listenerData, source, &info); 
                    gapi_objectClearBusy(target);
                } else {
                    callback(listenerData, source, &info); 
                }

                gapi_objectClearBusy(source);
                gapi_entityClaim(source, NULL);
            }
        }
    }
}

void
_DataReaderNotifyListener(
    _DataReader _this,
    gapi_statusMask triggerMask)
{
    while ( _this && (triggerMask != GAPI_STATUS_KIND_NULL) ) {
        if ( triggerMask & GAPI_DATA_AVAILABLE_STATUS ) {
            onDataAvailable(_this);
            triggerMask &= ~GAPI_DATA_AVAILABLE_STATUS;
        }
        if ( triggerMask & GAPI_SAMPLE_REJECTED_STATUS ) {
            onSampleRejected(_this);
            triggerMask &= ~GAPI_SAMPLE_REJECTED_STATUS;
        }
        if ( triggerMask & GAPI_LIVELINESS_CHANGED_STATUS ) {
            onLivelinessChanged(_this);
            triggerMask &= ~GAPI_LIVELINESS_CHANGED_STATUS;
        }
        if ( triggerMask & GAPI_REQUESTED_DEADLINE_MISSED_STATUS ) {
            onRequestedDeadlineMissed(_this);
            triggerMask &= ~GAPI_REQUESTED_DEADLINE_MISSED_STATUS;
        }
        if ( triggerMask & GAPI_REQUESTED_INCOMPATIBLE_QOS_STATUS ) {
            onRequestedIncompatibleQos(_this);
            triggerMask &= ~GAPI_REQUESTED_INCOMPATIBLE_QOS_STATUS;
        }
        if ( triggerMask & GAPI_SAMPLE_LOST_STATUS ) {
            onSampleLost(_this);
            triggerMask &= ~GAPI_SAMPLE_LOST_STATUS;
        }
        if ( triggerMask & GAPI_SUBSCRIPTION_MATCH_STATUS ) {
            onSubscriptionMatch(_this);
            triggerMask &= ~GAPI_SUBSCRIPTION_MATCH_STATUS;
        }
    }
}


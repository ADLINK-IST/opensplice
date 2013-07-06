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
/* C includes */
#include <assert.h>

/* user layer includes */
#include "u_entity.h"

/* DLRL util includes */
#include "DLRL_Report.h"
#include "DLRL_Util.h"

/* DLRL kernel includes */
#include "DK_DCPSUtilityBridge.h"
#include "DK_DCPSUtility.h"
#include "DK_TopicInfo.h"

#define ENTITY_NAME "DLRL Kernel TopicInfo"
static LOC_string allocError = "Unable to allocate " ENTITY_NAME;

static void
DK_TopicInfo_us_destroy(
    DK_Entity * _this);

/* topicUserData may be null */
DK_TopicInfo*
DK_TopicInfo_new(
    DLRL_Exception* exception,
    u_topic topic,
    DLRL_LS_object ls_topic,
    DK_ObjectHomeAdmin* owner,
    DMM_DCPSTopic* metaTopic,
    void* topicUserData)
{
    DK_TopicInfo* _this = NULL;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(topic);
    assert(owner);
    assert(metaTopic);
    /* topicUserData and ls_topic may be null */

    DLRL_ALLOC(_this, DK_TopicInfo, exception, "%s '%s'", allocError,
                    DLRL_VALID_NAME(DMM_DCPSTopic_getTopicName(_this->metaTopic)));

    _this->alive = TRUE;
    _this->owner = (DK_ObjectHomeAdmin*)DK_Entity_ts_duplicate((DK_Entity*)owner);
    _this->topicUserData  = topicUserData;
    _this->metaTopic = metaTopic;/* meta model isnt ref counted */
    _this->ls_topic = ls_topic;
    _this->dataSample = NULL;
    _this->topicDataSampleOffset = 0;
    _this->message = NULL;
    _this->topic = topic;

    DK_Entity_us_init(&(_this->entity), DK_CLASS_TOPIC_INFO, DK_TopicInfo_us_destroy);
    DLRL_INFO(INF_ENTITY, "created %s, address = %p", ENTITY_NAME, _this);
    DLRL_Exception_EXIT(exception);
    if((exception->exceptionID != DLRL_NO_EXCEPTION) && _this)
    {
        /* set user layer topic to null to prevent double free, as caller
         * of this operation will still assume ownership if this function has
         * failed!
         */
        _this->topic = NULL;
        DK_TopicInfo_us_delete(_this, NULL);
        DK_Entity_ts_release((DK_Entity*)_this);
        _this = NULL;
    }

    DLRL_INFO(INF_EXIT);
    return _this;
}

void
DK_TopicInfo_us_destroy(
    DK_Entity * _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    /* _this may be null */

    if(_this)
    {
        DLRL_INFO(INF_ENTITY, "destroyed %s, address = %p", ENTITY_NAME, _this);
        os_free((DK_TopicInfo*)_this);
    }

    DLRL_INFO(INF_EXIT);
}

void
DK_TopicInfo_us_delete(
    DK_TopicInfo* _this,
    void* userData)
{
    DK_CacheAdmin* cache = NULL;
    DLRL_Exception exception;
    u_result result;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    /* userData may be null */

    if(_this->alive)
    {
        DLRL_Exception_init(&exception);
        if(_this->message)
        {
            u_result result;

            result = u_entityAction(
                u_entity(_this->topic),
                DK_DCPSUtility_us_freeMessage,
                _this->message);
            if(result != U_RESULT_OK)
            {
               DLRL_Exception_transformResultToException(&exception, result, "Unable to free v_message object.");
               DLRL_REPORT(REPORT_ERROR, "Exception %s occured when attempting to free a v_message object cached at the DK_TopicInfo object.\n%s",
                    DLRL_Exception_exceptionIDToString(exception.exceptionID), exception.exceptionMessage);
               /*reinit the exception, we may use it later on and we dont want wierd ass errors because someone forgets this
                * step*/
               DLRL_Exception_init(&exception);
            }
            /* data sample is cleared through the deletion of the message */
            _this->dataSample = NULL;
            _this->topicDataSampleOffset = 0;
            _this->message = NULL;
        }
        if(_this->topic)
        {
            cache = DK_ObjectHomeAdmin_us_getCache(_this->owner);/* no duplicate done */
            dcpsUtilityBridge.deleteTopic(&exception, userData, cache, _this);
            if(exception.exceptionID != DLRL_NO_EXCEPTION)
            {
                DLRL_REPORT(REPORT_ERROR, "Exception %s occured when attempting to delete the DCPS topic\n%s",
                    DLRL_Exception_exceptionIDToString(exception.exceptionID), exception.exceptionMessage);
                /* reset the exception, maybe it's used again later in this deletion function. We dont propagate the */
                /* exception here anyway, so it can do no harm as we already logged the exception directly above. */
                DLRL_Exception_init(&exception);
            }
            result = u_entityFree(u_entity(_this->topic));/* destroy the proxy object */
            if(result != U_RESULT_OK)
            {
               DLRL_Exception_transformResultToException(
                   &exception,
                   result,
                   "Unable to free the user layer topic!");
               DLRL_REPORT(
                   REPORT_ERROR,
                   "Exception %s occured when attempting to delete the DCPS "
                        "user layer topic\n%s",
                    DLRL_Exception_exceptionIDToString(exception.exceptionID),
                   exception.exceptionMessage);
               DLRL_Exception_init(&exception);
            }
            _this->topic = NULL;
            _this->ls_topic = NULL;
        }

        if(_this->owner)
        {
            DK_Entity_ts_release((DK_Entity*)_this->owner);
        }
        if(_this->metaTopic)
        {
            _this->metaTopic = NULL;/* isnt owner of meta topic, so doesn't have to delete it. */
        }
        if(_this->topicUserData)
        {
            dcpsUtilityBridge.releaseTopicUserData(userData, _this->topicUserData);
            _this->topicUserData = NULL;
        }
        _this->alive = FALSE;
    }
    DLRL_INFO(INF_EXIT);
}

/* no duplicate done by this operation! */
DK_ObjectHomeAdmin*
DK_TopicInfo_us_getOwner(
    DK_TopicInfo* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->owner;
}

DMM_DCPSTopic*
DK_TopicInfo_us_getMetaTopic(
    DK_TopicInfo* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->metaTopic;
}

LOC_string
DK_TopicInfo_us_getTopicName(
    DK_TopicInfo* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return DMM_DCPSTopic_getTopicName(_this->metaTopic);
}

LOC_string
DK_TopicInfo_us_getTopicType(
    DK_TopicInfo* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return DMM_DCPSTopic_getTopicTypeName(_this->metaTopic);
}

u_topic
DK_TopicInfo_us_getTopic(
    DK_TopicInfo* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->topic;
}

DLRL_LS_object
DK_TopicInfo_us_getLSTopic(
    DK_TopicInfo* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->ls_topic;
}

void*
DK_TopicInfo_us_getTopicUserData(
    DK_TopicInfo* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->topicUserData;
}

void
DK_TopicInfo_us_setDataSample(
    DK_TopicInfo* _this,
    c_object dataSample)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(dataSample);

    _this->dataSample = dataSample;
    DLRL_INFO(INF_EXIT);
}

void
DK_TopicInfo_us_setDataSampleOffset(
    DK_TopicInfo* _this,
    c_long dataSampleOffset)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(dataSampleOffset >= 0);

    _this->topicDataSampleOffset = dataSampleOffset;
    DLRL_INFO(INF_EXIT);
}

void
DK_TopicInfo_us_setMessage(
    DK_TopicInfo* _this,
    v_message message)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(message);

    _this->message = message;
    DLRL_INFO(INF_EXIT);
}

void
DK_TopicInfo_us_enable(
    DK_TopicInfo* _this,
    DLRL_Exception* exception,
    void* userData)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    dcpsUtilityBridge.enableEntity(exception, userData, _this->ls_topic);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

c_object
DK_TopicInfo_us_getDataSample(
    DK_TopicInfo* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->dataSample;
}

c_long
DK_TopicInfo_us_getDataSampleOffset(
    DK_TopicInfo* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->topicDataSampleOffset;
}

v_message
DK_TopicInfo_us_getMessage(
    DK_TopicInfo* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->message;
}

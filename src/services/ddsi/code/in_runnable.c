#include "in_channel.h"
#include "os_heap.h"
#include "in_report.h"

#define IN_SCHEDCLASS_REALTIME  "Realtime"
#define IN_SCHEDCLASS_TIMESHARE "Timeshare"

static void*
in_runnableDispatcher(
    void* userData);

void
in_runnableStart(
    in_runnable _this)
{
    os_threadAttr threadAttr;
    os_result result;

    assert(_this);

    os_threadAttrInit(&threadAttr);
    if (!_this->schedulingAttr.schedulingPrioUseDefault)
    {
        threadAttr.schedPriority = _this->schedulingAttr.schedulingPrioValue;
    }
    threadAttr.schedClass = _this->schedulingAttr.schedulingClass;

    IN_TRACE_2(Receive,3,"in_runnableStart %x %d",_this,in_objectGetKind(in_object(_this)));

    result = os_threadCreate(
        &_this->threadId,
        _this->name,
        &threadAttr,
        in_runnableDispatcher,
        _this);
    if (result != os_resultSuccess)
    {
        IN_REPORT_ERROR_1(
        "thread creation",
        "Creation of thread \"%s\" failed",
        in_runnableGetName(_this));
    }
}

void
in_runnableStop(
    in_runnable _this)
{
    assert(_this);
/*
    if (_this->runState == IN_RUNSTATE_RUNNING)
    {
*/
        /* First stop the running thread */
        os_mutexLock(&_this->mutex);
        _this->terminate = OS_TRUE;
        in_runnableTrigger(_this);
        os_mutexUnlock(&_this->mutex);

        /* Wait for termination of the thread. The thread mainfunc itself has
         * to react on the change of the status of runnable->terminate by
         * setting the runState to rs_Terminated.
         */
        os_threadWaitExit(_this->threadId, NULL);
        assert(_this->runState != IN_RUNSTATE_RUNNING);
/*
    }
*/
}

os_boolean
in_runnableInit(
    in_runnable _this,
    const in_objectKind kind,
    const in_objectDeinitFunc deinit,
    const os_char* name,
    const os_char* pathName,
    const in_runnableMainFunc runnableMainFunc,
    const in_runnableTriggerFunc triggerFunc)
{
    os_boolean success;
    os_mutexAttr mutexAttr;
    struct in_schedulingAttr_s schedulingAttr;
    os_int32 schedPrio = 0x8000000;
    /* todo os_char* schedClassString;*/

    assert(_this);
    assert(kind < IN_OBJECT_KIND_COUNT);
    assert(kind > IN_OBJECT_KIND_INVALID);
    assert(deinit);
    assert(name);
    /* todo read config
    /assert(pathName);*/
    assert(runnableMainFunc);
    assert(triggerFunc);

    success = in_objectInit(in_object(_this), kind, deinit);
    assert(in_objectIsValid(in_object(_this)));
    if(success)
    {
        _this->name = os_strdup(name);
        if(!_this->name)
        {
            success = OS_FALSE;
        }
    }
    if(success)
    {
        _this->mainFunc = runnableMainFunc;
        _this->triggerFunc = triggerFunc;

        _this->runState = IN_RUNSTATE_NONE;
        _this->terminate = OS_FALSE;
        _this->threadId = OS_THREAD_ID_NONE;

        os_mutexAttrInit(&mutexAttr);
        mutexAttr.scopeAttr = OS_SCOPE_PRIVATE;
        os_mutexInit(&_this->mutex, &mutexAttr);

        /* Get scheduling attributes */
        /* todo read config
        schedPrio = INCF_SIMPLE_PARAM(Long, pathName, Priority);*/
        schedulingAttr.schedulingPrioUseDefault = (schedPrio == INCF_DEF(Priority));
/*        if (!schedulingAttr.schedulingPrioUseDefault)
        {*/
            schedulingAttr.schedulingPrioValue = schedPrio;
       /* }
*/
        schedulingAttr.schedulingClass = OS_SCHED_DEFAULT;
/*        schedClassString = INCF_SIMPLE_PARAM(String, pathName, Class);
        if (0 == strcmp(schedClassString, IN_SCHEDCLASS_REALTIME))
        {
            schedulingAttr.schedulingClass = OS_SCHED_REALTIME;
        } else if (0 == strcmp(schedClassString, IN_SCHEDCLASS_TIMESHARE))
        {
            schedulingAttr.schedulingClass = OS_SCHED_TIMESHARE;
        }
        os_free(schedClassString);
*/
        _this->schedulingAttr = schedulingAttr;
    }

    IN_TRACE_1(Construction,2,"in_runnableInit success:%d",success);
    assert(in_objectIsValid(in_object(_this)));
    return success;
}

void
in_runnableDeinit(
    in_object _this)
{
    in_runnable runnable;

    assert(_this);

    runnable = in_runnable(_this);

    os_mutexDestroy(&runnable->mutex);
    os_free(runnable->name);
}


void*
in_runnableDispatcher(
    void* userData)
{
    in_runnable _this;

    assert(userData);

    _this = (in_runnable)userData;

    if (_this && _this->mainFunc)
    {
        _this->mainFunc(_this);
    }
    return NULL;
}

const os_char*
in_runnableGetName(
    in_runnable _this)
{
    const os_char* result = NULL;

    assert(_this);

    os_mutexLock(&_this->mutex);
    result = os_strdup(_this->name);
    os_mutexUnlock(&_this->mutex);

    return result;
}


void
in_runnableSetRunState(
    in_runnable _this,
    in_runState runState)
{
    assert(_this);

    os_mutexLock(&_this->mutex);
    _this->runState = runState;
    os_mutexUnlock(&_this->mutex);
}


os_boolean
in_runnableTerminationRequested(
    in_runnable _this)
{
    os_boolean result = OS_TRUE;

    assert(_this);

    os_mutexLock(&_this->mutex);
    result = _this->terminate;
    os_mutexUnlock(&_this->mutex);

    return result;
}


void
in_runnableTrigger(
    in_runnable _this)
{
    assert(_this);
    assert(_this->triggerFunc);

    _this->triggerFunc(_this);
}



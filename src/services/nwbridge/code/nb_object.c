/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#include "nb__object.h"
#include "nb__util.h"
#include "nb__log.h"

#include "os_heap.h"
#include "os_defs.h"
#include "os_abstract.h"
#include "os_atomics.h"
#include <string.h>

#include "v_state.h"          /* for status flags */
#include "v_dataReaderSample.h"

#include "u_writer.h"

void
nb__objectInit(
    nb_object _this,
    nb_objectKind kind,
    nb_objectDeinitFunc deinit)
{
    assert(_this);
    assert(NB_OBJECT_VALIDKIND(kind));
    assert(deinit);

#ifndef NDEBUG
    _this->confidence = NB_OBJECT_CONFIDENCE;
#endif /* NDEBUG */
    pa_st32(&_this->refCount, 1);
    _this->deinit = deinit;
    _this->kind = kind;

}

void
nb__objectDeinit(
    nb_object _this)
{
#ifndef NDEBUG
    assert(nb__objectIsValid(_this));
    _this->confidence = 0;
#else
    OS_UNUSED_ARG(_this);
#endif /* NDEBUG */
}

nb_objectKind
nb__objectKind (
    nb_object _this)
{
    assert(nb__objectIsValid(_this));
    return _this->kind;
}

void
nb__objectSetDeinit(
    nb_object _this,
    nb_objectDeinitFunc deinit)
{
    assert(nb__objectIsValid(_this));

    _this->deinit = deinit;
}

nb_object
nb__objectKeep(
    nb_object _this)
{
    assert(nb__objectIsValid(_this));

    pa_inc32(&(_this->refCount));

    return _this;
}

void
nb__objectFree(
    nb_object _this)
{
    if(_this){
        os_uint32 refCount;

        assert(nb__objectIsValid(_this));

        refCount = pa_dec32_nv(&(_this->refCount));
        if(refCount == 0) {
            assert(_this->deinit);
            _this->deinit(_this);
            os_free(_this);
        }
    }
}

#ifndef NDEBUG
c_bool
nb__objectIsValid(
    nb_object _this)
{
    if(!_this) return FALSE;
    if(!(_this->confidence == NB_OBJECT_CONFIDENCE)) return FALSE;
    if(!NB_OBJECT_VALIDKIND(_this->kind)) return FALSE;
    return TRUE;
}

c_bool
nb__objectIsValidKind(
    nb_object _this,
    nb_objectKind kind)
{
    if(!nb__objectIsValid(_this)) return FALSE;
    if(nb__objectKind(_this) != kind) return FALSE;
    return TRUE;
}

#endif /* NDEBUG */

const char*
nb__objectKindImage(
    nb_object _this)
{
    const char* image;

    assert(nb__objectIsValid(_this));

#define _NB_IMAGE_STR_(k) ((char*)((os_address)(#k) + strlen("NB_OBJECT_")))
#define _NB_IMAGE_STR_CASE(k) case (k): image = _NB_IMAGE_STR_(k); break;
    switch(_this->kind) {
        _NB_IMAGE_STR_CASE(NB_OBJECT_SERVICE);
        _NB_IMAGE_STR_CASE(NB_OBJECT_THREAD);
        _NB_IMAGE_STR_CASE(NB_OBJECT_LOGBUF);
        _NB_IMAGE_STR_CASE(NB_OBJECT_CONFIGURATION);
        _NB_IMAGE_STR_CASE(NB_OBJECT_GROUP);
        _NB_IMAGE_STR_CASE(NB_TOPIC_OBJECT_DCPS_TOPIC);
        _NB_IMAGE_STR_CASE(NB_TOPIC_OBJECT_DCPS_SUBSCRIPTION);
        _NB_IMAGE_STR_CASE(NB_TOPIC_OBJECT_CM_READER);
        _NB_IMAGE_STR_CASE(NB_TOPIC_OBJECT_DCPS_PUBLICATION);
        default:
            image = "(invalid kind)";
            break;
    }
#undef _NB_IMAGE_STR_CASE
#undef _NB_IMAGE_STR_
    return image;
}

void
nb__topicObjectInit (
    nb_topicObject _this,
    nb_objectKind kind,
    nb_objectDeinitFunc deinit,
    const c_char * name,
    nb_topicObjectCopyOutFunc copyOut,
    nb_topicObjectCopyInFunc copyIn)
{
    /* super-init */
    nb__objectInit(nb_object(_this), kind, deinit);

    assert(name);

    _this->name = os_strdup(name);
    _this->state = 0;
    _this->writeTime = OS_TIMEW_INVALID;
    _this->copyOut = copyOut;
    _this->copyIn = copyIn;
}

void
nb__topicObjectDeinit (
    nb_object _this)
{
    os_free(nb_topicObject(_this)->name);

    /* super-deinit */
    nb__objectDeinit(_this);
}

u_result
nb_topicObjectCopyOut(
    nb_topicObject _this,
    const void * from)
{
    u_result result = U_RESULT_UNSUPPORTED;

    assert(_this);

    if (_this->copyOut) {
        result = _this->copyOut(_this, from);
    }

    return result;
}

v_copyin_result
nb_topicObjectCopyIn(
    c_type type,
    const void *from, /* nb_topicObject */
    void *to)
{
    nb_topicObject _this = nb_topicObject(from);
    v_copyin_result result = V_COPYIN_RESULT_OK;

    assert(_this);
    assert(from);
    assert(to);

    if (_this->copyIn) {
        result = _this->copyIn(type, _this, to);
    }

    if(!V_COPYIN_RESULT_IS_OK(result)){
        NB_TRACE(("nb_topicObjectCopyIn for %s failed\n", nb_objectKindImage(_this)));
    }

    return result;
}

u_result
nb_topicObjectWrite(
        u_writer writer,
        nb_topicObject _this)
{
    u_result result = U_RESULT_UNSUPPORTED;
    os_timeW timestamp;

    assert(writer);
    nb_objectIsValid(_this);

    timestamp = _this->writeTime;

    if(v_stateTest(_this->state, L_REGISTER)){
        result = u_writerRegisterInstance(
                     writer,
                     nb_topicObjectCopyIn,
                     _this,
                     timestamp,
                     U_INSTANCEHANDLE_NIL);
    } else if(v_stateTest(_this->state, L_WRITE) && v_stateTest(_this->state, L_DISPOSED)){
        result = u_writerWriteDispose(
                     writer,
                     nb_topicObjectCopyIn,
                     _this,
                     timestamp,
                     U_INSTANCEHANDLE_NIL);
    } else if(v_stateTest(_this->state, L_WRITE)) {
        result = u_writerWrite(
                     writer,
                     nb_topicObjectCopyIn,
                     _this,
                     timestamp,
                     U_INSTANCEHANDLE_NIL);
    } else if(v_stateTest(_this->state, L_DISPOSED)) {
        result = u_writerDispose(
                     writer,
                     nb_topicObjectCopyIn,
                     _this,
                     timestamp,
                     U_INSTANCEHANDLE_NIL);
    }

    return result;
}

const c_char *
nb_topicObjectName(
        nb_topicObject _this)
{
    nb_objectIsValid(_this);

    assert(_this->name);
    return _this->name;
}

v_state
nb_topicObjectState(
        nb_topicObject _this)
{
    nb_objectIsValid(_this);

    return _this->state;
}

v_actionResult
nb_topicObjectReaderAction(
    c_object o,
    c_voidp copyArg, /* c_iter<nb_topicObject> * */
    nb_topicObjectAllocFunc allocFunc)
{
    nb_topicObject to;
    v_actionResult result = 0;

    assert(allocFunc);

    if(o != NULL){
        c_iter *iter;
        v_dataReaderSample s = v_dataReaderSample(o);
        v_message message = v_dataReaderSampleMessage(s);
        const void * from = C_DISPLACE (message, C_MAXALIGNSIZE(sizeof(*message)));

        iter = (c_iter*)copyArg;
        assert(iter);

        v_actionResultSet(result, V_PROCEED);
        to = allocFunc();
        to->state = v_nodeState(message);
        to->writeTime = message->writeTime;

        nb_topicObjectCopyOut(to, from);

        *iter = c_iterAppend(*iter, to);
    }
    return result;
}

#ifndef NDEBUG
void
nb_topicObjectFree (
    nb_topicObject _this)
{
    /* TODO: check proper inheritance? */
    nb_objectFree(_this);
}
#endif

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

#include "u__types.h"
#include "u__object.h"
#include "u__observable.h"
#include "u__entity.h"
#include "u__reader.h"
#include "u_subscriber.h"
#include "u_user.h"
#include "u_groupQueue.h"

#include "v_subscriber.h"
#include "v_topic.h"
#include "v_groupQueue.h"
#include "v_reader.h"
#include "v_query.h"
#include "os_report.h"

static u_result
u_groupQueueInit(
    const u_groupQueue _this,
    const v_groupQueue queue,
    const u_subscriber subscriber)
{
    return u_readerInit(u_reader(_this), v_reader(queue), subscriber);
}

static u_result
u__groupQueueDeinitW(
    void *_this)
{
    return u__readerDeinitW(_this);
}

static void
u__groupQueueFreeW(
    void *_this)
{
    u__readerFreeW(_this);
}

u_groupQueue
u_groupQueueNew(
    const u_subscriber s,
    const c_char *name,
    c_ulong queueSize,
    const v_readerQos qos,
    c_iter expr)
{
    u_groupQueue _this = NULL;
    v_subscriber ks = NULL;
    v_groupQueue kn;
    u_result result;

    assert (s != NULL);
    assert (name != NULL);

    result = u_observableWriteClaim(u_observable(s),(v_public *)(&ks), C_MM_RESERVATION_HIGH);
    if (result == U_RESULT_OK) {
        assert(ks);
        kn = v_groupQueueNew(ks,name,queueSize,qos, expr);
        if (kn != NULL) {
            _this = u_objectAlloc(sizeof(*_this), U_GROUPQUEUE, u__groupQueueDeinitW, u__groupQueueFreeW);
            if (_this != NULL) {
                result = u_groupQueueInit(_this, kn, s);
                if (result != U_RESULT_OK) {
                    OS_REPORT(OS_ERROR, "u_groupQueueNew", result,
                                "Initialisation failed. "
                                "For groupQueue: <%s>.", name);
                    u_objectFree (u_object (_this));
                    _this = NULL;
                }
            } else {
                OS_REPORT(OS_ERROR, "u_groupQueueNew", U_RESULT_INTERNAL_ERROR,
                            "Create proxy failed. "
                            "For groupQueue: <%s>.", name);
            }
            c_free(kn);
        } else {
            OS_REPORT(OS_ERROR, "u_groupQueueNew", U_RESULT_INTERNAL_ERROR,
                        "Create kernel entity failed. "
                        "For groupQueue: <%s>.", name);
        }
        u_observableRelease(u_observable(s), C_MM_RESERVATION_HIGH);
    } else {
        OS_REPORT(OS_WARNING, "u_groupQueueNew", U_RESULT_INTERNAL_ERROR,
                    "Claim Subscriber (0x%"PA_PRIxADDR") failed. "
                    "For groupQueue: <%s>.", (os_address)s, name);
    }
    return _this;
}

u_result
u_groupQueueSize(
    u_groupQueue _this,
    c_ulong * size)
{
    u_result result;
    v_groupQueue kq;

    result = u_observableWriteClaim(u_observable(_this),(v_public *)(&kq), C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        *size = v_groupQueueSize(kq);
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
    }

    return result;
}

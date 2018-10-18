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

#ifndef U__TYPES_H
#define U__TYPES_H

#include "u_types.h"
#include "v_handle.h"
#include "c_iterator.h"
#include "vortex_os.h"
#include "os_heap.h"

typedef v_handle u_handle;

C_STRUCT(u_dispatcher)
{
    u_observable             observable;
    c_iter                   callbacks;
    os_mutex                 mutex;
    os_threadId              threadId;
    c_ulong                  event;
};

typedef u_result (*u_deinitFunction_t)(void *_this);
typedef void (*u_freeFunction_t)(void *_this);

C_STRUCT(u_object) {
    /* The kind attribute uniquely identifis the type of the user layer object.
     */
    u_kind kind;

    pa_voidp_t deinit;
    u_freeFunction_t free;
};

C_STRUCT(u_observable) {
    C_EXTENDS(u_object);

    u_domain domain;

    /* The handle attribute is the smart reference to the shared domain object
     * this proxy object is associated with.
     */
    u_handle handle;
    u_bool isService;

    /* The magic attribute is an optimization used by v_gidClaim and v_gidRelease
     * for validity checking.
     */
    v_kernel magic;

    /* The gid attribute is the global unique id of the associated shared domain object.
     */
    v_gid gid;

    /* Each object can create a dispatcher (thread) to handle events if
     * a listener is set. This mechanism is only used by user layer
     * activities (watching service liveliness or creation of new groups)
     * There are some fundamental issues with this mechanism and therefore
     * it is subject to be replaced in the near future.
     */
    u_dispatcher dispatcher;

    /* userData is passed to listeners together with the event that occurred.
     * This is typically the language binding object on which the listener was set.
     */
    void *userData;
};

C_STRUCT(u_entity) {
    C_EXTENDS(u_observable);
    /* The enabled flag is actually implemented in the kernel but is cached is
     * here to avoid going to the kernel for each call. For as long as the entity
     * is not enabled each call will go to the kernel to fetch the actual state but
     * as soon as it is enabled it will cache the state and never go to the kernel
     * again because once enabled it will always be enabled.
     */
    u_bool enabled;
};

C_STRUCT(u_publisher) {
    C_EXTENDS(u_entity);
};

C_STRUCT(u_writer) {
    C_EXTENDS(u_entity);
};

C_STRUCT(u_subscriber) {
    C_EXTENDS(u_entity);
};

C_STRUCT(u_reader) {
    C_EXTENDS(u_entity);
};

C_STRUCT(u_dataReader) {
    C_EXTENDS(u_reader);
};

C_STRUCT(u_networkReader) {
    C_EXTENDS(u_reader);
};

C_STRUCT(u_groupQueue) {
    C_EXTENDS(u_reader);
};

C_STRUCT(u_query) {
    C_EXTENDS(u_entity);
};

C_STRUCT(u_dataView) {
    C_EXTENDS(u_entity);
};

C_STRUCT(u_group) {
    C_EXTENDS(u_observable);
};

C_STRUCT(u_partition) {
    C_EXTENDS(u_entity);
};

C_STRUCT(u_topic) {
    C_EXTENDS(u_entity);
    os_char *name;
    u_participant participant; /* Is really this required? */
};

C_STRUCT(u_waitsetEntry) {
    C_EXTENDS(u_observable);
    os_mutex mutex;
    u_handle handle;
    os_threadId thread;
    u_bool alive;
    c_ulong waitCount;
    u_waitset waitset;
};

C_STRUCT(u_waitset) {
    C_EXTENDS(u_object);
    os_mutex mutex;
    os_cond  cv;
    c_iter   entries; /* u_waitsetEntry */
    c_ulong  eventMask;
    u_bool   alive;
    u_bool   waitBusy;
    c_ushort detachCnt;
    os_cond  waitCv;
    os_boolean multi_mode;
    u_bool eventsEnabled;
    os_boolean notifyDetached;
    pa_uint32_t useCount;
};

C_STRUCT(u_listener) {
    C_EXTENDS(u_observable);
};

C_STRUCT(u_statusCondition) {
    C_EXTENDS(u_observable);
};

C_STRUCT(u_participant)
{
    C_EXTENDS(u_entity);
    os_threadId       threadId;
    os_threadId       threadIdResend;
    pa_uint32_t       useCount;
    os_mutex          mutex;
    os_cond           cv;
    os_uint32         threadWaitCount;
};

C_STRUCT(u_serviceManager) {
    C_EXTENDS(u_entity);
};

C_STRUCT(u_service) {
    C_EXTENDS(u_participant);
    u_serviceManager serviceManager;
    u_serviceSplicedaemonListener callback;
    void *usrData;
};

C_STRUCT(u_spliced) {
    C_EXTENDS(u_service);
};

#define U_DOMAIN_STATE_ALIVE                ((os_uint32)(0))
#define U_DOMAIN_STATE_DETACHING            ((os_uint32)(1 << 0))
#define U_DOMAIN_STATE_DELETE               ((os_uint32)(1 << 1))
#define U_DOMAIN_BLOCK_IN_USER              ((os_uint32)(1 << 29))
#define U_DOMAIN_DELETE_ENTITIES            ((os_uint32)(1 << 30))
#define U_DOMAIN_BLOCK_IN_KERNEL            ((os_uint32)(1 << 31))

C_CLASS(u_splicedThread);

C_STRUCT(u_domain) {
    C_EXTENDS(u_entity);
    pa_uint32_t     refCount; /* number of domain pointers in user layer objects */
    os_uint32       openCount; /* number of participants, waitset entries, &c. */
    int             closing; /* Indicates that the domain is being closed and cannot be opened.
                              * This flag is set when the last application participant is
                              * deleted. In case of single process there may still be services
                              * using the domain, therefore the state flag can not be used for
                              * this purpose. */
    v_kernel        kernel;
    v_processInfo   procInfo;
    os_uint32       serial; /* copy of procInfo->serial stored outside kernel */
    os_sharedHandle shm;
    u_splicedThread spliced_thread; /* NULL for SHM domains*/
    c_iter          participants;
    c_iter          waitsets;
    os_char         *uri;
    os_char         *name;
    os_lockPolicy   lockPolicy;
    os_uint32       protectCount;
    u_domainId_t    id;
    os_boolean      owner; /* Used by u_domainClose to destroy SHM if called by spliced */
    os_mutex        mutex;
    os_cond         cond; /* Used to synchronize the domain detach operation */
    os_mutex        deadlock; /* Trying to obtain this mutex will cause a guaranteed deadlock */
    u_bool          inProcessExceptionHandling;
    u_bool          y2038Ready; /* Indicates that the configuration option y2038ready is set.
                                 * Timestamps sent by this federation will deviate from the
                                 * previously used c_type representation and now sends out
                                 * os_timeW. When this option is set it's not possible to
                                 * correctly communicate with federation which don't support
                                 * this option. */
    u_bool          isService;
    /* No threads can have access to the kernel during destruction of the user domain,
     * the state flag will be set when destruction is initiated and
     * reject all subsequent access by returning already deleted.
     * The only thread that must have access is the thread executing the destruction.
     */
    pa_uint32_t     state;
    pa_uint32_t     claimed; /* Used to protect access to the process info */
    os_threadId     threadWithAccess;
    c_iter          reportPlugins;
    os_duration     serviceTerminatePeriod;
};

#endif

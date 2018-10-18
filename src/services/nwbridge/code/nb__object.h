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
#ifndef NB__OBJECT_H_
#define NB__OBJECT_H_

#include "nb__types.h"
#include "u_types.h"

#include "v_readerSample.h"   /* for v_actionResult */

#ifndef NDEBUG
#define NB_OBJECT_VALIDKIND(k)  (((k) > NB_OBJECT_INVALID) && ((k) < NB_OBJECT_COUNT))
#define NB_OBJECT_CONFIDENCE    (3123123123U)
#else
#define NB_OBJECT_VALIDKIND(k)
#endif

typedef enum nb_objectKind_e {
#ifndef NDEBUG
    NB_OBJECT_INVALID,
#endif
    NB_OBJECT_SERVICE,
    NB_OBJECT_THREAD,
    NB_OBJECT_LOGBUF,
    NB_OBJECT_CONFIGURATION,
    NB_OBJECT_GROUP,
    NB_TOPIC_OBJECT_DCPS_TOPIC,
    NB_TOPIC_OBJECT_DCPS_PARTICIPANT,
    NB_TOPIC_OBJECT_CM_PARTICIPANT,
    NB_TOPIC_OBJECT_DCPS_PUBLICATION,
    NB_TOPIC_OBJECT_CM_WRITER,
    NB_TOPIC_OBJECT_DCPS_SUBSCRIPTION,
    NB_TOPIC_OBJECT_CM_READER,
    NB_TOPIC_OBJECT_CM_PUBLISHER,
    NB_TOPIC_OBJECT_CM_SUBSCRIBER /* No comma */
#ifndef NDEBUG
    , NB_OBJECT_COUNT
#endif
} nb_objectKind;

typedef void (* nb_objectDeinitFunc)(nb_object);

C_STRUCT(nb_object) {
#ifndef NDEBUG
    os_uint32 confidence;
#endif /* NDEBUG */
    /* The reference count of this object. It is initialized at 1 and once it reaches 0 it will trigger the
     * registered destroy operation. */
    pa_uint32_t refCount;
    /* The destroy operation that will be executed once an object reference count reaches zero. */
    nb_objectDeinitFunc deinit;
    /* The enum value which identifies the subclass of this object. */
    nb_objectKind kind;
};

#define nb_object(o) ((nb_object)(o))

/**
 * \brief Initializes the object
 *
 * This operation will set the class id, destroy function pointer, reference count to 1 and the alive boolean to true
 *
 * \param _this the object to operate on
 * \param classId The class ID of the (bottom most) child object
 * \param deinit The function to call when the reference count of this object reaches zero.
 */
void            nb__objectInit (nb_object _this,
                                nb_objectKind kind,
                                nb_objectDeinitFunc deinit) __nonnull_all__;

/**
 * \Brief Deinitializes the object
 *
 * \param _this the object to operate on
 */
void            nb__objectDeinit(nb_object _this) __nonnull_all__;

nb_objectKind   nb__objectKind(nb_object _this) __nonnull_all__;

void            nb__objectSetDeinit(nb_object _this,
                                    nb_objectDeinitFunc deinit) __nonnull_all__;

/* \brief This operation increases the reference count of the provided object by one.
 *
 * For every call to this operation the <code>r_objectRelease(...)</code> operation must be called once.
 *
 * \param _this The object for which the reference count has to be increased.
 *
 * \return _this
 */
nb_object       nb__objectKeep(nb_object _this) __attribute_returns_nonnull__
                                                __nonnull_all__;

/* Convenience macro to not having to do a cast on every keep */
#define nb_objectKeep(o) nb__objectKeep(nb_object(o))

/* \brief This operation decreases the reference count of the provided object with one.
 *
 * \param _this The object for which the reference count has to be decreased.
 */
void            nb__objectFree(nb_object _this);

/* Convenience macro for not having to do a cast on every free */
#define nb_objectFree(o) nb__objectFree(nb_object(o))

#ifndef NDEBUG
c_bool          nb__objectIsValid(nb_object _this) __attribute_pure__;
c_bool          nb__objectIsValidKind(nb_object _this, nb_objectKind kind) __attribute_pure__;
#else
#define nb__objectIsValid(o) (TRUE)
#define nb__objectIsValidKind(o, k) (TRUE)
#endif /* NDEBUG */

/* Convenience macro to not having to do a cast on every validation */
#define nb_objectIsValid(o) assert(nb__objectIsValid(nb_object(o)))
#define nb_objectIsValidKind(o, k) assert(nb__objectIsValidKind(nb_object(o), k))

const char*     nb__objectKindImage(nb_object _this) __nonnull_all__
                                                     __attribute_pure__;
/* Convenience macro for not having to do a cast */
#define nb_objectKindImage(o) nb__objectKindImage(nb_object(o))

/****** nb_topicObject ******/

typedef nb_topicObject (* nb_topicObjectAllocFunc)(void);

typedef u_result        (* nb_topicObjectCopyOutFunc)(nb_topicObject, const void *) __nonnull((1));
typedef v_copyin_result (* nb_topicObjectCopyInFunc)(c_type, nb_topicObject, void *) __nonnull_all__;

/* Objects that can be published and/or subscribed to should extend from the
 * abstract nb_topicObject class. Classes implementing the copyOut interface
 * should provide nb_xxxAlloc routine (init done by copyOut. */
C_STRUCT(nb_topicObject){
    C_EXTENDS(nb_object);
    c_char * name;
    v_state state;
    os_timeW writeTime;
    /* Copy-out routine (out from database/shared memory) */
    nb_topicObjectCopyOutFunc copyOut;
    /* Copy-in routine (into database/shared memory) */
    nb_topicObjectCopyInFunc copyIn;
};

#define nb_topicObject(o) ((nb_topicObject)(o))

void            nb__topicObjectInit (nb_topicObject _this,
                                     nb_objectKind kind,
                                     nb_objectDeinitFunc deinit,
                                     const c_char * name,
                                     nb_topicObjectCopyOutFunc copyOut,
                                     nb_topicObjectCopyInFunc copyIn) __nonnull((1,3,4));

void            nb__topicObjectDeinit (nb_object _this) __nonnull_all__;

/* TODO: add more versatile return type? */
u_result        nb_topicObjectCopyOut   (nb_topicObject _this,
                                         const void * from) __nonnull((1));

v_copyin_result nb_topicObjectCopyIn    (c_type type,
                                         const void * from, /* nb_topicObject */
                                         void * to) __nonnull_all__;

u_result        nb_topicObjectWrite     (u_writer writer,
                                         nb_topicObject _this) __nonnull_all__;

const c_char *  nb_topicObjectName      (nb_topicObject _this) __nonnull_all__
                                                               __attribute_returns_nonnull__;

v_state         nb_topicObjectState     (nb_topicObject _this) __nonnull_all__;

v_actionResult  nb_topicObjectReaderAction(c_object o,
                                           c_voidp copyArg, /* c_iter<nb_topicObject> * */
                                           nb_topicObjectAllocFunc allocFunc)
                                        __nonnull((2,3));

#ifndef NDEBUG
void            nb_topicObjectFree      (nb_topicObject _this);
#else
#define         nb_topicObjectFree(s)    nb_objectFree(s)
#endif


#endif /* NB__OBJECT_H_ */

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

#include "nb__topic.h"
#include "nb__service.h"
#include "nb__util.h"
#include "nb__log.h"

#include "u_writer.h"
#include "ut_collection.h"
#include "c_iterator.h"
#include "c_stringSupport.h"
#include "v_builtin.h" /* for built-in partition and topic names */

#include <stddef.h> /* for offsetof */

typedef struct lenField_s {
    c_ulong len;
} lenField;

/**************** nb_dcpsTopic ****************/
C_STRUCT(nb_dcpsTopic) {
    C_EXTENDS(nb_topicObject);
    c_bool interested;
    lenField topic_data;
    /* This will be used to store a copy of the kernel-struct. Strings are
     * strdup'd, sequences are copied as arrays. The length of the arrays is
     * maintained in the above lenField's. */
    struct v_topicInfo info;
};

static nb_topicObject  nb__dcpsTopicAllocFunc(void) __attribute_returns_nonnull__
                                                           __attribute_malloc__;
static v_copyin_result nb__dcpsTopicCopyIn (
                                    c_type type,
                                    nb_topicObject _from, /* nb_dcpsTopic */
                                    void * _to /* struct v_topicInfo* */)
                            __nonnull_all__;

static u_result        nb__dcpsTopicCopyOut(
                                    nb_topicObject _to, /* nb_dcpsTopic */
                                    const void * _from /* struct v_topicInfo * */)
                            __nonnull((1));

/**************** nb_dcpsParticipant ****************/
C_STRUCT(nb_dcpsParticipant) {
    C_EXTENDS(nb_topicObject);
    struct v_participantInfo info;
};

static nb_topicObject  nb__dcpsParticipantAllocFunc(void) __attribute_returns_nonnull__
                                                          __attribute_malloc__;
static v_copyin_result nb__dcpsParticipantCopyIn (
                                    c_type type,
                                    nb_topicObject _from, /* nb_dcpsParticipant */
                                    void * _to /* struct v_participantInfo* */)
                            __nonnull_all__;

static u_result        nb__dcpsParticipantCopyOut(
                                    nb_topicObject _to, /* nb_dcpsParticipant */
                                    const void * _from /* struct v_participantInfo* */)
                            __nonnull((1));

/**************** nb_cmParticipant ****************/
C_STRUCT(nb_cmParticipant) {
    C_EXTENDS(nb_topicObject);
    struct v_participantCMInfo info;
};

static nb_topicObject  nb__cmParticipantAllocFunc(void) __attribute_returns_nonnull__
                                                        __attribute_malloc__;
static v_copyin_result nb__cmParticipantCopyIn (
                                    c_type type,
                                    nb_topicObject _from, /* nb_cmParticipant */
                                    void * _to /* struct v_topicInfo* */)
                            __nonnull_all__;

static u_result        nb__cmParticipantCopyOut(
                                    nb_topicObject _to, /* nb_cmParticipant */
                                    const void * _from /* struct v_topicInfo * */)
                            __nonnull((1));

/**************** nb_dcpsSubscription ****************/
C_STRUCT(nb_dcpsSubscription) {
    C_EXTENDS(nb_topicObject);
    c_bool interested;
    lenField user_data;
    lenField partition;
    lenField topic_data;
    lenField group_data;
    /* This will be used to store a copy of the kernel-struct. Strings are
     * strdup'd, sequences are copied as arrays. The length of the arrays is
     * maintained in the above lenField's. */
    struct v_subscriptionInfo info;
};

static nb_topicObject  nb__dcpsSubscriptionAllocFunc(void) __attribute_returns_nonnull__
                                                           __attribute_malloc__;
static v_copyin_result nb__dcpsSubscriptionCopyIn (
                                    c_type type,
                                    nb_topicObject _from, /* nb_dcpsSubscription */
                                    void * _to /* struct v_subscriptionInfo* */)
                            __nonnull_all__;

static u_result        nb__dcpsSubscriptionCopyOut(
                                    nb_topicObject _to, /* nb_dcpsSubscription */
                                    const void * _from /* struct v_publicationInfo * */)
                            __nonnull((1));

/***************** nb_dcpsPublication ****************/
C_STRUCT(nb_dcpsPublication) {
    C_EXTENDS(nb_topicObject);
    c_bool interested;
    lenField user_data;
    lenField partition;
    lenField topic_data;
    lenField group_data;
    /* This will be used to store a copy of the kernel-struct. Strings are
     * strdup'd, sequences are copied as arrays. The length of the arrays is
     * maintained in the above lenField's. */
    struct v_publicationInfo info;
};

static nb_topicObject  nb__dcpsPublicationAllocFunc(void) __attribute_returns_nonnull__
                                                          __attribute_malloc__;
static v_copyin_result nb__dcpsPublicationCopyIn (
                                    c_type type,
                                    nb_topicObject _from, /* nb_dcpsPublication */
                                    void * _to /* struct v_publicationInfo* */)
                            __nonnull_all__;

static u_result        nb__dcpsPublicationCopyOut(
                                    nb_topicObject _to, /* nb_dcpsPublication */
                                    const void * _from /* struct v_publicationInfo * */)
                            __nonnull((1));


/******************** nb_cmReader ********************/
C_STRUCT(nb_cmReader) {
    C_EXTENDS(nb_topicObject);
    /* This will be used to store a copy of the kernel-struct. Strings are
     * strdup'd. */
    struct v_dataReaderCMInfo info;
};

static nb_topicObject  nb__cmReaderAllocFunc(void) __attribute_returns_nonnull__
                                                   __attribute_malloc__;
static v_copyin_result nb__cmReaderCopyIn (
                                    c_type type,
                                    nb_topicObject _from, /* nb_cmReader */
                                    void * _to /* struct v_dataReaderCMInfo* */)
                            __nonnull_all__;

static u_result        nb__cmReaderCopyOut(
                                    nb_topicObject _to, /* nb_cmReader */
                                    const void * _from /* struct v_dataReaderCMInfo * */)
                            __nonnull((1));

/******************** nb_cmWriter ********************/
C_STRUCT(nb_cmWriter) {
    C_EXTENDS(nb_topicObject);
    /* This will be used to store a copy of the kernel-struct. Strings are
     * strdup'd. */
    struct v_dataWriterCMInfo info;
};

static nb_topicObject  nb__cmWriterAllocFunc(void) __attribute_returns_nonnull__
                                                   __attribute_malloc__;
static v_copyin_result nb__cmWriterCopyIn (
                                    c_type type,
                                    nb_topicObject _from, /* nb_cmWriter */
                                    void * _to /* struct v_dataWriterCMInfo* */)
                            __nonnull_all__;

static u_result        nb__cmWriterCopyOut(
                                    nb_topicObject _to, /* nb_cmWriter */
                                    const void * _from /* struct v_dataWriterCMInfo * */)
                            __nonnull((1));

/******************** nb_cmPublisher ********************/
C_STRUCT(nb_cmPublisher) {
    C_EXTENDS(nb_topicObject);
    lenField partition;
    /* This will be used to store a copy of the kernel-struct. Strings are
     * strdup'd. */
    struct v_publisherCMInfo info;
};

static nb_topicObject  nb__cmPublisherAllocFunc(void) __attribute_returns_nonnull__
                                                      __attribute_malloc__;
static v_copyin_result nb__cmPublisherCopyIn (
                                    c_type type,
                                    nb_topicObject _from, /* nb_cmPublisher */
                                    void * _to /* struct v_publisherCMInfo* */)
                            __nonnull_all__;

static u_result        nb__cmPublisherCopyOut(
                                    nb_topicObject _to, /* nb_cmPublisher */
                                    const void * _from /* struct v_publisherCMInfo * */)
                            __nonnull((1));

/******************** nb_cmSubscriber ********************/
C_STRUCT(nb_cmSubscriber) {
    C_EXTENDS(nb_topicObject);
    /* This will be used to store a copy of the kernel-struct. Strings are
     * strdup'd. */
    lenField partition;
    struct v_subscriberCMInfo info;
};

static nb_topicObject  nb__cmSubscriberAllocFunc(void) __attribute_returns_nonnull__
                                                      __attribute_malloc__;
static v_copyin_result nb__cmSubscriberCopyIn (
                                    c_type type,
                                    nb_topicObject _from, /* nb_cmSubscriber */
                                    void * _to /* struct v_subscriberCMInfo* */)
                            __nonnull_all__;

static u_result        nb__cmSubscriberCopyOut(
                                    nb_topicObject _to, /* nb_cmSubscriber */
                                    const void * _from /* struct v_subscriberCMInfo * */)
                            __nonnull((1));

static v_copyin_result
nb__dcpsTopicCopyIn (
    c_type type,
    nb_topicObject _from, /* nb_dcpsTopic */
    void * _to /* struct v_topicInfo* */)
{
    const nb_dcpsTopic top = nb_dcpsTopic(_from);
    /* This is a *HEAP* copy of the kernel-struct. Strings are malloc'd,
     * sequences have an accompanying len and are arrays. */
    struct v_topicInfo const * const from = &top->info;
    struct v_topicInfo *to = (struct v_topicInfo *)_to;
    c_base base = c_getBase(type);
    c_type seqOctetType = NULL;
    c_type octetType = c_octet_t(base);

    assert(octetType);

    assert(from);
    assert(to);

    *to = *from;

    /* Copy all references by hand; it seems to be impossible to generate
     * proper copy-routines on SAC for the kernel-types. */
    to->name = c_stringNew_s(base, from->name);
    if(from->name && !to->name){
        /* Out of shared memory; warning reported by memory-manager. */
        goto err_name_copy;
    }

    to->type_name = c_stringNew_s(base, from->type_name);
    if(from->type_name && !to->type_name){
        /* Out of shared memory; warning reported by memory-manager. */
        goto err_typename_copy;
    }

    assert(top->topic_data.len || from->topic_data.value == NULL);
    if(top->topic_data.len) {
        if(!seqOctetType){
            /* TODO: should be C_SEQUENCE<c_octet>; now it's consistent with the
             * comment in kernelModule.h, but it is strange. */
            seqOctetType = c_metaArrayTypeNew(c_metaObject(base), "C_ARRAY<c_octet>", octetType, 0);
        }
        assert(seqOctetType); /* This should be resolvable, so no need to check */
        to->topic_data.value = c_newArray_s(c_collectionType(seqOctetType), top->topic_data.len);
        if (!to->topic_data.value) {
            /* Out of shared memory; warning reported by memory-manager. */
            goto err_topic_data_copy;
        }
        memcpy(to->topic_data.value, from->topic_data.value, top->topic_data.len);
    }

    to->meta_data = c_stringNew_s(base, from->meta_data);
    if(from->meta_data && !to->meta_data){
        /* Out of shared memory; warning reported by memory-manager. */
        goto err_meta_data_copy;
    }

    to->key_list = c_stringNew_s(base, from->key_list);
    if(from->key_list && !to->key_list){
        /* Out of shared memory; warning reported by memory-manager. */
        goto err_key_list_copy;
    }

    c_free(seqOctetType);
    return V_COPYIN_RESULT_OK;

/* Error handling */
err_key_list_copy:
    c_free(to->meta_data);
    to->meta_data = NULL;
err_meta_data_copy:
    c_free(to->topic_data.value);
    to->topic_data.value = NULL;
err_topic_data_copy:
    c_free(to->type_name);
    to->type_name = NULL;
err_typename_copy:
    c_free(to->name);
    to->name = NULL;
err_name_copy:
    c_free(seqOctetType);
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
}

static u_result
nb__dcpsTopicCopyOut(
    nb_topicObject _to, /* nb_dcpsTopic */
    const void * _from /* struct v_topicInfo * */)
{
    const nb_dcpsTopic top = nb_dcpsTopic(_to);
    struct v_topicInfo * const to = &top->info;
    struct v_topicInfo const * const from = (struct v_topicInfo *)_from;

    assert(top);

    if(from){
        *to = *from;

        if(from->name) {
            to->name = os_strdup(from->name);
        }
        if(from->type_name) {
            to->type_name = os_strdup(from->type_name);
        }

        /* topic_data */
        top->topic_data.len = c_sequenceSize(from->topic_data.value);
        if(top->topic_data.len) {
            to->topic_data.value = os_malloc(top->topic_data.len * sizeof(*to->topic_data.value));
            memcpy(to->topic_data.value, from->topic_data.value, top->topic_data.len * sizeof(*to->topic_data.value));
        }

        if(from->meta_data) {
            to->meta_data = os_strdup(from->meta_data);
        }

        if(from->key_list){
            to->key_list = os_strdup(from->key_list);
        }

        /* Successfully initialized object now; set proper deinit */
        nb__objectSetDeinit(nb_object(top), nb_dcpsTopicDeinit);
    } else {
        /* Initialize all fields to default values */
        nb_dcpsTopicInit(top);
    }

    return U_RESULT_OK;
}

/* Allocates memory for a new nb_dcpsTopic. This doesn't initialize
 * the object fully. */
nb_dcpsTopic
nb_dcpsTopicAlloc(void)
{
    nb_dcpsTopic _this;

    _this = os_malloc(sizeof *_this);

    /* Only init nb_topicObject */
    nb__topicObjectInit(
        (nb_topicObject)_this,
        NB_TOPIC_OBJECT_DCPS_TOPIC,
        nb__topicObjectDeinit, /* Should be reset to nb_dcpsTopicDeinit after full init by copyOut */
        V_TOPICINFO_NAME,
        nb__dcpsTopicCopyOut,
        nb__dcpsTopicCopyIn);

    /* TODO: match interest */
    _this->interested = TRUE;

    return _this;
}

/* nb_topicObjectAllocFunc signature wrapper */
static nb_topicObject nb__dcpsTopicAllocFunc(void) { return nb_topicObject(nb_dcpsTopicAlloc()); }

/* Allocates and initializes (to 0) a new nb_dcpsTopic */
nb_dcpsTopic
nb_dcpsTopicNew(void)
{
    nb_dcpsTopic _this;

    _this = os_malloc(sizeof *_this);

    nb_dcpsTopicInit(_this);

    return _this;
}

/* Initializes all nb_dcpsTopic fields of _this to 0.  */
void
nb_dcpsTopicInit(
        nb_dcpsTopic _this)
{
    assert(_this);

    /* Super-init */
    nb__topicObjectInit(
        nb_topicObject(_this),
        NB_TOPIC_OBJECT_DCPS_TOPIC,
        nb_dcpsTopicDeinit,
        V_TOPICINFO_NAME,
        nb__dcpsTopicCopyOut,
        nb__dcpsTopicCopyIn);

    /* This compile-time constraint assures that _parent is the first member
     * of nb_serviceTopic and thus sizeof(_this->_parent) can be used
     * to determine the offset. */
    {
        struct nb_dcpsTopic_parent_offset_constraint {
                char _parent_member_at_offset_0 [offsetof(C_STRUCT(nb_dcpsTopic), _parent) == 0];
                char non_empty_dummy_last_member[1];
        };
    }

    /* Only memset the non-_parent part */
    memset((void*)((os_address)_this + sizeof(_this->_parent)), 0, sizeof(*_this) - sizeof(_this->_parent));

    /* TODO: match interest */
    _this->interested = TRUE;
}

void
nb_dcpsTopicDeinit(
    nb_object o /* nb_dcpsTopic */)
{
    nb_dcpsTopic _this = nb_dcpsTopic(o);

    assert(_this);

    /* Object-deinit */
    os_free(_this->info.name);
    os_free(_this->info.type_name);
    os_free(_this->info.topic_data.value);
    os_free(_this->info.meta_data);
    os_free(_this->info.key_list);

    /* Super-deinit */
    nb__topicObjectDeinit(o);
}

#ifndef NDEBUG
/* Type-checking free function */
void
nb_dcpsTopicFree (
        nb_dcpsTopic _this)
{
    if(_this){
        nb_objectIsValidKind(_this, NB_TOPIC_OBJECT_DCPS_TOPIC);
        nb_objectFree(_this);
    }
}
#endif /* NDEBUG */

const c_char *
nb_dcpsTopicTopicName(
        nb_dcpsTopic _this)
{
    nb_objectIsValidKind(_this, NB_TOPIC_OBJECT_DCPS_TOPIC);

    return _this->info.name;
}

const v_builtinTopicKey *
nb_dcpsTopicKey(
        nb_dcpsTopic _this)
{
    nb_objectIsValidKind(_this, NB_TOPIC_OBJECT_DCPS_TOPIC);

    return &_this->info.key;
}

v_actionResult
nb_dcpsTopicReaderAction(
    c_object o,
    c_voidp copyArg /* c_iter<nb_topicObject> * */)
{
    return nb_topicObjectReaderAction(o, copyArg, nb__dcpsTopicAllocFunc);
}


static v_copyin_result
nb__dcpsParticipantCopyIn (
    c_type type,
    nb_topicObject _from, /* nb_dcpsParticipant */
    void * _to /* struct v_participantInfo* */)
{
    const nb_dcpsParticipant part = nb_dcpsParticipant(_from);
    /* This is a *HEAP* copy of the kernel-struct. Strings are malloc'd,
     * sequences have an accompanying len and are arrays. */
    struct v_participantInfo const * const from = &part->info;
    struct v_participantInfo *to = (struct v_participantInfo *)_to;
    c_base base = c_getBase(type);
    c_type seqOctetType = NULL;
    c_type octetType = c_octet_t(base);

    assert(octetType);

    assert(from);
    assert(to);

    *to = *from;

    assert(from->user_data.size || from->user_data.value == NULL);
    if(from->user_data.size) {
        if(!seqOctetType){
            /* TODO: should be C_SEQUENCE<c_octet>; now it's consistent with the
             * comment in kernelModule.h, but it is strange. */
            seqOctetType = c_metaArrayTypeNew(c_metaObject(base), "C_ARRAY<c_octet>", octetType, 0);
        }
        assert(seqOctetType); /* This should be resolvable, so no need to check */
        assert(from->user_data.size >= 0);
        to->user_data.value = c_newArray_s(c_collectionType(seqOctetType), (c_ulong)from->user_data.size);
        if (!to->user_data.value) {
            /* Out of shared memory; warning reported by memory-manager. */
            goto err_user_data_copy;
        }
        memcpy(to->user_data.value, from->user_data.value, (c_ulong)from->user_data.size);
    }

    c_free(seqOctetType);
    return V_COPYIN_RESULT_OK;

/* Error handling */
err_user_data_copy:
    c_free(seqOctetType);
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
}

static u_result
nb__dcpsParticipantCopyOut(
    nb_topicObject _to, /* nb_dcpsParticipant */
    const void * _from /* struct v_participantInfo * */)
{
    const nb_dcpsParticipant part = nb_dcpsParticipant(_to);
    struct v_participantInfo * const to = &part->info;
    struct v_participantInfo const * const from = (struct v_participantInfo *)_from;

    assert(part);

    if(from){
        *to = *from;

        /* user_data */
        assert(from->user_data.size || from->user_data.value == NULL);

        if(from->user_data.size > 0) {
            to->user_data.value = os_malloc((c_ulong)from->user_data.size);
            memcpy(to->user_data.value, from->user_data.value, (c_ulong)from->user_data.size);
        }

        /* Successfully initialized object now; set proper deinit */
        nb__objectSetDeinit(nb_object(part), nb_dcpsParticipantDeinit);
    } else {
        /* Initialize all fields to default values */
        nb_dcpsParticipantInit(part);
    }

    return U_RESULT_OK;
}

/* Allocates memory for a new nb_dcpsParticipant. This doesn't initialize
 * the object fully. */
nb_dcpsParticipant
nb_dcpsParticipantAlloc(void)
{
    nb_dcpsParticipant _this;

    _this = os_malloc(sizeof *_this);

    /* Only init nb_topicObject */
    nb__topicObjectInit(
        (nb_topicObject)_this,
        NB_TOPIC_OBJECT_DCPS_PARTICIPANT,
        nb__topicObjectDeinit, /* Should be reset to nb_dcpsParticipantDeinit after full init by copyOut */
        V_PARTICIPANTINFO_NAME,
        nb__dcpsParticipantCopyOut,
        nb__dcpsParticipantCopyIn);

    return _this;
}

/* nb_topicObjectAllocFunc signature wrapper */
static nb_topicObject nb__dcpsParticipantAllocFunc(void) { return nb_topicObject(nb_dcpsParticipantAlloc()); }

/* Allocates and initializes (to 0) a new nb_dcpsParticipant */
nb_dcpsParticipant
nb_dcpsParticipantNew(void)
{
    nb_dcpsParticipant _this;

    _this = os_malloc(sizeof *_this);

    nb_dcpsParticipantInit(_this);

    return _this;
}

/* Initializes all nb_dcpsParticipant fields of _this to 0.  */
void
nb_dcpsParticipantInit(
        nb_dcpsParticipant _this)
{
    assert(_this);

    /* Super-init */
    nb__topicObjectInit(
        nb_topicObject(_this),
        NB_TOPIC_OBJECT_DCPS_PARTICIPANT,
        nb_dcpsParticipantDeinit,
        V_PARTICIPANTINFO_NAME,
        nb__dcpsParticipantCopyOut,
        nb__dcpsParticipantCopyIn);

    /* This compile-time constraint assures that _parent is the first member
     * of nb_serviceParticipant and thus sizeof(_this->_parent) can be used
     * to determine the offset. */
    {
        struct nb_dcpsParticipant_parent_offset_constraint {
                char _parent_member_at_offset_0 [offsetof(C_STRUCT(nb_dcpsParticipant), _parent) == 0];
                char non_empty_dummy_last_member[1];
        };
    }

    /* Only memset the non-_parent part */
    memset((void*)((os_address)_this + sizeof(_this->_parent)), 0, sizeof(*_this) - sizeof(_this->_parent));
}

void
nb_dcpsParticipantDeinit(
    nb_object o /* nb_dcpsParticipant */)
{
    nb_dcpsParticipant _this = nb_dcpsParticipant(o);

    assert(_this);

    /* Object-deinit */
    os_free(_this->info.user_data.value);

    /* Super-deinit */
    nb__topicObjectDeinit(o);
}

#ifndef NDEBUG
/* Type-checking free function */
void
nb_dcpsParticipantFree (
        nb_dcpsParticipant _this)
{
    if(_this){
        nb_objectIsValidKind(_this, NB_TOPIC_OBJECT_DCPS_PARTICIPANT);
        nb_objectFree(_this);
    }
}
#endif /* NDEBUG */

const v_builtinTopicKey *
nb_dcpsParticipantKey(
        nb_dcpsParticipant _this)
{
    nb_objectIsValidKind(_this, NB_TOPIC_OBJECT_DCPS_PARTICIPANT);

    return &_this->info.key;
}

v_actionResult
nb_dcpsParticipantReaderAction(
    c_object o,
    c_voidp copyArg /* c_iter<nb_topicObject> * */)
{
    return nb_topicObjectReaderAction(o, copyArg, nb__dcpsParticipantAllocFunc);
}

static v_copyin_result
nb__cmParticipantCopyIn (
    c_type type,
    nb_topicObject _from, /* nb_cmParticipant */
    void * _to /* struct v_participantCMInfo* */)
{
    const nb_cmParticipant part = nb_cmParticipant(_from);
    /* This is a *HEAP* copy of the kernel-struct. Strings are malloc'd,
     * sequences have an accompanying len and are arrays. */
    struct v_participantCMInfo const * const from = &part->info;
    struct v_participantCMInfo *to = (struct v_participantCMInfo *)_to;
    c_base base = c_getBase(type);

    assert(from);
    assert(to);

    *to = *from;

    /* Copy all references by hand; it seems to be impossible to generate
     * proper copy-routines on SAC for the kernel-types. */
    to->product.value = c_stringNew_s(base, from->product.value);
    if(from->product.value && !to->product.value){
        /* Out of shared memory; warning reported by memory-manager. */
        goto err_product_copy;
    };

    return V_COPYIN_RESULT_OK;

/* Error handling */
err_product_copy:
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
}

static u_result
nb__cmParticipantCopyOut(
    nb_topicObject _to, /* nb_cmParticipant */
    const void * _from /* struct v_participantInfo * */)
{
    const nb_cmParticipant part = nb_cmParticipant(_to);
    struct v_participantCMInfo * const to = &part->info;
    struct v_participantCMInfo const * const from = (struct v_participantCMInfo *)_from;

    assert(part);

    if(from){
        *to = *from;

        if(from->product.value){
            to->product.value = os_strdup(from->product.value);
        }

        /* Successfully initialized object now; set proper deinit */
        nb__objectSetDeinit(nb_object(part), nb_cmParticipantDeinit);
    } else {
        /* Initialize all fields to default values */
        nb_cmParticipantInit(part);
    }

    return U_RESULT_OK;
}

/* Allocates memory for a new nb_cmParticipant. This doesn't initialize
 * the object fully. */
nb_cmParticipant
nb_cmParticipantAlloc(void)
{
    nb_cmParticipant _this;

    _this = os_malloc(sizeof *_this);

    /* Only init nb_topicObject */
    nb__topicObjectInit(
        (nb_topicObject)_this,
        NB_TOPIC_OBJECT_CM_PARTICIPANT,
        nb__topicObjectDeinit, /* Should be reset to nb_cmParticipantDeinit after full init by copyOut */
        V_CMPARTICIPANTINFO_NAME,
        nb__cmParticipantCopyOut,
        nb__cmParticipantCopyIn);

    return _this;
}

/* nb_topicObjectAllocFunc signature wrapper */
static nb_topicObject
nb__cmParticipantAllocFunc(void)
{
    return nb_topicObject(nb_cmParticipantAlloc());
}


/* Allocates and initializes (to 0) a new nb_cmParticipant */
nb_cmParticipant
nb_cmParticipantNew(void)
{
    nb_cmParticipant _this;

    _this = os_malloc(sizeof *_this);

    nb_cmParticipantInit(_this);

    return _this;
}

/* Initializes all nb_cmParticipant fields of _this to 0.  */
void
nb_cmParticipantInit(
        nb_cmParticipant _this)
{
    assert(_this);

    /* Super-init */
    nb__topicObjectInit(
        nb_topicObject(_this),
        NB_TOPIC_OBJECT_CM_PARTICIPANT,
        nb_cmParticipantDeinit,
        V_CMPARTICIPANTINFO_NAME,
        nb__cmParticipantCopyOut,
        nb__cmParticipantCopyIn);

    /* This compile-time constraint assures that _parent is the first member
     * of nb_serviceParticipant and thus sizeof(_this->_parent) can be used
     * to determine the offset. */
    {
        struct nb_cmParticipant_parent_offset_constraint {
                char _parent_member_at_offset_0 [offsetof(C_STRUCT(nb_cmParticipant), _parent) == 0];
                char non_empty_dummy_last_member[1];
        };
    }

    /* Only memset the non-_parent part */
    memset((void*)((os_address)_this + sizeof(_this->_parent)), 0, sizeof(*_this) - sizeof(_this->_parent));
}

void
nb_cmParticipantDeinit(
    nb_object o /* nb_cmParticipant */)
{
    nb_cmParticipant _this = nb_cmParticipant(o);

    assert(_this);

    /* Object-deinit */
    os_free(_this->info.product.value);

    /* Super-deinit */
    nb__topicObjectDeinit(o);
}

#ifndef NDEBUG
/* Type-checking free function */
void
nb_cmParticipantFree (
        nb_cmParticipant _this)
{
    if(_this){
        nb_objectIsValidKind(_this, NB_TOPIC_OBJECT_CM_PARTICIPANT);
        nb_objectFree(_this);
    }
}
#endif /* NDEBUG */

const v_builtinTopicKey *
nb_cmParticipantKey(
        nb_cmParticipant _this)
{
    nb_objectIsValidKind(_this, NB_TOPIC_OBJECT_CM_PARTICIPANT);

    return &_this->info.key;
}

v_actionResult
nb_cmParticipantReaderAction(
    c_object o,
    c_voidp copyArg /* c_iter<nb_topicObject> * */)
{
    return nb_topicObjectReaderAction(o, copyArg, nb__cmParticipantAllocFunc);
}

static v_copyin_result
nb__dcpsSubscriptionCopyIn (
    c_type type,
    nb_topicObject _from, /* nb_dcpsSubscription */
    void * _to /* struct v_subscriptionInfo* */)
{
    const nb_dcpsSubscription sub = nb_dcpsSubscription(_from);
    /* This is a *HEAP* copy of the kernel-struct. Strings are malloc'd,
     * sequences have an accompanying len and are arrays. */
    struct v_subscriptionInfo const * const from = &sub->info;
    struct v_subscriptionInfo *to = (struct v_subscriptionInfo *)_to;
    c_base base = c_getBase(type);
    c_type seqOctetType = NULL;
    c_type octetType = c_octet_t(base);
    c_type stringType = c_string_t(base);
    c_type seqStringType;

    assert(octetType);
    assert(stringType);

    assert(from);
    assert(to);

    *to = *from;

    /* Copy all references by hand; it seems to be impossible to generate
     * proper copy-routines on SAC for the kernel-types. */
    to->topic_name = c_stringNew_s(base, from->topic_name);
    if(from->topic_name && !to->topic_name){
        /* Out of shared memory; warning reported by memory-manager. */
        goto err_topicname_copy;
    }

    to->type_name = c_stringNew_s(base, from->type_name);
    if(from->type_name && !to->type_name){
        /* Out of shared memory; warning reported by memory-manager. */
        goto err_typename_copy;
    }

    assert(sub->user_data.len || from->user_data.value == NULL);
    if(sub->user_data.len) {
        if(!seqOctetType){
            /* TODO: should be C_SEQUENCE<c_octet>; now it's consistent with the
             * comment in kernelModule.h, but it is strange. */
            seqOctetType = c_metaArrayTypeNew(c_metaObject(base), "C_ARRAY<c_octet>", octetType, 0);
        }
        assert(seqOctetType); /* This should be resolvable, so no need to check */
        to->user_data.value = c_newArray_s(c_collectionType(seqOctetType), sub->user_data.len);
        if (!to->user_data.value) {
            /* Out of shared memory; warning reported by memory-manager. */
            goto err_user_data_copy;
        }
        memcpy(to->user_data.value, from->user_data.value, sub->user_data.len);
    }

    assert(sub->partition.len || from->partition.name == NULL);
    if(sub->partition.len) {
        seqStringType = c_metaSequenceTypeNew(c_metaObject(base), "C_SEQUENCE<c_string>", stringType, 0);
        assert(seqStringType); /* This should be resolvable, so no need to check */
        to->partition.name = c_newSequence_s(c_collectionType(seqStringType), sub->partition.len);
        c_free(seqStringType);
        if (!to->partition.name) {
            /* Out of shared memory; warning reported by memory-manager. */
            goto err_partition_seq_new;
        }

        {
            c_ulong i;

            for(i = 0; i < sub->partition.len; i++){
                to->partition.name[i] = c_stringNew_s(base, from->partition.name[i]);
                if(from->partition.name[i] && !to->partition.name[i]){
                    /* Out of shared memory; warning reported by memory-manager. */
                    goto err_partition_copy;
                }
            }
        }
    }

    assert(sub->topic_data.len || from->topic_data.value == NULL);
    if(sub->topic_data.len) {
        if(!seqOctetType){
            /* TODO: should be C_SEQUENCE<c_octet>; now it's consistent with the
             * comment in kernelModule.h, but it is strange. */
            seqOctetType = c_metaArrayTypeNew(c_metaObject(base), "C_ARRAY<c_octet>", octetType, 0);
        }
        assert(seqOctetType); /* This should be resolvable, so no need to check */
        to->topic_data.value = c_newArray_s(c_collectionType(seqOctetType), sub->topic_data.len);
        if (!to->topic_data.value) {
            /* Out of shared memory; warning reported by memory-manager. */
            goto err_topic_data_copy;
        }
        memcpy(to->topic_data.value, from->topic_data.value, sub->topic_data.len);
    }

    assert(sub->group_data.len || from->group_data.value == NULL);
    if(sub->group_data.len) {
        if(!seqOctetType){
            /* TODO: should be C_SEQUENCE<c_octet>; now it's consistent with the
             * comment in kernelModule.h, but it is strange. */
            seqOctetType = c_metaArrayTypeNew(c_metaObject(base), "C_ARRAY<c_octet>", octetType, 0);
        }
        assert(seqOctetType); /* This should be resolvable, so no need to check */
        to->group_data.value = c_newArray_s(c_collectionType(seqOctetType), sub->group_data.len);
        if (!to->group_data.value) {
            /* Out of shared memory; warning reported by memory-manager. */
            goto err_group_data_copy;
        }
        memcpy(to->group_data.value, from->group_data.value, sub->group_data.len);
    }

    c_free(seqOctetType);
    return V_COPYIN_RESULT_OK;

err_group_data_copy:
    c_free(to->topic_data.value);
    to->topic_data.value = NULL;
err_topic_data_copy:
    /* Sequence-members are freed by c_free */
err_partition_copy:
    c_free(to->partition.name);
    to->partition.name = NULL;
err_partition_seq_new:
    c_free(to->user_data.value);
    to->user_data.value = NULL;
err_user_data_copy:
    c_free(to->type_name);
    to->type_name = NULL;
err_typename_copy:
    c_free(to->topic_name);
    to->topic_name = NULL;
err_topicname_copy:
    c_free(seqOctetType);
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
}

static u_result
nb__dcpsSubscriptionCopyOut(
    nb_topicObject _to, /* nb_dcpsSubscription */
    const void * _from /* struct v_subscribtionInfo * */)
{
    const nb_dcpsSubscription sub = nb_dcpsSubscription(_to);
    struct v_subscriptionInfo * const to = &sub->info;
    struct v_subscriptionInfo const * const from = (struct v_subscriptionInfo *)_from;

    assert(sub);

    if(from){
        *to = *from;

        if(from->topic_name) {
            to->topic_name = os_strdup(from->topic_name);
        }

        if(from->type_name) {
            to->type_name = os_strdup(from->type_name);
        }

        /* user_data */
        sub->user_data.len = c_sequenceSize(from->user_data.value);
        if(sub->user_data.len) {
            to->user_data.value = os_malloc(sub->user_data.len * sizeof(*to->user_data.value));
            memcpy(to->user_data.value, from->user_data.value, sub->user_data.len * sizeof(*to->user_data.value));
        }

        /* partition */
        sub->partition.len = c_arraySize(from->partition.name);
        if(sub->partition.len){
            c_ulong i;

            to->partition.name = os_malloc(sub->partition.len * sizeof(*to->partition.name));

            for(i = 0; i < sub->partition.len; i++){
                assert(from->partition.name[i]);
                to->partition.name[i] = os_strdup(from->partition.name[i]);
            }
        }

        /* topic_data */
        sub->topic_data.len = c_sequenceSize(from->topic_data.value);
        if(sub->topic_data.len) {
            to->topic_data.value = os_malloc(sub->topic_data.len * sizeof(*to->topic_data.value));
            memcpy(to->topic_data.value, from->topic_data.value, sub->topic_data.len * sizeof(*to->topic_data.value));
        }

        /* group_data */
        sub->group_data.len = c_sequenceSize(from->group_data.value);
        if(sub->group_data.len) {
            to->group_data.value = os_malloc(sub->group_data.len * sizeof(*to->group_data.value));
            memcpy(to->group_data.value, from->group_data.value, sub->group_data.len * sizeof(*to->group_data.value));
        }

        /* Successfully initialized object now; set proper deinit */
        nb__objectSetDeinit(nb_object(sub), nb_dcpsSubscriptionDeinit);
    } else {
        /* Initialize all fields to default values */
        nb_dcpsSubscriptionInit(sub);
    }

    return U_RESULT_OK;
}

/* Allocates memory for a new nb_dcpsSubscription. This doesn't initialize
 * the object fully. */
nb_dcpsSubscription
nb_dcpsSubscriptionAlloc(void)
{
    nb_dcpsSubscription _this;

    _this = os_malloc(sizeof *_this);

    /* Only init nb_topicObject */
    nb__topicObjectInit(
        (nb_topicObject)_this,
        NB_TOPIC_OBJECT_DCPS_SUBSCRIPTION,
        nb__topicObjectDeinit, /* Should be reset to nb_dcpsSubscriptionDeinit after full init by copyOut */
        V_SUBSCRIPTIONINFO_NAME,
        nb__dcpsSubscriptionCopyOut,
        nb__dcpsSubscriptionCopyIn);

    /* TODO: match interest */
    _this->interested = TRUE;

    return _this;
}

/* nb_topicObjectAllocFunc signature wrapper */
static nb_topicObject nb__dcpsSubscriptionAllocFunc(void) { return nb_topicObject(nb_dcpsSubscriptionAlloc()); }

/* Allocates and initializes (to 0) a new nb_dcpsSubscription */
nb_dcpsSubscription
nb_dcpsSubscriptionNew(void)
{
    nb_dcpsSubscription _this;

    _this = os_malloc(sizeof *_this);

    nb_dcpsSubscriptionInit(_this);

    return _this;
}

/* Initializes all nb_dcpsSubscription fields of _this to 0.  */
void
nb_dcpsSubscriptionInit(
        nb_dcpsSubscription _this)
{
    assert(_this);

    /* Super-init */
    nb__topicObjectInit(
        nb_topicObject(_this),
        NB_TOPIC_OBJECT_DCPS_SUBSCRIPTION,
        nb_dcpsSubscriptionDeinit,
        V_SUBSCRIPTIONINFO_NAME,
        nb__dcpsSubscriptionCopyOut,
        nb__dcpsSubscriptionCopyIn);

    /* This compile-time constraint assures that _parent is the first member
     * of nb_serviceSubscription and thus sizeof(_this->_parent) can be used
     * to determine the offset. */
    {
        struct nb_dcpsSubscription_parent_offset_constraint {
                char _parent_member_at_offset_0 [offsetof(C_STRUCT(nb_dcpsSubscription), _parent) == 0];
                char non_empty_dummy_last_member[1];
        };
    }

    /* Only memset the non-_parent part */
    memset((void*)((os_address)_this + sizeof(_this->_parent)), 0, sizeof(*_this) - sizeof(_this->_parent));

    /* TODO: match interest */
    _this->interested = TRUE;
}

void
nb_dcpsSubscriptionDeinit(
    nb_object o /* nb_dcpsSubscription */)
{
    nb_dcpsSubscription _this = nb_dcpsSubscription(o);

    assert(_this);

    /* Object-deinit */
    os_free(_this->info.topic_name);
    os_free(_this->info.type_name);
    os_free(_this->info.user_data.value);

    /* partition */
    {
        c_ulong i;

        for(i = 0; i < _this->partition.len; i++){
            os_free(_this->info.partition.name[i]);
        }
        os_free(_this->info.partition.name);
    }
    os_free(_this->info.topic_data.value);
    os_free(_this->info.group_data.value);

    /* Super-deinit */
    nb__topicObjectDeinit(o);
}

#ifndef NDEBUG
/* Type-checking free function */
void
nb_dcpsSubscriptionFree (
        nb_dcpsSubscription _this)
{
    if(_this){
        nb_objectIsValidKind(_this, NB_TOPIC_OBJECT_DCPS_SUBSCRIPTION);
        nb_objectFree(_this);
    }
}
#endif /* NDEBUG */


const c_char *
nb_dcpsSubscriptionTopicName(
        nb_dcpsSubscription _this)
{
    nb_objectIsValidKind(_this, NB_TOPIC_OBJECT_DCPS_SUBSCRIPTION);

    return _this->info.topic_name;
}

const v_builtinTopicKey *
nb_dcpsSubscriptionKey(
        nb_dcpsSubscription _this)
{
    nb_objectIsValidKind(_this, NB_TOPIC_OBJECT_DCPS_SUBSCRIPTION);

    return &_this->info.key;
}

const v_builtinTopicKey *
nb_dcpsSubscriptionParticipantKey(
        nb_dcpsSubscription _this)
{
    nb_objectIsValidKind(_this, NB_TOPIC_OBJECT_DCPS_SUBSCRIPTION);

    return &_this->info.participant_key;
}

const char * const *
nb_dcpsSubscriptionPartitions(
        nb_dcpsSubscription _this,
        c_ulong *len)
{
    nb_objectIsValidKind(_this, NB_TOPIC_OBJECT_DCPS_SUBSCRIPTION);
    assert(len);

    *len = _this->partition.len;

    return (const char * const *) _this->info.partition.name;
}

c_bool
nb_dcpsSubscriptionGetInterested(
        nb_dcpsSubscription _this)
{
    nb_objectIsValidKind(_this, NB_TOPIC_OBJECT_DCPS_SUBSCRIPTION);

    return _this->interested;
}

void
nb_dcpsSubscriptionSetInterested(
        nb_dcpsSubscription _this,
        const char * const * includes,
        const char * const * excludes)
{
    c_bool interested;

    nb_objectIsValidKind(_this, NB_TOPIC_OBJECT_DCPS_SUBSCRIPTION);
    assert(includes);
    assert(excludes);

    interested = nb_match(
            (const char * const *)_this->info.partition.name,
            _this->partition.len,
            nb_dcpsSubscriptionTopicName(_this),
            includes,
            excludes);

    NB_TRACE(("nb_dcpsSubscription={key="NB_KEYFMT"} %s interest for forwarding of %s for topic \'%s\'\n",
                    v_gidSystemId(*nb_dcpsSubscriptionKey(_this)),
                    v_gidLocalId(*nb_dcpsSubscriptionKey(_this)),
                    interested ? "matches" : "doesn't match",
                    nb_topicObjectName(nb_topicObject(_this)),
                    nb_dcpsSubscriptionTopicName(_this)
    ));

    _this->interested = interested;
}

v_actionResult
nb_dcpsSubscriptionReaderAction(
    c_object o,
    c_voidp copyArg /* c_iter<nb_topicObject> * */)
{
    return nb_topicObjectReaderAction(o, copyArg, nb__dcpsSubscriptionAllocFunc);
}

/* nb_dcpsPublication */
static v_copyin_result
nb__dcpsPublicationCopyIn (
    c_type type,
    nb_topicObject _from, /* nb_dcpsPublication */
    void * _to /* struct v_publicationInfo* */)
{
    const nb_dcpsPublication pub = nb_dcpsPublication(_from);
    /* This is a *HEAP* copy of the kernel-struct. Strings are malloc'd,
     * sequences have an accompanying len and are arrays. */
    struct v_publicationInfo const * const from = &pub->info;
    struct v_publicationInfo *to = (struct v_publicationInfo *)_to;
    c_base base = c_getBase(type);
    c_type seqOctetType = NULL;
    c_type octetType = c_octet_t(base);
    c_type stringType = c_string_t(base);
    c_type seqStringType;

    assert(octetType);
    assert(stringType);

    assert(from);
    assert(to);

    *to = *from;

    /* Copy all references by hand; it seems to be impossible to generate
     * proper copy-routines on SAC for the kernel-types. */
    to->topic_name = c_stringNew_s(base, from->topic_name);
    if(from->topic_name && !to->topic_name){
        /* Out of shared memory; warning reported by memory-manager. */
        goto err_topicname_copy;
    }

    to->type_name = c_stringNew_s(base, from->type_name);
    if(from->type_name && !to->type_name){
        /* Out of shared memory; warning reported by memory-manager. */
        goto err_typename_copy;
    }

    assert(pub->user_data.len || from->user_data.value == NULL);
    if(pub->user_data.len) {
        if(!seqOctetType){
            /* TODO: should be C_SEQUENCE<c_octet>; now it's consistent with the
             * comment in kernelModule.h, but it is strange. */
            seqOctetType = c_metaArrayTypeNew(c_metaObject(base), "C_ARRAY<c_octet>", octetType, 0);
        }
        assert(seqOctetType); /* This should be resolvable, so no need to check */
        to->user_data.value = c_newArray_s(c_collectionType(seqOctetType), pub->user_data.len);
        if (!to->user_data.value) {
            /* Out of shared memory; warning reported by memory-manager. */
            goto err_user_data_copy;
        }
        memcpy(to->user_data.value, from->user_data.value, pub->user_data.len);
    }

    assert(pub->partition.len || from->partition.name == NULL);
    if(pub->partition.len) {
        seqStringType = c_metaSequenceTypeNew(c_metaObject(base), "C_SEQUENCE<c_string>", stringType, 0);
        assert(seqStringType); /* This should be resolvable, so no need to check */
        to->partition.name = c_newSequence_s(c_collectionType(seqStringType), pub->partition.len);
        c_free(seqStringType);
        if (!to->partition.name) {
            /* Out of shared memory; warning reported by memory-manager. */
            goto err_partition_seq_new;
        }

        {
            c_ulong i;

            for(i = 0; i < pub->partition.len; i++){
                to->partition.name[i] = c_stringNew_s(base, from->partition.name[i]);
                if(from->partition.name[i] && !to->partition.name[i]){
                    /* Out of shared memory; warning reported by memory-manager. */
                    goto err_partition_copy;
                }
            }
        }
    }

    assert(pub->topic_data.len || from->topic_data.value == NULL);
    if(pub->topic_data.len) {
        if(!seqOctetType){
            /* TODO: should be C_SEQUENCE<c_octet>; now it's consistent with the
             * comment in kernelModule.h, but it is strange. */
            seqOctetType = c_metaArrayTypeNew(c_metaObject(base), "C_ARRAY<c_octet>", octetType, 0);
        }
        assert(seqOctetType); /* This should be resolvable, so no need to check */
        to->topic_data.value = c_newArray_s(c_collectionType(seqOctetType), pub->topic_data.len);
        if (!to->topic_data.value) {
            /* Out of shared memory; warning reported by memory-manager. */
            goto err_topic_data_copy;
        }
        memcpy(to->topic_data.value, from->topic_data.value, pub->topic_data.len);
    }

    assert(pub->group_data.len || from->group_data.value == NULL);
    if(pub->group_data.len) {
        if(!seqOctetType){
            /* TODO: should be C_SEQUENCE<c_octet>; now it's consistent with the
             * comment in kernelModule.h, but it is strange. */
            seqOctetType = c_metaArrayTypeNew(c_metaObject(base), "C_ARRAY<c_octet>", octetType, 0);
        }
        assert(seqOctetType); /* This should be resolvable, so no need to check */
        to->group_data.value = c_newArray_s(c_collectionType(seqOctetType), pub->group_data.len);
        if (!to->group_data.value) {
            /* Out of shared memory; warning reported by memory-manager. */
            goto err_group_data_copy;
        }
        memcpy(to->group_data.value, from->group_data.value, pub->group_data.len);
    }

    c_free(seqOctetType);
    return V_COPYIN_RESULT_OK;

err_group_data_copy:
    c_free(to->topic_data.value);
    to->topic_data.value = NULL;
err_topic_data_copy:
    /* Sequence-members are freed by c_free */
err_partition_copy:
    c_free(to->partition.name);
    to->partition.name = NULL;
err_partition_seq_new:
    c_free(to->user_data.value);
    to->user_data.value = NULL;
err_user_data_copy:
    c_free(to->type_name);
    to->type_name = NULL;
err_typename_copy:
    c_free(to->topic_name);
    to->topic_name = NULL;
err_topicname_copy:
    c_free(seqOctetType);
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
}

static u_result
nb__dcpsPublicationCopyOut(
    nb_topicObject _to, /* nb_dcpsPublication */
    const void * _from /* struct v_publicationInfo * */)
{
    const nb_dcpsPublication pub = nb_dcpsPublication(_to);
    struct v_publicationInfo * const to = &pub->info;
    struct v_publicationInfo const * const from = (struct v_publicationInfo *)_from;

    assert(pub);

    if(from){
        *to = *from;

        if(from->topic_name) {
            to->topic_name = os_strdup(from->topic_name);
        }
        if(from->type_name) {
            to->type_name = os_strdup(from->type_name);
        }

        /* user_data */
        pub->user_data.len = c_sequenceSize(from->user_data.value);
        if(pub->user_data.len) {
            to->user_data.value = os_malloc(pub->user_data.len * sizeof(*to->user_data.value));
            memcpy(to->user_data.value, from->user_data.value, pub->user_data.len * sizeof(*to->user_data.value));
        }

        /* partition */
        pub->partition.len = c_arraySize(from->partition.name);
        if(pub->partition.len){
            c_ulong i;

            to->partition.name = os_malloc(pub->partition.len * sizeof(*to->partition.name));

            for(i = 0; i < pub->partition.len; i++){
                assert(from->partition.name[i]);
                to->partition.name[i] = os_strdup(from->partition.name[i]);
            }
        }

        /* topic_data */
        pub->topic_data.len = c_sequenceSize(from->topic_data.value);
        if(pub->topic_data.len) {
            to->topic_data.value = os_malloc(pub->topic_data.len * sizeof(*to->topic_data.value));
            memcpy(to->topic_data.value, from->topic_data.value, pub->topic_data.len * sizeof(*to->topic_data.value));
        }

        /* group_data */
        pub->group_data.len = c_sequenceSize(from->group_data.value);
        if(pub->group_data.len) {
            to->group_data.value = os_malloc(pub->group_data.len * sizeof(*to->group_data.value));
            memcpy(to->group_data.value, from->group_data.value, pub->group_data.len * sizeof(*to->group_data.value));
        }

        /* Successfully initialized object now; set proper deinit */
        nb__objectSetDeinit(nb_object(pub), nb_dcpsPublicationDeinit);
    } else {
        /* Initialize all fields to default values */
        nb_dcpsPublicationInit(pub);
    }

    return U_RESULT_OK;
}

/* Allocates memory for a new nb_dcpsPublication. This doesn't initialize
 * the object fully. */
nb_dcpsPublication
nb_dcpsPublicationAlloc(void)
{
    nb_dcpsPublication _this;

    _this = os_malloc(sizeof *_this);

    /* Only init nb_topicObject */
    nb__topicObjectInit(
        (nb_topicObject)_this,
        NB_TOPIC_OBJECT_DCPS_PUBLICATION,
        nb__topicObjectDeinit, /* Should be reset to nb_dcpsPublicationDeinit after full init by copyOut */
        V_PUBLICATIONINFO_NAME,
        nb__dcpsPublicationCopyOut,
        nb__dcpsPublicationCopyIn);

    return _this;
}

/* nb_topicObjectAllocFunc signature wrapper */
static nb_topicObject nb__dcpsPublicationAllocFunc(void) { return nb_topicObject(nb_dcpsPublicationAlloc()); }

/* Allocates and initializes (to 0) a new nb_dcpsPublication */
nb_dcpsPublication
nb_dcpsPublicationNew(void)
{
    nb_dcpsPublication _this;

    _this = os_malloc(sizeof *_this);

    nb_dcpsPublicationInit(_this);

    return _this;
}

/* Initializes all nb_dcpsPublication fields of _this to 0.  */
void
nb_dcpsPublicationInit(
        nb_dcpsPublication _this)
{
    assert(_this);

    /* Super-init */
    nb__topicObjectInit(
        nb_topicObject(_this),
        NB_TOPIC_OBJECT_DCPS_PUBLICATION,
        nb_dcpsPublicationDeinit,
        V_PUBLICATIONINFO_NAME,
        nb__dcpsPublicationCopyOut,
        nb__dcpsPublicationCopyIn);

    /* This compile-time constraint assures that _parent is the first member
     * of nb_serviceSubscription and thus sizeof(_this->_parent) can be used
     * to determine the offset. */
    {
        struct nb_dcpsPublication_parent_offset_constraint {
                char _parent_member_at_offset_0 [offsetof(C_STRUCT(nb_dcpsPublication), _parent) == 0];
                char non_empty_dummy_last_member[1];
        };
    }

    /* Only memset the non-_parent part */
    memset((void*)((os_address)_this + sizeof(_this->_parent)), 0, sizeof(*_this) - sizeof(_this->_parent));
}

void
nb_dcpsPublicationDeinit(
    nb_object o /* nb_dcpsPublication */)
{
    nb_dcpsPublication _this = nb_dcpsPublication(o);

    assert(_this);

    /* Object-deinit */
    os_free(_this->info.topic_name);
    os_free(_this->info.type_name);
    os_free(_this->info.user_data.value);

    /* partition */
    {
        c_ulong i;

        for(i = 0; i < _this->partition.len; i++){
            os_free(_this->info.partition.name[i]);
        }
        os_free(_this->info.partition.name);
    }
    os_free(_this->info.topic_data.value);
    os_free(_this->info.group_data.value);

    /* Super-deinit */
    nb__topicObjectDeinit(o);
}

#ifndef NDEBUG
/* Type-checking free function */
void
nb_dcpsPublicationFree (
        nb_dcpsPublication _this)
{
    if(_this){
        nb_objectIsValidKind(_this, NB_TOPIC_OBJECT_DCPS_PUBLICATION);
        nb_objectFree(_this);
    }
}
#endif /* NDEBUG */


const c_char *
nb_dcpsPublicationTopicName(
        nb_dcpsPublication _this)
{
    nb_objectIsValidKind(_this, NB_TOPIC_OBJECT_DCPS_PUBLICATION);

    return _this->info.topic_name;
}

const v_builtinTopicKey *
nb_dcpsPublicationKey(
        nb_dcpsPublication _this)
{
    nb_objectIsValidKind(_this, NB_TOPIC_OBJECT_DCPS_PUBLICATION);

    return &_this->info.key;
}

const v_builtinTopicKey *
nb_dcpsPublicationParticipantKey(
        nb_dcpsPublication _this)
{
    nb_objectIsValidKind(_this, NB_TOPIC_OBJECT_DCPS_PUBLICATION);

    return &_this->info.participant_key;
}

const char * const *
nb_dcpsPublicationPartitions(
        nb_dcpsPublication _this,
        c_ulong *len)
{
    nb_objectIsValidKind(_this, NB_TOPIC_OBJECT_DCPS_PUBLICATION);
    assert(len);

    *len = _this->partition.len;

    return (const char * const *) _this->info.partition.name;
}

c_bool
nb_dcpsPublicationGetInterested(
        nb_dcpsPublication _this)
{
    nb_objectIsValidKind(_this, NB_TOPIC_OBJECT_DCPS_PUBLICATION);

    return _this->interested;
}

void
nb_dcpsPublicationSetInterested(
        nb_dcpsPublication _this,
        const char * const * includes,
        const char * const * excludes)
{
    c_bool interested;

    nb_objectIsValidKind(_this, NB_TOPIC_OBJECT_DCPS_PUBLICATION);
    assert(includes);
    assert(excludes);

    interested = nb_match(
            (const char * const *)_this->info.partition.name,
            _this->partition.len,
            nb_dcpsPublicationTopicName(_this),
            includes,
            excludes);

    NB_TRACE(("nb_dcpsPublication={key="NB_KEYFMT"} %s interest for forwarding of %s for topic \'%s\'\n",
                    v_gidSystemId(*nb_dcpsPublicationKey(_this)),
                    v_gidLocalId(*nb_dcpsPublicationKey(_this)),
                    interested ? "matches" : "doesn't match",
                    nb_topicObjectName(nb_topicObject(_this)),
                    nb_dcpsPublicationTopicName(_this)
    ));

    _this->interested = interested;
}


v_actionResult
nb_dcpsPublicationReaderAction(
    c_object o,
    c_voidp copyArg /* c_iter<nb_topicObject> * */)
{
    return nb_topicObjectReaderAction(o, copyArg, nb__dcpsPublicationAllocFunc);
}

/* nb_cmReader */
static v_copyin_result
nb__cmReaderCopyIn (
    c_type type,
    nb_topicObject _from, /* nb_cmReader */
    void * _to /* struct v_dataReaderCMInfo* */)
{
    const nb_cmReader cmr = nb_cmReader(_from);
    /* This is a *HEAP* copy of the kernel-struct. Strings are malloc'd. */
    struct v_dataReaderCMInfo const * const from = &cmr->info;
    struct v_dataReaderCMInfo *to = (struct v_dataReaderCMInfo *)_to;
    c_base base = c_getBase(type);

    assert(from);
    assert(to);

    *to = *from;

    /* Copy all references by hand; it seems to be impossible to generate
     * proper copy-routines on SAC for the kernel-types. */
    to->product.value = c_stringNew_s(base, from->product.value);
    if(from->product.value && !to->product.value){
        /* Out of shared memory; warning reported by memory-manager. */
        goto err_product_copy;
    };

    to->name = c_stringNew_s(base, from->name);
    if(from->name && !to->name){
       /* Out of shared memory; warning reported by memory-manager. */
       goto err_name_copy;
    }

    to->subscription_keys.expression = c_stringNew_s(base, from->subscription_keys.expression);
    if(from->subscription_keys.expression && !to->subscription_keys.expression){
       /* Out of shared memory; warning reported by memory-manager. */
       goto err_subscription_keys_copy;
    }

    to->share.name = c_stringNew_s(base, from->share.name);
    if(from->share.name && !to->share.name){
       /* Out of shared memory; warning reported by memory-manager. */
       goto err_share_copy;
    }

    return V_COPYIN_RESULT_OK;

err_share_copy:
    c_free(to->subscription_keys.expression);
    to->subscription_keys.expression = NULL;
err_subscription_keys_copy:
    c_free(to->name);
    to->name = NULL;
err_name_copy:
    c_free(to->product.value);
    to->product.value = NULL;
err_product_copy:
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
}

static u_result
nb__cmReaderCopyOut(
    nb_topicObject _to, /* nb_cmReader */
    const void * _from /* struct v_dataReaderCMInfo * */)
{
    const nb_cmReader cmr = nb_cmReader(_to);
    struct v_dataReaderCMInfo * const to = &cmr->info;
    struct v_dataReaderCMInfo const * const from = (struct v_dataReaderCMInfo *)_from;

    assert(cmr);

    if(from){
        *to = *from;

        if(from->product.value){
            to->product.value = os_strdup(from->product.value);
        }

        if(from->name){
            to->name = os_strdup(from->name);
        }

        if(from->subscription_keys.expression){
            to->subscription_keys.expression = os_strdup(from->subscription_keys.expression);
        }

        if(from->share.name){
            to->share.name = os_strdup(from->share.name);
        }

        /* Successfully initialized object now; set proper deinit */
        nb__objectSetDeinit(nb_object(cmr), nb_cmReaderDeinit);
    } else {
        /* Initialize all fields to default values */
        nb_cmReaderInit(cmr);
    }

    return U_RESULT_OK;
}

/* Allocates memory for a new nb_cmReader. This doesn't initialize
 * the object fully. */
nb_cmReader
nb_cmReaderAlloc(void)
{
    nb_cmReader _this;

    _this = os_malloc(sizeof *_this);

    /* Only init nb_topicObject */
    nb__topicObjectInit(
        (nb_topicObject)_this,
        NB_TOPIC_OBJECT_CM_READER,
        nb__topicObjectDeinit, /* Should be reset to nb_cmReaderDeinit after full init by copyOut */
        V_CMDATAREADERINFO_NAME,
        nb__cmReaderCopyOut,
        nb__cmReaderCopyIn);

    return _this;
}

/* nb_topicObjectAllocFunc signature wrapper */
static nb_topicObject
nb__cmReaderAllocFunc(void)
{
    return nb_topicObject(nb_cmReaderAlloc());
}

/* Allocates and initializes (to 0) a new nb_cmReader */
nb_cmReader
nb_cmReaderNew(void)
{
    nb_cmReader _this;

    _this = os_malloc(sizeof *_this);

    nb_cmReaderInit(_this);

    return _this;
}

/* Initializes all nb_cmReader fields of _this to 0.  */
void
nb_cmReaderInit(
        nb_cmReader _this)
{
    assert(_this);

    /* Super-init */
    nb__topicObjectInit(
        nb_topicObject(_this),
        NB_TOPIC_OBJECT_CM_READER,
        nb_cmReaderDeinit,
        V_CMDATAREADERINFO_NAME,
        nb__cmReaderCopyOut,
        nb__cmReaderCopyIn);

    /* This compile-time constraint assures that _parent is the first member
     * of nb_serviceSubscription and thus sizeof(_this->_parent) can be used
     * to determine the offset. */
    {
        struct nb_cmReader_parent_offset_constraint {
                char _parent_member_at_offset_0 [offsetof(C_STRUCT(nb_cmReader), _parent) == 0];
                char non_empty_dummy_last_member[1];
        };
    }

    /* Only memset the non-_parent part */
    memset((void*)((os_address)_this + sizeof(_this->_parent)), 0, sizeof(*_this) - sizeof(_this->_parent));
}

void
nb_cmReaderDeinit(
    nb_object o /* nb_cmReader */)
{
    nb_cmReader _this = nb_cmReader(o);

    assert(_this);

    /* Object-deinit */
    os_free(_this->info.product.value);
    os_free(_this->info.name);
    os_free(_this->info.subscription_keys.expression);
    os_free(_this->info.share.name);

    /* Super-deinit */
    nb__topicObjectDeinit(o);
}

#ifndef NDEBUG
/* Type-checking free function */
void
nb_cmReaderFree (
        nb_cmReader _this)
{
    if(_this){
        nb_objectIsValidKind(_this, NB_TOPIC_OBJECT_CM_READER);
        nb_objectFree(_this);
    }
}
#endif /* NDEBUG */


const v_builtinTopicKey *
nb_cmReaderKey(
        nb_cmReader _this)
{
    nb_objectIsValidKind(_this, NB_TOPIC_OBJECT_CM_READER);

    return &_this->info.key;
}

const v_builtinTopicKey *
nb_cmReaderSubscriberKey(
        nb_cmReader _this)
{
    nb_objectIsValidKind(_this, NB_TOPIC_OBJECT_CM_READER);

    return &_this->info.subscriber_key;
}


v_actionResult
nb_cmReaderReaderAction(
    c_object o,
    c_voidp copyArg /* c_iter<nb_topicObject> * */)
{
    return nb_topicObjectReaderAction(o, copyArg, nb__cmReaderAllocFunc);
}

/* nb_cmWriter */
static v_copyin_result
nb__cmWriterCopyIn (
    c_type type,
    nb_topicObject _from, /* nb_cmWriter */
    void * _to /* struct v_dataWriterCMInfo* */)
{
    const nb_cmWriter cmr = nb_cmWriter(_from);
    /* This is a *HEAP* copy of the kernel-struct. Strings are malloc'd. */
    struct v_dataWriterCMInfo const * const from = &cmr->info;
    struct v_dataWriterCMInfo *to = (struct v_dataWriterCMInfo *)_to;
    c_base base = c_getBase(type);

    assert(from);
    assert(to);

    *to = *from;

    /* Copy all references by hand; it seems to be impossible to generate
     * proper copy-routines on SAC for the kernel-types. */
    to->product.value = c_stringNew_s(base, from->product.value);
    if(from->product.value && !to->product.value){
        /* Out of shared memory; warning reported by memory-manager. */
        goto err_product_copy;
    };

    to->name = c_stringNew_s(base, from->name);
    if(from->name && !to->name){
       /* Out of shared memory; warning reported by memory-manager. */
       goto err_name_copy;
    }
    return V_COPYIN_RESULT_OK;

err_name_copy:
    c_free(to->product.value);
    to->product.value = NULL;
err_product_copy:
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
}

static u_result
nb__cmWriterCopyOut(
    nb_topicObject _to, /* nb_cmWriter */
    const void * _from /* struct v_dataWriterCMInfo * */)
{
    const nb_cmWriter cmw = nb_cmWriter(_to);
    struct v_dataWriterCMInfo * const to = &cmw->info;
    struct v_dataWriterCMInfo const * const from = (struct v_dataWriterCMInfo *)_from;

    assert(cmw);

    if(from){
        *to = *from;

        if(from->product.value){
            to->product.value = os_strdup(from->product.value);
        }

        if(from->name){
            to->name = os_strdup(from->name);
        }

        /* Successfully initialized object now; set proper deinit */
        nb__objectSetDeinit(nb_object(cmw), nb_cmWriterDeinit);
    } else {
        /* Initialize all fields to default values */
        nb_cmWriterInit(cmw);
    }

    return U_RESULT_OK;
}

/* Allocates memory for a new nb_cmWriter. This doesn't initialize
 * the object fully. */
nb_cmWriter
nb_cmWriterAlloc(void)
{
    nb_cmWriter _this;

    _this = os_malloc(sizeof *_this);

    /* Only init nb_topicObject */
    nb__topicObjectInit(
        (nb_topicObject)_this,
        NB_TOPIC_OBJECT_CM_WRITER,
        nb__topicObjectDeinit, /* Should be reset to nb_cmWriterDeinit after full init by copyOut */
        V_CMDATAWRITERINFO_NAME,
        nb__cmWriterCopyOut,
        nb__cmWriterCopyIn);

    return _this;
}

/* nb_topicObjectAllocFunc signature wrapper */
static nb_topicObject nb__cmWriterAllocFunc(void) { return nb_topicObject(nb_cmWriterAlloc()); }

/* Allocates and initializes (to 0) a new nb_cmWriter */
nb_cmWriter
nb_cmWriterNew(void)
{
    nb_cmWriter _this;

    _this = os_malloc(sizeof *_this);

    nb_cmWriterInit(_this);

    return _this;
}

/* Initializes all nb_cmWriter fields of _this to 0.  */
void
nb_cmWriterInit(
        nb_cmWriter _this)
{
    assert(_this);

    /* Super-init */
    nb__topicObjectInit(
        nb_topicObject(_this),
        NB_TOPIC_OBJECT_CM_WRITER,
        nb_cmWriterDeinit,
        V_CMDATAWRITERINFO_NAME,
        nb__cmWriterCopyOut,
        nb__cmWriterCopyIn);

    /* This compile-time constraint assures that _parent is the first member
     * of nb_serviceSubscription and thus sizeof(_this->_parent) can be used
     * to determine the offset. */
    {
        struct nb_cmWriter_parent_offset_constraint {
                char _parent_member_at_offset_0 [offsetof(C_STRUCT(nb_cmWriter), _parent) == 0];
                char non_empty_dummy_last_member[1];
        };
    }

    /* Only memset the non-_parent part */
    memset((void*)((os_address)_this + sizeof(_this->_parent)), 0, sizeof(*_this) - sizeof(_this->_parent));
}

void
nb_cmWriterDeinit(
    nb_object o /* nb_cmWriter */)
{
    nb_cmWriter _this = nb_cmWriter(o);

    assert(_this);

    /* Object-deinit */
    os_free(_this->info.product.value);
    os_free(_this->info.name);

    /* Super-deinit */
    nb__topicObjectDeinit(o);
}

#ifndef NDEBUG
/* Type-checking free function */
void
nb_cmWriterFree (
        nb_cmWriter _this)
{
    if(_this){
        nb_objectIsValidKind(_this, NB_TOPIC_OBJECT_CM_WRITER);
        nb_objectFree(_this);
    }
}
#endif /* NDEBUG */


const v_builtinTopicKey *
nb_cmWriterKey(
        nb_cmWriter _this)
{
    nb_objectIsValidKind(_this, NB_TOPIC_OBJECT_CM_WRITER);

    return &_this->info.key;
}

const v_builtinTopicKey *
nb_cmWriterPublisherKey(
        nb_cmWriter _this)
{
    nb_objectIsValidKind(_this, NB_TOPIC_OBJECT_CM_WRITER);

    return &_this->info.publisher_key;
}

v_actionResult
nb_cmWriterReaderAction(
    c_object o,
    c_voidp copyArg /* c_iter<nb_topicObject> * */)
{
    return nb_topicObjectReaderAction(o, copyArg, nb__cmWriterAllocFunc);
}

/* nb_cmPublisher */
static v_copyin_result
nb__cmPublisherCopyIn (
    c_type type,
    nb_topicObject _from, /* nb_cmPublisher */
    void * _to /* struct v_publisherCMInfo* */)
{
    const nb_cmPublisher cmp = nb_cmPublisher(_from);
    /* This is a *HEAP* copy of the kernel-struct. Strings are malloc'd. */
    struct v_publisherCMInfo const * const from = &cmp->info;
    struct v_publisherCMInfo *to = (struct v_publisherCMInfo *)_to;
    c_base base = c_getBase(type);
    c_type stringType = c_string_t(base);
    c_type seqStringType;

    assert(stringType);

    assert(from);
    assert(to);

    *to = *from;

    /* Copy all references by hand; it seems to be impossible to generate
     * proper copy-routines on SAC for the kernel-types. */
    to->product.value = c_stringNew_s(base, from->product.value);
    if(from->product.value && !to->product.value){
        /* Out of shared memory; warning reported by memory-manager. */
        goto err_product_copy;
    };

    to->name = c_stringNew_s(base, from->name);
    if(from->name && !to->name){
       /* Out of shared memory; warning reported by memory-manager. */
       goto err_name_copy;
    }

    assert(cmp->partition.len || from->partition.name == NULL);
    if(cmp->partition.len) {
        seqStringType = c_metaSequenceTypeNew(c_metaObject(base), "C_SEQUENCE<c_string>", stringType, 0);
        assert(seqStringType); /* This should be resolvable, so no need to check */
        to->partition.name = c_newSequence_s(c_collectionType(seqStringType), cmp->partition.len);
        c_free(seqStringType);
        if (!to->partition.name) {
            /* Out of shared memory; warning reported by memory-manager. */
            goto err_partition_seq_new;
        }

        {
            c_ulong i;

            for(i = 0; i < cmp->partition.len; i++){
                to->partition.name[i] = c_stringNew_s(base, from->partition.name[i]);
                if(from->partition.name[i] && !to->partition.name[i]){
                    /* Out of shared memory; warning reported by memory-manager. */
                    goto err_partition_copy;
                }
            }
        }
    }

    return V_COPYIN_RESULT_OK;

/* Error handling */
err_partition_copy:
    c_free(to->partition.name);
    to->partition.name = NULL;
err_partition_seq_new:
    c_free(to->name);
    to->name = NULL;
err_name_copy:
    c_free(to->product.value);
    to->product.value = NULL;
err_product_copy:
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
}

static u_result
nb__cmPublisherCopyOut(
    nb_topicObject _to, /* nb_cmPublisher */
    const void * _from /* struct v_publisherCMInfo * */)
{
    const nb_cmPublisher cmp = nb_cmPublisher(_to);
    struct v_publisherCMInfo * const to = &cmp->info;
    struct v_publisherCMInfo const * const from = (struct v_publisherCMInfo *)_from;

    assert(cmp);

    if(from){
        *to = *from;

        if(from->product.value){
            to->product.value = os_strdup(from->product.value);
        }

        if(from->name){
            to->name = os_strdup(from->name);
        }

        /* partition */
        cmp->partition.len = c_arraySize(from->partition.name);
        if(cmp->partition.len){
            c_ulong i;

            to->partition.name = os_malloc(cmp->partition.len * sizeof(*to->partition.name));

            for(i = 0; i < cmp->partition.len; i++){
                assert(from->partition.name[i]);
                to->partition.name[i] = os_strdup(from->partition.name[i]);
            }
        }

        /* Successfully initialized object now; set proper deinit */
        nb__objectSetDeinit(nb_object(cmp), nb_cmPublisherDeinit);
    } else {
        /* Initialize all fields to default values */
        nb_cmPublisherInit(cmp);
    }

    return U_RESULT_OK;
}

/* Allocates memory for a new nb_cmPublisher. This doesn't initialize
 * the object fully. */
nb_cmPublisher
nb_cmPublisherAlloc(void)
{
    nb_cmPublisher _this;

    _this = os_malloc(sizeof *_this);

    /* Only init nb_topicObject */
    nb__topicObjectInit(
        (nb_topicObject)_this,
        NB_TOPIC_OBJECT_CM_PUBLISHER,
        nb__topicObjectDeinit, /* Should be reset to nb_cmPublisherDeinit after full init by copyOut */
        V_CMPUBLISHERINFO_NAME,
        nb__cmPublisherCopyOut,
        nb__cmPublisherCopyIn);

    return _this;
}

/* nb_topicObjectAllocFunc signature wrapper */
static nb_topicObject
nb__cmPublisherAllocFunc(void)
{
    return nb_topicObject(nb_cmPublisherAlloc());
}

/* Allocates and initializes (to 0) a new nb_cmPublisher */
nb_cmPublisher
nb_cmPublisherNew(void)
{
    nb_cmPublisher _this;

    _this = os_malloc(sizeof *_this);

    nb_cmPublisherInit(_this);

    return _this;
}

/* Initializes all nb_cmPublisher fields of _this to 0.  */
void
nb_cmPublisherInit(
        nb_cmPublisher _this)
{
    assert(_this);

    /* Super-init */
    nb__topicObjectInit(
        nb_topicObject(_this),
        NB_TOPIC_OBJECT_CM_PUBLISHER,
        nb_cmPublisherDeinit,
        V_CMPUBLISHERINFO_NAME,
        nb__cmPublisherCopyOut,
        nb__cmPublisherCopyIn);

    /* This compile-time constraint assures that _parent is the first member
     * of nb_serviceSubscription and thus sizeof(_this->_parent) can be used
     * to determine the offset. */
    {
        struct nb_cmPublisher_parent_offset_constraint {
                char _parent_member_at_offset_0 [offsetof(C_STRUCT(nb_cmPublisher), _parent) == 0];
                char non_empty_dummy_last_member[1];
        };
    }

    /* Only memset the non-_parent part */
    memset((void*)((os_address)_this + sizeof(_this->_parent)), 0, sizeof(*_this) - sizeof(_this->_parent));
}

void
nb_cmPublisherDeinit(
    nb_object o /* nb_cmPublisher */)
{
    nb_cmPublisher _this = nb_cmPublisher(o);

    assert(_this);

    /* Object-deinit */
    os_free(_this->info.product.value);
    os_free(_this->info.name);

    /* partition */
    {
        c_ulong i;

        for(i = 0; i < _this->partition.len; i++){
            os_free(_this->info.partition.name[i]);
        }
        os_free(_this->info.partition.name);
    }

    /* Super-deinit */
    nb__topicObjectDeinit(o);
}

#ifndef NDEBUG
/* Type-checking free function */
void
nb_cmPublisherFree (
        nb_cmPublisher _this)
{
    if(_this){
        nb_objectIsValidKind(_this, NB_TOPIC_OBJECT_CM_PUBLISHER);
        nb_objectFree(_this);
    }
}
#endif /* NDEBUG */

const v_builtinTopicKey *
nb_cmPublisherKey(
        nb_cmPublisher _this)
{
    nb_objectIsValidKind(_this, NB_TOPIC_OBJECT_CM_PUBLISHER);

    return &_this->info.key;
}

const v_builtinTopicKey *
nb_cmPublisherParticipantKey(
        nb_cmPublisher _this)
{
    nb_objectIsValidKind(_this, NB_TOPIC_OBJECT_CM_PUBLISHER);

    return &_this->info.participant_key;
}

v_actionResult
nb_cmPublisherReaderAction(
    c_object o,
    c_voidp copyArg /* c_iter<nb_topicObject> * */)
{
    return nb_topicObjectReaderAction(o, copyArg, nb__cmPublisherAllocFunc);
}


/* nb_cmSubscriber */
static v_copyin_result
nb__cmSubscriberCopyIn (
    c_type type,
    nb_topicObject _from, /* nb_cmSubscriber */
    void * _to /* struct v_subscriberCMInfo* */)
{
    const nb_cmSubscriber cms = nb_cmSubscriber(_from);
    /* This is a *HEAP* copy of the kernel-struct. Strings are malloc'd. */
    struct v_subscriberCMInfo const * const from = &cms->info;
    struct v_subscriberCMInfo *to = (struct v_subscriberCMInfo *)_to;
    c_base base = c_getBase(type);
    c_type stringType = c_string_t(base);
    c_type seqStringType;

    assert(stringType);

    assert(from);
    assert(to);

    *to = *from;

    /* Copy all references by hand; it seems to be impossible to generate
     * proper copy-routines on SAC for the kernel-types. */
    to->product.value = c_stringNew_s(base, from->product.value);
    if(from->product.value && !to->product.value){
        /* Out of shared memory; warning reported by memory-manager. */
        goto err_product_copy;
    };

    to->name = c_stringNew_s(base, from->name);
    if(from->name && !to->name){
       /* Out of shared memory; warning reported by memory-manager. */
       goto err_name_copy;
    }

    to->share.name = c_stringNew_s(base, from->share.name);
    if(from->share.name && !to->share.name){
       /* Out of shared memory; warning reported by memory-manager. */
       goto err_share_copy;
    }

    assert(cms->partition.len || from->partition.name == NULL);
    if(cms->partition.len) {
        seqStringType = c_metaSequenceTypeNew(c_metaObject(base), "C_SEQUENCE<c_string>", stringType, 0);
        assert(seqStringType); /* This should be resolvable, so no need to check */
        to->partition.name = c_newSequence_s(c_collectionType(seqStringType), cms->partition.len);
        c_free(seqStringType);
        if (!to->partition.name) {
            /* Out of shared memory; warning reported by memory-manager. */
            goto err_partition_seq_new;
        }

        {
            c_ulong i;

            for(i = 0; i < cms->partition.len; i++){
                to->partition.name[i] = c_stringNew_s(base, from->partition.name[i]);
                if(from->partition.name[i] && !to->partition.name[i]){
                    /* Out of shared memory; warning reported by memory-manager. */
                    goto err_partition_copy;
                }
            }
        }
    }

    return V_COPYIN_RESULT_OK;

/* Error handling */
err_partition_copy:
    c_free(to->partition.name);
    to->partition.name = NULL;
err_partition_seq_new:
    c_free(to->share.name);
    to->share.name = NULL;
err_share_copy:
    c_free(to->name);
    to->name = NULL;
err_name_copy:
    c_free(to->product.value);
    to->product.value = NULL;
err_product_copy:
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
}

static u_result
nb__cmSubscriberCopyOut(
    nb_topicObject _to, /* nb_cmSubscriber */
    const void * _from /* struct v_subscriberCMInfo * */)
{
    const nb_cmSubscriber cms = nb_cmSubscriber(_to);
    struct v_subscriberCMInfo * const to = &cms->info;
    struct v_subscriberCMInfo const * const from = (struct v_subscriberCMInfo *)_from;

    assert(cms);

    if(from){
        *to = *from;

        if(from->product.value){
            to->product.value = os_strdup(from->product.value);
        }

        if(from->name){
            to->name = os_strdup(from->name);
        }

        if(from->share.name){
            to->share.name = os_strdup(from->share.name);
        }

        /* partition */
        cms->partition.len = c_arraySize(from->partition.name);
        if(cms->partition.len){
            c_ulong i;

            to->partition.name = os_malloc(cms->partition.len * sizeof(*to->partition.name));

            for(i = 0; i < cms->partition.len; i++){
                assert(from->partition.name[i]);
                to->partition.name[i] = os_strdup(from->partition.name[i]);
            }
        }


        /* Successfully initialized object now; set proper deinit */
        nb__objectSetDeinit(nb_object(cms), nb_cmSubscriberDeinit);
    } else {
        /* Initialize all fields to default values */
        nb_cmSubscriberInit(cms);
    }

    return U_RESULT_OK;
}

/* Allocates memory for a new nb_cmSubscriber. This doesn't initialize
 * the object fully. */
nb_cmSubscriber
nb_cmSubscriberAlloc(void)
{
    nb_cmSubscriber _this;

    _this = os_malloc(sizeof *_this);

    /* Only init nb_topicObject */
    nb__topicObjectInit(
        (nb_topicObject)_this,
        NB_TOPIC_OBJECT_CM_SUBSCRIBER,
        nb__topicObjectDeinit, /* Should be reset to nb_cmSubscriberDeinit after full init by copyOut */
        V_CMSUBSCRIBERINFO_NAME,
        nb__cmSubscriberCopyOut,
        nb__cmSubscriberCopyIn);

    return _this;
}

/* nb_topicObjectAllocFunc signature wrapper */
static nb_topicObject
nb__cmSubscriberAllocFunc(void)
{
    return nb_topicObject(nb_cmSubscriberAlloc());
}

/* Allocates and initializes (to 0) a new nb_cmSubscriber */
nb_cmSubscriber
nb_cmSubscriberNew(void)
{
    nb_cmSubscriber _this;

    _this = os_malloc(sizeof *_this);

    nb_cmSubscriberInit(_this);

    return _this;
}

/* Initializes all nb_cmSubscriber fields of _this to 0.  */
void
nb_cmSubscriberInit(
        nb_cmSubscriber _this)
{
    assert(_this);

    /* Super-init */
    nb__topicObjectInit(
        nb_topicObject(_this),
        NB_TOPIC_OBJECT_CM_SUBSCRIBER,
        nb_cmSubscriberDeinit,
        V_CMSUBSCRIBERINFO_NAME,
        nb__cmSubscriberCopyOut,
        nb__cmSubscriberCopyIn);

    /* This compile-time constraint assures that _parent is the first member
     * of nb_serviceSubscription and thus sizeof(_this->_parent) can be used
     * to determine the offset. */
    {
        struct nb_cmSubscriber_parent_offset_constraint {
                char _parent_member_at_offset_0 [offsetof(C_STRUCT(nb_cmSubscriber), _parent) == 0];
                char non_empty_dummy_last_member[1];
        };
    }

    /* Only memset the non-_parent part */
    memset((void*)((os_address)_this + sizeof(_this->_parent)), 0, sizeof(*_this) - sizeof(_this->_parent));
}

void
nb_cmSubscriberDeinit(
    nb_object o /* nb_cmSubscriber */)
{
    nb_cmSubscriber _this = nb_cmSubscriber(o);

    assert(_this);

    /* Object-deinit */
    os_free(_this->info.product.value);
    os_free(_this->info.name);
    os_free(_this->info.share.name);

    /* partition */
    {
        c_ulong i;

        for(i = 0; i < _this->partition.len; i++){
            os_free(_this->info.partition.name[i]);
        }
        os_free(_this->info.partition.name);
    }

    /* Super-deinit */
    nb__topicObjectDeinit(o);
}

#ifndef NDEBUG
/* Type-checking free function */
void
nb_cmSubscriberFree (
        nb_cmSubscriber _this)
{
    if(_this){
        nb_objectIsValidKind(_this, NB_TOPIC_OBJECT_CM_SUBSCRIBER);
        nb_objectFree(_this);
    }
}
#endif /* NDEBUG */

const v_builtinTopicKey *
nb_cmSubscriberKey(
        nb_cmSubscriber _this)
{
    nb_objectIsValidKind(_this, NB_TOPIC_OBJECT_CM_SUBSCRIBER);

    return &_this->info.key;
}

v_actionResult
nb_cmSubscriberReaderAction(
    c_object o,
    c_voidp copyArg /* c_iter<nb_topicObject> * */)
{
    return nb_topicObjectReaderAction(o, copyArg, nb__cmSubscriberAllocFunc);
}

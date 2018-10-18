/* -*- mode: c; c-file-style: "k&r"; c-basic-offset: 4; -*- */

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
#include "vortex_os.h"
#include "ut_avl.h"
#include "d__misc.h"
#include "d_storeKV.h"
#include "d_store.h"
#include "d__nameSpace.h"
#include "d__configuration.h"
#include "d__table.h"
#include "d_object.h"
#include "d__types.h"
#include "d__store.h"
#include "d__pQos.h"
#include "d__thread.h"
#include "d__groupHash.h"
#include "u_user.h"
#include "u_group.h"
#include "u_observable.h"
#include "u_entity.h"
#include "u_partition.h"
#include "u_topic.h"
#include "v_public.h"
#include "v_entity.h"
#include "v_time.h"
#include "v_topicQos.h"
#include "v_topic.h"
#include "v_message.h"
#include "v_messageExt.h"
#include "v_partition.h"
#include "v_state.h"
#include "sd_serializer.h"
#include "sd_serializerXML.h"
#include "sd_serializerBigE.h"
#include "c_base.h"
#include "c_laptime.h"
#include "c_field.h"
#include "os_heap.h"
#include "os_time.h"
#include "os_stdlib.h"
#include "os_report.h"
#include "os_socket.h"
#include "sd_cdr.h"
#include "ut_compressor.h"

#include <stddef.h>
#include <limits.h>
#include <ctype.h>


#define KV_INCLUDE_DUMMY 1


/* Explicitly mapping identifiers to numerical values because the
 * numerical values are what is potentially relevant to backwards
 * compatibility. */
typedef enum kv_table {
    TABLE_VERSION = 0,
    TABLE_TOPIC = 1, TABLE_TOPIC_TYPE = 2,
    TABLE_NAMESPACE = 3, TABLE_NAMESPACE_QUALITY = 4, TABLE_NAMESPACE_COMPLETENESS = 5,
    TABLE_GROUP = 6,
    TABLE_MESSAGE = 7,
    TABLE_MESSAGE_EOT = 8,
} kv_table_t;
#define N_TABLES 9


typedef struct kv_key {
    enum kv_table table;
    os_uint32 gid; /* TABLE_VERSION: 0; else topic, namespace or group id, depending on table */
    os_uint32 iid; /* TABLE_MESSAGE: instance id; others: 0 */
    os_uint64 mid; /* TABLE_MESSAGE: message id; others: 0 */
} kv_key_t;

typedef enum kv_kind {
    KV_KIND_SQLITE,
    KV_KIND_LEVELDB,
    KV_KIND_LSDS,
    KV_KIND_DUMMY,
    KV_KIND_SQLITEMT
} kv_kind_t;


typedef enum kv_commit_state {
    UNDECIDED,
    OPEN,          /* Begin marker of transaction received, but no end-marker */
    COMMITTED      /* End of transaction has been received */
} kv_commit_state_t;


/* Maximum GID is less than the full range because we reserve the 8
 * bits for encoding the table id, just in case when end up using a
 * backing K-V store that benefits from that possibility.  2**24
 * groups should be enough; if it is not, we can reasonably easily
 * change this. */
#define MAX_GID 0x00ffffffu

/* kv_result: 0 is success, all errors < 0 */
typedef int kv_result_t;
#define KV_RESULT_ACCESS_DENIED -1
#define KV_RESULT_NODATA -2
#define KV_RESULT_ERROR -3
#define KV_RESULT_CORRUPT -4
#define KV_RESULT_OUT_OF_RESOURCES -5

typedef struct kv_store_common *kv_store_t;
typedef struct kv_iter_common *kv_iter_t;


struct transaction_msg {
    /* List that contains references to transactional samples
     * with the same transactionId. The samples themselves
     * are maintained in the instance. There is no need
     * for a double-linked list, a single list will do. */
    struct msg *msg_ref;
    struct transaction_msg *next;
};

struct transaction_list {
    v_gid gid;       /* the writer that wrote the transaction */
    c_ulong seqNum;  /* the transactionId (unique per writer) */
    os_uint64 otid;  /* first free id in table of open transactions */
    kv_commit_state_t commit_state;
    struct transaction_msg *head;
    struct transaction_msg *tail;
};

struct msg {
    /* Older/newer: double linked list of messages for one instance,
     * sorted either on time stamp or in reception order, depending on
     * the DESTINATION_ORDER QoS */
    struct msg *older;
    struct msg *newer;

    /* Backreference to the instance to which the message belongs */
    struct inst *inst;

    /* Message key used in the K-V store for this message */
    kv_key_t key;

    /* Fields copied from the v_message so we don't have to keep a
     * reference to it at all times.  For normal persistency, we would
     * only have to keep references to data that has to be in shared
     * memory anyway, and hence this isn't really necessary.  On the
     * other hand, if we ever want to use this framework for other
     * (logging) purposes, it would become an issue. */
    v_state state;
    os_timeW writeTime;
    v_gid writerGID;
    c_ulong sequenceNumber;
    c_ulong transactionId;

    /* During reconstruction/injection, we deserialise the data and
     * keep a reference to it until we have inserted it in the
     * group. */
    v_message v_msg;
};

struct inst {
    /* Instance key is next_msg_key.{table,gid,iid}, next_msg_key is
     * the next free message id. */
    kv_key_t next_msg_key;

    /* instances_by_iid uses avlnode and has some things maintained by
     * inst_augment: lowest and highest iid in subtree and 0 or 1 + an
     * unusued iid. */
    ut_avlNode_t avlnode;
    os_uint32 iid_min;
    os_uint32 iid_max;
    os_uint32 iid_holep1;

    /* We treat the key as a blob once it has been extracted from the
     * message.  Although we do define an ordering over the keyblob so
     * that we can use it as a key in, e.g., a binary search tree, it
     * is generally not a meaningful one. */
    size_t keysize;
    void *keyvalue;

    /* Reference to groupList associated with the instance */
    struct d_groupListKV_s *g;

    /* The messages associated with this instance.  writeCount is the
     * number of writes and write-disposes -- that is, the number of
     * messages with the L_WRITE flag set. Messages that belong to
     * a coherent transaction are not counted, so if the L_TRANSACTION
     * flag is set the writeCount is not affected. */
    c_long writeCount;
    struct msg *oldest;
    struct msg *latest;                  /* undefined if oldest == NULL */
    struct msg *oldest_non_transaction;  /* NULL if not present */

};

struct namespace {
    os_uint32 id;               /* local numerical id (for TABLE_NAMESPACE) */
    os_uint32 version;          /* version number of namespace (for backup/restore), starts at 0 */
    char *name;                 /* namespace name, as extracted from durability */
    d_quality quality;          /* current quality of namespace */
    int on_disk;                /* 0 if not yet written to disk */
    struct namespace *next;     /* next namespace in list sorted s.t. (id, v) always precedes (id, w) if v > w */
};

struct ser_deser_ctx {
    struct sd_cdrInfo *ci;
    c_type xmsgType;
};

C_STRUCT (d_groupListKV) {
    C_EXTENDS (d_groupList);
    unsigned group_id;             /* local numerical id for group */
    unsigned topic_id;             /* same for all groups with this topic */
    d_table instances;             /* set of instances in this group */
    ut_avlTree_t instances_by_iid; /* aliases instances, ordered by iid and used for finding free iids */
    struct namespace *namespace;   /* versioned namespace to which this group belongs */
    v_group vgroup;             /* reference to the corresponding v_group (NOT written to disk)*/
    struct ser_deser_ctx tctx;
};
C_CLASS (d_groupListKV);

C_STRUCT (d_storeKV) {
    C_EXTENDS (d_store);
    c_base base;                /* cached base pointer */
    c_bool opened;              /* whether currently between ...Open and ...Close */
    d_groupListKV groups;       /* discovered groups */
    struct namespace *namespaces; /* discovered namespaces */
    kv_store_t kv;              /* backing key-value store */
    c_char *diskStorePath;      /* path to persistent directory */
    d_encodingKV encoding;      /* payload encoding used in file */

    /* These are used to emulate a mutex in such a way that we can notify the
     * liveliness mechanism we're alive while waiting for the store to become
     * unlocked. The problem is that some operations on very large persistent
     * data sets can take very long, so long that calling d_lockLock(st) would
     * cause the thread to lose liveliness. */
    int locked;
    os_mutex lock;
    os_cond cond;

    struct hashtable *groupTable;

    /* Next free id for groups, topics and namespaces */
    os_uint32 next_group_id;
    os_uint32 next_topic_id;
    os_uint32 next_namespace_id;

    /* Cached pointers to the various KV-store specific payload types
     * defined in durability.odl, used for (de)serialisation */
    c_type topic_type;
    c_type topicinfo_type;
    c_type namespace_type;
    c_type namespace_quality_type;
    c_type namespace_completeness_type;
    c_type group_type;
    c_type open_transaction_eot_type;
    c_type open_transaction_vtid_type;

    struct ser_deser_ctx topic_tctx;
    struct ser_deser_ctx topicinfo_tctx;
    struct ser_deser_ctx namespace_tctx;
    struct ser_deser_ctx namespace_quality_tctx;
    struct ser_deser_ctx namespace_completeness_tctx;
    struct ser_deser_ctx group_tctx;
    struct ser_deser_ctx open_transaction_eot_tctx;
    struct ser_deser_ctx open_transaction_vtid_tctx;


    /* table containing the updated namespaces for which the quality has to be
     * updated on disk */
    struct hashtable *updated_namespaces;

    /* Support for benchmark KV store */
    c_bool  action_started;
    os_timeE first_time;
    os_timeE last_time;

    /* Table of open transactions for which no EOT has been received.
     * Each entry in the table contains a list of references to
     * samples that belong to the transaction. */
    d_table open_transactions;

    /* First free id for open transactions.
     * We use a 64-bit of which 63 bits are used for the key
     * and 1 bit to indicate if the transaction is open (0)
     * or committed (1). This gives 2^63 possible keys which
     * is sufficient to generate ids for almost 300 years
     * at a rate of 1GHz ... */
    os_uint64 otid;

    d_compressionKV compression;
    ut_compressor compressor;
};

static void kvlog (const struct d_storeKV_s *st, d_level level, const char *fmt, ...);
static void *serialize (os_uint32 *sz, d_storeKV st, const struct c_type_s *type, struct ser_deser_ctx *ctx, const void *obj);
static kv_result_t deserialize (void **obj, d_storeKV st, const struct c_type_s *type, struct ser_deser_ctx *ctx, os_uint32 sz, const void *val);

#define ASSERT_STOREKV_NORET(object) do {                       \
        assert (d_objectIsValid (d_object (object), D_STORE));  \
        assert (((d_store) (object))->type == D_STORE_TYPE_KV); \
    } while (0)

#define ASSERT_STOREKV(st, object) do {                         \
        if ((object) == NULL) {                                 \
            return D_STORE_RESULT_ILL_PARAM;                    \
        }                                                       \
        ASSERT_STOREKV_NORET (object);                          \
        st = (d_storeKV) (object);                              \
    } while (0)

static void store_wait_for_lock (struct d_storeKV_s *st)
{
    if (st->locked) {
        d_thread self = d_threadLookupSelf ();
        const os_duration maxwait = OS_DURATION_INIT(180, 0);  /* 3 min */
        os_timeM tnow = os_timeMGet ();
        const os_timeM tend = os_timeMAdd (tnow, maxwait);
        /* Try to acquire the lock for maxwait sec. while asserting liveliness. */
        os_duration timeout = os_timeMDiff(tend, tnow);
        while (st->locked && os_durationCompare (timeout, OS_DURATION_ZERO) == OS_MORE) {
            (void) d_condTimedWait (self, &st->cond, &st->lock, timeout);
            tnow = os_timeMGet ();
            timeout = os_timeMDiff(tend, tnow);
        }
        if (st->locked)
        {
            /* After waiting for maxwait, switch to using os_condWait because that one
             * doesn't maintain liveliness, and we think something is badly wrong. */
            kvlog (st, D_LEVEL_WARNING, "waiting more than %"PA_PRIduration" s for persistent store lock",
                    OS_DURATION_PRINT(maxwait));
            while (st->locked) {
                os_condWait (&st->cond, &st->lock);
            }
        }
    }
    assert (!st->locked);
}

#define LOCK_OPEN_STOREKV(st, object) do {              \
        ASSERT_STOREKV (st, object);                    \
        os_mutexLock (&st->lock);                       \
        store_wait_for_lock (st);                       \
        if (!st->opened) {                              \
            os_mutexUnlock (&st->lock);                 \
            return D_STORE_RESULT_PRECONDITION_NOT_MET; \
        }                                               \
        st->locked = 1;                                 \
        os_mutexUnlock (&st->lock);                     \
    } while (0)

#define UNLOCK_OPEN_STOREKV(st) do { \
        os_mutexLock (&st->lock);    \
        st->locked = 0;              \
        os_condSignal (&st->cond);   \
        os_mutexUnlock (&st->lock);  \
    } while (0)

#define CONSTR_UINT64(x, y, z) (((os_uint64) (x) * 1000000 + (y)) * 1000000 + (z))
static const os_uint64 hashTableConst =  CONSTR_UINT64(14585777, 916479, 446343);

void          d_storeDeinitKV                   (d_storeKV store);
d_storeKV     d_storeNewKV                      (u_participant participant);
d_storeResult d_storeFreeKV                     (d_storeKV store);
d_storeResult d_storeOpenKV                     (d_store store);
d_storeResult d_storeCloseKV                    (d_store store);
d_storeResult d_storeActionStartKV              (const d_store store);
d_storeResult d_storeActionStopKV               (const d_store store);
d_storeResult d_storeGetQualityKV               (const d_store store, const d_nameSpace nameSpace, os_timeW* quality);
d_storeResult d_storeBackupKV                   (const d_store store, const d_nameSpace nameSpace);
d_storeResult d_storeRestoreBackupKV            (const d_store store, const d_nameSpace nameSpace);
d_storeResult d_storeGroupsReadKV               (const d_store store, d_groupList *list);
d_storeResult d_storeGroupListFreeKV            (const d_store store, d_groupList list);
d_storeResult d_storeGroupInjectKV              (const d_store store, const c_char* partition, const c_char* topic, const u_participant participant, d_group *group);
d_storeResult d_storeGroupStoreKV               (const d_store store, const d_group group, const d_nameSpace nameSpace);
d_storeResult d_storeMessageStoreKV             (const d_store store, const v_groupAction message);
d_storeResult d_storeInstanceDisposeKV          (const d_store store, const v_groupAction message);
d_storeResult d_storeInstanceExpungeKV          (const d_store store, const v_groupAction message);
d_storeResult d_storeMessageExpungeKV           (const d_store store, const v_groupAction message);
d_storeResult d_storeDeleteHistoricalDataKV     (const d_store store, const v_groupAction message);
d_storeResult d_storeMessagesInjectKV           (const d_store store, const d_group group);
d_storeResult d_storeInstanceRegisterKV         (const d_store store, const v_groupAction message);
d_storeResult d_storeCreatePersistentSnapshotKV (const d_store store, const c_char* partitionExpr, const c_char* topicExpr, const c_char* uri);
d_storeResult d_storeInstanceUnregisterKV       (const d_store store, const v_groupAction message);
d_storeResult d_storeOptimizeGroupKV            (const d_store store, const d_group group);
d_storeResult d_storeNsIsCompleteKV             (const d_store store, const d_nameSpace nameSpace, c_bool* isComplete);
d_storeResult d_storeNsMarkCompleteKV           (const d_store store, const d_nameSpace nameSpace, c_bool isComplete);
d_storeResult d_storeTransactionCompleteKV      (const d_store store, const v_groupAction message);
d_storeResult d_storeMessagesLoadKV             (const d_store store, const d_group group, struct d_groupHash *groupHash);
d_storeResult d_storeMessagesLoadFlushKV        (const d_store store, const d_group group, c_bool inject);


static int inst_cmp (const struct inst *a, const struct inst *b);
static void inst_free (struct inst *a);
static void inst_augment (void *vinst, const void *vleft, const void *vright);
static int compare_iid (const void *va, const void *vb);
static int transaction_list_cmp (const struct transaction_list *a, const struct transaction_list *b);
static void transaction_list_free (struct transaction_list *);
static d_storeResult commit_group_transaction (struct d_storeKV_s *st, const struct v_messageEOT_s *msg, int reconstructing);
static d_storeResult commit_writer_transaction (struct d_storeKV_s *st, struct transaction_list *tr_list, int reconstructing);
static struct msg *get_oldest_non_transaction (struct msg *a, struct msg *b, v_orderbyKind order_by) __nonnull((1)) __attribute_returns_nonnull__;

static int enable_kvdebug   = 0;
static int kvlog_statistics = 0;

static ut_avlTreedef_t group_inst_td =
    UT_AVL_TREEDEF_INITIALIZER (offsetof (struct inst, avlnode), offsetof (struct inst, next_msg_key.iid),
                                compare_iid, inst_augment);

static void kvlog (const struct d_storeKV_s *st, d_level level, const char *fmt, ...)
{
    char str[512];
    va_list ap;
    va_start (ap, fmt);
    (void)os_vsnprintf (str, sizeof (str), fmt, ap);
    va_end (ap);
    if (enable_kvdebug) {
        printf ("%s", str); fflush (stdout);
    }
    d_storeReport ((d_store) st, level, "%s", str);
    switch (level)
    {
    case D_LEVEL_FINEST: case D_LEVEL_FINER:
    case D_LEVEL_FINE: case D_LEVEL_CONFIG:
    case D_LEVEL_INFO: case D_LEVEL_NONE:
        break;
    case D_LEVEL_WARNING:
        OS_REPORT (OS_WARNING, D_CONTEXT, 0, "%s", str);
        break;
    case D_LEVEL_SEVERE:
        OS_REPORT (OS_ERROR, D_CONTEXT, 0, "%s", str);
        break;
    }
}

static void kvdebug (const char *fmt, ...)
{
    if (enable_kvdebug) {
        va_list ap;
        va_start (ap, fmt);
        vprintf (fmt, ap);
        va_end (ap);
    }
}

static d_storeResult kv_convert_result(kv_result_t rc)
{
    d_storeResult result = D_STORE_RESULT_ERROR;

    if (rc >= 0) {
        return D_STORE_RESULT_OK;
    }

    switch (rc) {
    case KV_RESULT_ACCESS_DENIED:
        result = D_STORE_RESULT_IO_ERROR;
        break;
    case KV_RESULT_NODATA:
        result = D_STORE_RESULT_ERROR;
        break;
    case KV_RESULT_ERROR:
        result = D_STORE_RESULT_ERROR;
        break;
    case KV_RESULT_CORRUPT:
        result = D_STORE_RESULT_MUTILATED;
        break;
    case KV_RESULT_OUT_OF_RESOURCES:
        result = D_STORE_RESULT_OUT_OF_RESOURCES;
        break;
    default:
        assert(FALSE);
        break;
    }

    return result;
}

/*****************************************************************************
 *
 *         ID TABLE FUNCTIONS
 *
 *****************************************************************************/

#define IDTABLE_ENTRY_EMPTY   UINT32_MAX
#define IDTABLE_INITIAL_SIZE  1024
#define IDTABLE_INITIAL_BITS  10
#define IDTABLE_THRESHOLD     10

struct idtable {
    os_uint32  size;
    os_uint32  scale;
    os_uint32  count;
    os_uint32 *entries;
};

static void
idtable_resize (
    struct idtable *tbl)
{
    os_uint32  oldsize = 0;
    os_uint32 *entries = NULL;
    os_uint32  key;
    os_uint32  i;

    if (tbl->size == 0) {
        tbl->size = IDTABLE_INITIAL_SIZE;
        tbl->scale = IDTABLE_INITIAL_BITS;

    } else {
        oldsize = tbl->size;
        tbl->size *= 2;
        tbl->scale++;
        entries = tbl->entries;
    }

    tbl->entries = os_malloc(sizeof(unsigned int) * tbl->size);
    memset(tbl->entries, -1, sizeof(unsigned int) * tbl->size);
    for (i = 0; i < oldsize; i++) {
        if (entries[i] != IDTABLE_ENTRY_EMPTY) {
            key = (os_uint32)(((entries[i] * hashTableConst) >> (64 - tbl->scale)));
            while (tbl->entries[key] != IDTABLE_ENTRY_EMPTY) {
                key = (key + 1) % tbl->size;
            }

            tbl->entries[key] = entries[i];
        }
    }
    if (entries) {
        os_free(entries);
    }
}

static struct idtable *
idtable_new (
    void)
{
    struct idtable *tbl = os_malloc(sizeof(struct idtable));
    tbl->size = 0;
    idtable_resize(tbl);
    return tbl;
}

static void
idtable_free (
    struct idtable *tbl)
{
    if (tbl) {
        if (tbl->entries) {
            os_free(tbl->entries);
        }
        os_free(tbl);
    }
}

static int
idtable_put (
    struct idtable *tbl,
    os_uint32       id)
{
    os_uint32 key;

    assert(tbl);

    key = (os_uint32)((id * hashTableConst) >> (64 - tbl->scale));
    assert(key < tbl->size);

    if (tbl->entries[key] == id) {
        return 1;
    }

    if (((tbl->size - tbl->count + 1) * 100) / tbl->size < IDTABLE_THRESHOLD) {
        idtable_resize(tbl);
        key = (os_uint32)((id * hashTableConst) >> (64 - tbl->scale));
        assert(key < tbl->size);
    }

    while (tbl->entries[key] != IDTABLE_ENTRY_EMPTY) {
        key = (key + 1) % tbl->size;
    }

    tbl->entries[key] = id;
    tbl->count++;

    return 1;
}

static os_uint32
idtable_get (
    struct idtable *tbl,
    os_uint32       id)
{
    os_uint32 key;

    assert(tbl);

    key = (os_uint32)((id * hashTableConst) >> (64 - tbl->scale));
    assert(key < tbl->size);

    while ((tbl->entries[key] != IDTABLE_ENTRY_EMPTY) && (tbl->entries[key] != id)) {
        key = (key + 1) % tbl->size;
    }

    return tbl->entries[key];
}


/*****************************************************************************
 *
 *         OBJECT HASH TABLE FUNCTIONS
 *
 *****************************************************************************/

#define HASHTABLE_SIZE  1024

typedef os_uint32 (*hashtable_key_func_t)(void *obj);
typedef int       (*hashtable_cmp_func_t)(void *obj1, void *obj2);
typedef int       (*hashtable_action_func_t)(void *obj, void *arg);


struct hashtable_entry {
    struct hashtable_entry  *next;
    struct hashtable_entry  *link;
    void                    *object;
};

struct hashtable {
    os_uint32                size;
    hashtable_key_func_t     keyFunc;
    hashtable_cmp_func_t     compareFunc;
    struct hashtable_entry  *chain;
    struct hashtable_entry **heads;
};


static struct hashtable *
hashtable_new (
    hashtable_key_func_t  keyFunc,
    hashtable_cmp_func_t  compareFunc)
{
    struct hashtable *tbl = os_malloc(sizeof(struct hashtable));
    tbl->size = HASHTABLE_SIZE;
    tbl->keyFunc = keyFunc;
    tbl->compareFunc = compareFunc;
    tbl->chain = NULL;
    tbl->heads = os_malloc(HASHTABLE_SIZE * sizeof(struct hashtable_entry *));
    memset(tbl->heads, 0, HASHTABLE_SIZE * sizeof(struct hashtable_entry *));
    return tbl;
}

static void
hashtable_free (
    struct hashtable *tbl)
{
    struct hashtable_entry *curr;
    struct hashtable_entry *next;
    os_uint32 i;

    if (tbl) {
        if (tbl->heads) {
            for (i = 0; i < tbl->size; i++) {
                curr = tbl->heads[i];
                while (curr) {
                    next = curr->next;
                    os_free(curr);
                    curr = next;
                }
            }
            os_free(tbl->heads);
        }
        os_free(tbl);
    }
}

static void
hashtable_put (
    struct hashtable *tbl,
    void             *obj)
{
    os_uint32 idx;
    struct hashtable_entry *entry = NULL;

    assert(tbl);
    assert(obj);

    idx = tbl->keyFunc(obj) % tbl->size;

    for (entry = tbl->heads[idx]; entry; entry = entry->next) {
        if (tbl->compareFunc(entry->object, obj)) {
            break;
        }
    }

    if (!entry) {
        entry = os_malloc(sizeof(struct hashtable_entry));
        entry->next = tbl->heads[idx];
        tbl->heads[idx] = entry;
        entry->link = tbl->chain;
        tbl->chain = entry;
        entry->object = obj;
    }
}

static void
hashtable_remove_entry (
    struct hashtable       *tbl,
    os_uint32               idx,
    struct hashtable_entry *entry)
{
    struct hashtable_entry *curr;

    assert(tbl);
    assert(idx < tbl->size);
    assert(entry);

    if (entry != tbl->heads[idx]) {
        curr = tbl->heads[idx];
        while (curr && (curr->next != entry)) {
            curr = curr->next;
        }
        if (curr) {
            curr->next = entry->next;
        }
    } else {
        tbl->heads[idx] = entry->next;
    }
    if (entry != tbl->chain) {
        curr = tbl->chain;
        while (curr && (curr->link != entry)) {
            curr = curr->link;
        }
        if (curr) {
            curr->link = entry->link;
        }
    } else {
        tbl->chain = entry->link;
    }

    os_free(entry);
}

static void *
hashtable_del (
    struct hashtable *tbl,
    void             *obj)
{
    os_uint32 idx;
    struct hashtable_entry *entry;
    void *object = NULL;

    assert(tbl);
    assert(obj);

    idx = tbl->keyFunc(obj) % tbl->size;

    for (entry = tbl->heads[idx]; entry; entry = entry->next) {
        if (entry->object == obj) {
            object = entry->object;
            break;
        }
    }

    if (entry) {
        hashtable_remove_entry(tbl, idx, entry);
    }

    return object;
}


static void *
hashtable_find (
    struct hashtable     *tbl,
    os_uint32             key,
    hashtable_cmp_func_t  compareFunc,
    void                 *arg)
{
    os_uint32 idx;
    struct hashtable_entry *entry;
    void *object = NULL;

    assert(tbl);

    idx = key % tbl->size;

    for (entry = tbl->heads[idx]; entry; entry = entry->next) {
        if (compareFunc(entry->object, arg)) {
            object = entry->object;
            break;
        }
    }

    return object;
}

static void
hashtable_walk (
    struct hashtable        *tbl,
    hashtable_action_func_t  action,
    void                    *arg)
{
    struct hashtable_entry *entry;

    assert(tbl);
    assert(action);

    for (entry = tbl->chain; entry; entry = entry->link) {
        action(entry->object, arg);
    }
}

static void *
hashtable_take (
    struct hashtable *tbl)
{
    struct hashtable_entry *entry;
    void *object = NULL;

    assert(tbl);

    entry = tbl->chain;
    if (entry) {
        os_uint32 idx;

        object = entry->object;
        assert(object);

        idx = tbl->keyFunc(object) % tbl->size;

        hashtable_remove_entry(tbl, idx, entry);
    }

    return object;
}

/*****************************************************************************
 *
 *         SPECIAL COMPARISON FUNCTIONS
 *
 *****************************************************************************/

static int quality_le (const os_timeW *a, const os_timeW *b)
{
    os_compare eq = os_timeWCompare(*a,*b);
    return ((eq == OS_LESS) || (eq == OS_EQUAL));
}

static int msg_cmp (const struct msg *a, const struct msg *b)
{
    /* Message ordering key precedence: time, gid, sequence number.
     * Sequence number may be superfluous in this. */
    c_equality eq;
    os_compare cmp;
    os_int32 d;
    cmp = os_timeWCompare (a->writeTime, b->writeTime);
    if (cmp == OS_LESS) {
        return -1;
    } else if (cmp == OS_MORE) {
        return 1;
    }
    eq = v_gidCompare (a->writerGID, b->writerGID);
    if (eq == C_LT) {
        return -1;
    } else if (eq == C_GT) {
        return 1;
    }
    /* Serial number arithmetic; see:
     *
     * - RFC 1982;
     * - http://en.wikipedia.org/wiki/Serial_number_arithmetic)
     *
     * It seems the only reasonable way of ordering the sequence
     * numbers, although it is not ideal: you can (easily) construct a
     * situation in which an arbitrarily large gaps of sequence
     * numbers exists between two messages that get stored and a few
     * billion is still practically doable. Under those circumstances
     * there is no good comparison routine with 32-bit sequence
     * numbers. */
    d = (os_int32) (a->sequenceNumber - b->sequenceNumber);
    if (d < 0) {
        return -1;
    } else if (d > 0) {
        return 1;
    }
    return 0;
}

static int msg_v_message_cmp (const struct msg *a, const struct v_message_s *b)
{
    struct msg b1;
    b1.writeTime = b->writeTime;
    b1.writerGID = b->writerGID;
    b1.sequenceNumber = b->sequenceNumber;
    return msg_cmp(a, &b1);
}

static int msg_lt_v_message (const struct msg *a, const struct v_message_s *b)
{
    return msg_v_message_cmp (a, b) < 0;
}

/*****************************************************************************
 *
 *         RAW KEY VALUE STORE INTERFACE
 *
 *****************************************************************************/

typedef kv_result_t (*kv_close_t) (kv_store_t gst);
typedef kv_result_t (*kv_commit_t) (kv_store_t gst);
typedef kv_result_t (*kv_get_t) (kv_store_t gst, const kv_key_t *key, os_uint32 *sz, void **val, const struct c_type_s *type);
typedef kv_result_t (*kv_put_t) (kv_store_t gst, const kv_key_t *key, os_uint32 sz, const void *val, const struct c_type_s *type);
typedef kv_result_t (*kv_delete_t) (kv_store_t gst, const kv_key_t *key);
typedef kv_result_t (*kv_bulkdelete_t) (kv_store_t gst, const kv_key_t *first, const kv_key_t *last);
typedef kv_result_t (*kv_iter_new_t) (kv_store_t gst, kv_iter_t *giter, const kv_key_t *first, const kv_key_t *last, const struct c_type_s *type);
typedef kv_result_t (*kv_iter_free_t) (kv_store_t gst, kv_iter_t giter);
typedef kv_result_t (*kv_iter_next_t) (int *hasdata, kv_store_t gst, kv_iter_t giter);
typedef kv_result_t (*kv_iter_getkey_t) (kv_store_t gst, kv_iter_t giter, kv_key_t *key);
typedef kv_result_t (*kv_iter_getvalue_t) (kv_store_t gst, kv_iter_t giter, os_uint32 *sz, void **val);
typedef d_encodingKV (*kv_choose_encoding_t) (kv_store_t gst, d_encodingKV req);

struct kv_store_common {
    int write_access;
    kv_kind_t kind;
    kv_close_t close;
    kv_commit_t commit;
    kv_get_t get;
    kv_put_t put;
    kv_delete_t delete;
    kv_bulkdelete_t bulkdelete;
    kv_iter_new_t iter_new;
    kv_iter_free_t iter_free;
    kv_iter_next_t iter_next;
    kv_iter_getkey_t iter_getkey;
    kv_iter_getvalue_t iter_getvalue;
    kv_choose_encoding_t choose_encoding;
};

static d_encodingKV kv_choose_encoding_def (kv_store_t gst, d_encodingKV req)
{
    OS_UNUSED_ARG(gst);
    return req;
}

#if KV_INCLUDE_SQLITE || KV_INCLUDE_LEVELDB
struct kv_blobkey {
    os_uint32 tab_gid;
    os_uint32 iid;
    os_uint32 midhigh, midlow;
};
#endif

#if KV_INCLUDE_SQLITE
#include <sqlite3.h>

/****** BLOB-TO-BLOB MAPPING VARIANT ******/

static void to_blobkey (struct kv_blobkey *k, const kv_key_t *key)
{
    os_uint32 tab_gid = (key->table << 24) | key->gid;
    k->tab_gid = htonl (tab_gid);
    k->iid = htonl (key->iid);
    k->midhigh = htonl ((os_uint32) (key->mid >> 32));
    k->midlow = htonl ((os_uint32) key->mid);
}

static void from_blobkey (kv_key_t *key, const struct kv_blobkey *k)
{
    os_uint32 tab_gid = ntohl (k->tab_gid);
    key->table = tab_gid >> 24;
    key->gid = tab_gid & 0xffffff;
    key->iid = ntohl (k->iid);
    key->mid = (os_uint64) ntohl (k->midhigh) << 32 | ntohl (k->midlow);
}

struct kv_store_sqlite {
    struct kv_store_common c;
    struct d_storeKV_s *logst;
    sqlite3 *db;
    sqlite3_stmt *pstmt_get;
    sqlite3_stmt *pstmt_put;
    sqlite3_stmt *pstmt_delete;
};

struct kv_iter_sqlite {
    kv_table_t table;
    sqlite3_stmt *pstmt;
};

static kv_result_t sqlite_execsimple (struct kv_store_sqlite *st, const char *sql)
{
    struct sqlite3_stmt *pstmt;
    kv_result_t result;
    int code;
    if ((code = sqlite3_prepare_v2 (st->db, sql, (int) strlen (sql) + 1, &pstmt, NULL)) != SQLITE_OK) {
        kvlog (st->logst, D_LEVEL_SEVERE, "sqlite: execsimple: prepare %s failed (%d)\n", code);
        return KV_RESULT_ERROR;
    }
    do {
        code = sqlite3_step (pstmt);
    } while (code == SQLITE_ROW);
    if (code != SQLITE_DONE) {
        kvlog (st->logst, D_LEVEL_SEVERE, "sqlite: execsimple: step %s failed (%d)\n", sql, code);
        result = KV_RESULT_ERROR;
    } else {
        result = 0;
    }
    (void)sqlite3_finalize (pstmt);
    return result;
}

static kv_result_t kv_close_sqlite (kv_store_t gst)
{
    struct kv_store_sqlite *st = (struct kv_store_sqlite *) gst;
    (void)sqlite_execsimple (st, "COMMIT");
    (void)sqlite3_finalize (st->pstmt_get);
    (void)sqlite3_finalize (st->pstmt_put);
    (void)sqlite3_finalize (st->pstmt_delete);
    sqlite3_close (st->db);
    os_free (st);
    return 0;
}

static kv_result_t kv_commit_sqlite (kv_store_t gst)
{
    struct kv_store_sqlite *st = (struct kv_store_sqlite *) gst;
    kv_result_t rc;
    /* Commits can take very long if the store session time is long
     * enough to accumulate a lot of data in the transaction. Declare
     * the thread to be asleep to prevent the liveliness checker from
     * complaining. A year is roughly 31e6 seconds - surely the commit
     * can complete within a year ... */
    d_threadAsleep (d_threadLookupSelf (), 31000000);
    if ((rc = sqlite_execsimple (st, "COMMIT")) >= 0) {
        rc = sqlite_execsimple (st, "BEGIN");
    }
    d_threadAwake (d_threadLookupSelf ());
    return rc;
}

static kv_result_t kv_get_sqlite (kv_store_t gst, const kv_key_t *key, os_uint32 *sz, void **val, const struct c_type_s *type)
{
    struct kv_store_sqlite *st = (struct kv_store_sqlite *) gst;
    struct sqlite3_stmt *pstmt = st->pstmt_get;
    struct kv_blobkey k;
    kv_result_t result;
    OS_UNUSED_ARG (type);
    to_blobkey (&k, key);
    if (sqlite3_bind_blob (pstmt, 1, &k, sizeof (k), SQLITE_TRANSIENT) != SQLITE_OK) {
        return KV_RESULT_ERROR;
    }
    switch (sqlite3_step (pstmt))
    {
    case SQLITE_DONE:
        result = KV_RESULT_NODATA;
        break;
    case SQLITE_ROW:
    {
        const void *val_alias;
        os_uint32 val_sz;
        val_alias = sqlite3_column_blob (pstmt, 0);
        val_sz = (os_uint32) sqlite3_column_bytes (pstmt, 0);
        if ((*val = os_malloc ((val_sz == 0) ? 1 : val_sz)) == NULL) {
            result = KV_RESULT_OUT_OF_RESOURCES;
        } else {
            *sz = val_sz;
            memcpy (*val, val_alias, val_sz);
            result = 0;
        }
        break;
    }
    default:
        result = KV_RESULT_ERROR;
        break;
    }
    sqlite3_reset (pstmt);
    return result;
}

static kv_result_t kv_put_sqlite (kv_store_t gst, const kv_key_t *key, os_uint32 sz, const void *val, const struct c_type_s *type)
{
    struct kv_store_sqlite *st = (struct kv_store_sqlite *) gst;
    struct sqlite3_stmt *pstmt = st->pstmt_put;
    struct kv_blobkey k;
    kv_result_t result;
    int result1 = 0;
    int result2 = 0;
    OS_UNUSED_ARG (type);
    to_blobkey (&k, key);
    if (((result1 = sqlite3_bind_blob (pstmt, 1, &k, sizeof (k), SQLITE_TRANSIENT)) != SQLITE_OK) ||
        ((result2 = sqlite3_bind_blob (pstmt, 2, val, (int) sz, SQLITE_TRANSIENT)) != SQLITE_OK)) {
        kvlog (st->logst, D_LEVEL_WARNING, "kv_put_sqlite: sqlite3_bind_blob failed, result1 = %d, result2 = %d\n", result1, result2);
        return KV_RESULT_ERROR;
    }
    switch (sqlite3_step (pstmt))
    {
    case SQLITE_DONE:
        result = 0;
        break;
    default:
        kvlog (st->logst, D_LEVEL_WARNING, "kv_put_sqlite: sqlite3_step does not return SQLITE_DONE\n");
        result = KV_RESULT_ERROR;
        break;
    }
    sqlite3_reset (pstmt);
    return result;
}

static kv_result_t kv_delete_sqlite (kv_store_t gst, const kv_key_t *key)
{
    struct kv_store_sqlite *st = (struct kv_store_sqlite *) gst;
    struct sqlite3_stmt *pstmt = st->pstmt_delete;
    struct kv_blobkey k;
    kv_result_t result;
    to_blobkey (&k, key);
    if (sqlite3_bind_blob (pstmt, 1, &k, sizeof (k), SQLITE_TRANSIENT) != SQLITE_OK) {
        return KV_RESULT_ERROR;
    }
    switch (sqlite3_step (pstmt))
    {
    case SQLITE_DONE:
        result = 0;
        break;
    default:
        result = KV_RESULT_ERROR;
        break;
    }
    sqlite3_reset (pstmt);
    return result;
}

static kv_result_t kv_bulkdelete_sqlite (kv_store_t gst, const kv_key_t *first, const kv_key_t *last)
{
    struct kv_store_sqlite *st = (struct kv_store_sqlite *) gst;
    const char *sql = "DELETE FROM data WHERE k BETWEEN ? AND ?";
    struct sqlite3_stmt *pstmt;
    struct kv_blobkey k0, k1;
    kv_result_t result;
    if (sqlite3_prepare_v2 (st->db, sql, (int) strlen (sql) + 1, &pstmt, NULL) != SQLITE_OK) {
        return KV_RESULT_ERROR;
    }
    to_blobkey (&k0, first);
    to_blobkey (&k1, last);
    if (sqlite3_bind_blob (pstmt, 1, &k0, sizeof (k0), SQLITE_TRANSIENT) != SQLITE_OK ||
        sqlite3_bind_blob (pstmt, 2, &k1, sizeof (k1), SQLITE_TRANSIENT) != SQLITE_OK) {
        return KV_RESULT_ERROR;
    }
    /* Deleting a lot of data in one operation can take very long. Declare
     * the thread to be asleep to prevent the liveliness checker from
     * complaining. A year is roughly 31e6 seconds - surely that is enough */
    d_threadAsleep (d_threadLookupSelf (), 31000000);
    switch (sqlite3_step (pstmt))
    {
    case SQLITE_DONE:
        result = 0;
        break;
    default:
        result = KV_RESULT_ERROR;
        break;
    }
    d_threadAwake (d_threadLookupSelf ());
    (void)sqlite3_finalize (pstmt);
    return result;
}

static kv_result_t kv_iter_free_sqlite (kv_store_t gst, kv_iter_t giter)
{
    kv_result_t rc = 0;
    struct kv_iter_sqlite *it = (struct kv_iter_sqlite *) giter;
    OS_UNUSED_ARG (gst);
    if (sqlite3_finalize (it->pstmt) != SQLITE_OK) {
        rc = KV_RESULT_ERROR;
    }
    os_free (it);
    return rc;
}

static kv_result_t kv_iter_next_sqlite (int *hasdata, kv_store_t gst, kv_iter_t giter)
{
    struct kv_iter_sqlite *it = (struct kv_iter_sqlite *) giter;
    OS_UNUSED_ARG (gst);
    switch (sqlite3_step (it->pstmt))
    {
    case SQLITE_DONE:
        *hasdata = 0;
        return 0;
    case SQLITE_ROW:
        *hasdata = 1;
        return 0;
    default:
        return KV_RESULT_ERROR;
    }
}

static kv_result_t kv_iter_getkey_sqlite (kv_store_t gst, kv_iter_t giter, kv_key_t *key)
{
    struct kv_iter_sqlite *it = (struct kv_iter_sqlite *) giter;
    const void *val_alias;
    os_uint32 val_sz;
    OS_UNUSED_ARG (gst);
    val_alias = sqlite3_column_blob (it->pstmt, 0);
    val_sz = (os_uint32) sqlite3_column_bytes (it->pstmt, 0);
    if (val_sz != sizeof (struct kv_blobkey)) {
        return KV_RESULT_ERROR;
    }
    from_blobkey (key, val_alias);
    return 0;
}

static kv_result_t kv_iter_getvalue_sqlite (kv_store_t gst, kv_iter_t giter, os_uint32 *sz, void **val)
{
    struct kv_iter_sqlite *it = (struct kv_iter_sqlite *) giter;
    const void *val_alias;
    os_uint32 val_sz;
    OS_UNUSED_ARG (gst);
    val_alias = sqlite3_column_blob (it->pstmt, 1);
    val_sz = (os_uint32) sqlite3_column_bytes (it->pstmt, 1);
    if ((*val = os_malloc ((val_sz == 0) ? 1 : val_sz)) == NULL) {
        return KV_RESULT_OUT_OF_RESOURCES;
    } else {
        *sz = val_sz;
        memcpy (*val, val_alias, val_sz);
        return 0;
    }
}

static kv_result_t kv_iter_new_sqlite (kv_store_t gst, kv_iter_t *giter, const kv_key_t *first, const kv_key_t *last, const struct c_type_s *type)
{
    struct kv_store_sqlite *st = (struct kv_store_sqlite *) gst;
    struct kv_iter_sqlite *it;
    const char *sql = "SELECT * FROM data WHERE k BETWEEN ? AND ? ORDER BY k";
    struct kv_blobkey k0, k1;
    OS_UNUSED_ARG (type);
    it = os_malloc (sizeof (*it));
    it->table = first->table;
    if (sqlite3_prepare_v2 (st->db, sql, (int) strlen (sql) + 1, &it->pstmt, NULL) != SQLITE_OK) {
        os_free (it);
        return KV_RESULT_ERROR;
    }
    to_blobkey (&k0, first);
    to_blobkey (&k1, last);
    if (sqlite3_bind_blob (it->pstmt, 1, &k0, sizeof (k0), SQLITE_TRANSIENT) != SQLITE_OK ||
        sqlite3_bind_blob (it->pstmt, 2, &k1, sizeof (k1), SQLITE_TRANSIENT) != SQLITE_OK) {
        kv_iter_free_sqlite (gst, (kv_iter_t) it);
        return KV_RESULT_ERROR;
    }
    *giter = (kv_iter_t) it;
    return 0;
}

#define KV_SQLITE_PRAGMA_TEMPLATE "PRAGMA %s;"

static const char *kv_sqlite_predef_parms[] = {
        "locking_mode",
        "journal_mode",
        "wal_autocheckpoint",
        "synchronous"
};
static unsigned int kv_sqlite_predef_parms_len = sizeof(kv_sqlite_predef_parms)/sizeof(const char *);

static int kv_configure_sqlite (struct kv_store_sqlite *st, const c_char *parameters)
{
    c_char *str;
    c_char *ptr;
    c_char *tok;
    c_char *expr;
    size_t len;

    if (parameters) {
        str = os_strdup(parameters);
        tok = os_strtok_r(str, ";", &ptr);
        while (tok) {
            c_char *s;
            unsigned int i;
            c_bool found = FALSE;
            for (s = tok; isspace((unsigned char) *s); ++s);
            for (i = 0; !found && (i < kv_sqlite_predef_parms_len); i++) {
                found = os_strncasecmp(kv_sqlite_predef_parms[i], s, strlen(kv_sqlite_predef_parms[i])) == 0;
            }
            if (!found) {
                len = strlen(KV_SQLITE_PRAGMA_TEMPLATE) + strlen(tok) + 1;
                expr = os_malloc(len);
                snprintf(expr, len, KV_SQLITE_PRAGMA_TEMPLATE, tok);
                if (sqlite_execsimple (st, expr) < 0) {
                    kvlog (st->logst, D_LEVEL_WARNING, "Failed to set Sqlite pragma: %s\n", expr);
                }
                os_free(expr);
            }
            tok = os_strtok_r(NULL, ";", &ptr);
        }
        os_free(str);
    }

    return 0;
}
#undef KV_SQLITE_PRAGMA_TEMPLATE

static kv_store_t kv_open_sqlite (struct d_storeKV_s *logst, const char *dir, const c_char *parameters)
{
    const char *filename = "kv.sqlite";
    struct kv_store_sqlite *st;
    char *path;
    int ret;
    size_t len;

    assert (dir);

    len = strlen (dir) + 1 + strlen (filename) + 1;

    path = os_malloc (len);
    snprintf (path, len, "%s/%s", dir, filename);

    st = os_malloc (sizeof (*st));
    if ((ret = sqlite3_open (path, &st->db)) != SQLITE_OK) {
        sqlite3_close (st->db);
        os_free (st);
        os_free (path);
        return NULL;
    } else {
        char sql[256];
        int n;
        os_free (path);
        st->logst = logst;

        {
            sqlite3_stmt *pstmt;
            int ok;
            n = snprintf (sql, sizeof (sql), "CREATE TABLE IF NOT EXISTS data (k BLOB PRIMARY KEY, v BLOB)");
            assert (n < (int) sizeof (sql));
            (void)n;
            if ((ret = sqlite3_prepare_v2 (st->db, sql, (int) strlen (sql) + 1, &pstmt, NULL)) != SQLITE_OK) {
                sqlite3_close (st->db);
                os_free (st);
                return NULL;
            }
            ok = (sqlite3_step (pstmt) == SQLITE_DONE);
            (void)sqlite3_finalize (pstmt);
            if (!ok) {
                sqlite3_close (st->db);
                os_free (st);
                return NULL;
            }
        }

        n = snprintf (sql, sizeof (sql), "SELECT v FROM data WHERE k = ?");
        assert (n < (int) sizeof (sql));
        (void)n;
        if ((ret = sqlite3_prepare_v2 (st->db, sql, (int) strlen (sql) + 1, &st->pstmt_get, NULL)) != SQLITE_OK) {
            sqlite3_close (st->db);
            os_free (st);
            return NULL;
        }
        n = snprintf (sql, sizeof (sql), "INSERT OR REPLACE INTO data VALUES (?, ?)");
        assert (n < (int) sizeof (sql));
        (void)n;
        if ((ret = sqlite3_prepare_v2 (st->db, sql, (int) strlen (sql) + 1, &st->pstmt_put, NULL)) != SQLITE_OK) {
            (void)sqlite3_finalize (st->pstmt_get);
            sqlite3_close (st->db);
            os_free (st);
            return NULL;
        }
        n = snprintf (sql, sizeof (sql), "DELETE FROM data WHERE k = ?");
        assert (n < (int) sizeof (sql));
        (void)n;
        if ((ret = sqlite3_prepare_v2 (st->db, sql, (int) strlen (sql) + 1, &st->pstmt_delete, NULL)) != SQLITE_OK) {
            (void)sqlite3_finalize (st->pstmt_get);
            (void)sqlite3_finalize (st->pstmt_put);
            sqlite3_close (st->db);
            os_free (st);
            return NULL;
        }

        st->c.write_access = 1;
        st->c.kind = KV_KIND_SQLITE;
        st->c.close = kv_close_sqlite;
        st->c.commit = kv_commit_sqlite;
        st->c.get = kv_get_sqlite;
        st->c.put = kv_put_sqlite;
        st->c.delete = kv_delete_sqlite;
        st->c.bulkdelete = kv_bulkdelete_sqlite;
        st->c.iter_new = kv_iter_new_sqlite;
        st->c.iter_free = kv_iter_free_sqlite;
        st->c.iter_next = kv_iter_next_sqlite;
        st->c.iter_getkey = kv_iter_getkey_sqlite;
        st->c.iter_getvalue = kv_iter_getvalue_sqlite;
        st->c.choose_encoding = kv_choose_encoding_def;

        if (sqlite_execsimple (st, "PRAGMA locking_mode=EXCLUSIVE") < 0) {
            kv_close_sqlite ((kv_store_t) st);
            return NULL;
        } if (sqlite_execsimple (st, "PRAGMA journal_mode=WAL") < 0) {
            kv_close_sqlite ((kv_store_t) st);
            return NULL;
        } if (sqlite_execsimple (st, "PRAGMA wal_autocheckpoint=4096") < 0) {
            kv_close_sqlite ((kv_store_t) st);
            return NULL;
        } if (sqlite_execsimple (st, "PRAGMA synchronous=NORMAL") < 0) {
            kv_close_sqlite ((kv_store_t) st);
            return NULL;
        } if (kv_configure_sqlite(st, parameters) != 0) {
            kv_close_sqlite ((kv_store_t) st);
            return NULL;
        } else if (sqlite_execsimple (st, "BEGIN") < 0) {
            kv_close_sqlite ((kv_store_t) st);
            return NULL;
        } else {
            return (kv_store_t) st;
        }
    }
}

/****** MULTI-TABLE VARIANT ******/

struct kv_store_sqlitemt {
    /* Incorporate blob-to-blob sqlite so we can reuse some functions,
     * the get/put/delete fields will be all 0 */
    struct kv_store_sqlite x;
    sqlite3_stmt *pstmt_get[N_TABLES];
    sqlite3_stmt *pstmt_put[N_TABLES];
    sqlite3_stmt *pstmt_delete[N_TABLES];
};

struct kv_iter_sqlitemt {
    kv_table_t table;
    int nkeys;
    const struct c_type_s *type;
    sqlite3_stmt *pstmt;
};

static const struct {
    enum kv_table k;
    const char *name;
    const char *cols;
    const char *primkey;
    const char *valcols;
    const char *kspec;
    const char *vspec;
} kv_sqlitemt_tabtab[] = {
    { TABLE_VERSION, "version", "tag INTEGER, value BLOB",
        "tag", "value",
        "tag = ?",
        "?, ?" },
    { TABLE_TOPIC, "topics", "topic_id INTEGER, name TEXT, type_name TEXT, keylist TEXT, data TEXT",
        "topic_id", "name, type_name, keylist, data",
        "topic_id = ?",
        "?, ?, ?, ?, ?" },
    { TABLE_TOPIC_TYPE, "topics_type", "topic_id INTEGER, type TEXT",
        "topic_id", "type",
        "topic_id = ?",
        "?, ?" },
    { TABLE_NAMESPACE, "namespaces", "namespace_id INTEGER, namespace_version INTEGER, name TEXT",
        "namespace_id, namespace_version", "name",
        "namespace_id = ? AND namespace_version = ?",
        "?, ?, ?" },
    { TABLE_NAMESPACE_QUALITY, "namespaces_quality",
        "namespace_id INTEGER, namespace_version INTEGER, quality_sec INTEGER, quality_nsec INTEGER",
        "namespace_id, namespace_version", "quality_sec, quality_nsec",
        "namespace_id = ? AND namespace_version = ?",
        "?, ?, ?, ?" },
    { TABLE_NAMESPACE_COMPLETENESS, "namespaces_completeness",
        "namespace_id INTEGER, namespace_version INTEGER, complete INTEGER",
        "namespace_id, namespace_version", "complete",
        "namespace_id = ? AND namespace_version = ?",
        "?, ?, ?" },
    { TABLE_GROUP, "groups",
        "group_id INTEGER, topic_id INTEGER, namespace_id INTEGER, namespace_version INTEGER, partition TEXT, topic TEXT",
        "group_id",
        "topic_id, namespace_id, namespace_version, partition, topic",
        "group_id = ?",
        "?, ?, ?, ?, ?, ?" },
    { TABLE_MESSAGE, "messages", "group_id INTEGER, instance_id INTEGER, message_id INTEGER, data TEXT",
        "group_id, instance_id, message_id", "data",
        "group_id = ? AND instance_id = ? AND message_id = ?",
        "?, ?, ?, ?" },
    { TABLE_MESSAGE_EOT, "messages_eot", "xid INTEGER, data TEXT",
        "xid", "data",
        "xid = ?",
        "?, ?" }
};

static kv_result_t kv_close_sqlitemt (kv_store_t gst)
{
    struct kv_store_sqlitemt *st = (struct kv_store_sqlitemt *) gst;
    int i;
    (void)sqlite_execsimple (&st->x, "COMMIT");
    for (i = 0; i < N_TABLES; i++) {
        (void)sqlite3_finalize (st->pstmt_get[i]);
        (void)sqlite3_finalize (st->pstmt_put[i]);
        (void)sqlite3_finalize (st->pstmt_delete[i]);
    }
    sqlite3_close (st->x.db);
    os_free (st);
    return 0;
}

static kv_result_t kv_commit_sqlitemt (kv_store_t gst)
{
    return kv_commit_sqlite (gst);
}

static void *kv_sqlitemt_ser (os_uint32 *sz, struct kv_store_sqlitemt *st, const struct c_type_s *type, const void *obj)
{
    void *blob;
    if ((blob = serialize (sz, st->x.logst, type, NULL, obj)) == NULL) {
        abort();
    }
    return blob;
}

static void *kv_sqlitemt_deser (struct kv_store_sqlitemt *st, const struct c_type_s *type, os_uint32 sz, const void *val)
{
    void *vx;
    if (deserialize (&vx, st->x.logst, type, NULL, sz, val) < 0) {
        abort();
    }
    return vx;
}

static kv_result_t kv_sqlitemt_bind_key (const kv_key_t *key, struct sqlite3_stmt *pstmt)
{
    switch (key->table) {
        case TABLE_VERSION:
        case TABLE_TOPIC:
        case TABLE_TOPIC_TYPE:
        case TABLE_GROUP:
            assert (key->iid == 0 && key->mid == 0);
            if (sqlite3_bind_int (pstmt, 1, (int) key->gid) != SQLITE_OK) {
                return KV_RESULT_ERROR;
            }
            break;
        case TABLE_NAMESPACE:
        case TABLE_NAMESPACE_QUALITY:
        case TABLE_NAMESPACE_COMPLETENESS:
            assert (key->mid == 0);
            if (sqlite3_bind_int (pstmt, 1, (int) key->gid) != SQLITE_OK ||
                sqlite3_bind_int (pstmt, 2, (int) key->iid) != SQLITE_OK) {
                return KV_RESULT_ERROR;
            }
            break;
        case TABLE_MESSAGE:
            if (sqlite3_bind_int (pstmt, 1, (int) key->gid) != SQLITE_OK ||
                sqlite3_bind_int (pstmt, 2, (int) key->iid) != SQLITE_OK ||
                sqlite3_bind_int64 (pstmt, 3, (os_int64) key->mid) != SQLITE_OK) {
                return KV_RESULT_ERROR;
            }
            break;
        case TABLE_MESSAGE_EOT:
            assert (key->gid == 0 && key->iid == 0);
            if (sqlite3_bind_int64 (pstmt, 1, (os_int64) key->mid) != SQLITE_OK) {
                return KV_RESULT_ERROR;
            }
            break;
    }
    return 0;
}

static void kv_sqlitemt_extract_payload (os_uint32 *sz, void **val, struct kv_store_sqlitemt *st, enum kv_table table, struct sqlite3_stmt *pstmt, const struct c_type_s *type, int coloff)
{
    switch (table) {
        case TABLE_VERSION: {
            const void *val_alias;
            os_uint32 val_sz;
            val_alias = sqlite3_column_blob (pstmt, coloff);
            val_sz = (os_uint32) sqlite3_column_bytes (pstmt, coloff);
            *val = os_malloc ((val_sz == 0) ? 1 : val_sz);
            *sz = val_sz;
            memcpy (*val, val_alias, val_sz);
            break;
        }
        case TABLE_MESSAGE:
        case TABLE_MESSAGE_EOT:
        case TABLE_TOPIC:
        case TABLE_TOPIC_TYPE: {
            const char *val_alias;
            os_uint32 val_sz;
            int col = (table == TABLE_TOPIC) ? 3 : 0;
            val_alias = (const char *) sqlite3_column_text (pstmt, coloff + col);
            val_sz = (os_uint32) (strlen (val_alias) + 1);
            *val = os_malloc (val_sz);
            *sz = val_sz;
            memcpy (*val, val_alias, val_sz);
            break;
        }
        case TABLE_GROUP: {
            c_base base = c_getBase ((c_object) type);
            struct d_groupKV *x = c_new ((c_type) type);
            x->topic_id = (c_ulong) sqlite3_column_int (pstmt, coloff + 0);
            x->namespace_id = (c_ulong) sqlite3_column_int (pstmt, coloff + 1);
            x->namespace_version = (c_ulong) sqlite3_column_int (pstmt, coloff + 2);
            x->partition = c_stringNew (base, (const char *) sqlite3_column_text (pstmt, coloff + 3));
            x->topic = c_stringNew (base, (const char *) sqlite3_column_text (pstmt, coloff + 4));
            *val = kv_sqlitemt_ser (sz, st, type, x);
            c_free (x);
            break;
        }
        case TABLE_NAMESPACE: {
            c_base base = c_getBase ((c_object) type);
            struct d_namespaceKV *x = c_new ((c_type) type);
            x->name = c_stringNew (base, (const char *) sqlite3_column_text (pstmt, coloff + 0));
            *val = kv_sqlitemt_ser (sz, st, type, x);
            c_free (x);
            break;
        }
        case TABLE_NAMESPACE_QUALITY: {
            struct d_namespaceQualityKV *x = c_new ((c_type) type);
            x->quality.seconds = sqlite3_column_int (pstmt, coloff + 0);
            x->quality.nanoseconds = (c_ulong) sqlite3_column_int (pstmt, coloff + 1);
            *val = kv_sqlitemt_ser (sz, st, type, x);
            c_free (x);
            break;
        }
        case TABLE_NAMESPACE_COMPLETENESS: {
            struct d_namespaceCompletenessKV *x = c_new ((c_type) type);
            x->complete = (c_bool) sqlite3_column_int (pstmt, coloff + 0);
            *val = kv_sqlitemt_ser (sz, st, type, x);
            c_free (x);
            break;
        }
    }
}

static kv_result_t kv_get_sqlitemt (kv_store_t gst, const kv_key_t *key, os_uint32 *sz, void **val, const struct c_type_s *type)
{
    struct kv_store_sqlitemt *st = (struct kv_store_sqlitemt *) gst;
    struct sqlite3_stmt *pstmt = st->pstmt_get[key->table];
    kv_result_t result;
    if ((int) key->gid < 0 || (int) key->iid < 0 || (os_int64) key->mid < 0) {
        return KV_RESULT_ERROR;
    }
    if ((result = kv_sqlitemt_bind_key (key, pstmt)) < 0) {
        return result;
    }
    switch (sqlite3_step (pstmt))
    {
        case SQLITE_DONE:
            result = KV_RESULT_NODATA;
            break;
        case SQLITE_ROW:
            result = 0;
            kv_sqlitemt_extract_payload (sz, val, st, key->table, pstmt, type, 0);
            break;
        default:
            result = KV_RESULT_ERROR;
            break;
    }
    sqlite3_reset (pstmt);
    return result;
}

static kv_result_t kv_put_sqlitemt (kv_store_t gst, const kv_key_t *key, os_uint32 sz, const void *val, const struct c_type_s *type)
{
    struct kv_store_sqlitemt *st = (struct kv_store_sqlitemt *) gst;
    struct sqlite3_stmt *pstmt = st->pstmt_put[key->table];
    kv_result_t result;
    if ((int) key->gid < 0 || (int) key->iid < 0 || (os_int64) key->mid < 0) {
        return KV_RESULT_ERROR;
    }
    if ((result = kv_sqlitemt_bind_key (key, pstmt)) < 0) {
        return result;
    }
    result = 0;
    switch (key->table) {
        case TABLE_VERSION: {
            if (sqlite3_bind_blob (pstmt, 2, val, (int) sz, SQLITE_TRANSIENT) != SQLITE_OK) {
                result = KV_RESULT_ERROR;
            }
            break;
        }
        case TABLE_TOPIC: {
            struct d_topicKV *x = kv_sqlitemt_deser (st, type, sz, val);
            if (sqlite3_bind_text (pstmt, 2, x->name, -1, SQLITE_TRANSIENT) != SQLITE_OK ||
                sqlite3_bind_text (pstmt, 3, x->type_name, -1, SQLITE_TRANSIENT) != SQLITE_OK ||
                sqlite3_bind_text (pstmt, 4, x->keylist, -1, SQLITE_TRANSIENT) != SQLITE_OK ||
                sqlite3_bind_text (pstmt, 5, val, -1, SQLITE_TRANSIENT) != SQLITE_OK) {
                result = KV_RESULT_ERROR;
            }
            c_free (x);
            break;
        }
        case TABLE_TOPIC_TYPE: {
            if (sqlite3_bind_text (pstmt, 2, val, (int) sz, SQLITE_TRANSIENT) != SQLITE_OK) {
                result = KV_RESULT_ERROR;
            }
            break;
        }
        case TABLE_GROUP: {
            struct d_groupKV *x = kv_sqlitemt_deser (st, type, sz, val);
            if (sqlite3_bind_int (pstmt, 2, (int) x->topic_id) != SQLITE_OK ||
                sqlite3_bind_int (pstmt, 3, (int) x->namespace_id) != SQLITE_OK ||
                sqlite3_bind_int (pstmt, 4, (int) x->namespace_version) != SQLITE_OK ||
                sqlite3_bind_text (pstmt, 5, x->partition, -1, SQLITE_TRANSIENT) != SQLITE_OK ||
                sqlite3_bind_text (pstmt, 6, x->topic, -1, SQLITE_TRANSIENT) != SQLITE_OK) {
                result = KV_RESULT_ERROR;
            }
            c_free (x);
            break;
        }
        case TABLE_NAMESPACE: {
            struct d_namespaceKV *x = kv_sqlitemt_deser (st, type, sz, val);
            if (sqlite3_bind_text (pstmt, 3, x->name, -1, SQLITE_TRANSIENT) != SQLITE_OK) {
                result = KV_RESULT_ERROR;
            }
            c_free (x);
            break;
        }
        case TABLE_NAMESPACE_QUALITY: {
            struct d_namespaceQualityKV *x = kv_sqlitemt_deser (st, type, sz, val);
            if (sqlite3_bind_int (pstmt, 3, (int) x->quality.seconds) != SQLITE_OK ||
                sqlite3_bind_int (pstmt, 4, (int) x->quality.nanoseconds) != SQLITE_OK) {
                result = KV_RESULT_ERROR;
            }
            c_free (x);
            break;
        }
        case TABLE_NAMESPACE_COMPLETENESS: {
            struct d_namespaceCompletenessKV *x = kv_sqlitemt_deser (st, type, sz, val);
            if (sqlite3_bind_int (pstmt, 3, x->complete) != SQLITE_OK) {
                result = KV_RESULT_ERROR;
            }
            c_free (x);
            break;
        }
        case TABLE_MESSAGE:
            if (sqlite3_bind_text (pstmt, 4, val, (int) sz, SQLITE_TRANSIENT) != SQLITE_OK) {
                result = KV_RESULT_ERROR;
            }
            break;
        case TABLE_MESSAGE_EOT:
            if (sqlite3_bind_text (pstmt, 2, val, (int) sz, SQLITE_TRANSIENT) != SQLITE_OK) {
                result = KV_RESULT_ERROR;
            }
            break;
    }
    if (result < 0) {
        return result;
    }
    switch (sqlite3_step (pstmt))
    {
        case SQLITE_DONE:
            result = 0;
            break;
        default:
            result = KV_RESULT_ERROR;
            break;
    }
    sqlite3_reset (pstmt);
    return result;
}

static kv_result_t kv_delete_sqlitemt (kv_store_t gst, const kv_key_t *key)
{
    struct kv_store_sqlitemt *st = (struct kv_store_sqlitemt *) gst;
    struct sqlite3_stmt *pstmt = st->pstmt_delete[key->table];
    kv_result_t result;
    if ((int) key->gid < 0 || (int) key->iid < 0 || (os_int64) key->mid < 0) {
        return KV_RESULT_ERROR;
    }
    if ((result = kv_sqlitemt_bind_key (key, pstmt)) < 0) {
        return result;
    }
    switch (sqlite3_step (pstmt))
    {
        case SQLITE_DONE:
            result = 0;
            break;
        default:
            result = KV_RESULT_ERROR;
            break;
    }
    sqlite3_reset (pstmt);
    return result;
}

static int kv_sqlitemt_rangequery (char *sql, size_t sqlsize, enum kv_table table, const char *oper, const char *orderby)
{
    int nkeys = 0;
    int pos;
    char *primkey_copy, *cursor, *tok;
    pos = snprintf (sql, sqlsize, "%s %s WHERE", oper, kv_sqlitemt_tabtab[table].name);
    primkey_copy = cursor = os_strdup (kv_sqlitemt_tabtab[table].primkey);
    for (tok = os_strtok_r (primkey_copy, ", ", &cursor); tok != NULL; tok = os_strtok_r (NULL, ", ", &cursor)) {
        assert ((size_t) pos < sqlsize);
        pos += snprintf (sql + pos, sqlsize - (size_t) pos, " %s(%s BETWEEN ? AND ?)", (nkeys == 0) ? "" : "AND ", tok);
        nkeys++;
    }
    os_free (primkey_copy);
    assert ((size_t) pos < sqlsize);
    if (orderby) {
        pos += snprintf (sql + pos, sqlsize - (size_t) pos, " ORDER BY %s", orderby);
        assert ((size_t) pos < sqlsize);
    }
    return nkeys;
}

static kv_result_t kv_sqlitemt_bindrange (struct sqlite3_stmt *pstmt, int nkeys, const kv_key_t *first, const kv_key_t *last)
{
    assert (1 <= nkeys && nkeys <= 3);
    if (nkeys >= 1) {
        if (sqlite3_bind_int (pstmt, 1, (int) (first->gid & 0x7fffffff)) != SQLITE_OK ||
            sqlite3_bind_int (pstmt, 2, (int) (last->gid & 0x7fffffff)) != SQLITE_OK) {
            return KV_RESULT_ERROR;
        }
    }
    if (nkeys >= 2) {
        if (sqlite3_bind_int (pstmt, 3, (int) (first->iid & 0x7fffffff)) != SQLITE_OK ||
            sqlite3_bind_int (pstmt, 4, (int) (last->iid & 0x7fffffff)) != SQLITE_OK) {
            return KV_RESULT_ERROR;
        }
    }
    if (nkeys >= 3) {
        if (sqlite3_bind_int64 (pstmt, 5, (os_int64) (first->mid & ~((os_uint64)1 << 63))) != SQLITE_OK ||
            sqlite3_bind_int64 (pstmt, 6, (os_int64) (last->mid & ~((os_uint64)1 << 63))) != SQLITE_OK) {
            return KV_RESULT_ERROR;
        }
    }
    return 0;
}

static kv_result_t kv_bulkdelete_sqlitemt (kv_store_t gst, const kv_key_t *first, const kv_key_t *last)
{
    struct kv_store_sqlitemt *st = (struct kv_store_sqlitemt *) gst;
    struct sqlite3_stmt *pstmt;
    int nkeys;
    char sql[512];
    kv_result_t result;

    nkeys = kv_sqlitemt_rangequery (sql, sizeof (sql), first->table, "DELETE FROM", NULL);
    if (sqlite3_prepare_v2 (st->x.db, sql, -1, &pstmt, NULL) != SQLITE_OK) {
        return KV_RESULT_ERROR;
    }
    if ((result = kv_sqlitemt_bindrange (pstmt, nkeys, first, last)) < 0) {
        return result;
    }
    /* Deleting a lot of data in one operation can take very long. Declare
     * the thread to be asleep to prevent the liveliness checker from
     * complaining. A year is roughly 31e6 seconds - surely that is enough */
    d_threadAsleep (d_threadLookupSelf (), 31000000);
    switch (sqlite3_step (pstmt))
    {
        case SQLITE_DONE:
            result = 0;
            break;
        default:
            result = KV_RESULT_ERROR;
            break;
    }
    d_threadAwake (d_threadLookupSelf ());
    (void)sqlite3_finalize (pstmt);
    return result;
}

static kv_result_t kv_iter_free_sqlitemt (kv_store_t gst, kv_iter_t giter)
{
    kv_result_t rc = 0;
    struct kv_iter_sqlitemt *it = (struct kv_iter_sqlitemt *) giter;
    OS_UNUSED_ARG (gst);
    if (sqlite3_finalize (it->pstmt) != SQLITE_OK) {
        rc = KV_RESULT_ERROR;
    }
    os_free (it);
    return rc;
}

static kv_result_t kv_iter_next_sqlitemt (int *hasdata, kv_store_t gst, kv_iter_t giter)
{
    struct kv_iter_sqlitemt *it = (struct kv_iter_sqlitemt *) giter;
    OS_UNUSED_ARG (gst);
    switch (sqlite3_step (it->pstmt))
    {
        case SQLITE_DONE:
            *hasdata = 0;
            return 0;
        case SQLITE_ROW:
            *hasdata = 1;
            return 0;
        default:
            return KV_RESULT_ERROR;
    }
}

static kv_result_t kv_iter_getkey_sqlitemt (kv_store_t gst, kv_iter_t giter, kv_key_t *key)
{
    struct kv_iter_sqlitemt *it = (struct kv_iter_sqlitemt *) giter;
    OS_UNUSED_ARG (gst);
    key->table = it->table;
    if (it->nkeys >= 1) {
        key->gid = (os_uint32) sqlite3_column_int (it->pstmt, 0);
    }
    if (it->nkeys >= 2) {
        key->iid = (os_uint32) sqlite3_column_int (it->pstmt, 1);
    }
    if (it->nkeys >= 3) {
        key->mid = (os_uint64) sqlite3_column_int64 (it->pstmt, 2);
    }
    return 0;
}

static kv_result_t kv_iter_getvalue_sqlitemt (kv_store_t gst, kv_iter_t giter, os_uint32 *sz, void **val)
{
    struct kv_store_sqlitemt *st = (struct kv_store_sqlitemt *) gst;
    struct kv_iter_sqlitemt *it = (struct kv_iter_sqlitemt *) giter;
    OS_UNUSED_ARG (gst);
    kv_sqlitemt_extract_payload (sz, val, st, it->table, it->pstmt, it->type, it->nkeys);
    return 0;
}

static kv_result_t kv_iter_new_sqlitemt (kv_store_t gst, kv_iter_t *giter, const kv_key_t *first, const kv_key_t *last, const struct c_type_s *type)
{
    struct kv_store_sqlitemt *st = (struct kv_store_sqlitemt *) gst;
    struct kv_iter_sqlitemt *it;
    kv_result_t result;
    char sql[512];
    it = os_malloc (sizeof (*it));
    it->table = first->table;
    it->type = type;
    it->nkeys = kv_sqlitemt_rangequery (sql, sizeof (sql), first->table, "SELECT * FROM", kv_sqlitemt_tabtab[first->table].primkey);
    if (sqlite3_prepare_v2 (st->x.db, sql, -1, &it->pstmt, NULL) != SQLITE_OK) {
        os_free (it);
        return KV_RESULT_ERROR;
    }
    if ((result = kv_sqlitemt_bindrange (it->pstmt, it->nkeys, first, last)) < 0) {
        kv_iter_free_sqlite (gst, (kv_iter_t) it);
        return result;
    }
    *giter = (kv_iter_t) it;
    return 0;
}

static d_encodingKV kv_choose_encoding_sqlitemt (kv_store_t gst, d_encodingKV req)
{
    OS_UNUSED_ARG(gst);
    OS_UNUSED_ARG(req);
    return D_ENCODINGKV_XML_XML;
}

static kv_store_t kv_open_sqlitemt (struct d_storeKV_s *logst, const char *dir, const c_char *parameters)
{
    const char *filename = "kvmt.sqlite";
    struct kv_store_sqlitemt *st;
    char *path;
    int ret;
    size_t len;

    assert (dir);

    len = strlen (dir) + 1 + strlen (filename) + 1;

    path = os_malloc (len);
    snprintf (path, len, "%s/%s", dir, filename);

    st = os_malloc (sizeof (*st));
    if ((ret = sqlite3_open (path, &st->x.db)) != SQLITE_OK) {
        sqlite3_close (st->x.db);
        os_free (st);
        os_free (path);
        return NULL;
    } else {
        char sql[256];
        int i, n;
        os_free (path);
        st->x.logst = logst;
        st->x.pstmt_get = 0;
        st->x.pstmt_put = 0;
        st->x.pstmt_delete = 0;
        for (i = 0; i < N_TABLES; i++) {
            st->pstmt_get[i] = 0;
            st->pstmt_put[i] = 0;
            st->pstmt_delete[i] = 0;
        }

        assert (sizeof (kv_sqlitemt_tabtab) / sizeof (kv_sqlitemt_tabtab[0]) == (size_t) N_TABLES);
        for (i = 0; i < N_TABLES; i++) {
            sqlite3_stmt *pstmt;
            int ok;
            assert (kv_sqlitemt_tabtab[i].k == (enum kv_table) i);
            n = snprintf (sql, sizeof (sql), "CREATE TABLE IF NOT EXISTS \"%s\" (%s, PRIMARY KEY(%s))", kv_sqlitemt_tabtab[i].name, kv_sqlitemt_tabtab[i].cols, kv_sqlitemt_tabtab[i].primkey);
            assert (n < (int) sizeof (sql));
            if ((ret = sqlite3_prepare_v2 (st->x.db, sql, (int) (strlen (sql) + 1), &pstmt, NULL)) != SQLITE_OK) {
                break;
            }
            ok = (sqlite3_step (pstmt) == SQLITE_DONE);
            (void)sqlite3_finalize (pstmt);
            if (!ok) {
                break;
            }
            n = snprintf (sql, sizeof (sql), "SELECT %s FROM %s WHERE %s", kv_sqlitemt_tabtab[i].valcols, kv_sqlitemt_tabtab[i].name, kv_sqlitemt_tabtab[i].kspec);
            assert (n < (int) sizeof (sql));
            if ((ret = sqlite3_prepare_v2 (st->x.db, sql, (int) (strlen (sql) + 1), &st->pstmt_get[i], NULL)) != SQLITE_OK) {
                break;
            }
            n = snprintf (sql, sizeof (sql), "INSERT OR REPLACE INTO %s VALUES (%s)", kv_sqlitemt_tabtab[i].name, kv_sqlitemt_tabtab[i].vspec);
            assert (n < (int) sizeof (sql));
            if ((ret = sqlite3_prepare_v2 (st->x.db, sql, (int) (strlen (sql) + 1), &st->pstmt_put[i], NULL)) != SQLITE_OK) {
                break;
            }
            n = snprintf (sql, sizeof (sql), "DELETE FROM %s WHERE %s", kv_sqlitemt_tabtab[i].name, kv_sqlitemt_tabtab[i].kspec);
            assert (n < (int) sizeof (sql));
            if ((ret = sqlite3_prepare_v2 (st->x.db, sql, (int) (strlen (sql) + 1), &st->pstmt_delete[i], NULL)) != SQLITE_OK) {
                break;
            }
            OS_UNUSED_ARG(n);
        }

        if (i != N_TABLES) {
            int j;
            for (j = 0; j < N_TABLES; j++) {
                if (st->pstmt_get[j]) { (void)sqlite3_finalize (st->pstmt_get[j]); }
                if (st->pstmt_put[j]) { (void)sqlite3_finalize (st->pstmt_put[j]); }
                if (st->pstmt_delete[j]) { (void)sqlite3_finalize (st->pstmt_delete[j]); }
            }
            sqlite3_close (st->x.db);
            os_free (st);
            return NULL;
        }

        st->x.c.write_access = 1;
        st->x.c.kind = KV_KIND_SQLITEMT;
        st->x.c.close = kv_close_sqlitemt;
        st->x.c.commit = kv_commit_sqlitemt;
        st->x.c.get = kv_get_sqlitemt;
        st->x.c.put = kv_put_sqlitemt;
        st->x.c.delete = kv_delete_sqlitemt;
        st->x.c.bulkdelete = kv_bulkdelete_sqlitemt;
        st->x.c.iter_new = kv_iter_new_sqlitemt;
        st->x.c.iter_free = kv_iter_free_sqlitemt;
        st->x.c.iter_next = kv_iter_next_sqlitemt;
        st->x.c.iter_getkey = kv_iter_getkey_sqlitemt;
        st->x.c.iter_getvalue = kv_iter_getvalue_sqlitemt;
        st->x.c.choose_encoding = kv_choose_encoding_sqlitemt;

        if (kv_configure_sqlite (&st->x, parameters) != 0) {
            kv_close_sqlitemt ((kv_store_t) st);
            return NULL;
        } else if (sqlite_execsimple (&st->x, "BEGIN") < 0) {
            kv_close_sqlitemt ((kv_store_t) st);
            return NULL;
        } else {
            return (kv_store_t) st;
        }
    }
}
#endif /* KV_INCLUDE_SQLITE */

#if KV_INCLUDE_LEVELDB
#include <leveldb/c.h>

struct kv_store_leveldb {
    struct kv_store_common c;
    struct d_storeKV_s *logst;
    leveldb_t *db;
    os_timeW t_last_commit;
    leveldb_options_t *options;
    leveldb_readoptions_t *readoptions;
    leveldb_writeoptions_t *writeoptions;
    leveldb_writeoptions_t *writeoptions_sync;
};

struct kv_iter_leveldb {
    leveldb_iterator_t *iter;
    struct kv_blobkey first;
    struct kv_blobkey last;
    int started;
    struct kv_blobkey cursor;
};

static kv_result_t kv_leveldb_err (struct kv_store_leveldb *st, char *errptr)
{
    if (errptr == NULL) {
        return 0;
    } else {
        kvlog (st->logst, D_LEVEL_SEVERE, "leveldb: %s\n", errptr);
        leveldb_free (errptr);
        return KV_RESULT_ERROR;
    }
}

static kv_result_t kv_close_leveldb (kv_store_t gst)
{
    struct kv_store_leveldb *st = (struct kv_store_leveldb *) gst;
    leveldb_options_destroy (st->options);
    leveldb_readoptions_destroy (st->readoptions);
    leveldb_writeoptions_destroy (st->writeoptions);
    leveldb_writeoptions_destroy (st->writeoptions_sync);
    leveldb_close (st->db);
    os_free (st);
    return 0;
}

static kv_result_t kv_commit_leveldb (kv_store_t gst)
{
    const os_duration tdelta = OS_DURATION_INIT(1,0);
    struct kv_store_leveldb *st = (struct kv_store_leveldb *) gst;
    os_timeW tnow = os_timeWGet ();
    if (1 || os_durationCompare (os_timeWDiff (tnow, st->t_last_commit), tdelta) == OS_LESS) {
        return 0;
    } else {
        kv_key_t key;
        struct kv_blobkey k;
        char val = 0;
        char *errptr = NULL;
        st->t_last_commit = tnow;
        key.table = TABLE_VERSION;
        key.gid = MAX_GID;
        key.iid = 0;
        key.mid = 0;
        to_blobkey (&k, &key);
        leveldb_put (st->db, st->writeoptions_sync, (char *) &k, sizeof (k), &val, 1, &errptr);
        return kv_leveldb_err (st, errptr);
    }
}

static kv_result_t kv_get_leveldb (kv_store_t gst, const kv_key_t *key, os_uint32 *sz, void **val, const struct c_type_s *type)
{
    struct kv_store_leveldb *st = (struct kv_store_leveldb *) gst;
    struct kv_blobkey k;
    size_t tmpsz;
    char *tmpval;
    char *errptr = NULL;
    OS_UNUSED_ARG (type);
    to_blobkey (&k, key);
    tmpval = leveldb_get (st->db, st->readoptions, (char *) &k, sizeof (k), &tmpsz, &errptr);
    if (errptr) {
        return kv_leveldb_err (st, errptr);
    } else if (tmpval == NULL) {
        return KV_RESULT_NODATA;
    } else {
        *sz = (os_uint32) tmpsz;
        if ((*val = os_malloc (*sz ? *sz : 1)) == NULL) {
            leveldb_free (tmpval);
            return KV_RESULT_OUT_OF_RESOURCES;
        } else {
            memcpy (*val, tmpval, *sz);
            leveldb_free (tmpval);
            return 0;
        }
    }
}

static kv_result_t kv_put_leveldb (kv_store_t gst, const kv_key_t *key, os_uint32 sz, const void *val, const struct c_type_s *type)
{
    struct kv_store_leveldb *st = (struct kv_store_leveldb *) gst;
    struct kv_blobkey k;
    char *errptr = NULL;
    OS_UNUSED_ARG (type);
    to_blobkey (&k, key);
    leveldb_put (st->db, st->writeoptions_sync, (char *) &k, sizeof (k), val, sz, &errptr);
    return kv_leveldb_err (st, errptr);
}

static kv_result_t kv_delete_leveldb (kv_store_t gst, const kv_key_t *key)
{
    struct kv_store_leveldb *st = (struct kv_store_leveldb *) gst;
    struct kv_blobkey k;
    char *errptr = NULL;
    to_blobkey (&k, key);
    leveldb_delete (st->db, st->writeoptions, (char *) &k, sizeof (k), &errptr);
    return kv_leveldb_err (st, errptr);
}

static kv_result_t kv_bulkdelete_leveldb (kv_store_t gst, const kv_key_t *first, const kv_key_t *last)
{
    struct kv_store_leveldb *st = (struct kv_store_leveldb *) gst;
    struct kv_blobkey k0, k1;
    leveldb_iterator_t *iter;
    char *errptr = NULL;
    to_blobkey (&k0, first);
    to_blobkey (&k1, last);
    if ((iter = leveldb_create_iterator (st->db, st->readoptions)) == NULL) {
        return KV_RESULT_ERROR;
    }
    leveldb_iter_seek (iter, (char *) &k0, sizeof (k0));
    while (leveldb_iter_valid (iter)) {
        const char *tmpkey;
        size_t tmpsz;
        if ((tmpkey = leveldb_iter_key (iter, &tmpsz)) == NULL) {
            leveldb_iter_destroy (iter);
            return KV_RESULT_OUT_OF_RESOURCES;
        } else if (tmpsz != sizeof (k1)) {
            leveldb_iter_destroy (iter);
            return KV_RESULT_CORRUPT;
        } else if (memcmp (tmpkey, &k1, sizeof (k1)) > 0) {
            break;
        }
        leveldb_iter_next (iter);
        leveldb_delete (st->db, st->writeoptions, tmpkey, tmpsz, &errptr);
    }
    leveldb_iter_get_error (iter, &errptr);
    leveldb_iter_destroy (iter);
    return kv_leveldb_err (st, errptr);
}

static kv_result_t kv_iter_free_leveldb (kv_store_t gst, kv_iter_t giter)
{
    struct kv_iter_leveldb *it = (struct kv_iter_leveldb *) giter;
    OS_UNUSED_ARG (gst);
    leveldb_iter_destroy (it->iter);
    os_free (it);
    return 0;
}

static kv_result_t kv_iter_next_leveldb (int *hasdata, kv_store_t gst, kv_iter_t giter)
{
    struct kv_store_leveldb *st = (struct kv_store_leveldb *) gst;
    struct kv_iter_leveldb *it = (struct kv_iter_leveldb *) giter;
    if (it->started) {
        leveldb_iter_next (it->iter);
    } else {
        leveldb_iter_seek (it->iter, (char *) &it->first, sizeof (it->first));
        it->started = 1;
    }
    if (!leveldb_iter_valid (it->iter)) {
        char *errptr = NULL;
        leveldb_iter_get_error (it->iter, &errptr);
        *hasdata = 0;
        return kv_leveldb_err (st, errptr);
    } else {
        const char *tmpkey;
        size_t tmpsz;
        kv_result_t rc;
        if ((tmpkey = leveldb_iter_key (it->iter, &tmpsz)) == NULL) {
            return KV_RESULT_OUT_OF_RESOURCES;
        }
        if (tmpsz != sizeof (it->cursor)) {
            rc = KV_RESULT_CORRUPT;
        } else if (memcmp (tmpkey, &it->last, sizeof (it->last)) > 0) {
            *hasdata = 0;
            rc = 0;
        } else {
            memcpy (&it->cursor, tmpkey, sizeof (it->cursor));
            *hasdata = 1;
            rc = 0;
        }
        return rc;
    }
}

static kv_result_t kv_iter_getkey_leveldb (kv_store_t gst, kv_iter_t giter, kv_key_t *key)
{
    struct kv_iter_leveldb *it = (struct kv_iter_leveldb *) giter;
    OS_UNUSED_ARG (gst);
    from_blobkey (key, &it->cursor);
    return 0;
}

static kv_result_t kv_iter_getvalue_leveldb (kv_store_t gst, kv_iter_t giter, os_uint32 *sz, void **val)
{
    struct kv_iter_leveldb *it = (struct kv_iter_leveldb *) giter;
    const char *tmpval;
    size_t tmpsz;
    kv_result_t rc;
    OS_UNUSED_ARG (gst);
    if ((tmpval = leveldb_iter_value (it->iter, &tmpsz)) == NULL) {
        return KV_RESULT_OUT_OF_RESOURCES;
    }
    *sz = (os_uint32) tmpsz;
    if ((*val = os_malloc (*sz ? *sz : 1)) == NULL) {
        rc = KV_RESULT_CORRUPT;
    } else {
        memcpy (*val, tmpval, tmpsz);
        rc = 0;
    }
    return rc;
}

static kv_result_t kv_iter_new_leveldb (kv_store_t gst, kv_iter_t *giter, const kv_key_t *first, const kv_key_t *last, const struct c_type_s *type)
{
    struct kv_store_leveldb *st = (struct kv_store_leveldb *) gst;
    struct kv_iter_leveldb *it;
    OS_UNUSED_ARG (type);
    it = os_malloc (sizeof (*it));
    it->started = 0;
    to_blobkey (&it->first, first);
    to_blobkey (&it->last, last);
    memset (&it->cursor, 0, sizeof (it->cursor));
    if ((it->iter = leveldb_create_iterator (st->db, st->readoptions)) == NULL) {
        os_free (it);
        return KV_RESULT_ERROR;
    }
    *giter = (kv_iter_t) it;
    return 0;
}

typedef enum {
    KV_LEVELDB_PARANOID_CHECKS,
    KV_LEVELDB_WRITE_BUFFER_SIZE,
    KV_LEVELDB_MAX_OPEN_FILES,
    KV_LEVELDB_BLOCK_SIZE,
    KV_LEVELDB_VERIFY_CHECKSUMS,
    KV_LEVELDB_FILL_CACHE
} kv_leveldb_parm_t;

struct kv_leveldb_parm {
    const char        *name;
    kv_leveldb_parm_t  kind;
};

static struct kv_leveldb_parm kv_leveldb_parms[] = {
    { "paranoid_checks",   KV_LEVELDB_PARANOID_CHECKS   },
    { "write_buffer_size", KV_LEVELDB_WRITE_BUFFER_SIZE },
    { "max_open_files",    KV_LEVELDB_MAX_OPEN_FILES    },
    { "block_size",        KV_LEVELDB_BLOCK_SIZE        },
    { "verify_checksums",  KV_LEVELDB_VERIFY_CHECKSUMS  },
    { "fill_cache",        KV_LEVELDB_FILL_CACHE        }
};
static unsigned int kv_leveldb_parms_len = sizeof(kv_leveldb_parms)/sizeof(struct kv_leveldb_parm);


static int kv_leveldb_get_parameter(const char *pstr, char **key, char **value)
{
    c_char *str;
    c_char *p;
    c_char kstr[256];
    c_char vstr[256];
    int r = 0;

    *key = NULL;
    *value = NULL;

    str = os_strdup(pstr);
    p = os_index(str, '=');
    if (p) {
        *p = ' ';
        if (sscanf(str, "%s%s", kstr, vstr) == 2) {
            *key = os_strdup(kstr);
            *value = os_strdup(vstr);
            r = 1;
        }
    }
    os_free(str);

    return r;
}

static int kv_configure_leveldb (struct kv_store_leveldb *st, const c_char *parameters)
{
    c_char *str;
    c_char *ptr;
    c_char *tok;

    if (parameters) {
        str = os_strdup(parameters);
        tok = os_strtok_r(str, ";", &ptr);
        while (tok) {
            unsigned int i;
            c_bool found  = FALSE;
            c_char *key   = NULL;
            c_char *value = NULL;
            int val;

            if (kv_leveldb_get_parameter(tok, &key, &value)) {
                for (i = 0; !found && (i < kv_leveldb_parms_len); i++) {
                    found = os_strncasecmp(kv_leveldb_parms[i].name, key, strlen(kv_leveldb_parms[i].name)) == 0;
                }
            }
            if (found) {
                switch (kv_leveldb_parms[i].kind) {
                case KV_LEVELDB_PARANOID_CHECKS:
                    if (value) {
                        if (os_strcasecmp(value, "true") == 0) {
                            leveldb_options_set_paranoid_checks(st->options, 1);
                        } else if (os_strcasecmp(value, "false") == 0) {
                            leveldb_options_set_paranoid_checks(st->options, 0);
                        }
                    }
                    break;
                case KV_LEVELDB_WRITE_BUFFER_SIZE:
                    if (value) {
                        val = atoi(value);
                        if (val > 0) {
                            leveldb_options_set_write_buffer_size(st->options, (size_t) val);
                        }
                    }
                    break;
                case KV_LEVELDB_MAX_OPEN_FILES:
                    if (value) {
                        val = atoi(value);
                        if (val) {
                            leveldb_options_set_max_open_files(st->options, val);
                        }
                    }
                    break;
                case KV_LEVELDB_BLOCK_SIZE:
                    if (value) {
                        val = atoi(value);
                        if (val > 0) {
                            leveldb_options_set_block_size(st->options, (size_t) val);
                        }
                    }
                    break;
                case KV_LEVELDB_VERIFY_CHECKSUMS:
                    if (value) {
                        if (os_strcasecmp(value, "true") == 0) {
                            leveldb_readoptions_set_verify_checksums(st->readoptions, 1);
                        } else if (os_strcasecmp(value, "false") == 0) {
                            leveldb_readoptions_set_verify_checksums(st->readoptions, 1);
                        }
                    }
                    break;
                case KV_LEVELDB_FILL_CACHE:
                    if (value) {
                        if (os_strcasecmp(value, "true") == 0) {
                            leveldb_options_set_paranoid_checks(st->options, 1);
                        } else if (os_strcasecmp(value, "false") == 0) {
                            leveldb_options_set_paranoid_checks(st->options, 0);
                        }
                    }
                    break;
                }
            }
            os_free(key);
            os_free(value);
            tok = os_strtok_r(NULL, ";", &ptr);
        }
        os_free(str);
    }

    return 0;
}


static kv_store_t kv_open_leveldb (struct d_storeKV_s *logst, const char *dir, const c_char *parameters)
{
    const char *filename = "kv.leveldb";
    struct kv_store_leveldb *st;
    char *path;
    char *errptr = NULL;
    size_t len;

    assert (dir);

    len = strlen (dir) + 1 + strlen (filename) + 1;

    path = os_malloc (len);
    snprintf (path, len, "%s/%s", dir, filename);

    st = os_malloc (sizeof (*st));
    st->logst =logst;
    st->t_last_commit = os_timeWGet ();
    st->options = leveldb_options_create ();
    leveldb_options_set_create_if_missing (st->options, 1);
    leveldb_options_set_block_size (st->options, 1048576);

    st->readoptions = leveldb_readoptions_create ();
    st->writeoptions = leveldb_writeoptions_create ();
    st->writeoptions_sync = leveldb_writeoptions_create ();
    leveldb_writeoptions_set_sync (st->writeoptions_sync, 1);

    (void)kv_configure_leveldb(st, parameters);

    if ((st->db = leveldb_open (st->options, path, &errptr)) == NULL) {
        (void) kv_leveldb_err (st, errptr);
        os_free (st);
        os_free (path);
        return NULL;
    }

    st->c.write_access = 1;
    st->c.kind = KV_KIND_LEVELDB;
    st->c.close = kv_close_leveldb;
    st->c.commit = kv_commit_leveldb;
    st->c.get = kv_get_leveldb;
    st->c.put = kv_put_leveldb;
    st->c.delete = kv_delete_leveldb;
    st->c.bulkdelete = kv_bulkdelete_leveldb;
    st->c.iter_new = kv_iter_new_leveldb;
    st->c.iter_free = kv_iter_free_leveldb;
    st->c.iter_next = kv_iter_next_leveldb;
    st->c.iter_getkey = kv_iter_getkey_leveldb;
    st->c.iter_getvalue = kv_iter_getvalue_leveldb;
    st->c.choose_encoding = kv_choose_encoding_def;

    os_free(path);

    return (kv_store_t) st;
}
#endif /* KV_INCLUDE_LEVELDB */

#if KV_INCLUDE_LSDS
#include <log.h>

struct kv_store_lsds {
    struct kv_store_common c;
    struct d_storeKV_s *logst;
    log_t db;
};

struct kv_iter_lsds {
    int dummy;
};

static kv_result_t kv_close_lsds (kv_store_t gst)
{
    struct kv_store_lsds *st = (struct kv_store_lsds *) gst;
    closelog (st->db);
    os_free (st);
    return 0;
}

static kv_result_t kv_commit_lsds (kv_store_t gst)
{
    OS_UNUSED_ARG (gst);
    return 0;
}

static kv_result_t kv_get_lsds (kv_store_t gst, const kv_key_t *key, os_uint32 *sz, void **val, const struct c_type_s *type)
{
    struct kv_store_lsds *st = (struct kv_store_lsds *) gst;
    uint32_t tmpsz;
    struct { struct l_insertion hdr; os_uint32 realsize; } *tmpval;
    l_key_t k;
    OS_UNUSED_ARG (type);
    k.size = 0;
    k.group = ((os_uint64) key->table << 56) | ((os_uint64) key->gid << 32) | key->iid;
    k.id = key->mid;
    if ((tmpval = logread (&tmpsz, st->db, &k)) == NULL) {
        return KV_RESULT_NODATA;
    } else if (tmpsz <= sizeof (*tmpval)) {
        free (tmpval);
        return KV_RESULT_CORRUPT;
    } else {
        *sz = tmpval->realsize;
        if ((*val = os_malloc (*sz ? *sz : 1)) == NULL) {
            free (tmpval);
            return KV_RESULT_OUT_OF_RESOURCES;
        } else {
            memcpy (*val, tmpval + 1, *sz);
            free (tmpval);
            return 0;
        }
    }
}

static kv_result_t kv_put_lsds (kv_store_t gst, const kv_key_t *key, os_uint32 sz, const void *val, const struct c_type_s *type)
{
    struct kv_store_lsds *st = (struct kv_store_lsds *) gst;
    l_key_t k;
    struct { struct l_insertion hdr; os_uint32 realsize; } *x;
    OS_UNUSED_ARG (type);
    x = os_malloc (sizeof (*x) + ((sz + 7) & -8));
    x->realsize = sz;
    memcpy (x + 1, val, sz);
    k.size = sizeof (*x) + ((sz + 7) & -8);
    k.group = ((os_uint64) key->table << 56) | ((os_uint64) key->gid << 32) | key->iid;
    k.id = key->mid;
    loginsertion (st->db, &k, x);
    os_free (x);
    return 0;
}

static kv_result_t kv_delete_lsds (kv_store_t gst, const kv_key_t *key)
{
    struct kv_store_lsds *st = (struct kv_store_lsds *) gst;
    l_key_t k;
    k.size = 0;
    k.group = ((os_uint64) key->table << 56) | ((os_uint64) key->gid << 32) | key->iid;
    k.id = key->mid;
    logdeletion (st->db, &k);
    return 0;
}

static kv_result_t kv_bulkdelete_lsds (kv_store_t gst, const kv_key_t *first, const kv_key_t *last)
{
    OS_UNUSED_ARG (gst);
    OS_UNUSED_ARG (first);
    OS_UNUSED_ARG (last);
    return 0;
}

static kv_result_t kv_iter_free_lsds (kv_store_t gst, kv_iter_t giter)
{
    struct kv_iter_lsds *it = (struct kv_iter_lsds *) giter;
    OS_UNUSED_ARG (gst);
    os_free (it);
    return 0;
}

static kv_result_t kv_iter_next_lsds (int *hasdata, kv_store_t gst, kv_iter_t giter)
{
    OS_UNUSED_ARG (gst);
    OS_UNUSED_ARG (giter);
    *hasdata = 0;
    return 0;
}

static kv_result_t kv_iter_getkey_lsds (kv_store_t gst, kv_iter_t giter, kv_key_t *key)
{
    OS_UNUSED_ARG (gst);
    OS_UNUSED_ARG (giter);
    OS_UNUSED_ARG (key);
    return 0;
}

static kv_result_t kv_iter_getvalue_lsds (kv_store_t gst, kv_iter_t giter, os_uint32 *sz, void **val)
{
    OS_UNUSED_ARG (gst);
    OS_UNUSED_ARG (giter);
    OS_UNUSED_ARG (sz);
    OS_UNUSED_ARG (val);
    return 0;
}

static kv_result_t kv_iter_new_lsds (kv_store_t gst, kv_iter_t *giter, const kv_key_t *first, const kv_key_t *last, const struct c_type_s *type)
{
    struct kv_iter_lsds *it;
    OS_UNUSED_ARG (gst);
    OS_UNUSED_ARG (first);
    OS_UNUSED_ARG (last);
    OS_UNUSED_ARG (type);
    it = os_malloc (sizeof (*it));
    *giter = (kv_iter_t) it;
    return 0;
}

static kv_store_t kv_open_lsds (struct d_storeKV_s *logst, const char *dir, const c_char *parameters)
{
    const char *filename = "kv.lsds";
    struct kv_store_lsds *st;
    char *path;

    path = os_malloc (strlen (dir) + 1 + strlen (filename) + 1);
    sprintf (path, "%s/%s", dir, filename);

    st = os_malloc (sizeof (*st));
    st->logst = logst;

    if (access (path, R_OK | W_OK) == -1) {
        if ((st->db = newlog (path, 160 * 1048576, 2 * 1048576, 1, 100, USE_SYNC | MARK_ROOTS | ENABLE_SCAVENGER | OPTIMIZED_SCAVENGER)) == NULL) {
            kvlog (logst, D_LEVEL_SEVERE, "lsds: newlog failed\n");
        }
    } else if ((st->db = openlog (path, 100, USE_SYNC | MARK_ROOTS)) == NULL) {
        kvlog (logst, D_LEVEL_SEVERE, "lsds: openlog failed\n");
    }
    if (st->db == NULL) {
        os_free (path);
        os_free (st);
        return NULL;
    }

    st->c.write_access = 1;
    st->c.kind = KV_KIND_LSDS;
    st->c.close = kv_close_lsds;
    st->c.commit = kv_commit_lsds;
    st->c.get = kv_get_lsds;
    st->c.put = kv_put_lsds;
    st->c.delete = kv_delete_lsds;
    st->c.bulkdelete = kv_bulkdelete_lsds;
    st->c.iter_new = kv_iter_new_lsds;
    st->c.iter_free = kv_iter_free_lsds;
    st->c.iter_next = kv_iter_next_lsds;
    st->c.iter_getkey = kv_iter_getkey_lsds;
    st->c.iter_getvalue = kv_iter_getvalue_lsds;
    st->c.choose_encoding = kv_choose_encoding_def;

    return (kv_store_t) st;
}
#endif /* KV_INCLUDE_LSDS */

#if KV_INCLUDE_DUMMY
struct kv_store_dummy {
    struct kv_store_common c;
};

struct kv_iter_dummy {
    int dummy;
};

static kv_result_t kv_close_dummy (kv_store_t gst)
{
    os_free (gst);
    return 0;
}

static kv_result_t kv_commit_dummy (kv_store_t gst)
{
    OS_UNUSED_ARG (gst);
    return 0;
}

static kv_result_t kv_get_dummy (kv_store_t gst, const kv_key_t *key, os_uint32 *sz, void **val, const struct c_type_s *type)
{
    OS_UNUSED_ARG (gst);
    OS_UNUSED_ARG (key);
    OS_UNUSED_ARG (sz);
    OS_UNUSED_ARG (val);
    OS_UNUSED_ARG (type);
    return KV_RESULT_NODATA;
}

static kv_result_t kv_put_dummy (kv_store_t gst, const kv_key_t *key, os_uint32 sz, const void *val, const struct c_type_s *type)
{
    OS_UNUSED_ARG (gst);
    OS_UNUSED_ARG (key);
    OS_UNUSED_ARG (sz);
    OS_UNUSED_ARG (val);
    OS_UNUSED_ARG (type);
    return 0;
}

static kv_result_t kv_delete_dummy (kv_store_t gst, const kv_key_t *key)
{
    OS_UNUSED_ARG (gst);
    OS_UNUSED_ARG (key);
    return 0;
}

static kv_result_t kv_bulkdelete_dummy (kv_store_t gst, const kv_key_t *first, const kv_key_t *last)
{
    OS_UNUSED_ARG (gst);
    OS_UNUSED_ARG (first);
    OS_UNUSED_ARG (last);
    return 0;
}

static kv_result_t kv_iter_free_dummy (kv_store_t gst, kv_iter_t giter)
{
    struct kv_iter_dummy *it = (struct kv_iter_dummy *) giter;
    OS_UNUSED_ARG (gst);
    os_free (it);
    return 0;
}

static kv_result_t kv_iter_next_dummy (int *hasdata, kv_store_t gst, kv_iter_t giter)
{
    OS_UNUSED_ARG (gst);
    OS_UNUSED_ARG (giter);
    *hasdata = 0;
    return 0;
}

static kv_result_t kv_iter_getkey_dummy (kv_store_t gst, kv_iter_t giter, kv_key_t *key)
{
    OS_UNUSED_ARG (gst);
    OS_UNUSED_ARG (giter);
    OS_UNUSED_ARG (key);
    return 0;
}

static kv_result_t kv_iter_getvalue_dummy (kv_store_t gst, kv_iter_t giter, os_uint32 *sz, void **val)
{
    OS_UNUSED_ARG (gst);
    OS_UNUSED_ARG (giter);
    OS_UNUSED_ARG (sz);
    OS_UNUSED_ARG (val);
    return 0;
}

static kv_result_t kv_iter_new_dummy (kv_store_t gst, kv_iter_t *giter, const kv_key_t *first, const kv_key_t *last, const struct c_type_s *type)
{
    struct kv_iter_dummy *it;
    OS_UNUSED_ARG (gst);
    OS_UNUSED_ARG (first);
    OS_UNUSED_ARG (last);
    OS_UNUSED_ARG (type);
    it = os_malloc (sizeof (*it));
    *giter = (kv_iter_t) it;
    return 0;
}

static kv_store_t kv_open_dummy (struct d_storeKV_s *logst, const char *dir, const c_char *parameters)
{
    struct kv_store_dummy *st;

    OS_UNUSED_ARG (logst);
    OS_UNUSED_ARG (parameters);
    OS_UNUSED_ARG (dir);
    OS_UNUSED_ARG (parameters);

    st = os_malloc (sizeof (*st));
    st->c.write_access = 1;
    st->c.kind = KV_KIND_DUMMY;
    st->c.close = kv_close_dummy;
    st->c.commit = kv_commit_dummy;
    st->c.get = kv_get_dummy;
    st->c.put = kv_put_dummy;
    st->c.delete = kv_delete_dummy;
    st->c.bulkdelete = kv_bulkdelete_dummy;
    st->c.iter_new = kv_iter_new_dummy;
    st->c.iter_free = kv_iter_free_dummy;
    st->c.iter_next = kv_iter_next_dummy;
    st->c.iter_getkey = kv_iter_getkey_dummy;
    st->c.iter_getvalue = kv_iter_getvalue_dummy;
    st->c.choose_encoding = kv_choose_encoding_def;

    return (kv_store_t) st;
}
#endif /* KV_INCLUDE_DUMMY */

static kv_store_t kv_open (struct d_storeKV_s *logst, const char *dir, kv_kind_t kind, const c_char *parameters)
{
    switch (kind) {
        case KV_KIND_SQLITE:
#if KV_INCLUDE_SQLITE
            return kv_open_sqlite (logst, dir, parameters);
#else
            kvlog (logst, D_LEVEL_SEVERE, "kv: sqlite support not included\n");
            return NULL;
#endif

        case KV_KIND_LEVELDB:
#if KV_INCLUDE_LEVELDB
            return kv_open_leveldb (logst, dir, parameters);
#else
            kvlog (logst, D_LEVEL_SEVERE, "kv: leveldb support not included\n");
            return NULL;
#endif
        case KV_KIND_LSDS:
#if KV_INCLUDE_LSDS
            return kv_open_lsds (logst, dir, parameters);
#else
            kvlog (logst, D_LEVEL_SEVERE, "kv: lsds support not included\n");
            return NULL;
#endif
        case KV_KIND_DUMMY:
#if KV_INCLUDE_DUMMY
            return kv_open_dummy (logst, dir, parameters);
#else
            kvlog (logst, D_LEVEL_SEVERE, "kv: dummy support not included\n");
            return NULL;
#endif

        case KV_KIND_SQLITEMT:
#if KV_INCLUDE_SQLITE
            return kv_open_sqlitemt (logst, dir, parameters);
#else
            kvlog (logst, D_LEVEL_SEVERE, "kv: sqlitemt support not included\n");
            return NULL;
#endif
    }

    kvlog (logst, D_LEVEL_SEVERE, "kv: undefined kind requested\n");
    return NULL;
}

static kv_result_t kv_close (kv_store_t st)
{
    return st->close (st);
}

static kv_result_t kv_commit (kv_store_t st)
{
    return st->commit (st);
}

static kv_result_t kv_get (kv_store_t st, const kv_key_t *key, os_uint32 *sz, void **val, const struct c_type_s *type)
{
    return st->get (st, key, sz, val, type);
}

static kv_result_t kv_put (kv_store_t st, const kv_key_t *key, os_uint32 sz, const void *val, const struct c_type_s *type)
{
    if (st->write_access) {
        return st->put (st, key, sz, val, type);
    } else {
        return KV_RESULT_ACCESS_DENIED;
    }
}

static kv_result_t kv_delete (kv_store_t st, const kv_key_t *key)
{
    if (st->write_access) {
        return st->delete (st, key);
    } else {
        return KV_RESULT_ACCESS_DENIED;
    }
}

static kv_result_t kv_bulkdelete (kv_store_t st, const kv_key_t *first, const kv_key_t *last)
{
    if (st->write_access) {
        return st->bulkdelete (st, first, last);
    } else {
        return KV_RESULT_ACCESS_DENIED;
    }
}

static kv_result_t kv_iter_new (kv_store_t st, kv_iter_t *iter, const kv_key_t *first, const kv_key_t *last, const struct c_type_s *type)
{
    /* Iterates over all keys in one table. Not doing any filtering on
     * mid, but for consistency require that {first,last}->mid
     * correspond to the full range. */
    assert (first->table == last->table);
    assert (first->mid == 0);
    assert (last->mid == ~(os_uint64)0);
    return st->iter_new (st, iter, first, last, type);
}

static kv_result_t kv_iter_free (kv_store_t st, kv_iter_t iter)
{
    return st->iter_free (st, iter);
}

static kv_result_t kv_iter_next (int *hasdata, kv_store_t st, kv_iter_t iter)
{
    return st->iter_next (hasdata, st, iter);
}

static kv_result_t kv_iter_getkey (kv_store_t st, kv_iter_t iter, kv_key_t *key)
{
    return st->iter_getkey (st, iter, key);
}

static kv_result_t kv_iter_getvalue (kv_store_t st, kv_iter_t iter, os_uint32 *sz, void **val)
{
    return st->iter_getvalue (st, iter, sz, val);
}

static d_encodingKV kv_choose_encoding (kv_store_t st, d_encodingKV req)
{
    return st->choose_encoding (st, req);
}

/*****************************************************************************
 *
 *         EASY KEY VALUE STORE INTERFACE
 *
 *****************************************************************************/

static kv_result_t compress (d_storeKV st, os_uint32 src_sz, const void *src, os_uint32 *dest_sz, void **dest)
{
    void *blob = NULL;
    os_size_t blob_sz = 0;
    os_size_t data_sz;
    if (ut_compressorCompress(st->compressor, src, src_sz, &blob, &blob_sz, &data_sz) == UT_RESULT_OK) {
        *dest = blob;
        *dest_sz = (os_uint32)data_sz;
        assert(*dest_sz == data_sz);
        return 0;
    } else {
        return KV_RESULT_ERROR;
    }
}

static kv_result_t uncompress (d_storeKV st, os_uint32 src_sz, const void *src, os_uint32 *dest_sz, void **dest)
{
    void *blob = NULL;
    os_size_t blob_sz = 0;
    os_size_t data_sz;
    if (ut_compressorUncompress(st->compressor, src, src_sz, &blob, &blob_sz, &data_sz) == UT_RESULT_OK) {
        *dest = blob;
        *dest_sz = (os_uint32)data_sz;
        assert(*dest_sz == data_sz);
        return 0;
    } else {
        return KV_RESULT_ERROR;
    }
}

static void init_ser_deser_ctx(struct ser_deser_ctx *ctx)
{
    ctx->xmsgType = NULL;
    ctx->ci = NULL;
}

static void deinit_ser_deser_ctx(struct ser_deser_ctx *ctx)
{
    if (ctx->xmsgType) {
        v_messageExtTypeFree (ctx->xmsgType);
    }
    if (ctx->ci) {
        sd_cdrInfoFree(ctx->ci);
    }
}

static void *serialize_clearflag (os_uint32 *sz, d_storeKV st, const struct c_type_s *type, struct ser_deser_ctx *ctx, const void *obj, int clearflag)
    __nonnull((1,2,3,5));

static void *serialize_clearflag (os_uint32 *sz, d_storeKV st, const struct c_type_s *type, struct ser_deser_ctx *ctx, const void *obj, int clearflag)
{
    sd_serializedData serdata = NULL;
    void *blob = NULL;
    assert (type == c_getType ((c_object) obj));
    switch (st->encoding) {
    case D_ENCODINGKV_XML_XML: {
        sd_serializer ser = sd_serializerXMLNewTyped ((c_type) type);
        serdata = sd_serializerSerialize (ser, (c_object) obj);
        blob = sd_serializerToString (ser, serdata);
        *sz = (os_uint32) (strlen (blob) + 1);
        sd_serializerFree (ser);
        break;
    }
    case D_ENCODINGKV_BIGE_XML: {
        sd_serializer ser = sd_serializerBigENewTyped ((c_type) type);
        serdata = sd_serializerSerialize (ser, (c_object) obj);
        *sz = sd_serializedDataGetTotalSize (serdata);
        blob = os_malloc (*sz);
        memcpy (blob, serdata, *sz);
        sd_serializerFree (ser);
        break;
    }
    case D_ENCODINGKV_BIGECDR_XML:
    case D_ENCODINGKV_BIGECDR_V1:
        {
            struct sd_cdrSerdata *sd;
            const void *blob0;
            assert(ctx != NULL);
            if (!ctx->ci) {
               if ((ctx->ci = sd_cdrInfoNew (type)) == NULL) {
                   return NULL;
               }
               if (sd_cdrCompile (ctx->ci) != 0) {
                   return NULL;
               }
            }
            if (clearflag) {
                sd_cdrInfoClearPadding (ctx->ci);
            }
            if ((sd = sd_cdrSerializeBE (ctx->ci, obj)) == NULL) {
                return NULL;
            }
            *sz = sd_cdrSerdataBlob (&blob0, sd);
            blob = os_malloc (*sz);
            memcpy (blob, blob0, *sz);
            sd_cdrSerdataFree (sd);
        }
        break;
    }
    if (serdata) {
        sd_serializedDataFree (serdata);
    }

    if ((blob) && (*sz > 0) && (st->compression != D_COMPRESSION_NONE)) {
        kv_result_t result;
        os_uint32 tmp_sz = 0;
        void *tmp_blob = NULL;
        result = compress(st, *sz, blob, &tmp_sz, &tmp_blob);
        /* Check if compression does not mutilate data */
        if ((result == 0) &&
            (enable_kvdebug)) {
            os_uint32 dest_sz;
            void *dest;
            if (uncompress(st, tmp_sz, tmp_blob, &dest_sz, &dest) == 0) {
                if ((dest_sz != *sz) ||
                    ((memcmp(blob, dest, *sz) != 0))) {
                    kvlog (st, D_LEVEL_SEVERE, "Compression check failed\n");
                    assert(FALSE);
                }
                os_free(dest);
            }
        }
        os_free(blob);
        if (result == 0) {
            blob = tmp_blob;
            *sz = tmp_sz;
        } else {
            blob = NULL;
            *sz = 0;
        }
    }

    return blob;
}

static void *serialize (os_uint32 *sz, d_storeKV st, const struct c_type_s *type, struct ser_deser_ctx *ctx, const void *obj)
    __nonnull((1,2,3,5));

static void *serialize (os_uint32 *sz, d_storeKV st, const struct c_type_s *type, struct ser_deser_ctx *ctx, const void *obj)
{
    return serialize_clearflag (sz, st, type, ctx, obj, 0);
}

static kv_result_t deserialize (void **obj, d_storeKV st, const struct c_type_s *type, struct ser_deser_ctx *ctx, os_uint32 sz, const void *val)
    __nonnull((1,2,3,6));

static kv_result_t deserialize (void **obj, d_storeKV st, const struct c_type_s *type, struct ser_deser_ctx *ctx, os_uint32 sz, const void *val)
{
    sd_serializer ser = NULL;
    sd_serializedData serdata;
    kv_result_t rc = KV_RESULT_ERROR;
    const void *blob = val;
    void *tmp_blob = NULL;

    assert(blob);

    if ((sz > 0) && (st->compression != D_COMPRESSION_NONE)) {
        os_uint32 tmp_sz;
        if ((rc = uncompress(st, sz, blob, &tmp_sz, &tmp_blob)) >= 0) {
            blob = tmp_blob;
            sz = tmp_sz;
        } else {
            return rc;
        }
    }

    switch (st->encoding) {
    case D_ENCODINGKV_XML_XML:
        if (sz == 0 || ((char *) blob)[sz-1] != 0 || strlen (blob) + 1 != sz) {
            return KV_RESULT_CORRUPT;
        }
        ser = sd_serializerXMLNewTyped ((c_type) type);
        serdata = sd_serializerFromString (ser, blob);
        *obj = sd_serializerDeserialize (ser, serdata);
        sd_serializedDataFree (serdata);
        sd_serializerFree (ser);
        if (*obj == NULL) {
            rc = KV_RESULT_CORRUPT;
        } else {
            rc = 0;
        }
        break;
    case D_ENCODINGKV_BIGE_XML:
        ser = sd_serializerBigENewTyped ((c_type) type);
        *obj = sd_serializerDeserialize (ser, (sd_serializedData) blob);
        sd_serializerFree (ser);
        if (*obj == NULL) {
            rc = KV_RESULT_CORRUPT;
        } else {
            rc = 0;
        }
        break;
    case D_ENCODINGKV_BIGECDR_XML:
    case D_ENCODINGKV_BIGECDR_V1:
        {
            assert(ctx != NULL);
            if (!ctx->ci) {
                if ((ctx->ci = sd_cdrInfoNew (type)) == NULL) {
                     return KV_RESULT_ERROR;
                 }
                 if (sd_cdrCompile (ctx->ci) != 0) {
                     return KV_RESULT_ERROR;
                 }
            }
            if (sd_cdrDeserializeObjectBE (obj, ctx->ci, sz, blob) < 0) {
                rc = KV_RESULT_CORRUPT;
            } else {
                rc = 0;
            }
        }
        break;
    }
    os_free(tmp_blob);

    return rc;
}

static kv_result_t get_deser (d_storeKV st, const kv_key_t *key, const struct c_type_s *type, struct ser_deser_ctx *ctx, void **obj)
{
    kv_result_t rc;
    os_uint32 sz;
    void *val;
    if ((rc = kv_get (st->kv, key, &sz, &val, type)) >= 0) {
        rc = deserialize (obj, st, type, ctx, sz, val);
        os_free (val);
    }
    return rc;
}

static kv_result_t get1_deser (d_storeKV st, kv_table_t table, os_uint32 k0, const struct c_type_s *type, struct ser_deser_ctx *ctx, void **obj)
{
    kv_key_t key;
    key.table = table; key.gid = k0; key.iid = 0; key.mid = 0;
    return get_deser (st, &key, type, ctx, obj);
}

static kv_result_t get2_deser (d_storeKV st, kv_table_t table, os_uint32 k0, os_uint32 k1, const struct c_type_s *type, struct ser_deser_ctx *ctx, void **obj)
{
    kv_key_t key;
    key.table = table; key.gid = k0; key.iid = k1; key.mid = 0;
    return get_deser (st, &key, type, ctx, obj);
}

static kv_result_t iter_value_deser (d_storeKV st, kv_iter_t iter, const struct c_type_s *type, struct ser_deser_ctx *ctx, void **obj)
{
    kv_result_t rc;
    os_uint32 sz;
    void *val;
    if ((rc = kv_iter_getvalue (st->kv, iter, &sz, &val)) < 0) {
        return rc;
    } else {
        rc = deserialize (obj, st, type, ctx, sz, val);
        os_free (val);
        return rc;
    }
}

static kv_result_t put_ser (d_storeKV st, const kv_key_t *key, const struct c_type_s *type, struct ser_deser_ctx *ctx, const void *obj)
{
    void *blob;
    os_uint32 sz;
    kv_result_t rc;
    if ((blob = serialize (&sz, st, type, ctx, obj)) == NULL) {
        rc = KV_RESULT_OUT_OF_RESOURCES;
    } else {
        rc = kv_put (st->kv, key, sz, blob, type);
        os_free (blob);
    }
    return rc;
}

static kv_result_t put_ser_vmsg (d_storeKV st, const kv_key_t *key, v_topic vtopic, struct ser_deser_ctx *ctx, const struct v_message_s *vmsg)
{
    void *blob = NULL;
    c_type type = NULL;
    os_uint32 sz = 0;
    kv_result_t rc;
    switch (st->encoding) {
        case D_ENCODINGKV_XML_XML:
        case D_ENCODINGKV_BIGE_XML:
        case D_ENCODINGKV_BIGECDR_XML:
        {
            v_messageExt xmsg;
            if (ctx->xmsgType == NULL) {
                if ((ctx->xmsgType = v_messageExtTypeNew (vtopic)) == NULL) {
                    return KV_RESULT_OUT_OF_RESOURCES;
                }
            }
            if ((xmsg = v_messageExtCopyToExtType (ctx->xmsgType, vmsg)) == NULL) {
                return KV_RESULT_OUT_OF_RESOURCES;
            }
            type = ctx->xmsgType;
            if ((blob = serialize (&sz, st, type, ctx, xmsg)) == NULL) {
                c_free (xmsg);
                return KV_RESULT_OUT_OF_RESOURCES;
            }
        }
            break;
        case D_ENCODINGKV_BIGECDR_V1:
            type = c_getType ((v_message) vmsg);
            if ((blob = serialize (&sz, st, type, ctx, vmsg)) == NULL) {
                return KV_RESULT_OUT_OF_RESOURCES;
            }
            break;
    }
    rc = kv_put (st->kv, key, sz, blob, type);
    os_free (blob);
    return rc;
}

static kv_result_t put1_ser (d_storeKV st, kv_table_t table, os_uint32 k0, const struct c_type_s *type, struct ser_deser_ctx *ctx, const void *obj)
{
    kv_key_t key;
    key.table = table; key.gid = k0; key.iid = 0; key.mid = 0;
    return put_ser (st, &key, type, ctx, obj);
}

static kv_result_t put2_ser (d_storeKV st, kv_table_t table, os_uint32 k0, os_uint32 k1, const struct c_type_s *type, struct ser_deser_ctx *ctx, const void *obj)
{
    kv_key_t key;
    key.table = table; key.gid = k0; key.iid = k1; key.mid = 0;
    return put_ser (st, &key, type, ctx, obj);
}

/*****************************************************************************
 *
 *         TOPIC MANAGEMENT
 *
 * Note: independent treatment of namespaces and backup/restore
 * functionality can lead to inconsistent topic definitions in an
 * otherwise consistent persistent data store.
 *
 * FIXME: since we're not (yet) maintaining a topic administration in
 * memory that mirrors the one on disk, topic management is sort-of
 * absent.  Instead, we just scan the list of the groups, which is a
 * bit inefficient and silly.
 *
 *****************************************************************************/

static d_persistentTopicQosV0 d_pQosFromTopicInfo (d_storeKV st, const struct d_topicInfoKV *info)
{
    v_topicQos qos;
    if ((qos = v_topicQosFromTopicInfo (st->base, &info->info)) == NULL) {
        return NULL;
    } else {
        d_persistentTopicQosV0 pqos = d_pQosFromTopicQos (qos);
        c_free (qos);
        return pqos;
    }
}

static struct d_topicInfoKV *d_topicInfoFromPQos (d_storeKV st, const struct d_persistentTopicQosV0_s *pqos)
{
    v_topicQos qos;
    struct d_topicInfoKV *info;
    if ((qos = d_topicQosFromPQos (st->base, pqos)) == NULL) {
        return NULL;
    } else if ((info = c_new (st->topicinfo_type)) == NULL) {
        c_free (qos);
        return NULL;
    } else if (v_topicQosFillTopicInfo (&info->info, qos) != V_RESULT_OK) {
        c_free (info);
        c_free (qos);
        return NULL;
    } else {
        c_free (qos);
        return info;
    }
}

static struct d_topicInfoKV *make_topicInfoKV (d_storeKV st, v_topic topic)
{
    struct d_topicInfoKV *info;
    if ((info = c_new (st->topicinfo_type)) == NULL) {
        kvlog (st, D_LEVEL_SEVERE, "Store of metadata of topic %s failed: out of memory (1)\n", v_topicName (topic));
        return NULL;
    }
    if ((v_topicFillTopicInfo (&info->info, topic)) != V_RESULT_OK) {
        kvlog (st, D_LEVEL_SEVERE, "Store of metadata of topic %s failed: out of memory (2)\n", v_topicName (topic));
        return NULL;
    }

    /* We don't want to persist the key: it is of no value whatsoever,
     * and with it being absent from the old format it only creates
     * problems.  */
    info->info.key.systemId =
        info->info.key.localId =
        info->info.key.serial = 0;
    return info;
}

static int put_topicInfoKV (d_storeKV st, os_uint32 topic_id, const struct d_topicInfoKV *tp)
{
    switch (st->encoding) {
    case D_ENCODINGKV_XML_XML:
    case D_ENCODINGKV_BIGE_XML:
    case D_ENCODINGKV_BIGECDR_XML:
    {
        kv_key_t topic_key, tptype_key;
        void *topic_blob;
        os_uint32 topic_sz;
        struct d_topicKV *tp_old;
        int rc = -1;
        topic_key.table  = TABLE_TOPIC;      topic_key.gid  = topic_id; topic_key.iid  = 0; topic_key.mid  = 0;
        tptype_key.table = TABLE_TOPIC_TYPE; tptype_key.gid = topic_id; tptype_key.iid = 0; tptype_key.mid = 0;
        if ((tp_old = c_new (st->topic_type)) == NULL) {
            kvlog (st, D_LEVEL_SEVERE, "Storing topic %u (%s) failed: out of resources\n", topic_id, tp->info.name);
            goto err_newtopic;
        }
        tp_old->name = c_keep (tp->info.name);
        tp_old->type_name = c_keep (tp->info.type_name);
        tp_old->keylist = c_keep (tp->info.key_list);
        if ((tp_old->qos = d_pQosFromTopicInfo (st, tp)) == NULL) {
            kvlog (st, D_LEVEL_SEVERE, "Storing topic %u (%s) failed: out of resources (qos)\n", topic_id, tp->info.name);
            goto err_convqos;
        }
        if ((topic_blob = serialize (&topic_sz, st, st->topic_type, &st->topic_tctx, tp_old)) == NULL) {
            kvlog (st, D_LEVEL_SEVERE, "Storing topic %u (%s) failed: out of resources (topic)\n", topic_id, tp->info.name);
            goto err_sertopic;
        }
        if (kv_put (st->kv, &tptype_key, (os_uint32) strlen (tp->info.meta_data) + 1, tp->info.meta_data, c_string_t (st->base)) < 0) {
            kvlog (st, D_LEVEL_SEVERE, "Storing topic type %u (%s) failed\n", topic_id, tp->info.name);
            goto err_puttype;
        }
        if (kv_put (st->kv, &topic_key, topic_sz, topic_blob, st->topic_type) < 0) {
            kvlog (st, D_LEVEL_SEVERE, "Storing topic %u (%s) failed\n", topic_id, tp->info.name);
            goto err_puttopic;
        }
        rc = 0;
    err_puttopic:
    err_puttype:
        os_free (topic_blob);
    err_sertopic:
    err_convqos:
        c_free (tp_old);
    err_newtopic:
        return rc;
    }
    case D_ENCODINGKV_BIGECDR_V1:
    {
        kv_key_t topic_key;
        void *topic_blob;
        os_uint32 topic_sz;
        int rc;
        topic_key.table = TABLE_TOPIC; topic_key.gid = topic_id; topic_key.iid = 0; topic_key.mid = 0;
        if ((topic_blob = serialize (&topic_sz, st, st->topicinfo_type, &st->topicinfo_tctx, tp)) == NULL) {
            kvlog (st, D_LEVEL_SEVERE, "Storing topic %u (%s) failed: out of resources (topic)\n", topic_id, tp->info.name);
            return -1;
        }
        if ((rc = kv_put (st->kv, &topic_key, topic_sz, topic_blob, st->topicinfo_type)) < 0) {
            kvlog (st, D_LEVEL_SEVERE, "Storing topic %u (%s) failed\n", topic_id, tp->info.name);
        }
        os_free (topic_blob);
        return rc;
    }
    }
    assert (0);
    return -1;
}

static struct d_topicInfoKV *get_topicInfoKV (d_storeKV st, unsigned topic_id, const c_char *topic_name)
{
    switch (st->encoding) {
    case D_ENCODINGKV_XML_XML:
    case D_ENCODINGKV_BIGE_XML:
    case D_ENCODINGKV_BIGECDR_XML:
    {
        kv_key_t tptype_key;
        void *type_blob;
        os_uint32 type_sz;
        void *obj;
        struct d_topicKV *old;
        struct d_topicInfoKV *info;
        tptype_key.table = TABLE_TOPIC_TYPE; tptype_key.gid = topic_id; tptype_key.iid = 0; tptype_key.mid = 0;
        if (get1_deser (st, TABLE_TOPIC, topic_id, st->topic_type, &st->topic_tctx, &obj) < 0) {
            kvlog (st, D_LEVEL_SEVERE, "Retrieval of topic %u (%s) failed: no definition found\n", topic_id, topic_name);
            return NULL;
        }
        if (kv_get (st->kv, &tptype_key, &type_sz, &type_blob, c_string_t (st->base)) < 0) {
            kvlog (st, D_LEVEL_SEVERE, "Retrieval of topic type %u (%s) failed\n", topic_id, topic_name);
            c_free (obj);
            return NULL;
        }
        if (type_sz == 0 || ((char *) type_blob)[type_sz-1] != 0) {
            kvlog (st, D_LEVEL_SEVERE, "Retrieval of topic type %u (%s) failed: invalid string\n", topic_id, topic_name);
            os_free (type_blob);
            c_free (obj);
            return NULL;
        }
        old = obj;
        if ((info = d_topicInfoFromPQos (st, old->qos)) == NULL) {
            kvlog (st, D_LEVEL_SEVERE, "Retrieval of topic %u (%s) failed: out of memory\n", topic_id, topic_name);
            os_free (type_blob);
            c_free (obj);
            return NULL;
        }
        info->info.name = c_keep (old->name);
        info->info.type_name = c_keep (old->type_name);
        info->info.key_list = c_keep (old->keylist);
        info->info.meta_data = c_stringNew (st->base, type_blob);
        os_free (type_blob);
        c_free (old);
        return info;
    }
    case D_ENCODINGKV_BIGECDR_V1:
    {
        void *obj;
        if (get1_deser (st, TABLE_TOPIC, topic_id, st->topicinfo_type, &st->topicinfo_tctx, &obj) < 0) {
            kvlog (st, D_LEVEL_SEVERE, "Retrieval of topic %u (%s) failed: no definition found\n", topic_id, topic_name);
            return NULL;
        }
        return obj;
    }
    }
    assert (0);
    return NULL;
}

/*****************************************************************************
 *
 *         NAMESPACES
 *
 *****************************************************************************/

struct get_namespace_helper_arg {
    const char *partition;
    const char *topic;
    os_uint32 rank;
    d_nameSpace namespace;
    int found;
};

static struct namespace *find_namespace (const struct d_storeKV_s *st, const d_nameSpace nameSpace)
{
    /* Namespaces sorted s.t. (id,v) precedes (id,w) if v>w.
     * Therefore, returns latest version of name space. */
    const char *nsname = d_nameSpaceGetName (nameSpace);
    struct namespace *ns;
    for (ns = st->namespaces; ns; ns = ns->next) {
        if (strcmp (ns->name, nsname) == 0) {
            return ns;
        }
    }
    return NULL;
}

static struct namespace *find_namespace_by_id (const struct d_storeKV_s *st, os_uint32 nsid)
{
    /* Namespaces sorted s.t. (id,v) precedes (id,w) if v>w.
     * Therefore, returns latest version of name space. */
    struct namespace *ns;
    for (ns = st->namespaces; ns; ns = ns->next) {
        if (ns->id == nsid) {
            return ns;
        }
    }
    return NULL;
}

static struct namespace *find_namespace_by_id_version (const struct d_storeKV_s *st, os_uint32 nsid, os_uint32 nsversion)
{
    struct namespace *ns;
    for (ns = st->namespaces; ns; ns = ns->next) {
        if (ns->id == nsid && ns->version == nsversion) {
            return ns;
        }
    }
    return NULL;
}

#ifndef NDEBUG
static int namespace_id_is_free (const struct d_storeKV_s *st, os_uint32 nsid)
{
    struct namespace *ns;
    for (ns = st->namespaces; ns; ns = ns->next) {
        if (ns->id == nsid) {
            return 0;
        }
    }
    return 1;
}
#endif

static struct namespace *new_namespace_common (struct d_storeKV_s *st, os_uint32 nsid, os_uint32 version, const char *name, const os_timeW *quality, int on_disk)
{
    struct namespace *ns;
    assert (namespace_id_is_free (st, nsid) || version > find_namespace_by_id (st, nsid)->version);
    ns = os_malloc (sizeof (*ns));
    ns->id = nsid;
    ns->version = version;
    ns->name = os_strdup (name);
    if (quality) {
        ns->quality = *quality;
    } else {
        ns->quality = OS_TIMEW_ZERO;
    }
    ns->on_disk = on_disk;
    /* Insert at head maintains sort order */
    ns->next = st->namespaces;
    st->namespaces = ns;
    if (nsid >= st->next_namespace_id) {
        st->next_namespace_id = nsid + 1;
    }
    return ns;
}

static struct namespace *new_namespace (struct d_storeKV_s *st, os_uint32 nsid, const char *name, const os_timeW *quality, int on_disk)
{
    return new_namespace_common (st, nsid, 0, name, quality, on_disk);
}

static struct namespace *new_namespace_version (struct d_storeKV_s *st, const struct namespace *old_version)
{
    return new_namespace_common (st, old_version->id, old_version->version + 1, old_version->name, NULL, 0);
}

static struct namespace *find_or_create_namespace (struct d_storeKV_s *st, const d_nameSpace nameSpace)
{
    struct namespace *ns;
    if ((ns = find_namespace (st, nameSpace)) == NULL) {
        ns = new_namespace (st, st->next_namespace_id, d_nameSpaceGetName (nameSpace), NULL, 0);
    }
    return ns;
}

static d_storeResult store_namespace_quality (d_storeKV st, struct namespace *ns)
{
    struct d_namespaceQualityKV *q;
    d_storeResult result;
    kv_result_t rc;

    kvdebug ("Namespace %s id %u/%u quality %"PA_PRItime" updating-disk\n",
             ns->name, ns->id, ns->version, OS_TIMEW_PRINT(ns->quality));
    q = c_new (st->namespace_quality_type);
    /* Map quality to c_time, store quality as extended time (64-bit) */
    d_qualityExtFromQuality(&q->quality, &ns->quality, TRUE);
    if ((rc = put2_ser (st, TABLE_NAMESPACE_QUALITY, ns->id, ns->version, st->namespace_quality_type, &st->namespace_quality_tctx, q)) < 0) {
        result = kv_convert_result(rc);
    } else {
        result = D_STORE_RESULT_OK;
    }
    c_free (q);
    return result;
}


static d_storeResult set_namespace_quality (d_storeKV st, struct namespace *ns, const os_timeW *quality)
{
    if (quality_le (quality, &ns->quality)) {
        return D_STORE_RESULT_OK;
    }

    ns->quality = *quality;
    if (ns->on_disk) {
        return store_namespace_quality (st, ns);
    } else {
        kvdebug ("Namespace %s id %u/%u quality %"PA_PRItime" not-updating-disk\n",
                 ns->name, ns->id, ns->version, OS_TIMEW_PRINT(*quality));
        return D_STORE_RESULT_OK;
    }
}

static d_storeResult set_namespace_quality_deferred (d_storeKV st, struct namespace *ns, const os_timeW *quality)
{
    if (quality_le (quality, &ns->quality)) {
        return D_STORE_RESULT_OK;
    }

    ns->quality = *quality;
    if (ns->on_disk) {
        hashtable_put(st->updated_namespaces, ns);
    } else {
        kvdebug ("Namespace %s id %u/%u quality %"PA_PRItime" not-updating-disk\n", ns->name, ns->id, ns->version, OS_TIMEW_PRINT(*quality));
    }

    return D_STORE_RESULT_OK;
}


static d_storeResult store_namespace (struct d_storeKV_s *st, struct namespace *ns)
{
    d_storeResult result;
    kv_result_t rc;

    if (ns->on_disk) {
        kvlog (st, D_LEVEL_FINEST, "Namespace %s id %u/%u already on disk\n", ns->name, ns->id, ns->version);
        result = D_STORE_RESULT_OK;
    } else {
        struct d_namespaceKV *d;
        d = c_new (st->namespace_type);
        d->name = c_stringNew (c_getBase (d), ns->name);
        if ((rc = put2_ser (st, TABLE_NAMESPACE, ns->id, ns->version, st->namespace_type, &st->namespace_tctx, d)) < 0) {
            result = kv_convert_result(rc);
            kvlog (st, D_LEVEL_SEVERE, "Namespace %s id %u/%u could not be written to disk\n", ns->name, ns->id, ns->version);
        } else {
            result = D_STORE_RESULT_OK;
            kvlog (st, D_LEVEL_FINEST, "Namespace %s id %u/%u written to disk\n", ns->name, ns->id, ns->version);
            ns->on_disk = 1;
        }
        c_free (d);
    }
    return result;
}

static void free_namespace (struct namespace *ns)
{
    if (ns) {
        os_free (ns->name);
        os_free (ns);
    }
}

static os_uint32 get_namespace_key (const struct namespace *ns)
{
    return ns->id;
}

static int compare_namespace_by_name (const struct namespace *ns1, const struct namespace *ns2)
{
    assert(ns1);
    assert(ns2);

    return strcmp(ns1->name, ns2->name) == 0;
}

static int compare_namespace_by_id (const struct namespace *ns1, const struct namespace *ns2)
{
    assert(ns1);
    assert(ns2);

    return ns1->id == ns2->id;
}


/*****************************************************************************
 *
 *         LOCAL GROUP MANAGEMENT
 *
 * Note: see comment at TOPIC MANAGEMENT.  Furthermore, the use of a
 * mostly-unsorted linked list is a bit silly.  It would be much
 * better to use a hash table, or even an balanced binary tree.
 *
 *****************************************************************************/
static os_uint32 get_partition_topic_key (const char *partition, const char *topic)
{
    os_uint32 result = 5381;
    const char *ptr;
    unsigned c;

    ptr = partition;
    while((c = (unsigned char) *ptr++)) {
        result = ((result << 5) + result) ^ c;
    }

    ptr = topic;
    while((c = (unsigned char) *ptr++)) {
         result = ((result << 5) + result) ^ c;
     }

    return result;
}

static os_uint32 get_group_key (const struct d_groupListKV_s *g)
{
    assert (g);
    return get_partition_topic_key(g->_parent.partition, g->_parent.topic);
}


static int compare_group (const struct d_groupListKV_s *g1, const struct d_groupListKV_s *g2)
{
    assert(g1);
    assert(g2);

    return (strcmp (g1->_parent.topic, g2->_parent.topic) == 0 && strcmp (g1->_parent.partition, g2->_parent.partition) == 0);
}

struct find_group_helper_arg {
    const char *partition;
    const char *topic;
};

static int find_group_helper (const struct d_groupListKV_s *g, struct find_group_helper_arg *arg)
{
    assert(g);
    assert(arg);

    return (strcmp (g->_parent.topic, arg->topic) == 0 && strcmp (g->_parent.partition, arg->partition) == 0);
}

static int group_latest_namespace_version (const struct d_storeKV_s *st, const struct d_groupListKV_s *g)
{
    const struct namespace *ns;
    for (ns = st->namespaces; ns; ns = ns->next) {
        if (ns->id == g->namespace->id) {
            break;
        }
    }
    assert (ns != NULL);
    return ns == g->namespace;
}

static d_groupListKV find_group (const struct d_storeKV_s *st, const char *partition, const char *topic)
{
    const struct d_groupListKV_s *g;
    os_uint32 key;
    struct find_group_helper_arg arg;

    key = get_partition_topic_key(partition, topic);
    arg.partition = partition;
    arg.topic = topic;

    if ((g = hashtable_find(st->groupTable, key, (hashtable_cmp_func_t)find_group_helper, &arg)) != NULL) {
        return (d_groupListKV) (group_latest_namespace_version (st, g) ? g : NULL);
    }

    return NULL;
}

static d_groupListKV find_group_w_topic (const struct d_storeKV_s *st, const char *topic)
{
    /* FIXME: Should do a proper in-memory topic name -> id mapping that
     * is kept consistent with the data on disk. */
    const struct d_groupListKV_s *g;
    for (g = st->groups; g != NULL; g = g->_parent.next) {
        if (strcmp (g->_parent.topic, topic) == 0) {
            return (d_groupListKV) (group_latest_namespace_version (st, g) ? g : NULL);
        }
    }
    return NULL;
}

static d_groupListKV find_group_w_kernelgroup (const struct d_storeKV_s *st, const struct v_group_s *group)
{
    /* FIXME: Should do a proper in-memory topic name -> id mapping that
     * is kept consistent with the data on disk. */
    return find_group (st, v_partitionName (v_groupPartition ((c_object)group)), v_topicName (v_groupTopic ((c_object)group)));
}

static d_storeResult new_group (d_groupListKV *pg, d_storeKV st, os_uint32 group_id, struct namespace *ns, const char *partition, os_uint32 topic_id, const char *topic, const os_timeW *quality)
{
    d_storeResult result;
    d_groupListKV g;
    struct d_groupListKV_s *PresentGroupInList = NULL;
    os_uint32 key;
    struct find_group_helper_arg arg;
    assert (find_group (st, partition, topic) == NULL);
    if ((g = os_malloc (sizeof (*g))) == NULL) {
        result = D_STORE_RESULT_OUT_OF_RESOURCES;
    } else if ((result = set_namespace_quality (st, ns, quality)) != D_STORE_RESULT_OK) {
        os_free (g);
    } else {
        g->_parent.partition = os_strdup (partition);
        g->_parent.topic = os_strdup (topic);
        g->_parent.quality = *quality;
        g->_parent.completeness = D_GROUP_INCOMPLETE;
        g->_parent.optimized = FALSE;
        g->_parent.next = st->groups;
        g->group_id = group_id;
        g->topic_id = topic_id;
        g->instances = d_tableNew(inst_cmp, inst_free);
        ut_avlInit (&group_inst_td, &g->instances_by_iid);
        g->namespace = ns;
        g->vgroup = NULL;
        init_ser_deser_ctx (&g->tctx);
        g->tctx.ci = NULL;
        st->groups = g;
        *pg = g;

        key = get_partition_topic_key(partition, topic);
        arg.partition = partition;
        arg.topic = topic;
        PresentGroupInList = hashtable_find(st->groupTable, key, (hashtable_cmp_func_t)find_group_helper, &arg);
        if (PresentGroupInList != NULL) {
            /* check version of namespace from same group */
            if (PresentGroupInList->namespace->version < ns->version) {
                /* old group present with lower version nr in hash table, needs to be overwritten by newer one, delete old one */
                (void)hashtable_del(st->groupTable, PresentGroupInList);
            }
        }
        /* always put new group, if same already present this one will be ignored */
        hashtable_put(st->groupTable, g);
    }
    return result;
}

static void free_group (struct d_groupListKV_s *g, d_storeKV st)
{
    void* pvTest = NULL;
    /* be sure freed group is not in hash table anymore */
    pvTest = hashtable_del(st->groupTable, g);
    if (pvTest != NULL) {
        /* this group was also in hash table, find lower version group if present to insert in hash table (rollback?) */
        struct d_groupListKV_s *fg = st->groups;
        struct d_groupListKV_s *pstNewGroupToInsert = NULL;
        while (fg) {
            if ((fg != g) && (strcmp(fg->_parent.topic, g->_parent.topic) == 0) && (strcmp(fg->_parent.partition, g->_parent.partition) == 0)) {
                if (pstNewGroupToInsert == NULL) {
                    pstNewGroupToInsert = fg;
                }
                else if (fg->namespace->version > pstNewGroupToInsert->namespace->version) {
                    pstNewGroupToInsert = fg;
                }
            }
            fg = fg->_parent.next;
        }
        if (pstNewGroupToInsert != NULL) {
            hashtable_put(st->groupTable, pstNewGroupToInsert);
        }
    }
    d_tableFree(g->instances);
    deinit_ser_deser_ctx (&g->tctx);
    os_free (g->_parent.partition);
    os_free (g->_parent.topic);
    os_free (g);
}

/*****************************************************************************
 *
 *         LOCAL INSTANCE MANAGEMENT
 *
 *****************************************************************************/

static d_storeResult get_blobkey (size_t *psz, void **pval, const struct v_group_s *group, const struct v_message_s *msg)
{
    c_size sz = 0;
    c_ulong i, nkeys;
    c_array keylist;
    char *ptr;

    assert (psz && pval && group && msg);

    keylist = v_topicMessageKeyList (v_groupTopic ((c_object)group));
    (void)c_getType ((v_message) msg);
    nkeys = c_arraySize (keylist);

    /* First determine size of key blob. We don't care what the
     * ordering looks like to a human, only that it is a total
     * ordering of instances.  Hence we can get away by simply
     * concatenating the binary representation of all fields.  Strings
     * will vary in length, so fields won't be in the same place all
     * the time.  FIXME: is this really safe, or should I do the
     * scalars first?
     *
     * Allocate at least 1 byte to be certain that we don't get a null
     * pointer because we want to pass them straight into memcmp,
     * which has undefined behaviour when passed a null pointer even
     * when the number of bytes to compare is 0. */
    for (i = 0; i < nkeys; i++) {
        sz += c_fieldBlobSize (keylist[i], (c_object) msg);
    }
    *psz = (size_t) sz;
    if ((*pval = os_malloc (sz == 0 ? 1 : sz)) == NULL) {
        return D_STORE_RESULT_OUT_OF_RESOURCES;
    }
    ptr = *pval;
    for (i = 0; i < nkeys; i++) {
        ptr += c_fieldBlobCopy (keylist[i], (c_object) msg, ptr);
    }
    return D_STORE_RESULT_OK;
}


static int transaction_list_cmp (const struct transaction_list *a, const struct transaction_list *b)
{
    c_equality eq;

    /* comparision of transaction lists is based on writerGID and transactionId */
    eq = v_gidCompare (a->gid, b->gid);
    if (eq == C_LT) {
        return -1;
    } else if (eq == C_GT) {
        return 1;
    } else if (a->seqNum < b->seqNum) {
       return -1;
    } else if (a->seqNum > b->seqNum) {
       return 1;
    }
    return 0;
}


static int inst_cmp (const struct inst *a, const struct inst *b)
{
    /* All we need is a total ordering of instances, and the only time
     * we need to lookup an inst is when we get a v_message and need
     * the corresponding inst.  Hence the instance compare function
     * is based on the real key rather than the (unique) instance_id.
     * The key is treated as a blob, on which we do memcmp. */
    size_t sz = (a->keysize < b->keysize) ? a->keysize : b->keysize;
    int eq = memcmp (a->keyvalue, b->keyvalue, sz);
    if (eq < 0) {
        return -1;
    } else if (eq > 0) {
        return 1;
    } else if (a->keysize < b->keysize) {
        return -1;
    } else if (a->keysize > b->keysize) {
        return 1;
    } else {
        return 0;
    }
}

static int compare_iid (const void *va, const void *vb)
{
    const os_uint32 *a = va;
    const os_uint32 *b = vb;
    return (*a == *b) ? 0 : (*a < *b) ? -1 : 1;
}

static void inst_augment (void *vinst, const void *vleft, const void *vright)
{
    struct inst *inst = vinst;
    const struct inst *left = vleft;
    const struct inst *right = vright;
    const os_uint32 iid = inst->next_msg_key.iid;
    os_uint32 holep1;

    if (left && left->iid_holep1) {
        holep1 = left->iid_holep1;
    } else if (left && iid > left->iid_max + 1) {
        holep1 = iid;
    } else if (right && iid + 1 < right->iid_min) {
        holep1 = right->iid_min;
    } else if (right && right->iid_holep1) {
        holep1 = right->iid_holep1;
    } else {
        holep1 = 0;
    }

    inst->iid_holep1 = holep1;
    inst->iid_min = left ? left->iid_min : iid;
    inst->iid_max = right ? right->iid_max : iid;
}

static int new_instance_id (struct d_storeKV_s *st, os_uint32 *iid, const struct d_groupListKV_s *g, const os_uint32 *p_iid)
{
    int ok;
    if (p_iid) {
        if (ut_avlLookup (&group_inst_td, &g->instances_by_iid, p_iid) == NULL) {
            *iid = *p_iid;
            ok = 1;
        } else {
            kvlog (st, D_LEVEL_SEVERE, "new_instance_id: required id %u already in use\n", *p_iid);
            ok = 0;
        }
    } else if (ut_avlIsEmpty (&g->instances_by_iid)) {
        *iid = 0;
        ok = 1;
    } else {
        struct inst *r = ut_avlRoot (&group_inst_td, &g->instances_by_iid);
        if (r->iid_min) {
            *iid = r->iid_min - 1;
            ok = 1;
        } else if (r->iid_holep1) {
            *iid = r->iid_holep1 - 1;
            ok = 1;
        } else if (r->iid_max != 0xffffffff) {
            *iid = r->iid_max + 1;
            ok = 1;
        } else {
            kvlog (st, D_LEVEL_SEVERE, "new_instance_id: no available instance id\n");
            ok = 0;
        }
    }
    assert (!ok || ut_avlLookup (&group_inst_td, &g->instances_by_iid, iid) == NULL);
    return ok;
}


static struct inst *new_instance (struct d_storeKV_s *st, struct d_groupListKV_s *g, size_t keysize, void *keyvalue, const os_uint32 *p_iid)
{
    struct inst *inst;

    inst = os_malloc (sizeof (*inst));
    inst->next_msg_key.table = TABLE_MESSAGE;
    inst->next_msg_key.gid = g->group_id;
    if (!new_instance_id (st, &inst->next_msg_key.iid, g, p_iid)) {
        os_free (inst);
        return NULL;
    }
    inst->next_msg_key.mid = 0;

    inst->keysize = keysize;
    inst->keyvalue = keyvalue;

    inst->g = g;

    inst->writeCount = 0;
    inst->oldest = NULL;
    inst->latest = NULL;
    inst->oldest_non_transaction = NULL;

    return inst;
}

static d_storeResult find_instance (struct inst **inst, struct d_groupListKV_s *g, const struct v_group_s *group, const struct v_message_s *msg)
{
    d_storeResult result;
    size_t sz;
    void *val;
    if ((result = get_blobkey (&sz, &val, group, msg)) == D_STORE_RESULT_OK) {
        struct inst tmp;
        tmp.keysize = sz;
        tmp.keyvalue = val;
        *inst = d_tableFind (g->instances, &tmp);
        os_free (val);
    }
    return result;
}

static d_storeResult find_or_create_instance (struct inst **inst, struct d_storeKV_s *st, struct d_groupListKV_s *g, const struct v_group_s *group, const struct v_message_s *msg, const os_uint32 *p_iid)
{
    d_storeResult result;
    size_t sz;
    void *val;
    if ((result = get_blobkey (&sz, &val, group, msg)) == D_STORE_RESULT_OK) {
        struct inst tmp;
        tmp.keysize = sz;
        tmp.keyvalue = val;
        if ((*inst = d_tableFind (g->instances, &tmp)) != NULL) {
            assert (p_iid == NULL || (*inst)->next_msg_key.iid == *p_iid);
            assert (ut_avlLookup (&group_inst_td, &g->instances_by_iid, &(*inst)->next_msg_key.iid) == *inst);
            os_free (val);
        } else if ((*inst = new_instance (st, g, sz, val, p_iid)) == NULL) {
            os_free (val);
            result = D_STORE_RESULT_OUT_OF_RESOURCES;
        } else if (d_tableInsert (g->instances, *inst) != NULL) {
            kvlog (st, D_LEVEL_SEVERE, "find_or_create_instance: inconsistent instance table\n");
            inst_free (*inst);
            result = D_STORE_RESULT_MUTILATED;
        } else {
            assert (ut_avlLookup (&group_inst_td, &g->instances_by_iid, &(*inst)->next_msg_key.iid) == NULL);
            ut_avlInsert (&group_inst_td, &g->instances_by_iid, *inst);
        }
    }
    return result;
}


static void msg_free (struct msg *m)
{
    if (m->v_msg) {
        c_free (m->v_msg);
    }
    os_free (m);
}

static void inst_free (struct inst *a)
{
    while (a->oldest) {
        struct msg *m = a->oldest;
        a->oldest = m->newer;
        msg_free (m);
    }
    os_free (a->keyvalue);
    os_free (a);
}


static void transaction_list_free (struct transaction_list *a) {
    struct transaction_msg *tr_msg;

    tr_msg = a->head;
    while (tr_msg) {
        struct transaction_msg *tr_m = tr_msg;
        tr_msg = tr_msg->next;
        os_free(tr_m);
    }
    os_free(a);
}

static d_storeResult init_namespaces (d_storeKV st)
{
    /* Note: iteration of keys is in ascending order, therefore newer
     * versions of a namespace are inserted later than older versions,
     * which matches the assumption in new_namespace() */
    d_storeResult result = D_STORE_RESULT_OK;
    struct d_namespaceQualityKV *default_nsq;
    kv_key_t min, max;
    kv_iter_t iter;
    int hasdata;
    os_timeW quality;

    kvlog (st, D_LEVEL_FINE, "Loading namespaces ...\n");
    assert (st->groups == NULL);

    min.table = TABLE_NAMESPACE;
    min.gid = 0;
    min.iid = 0;
    min.mid = 0;
    max.table = TABLE_NAMESPACE;
    max.gid = MAX_GID;
    max.iid = ~(os_uint32)0;
    max.mid = ~(os_uint64)0;

    default_nsq = c_new (st->namespace_quality_type);
    default_nsq->quality.seconds = 0;
    default_nsq->quality.nanoseconds = 0;
    kv_iter_new (st->kv, &iter, &min, &max, st->namespace_type);
    while (result == D_STORE_RESULT_OK && kv_iter_next (&hasdata, st->kv, iter) >= 0 && hasdata) {
        kv_key_t k;
        void *obj;
        kv_iter_getkey (st->kv, iter, &k);
        if (iter_value_deser (st, iter, st->namespace_type, &st->namespace_tctx, &obj) < 0) {
            kvlog (st, D_LEVEL_SEVERE, "namespace {%u:%u:%llu:%llu}: stored data is corrupt\n",
                   (unsigned) k.table, k.gid, k.iid, k.mid);
            result = D_STORE_RESULT_MUTILATED;
        } else {
            const struct d_namespaceKV *nskv = obj;
            struct d_namespaceQualityKV *nsq;
            struct namespace *ns;
            void *tmp;
            if (get2_deser (st, TABLE_NAMESPACE_QUALITY, k.gid, k.iid, st->namespace_quality_type, &st->namespace_quality_tctx, &tmp) >= 0) {
                nsq = tmp;
            } else {
                nsq = c_keep (default_nsq);
            }
            /* Map qualityExt to quality, use extended time (64-bit) */
            d_qualityExtToQuality(&quality, &nsq->quality, TRUE);
            if ((ns = new_namespace_common (st, k.gid, k.iid, nskv->name, &quality, 1)) == NULL) {
                kvlog (st, D_LEVEL_SEVERE, "namespace %s id %u/%u: out of resources\n", nskv->name, k.gid, k.iid);
            } else {
                kvlog (st, D_LEVEL_FINE, "  namespace %s id %u/%u quality %"PA_PRItime"\n", nskv->name, k.gid, k.iid, OS_TIMEW_PRINT(quality));
            }
            c_free (nsq);
            c_free (obj);
        }
    }
    kv_iter_free (st->kv, iter);
    c_free (default_nsq);
    return result;
}

static d_storeResult init_groups (d_storeKV st)
{
    d_storeResult result = D_STORE_RESULT_OK;
    kv_key_t min, max;
    kv_iter_t iter;
    int hasdata;

    kvlog (st, D_LEVEL_FINE, "Loading groups ...\n");
    assert (st->groups == NULL);

    min.table = TABLE_GROUP;
    min.gid = 0;
    min.iid = 0;
    min.mid = 0;
    max.table = TABLE_GROUP;
    max.gid = MAX_GID;
    max.iid = ~(os_uint32)0;
    max.mid = ~(os_uint64)0;

    kv_iter_new (st->kv, &iter, &min, &max, st->group_type);
    while (result == D_STORE_RESULT_OK && kv_iter_next (&hasdata, st->kv, iter) >= 0 && hasdata) {
        kv_key_t k;
        void *obj;
        kv_iter_getkey (st->kv, iter, &k);
        if (iter_value_deser (st, iter, st->group_type, &st->group_tctx, &obj) < 0) {
            kvlog (st, D_LEVEL_SEVERE, "group {%u:%u:%llu:%llu}: stored data is corrupt\n", (unsigned) k.table, k.gid, k.iid, k.mid);
            result = D_STORE_RESULT_MUTILATED;
        } else {
            const struct d_groupKV *gd = obj;
            struct namespace *ns;
            d_groupListKV gg;
            if ((ns = find_namespace_by_id_version (st, gd->namespace_id, gd->namespace_version)) == NULL) {
                kvlog (st, D_LEVEL_WARNING, "group %s.%s (%u): namespace id %u missing, assuming partial delete of namespace\n", gd->partition, gd->topic, k.gid, gd->namespace_id);
                /* Normally, next_namespace_id is updated by
                 * new_namespace and groups refer to existing
                 * namespaces.  However, now it is referring to a
                 * non-existent one, and that may be one with a higher
                 * id than we have seen so far */
                if (gd->namespace_id >= st->next_namespace_id) {
                    st->next_namespace_id = gd->namespace_id + 1;
                }
            } else if ((result = new_group (&gg, st, k.gid, ns, gd->partition, gd->topic_id, gd->topic, &ns->quality)) != D_STORE_RESULT_OK) {
                kvlog (st, D_LEVEL_SEVERE, "group %s.%s (%u): out of resources or worse\n", gd->partition, gd->topic, k.gid);
            } else {
                kvlog (st, D_LEVEL_FINE, "  group %s.%s (%u) namespace %s id %u/%u quality %"PA_PRItime"\n", gd->partition, gd->topic, k.gid, ns->name, ns->id, ns->version, OS_TIMEW_PRINT(ns->quality));

                /* Reading keys in ascending order, so group id must
                 * be >= next_group_id */
                assert (k.gid >= st->next_group_id);
                st->next_group_id = k.gid + 1;

                /* Introduction of a new group is the only way a new
                 * topic can be introduced, but an old topic can
                 * always be referenced by a new group.  Therefore,
                 * only conditionally update next_topic_id. */
                if (gd->topic_id >= st->next_topic_id) {
                    st->next_topic_id = gd->topic_id + 1;
                }
            }
            c_free (obj);
        }
    }
    kv_iter_free (st->kv, iter);
    return result;
}

d_storeKV d_storeNewKV (u_participant participant)
{
    d_storeKV st;
    struct baseFind f;

    {
        char *p;
        if ((p = os_getenv ("OSPL_KVDEBUG")) != NULL && atoi (p) != 0) {
            enable_kvdebug = 1;
        }
    }

    {
        char *p;
        if ((p = os_getenv ("OSPL_DURABILITY_LOG_STATISTICS")) != NULL && atoi (p) != 0) {
            kvlog_statistics = 1;
        }
    }

    st = os_malloc (C_SIZEOF (d_storeKV));
    if (os_mutexInit (&st->lock, NULL) != os_resultSuccess) {
        kvlog (st, D_LEVEL_SEVERE, "storeNewKV: mutexInit failed\n");
        goto err_storeNewKV_early;
    }
    if (os_condInit (&st->cond, &st->lock, NULL) != os_resultSuccess) {
        kvlog (st, D_LEVEL_SEVERE, "storeNewKV: condInit failed\n");
        os_mutexDestroy (&st->lock);
        goto err_storeNewKV_early;
    }

    d_storeInit ((d_store) st, (d_objectDeinitFunc)d_storeDeinitKV);
    (void)u_observableAction (u_observable(participant), d_storeGetBase, &f);
    st->base = f.base;
    st->opened = 0;
    st->groups = NULL;
    st->namespaces = NULL;
    st->kv = NULL;
    st->diskStorePath = NULL;
    st->updated_namespaces = NULL;
    st->groupTable = NULL;
    st->locked = 0;
    st->compression = D_COMPRESSION_NONE;
    st->compressor = NULL;
    init_ser_deser_ctx(&st->topic_tctx);
    init_ser_deser_ctx(&st->topicinfo_tctx);
    init_ser_deser_ctx(&st->namespace_tctx);
    init_ser_deser_ctx(&st->namespace_quality_tctx);
    init_ser_deser_ctx(&st->namespace_completeness_tctx);
    init_ser_deser_ctx(&st->group_tctx);
    init_ser_deser_ctx(&st->open_transaction_eot_tctx);
    init_ser_deser_ctx(&st->open_transaction_vtid_tctx);

    st->next_group_id = 0;
    st->next_topic_id = 0;
    st->next_namespace_id = 0;

    st->otid = 0;    /* The first free open transaction id */

    if ((st->updated_namespaces = hashtable_new ((hashtable_key_func_t)get_namespace_key, (hashtable_cmp_func_t)compare_namespace_by_id)) == NULL) {
        kvlog (st, D_LEVEL_SEVERE, "d_storeNewKV: allocation failed.\n");
        goto err_storeNewKV;

    }

    if ((st->groupTable = hashtable_new ((hashtable_key_func_t)get_group_key, (hashtable_cmp_func_t)compare_group)) == NULL) {
        kvlog (st, D_LEVEL_SEVERE, "d_storeNewKV: allocation failed.\n");
        goto err_storeNewKV;
    }

    if ((st->open_transactions = d_tableNew(transaction_list_cmp, transaction_list_free)) == NULL) {
        kvlog (st, D_LEVEL_SEVERE, "d_storeNewKV: allocation failed.\n");
        goto err_storeNewKV;
    }


    st->topic_type = c_resolve (f.base, "durabilityModule2::d_topicKV");
    st->topicinfo_type = c_resolve (f.base, "durabilityModule2::d_topicInfoKV");
    st->namespace_type = c_resolve (f.base, "durabilityModule2::d_namespaceKV");
    st->namespace_quality_type = c_resolve (f.base, "durabilityModule2::d_namespaceQualityKV");
    st->namespace_completeness_type = c_resolve (f.base, "durabilityModule2::d_namespaceCompletenessKV");
    st->group_type = c_resolve (f.base, "durabilityModule2::d_groupKV");
    st->open_transaction_eot_type = c_resolve (f.base, "kernelModuleI::v_messageEOT");
    st->open_transaction_vtid_type = c_resolve (f.base, "kernelModule::v_tid");

    {
        d_store s = &st->_parent;
        s->openFunc                     = d_storeOpenKV;
        s->closeFunc                    = d_storeCloseKV;
        s->groupsReadFunc               = d_storeGroupsReadKV;
        s->groupListFreeFunc            = d_storeGroupListFreeKV;
        s->groupInjectFunc              = d_storeGroupInjectKV;
        s->groupStoreFunc               = d_storeGroupStoreKV;
        s->getQualityFunc               = d_storeGetQualityKV;
        s->backupFunc                   = d_storeBackupKV;
        s->restoreBackupFunc            = d_storeRestoreBackupKV;
        s->actionStartFunc              = d_storeActionStartKV;
        s->actionStopFunc               = d_storeActionStopKV;
        s->messageStoreFunc             = d_storeMessageStoreKV;
        s->instanceDisposeFunc          = d_storeInstanceDisposeKV;
        s->instanceExpungeFunc          = d_storeInstanceExpungeKV;
        s->messageExpungeFunc           = d_storeMessageExpungeKV;
        s->deleteHistoricalDataFunc     = d_storeDeleteHistoricalDataKV;
        s->messagesInjectFunc           = d_storeMessagesInjectKV;
        s->instanceRegisterFunc         = d_storeInstanceRegisterKV;
        s->createPersistentSnapshotFunc = d_storeCreatePersistentSnapshotKV;
        s->instanceUnregisterFunc       = d_storeInstanceUnregisterKV;
        s->optimizeGroupFunc            = d_storeOptimizeGroupKV;
        s->nsIsCompleteFunc             = d_storeNsIsCompleteKV;
        s->nsMarkCompleteFunc           = d_storeNsMarkCompleteKV;
        s->transactionCompleteFunc      = d_storeTransactionCompleteKV;
        s->messagesLoadFunc             = d_storeMessagesLoadKV;
        s->messagesLoadFlushFunc        = d_storeMessagesLoadFlushKV;
    }

    return st;

err_storeNewKV:
    d_storeDeinitKV (st);
err_storeNewKV_early:
    os_free (st);
    return NULL;
}

void d_storeDeinitKV (d_storeKV st)
{
    kvdebug ("d_storeDeinitKV %p\n", (void *) st);

    ASSERT_STOREKV_NORET (st);

    assert (st->groups == NULL);
    assert (st->namespaces == NULL);

    c_free (st->topic_type);
        c_free (st->topicinfo_type);
    c_free (st->namespace_type);
    c_free (st->namespace_quality_type);
    c_free (st->namespace_completeness_type);
    c_free (st->group_type);
    c_free (st->open_transaction_eot_type);
    c_free (st->open_transaction_vtid_type);

    deinit_ser_deser_ctx(&st->topic_tctx);
        deinit_ser_deser_ctx(&st->topicinfo_tctx);
    deinit_ser_deser_ctx(&st->namespace_tctx);
    deinit_ser_deser_ctx(&st->namespace_completeness_tctx);
    deinit_ser_deser_ctx(&st->group_tctx);
    deinit_ser_deser_ctx(&st->open_transaction_eot_tctx);
    deinit_ser_deser_ctx(&st->open_transaction_vtid_tctx);

    hashtable_free(st->updated_namespaces);
    hashtable_free(st->groupTable);

    d_tableFree(st->open_transactions);

    os_mutexDestroy (&st->lock);
    os_condDestroy (&st->cond);
    /* call super-deinit */
    d_storeDeinit(d_store(st));
}

d_storeResult d_storeFreeKV (d_storeKV store)
{
    d_storeKV st;

    ASSERT_STOREKV (st, store);

    kvdebug ("d_storeFreeKV %p\n", (void *) st);

    if (store->opened) {
        return D_STORE_RESULT_PRECONDITION_NOT_MET;
    } else {
        d_objectFree(d_object(store));
        return D_STORE_RESULT_OK;
    }
}

#define VERSION_GET_ENCODING(x)     ( (x)&0x000000FF)
#define VERSION_GET_COMPRESSION(x)  (((x)&0x0000FF00) >> 8)
#define MAKE_VERSION(encoding, compression) (((encoding) & 0x000000FF) | (((compression) & 0x000000FF)<<8))

static d_storeResult getset_version (struct d_versionKV *out, d_storeKV st)
{
    d_storeResult result = D_STORE_RESULT_ERROR;
    const kv_key_t k = { TABLE_VERSION, 0, 0, 0 };
    kv_result_t rc;
    os_uint32 sz, tbuf[2];
    void *buf;
    if ((rc = kv_get (st->kv, &k, &sz, &buf, 0)) >= 0) {
        if (sz != sizeof (tbuf)) {
            kvlog (st, D_LEVEL_SEVERE, "getset_version: version from file corrupt\n");
            result = D_STORE_RESULT_MUTILATED;
        } else {
            int valid = 0;
            memcpy (tbuf, buf, sz);
            out->version = ntohl (tbuf[0]);
            out->encoding = (enum d_encodingKV) VERSION_GET_ENCODING(ntohl (tbuf[1]));
            out->compression = (enum d_compressionKV) VERSION_GET_COMPRESSION(ntohl (tbuf[1]));
            switch (out->encoding) {
            case D_ENCODINGKV_XML_XML:
            case D_ENCODINGKV_BIGE_XML:
            case D_ENCODINGKV_BIGECDR_XML:
            case D_ENCODINGKV_BIGECDR_V1:
                valid = 1;
                break;
                /* intentionally no default case: that results in
                 * warning with most compilers when the set of
                 * encodings is extended without updating this
                 * validation function */
            }
            if (valid) {
                valid = 0;
                switch (out->compression) {
                case D_COMPRESSION_NONE:
                case D_COMPRESSION_LZF:
                case D_COMPRESSION_SNAPPY:
                case D_COMPRESSION_ZLIB:
                case D_COMPRESSION_CUSTOM:
                    valid = 1;
                    break;
                    /* intentionally no default case: that results in
                     * warning with most compilers when the set of
                     * compression is extended without updating this
                     * validation function */
                }
                if (valid) {
                    result = D_STORE_RESULT_OK;
                } else {
                    kvlog (st, D_LEVEL_SEVERE, "getset_version: unknown compression %d\n", (int) out->compression);
                    result = D_STORE_RESULT_UNSUPPORTED;
                }
            } else {
                kvlog (st, D_LEVEL_SEVERE, "getset_version: unknown encoding %d\n", (int) out->encoding);
                result = D_STORE_RESULT_UNSUPPORTED;
            }
        }
        kvlog (st, D_LEVEL_FINEST, "getset_version: version read from file\n");
        os_free (buf);
    } else {
        switch (rc) {
        case KV_RESULT_NODATA:
            kvlog (st, D_LEVEL_FINE, "getset_version: no version in file yet\n");
            out->version = 0;
            out->encoding = kv_choose_encoding (st->kv, D_ENCODINGKV_BIGECDR_V1);
            out->compression = (out->encoding != D_ENCODINGKV_XML_XML) ? st->compression : D_COMPRESSION_NONE;
            tbuf[0] = htonl (out->version);
            tbuf[1] = htonl ((os_uint32)MAKE_VERSION(out->encoding, out->compression));
            rc = kv_put (st->kv, &k, sizeof (tbuf), tbuf, 0);
            if (rc >= 0) {
                kvlog (st, D_LEVEL_FINEST, "getset_version: version written\n");
                result = D_STORE_RESULT_OK;
            } else {
                kvlog (st, D_LEVEL_SEVERE, "getset_version: write of version failed\n");
                result = kv_convert_result(rc);
            }
            break;
        default:
            kvlog (st, D_LEVEL_SEVERE, "getset_version: read of version failed\n");
            result = kv_convert_result(rc);
            break;
        }
    }
    return result;
}

static d_storeResult submode_to_kind (kv_kind_t *k, const struct d_store_s *gstore)
{
    const char *sm = gstore->config->persistentKVStoreStorageType;
    if (sm == NULL || os_strcasecmp (sm, "sqlite") == 0 || os_strcasecmp (sm, "sqlite3") == 0) {
        *k = KV_KIND_SQLITE;
    } else if (os_strcasecmp (sm, "sqlitemt") == 0) {
        *k = KV_KIND_SQLITEMT;
    } else if (os_strcasecmp (sm, "leveldb") == 0) {
        *k = KV_KIND_LEVELDB;
    } else if (os_strcasecmp (sm, "lsds") == 0) {
        *k = KV_KIND_LSDS;
    } else if (os_strcasecmp (sm, "dummy") == 0) {
        *k = KV_KIND_DUMMY;
    } else {
        kvlog ((const struct d_storeKV_s *) gstore, D_LEVEL_SEVERE, "unknown kvstore submode '%s' configured\n", sm);
        return D_STORE_RESULT_ILL_PARAM;
    }
    return D_STORE_RESULT_OK;
}

d_storeResult d_storeOpenKV (d_store gstore)
{
    d_storeResult result;
    d_storeKV st;
    c_char* resultDir;
    struct d_versionKV version;
    kv_kind_t kind;
    memset (&version, 0, sizeof (version));

    ASSERT_STOREKV (st, gstore);

    if (st->opened) {
        result = D_STORE_RESULT_PRECONDITION_NOT_MET;
    } else if (gstore->config->persistentStoreDirectory == NULL) {
        result = D_STORE_RESULT_ILL_PARAM;
    } else if ((result = submode_to_kind (&kind, gstore)) != D_STORE_RESULT_OK) {
        ;
    } else if ((resultDir = d_storeDirNew (gstore, gstore->config->persistentStoreDirectory)) == NULL) {
        kvlog (st, D_LEVEL_SEVERE, "Persistent store directory '%s' could not be created.\n", gstore->config->persistentStoreDirectory);
        result = D_STORE_RESULT_PRECONDITION_NOT_MET;
    } else {
        os_free (gstore->config->persistentStoreDirectory);
        gstore->config->persistentStoreDirectory = resultDir;
        kvlog (st, D_LEVEL_INFO, "Persistent store directory '%s' opened.\n", gstore->config->persistentStoreDirectory);
        st->diskStorePath = os_strdup (gstore->config->persistentStoreDirectory);
        st->opened = 1;

        if (gstore->config->persistentKVStoreCompressionEnabled) {
            st->compression = gstore->config->persistentKVStoreCompressionAlgorithm;
            switch(st->compression) {
            case D_COMPRESSION_LZF:
                st->compressor = ut_compressorNew(NULL, "lzf", NULL);
                break;
            case D_COMPRESSION_SNAPPY:
                st->compressor = ut_compressorNew(NULL, "snappy", NULL);
                break;
            case D_COMPRESSION_ZLIB:
                st->compressor = ut_compressorNew(NULL, "zlib", NULL);
                break;
            case D_COMPRESSION_CUSTOM:
                /* fall through, custom not (yet) supported */
            default:
                st->compressor = NULL;
                break;
            }
        } else {
            st->compression = D_COMPRESSION_NONE;
        }

        if ((st->compression != D_COMPRESSION_NONE) && (st->compressor == NULL)) {
            kvlog (st, D_LEVEL_SEVERE, "Requested compressor '%s' could not be loaded\n", d_compressionKVImage(st->compression));
            result = D_STORE_RESULT_ERROR;
        } else if ((st->kv = kv_open (st, st->diskStorePath, kind, gstore->config->persistentKVStoreStorageParameters)) == NULL) {
            kvlog (st, D_LEVEL_SEVERE, "Persistent store '%s' could not be created.\n", gstore->config->persistentStoreDirectory);
            result = D_STORE_RESULT_ERROR;
        } else if ((result = getset_version (&version, st)) == D_STORE_RESULT_OK) {
            kvlog (st, D_LEVEL_INFO, "encoding %d compression %d version %d\n", (int) version.encoding, (int) version.compression, (int) version.version);
            st->encoding = version.encoding;
            if (st->compression != version.compression) {
                kvlog (st, D_LEVEL_SEVERE, "d_storeOpenKV: store compression does not match requested compression\n");
                result = D_STORE_RESULT_UNSUPPORTED;
            } else {
                result = init_namespaces (st);
                if (result == D_STORE_RESULT_OK) {
                    result = init_groups (st);
                }
                kvlog (st, D_LEVEL_INFO, "next available ids: namespace %u group %u topic %u\n", st->next_namespace_id, st->next_group_id, st->next_topic_id);
            }
        }
    }

    return result;
}

d_storeResult d_storeCloseKV (d_store gstore)
{
    d_storeKV st;
    LOCK_OPEN_STOREKV (st, gstore);

    kvdebug ("d_storeCloseKV %p\n", (void *) gstore);

    kv_close (st->kv);
    if (st->compression != D_COMPRESSION_NONE) {
        ut_compressorFree(st->compressor);
    }
    st->opened = 0;
    os_free (st->diskStorePath);
    st->diskStorePath = NULL;


    while (st->groups) {
        d_groupListKV g = st->groups;
        st->groups = g->_parent.next;
        free_group (g, st);
    }

    while (st->namespaces) {
        struct namespace *ns = st->namespaces;
        st->namespaces = ns->next;
        free_namespace (ns);
    }

    UNLOCK_OPEN_STOREKV (st);
    return D_STORE_RESULT_OK;
}

d_storeResult d_storeActionStartKV (const d_store gstore)
{
    d_storeKV st;

    if (kvlog_statistics) {
        LOCK_OPEN_STOREKV (st, gstore);

        st->action_started = TRUE;
        st->first_time = OS_TIMEE_ZERO;
        st->last_time = OS_TIMEE_ZERO;

        UNLOCK_OPEN_STOREKV (st);
    }

    return D_STORE_RESULT_OK;
}

d_storeResult d_storeActionStopKV (const d_store gstore)
{
    d_storeKV st;
    d_storeResult result = D_STORE_RESULT_OK;
    struct namespace *ns;
    os_duration diff;

    LOCK_OPEN_STOREKV (st, gstore);

    ns = (struct namespace *) hashtable_take (st->updated_namespaces);
    while (ns) {
       store_namespace_quality(st, ns);
       ns = (struct namespace *) hashtable_take (st->updated_namespaces);
    }
    st->action_started = FALSE;

    if (kvlog_statistics) {
        kvlog(st, D_LEVEL_FINEST, "Start time %"PA_PRItime"\n", OS_TIMEE_PRINT(st->first_time));
        kvlog(st, D_LEVEL_FINEST, "End time   %"PA_PRItime"\n", OS_TIMEE_PRINT(st->last_time));
        diff = os_timeEDiff(st->last_time, st->first_time);
        kvlog(st, D_LEVEL_FINEST, "Diff time  %"PA_PRIduration" seconds \n", OS_DURATION_PRINT(diff));
    }
    if (kv_commit (st->kv) < 0) {
        kvlog (st, D_LEVEL_SEVERE, "store-message: commit failed\n");
        result = D_STORE_RESULT_IO_ERROR;
    }
    UNLOCK_OPEN_STOREKV (st);
    return result;
}

d_storeResult d_storeGroupsReadKV (const d_store gstore, d_groupList *list)
{
    d_storeKV st;
    const struct d_groupListKV_s *g;
    if (list == NULL) {
        return D_STORE_RESULT_ILL_PARAM;
    }
    LOCK_OPEN_STOREKV (st, gstore);

    kvlog (st, D_LEVEL_INFO, "groups-read: building list of non-backup groups\n");

    *list = NULL;
    for (g = st->groups; g; g = g->_parent.next) {
        char buf[256];
        snprintf (buf, sizeof (buf), "groups-read: considering group %s.%s id %u namespace %s id %u/%u: ",
                  g->_parent.partition, g->_parent.topic, g->group_id,
                  g->namespace->name, g->namespace->id, g->namespace->version);
        if (group_latest_namespace_version (st, g)) {
            d_groupList tmp;
            kvlog (st, D_LEVEL_FINEST, "%scurrent\n", buf);
            tmp = os_malloc (sizeof (*tmp));
            *tmp = g->_parent;
            tmp->next = *list;
            *list = tmp;
        } else {
            kvlog (st, D_LEVEL_FINEST, "%sbackup\n", buf);
        }
    }

    UNLOCK_OPEN_STOREKV (st);
    return D_STORE_RESULT_OK;
}

d_storeResult d_storeGroupListFreeKV (const d_store gstore, d_groupList list)
{
    d_groupList next;

    ASSERT_STOREKV_NORET(gstore);
    OS_UNUSED_ARG(gstore);

    next = list;
    while (next) {
        list = list->next;
        os_free(next);
        next = list;
    }

    return D_STORE_RESULT_OK;
}

static void set_kernel_group (v_public entity, c_voidp args)
{
    d_group group = d_group (args);
    d_groupSetKernelGroup (group, v_group (entity));
}

d_storeResult d_storeGroupInjectKV (const d_store gstore, const c_char *partition, const c_char *topic, const u_participant participant, d_group *pgroup)
{
    d_groupListKV g;
    struct d_topicInfoKV *tp;
    d_storeResult result;
    u_topic utopic;
    u_group ugroup;
    u_partition upartition;
    v_partitionQos partitionQos;
    d_storeKV st;
    if (participant == NULL || partition == NULL || topic == NULL || pgroup == NULL) {
        return D_STORE_RESULT_ILL_PARAM;
    }
    LOCK_OPEN_STOREKV (st, gstore);

    kvdebug ("groupinject %s.%s\n", partition, topic);

    *pgroup = NULL;

    if ((g = find_group (st, partition, topic)) == NULL) {
        kvdebug ("  group not found\n");
        result = D_STORE_RESULT_ERROR;
        goto err_getgroup;
    }
    kvdebug ("  group %p topic_id %d\n", (void *) g, (int) g->topic_id);
    if ((tp = get_topicInfoKV (st, g->topic_id, g->_parent.topic)) == NULL) {
        kvdebug ("  get_topic failed\n");
        result = D_STORE_RESULT_MUTILATED; /* FIXME - not really a critical error, but "mutilated" sounds very bad */
        goto err_readTopicMetadata;
    }
    kvdebug ("  topic %p\n", (void *) tp);
    kvdebug ("  typename %s keylist %s\n", tp->info.type_name, tp->info.key_list);
    if ((utopic = u_topicNewFromTopicInfo (participant, &tp->info, TRUE)) == NULL) {
        kvlog (st, D_LEVEL_SEVERE, "Topic '%s' with typeName '%s' and keylist '%s' could NOT be created.\n", tp->info.name, tp->info.type_name, tp->info.key_list);
        result = D_STORE_RESULT_METADATA_MISMATCH;
        goto err_topicNew;
    }
    kvlog (st, D_LEVEL_FINE, "Topic %s created.\n", tp->info.name);

    if ((partitionQos = u_partitionQosNew (NULL)) == NULL) {
        kvlog (st, D_LEVEL_SEVERE, "PartitionQos could NOT be created.\n");
        result = D_STORE_RESULT_OUT_OF_RESOURCES;
        goto err_partitionQosNew;
    }
    kvlog (st, D_LEVEL_FINE, "PartitionQoS created.\n");
    if ((upartition = u_partitionNew (participant, partition, partitionQos)) == NULL) {
        kvlog (st, D_LEVEL_SEVERE, "Partition %s could NOT be created.\n", partition);
        result = D_STORE_RESULT_ERROR;
        goto err_partitionNew;
    }
    kvlog (st, D_LEVEL_FINE, "Partition %s created.\n", partition);

    if ((ugroup = u_groupNew (participant, partition, topic, OS_DURATION_ZERO)) == NULL) {
        kvlog (st, D_LEVEL_SEVERE, "Group %s.%s could NOT be created.\n", partition, tp->info.name);
        result = D_STORE_RESULT_ERROR;
        goto err_groupNew;
    }
    kvlog (st, D_LEVEL_INFO, "Group %s.%s created.\n", partition, tp->info.name);


    *pgroup = d_groupNew (partition, topic, D_DURABILITY_PERSISTENT, g->_parent.completeness, g->_parent.quality);
    result = D_STORE_RESULT_OK;

    u_observableAction (u_observable (ugroup), set_kernel_group, *pgroup);
    /* FIXME: enable again after fix is in place (See OSPL-2750)
    u_objectFree (u_object (ugroup)); */
err_groupNew:
    u_objectFree(u_object (upartition));
err_partitionNew:
    u_partitionQosFree (partitionQos);
err_partitionQosNew:
    (void)u_objectFree (u_object (utopic));
err_topicNew:
    c_free (tp);
err_readTopicMetadata:
err_getgroup:
    UNLOCK_OPEN_STOREKV (st);
    return result;
}

d_storeResult d_storeGetQualityKV (const d_store gstore, const d_nameSpace nameSpace, os_timeW *quality)
{
    /* TODO: what is returned if there are no namespaces or strcmp() fails */
    d_storeKV st;
    struct namespace *ns;
    char *name;
    if (quality == NULL || nameSpace == NULL) {
        return D_STORE_RESULT_ILL_PARAM;
    }
    name = d_nameSpaceGetName (nameSpace);
    LOCK_OPEN_STOREKV (st, gstore);
    for (ns = st->namespaces; ns; ns = ns->next) {
        if (strcmp (ns->name, name) == 0) {
            *quality = ns->quality;
            break;
        }
    }
    UNLOCK_OPEN_STOREKV (st);
    return D_STORE_RESULT_OK;
}

d_storeResult d_storeGroupStoreKV (const d_store gstore, const d_group dgroup, const d_nameSpace nameSpace)
{
    c_char *topicName = NULL;
    c_char *partitionName = NULL;
    v_group group;
    d_storeResult result = D_STORE_RESULT_ERROR;
    kv_result_t rc;
    os_timeW quality;
    d_storeKV st;
    os_uint32 topic_id;
    os_int ret;
    struct namespace *ns;
    if (dgroup == NULL) {
        return D_STORE_RESULT_ILL_PARAM;
    }

    LOCK_OPEN_STOREKV (st, gstore);

    quality = os_timeWGet();

    group = d_groupGetKernelGroup (dgroup);
    topicName = v_topicName (v_groupTopic (group));
    partitionName = v_partitionName (v_groupPartition (group));
    kvlog (st, D_LEVEL_FINE, "Storing group (partition.topic): '%s.%s'\n", partitionName, topicName);

    /* If we already have the group on-disk (not sure that can
     * happen), no need to do anything. */
    if (find_group (st, partitionName, topicName) != NULL) {
        kvlog (st, D_LEVEL_FINE, "Group '%s.%s' already known on-disk\n", partitionName, topicName);
        result = D_STORE_RESULT_OK;
        goto out;
    }

    /* Need a topic_id; need to ensure topic metadata is on-disk.
     *
     * FIXME: should be a bit more sensible in handling
     * topics/topic_ids. This scanning of a list of groups ok but
     * can be improved (it is functionally correct, though). */
    {
        d_groupListKV g = find_group_w_topic (st, topicName);
        if (g != NULL) {
            topic_id = g->topic_id;
            kvlog (st, D_LEVEL_FINER, "No need to store topic metadata because of existence of %s.%s; topic_id = %u\n", g->_parent.partition, topicName, topic_id);
        } else {
            struct d_topicInfoKV *tp;
            topic_id = st->next_topic_id;
            kvlog (st, D_LEVEL_FINER, "Must store topic metadata for '%s.%s'; topic_id = %u\n",
                           partitionName, topicName, topic_id);
            if ((tp = make_topicInfoKV (st, v_groupTopic (group))) == NULL) {
                kvlog (st, D_LEVEL_SEVERE, "Couldn't store topic metadata for topic '%s': out of resources\n", topicName);
                result = D_STORE_RESULT_OUT_OF_RESOURCES;
                goto out;
            } else {
                ret = put_topicInfoKV (st, topic_id, tp);
                c_free (tp);
                if (ret < 0) {
                    kvlog (st, D_LEVEL_SEVERE, "Couldn't store topic metadata for topic '%s'\n", topicName);
                    result = D_STORE_RESULT_PRECONDITION_NOT_MET;
                    goto out;
                } else {
                    kvlog (st, D_LEVEL_FINE, "Stored metadata for topic '%s'\n", topicName);
                    st->next_topic_id++;
                }
            }
        }
    }

    /* See if this is a new name space; if it is, store it */
    if ((ns = find_or_create_namespace (st, nameSpace)) == NULL) {
        kvlog (st, D_LEVEL_SEVERE, "No namespace found for group '%s.%s'\n", partitionName, topicName);
        goto out;
    } else if ((result = store_namespace (st, ns)) != D_STORE_RESULT_OK) {
        goto out;
    }


    /* Default completeness is INCOMPLETE, so no need to store
     * completeness. But we do need to store the quality. */
    {
        struct d_namespaceQualityKV *q;
        q = c_new (st->namespace_quality_type);
        /* Convert quality to c_time */
        d_qualityExtFromQuality(&q->quality, &quality, TRUE);
        if ((rc = put2_ser (st, TABLE_NAMESPACE_QUALITY, ns->id, ns->version, st->namespace_quality_type, &st->namespace_quality_tctx, q)) < 0) {
            kvlog (st, D_LEVEL_SEVERE, "Could not store quality for namespace '%s' id %u/%u\n", ns->name, ns->id, ns->version);
            result = kv_convert_result(rc);
            c_free (q);
            goto out;
        }
        c_free (q);
        kvlog (st, D_LEVEL_FINEST, "Namespace '%s' id %u/%u quality written to disk\n", ns->name, ns->id, ns->version);
        kvlog (st, D_LEVEL_FINE, "Group '%s.%s' has quality: %"PA_PRItime"\n", partitionName, topicName, OS_TIMEW_PRINT(quality));
    }

    /* Topic and namespace written to disk - so can now write group as
     * well. Note that we already know it is a new group. */
    {
        os_uint32 group_id = st->next_group_id;
        struct d_groupKV *g;
        g = c_new (st->group_type);
        g->topic_id = topic_id;
        g->namespace_id = ns->id;
        g->namespace_version = ns->version;
        g->partition = c_stringNew (c_getBase (g), partitionName);
        g->topic = c_stringNew (c_getBase (g), topicName);
        if ((rc = put1_ser (st, TABLE_GROUP, group_id, st->group_type, &st->group_tctx, g)) < 0) {
            kvlog (st, D_LEVEL_SEVERE, "Could not store group '%s.%s' id %u\n", partitionName, topicName, group_id);
            result = kv_convert_result(rc);
        } else {
            d_groupListKV gg;
            kvlog (st, D_LEVEL_FINEST, "Group '%s.%s' id %u written to disk\n", partitionName, topicName, group_id);
            st->next_group_id++;
            result = new_group (&gg, st, group_id, ns, partitionName, topic_id, topicName, &quality);
        }
        c_free (g);
    }

    /* Commit whatever outstanding transactions there are */
    if (kv_commit (st->kv) < 0) {
        kvlog (st, D_LEVEL_SEVERE, "store-group: commit failed\n");
        result = D_STORE_RESULT_IO_ERROR;
    }

out:
    c_free(group);
    UNLOCK_OPEN_STOREKV (st);
    return result;
}

static int history_is_keepall (const struct v_topicQos_s *topicQos)
{
    return
        topicQos->durabilityService.v.history_kind == V_HISTORY_KEEPALL ||
        topicQos->durabilityService.v.history_depth == V_LENGTH_UNLIMITED;
}

static int history_is_keeplast (const struct v_topicQos_s *topicQos)
{
    return !history_is_keepall (topicQos);
}

static struct msg *find_insertafter_point (const struct inst *inst, const struct v_message_s *msg)
{
    if (inst->oldest == NULL) {
        /* Trivial case: empty sequence */
        return NULL;
    } else if (os_timeWCompare (inst->latest->writeTime, msg->writeTime) == OS_LESS) {
        /* Expected case: msg is more recent than latest */
        return inst->latest;
    } else {
        /* Scan the list from new to old to find the insertion point:
         * the assumption is that it will be more likely that it is a
         * recent message than that it is a very old one.  Major key
         * is writeTime, but id and sequence number are factored in as
         * well to guarantee a total order, which is needed for
         * duplicate detection. */
        struct msg *mprev = inst->latest;
        while (mprev && msg_lt_v_message (mprev, msg)) {
            mprev = mprev->older;
        }
        return mprev;
    }
}

#ifndef NDEBUG
static int mid_is_available (const struct inst *inst, os_uint64 mid)
{
    struct msg *m;
    for (m = inst->oldest; m; m = m->newer) {
        if (m->key.mid == mid) {
            return 0;
        }
    }
    return 1;
}
#endif

static d_storeResult add_message_to_instance (struct msg **pm, d_storeKV st, struct d_groupListKV_s *g, v_topic vtopic, struct inst *inst, struct msg *mprev, const struct v_message_s *msg, const os_uint64 *p_mid, const v_orderbyKind order_by)
{
    const int reconstructing = (p_mid != NULL);
    struct msg *m, *oldest_non_transaction;
    struct transaction_list *tr_list, *tr_list_found;
    struct transaction_msg *tr_msg;
    kv_result_t rc;
    c_bool tr_list_created = FALSE;

    *pm = NULL; /* Needed to satisfy strict compiler rules */
    if ((m = os_malloc (sizeof (*m))) == NULL) {
        return D_STORE_RESULT_OUT_OF_RESOURCES;
    }
    m->key = inst->next_msg_key;
    if (p_mid != NULL) {
        /* Overrule MID part of key; but do check it is unique */
        assert (*p_mid >= m->key.mid || mid_is_available (inst, *p_mid));
        m->key.mid = *p_mid;
    }
    m->state = v_nodeState ((c_object)msg);
    m->writeTime = msg->writeTime;
    m->writerGID = msg->writerGID;
    m->sequenceNumber = msg->sequenceNumber;
    m->transactionId = msg->transactionId;
    m->inst = inst;

    if (m->state & L_TRANSACTION) {
        /* The message is an open transactional message so we
         * must add it to the list of open transaction
         * messages. The open transaction message contains a
         * reference to the actual message, which is inserted
         * in the instance. The actual message has the L_TRANSACTION
         * flag set. This flag is reset once the transaction becomes
         * complete. */
        tr_list = os_malloc(sizeof(*tr_list));
        tr_list->seqNum = msg->transactionId;
        tr_list->gid = msg->writerGID;
        tr_list->head = NULL;
        tr_list->tail = NULL;
        tr_list->commit_state = reconstructing ? UNDECIDED : OPEN;
        tr_list->otid = ((st->otid % 2) == 0) ? st->otid : st->otid+1;   /* unique id, 63-bits, always even for open transaction  message */
        st->otid = tr_list->otid + 1;
        if ((tr_list_found = d_tableInsert(st->open_transactions, tr_list)) != NULL) {
            os_free(tr_list);
            tr_list = tr_list_found;
        } else {
            tr_list_created = TRUE;
        }
        tr_msg = os_malloc(sizeof(*tr_msg));
        if (p_mid == NULL) {
            if (tr_list_created) {
                /* A new transaction list is generated, not reconstructing.
                 * Write an entry in the journal for the open transaction. */

                struct v_tid *tid;
                kv_key_t key;

                tid = c_new (st->open_transaction_vtid_type);
                tid->wgid = tr_list->gid;
                tid->seqnr = tr_list->seqNum;
                key.table = TABLE_MESSAGE_EOT;
                key.gid   = 0;
                key.iid   = 0;
                key.mid   = tr_list->otid;  /* unique id, 63-bits transactionId and 1-bit open/close */
                if ((rc = put_ser (st, &key, st->open_transaction_vtid_type, &st->open_transaction_vtid_tctx, tid)) < 0) {
                    os_free(m);
                    os_free(tr_msg);
                    c_free(tid);
                    return kv_convert_result(rc);
                }
                kvdebug("  add tid msg to journal with key=%"PA_PRIu32":%"PA_PRIu32":%"PA_PRIu64" for gid=(%d, %d, %d), transaction=%u\n",
                        key.gid, key.iid, key.mid, tr_list->gid.systemId, tr_list->gid.localId, tr_list->gid.serial, tr_list->seqNum);
                c_free(tid);
            } else {
                kvdebug("  transaction %d for gid=(%d, %d, %d) already open, NOT adding tid to journal\n", tr_list->seqNum, tr_list->gid.systemId, tr_list->gid.localId, tr_list->gid.serial);
            }
        }
        /* Add the message to the transaction list */
        tr_msg->msg_ref = m;
        tr_msg->next = NULL;
        if (tr_list->head == NULL) {
            assert(tr_list->tail == NULL);
            tr_list->head = tr_msg;
        } else {
            assert(tr_list->tail != NULL);
            tr_list->tail->next = tr_msg;
        }
        tr_list->tail = tr_msg;
    }

    /* When reconstructing state, we keep the v_message but do not
     * write it to the store.  Once we start injecting the data
     * properly, the v_messages we kept in shared memory will be
     * referenced by the group, then we drop the reference, and we
     * will never need significantly more memory than what the kernel
     * needs to store the persistent data anyway. */
    if (reconstructing) {
        m->v_msg = c_keep ((v_message) msg);
    } else {
        m->v_msg = NULL;
        if ((rc = put_ser_vmsg (st, &m->key, vtopic, &g->tctx, msg)) < 0) {
            os_free (m);
            return kv_convert_result(rc);
        }
    }

    if (mprev == NULL) {
        /* new oldest of possibly empty list */
        m->older = NULL;
        m->newer = inst->oldest;
        inst->oldest = m;
        if (m->newer) {
            m->newer->older = m;
        } else {
            inst->latest = m;
        }
    } else {
        m->newer = mprev->newer;
        if (m->newer) {
            m->newer->older = m;
        } else {
            inst->latest = m;
        }
        m->older = mprev;
        m->older->newer = m;
    }
    /* Only increase the writeCount for write messages that are
     * not part of a coherent transaction. */
    if ((!(m->state & L_TRANSACTION)) && (m->state & L_WRITE)) {
        inst->writeCount++;

        /* Update the oldest_non_transaction when needed */
        oldest_non_transaction = inst->oldest_non_transaction;
        /* Maintain the oldest_non_transaction */
        inst->oldest_non_transaction = get_oldest_non_transaction(m, inst->oldest_non_transaction, order_by);
        if (oldest_non_transaction != inst->oldest_non_transaction) {
            kvdebug("  oldest_non_transaction set to %p\n", (void *)inst->oldest_non_transaction);
        }
    }
    if (m->key.mid >= inst->next_msg_key.mid) {
        inst->next_msg_key.mid = m->key.mid + 1;
    }
    *pm = m;
    return D_STORE_RESULT_OK;
}


/**
 * Find the oldest non_transactional write message, starting from msg
 * (not considering msg as a candidate itself)
 *
 * A non-transaction write message is a write message with the L_WRITE flag set
 * and the L_TRANSACTION not set.
 *
 * This function returns the oldest non-transaction write message by starting
 * from msg and work your way upwards (to newer) messages.
 *
 * If no such message can be found NULL is returned.
 */
static struct msg *find_oldest_non_transaction (const struct inst *inst, struct msg *msg)
{
    struct msg *m;

    OS_UNUSED_ARG(inst);

    assert(msg);   /* If the message  */

    /* Start at msg and work your way up to newer messages
     * until you find a message with L_WRITE but not L_TRANSACTION. */
    m = (msg->newer) ? msg->newer : NULL;
    while ( (m != NULL) && !(((m->state & L_WRITE) && (!(m->state & L_TRANSACTION)))) ) {
        m = m->newer;
    }
    return m;
}


static d_storeResult drop_just_message (d_storeKV st, struct inst *inst, struct msg *m, int reconstructing)
{
    kv_result_t rc;

    assert(m);
    if (m->older == NULL) {
        inst->oldest = m->newer;
    } else {
        m->older->newer = m->newer;
    }
    if (m->newer == NULL) {
        inst->latest = m->older;
    } else {
        m->newer->older = m->older;
    }
    /* Decrease the writecount only for non-transactional write messages. */
    if ((!(m->state & L_TRANSACTION)) && (m->state & L_WRITE)) {
        assert (inst->writeCount > 0);
        inst->writeCount--;
    }
    /* Going to the oldest_non_transaction, recalculate a new one */
    if (m == inst->oldest_non_transaction) {
        inst->oldest_non_transaction = find_oldest_non_transaction(inst, m);
    }
    /* FIXME: during reconstruction, there is an argument to be made
     * that messages that get dropped should be deleted, to avoid
     * leaking them.  However, there is also an argument to be made
     * that reconstructing with the store in read-only mode is
     * attractive.  The risk of leakage is very low, at worst means
     * one data message and some disposes at every crash. */
    rc = reconstructing ? 0 : kv_delete (st->kv, &m->key);
    msg_free (m);
    return kv_convert_result(rc);
}

static d_storeResult drop_message (d_storeKV st, struct d_groupListKV_s *g, struct inst **inst, struct msg *m, int reconstructing)
{
    d_storeResult result;
    if ((result = drop_just_message (st, *inst, m, reconstructing)) != D_STORE_RESULT_OK) {
        return result;
    }
    if ((*inst)->oldest == NULL) {
        assert (ut_avlLookup (&group_inst_td, &g->instances_by_iid, &(*inst)->next_msg_key.iid) == *inst);
        ut_avlDelete (&group_inst_td, &g->instances_by_iid, *inst);
        d_tableRemove (g->instances, *inst);
        inst_free (*inst);
        *inst = NULL;
    }
    return result;
}

static struct msg *find_message (const struct inst *inst, const struct v_message_s *msg)
{
    const struct msg *m;
    for (m = inst->oldest; m; m = m->newer) {
        int cmp = msg_v_message_cmp (m, msg);
        if (cmp == 0) {
            return (struct msg *) m;
        } else if (cmp > 0) {
            return NULL;
        }
    }
    return NULL;
}


static d_storeResult check_and_update_history (struct d_storeKV_s *st, struct inst *inst, const os_int32 depth, const int reconstructing)
{
    d_storeResult result;

    kvdebug ("  keeplast, checking history\n");
    assert(depth > 0);
    assert(inst->writeCount <= depth + 1);
    if (inst->writeCount == depth + 1) {
        assert(inst->oldest_non_transaction);
        /* Drop the oldest_non_transaction_message.
         * A new oldest_non_transaction message is automatically selected */
        kvdebug ("    drop msg %p\n", (void *) inst->oldest_non_transaction);
        if ((result = drop_message (st, inst->g, &inst, inst->oldest_non_transaction, reconstructing)) != D_STORE_RESULT_OK) {
            kvdebug ("    drop failed\n");
            return result;
        }
    }
    assert(inst->writeCount <= depth);
    /* Drop outdated disposes that do not apply anymore, but only
     * if the dispose does not belong to a transaction. */
    while (inst && (inst->writeCount == depth && !(inst->oldest->state & L_WRITE) && !(inst->oldest->state & L_TRANSACTION))) {
        kvdebug ("    drop oldest msg %p\n", (void *) inst->oldest);
        if ((result = drop_message (st, inst->g, &inst, inst->oldest, reconstructing)) != D_STORE_RESULT_OK) {
            kvdebug ("    drop failed\n");
            return result;
        }
    }
    return D_STORE_RESULT_OK;
}


static d_storeResult store_message (struct d_storeKV_s *st, struct v_group_s *group, const struct v_message_s *msg, struct d_groupListKV_s *g, const os_uint32 *p_iid, const os_uint64 *p_mid)
{

    const int reconstructing = (p_iid != NULL);
    v_topic vtopic = v_groupTopic ((c_object) group);
    struct v_topicQos_s const * const topicQos = v_topicQosRef (vtopic);
    const os_int32 depth = topicQos->durabilityService.v.history_depth;
    struct inst *inst;
    struct msg *m, *mprev;
    d_storeResult result;
    v_orderbyKind order_by;

    assert (g);
    assert ((p_iid == NULL && p_mid == NULL) || (p_iid != NULL && p_mid != NULL));
    assert (c_getType (c_object (msg)) == v_topicMessageType (v_groupTopic ((c_object)group)));

    kvdebug ("store_message: group %p vmsg %p state %d (reconstructing=%d)\n", (void *) group, (void *) msg, v_nodeState((c_object)msg), reconstructing);

    if (reconstructing) {
        kvdebug ("  [reconstructing iid %u mid %llu]\n", *p_iid, *p_mid);
    }

#if 0
    if (!reconstructing && (d_tableSize(g->instances) == 0)) {
        kvlog(st, D_LEVEL_FINE, "No data exists yet on disk for group '%s.%s'\n", g->_parent.partition, g->_parent.topic);
    }
#endif
    kvdebug ("  groupKV %p\n", (void *) g);
    if ((result = find_or_create_instance (&inst, st, g, group, msg, p_iid)) != D_STORE_RESULT_OK) {
        kvdebug ("  find_or_create_instance failure\n");
        return result;
    }

    assert (history_is_keepall (topicQos) || inst->writeCount <= depth);

    /* Decide whether to add MSG to the on-disk store and at what
     * location in the in-memory sequence to adminster it. If the
     * order_by kind is BY_RECEPTIONTIME then the message should
     * always be added. If it is the first message non-transactional
     * message and the order is BY_RECEPTIONTIME then it is by
     * definition the oldest non-transaction message until it
     * gets pushed out of the history. */
    order_by = topicQos->orderby.v.kind;
    if (order_by == V_ORDERBY_RECEPTIONTIME) {
        /* oldest == NULL => latest is undefined */
        mprev = inst->oldest ? inst->latest : NULL;
        kvdebug ("  by-reception mprev msg %p\n", (void *) mprev);
    } else {
        mprev = find_insertafter_point (inst, msg);
        kvdebug ("  by-source mprev msg %p\n", (void *) mprev);
         if (mprev != NULL && msg_v_message_cmp (mprev, msg) == 0) {
            /* Discard duplicates. */
            kvdebug ("  drop: duplicate vmsg %p\n", (void *)msg);
            return D_STORE_RESULT_OK;
        }
        /* Messages that belong to a coherent transaction may always
         * be added. In all other cases check whether adding the message
         * is allowed */
        if (!(v_nodeState ((c_object)msg) & L_TRANSACTION)) {
            if ((mprev == NULL) && (history_is_keeplast (topicQos)) && (inst->writeCount >= depth)) {
                /* Msg is the oldest message in the list and the history is
                 * full, drop it immediately. */
                kvdebug ("  drop: vmsg %p too old, won't fit\n", (void *)msg);
                return D_STORE_RESULT_OK;
            } else if ((inst->oldest_non_transaction != NULL) && (msg_v_message_cmp(inst->oldest_non_transaction, msg) > 0) && (history_is_keeplast (topicQos)) && (inst->writeCount >= depth)) {
                /* Msg is older than the oldest non-transaction message
                 * and the history is full, drop it immediately. */
                kvdebug ("  drop: vmsg %p too old, won't fit\n", (void *)msg);
                return D_STORE_RESULT_OK;
            }
        }
    }
    /* Now we can safely add the message, but we may have to push out
     * the oldest non-transaction message to meet the resource limits. */

    kvdebug ("  adding ...\n");

    /* Add the new one, potentially growing the size of the history
     * beyond what is allowed. If we crash after this write is stable
     * but before the following deletes are, then we will have too
     * much data on disk.
     *
     * A "dumb" injection process will reproduce all this messages,
     * and that is application-visible (if the user starts a clean
     * system in which an application has history KEEP_ALL while the
     * durability service has history setting KEEP_LAST, and the
     * application exists before the persistent data gets injected).
     *
     * There are multiple approaches to solving this. The obvious one
     * is to use transactions to ensure this situation can never
     * occur. But that significantly increases the requirements on the
     * underlying key-value store.
     *
     * Another one is to rebuild the in-memory state first by reading
     * what's in the on-disk store, without writing anything to
     * disk. Note that the actual user data is not needed for
     * this. Any data that is pushed out of the memory by definition
     * is data that should've been deleted from the on-disk store --
     * and if desired, these can be removed later on.
     *
     * After this, the in-memory state contains just the messages that
     * need to be republished, so that it becomes a matter of
     * retrieving the payload for each of the message keys and
     * republishing it.
     *
     * A side-effect of this procedure is that it builds up enough
     * state in-memory to recognise injected data so that it can be
     * ignored rather than written to disk again. Thereby, the master
     * could essentially be read-only while injecting data. */
    if ((result = add_message_to_instance (&m, st, g, vtopic, inst, mprev, msg, p_mid, order_by)) != D_STORE_RESULT_OK) {
        kvdebug ("  add failed\n");
        return result;
    }

    kvdebug("  msg %p for vmsg %p added to instance %p\n", (void *)m, (void *)msg, (void *)inst);

    /* If the history is full then drop the oldest non-transaction
     * message. Keep dropping non-transactional messages until the history
     * is no longer overfull. When the number of writes equals the history
     * depth, we have no use for disposes at the beginning of the sequence,
     * so drop everything until the history is no longer overful and
     * there. But do not drop message when the is L_TRANSACTION
     * flag set */

    if (history_is_keeplast (topicQos)) {
        if ((result = check_and_update_history(st, inst, depth, reconstructing)) != D_STORE_RESULT_OK) {
            return result;
        }
    }

    /* Update namespace quality. FIXME: I don't think there's much use
     * for updating it on-disk each time, but do so for now. */
    if (!reconstructing) {
        os_timeW quality;
        quality = os_timeWGet ();
        set_namespace_quality_deferred (st, g->namespace, &quality);
    }
    return D_STORE_RESULT_OK;
}


d_storeResult d_storeMessageStoreKV (const d_store gstore, const v_groupAction msg)
{
    d_storeKV st;
    d_storeResult result;
    d_groupListKV g;

    kvdebug("d_storeMessageStoreKV\n");

    if (msg == NULL || msg->group == NULL || msg->message == NULL) {
        return D_STORE_RESULT_ILL_PARAM;
    }

    /* Analysis of v_group.c, v_writer.c, d_persistentDataListener.c
     * says this is either a WRITE or a WRITE_DISPOSE, and may
     * have TRANSACTION and/or SYNCHRONOUS and/or ALIGN set, but nothing else. */
    assert (v_nodeState (msg->message) & L_WRITE);
    assert ((v_nodeState (msg->message) & ~(L_WRITE | L_DISPOSED | L_TRANSACTION | L_SYNCHRONOUS)) == 0);

    LOCK_OPEN_STOREKV (st, gstore);

    /* Code used to benchmark KV store (DDS1582) */
    if (kvlog_statistics && st->action_started) {
        st->first_time = os_timeEGet();
        st->action_started = FALSE;
    }
    /* end of benchmark code */

    if ((g = find_group_w_kernelgroup (st, msg->group)) == NULL) {
        result = D_STORE_RESULT_PRECONDITION_NOT_MET;
    } else {
        /* Store the kernel group in the groupListKV for easy access */
        g->vgroup = msg->group;

        result = store_message (st, msg->group, msg->message, g, NULL, NULL);

        /* Code used to benchmark KV store (DDS1582) */
        if (kvlog_statistics) {
            st->last_time = os_timeEGet();
        }
        /* end of benchmark code */
    }

    UNLOCK_OPEN_STOREKV (st);
    return result;
}

d_storeResult d_storeInstanceDisposeKV (const d_store gstore, const v_groupAction msg)
{
    d_storeKV st;
    d_storeResult result;
    d_groupListKV g;

    if (msg == NULL || msg->group == NULL || msg->message == NULL) {
        return D_STORE_RESULT_ILL_PARAM;
    }

    /* Analysis of v_group.c, v_writer.c, d_persistentDataListener.c
     * says that this is DISPOSE, and may have TRANSACTION and/or
     * SYNCHRONOUS and/or ALIGNED set, but nothing else. */
    assert (v_nodeState (msg->message) & L_DISPOSED);
    assert ((v_nodeState (msg->message) & ~(L_DISPOSED | L_TRANSACTION | L_SYNCHRONOUS)) == 0);

    LOCK_OPEN_STOREKV (st, gstore);
    if ((g = find_group_w_kernelgroup (st, msg->group)) == NULL) {
        result = D_STORE_RESULT_PRECONDITION_NOT_MET;
    } else {
        /* Store the kernel group in the groupListKV for easy access */
        g->vgroup = msg->group;

        result = store_message (st, msg->group, msg->message, g, NULL, NULL);
    }
    UNLOCK_OPEN_STOREKV (st);
    return result;
}

d_storeResult d_storeInstanceExpungeKV (const d_store gstore, const v_groupAction msg)
{
    d_storeKV st;
    d_storeResult result;
    struct d_groupListKV_s *g;
    struct inst *inst;

    if (msg == NULL || msg->group == NULL || msg->message == NULL) {
        return D_STORE_RESULT_ILL_PARAM;
    }

    kvdebug ("d_storeInstanceExpungeKV\n");

    LOCK_OPEN_STOREKV (st, gstore);
    if ((g = find_group_w_kernelgroup (st, msg->group)) == NULL) {
        result = D_STORE_RESULT_PRECONDITION_NOT_MET;
    } else if ((result = find_instance (&inst, g, msg->group, msg->message)) != D_STORE_RESULT_OK || inst == NULL) {
        ; /* skip */
    } else {
        result = D_STORE_RESULT_OK;
        while ((result == D_STORE_RESULT_OK) &&
               (inst) &&
               (inst->oldest)) {
            result = drop_message (st, g, &inst, inst->oldest, 0);
        }
        if (result == D_STORE_RESULT_OK && kv_commit (st->kv) < 0) {
            kvlog (st, D_LEVEL_SEVERE, "instance-expunge: commit failed\n");
            result = D_STORE_RESULT_IO_ERROR;
        }
    }
    UNLOCK_OPEN_STOREKV (st);
    return result;
}

d_storeResult d_storeMessageExpungeKV (const d_store gstore, const v_groupAction msg)
{
    d_storeKV st;
    d_storeResult result;
    struct d_groupListKV_s *g;
    struct inst *inst;
    struct msg *m;

    if (msg == NULL || msg->group == NULL || msg->message == NULL) {
        return D_STORE_RESULT_ILL_PARAM;
    }

    kvdebug ("d_storeMessageExpungeKV\n");

    LOCK_OPEN_STOREKV (st, gstore);
    if ((g = find_group_w_kernelgroup (st, msg->group)) == NULL) {
        result = D_STORE_RESULT_PRECONDITION_NOT_MET;
    } else if ((result = find_instance (&inst, g, msg->group, msg->message)) != D_STORE_RESULT_OK || inst == NULL) {
        ; /* skip */
    } else if ((m = find_message (inst, msg->message)) != NULL) {
        result = drop_message (st, g, &inst, m, 0);
        if (result == D_STORE_RESULT_OK && kv_commit (st->kv) < 0) {
            kvlog (st, D_LEVEL_SEVERE, "message-expunge: commit failed\n");
            result = D_STORE_RESULT_IO_ERROR;
        }
    } else {
        result = D_STORE_RESULT_OK;
    }
    UNLOCK_OPEN_STOREKV (st);
    return result;
}

struct delete_historical_data_helper_arg {
    struct d_storeKV_s *st;
    os_timeW delete_time;
    c_iter instances_to_delete;
    d_storeResult result;
};


static c_bool delete_historical_data_helper (struct inst *inst, struct delete_historical_data_helper_arg *arg)
{
    struct msg *m;
    m = inst->oldest;
    while (m && arg->result == D_STORE_RESULT_OK) {
        struct msg *mnext = m->newer;
        if (os_timeWCompare (m->writeTime, arg->delete_time) != OS_MORE) {
            arg->result = drop_just_message (arg->st, inst, m, 0);
        }
        m = mnext;
    }
    if (inst->oldest == NULL) {
        /* Instance is empty, but the table walk routine doesn't allow
         * for deleting data while walking the tree. So remember the
         * instance as one that has to be deleted after the walk
         * completes. */
        arg->instances_to_delete = c_iterInsert (arg->instances_to_delete, inst);
    }
    return (arg->result == D_STORE_RESULT_OK);
}


d_storeResult d_storeDeleteHistoricalDataKV (const d_store gstore, const v_groupAction msg)
{
    d_storeKV st;
    d_storeResult result;
    struct d_groupListKV_s *g;
    os_duration d;
    struct delete_historical_data_helper_arg arg;
    struct inst *inst;

    if (msg == NULL || msg->group == NULL) {
        return D_STORE_RESULT_ILL_PARAM;
    }

    LOCK_OPEN_STOREKV (st, gstore);
    if ((g = find_group_w_kernelgroup (st, msg->group)) == NULL) {
        result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        goto err_group;
    }
    arg.st = st;
    /* The actiontime indicates the deleteTime as elapsed time.
     * We need to calculate the corresponding wall clock time */
    d = os_timeEDiff(os_timeEGet(), msg->actionTime);
    arg.delete_time = os_timeWSub(os_timeWGet(), d);
    if (OS_TIMEW_ISINVALID(arg.delete_time)) {
        result = D_STORE_RESULT_ERROR;
        kvlog (st, D_LEVEL_SEVERE, "delete-historical-data: invalid time\n");
        goto err_delete_time;
    }
    arg.instances_to_delete = c_iterNew (NULL);
    arg.result = D_STORE_RESULT_OK;
    (void)d_tableWalk (g->instances, delete_historical_data_helper, &arg);
    if (arg.result != D_STORE_RESULT_OK) {
        kvlog (st, D_LEVEL_SEVERE, "delete-historical-data: failed to delete data\n");
    }
    while ((inst = (struct inst *) c_iterTakeFirst (arg.instances_to_delete)) != NULL) {
        assert (ut_avlLookup (&group_inst_td, &g->instances_by_iid, &inst->next_msg_key.iid) == inst);
        ut_avlDelete (&group_inst_td, &g->instances_by_iid, inst);
        d_tableRemove (g->instances, inst);
        inst_free (inst);
    }
    c_iterFree (arg.instances_to_delete);
    result = arg.result;
    if (result == D_STORE_RESULT_OK && kv_commit (st->kv) < 0) {
        kvlog (st, D_LEVEL_SEVERE, "delete-historical-data: commit failed\n");
        result = D_STORE_RESULT_IO_ERROR;
    }

err_delete_time:
err_group:
    UNLOCK_OPEN_STOREKV (st);
    return result;
}

d_storeResult d_storeInstanceRegisterKV (const d_store gstore, const v_groupAction msg)
{
    OS_UNUSED_ARG (gstore);
    OS_UNUSED_ARG (msg);
    return D_STORE_RESULT_OK;
}

d_storeResult d_storeInstanceUnregisterKV (const d_store gstore, const v_groupAction msg)
{
    OS_UNUSED_ARG (gstore);
    OS_UNUSED_ARG (msg);
    return D_STORE_RESULT_OK;
}

d_storeResult d_storeOptimizeGroupKV (const d_store gstore, const d_group group)
{
    OS_UNUSED_ARG (gstore);
    OS_UNUSED_ARG (group);
    return D_STORE_RESULT_OK;
}

d_storeResult d_storeNsIsCompleteKV (const d_store gstore, const d_nameSpace nameSpace, c_bool *isComplete)
{
    d_storeKV st;
    d_storeResult result;
    struct namespace *ns;

    if (nameSpace == NULL) {
        return D_STORE_RESULT_ILL_PARAM;
    }

    LOCK_OPEN_STOREKV (st, gstore);
    if ((ns = find_or_create_namespace (st, nameSpace)) == NULL) {
        kvlog (st, D_LEVEL_SEVERE, "is-complete: namespace %s unknown\n", d_nameSpaceGetName (nameSpace));
        result = D_STORE_RESULT_PRECONDITION_NOT_MET;
    } else {
        kv_result_t rc;
        void *obj;
        rc = get2_deser (st, TABLE_NAMESPACE_COMPLETENESS, ns->id, ns->version, st->namespace_completeness_type, &st->namespace_completeness_tctx, &obj);
        switch (rc) {
        case 0:
            *isComplete = ((struct d_namespaceCompletenessKV *) obj)->complete;
            kvlog (st, D_LEVEL_FINEST, "is-complete: namespace %s %scomplete\n", ns->name, *isComplete ? "" : "in");
            result = D_STORE_RESULT_OK;
            c_free (obj);
            break;
        case KV_RESULT_NODATA:
            kvlog (st, D_LEVEL_INFO, "is-complete: namespace %s defaulting to incomplete\n", ns->name);
            *isComplete = 0;
            result = D_STORE_RESULT_OK;
            break;
        default:
            *isComplete = 0;
            result = kv_convert_result(rc);
            break;
        }
    }

    UNLOCK_OPEN_STOREKV (st);
    if (result != D_STORE_RESULT_OK) {
        kvlog (st, D_LEVEL_WARNING, "is-complete: namespace %s get completeness failed\n", d_nameSpaceGetName (nameSpace));
    }
    return result;
}

d_storeResult d_storeNsMarkCompleteKV (const d_store gstore, const d_nameSpace nameSpace, c_bool isComplete)
{
    d_storeKV st;
    d_storeResult result;
    kv_result_t rc;
    struct namespace *ns;
    struct d_namespaceCompletenessKV *v;

    if (nameSpace == NULL) {
        return D_STORE_RESULT_ILL_PARAM;
    }

    LOCK_OPEN_STOREKV (st, gstore);
    if ((ns = find_or_create_namespace (st, nameSpace)) == NULL) {
        kvlog (st, D_LEVEL_SEVERE, "mark-complete: namespace %s unknown\n", d_nameSpaceGetName (nameSpace));
        result = D_STORE_RESULT_PRECONDITION_NOT_MET;
    } else if ((v = c_new (st->namespace_completeness_type)) == NULL) {
        result = D_STORE_RESULT_OUT_OF_RESOURCES;
    } else {
        v->complete = (isComplete != 0);
        if ((rc = put2_ser (st, TABLE_NAMESPACE_COMPLETENESS, ns->id, ns->version, st->namespace_completeness_type, &st->namespace_completeness_tctx, v)) < 0) {
            result = kv_convert_result(rc);
        } else {
            kvlog (st, D_LEVEL_FINEST, "mark-complete: namespace %s marked %scomplete\n", ns->name, isComplete ? "" : "in");
            if (kv_commit (st->kv) < 0) {
                kvlog (st, D_LEVEL_SEVERE, "mark-complete: commit failed\n");
                result = D_STORE_RESULT_IO_ERROR;
            } else {
                result = D_STORE_RESULT_OK;
            }
        }
        c_free (v);
    }

    UNLOCK_OPEN_STOREKV (st);
    if (result != D_STORE_RESULT_OK) {
        kvlog (st, D_LEVEL_WARNING, "mark-complete: store completeness failed, result=%d\n", result);
    }
    return result;
}

static int topic_definition_matches (d_storeKV st, v_group group, os_uint32 topic_id, const char *topic_name)
{
    const char *failmsg = NULL;
    os_uint32 disk_sz, shm_sz;
    void *disk, *shm;
    struct d_topicInfoKV *tpdisk, *tpshm;

    if ((tpdisk = get_topicInfoKV (st, topic_id, topic_name)) == NULL) {
        failmsg = "no topic definition found / out of memory (disk)"; goto fail_disk_topic;
    }
    if ((tpshm = make_topicInfoKV (st, v_groupTopic (group))) == NULL) {
        failmsg = "out of memory (shm)"; goto fail_shm_topic;
    }

    if ((disk = serialize_clearflag (&disk_sz, st, st->topicinfo_type, &st->topicinfo_tctx, tpdisk, 1)) == NULL) {
        failmsg = "out of memory (re-serialize disk)"; goto fail_ser_disk;
    }
    if ((shm = serialize_clearflag (&shm_sz, st, st->topicinfo_type, &st->topicinfo_tctx, tpshm, 1)) == NULL) {
        failmsg = "out of memory (serialize shm)"; goto fail_ser_shm;
    }
    /* ... and do a byte-wise comparison of the two. Explicitly
     * check the sizes and do a memcmp instead of a simple strcmp
     * to catch cases where the on-disk data has been
     * (deliberately?) mutilated. */
    if (disk_sz != shm_sz || memcmp (disk, shm, shm_sz) != 0) {
        failmsg = "mismatch between definitions"; goto fail_compare;
    }

fail_compare:
    os_free (shm);
fail_ser_shm:
    os_free (disk);
fail_ser_disk:
    c_free (tpshm);
fail_shm_topic:
    c_free (tpdisk);
fail_disk_topic:
    if (failmsg) {
        kvlog (st, D_LEVEL_SEVERE, "Topic %u (%s): verification: %s\n", topic_id, topic_name, failmsg);
    }
    return failmsg == NULL;
}

static d_storeResult reconstruct_instances (struct d_storeKV_s *st, struct d_groupListKV_s *g, struct v_group_s *group)
{
    struct c_type_s *mtype, *xtype = NULL /* shut up compiler */;
    d_storeResult result = D_STORE_RESULT_OK;
    kv_key_t min, max;
    kv_iter_t iter;
    int hasdata;
    struct transaction_list *tr_list;

    kvdebug ("reconstruct_instances ...\n");
    mtype = v_topicMessageType (v_groupTopic ((c_object)group));
    switch (st->encoding)
    {
    case D_ENCODINGKV_XML_XML:
    case D_ENCODINGKV_BIGE_XML:
    case D_ENCODINGKV_BIGECDR_XML:
        if ((xtype = v_messageExtTypeNew (v_groupTopic ((c_object)group))) == NULL) {
            kvlog (st, D_LEVEL_SEVERE, "out of resources (allocation of xmsg type)\n");
            return D_STORE_RESULT_OUT_OF_RESOURCES;
        }
        break;
    case D_ENCODINGKV_BIGECDR_V1:
        xtype = mtype;
        break;
    }

    min.table = TABLE_MESSAGE;
    min.gid = g->group_id;
    min.iid = 0;
    min.mid = 0;
    max.table = TABLE_MESSAGE;
    max.gid = g->group_id;
    max.iid = ~(os_uint32)0;
    max.mid = ~(os_uint64)0;
    kv_iter_new (st->kv, &iter, &min, &max, xtype);
    /* Store all messages that are present in the TABLE_MESSAGE table */
    kvdebug("  reading all messages\n");
    while (result == D_STORE_RESULT_OK && kv_iter_next (&hasdata, st->kv, iter) >= 0 && hasdata) {
        kv_key_t k;
        void *x_msg;
        kv_iter_getkey (st->kv, iter, &k);
        kvdebug ("  %u:%u:%llu\n", k.gid, k.iid, k.mid);
        if (iter_value_deser (st, iter, xtype, &g->tctx, &x_msg) < 0) {
            kvlog (st, D_LEVEL_SEVERE, "  message {%u:%u:%llu:%llu}: stored data is corrupt\n", (unsigned) k.table, k.gid, k.iid, k.mid);
            result = D_STORE_RESULT_MUTILATED;
        } else {
            void *v_msg = (xtype == mtype) ? x_msg : v_messageExtConvertFromExtType (mtype, x_msg);
            result = store_message (st, group, v_msg, g, &k.iid, &k.mid);
            c_free (v_msg);
        }
    }
    kvdebug ("  done\n");
    kv_iter_free (st->kv, iter);
    if (xtype != mtype) {
        v_messageExtTypeFree (xtype);
    }

    /* Iterate over all EOT messages and commit the transactions
     * that have finished */
    kvdebug ("Processing transaction records\n");
    min.table = TABLE_MESSAGE_EOT;
    min.gid = 0;
    min.iid = 0;
    min.mid = 0;
    max.table = TABLE_MESSAGE_EOT;
    max.gid = 0;
    max.iid = 0;
    max.mid = ~(os_uint64)0;
    kv_iter_new (st->kv, &iter, &min, &max, st->open_transaction_eot_type);
    while (result == D_STORE_RESULT_OK && kv_iter_next (&hasdata, st->kv, iter) >= 0 && hasdata) {
        kv_key_t k;

        kv_iter_getkey (st->kv, iter, &k);
        if (k.mid >= st->otid) {
            /* Set otid to next free number */
            st->otid = k.mid + 1;
        }
        kvdebug ("  process %s journal msg with key %"PA_PRIu32":%"PA_PRIu32":%"PA_PRIu64"\n", k.mid & 1 ? "'commit'" : "'begin'", k.gid, k.iid, k.mid);
        if (k.mid & 1) {
            /* The message is an EOT message, so the transaction
             * can be committed (C). If the commit succeeds the
             * entry is removed from the list of open_transactions. */
            void *v_msgEOT;

            if (iter_value_deser (st, iter, st->open_transaction_eot_type, &st->open_transaction_eot_tctx, &v_msgEOT) < 0) {
                kvlog (st, D_LEVEL_SEVERE, "MessageEOT {%u:%u:%llu:%llu}: stored data is corrupt\n", (unsigned) k.table, k.gid, k.iid, k.mid);
                result = D_STORE_RESULT_MUTILATED;
            } else {
                result = commit_group_transaction (st, v_msgEOT, 1);
                c_free (v_msgEOT);
            }
        } else {
            /* The message is the begin (B) of a transaction, in which
             * case the payload contains the v_tid. The transaction is considered
             * OPEN until it becomes COMMITTED */
            void *tid;

            if (iter_value_deser (st, iter, st->open_transaction_vtid_type, &st->open_transaction_vtid_tctx, &tid) < 0) {
                kvlog (st, D_LEVEL_SEVERE, "Message {%u:%u:%llu:%llu}: stored data is corrupt\n", (unsigned) k.table, k.gid, k.iid, k.mid);
                result = D_STORE_RESULT_MUTILATED;
            } else {
                struct v_tid *vtid = tid;
                struct transaction_list tr_list_template;
                tr_list_template.gid = vtid->wgid;
                tr_list_template.seqNum = vtid->seqnr;
                if ((tr_list = d_tableFind(st->open_transactions, &tr_list_template)) != NULL) {
                    tr_list->commit_state = OPEN;
                }
                c_free (tid);
            }
        }
    }
    /* Now walk over all open transactions that are not yet
     * yet committed */
    while ((result == D_STORE_RESULT_OK) && ((tr_list = d_tableTake (st->open_transactions)) != NULL)) {
        if (tr_list->commit_state != OPEN) {
            assert(tr_list->commit_state != COMMITTED);
            result = commit_writer_transaction (st, tr_list, 1);
        } else {
            /* The transaction is OPEN and no EOT has
             * been received. No EOT will come anymore
             * so we do not have to maintain the
             * transaction administration. */
            struct transaction_msg *msg;
            while ((msg = tr_list->head) != NULL) {
               tr_list->head = msg->next;
               os_free (msg);
            }
            os_free (tr_list);
        }
    }
    kvdebug ("  done\n");
    kv_iter_free (st->kv, iter);
    return result;
}

struct inject_messages_helper_arg {
    struct d_storeKV_s *st;
    struct v_group_s *group;
    d_storeResult result;
    c_bool store_msg;
};

struct pending_unregister {
    v_message protomsg;
    c_ulong sequenceNumber;
    os_timeW writeTime;
};

static int pending_unregister_cmp (const struct pending_unregister *a, const struct pending_unregister *b)
{
    c_equality eq;
    eq = v_gidCompare (a->protomsg->writerGID, b->protomsg->writerGID);
    if (eq == C_LT) {
        return -1;
    } else if (eq == C_GT) {
        return 1;
    }
    eq = v_gidCompare (a->protomsg->writerInstanceGID, b->protomsg->writerInstanceGID);
    if (eq == C_LT) {
        return -1;
    } else if (eq == C_GT) {
        return 1;
    }
    return 0;
}

static void pending_unregister_free (struct pending_unregister *a)
{
    kvdebug ("  pending_unregister_free %p\n", (void *) a);
    c_free (a->protomsg);
    os_free (a);
}

static d_storeResult schedule_unregister (d_table tab, const struct v_message_s *msg)
{
    struct pending_unregister tmp, *pu;
    tmp.protomsg = (v_message) msg;
    if ((pu = d_tableFind (tab, &tmp)) != NULL) {
        kvdebug ("  schedule_unregister: %d:%d:%d already scheduled\n", msg->writerGID.systemId, msg->writerGID.localId, msg->writerGID.serial);
        if (pu->sequenceNumber <= msg->sequenceNumber) {
            pu->sequenceNumber = msg->sequenceNumber + 1;
            kvdebug ("    updating seq num\n");
        }
        if (os_timeWCompare (pu->writeTime, msg->writeTime) == OS_LESS) {
            pu->writeTime = msg->writeTime;
            kvdebug ("    updating time\n");
        }
        return D_STORE_RESULT_OK;
    } else {
        pu = os_malloc (sizeof (*pu));
        pu->protomsg = c_keep (v_message ((c_object)msg));
        pu->sequenceNumber = msg->sequenceNumber + 1;
        pu->writeTime = msg->writeTime;
        (void) d_tableInsert (tab, pu);
        kvdebug ("  schedule_unregister: %d:%d:%d new\n", msg->writerGID.systemId, msg->writerGID.localId, msg->writerGID.serial);
        return D_STORE_RESULT_OK;
    }
}

static c_bool inject_unregister_helper (struct pending_unregister *pu, struct inject_messages_helper_arg *arg)
{
    /* Create unregister message for given writer id, sequence number
     * and time stamp, and inject it. */
    const os_duration poll_intv = OS_DURATION_INIT(1,0);
    const os_duration inc = OS_DURATION_INIT(0, 100);
    c_base base = c_getBase(arg->group);
    c_array keys;
    c_ulong i, nkeys;
    v_message msg;
    v_writeResult wr;

    msg = v_topicMessageNew (v_groupTopic (arg->group));
    keys = v_topicMessageKeyList (v_groupTopic (arg->group));
    nkeys = c_arraySize (keys);
    for (i = 0; i < nkeys; i++) {
        c_fieldAssign (keys[i], msg, c_fieldValue (keys[i], pu->protomsg));
    }
    msg->writerGID = pu->protomsg->writerGID;
    msg->writerInstanceGID = pu->protomsg->writerInstanceGID;
    msg->qos = c_keep ((v_qos) pu->protomsg->qos);
    msg->writeTime = os_timeWAdd(pu->writeTime, inc);
    msg->sequenceNumber = pu->sequenceNumber;
    v_nodeState (msg) = L_UNREGISTER;
    if (v_messageStateTest (pu->protomsg, L_TRANSACTION)) {
        v_stateSet (v_nodeState (msg), L_TRANSACTION);
        msg->transactionId = pu->protomsg->transactionId;
    }

    kvdebug ("  inject_unregister_helper: %d:%d:%d state=%d\n", msg->writerGID.systemId, msg->writerGID.localId, msg->writerGID.serial, msg->_parent.nodeState);

    do {
        if (c_baseMakeMemReservation(base, C_MM_RESERVATION_ZERO)) {
            wr = v_groupWriteNoStream (arg->group, msg, NULL, V_NETWORKID_ANY);
            c_baseReleaseMemReservation(base, C_MM_RESERVATION_ZERO);
            if (wr == V_WRITE_REJECTED) {
                ospl_os_sleep (poll_intv);
            }
        } else {
            wr = V_WRITE_OUT_OF_RESOURCES;
        }
    } while (wr == V_WRITE_REJECTED);

    if (wr != V_WRITE_SUCCESS && wr != V_WRITE_REGISTERED && wr != V_WRITE_UNREGISTERED) {
        kvlog (arg->st, D_LEVEL_SEVERE, "Unable to write unregister messaged to group. (result: '%d')\n", wr);
        arg->result = D_STORE_RESULT_ERROR;
    } else {
        arg->result = D_STORE_RESULT_OK;
    }

    c_free (msg);
    return (arg->result == D_STORE_RESULT_OK);
}


static c_bool inject_messages_helper (const struct inst *inst, struct inject_messages_helper_arg *arg)
{
    d_table pending_unregisters;
    struct msg *m;
    struct d_groupListKV_s *g = NULL;
    c_base base = c_getBase(arg->group);

    kvdebug ("  inst %u\n", inst->next_msg_key.iid);
    /* Find group when messages needs to be added to store, group
     * should already exist. */
    if ((arg->store_msg == TRUE) &&
        (g = find_group_w_kernelgroup (arg->st, arg->group)) == NULL) {
        kvlog (arg->st, D_LEVEL_SEVERE, "Unable to find group while it existed earlier.\n");
        arg->result = D_STORE_RESULT_ERROR;
        return FALSE;
    }

    pending_unregisters = d_tableNew (pending_unregister_cmp, pending_unregister_free);
    for (m = inst->oldest; m; m = m->newer) {
        const os_duration poll_intv = OS_DURATION_INIT(1,0);
        v_writeResult wr;

        assert (m->v_msg != NULL);
        kvdebug ("    msg %llu\n", m->key.mid);
        kvdebug ("  inject_messages_helper: %d:%d:%d state=%d\n", m->v_msg->writerGID.systemId, m->v_msg->writerGID.localId, m->v_msg->writerGID.serial, m->v_msg->_parent.nodeState);
        do {
            if (c_baseMakeMemReservation(base, C_MM_RESERVATION_ZERO)) {
                wr = v_groupWriteNoStream (arg->group, m->v_msg, NULL, V_NETWORKID_ANY);
                c_baseReleaseMemReservation(base, C_MM_RESERVATION_ZERO);
                if (wr == V_WRITE_REJECTED) {
                    ospl_os_sleep (poll_intv);
                }
            } else {
                wr = V_WRITE_OUT_OF_RESOURCES;
            }
        } while (wr == V_WRITE_REJECTED);

        if (wr != V_WRITE_SUCCESS && wr != V_WRITE_REGISTERED && wr != V_WRITE_UNREGISTERED) {
            kvlog (arg->st, D_LEVEL_SEVERE, "Unable to write persistent data to group. (result: '%d')\n", wr);
            arg->result = D_STORE_RESULT_ERROR;
        } else {
            if (g) {
                d_storeResult result;
                kvdebug ("  inject_messages_helper: inject loaded message in KV store\n");
                result = store_message (arg->st, arg->group, m->v_msg, g, NULL, NULL);
                if (result != D_STORE_RESULT_OK) {
                    kvlog (arg->st, D_LEVEL_SEVERE, "Unable to inject loaded message in KV store. (result: '%d')\n", result);
                    arg->result = D_STORE_RESULT_ERROR;
                }
            }
            arg->result = schedule_unregister (pending_unregisters, m->v_msg);
        }

        c_free (m->v_msg);
        m->v_msg = NULL;
    }

    if (arg->result == D_STORE_RESULT_OK) {
        (void)d_tableWalk (pending_unregisters, inject_unregister_helper, arg);
    }
    d_tableFree (pending_unregisters);
    return (arg->result == D_STORE_RESULT_OK);
}


static d_storeResult inject_messages (struct d_storeKV_s *st, const struct d_groupListKV_s *g, struct v_group_s *group, c_bool store_msg)
{
    struct inject_messages_helper_arg arg;
    kvdebug ("injecting messages ...\n");
    arg.st = st;
    arg.group = group;
    arg.result = D_STORE_RESULT_OK;
    arg.store_msg = store_msg;
    (void)d_tableWalk (g->instances, inject_messages_helper, &arg);
    if (arg.result == D_STORE_RESULT_OK) {
        kvlog(st, D_LEVEL_FINEST, "injected %d instance(s) for group '%s.%s'\n", d_tableSize(g->instances), g->_parent.partition, g->_parent.topic);
    }
    kvdebug ("  done\n");
    return arg.result;
}

d_storeResult d_storeMessagesInjectKV (const d_store gstore, const d_group dgroup)
{
    d_storeKV st;
    d_storeResult result = D_STORE_RESULT_OK;
    c_char *topicName;
    c_char *partitionName;
    v_group group;
    os_uint32 topic_id;
    d_groupListKV g;

    if (gstore == NULL || dgroup == NULL) {
        return D_STORE_RESULT_ILL_PARAM;
    }

    LOCK_OPEN_STOREKV (st, gstore);

    group = d_groupGetKernelGroup (dgroup);
    topicName = v_topicName (v_groupTopic (group));
    partitionName = v_partitionName (v_groupPartition (group));
    kvlog (st, D_LEVEL_FINE, "inject-messages group '%s.%s'\n", partitionName, topicName);

    /* We need the topic_id, and the easiest way to find that is to
     * enumerate the in-memory representation of the groups. Those get
     * initialised from the disk as soon as the store is opened, and
     * so if we can't find it there, it doesn't exist on disk.
     *
     * I think it would be slightly odd to get a request for injecting
     * data for a group we know nothing about, but it is
     * conceivable. Since we know there is no data for this group on
     * disk, just say we injected all we had. */
    if ((g = find_group (st, partitionName, topicName)) == NULL) {
        kvlog (st, D_LEVEL_WARNING, "inject messages: group '%s.%s' unknown (guaranteed no data)\n", partitionName, topicName);
        result = D_STORE_RESULT_OK;
        goto out;
    }
    topic_id = g->topic_id;
    kvdebug ("  topic_id = %u\n", topic_id);

    /* If we know of the group, we must still verify that the topic
     * definitions in the kernel and on-disk match: there is no
     * guarantee some other process hasn't already defined the topic
     * in an incompatible way. */
    if (!topic_definition_matches (st, group, topic_id, topicName)) {
        kvlog (st, D_LEVEL_WARNING, "inject messages: topic '%s' definitions do not match, not injecting\n", topicName);
        result = D_STORE_RESULT_METADATA_MISMATCH;
        goto out;
    }
    kvdebug ("  topic definitions match\n");

    /* Store the kernel group in the d_groupListKV. This makes lookup
     * of the kernel group very easy. */
    g->vgroup = group;

    /* Two options for injecting data: first is to reconstruct the
     * instances in-memory, then regenerate the messages that matter
     * (there could be crash just before dropping some message,
     * leading to too much persistent data); the other is to simply
     * regenerate everything and accepting the potential publication
     * of a few messages more than we should.
     *
     * The first also gives us the option of re-using the data stored
     * on disk. For a first version, it is the nicer choice. */
    reconstruct_instances (st, g, group);

    /* Once we have everything in-memory start injecting */
    result = inject_messages (st, g, group, FALSE);

    kvdebug ("  done\n");
out:
    UNLOCK_OPEN_STOREKV (st);
    return result;
}

static int topic_referenced_in_groups_p (const struct d_groupListKV_s *gs, os_uint32 topic_id)
{
    /* FIXME: stupid way of finding unreferenced topics */
    const struct d_groupListKV_s *g;
    for (g = gs; g; g = (const struct d_groupListKV_s *) g->_parent.next) {
        if (g->topic_id == topic_id) {
            return 1;
        }
    }
    return 0;
}

static d_storeResult delete_namespace_version (struct d_storeKV_s *st, struct namespace *ns)
{
    /* Near all failures to delete data are but a warning, because the
     * first delete ensures the namespace version is no longer visible
     * when opening the store. */
    kv_key_t key;
    struct d_groupListKV_s *gs = NULL;

    kvlog (st, D_LEVEL_INFO, "delete-namespace-version: namespace %s id %u/%u\n", ns->name, ns->id, ns->version);

    /* Delete namespace quality, completeness from disk store: that
     * way this version will be considered incomplete and of minimal
     * quality if we crash during this operation. */
    key.gid = ns->id; key.iid = ns->version; key.mid = 0;
    key.table = TABLE_NAMESPACE;
    if (kv_delete (st->kv, &key) < 0) {
        kvlog (st, D_LEVEL_SEVERE, "delete-namespace-version: delete namespace failed\n");
        return D_STORE_RESULT_ERROR;
    }
    key.table = TABLE_NAMESPACE_COMPLETENESS;
    if (kv_delete (st->kv, &key) < 0) {
        kvlog (st, D_LEVEL_WARNING, "delete-namespace-version: delete namespace completeness failed\n");
    }
    key.table = TABLE_NAMESPACE_QUALITY;
    if (kv_delete (st->kv, &key) < 0) {
        kvlog (st, D_LEVEL_WARNING, "delete-namespace-version: delete namespace quality failed\n");
    }

    /* Remove namespace from list of namespaces */
    {
        struct namespace **pn = &st->namespaces;
        while (*pn != ns) {
            pn = &(*pn)->next;
        }
        *pn = ns->next;
    }
    /* Remove groups referring to namespace NS from list of groups,
     * transferring them to GS */
    {
        struct d_groupListKV_s **pg = &st->groups, *g = *pg;
        while (g) {
            if (g->namespace == ns) {
                *pg = g->_parent.next;
                g->_parent.next = gs;
                gs = g;
            } else {
                pg = (struct d_groupListKV_s **) &g->_parent.next;
            }
            g = *pg;
        }
    }

    /* Delete messages, group definitions, topic definitions from the
     * on-disk representation */
    {
        const struct d_groupListKV_s *g;
        for (g = gs; g; g = (struct d_groupListKV_s *) g->_parent.next) {
            kv_key_t first, last;
            kvlog (st, D_LEVEL_INFO, "delete-namespace-version: deleting messages group %s.%s (%u)\n",
                   g->_parent.partition, g->_parent.topic, g->group_id);
            first.table = last.table = TABLE_MESSAGE;
            first.gid = last.gid = g->group_id;
            first.iid = 0; last.iid = ~0u;
            first.mid = 0; last.mid = ~(os_uint64) 0;
            if (kv_bulkdelete (st->kv, &first, &last) < 0) {
                kvlog (st, D_LEVEL_WARNING, "delete-namespace-version: delete messages group %s.%s (%u) failed\n",
                       g->_parent.partition, g->_parent.topic, g->group_id);
            }

            kvlog (st, D_LEVEL_INFO, "delete-namespace-version: deleting group %s.%s (%u)\n",
                   g->_parent.partition, g->_parent.topic, g->group_id);
            key.table = TABLE_GROUP; key.gid = g->group_id; key.iid = 0; key.mid = 0;
            if (kv_delete (st->kv, &key) < 0) {
                kvlog (st, D_LEVEL_WARNING, "delete-namespace-version: delete group %s.%s (%u) failed\n",
                       g->_parent.partition, g->_parent.topic, g->group_id);
            }
        }

        for (g = gs; g; g = (struct d_groupListKV_s *) g->_parent.next) {
            /* Second term is only for pretty printing: without it,
             * the same topic can be deleted multiple times (where
             * only the first time has effect) */
            if (!topic_referenced_in_groups_p (st->groups, g->topic_id) &&
                !topic_referenced_in_groups_p ((struct d_groupListKV_s *) g->_parent.next, g->topic_id)) {
                kvlog (st, D_LEVEL_INFO, "delete-namespace-version: topic %s (%u) no longer needed\n",
                       g->_parent.topic, g->topic_id);
                key.gid = g->topic_id; key.iid = 0; key.mid = 0;
                key.table = TABLE_TOPIC;
                if (kv_delete (st->kv, &key) < 0) {
                    kvlog (st, D_LEVEL_WARNING, "delete-namespace-version: delete topic %s (%u) failed\n",
                           g->_parent.topic, g->topic_id);
                }
                key.table = TABLE_TOPIC_TYPE;
                if (kv_delete (st->kv, &key) < 0) {
                    kvlog (st, D_LEVEL_WARNING, "delete-namespace-version: delete topic %s (%u) failed\n",
                           g->_parent.topic, g->topic_id);
                }
            }
        }
    }

    /* Free memory -- need not be done this late. */
    while (gs) {
        struct d_groupListKV_s *g = gs;
        gs = (struct d_groupListKV_s *) gs->_parent.next;
        free_group (g, st);
    }
    free_namespace (ns);
    kvlog (st, D_LEVEL_INFO, "delete-namespace-version: delete namespace done\n");
    return D_STORE_RESULT_OK;
}

static struct namespace *find_older_namespace_version (const struct namespace *ns)
{
    struct namespace *ns0;
    for (ns0 = ns->next; ns0; ns0 = ns0->next) {
        if (ns0->id == ns->id) {
            return ns0;
        }
    }
    return NULL;
}

d_storeResult d_storeBackupKV (const d_store gstore, const d_nameSpace nameSpace)
{
    /* ASSUMPTION:
     *
     * Backup(NS) is only called by durability when it will NOT be
     * injecting data for namespace NS, but regardless of its being
     * complete.  It then calls d_storeGroupStore and
     * d_storeMessageStore to store the new groups and the data in
     * those groups.  After that, it expects namespace NS to be empty.
     *
     * All we need to do to generate the backup and to start afresh is
     * generate a new version of the matching namespace.  The old
     * group definition is then no longer accessible because it is of
     * the wrong namespace version, therefore GroupStore(G) will not
     * re-use it, therefore a new group id gets allocated.  Since
     * topics are (currently anyway) still stored in memory accessible
     * only via groups, this will then also force the creation of new
     * topics. */
    d_storeKV st;
    d_storeResult result;
    struct namespace *ns, *ns1;

    if (nameSpace == NULL) {
        return D_STORE_RESULT_ILL_PARAM;
    }

    LOCK_OPEN_STOREKV (st, gstore);
    if ((ns = find_namespace (st, nameSpace)) == NULL) {
        kvlog (st, D_LEVEL_INFO, "store-backup: namespace %s unknown\n", d_nameSpaceGetName (nameSpace));
        result = D_STORE_RESULT_OK;
    } else if ((ns1 = new_namespace_version (st, ns)) == NULL) {
        kvlog (st, D_LEVEL_SEVERE, "store-backup: backup of namespace %s failed: out of memory\n", d_nameSpaceGetName (nameSpace));
        result = D_STORE_RESULT_PRECONDITION_NOT_MET;
    } else {
        kvlog (st, D_LEVEL_FINEST, "store-backup: namespace %s new id %u/%u\n", ns1->name, ns1->id, ns1->version);
        if ((result = store_namespace (st, ns1)) != D_STORE_RESULT_OK) {
            /* failed to store namespace, do not delete anything;
             * error already logged by store_namespace() */
        } else {
            /* see if we have a backups preceding NS, and if so, delete them */
            struct namespace *ns0;
            while (result == D_STORE_RESULT_OK && (ns0 = find_older_namespace_version (ns)) != NULL) {
                result = delete_namespace_version (st, ns0);
            }
            if (result == D_STORE_RESULT_OK) {
                kvlog (st, D_LEVEL_INFO, "store-backup: no older backups\n");
            }
        }
    }
    UNLOCK_OPEN_STOREKV (st);
    return result;
}

d_storeResult d_storeRestoreBackupKV (const d_store gstore, const d_nameSpace nameSpace)
{
    /* ASSUMPTION:
     *
     * RestoreBackup(NS) is only called by durability when it faces an
     * incomplete persistent store.  The XML store leaves the primary
     * copy unchanged if there is no backup, but does return an error.
     *
     * To restore a backup, all we need to do is drop the latest
     * version of nameSpace, provided there is an older version (that
     * condition ensures we retain what we have if there is no
     * backup). */
    d_storeKV st;
    d_storeResult result;
    struct namespace *ns;

    if (nameSpace == NULL) {
        return D_STORE_RESULT_ILL_PARAM;
    }

    LOCK_OPEN_STOREKV (st, gstore);
    if ((ns = find_namespace (st, nameSpace)) == NULL) {
        kvlog (st, D_LEVEL_INFO, "restore-backup: namespace %s unknown\n", d_nameSpaceGetName (nameSpace));
        result = D_STORE_RESULT_OK;
    } else {
        if (find_older_namespace_version (ns) == NULL) {
            if (!ns->on_disk) {
                result = D_STORE_RESULT_OK;
            } else {
                kvlog (st, D_LEVEL_WARNING, "restore-backup: no backup available, keeping current data\n");
                result = D_STORE_RESULT_IO_ERROR;
            }
        } else {
            result = delete_namespace_version (st, ns);
        }
    }
    UNLOCK_OPEN_STOREKV (st);
    return result;
}


/**
 * \brief Store and commit journal entries for the EOT message
 *
 * Only EOT messages for which open transactions exist are committed.
 */
static d_storeResult journal_message_EOT (struct d_storeKV_s *st, const struct v_messageEOT_s *msg, os_uint64 *commit_mid)
{
    struct transaction_list tr_list_template, *tr_list;
    c_ulong i;
    kv_result_t rc;
    kv_key_t key;
    c_bool toStore = FALSE;

    /* As the key of the EOT message we use 64-bits.
     * The first 63 MSB bits will be used as a sequence number.
     * This LSB-bit will be used to indicate whether
     * message processing is busy (0) or has completed (1).
     * This will give in total 2^63 possible open transactions
     * to be stored. */

    kvdebug ("journal_message_EOT: vmsg %p\n", (void *) msg);

    /* Iterate over the list of writerGIDs in the EOT message,
     * and commit all open transaction messages that match
     * the writerGID and transactionId.  */
    for (i=0; i < c_arraySize(msg->tidList); i++) {
        struct v_tid *vtid;

        /* Find the transaction list */
        vtid = &(((struct v_tid *)msg->tidList)[i]);
        tr_list_template.gid = vtid->wgid;
        tr_list_template.seqNum = vtid->seqnr;
        tr_list_template.head = NULL;
        tr_list_template.tail = NULL;
        if ((tr_list = d_tableFind(st->open_transactions, &tr_list_template)) != NULL) {
             /* The EOT message contains an open transaction for which
              * messages are present in my administration. Store the EOT
              * message in the MESSAGE_EOT table to indicate that the EOT
              * has been received. */
            toStore = TRUE;
            break;
        }
    }
    if (!toStore) {
        *commit_mid = 0; /* can't ever be 0 for a real one */
    } else {
        key.table = TABLE_MESSAGE_EOT;
        key.gid   = 0;
        key.iid   = 0;
        key.mid   = (st->otid % 2) == 0 ? st->otid+1 : st->otid;  /* unique id, 63-bits transactionId, always odd for eot message */
        st->otid = key.mid + 1;
        *commit_mid = key.mid;
        if ((rc = put_ser (st, &key, st->open_transaction_eot_type, &st->open_transaction_eot_tctx, msg)) < 0) {
            return kv_convert_result(rc);
        }

        kvdebug("  adding eot %p to journal, key = %u\n", (void *)msg, key.mid);

        /* Commit the updates associated with the transaction */
        if (kv_commit (st->kv) < 0) {
            kvlog (st, D_LEVEL_SEVERE, "journal_message_EOT: commit failed\n");
            return D_STORE_RESULT_IO_ERROR;
        }
        kvdebug("  add eot msg to journal with key=%d:%d:%d\n", key.gid, key.iid, key.mid);


    }
    return D_STORE_RESULT_OK;
}

static struct msg *get_oldest_non_transaction (struct msg *a, struct msg *b, v_orderbyKind order_by)
{
    /* A must be a committed write, hence have L_WRITE set and L_TRANSACTION clear; B must be same if provided */
    assert((a->state & L_WRITE) && !(a->state & L_TRANSACTION));
    assert((b == NULL) || ((b->state & L_WRITE) && !(b->state & L_TRANSACTION)));
    if (b == NULL) {
        return a;
    } else {
        switch (order_by) {
            case V_ORDERBY_RECEPTIONTIME:
                /* message ids (mid) are monotonically increasing (per instance) over the lifetime of persistent store */
                return (a->key.mid < b->key.mid) ? a : b;
            case V_ORDERBY_SOURCETIME:
                return (msg_cmp(a, b) < 0) ? a : b;
        }
        assert(FALSE);
        return a;
    }
}

static d_storeResult commit_writer_transaction_message (struct d_storeKV_s *st, const struct transaction_msg *tr_msg, int reconstructing)
{
    struct v_topicQos_s *topicQos;
    v_orderbyKind order_by;
    os_int32 depth;
    struct inst *inst;
    d_storeResult result;
    struct msg *oldest_non_transaction;


    assert(tr_msg->msg_ref);
    inst = tr_msg->msg_ref->inst;
    /* Verify that the L_TRANSACTION flag is set. */
    assert(tr_msg->msg_ref->state & L_TRANSACTION);

    topicQos = v_topicQosRef (v_groupTopic (inst->g->vgroup));
    depth = topicQos->durabilityService.v.history_depth;
    order_by = topicQos->orderby.v.kind;
    /* Reset the L_TRANSACTION flag and update the writeCount */
    tr_msg->msg_ref->state = tr_msg->msg_ref->state & ~L_TRANSACTION;
    if (reconstructing) {
        tr_msg->msg_ref->v_msg->_parent.nodeState &= ~L_TRANSACTION;
    } else {
        assert (tr_msg->msg_ref->v_msg == NULL);
    }

    kvdebug("    msg %p state %x\n", (void *)tr_msg->msg_ref, tr_msg->msg_ref->state);

    if (tr_msg->msg_ref->state & L_WRITE) {
        inst->writeCount++;
        oldest_non_transaction = inst->oldest_non_transaction;
        /* Maintain the oldest_non_transaction */
        inst->oldest_non_transaction = get_oldest_non_transaction(tr_msg->msg_ref, inst->oldest_non_transaction, order_by);
        if (oldest_non_transaction != inst->oldest_non_transaction) {
            kvdebug("    oldest_non_transaction set to %p\n", (void *)inst->oldest_non_transaction);
        }
    }
    /* Check the history depth and push out the oldest_non_transaction
     * if it does not fit */
    if (history_is_keeplast(topicQos)) {
        if ((result = check_and_update_history(st, inst, depth, reconstructing)) != D_STORE_RESULT_OK) {
            return result;
        }
    }

    return D_STORE_RESULT_OK;
}

static d_storeResult commit_writer_transaction (struct d_storeKV_s *st, struct transaction_list *tr_list, int reconstructing)
{
   d_storeResult result = D_STORE_RESULT_OK;

   kvdebug("  commit_writer_transaction\n");

    while ((tr_list->head != NULL) && (result == D_STORE_RESULT_OK)) {
        struct transaction_msg *tr_msg = tr_list->head;
        tr_list->head = tr_msg->next;
        result = commit_writer_transaction_message (st, tr_msg, reconstructing);
        os_free (tr_msg);
    }
    if (result == D_STORE_RESULT_OK && !reconstructing)
    {
        /* All messages that belong to the writer and transaction have been committed.
         * Now it is safe to throw away the entry. */
        kv_key_t k;

        tr_list->commit_state = COMMITTED;
        k.table = TABLE_MESSAGE_EOT;
        k.gid = 0;
        k.iid = 0;
        k.mid = tr_list->otid;
        if (kv_delete (st->kv, &k) < 0) {
            kvlog (st, D_LEVEL_SEVERE, "Failed to delete message {%u:%u:%llu:%llu}\n", k.table, k.gid, k.iid, k.mid);
        }
    }
    os_free(tr_list);
    return result;
}

/**
 * \brief Insert the transaction messages that belong to the group transaction
 *        as "normal" messages.
 *
 * A side-effect of this operation is that the L_TRANSACTION flag
 * of the message that belongs to the transaction is cleared, and
 * samples may be pushed out of the history.
 */
static d_storeResult commit_group_transaction (struct d_storeKV_s *st, const struct v_messageEOT_s *msg, int reconstructing)
{
    struct transaction_list tr_list_template, *tr_list;
    const struct v_tid *tidlist = (const struct v_tid *)msg->tidList;
    c_ulong i;
    d_storeResult result = D_STORE_RESULT_OK;

    kvdebug ("commit_group_transaction: vmsg %p\n", (void *) msg);

    for (i=0; i < c_arraySize(msg->tidList) && (result == D_STORE_RESULT_OK); i++) {
        /* Find the transaction list */
        tr_list_template.gid = tidlist[i].wgid;
        tr_list_template.seqNum = tidlist[i].seqnr;
        if ((tr_list = d_tableRemove(st->open_transactions, &tr_list_template)) == NULL) {
            kvdebug ("  no transaction found, skip\n");
        } else {
            result = commit_writer_transaction (st, tr_list, reconstructing);
        }
    } /* for */
    return result;
}

static d_storeResult unjournal_message_EOT (struct d_storeKV_s *st, os_uint64 commit_mid)
{
    /* TODO: drop all completed transaction entries in the list of open transactions. */

    /* All transactions related to this EOT message have been
     * handled, now delete the EOT message from the MESSAGE_EOT
     * table to indicate that the message has been handled */

    kv_key_t key;

    key.table = TABLE_MESSAGE_EOT;
    key.gid   = 0;
    key.iid   = 0;
    key.mid   = commit_mid;
    /* delete the open transaction */
    if (kv_delete (st->kv, &key) < 0) {
        kvlog (st, D_LEVEL_SEVERE, "Failed to delete message {%u:%u:%llu:%llu}\n", key.table, key.gid, key.iid, key.mid);
    }

    return D_STORE_RESULT_OK;
}


d_storeResult d_storeTransactionCompleteKV (const d_store gstore, const v_groupAction msg)
{
    d_storeKV st;
    d_storeResult result;
    os_uint64 commit_mid;

    if (msg == NULL || msg->group == NULL || msg->message == NULL) {
        return D_STORE_RESULT_ILL_PARAM;
    }

    /* The message must have the L_ENDOFTRANSACTION flag */
    assert (v_nodeState (msg->message) & L_ENDOFTRANSACTION);

    LOCK_OPEN_STOREKV (st, gstore);

    if (kvlog_statistics && st->action_started) {
        st->first_time = os_timeEGet();
        st->action_started = FALSE;
    }

    if ((result = journal_message_EOT (st, (const struct v_messageEOT_s *)msg->message, &commit_mid)) != D_STORE_RESULT_OK) {
       /* skip */
    } else if (commit_mid) {
        if ((result = commit_group_transaction (st, (const struct v_messageEOT_s *)msg->message, 0)) != D_STORE_RESULT_OK) {
           /* skip */
        } else if ((result = unjournal_message_EOT(st, commit_mid)) != D_STORE_RESULT_OK) {
           /* skip */
        }
    }

    if (kvlog_statistics) {
        st->last_time = os_timeEGet();
    }

    UNLOCK_OPEN_STOREKV (st);
    return result;
}



/*****************************************************************************
 *
 *         SNAPSHOT FUNCTIONS
 *
 *****************************************************************************/

static d_storeResult snapshot_copy_version(const d_storeKV st, kv_store_t snapshot)
{
    d_storeResult result = D_STORE_RESULT_OK;
    const kv_key_t k = { TABLE_VERSION, 0, 0, 0 };
    kv_result_t rc;
    os_uint32 sz;
    void *buf;

    if ((rc = kv_get (st->kv, &k, &sz, &buf, 0)) >= 0) {
        if (sz != 2 * sizeof (os_uint32)) {
            kvlog (st, D_LEVEL_SEVERE, "snapshot: version from orig file corrupt\n");
            result = D_STORE_RESULT_MUTILATED;
        } else {
             rc = kv_put (snapshot, &k, sz, buf, 0);
             if (rc >= 0) {
                 return D_STORE_RESULT_OK;
             } else {
                 kvlog (st, D_LEVEL_SEVERE, "snapshot: write of version failed\n");
                 return D_STORE_RESULT_ERROR;
             }
        }
        os_free (buf);
    } else {
        kvlog (st, D_LEVEL_SEVERE, "snapshot: read of version failed\n");
        return D_STORE_RESULT_ERROR;
    }

    return result;
}

static d_storeResult snapshot_copy_topic_info(d_storeKV st, kv_store_t snapshot, unsigned topic_id, const char *topic_name)
{
    kv_result_t rc;
    kv_key_t key;
    void *blob;
    os_uint32 sz;

    assert(snapshot);

    key.table = TABLE_TOPIC;
    key.gid   = topic_id;
    key.iid   = 0;
    key.mid   = 0;

    {
        c_type type = NULL;
        switch (st->encoding) {
            case D_ENCODINGKV_XML_XML:
            case D_ENCODINGKV_BIGE_XML:
            case D_ENCODINGKV_BIGECDR_XML:
                type = st->topic_type;
                break;
            case D_ENCODINGKV_BIGECDR_V1:
                type = st->topicinfo_type;
                break;
        }
        if ((rc = kv_get (st->kv, &key, &sz, &blob, type)) < 0) {
            kvlog (st, D_LEVEL_SEVERE, "Retrieval of topic %u (%s) failed\n", topic_id, topic_name);
            return kv_convert_result(rc);
        } else if ((rc = kv_put (snapshot, &key, sz, blob, type)) < 0) {
            os_free(blob);
            kvlog (st, D_LEVEL_SEVERE, "Storage of topic %u (%s) failed\n", topic_id, topic_name);
            return kv_convert_result(rc);
        }
        os_free(blob);
    }

    switch (st->encoding) {
        case D_ENCODINGKV_XML_XML:
        case D_ENCODINGKV_BIGE_XML:
        case D_ENCODINGKV_BIGECDR_XML:
            key.table = TABLE_TOPIC_TYPE;
            if ((rc = kv_get (st->kv, &key, &sz, &blob, c_string_t (st->base))) < 0) {
                kvlog (st, D_LEVEL_SEVERE, "Retrieval of topic type %u (%s) failed\n", topic_id, topic_name);
                return kv_convert_result(rc);
            } else if ((rc = kv_put (snapshot, &key, sz, blob, c_string_t (st->base))) < 0) {
                os_free(blob);
                kvlog (st, D_LEVEL_SEVERE, "Storage of topic type %u (%s) failed\n", topic_id, topic_name);
                return kv_convert_result(rc);
            }
            os_free(blob);
            break;
        case D_ENCODINGKV_BIGECDR_V1:
            /* TABLE_TOPIC_TYPE eliminated in this encoding */
            break;
    }

    kvlog (st, D_LEVEL_FINEST, "Snapshot copied info related to topic '%s'\n", topic_name);

    return D_STORE_RESULT_OK;
}

static d_storeResult snapshot_copy_group_info(d_storeKV st, kv_store_t snapshot, const struct d_groupListKV_s *g)
{
    kv_result_t rc;
    kv_key_t key;
    void *blob;
    os_uint32 sz;

    assert(snapshot);

    key.table = TABLE_GROUP;
    key.gid   = g->group_id;
    key.iid   = 0;
    key.mid   = 0;

    if ((rc = kv_get (st->kv, &key, &sz, &blob, st->group_type)) < 0) {
        kvlog (st, D_LEVEL_SEVERE, "Retrieval of group %u (%s.%s) failed\n", g->group_id, g->_parent.partition,  g->_parent.topic);
        return kv_convert_result(rc);
    } else if ((rc = kv_put (snapshot, &key, sz, blob, st->group_type)) < 0) {
        os_free(blob);
        kvlog (st, D_LEVEL_SEVERE, "Storage of group %u (%s.%s) failed\n", g->_parent.partition,  g->_parent.topic);
        return kv_convert_result(rc);
    }
    os_free(blob);

    kvlog (st, D_LEVEL_FINEST, "Snapshot copied info related to group '%s.%s'\n", g->_parent.partition,  g->_parent.topic);

    return D_STORE_RESULT_OK;
}

static d_storeResult snapshot_copy_message_info(d_storeKV st, kv_store_t snapshot, const struct d_groupListKV_s *g)
{
    d_storeResult result = D_STORE_RESULT_OK;
    kv_result_t rc;
    kv_key_t min, max;
    kv_iter_t iter;
    int hasdata;

    min.table = TABLE_MESSAGE;
    min.gid = g->group_id;
    min.iid = 0;
    min.mid = 0;
    max.table = TABLE_MESSAGE;
    max.gid = g->group_id;
    max.iid = ~(os_uint32)0;
    max.mid = ~(os_uint64)0;

    kv_iter_new (st->kv, &iter, &min, &max, 0);
    while (result == D_STORE_RESULT_OK && kv_iter_next (&hasdata, st->kv, iter) >= 0 && hasdata) {
        kv_key_t k;
        os_uint32 sz;
        void *val;

        kv_iter_getkey (st->kv, iter, &k);
        kvdebug ("  %u:%u:%llu\n", k.gid, k.iid, k.mid);

        if (kv_iter_getvalue (st->kv, iter, &sz, &val) < 0) {
            kvlog (st, D_LEVEL_SEVERE, "Message {%u:%u:%llu:%llu}: stored data is corrupt\n", (unsigned) k.table, k.gid, k.iid, k.mid);
            result = D_STORE_RESULT_MUTILATED;
        } else {
            if ((rc = kv_put (snapshot, &k, sz, val, 0)) < 0) {
                kvlog (st, D_LEVEL_SEVERE, "Message {%u:%u:%llu:%llu} could not be written to disk\n", (unsigned) k.table, k.gid, k.iid, k.mid);
                result = kv_convert_result(rc);
            }
            os_free (val);
        }
    }
    kvdebug ("  done\n");
    kv_iter_free (st->kv, iter);

    if (result == D_STORE_RESULT_OK) {
        kvlog (st, D_LEVEL_FINEST, "Snapshot copied info messages of group '%s.%s'\n", g->_parent.partition,  g->_parent.topic);
    }

    return result;
}

static d_storeResult snapshot_copy_transaction_journal(d_storeKV st, kv_store_t snapshot)
{
    d_storeResult result = D_STORE_RESULT_OK;
    kv_result_t rc;
    kv_key_t min, max;
    kv_iter_t iter;
    int hasdata;

    min.table = TABLE_MESSAGE_EOT;
    min.gid = 0;
    min.iid = 0;
    min.mid = 0;
    max.table = TABLE_MESSAGE_EOT;
    max.gid = ~(os_uint32)0;
    max.iid = ~(os_uint32)0;
    max.mid = ~(os_uint64)0;

    kv_iter_new (st->kv, &iter, &min, &max, st->open_transaction_eot_type);
    while (result == D_STORE_RESULT_OK && kv_iter_next (&hasdata, st->kv, iter) >= 0 && hasdata) {
        kv_key_t k;
        os_uint32 sz;
        void *val;

        kv_iter_getkey (st->kv, iter, &k);
        kvdebug ("  %u:%u:%llu\n", k.gid, k.iid, k.mid);

        if (kv_iter_getvalue (st->kv, iter, &sz, &val) < 0) {
            kvlog (st, D_LEVEL_SEVERE, "Journal message {%u:%u:%llu:%llu}: stored data is corrupt\n", (unsigned) k.table, k.gid, k.iid, k.mid);
            result = D_STORE_RESULT_MUTILATED;
        } else {
            if ((rc = kv_put (snapshot, &k, sz, val, st->open_transaction_eot_type)) < 0) {
                kvlog (st, D_LEVEL_SEVERE, "journal message {%u:%u:%llu:%llu} could not be written to disk\n", (unsigned) k.table, k.gid, k.iid, k.mid);
                result = kv_convert_result(rc);
            }
            os_free (val);
        }
    }
    kvdebug ("  done\n");
    kv_iter_free (st->kv, iter);

    if (result == D_STORE_RESULT_OK) {
        kvlog (st, D_LEVEL_FINEST, "Snapshot copied journal messages\n");
    }

    return result;
}



struct namespace_action_arg {
    d_storeKV st;
    kv_store_t snapshot;
    d_storeResult result;
};

static void snapshot_namespace_action(struct namespace *ns, struct namespace_action_arg *arg)
{
    d_storeResult result = arg->result;
    kv_result_t rc;
    kv_key_t key;
    os_uint32 sz;
    void *blob = NULL;

    assert(ns);
    assert(ns->name);

    /* copy from NAMESPACE TABLE the corresponding namespace */
    if (result == D_STORE_RESULT_OK) {
        key.table = TABLE_NAMESPACE;
        key.gid = ns->id;
        key.iid = ns->version;
        key.mid = 0;

        if ((rc = kv_get (arg->st->kv, &key, &sz, &blob, arg->st->namespace_type)) < 0) {
            kvlog (arg->st, D_LEVEL_SEVERE, "Retrieval of namespace %u (%s) failed\n", ns->id, ns->name);
            result = D_STORE_RESULT_MUTILATED;
        } else if ((rc = kv_put (arg->snapshot, &key, sz, blob, arg->st->namespace_type)) < 0) {
            kvlog (arg->st, D_LEVEL_SEVERE, "Storage of namespace %u (%s) failed\n", ns->id, ns->name);
            result = kv_convert_result(rc);
        }
        os_free(blob);
    }

    /* copy from NAMESPACE_QUALITY TABLE the corresponding quality */
    if (result == D_STORE_RESULT_OK) {
        key.table = TABLE_NAMESPACE_QUALITY;
        key.gid = ns->id;
        key.iid = ns->version;
        key.mid = 0;
        blob = NULL;

        if ((rc = kv_get (arg->st->kv, &key, &sz, &blob, arg->st->namespace_quality_type)) < 0) {
            kvlog (arg->st, D_LEVEL_SEVERE, "Retrieval of namespace quality %u (%s) failed\n", ns->id, ns->name);
            result = D_STORE_RESULT_MUTILATED;
        } else if ((rc = kv_put (arg->snapshot, &key, sz, blob, arg->st->namespace_quality_type)) < 0) {
            kvlog (arg->st, D_LEVEL_SEVERE, "Storage of namespace quality %u (%s) failed\n", ns->id, ns->name);
            result = kv_convert_result(rc);
        }
        os_free(blob);
    }

    /* copy from NAMESPACE_COMPLETENESS TABLE the corresponding completeness */
    if (result == D_STORE_RESULT_OK) {
        key.table = TABLE_NAMESPACE_COMPLETENESS;
        key.gid = ns->id;
        key.iid = ns->version;
        key.mid = 0;
        blob = NULL;

        if ((rc = kv_get (arg->st->kv, &key, &sz, &blob, arg->st->namespace_completeness_type)) < 0) {
            kvlog (arg->st, D_LEVEL_SEVERE, "Retrieval of namespace completeness %u (%s) failed\n", ns->id, ns->name);
            result = D_STORE_RESULT_MUTILATED;
        } else if ((rc = kv_put (arg->snapshot, &key, sz, blob, arg->st->namespace_completeness_type)) < 0) {
            kvlog (arg->st, D_LEVEL_SEVERE, "Storage of namespace completeness %u (%s) failed\n", ns->id, ns->name);
            result = kv_convert_result(rc);
        }
        os_free(blob);
    }

    if (arg->result == D_STORE_RESULT_OK) {
        kvlog (arg->st, D_LEVEL_FINEST, "Snapshot copied namespace '%s'\n", ns->name);
    }

    arg->result = result;
}



d_storeResult d_storeCreatePersistentSnapshotKV (const d_store gstore, const c_char *partitionExpr, const c_char *topicExpr, const c_char *uri)
{
    d_storeResult result = D_STORE_RESULT_OK;
    d_storeKV st;
    kv_store_t snapshot = NULL;
    c_char *resultDir = NULL;
    struct idtable *pth = NULL;
    struct hashtable *nh = NULL;

    LOCK_OPEN_STOREKV (st, gstore);

    assert(gstore);
    assert(topicExpr);
    assert(partitionExpr);
    assert(uri);
    assert(d_objectIsValid(d_object(gstore), D_STORE));
    assert(gstore->type == D_STORE_TYPE_KV);

    if (strcmp(st->diskStorePath, uri) == 0) {
        kvlog (st, D_LEVEL_SEVERE, "snapshot: cannot create snapshot in current store directory %s\n", uri);
        result =  D_STORE_RESULT_PRECONDITION_NOT_MET;
    } else if ((resultDir = d_storeDirNew (gstore, uri)) == NULL) {
        kvlog (st, D_LEVEL_SEVERE, "snapshot: directory '%s' could not be created.\n", uri);
        result = D_STORE_RESULT_PRECONDITION_NOT_MET;
    } else if ((pth = idtable_new ()) == NULL) {
        kvlog (st, D_LEVEL_SEVERE, "create-snapshot: allocation failed.\n");
        result = D_STORE_RESULT_OUT_OF_RESOURCES;
    } else if ((nh = hashtable_new ((hashtable_key_func_t)get_namespace_key, (hashtable_cmp_func_t)compare_namespace_by_name)) == NULL) {
        kvlog (st, D_LEVEL_SEVERE, "snapshot: allocation failed.\n");
        result = D_STORE_RESULT_OUT_OF_RESOURCES;
    } else {

        assert(st->kv);

        if ((snapshot = kv_open(st, uri, st->kv->kind, NULL)) == NULL) {
            kvlog (st, D_LEVEL_SEVERE, "snapshot: store '%s' could not be created.\n", uri);
            result = D_STORE_RESULT_ERROR;
        } else {
            result = snapshot_copy_version(st, snapshot);
        }
    }

    if (result == D_STORE_RESULT_OK) {
        const struct d_groupListKV_s *g;

        /* walk group list */
        for (g = st->groups; g != NULL; g = g->_parent.next) {
            struct namespace *ns;

            /* check if group references namespace with highest version */
            assert(g->namespace);
            ns = find_namespace_by_id (st, g->namespace->id);
            if (ns != g->namespace) {
                continue;
            }

            /* check if group matches partition and topic expression */
            if (!d_patternMatch(g->_parent.partition, (c_string)partitionExpr) ||
                !d_patternMatch(g->_parent.topic, (c_string)topicExpr)) {
                continue;
            }

            /* check if topic information is present in snapshot */
            if (idtable_get(pth, g->topic_id) != g->topic_id) {
                if ((result = snapshot_copy_topic_info (st, snapshot, g->topic_id, g->_parent.topic)) != D_STORE_RESULT_OK) {
                    break;
                }
                idtable_put(pth, g->topic_id);
            }

            /* copy from GROUP TABLE the corresponding group */
            if ((result = snapshot_copy_group_info (st, snapshot, g)) != D_STORE_RESULT_OK) {
                break;
            }

            /* copy from MESSAGE TABLE all corresponding messages */
            if ((result = snapshot_copy_message_info (st, snapshot, g)) != D_STORE_RESULT_OK) {
                break;
            }

            hashtable_put(nh, ns);
        } /* for */

    }

    if (result == D_STORE_RESULT_OK) {
        struct namespace_action_arg argument;

        argument.result = D_STORE_RESULT_OK;
        argument.st = st;
        argument.snapshot = snapshot;

        /* walk list of namespaces to be copied to snapshot */
        hashtable_walk (nh, (hashtable_action_func_t)snapshot_namespace_action, &argument);

        result = argument.result;
    }

    if (result == D_STORE_RESULT_OK) {
        /* Copy transaction journal messages */
        result = snapshot_copy_transaction_journal(st, snapshot);
    }

    if (snapshot) {
        kv_close (snapshot);
    }

    idtable_free (pth);
    hashtable_free (nh);
    os_free (resultDir);

    if (result == D_STORE_RESULT_OK) {
        kvlog (st, D_LEVEL_FINE, "Created snapshot for expression '%s.%s' in '%s'\n", partitionExpr, topicExpr, uri);
    }

    UNLOCK_OPEN_STOREKV (st);

    return result;
}


/*****************************************************************************
 *
 *         LOAD FUNCTIONS
 *
 *****************************************************************************/

struct hash_helper_arg {
    v_group group;
    c_iter list;
};

static c_bool create_hash_helper (const struct inst *inst, c_iter list)
{
    struct msg *m;

    assert(inst);
    assert(list);

    for (m = inst->oldest; m; m = m->newer) {
        struct v_groupFlushData *flushData = os_malloc(sizeof(struct v_groupFlushData));
        assert (m->v_msg != NULL);

        flushData->object = c_keep(m->v_msg);
        flushData->instance = NULL;
        flushData->flushType = V_GROUP_FLUSH_MESSAGE;
        list = c_iterAppend(list, flushData);
    }
    return TRUE;
}

static void free_hash_helper (void *o, c_iterActionArg arg)
{
    struct v_groupFlushData *flushData = (struct v_groupFlushData *)o;

    OS_UNUSED_ARG(arg);

    if (flushData) {
        c_free(flushData->object);
        c_free(flushData->instance);
        os_free(flushData);
    }
}

static d_storeResult calculate_hash (struct d_storeKV_s *st, const struct d_groupListKV_s *g, struct d_groupHash *groupHash)
{
    c_iter list;

    OS_UNUSED_ARG(st);

    list = c_iterNew(NULL);
    (void)d_tableWalk (g->instances, create_hash_helper, list);
    d_groupHashCalculate(groupHash, list);
    c_iterWalk(list, free_hash_helper, NULL);
    c_iterFree(list);

    return D_STORE_RESULT_OK;
}

static d_groupListKV find_older_group_version (const d_groupListKV g)
{
    d_groupListKV entry;

    for (entry = g->_parent.next; entry; entry = entry->_parent.next) {
        if ((strcmp(g->_parent.topic, entry->_parent.topic) == 0) &&
            (strcmp(g->_parent.partition, entry->_parent.partition) == 0)) {
            if (g->namespace->version > entry->namespace->version) {
                return entry;
            }
        }
    }

    return NULL;
}

d_storeResult d_storeMessagesLoadKV (const d_store gstore, const d_group dgroup, struct d_groupHash *groupHash)
{
    d_storeKV st;
    d_storeResult result = D_STORE_RESULT_OK;
    c_char *topicName;
    c_char *partitionName;
    v_group group;
    os_uint32 topic_id;
    d_groupListKV g, g0;

    assert(groupHash);

    if (gstore == NULL || dgroup == NULL) {
        return D_STORE_RESULT_ILL_PARAM;
    }

    LOCK_OPEN_STOREKV (st, gstore);

    group = d_groupGetKernelGroup (dgroup);
    topicName = v_topicName (v_groupTopic (group));
    partitionName = v_partitionName (v_groupPartition (group));
    kvlog (st, D_LEVEL_FINE, "load messages for group '%s.%s'\n", partitionName, topicName);

    /* We need the topic_id, and the easiest way to find that is to
     * enumerate the in-memory representation of the groups. Those get
     * initialised from the disk as soon as the store is opened, and
     * so if we can't find it there, it doesn't exist on disk. */
    if ((g = find_group (st, partitionName, topicName)) == NULL) {
        kvlog (st, D_LEVEL_WARNING, "load messages: group '%s.%s' unknown (guaranteed no data)\n", partitionName, topicName);
        memset(groupHash, 0, sizeof(struct d_groupHash));
        result = D_STORE_RESULT_OK;
        goto out;
    }

    if ((g0 = find_older_group_version(g)) == NULL) {
        kvlog (st, D_LEVEL_WARNING, "load messages: group '%s.%s' is new (guaranteed no data)\n", partitionName, topicName);
        memset(groupHash, 0, sizeof(struct d_groupHash));
        result = D_STORE_RESULT_OK;
        goto out;
    }
    topic_id = g0->topic_id;
    kvdebug ("  topic_id = %u\n", topic_id);

    /* If we know of the group, we must still verify that the topic
     * definitions in the kernel and on-disk match: there is no
     * guarantee some other process hasn't already defined the topic
     * in an incompatible way. */
    if (!topic_definition_matches (st, group, topic_id, topicName)) {
        kvlog (st, D_LEVEL_WARNING, "load messages: topic '%s' definitions do not match, not injecting\n", topicName);
        result = D_STORE_RESULT_METADATA_MISMATCH;
        goto out;
    }
    kvdebug ("  topic definitions match\n");

    /* Store the kernel group in the d_groupListKV. This makes lookup
     * of the kernel group very easy. */
    g0->vgroup = group;

    /* Reconstruct the instances for the older group in-memory */
    reconstruct_instances (st, g0, group);

    /* Once we have everything in-memory calculate hash */
    calculate_hash (st, g0, groupHash);

    kvdebug ("  done\n");
out:
    UNLOCK_OPEN_STOREKV (st);
    return result;
}

static c_bool free_messages_helper (const struct inst *inst, c_voidp userData)
{
    struct msg *m;

    OS_UNUSED_ARG(userData);

    for (m = inst->oldest; m; m = m->newer) {
        assert (m->v_msg != NULL);
        c_free (m->v_msg);
        m->v_msg = NULL;
    }
    return TRUE;
}

d_storeResult d_storeMessagesLoadFlushKV (const d_store gstore, const d_group dgroup, c_bool inject)
{
    d_storeKV st;
    d_storeResult result = D_STORE_RESULT_OK;
    c_char *topicName;
    c_char *partitionName;
    v_group group;
    d_groupListKV g, g0;

    if (gstore == NULL || dgroup == NULL) {
        return D_STORE_RESULT_ILL_PARAM;
    }

    LOCK_OPEN_STOREKV (st, gstore);
    group = d_groupGetKernelGroup (dgroup);
    topicName = v_topicName (v_groupTopic (group));
    partitionName = v_partitionName (v_groupPartition (group));

    if ((g = find_group (st, partitionName, topicName)) == NULL) {
        kvlog (st, D_LEVEL_WARNING, "load-flush messages: group '%s.%s' unknown (guaranteed no data)\n", partitionName, topicName);
        result = D_STORE_RESULT_OK;
        goto out;
    }

    if ((g0 = find_older_group_version(g)) == NULL) {
        kvlog (st, D_LEVEL_WARNING, "load-flush messages: group '%s.%s' is new (guaranteed no data)\n", partitionName, topicName);
        result = D_STORE_RESULT_OK;
        goto out;
    }

    if (inject == TRUE) {
        /* Inject the older in-memory reconstructed instances into
         * the current group and store them in the active store. */
        result = inject_messages (st, g0, group, TRUE);
    } else {
        (void)d_tableWalk (g->instances, free_messages_helper, NULL);
        result = D_STORE_RESULT_OK;
    }

out:
    UNLOCK_OPEN_STOREKV (st);
    return result;
}

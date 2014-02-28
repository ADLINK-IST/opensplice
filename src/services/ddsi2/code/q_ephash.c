#include <stddef.h>
#include <assert.h>

#include "os_heap.h"
#include "os_mutex.h"
#include "os_cond.h"
#include "os_thread.h"
#include "sysdeps.h" /* pa_membar_..., ASSERT_MUTEX_HELD */

#include "ut_avl.h"
#include "q_ephash.h"
#include "q_config.h"
#include "q_globals.h"
#include "q_entity.h"
#include "sysdeps.h" /* pa_membar_..., ASSERT_MUTEX_HELD */
#include "q_rtps.h" /* guid_t */
#include "q_thread.h" /* for assert(thread is awake) */

#include "kernelModule.h" /* v_gid */
#include "v_public.h" /* v_handleIsEqual */

#define CONTAINER_OF(ptr, type, member) ((type *) ((char *) (ptr) - offsetof (type, member)))

struct ephash {
  os_mutex lock;
  int nbitskey;
  struct ephash_chain_entry **heads;

  /* Separate lists for enumerating are supposed to be here just
     temporarily, to be replaced when all this gets changed into a
     proper lock-free, wait-free, auto-resizing hash table. */
  struct ephash_chain_entry *enum_lists[EK_NKINDS];
  /* We track live enumerations, so we can avoid restarting on
     deletes */
  struct ephash_enum *live_enums;
};

/* Should fix the abstraction layer ... */
#define UINT64_CONST(x, y, z) (((os_uint64) (x) * 1000000 + (y)) * 1000000 + (z))

static const os_uint64 unihashconsts[] = {
  UINT64_CONST (16292676, 669999, 574021),
  UINT64_CONST (10242350, 189706, 880077),
  UINT64_CONST (12844332, 200329, 132887),
  UINT64_CONST (16728792, 139623, 414127)
};

static void ephash_update_enums_on_delete (struct ephash *h, struct ephash_chain_entry *ce);

static int hash_gid (const v_gid *gid, int nbitskey)
{
  /* Universal hashing relying on 64-bit arithmetic, using localId and
   serial, which are both 32-bit integers.

   SystemId is a constant for local writers and needn't be taken
   into account.

   Could consider ignoring "serial" altogether, because two localIds
   with the same serial is not very likely to happen, and if it
   happens, the older of the two will be removed shortly.

   See, e.g., http://en.wikipedia.org/wiki/Universal_hash_function. */
  return
  (int) ((((os_uint32) gid->localId + unihashconsts[0]) *
          ((os_uint32) gid->serial  + unihashconsts[1]))
         >> (64 - nbitskey));
}

static int hash_guid (const struct nn_guid *guid, int nbitskey)
{
  /* Universal hashing relying on 64-bit arithmetic, using localId and
     serial, which are both 32-bit integers.

     See, e.g., http://en.wikipedia.org/wiki/Universal_hash_function. */
  return
    (int) (((((os_uint32) guid->prefix.u[0] + unihashconsts[0]) *
             ((os_uint32) guid->prefix.u[1] + unihashconsts[1])) +
            (((os_uint32) guid->prefix.u[2] + unihashconsts[2]) *
             ((os_uint32) guid->entityid.u  + unihashconsts[3])))
           >> (64 - nbitskey));
}

static int gid_eq (const struct v_gid_s *a, const struct v_gid_s *b)
{
  return v_gidEqual (*a, *b);
}

static int guid_eq (const struct nn_guid *a, const struct nn_guid *b)
{
  return
    a->prefix.u[0] == b->prefix.u[0] && a->prefix.u[1] == b->prefix.u[1] &&
    a->prefix.u[2] == b->prefix.u[2] && a->entityid.u == b->entityid.u;
}

struct ephash *ephash_new (os_uint32 soft_limit)
{
  struct ephash *ephash;
  os_uint32 limit;
  int i, nbitskey, init_size;

  /* We use soft_limit to compute hash table size; 70% occupancy
     supposedly is ok so (3/2) * soft_limit should be okay-ish. We
     want a power of two, and hence compute:
     floor(log2(2*(3*soft_limit/2))) */
  assert (sizeof (nbitskey) >= sizeof (soft_limit));
  assert (soft_limit < (1 << 28));
  limit = 3 * soft_limit / 2;
  nbitskey = 0;
  while (limit)
  {
    limit >>= 1;
    nbitskey++;
  }
  init_size = 1 << nbitskey;
  TRACE (("ephash_new: soft_limit %u nbitskey %d init_size %d l.f. %f\n", soft_limit, nbitskey, init_size, (double) soft_limit / init_size));
  if ((ephash = os_malloc (sizeof (*ephash))) == NULL)
    goto fail_ephash;
  if (os_mutexInit (&ephash->lock, &gv.mattr) != os_resultSuccess)
    goto fail_mutex;
  ephash->nbitskey = nbitskey;
  if ((ephash->heads = os_malloc (init_size * sizeof (*ephash->heads))) == NULL)
    goto fail_heads;
  for (i = 0; i < init_size; i++)
    ephash->heads[i] = NULL;
  for (i = 0; i < (int) (sizeof (ephash->enum_lists) / sizeof (ephash->enum_lists[0])); i++)
    ephash->enum_lists[i] = NULL;
  ephash->live_enums = NULL;
  return ephash;
 fail_heads:
  os_mutexDestroy (&ephash->lock);
 fail_mutex:
  os_free (ephash);
 fail_ephash:
  return NULL;
}

void ephash_free (struct ephash *ephash)
{
  assert (ephash->live_enums == NULL);
  os_free (ephash->heads);
  os_mutexDestroy (&ephash->lock);
  os_free (ephash);
}

static void ephash_insert (struct ephash *ephash, int idx, struct ephash_chain_entry *ce, int listidx)
{
  assert (0 <= idx && idx < (1 << ephash->nbitskey));
  assert (0 <= listidx && listidx < (int) (sizeof (ephash->enum_lists) / sizeof (ephash->enum_lists[0])));
  ce->prev = NULL;
  os_mutexLock (&ephash->lock);
  ce->next = ephash->heads[idx];
  if (ce->next)
    ce->next->prev = ce;
  /* pa_membar ensures that the transmit and receive threads will see
     a properly linked hash chain */
  pa_membar_producer ();
  ephash->heads[idx] = ce;

  /* enumerate support (temporary ... I hope) */
  ce->enum_next = ephash->enum_lists[listidx];
  if (ce->enum_next)
    ce->enum_next->enum_prev = ce;
  ce->enum_prev = NULL;
  ephash->enum_lists[listidx] = ce;
  os_mutexUnlock (&ephash->lock);
}

static void ephash_remove (struct ephash *ephash, int idx, struct ephash_chain_entry *ce, int listidx)
{
  /* removing a local object from the hash chain must:
     (1) prevent any subsequent lookups from finding the lobj
     (2) allow any parallel lookups to keep walking the chain
     therefore, obj->hash_next of the removed obj must not be
     changed until no further parallel lookups may need to touch the
     obj */
  assert (0 <= idx && idx < (1 << ephash->nbitskey));
  assert (0 <= listidx && listidx < (int) (sizeof (ephash->enum_lists) / sizeof (ephash->enum_lists[0])));
  os_mutexLock (&ephash->lock);
  if (ce->next)
    ce->next->prev = ce->prev;
  if (ce->prev)
    ce->prev->next = ce->next;
  else
    ephash->heads[idx] = ce->next;

  /* enumerate support (temporary ... I hope) */
  if (ce->enum_next)
    ce->enum_next->enum_prev = ce->enum_prev;
  if (ce->enum_prev)
    ce->enum_prev->enum_next = ce->enum_next;
  else
    ephash->enum_lists[listidx] = ce->enum_next;
  ephash_update_enums_on_delete (ephash, ce);
  os_mutexUnlock (&ephash->lock);
}

/* GUID-based */

static void ephash_guid_insert (struct entity_common *e)
{
  ephash_insert (gv.guid_hash, hash_guid (&e->guid, gv.guid_hash->nbitskey), &e->guid_hash_chain, (int) e->kind);
}

static void ephash_guid_remove (struct entity_common *e)
{
  ephash_remove (gv.guid_hash, hash_guid (&e->guid, gv.guid_hash->nbitskey), &e->guid_hash_chain, (int) e->kind);
}

static void *ephash_lookup_guid (const struct ephash *ephash, const struct nn_guid *guid, enum entity_kind kind)
{
  struct ephash_chain_entry *ce;
  int idx = hash_guid (guid, ephash->nbitskey);

  assert (idx >= 0 && idx < (1 << ephash->nbitskey));
  for (ce = ephash->heads[idx]; ce; ce = ce->next)
  {
    struct entity_common *e = CONTAINER_OF (ce, struct entity_common, guid_hash_chain);
    if (guid_eq (guid, &e->guid))
    {
      return (e->kind == kind) ? e : NULL;
    }
  }
  return NULL;
}

void ephash_insert_participant_guid (struct participant *pp)
{
  ephash_guid_insert (&pp->e);
}

void ephash_insert_proxy_participant_guid (struct proxy_participant *proxypp)
{
  ephash_guid_insert (&proxypp->e);
}

void ephash_insert_writer_guid (struct writer *wr)
{
  ephash_guid_insert (&wr->e);
}

void ephash_insert_reader_guid (struct reader *rd)
{
  ephash_guid_insert (&rd->e);
}

void ephash_insert_proxy_writer_guid (struct proxy_writer *pwr)
{
  ephash_guid_insert (&pwr->e);
}

void ephash_insert_proxy_reader_guid (struct proxy_reader *prd)
{
  ephash_guid_insert (&prd->e);
}

void ephash_remove_participant_guid (struct participant *pp)
{
  ephash_guid_remove (&pp->e);
}

void ephash_remove_proxy_participant_guid (struct proxy_participant *proxypp)
{
  ephash_guid_remove (&proxypp->e);
}

void ephash_remove_writer_guid (struct writer *wr)
{
  ephash_guid_remove (&wr->e);
}

void ephash_remove_reader_guid (struct reader *rd)
{
  ephash_guid_remove (&rd->e);
}

void ephash_remove_proxy_writer_guid (struct proxy_writer *pwr)
{
  ephash_guid_remove (&pwr->e);
}

void ephash_remove_proxy_reader_guid (struct proxy_reader *prd)
{
  ephash_guid_remove (&prd->e);
}

struct participant *ephash_lookup_participant_guid (const struct nn_guid *guid)
{
  assert (guid->entityid.u == NN_ENTITYID_PARTICIPANT);
  assert (offsetof (struct participant, e) == 0);
  return ephash_lookup_guid (gv.guid_hash, guid, EK_PARTICIPANT);
}

struct proxy_participant *ephash_lookup_proxy_participant_guid (const struct nn_guid *guid)
{
  assert (guid->entityid.u == NN_ENTITYID_PARTICIPANT);
  assert (offsetof (struct proxy_participant, e) == 0);
  return ephash_lookup_guid (gv.guid_hash, guid, EK_PROXY_PARTICIPANT);
}

struct writer *ephash_lookup_writer_guid (const struct nn_guid *guid)
{
  assert (is_writer_entityid (guid->entityid));
  assert (offsetof (struct writer, e) == 0);
  return ephash_lookup_guid (gv.guid_hash, guid, EK_WRITER);
}

struct reader *ephash_lookup_reader_guid (const struct nn_guid *guid)
{
  assert (is_reader_entityid (guid->entityid));
  assert (offsetof (struct reader, e) == 0);
  return ephash_lookup_guid (gv.guid_hash, guid, EK_READER);
}

struct proxy_writer *ephash_lookup_proxy_writer_guid (const struct nn_guid *guid)
{
  assert (is_writer_entityid (guid->entityid));
  assert (offsetof (struct proxy_writer, e) == 0);
  return ephash_lookup_guid (gv.guid_hash, guid, EK_PROXY_WRITER);
}

struct proxy_reader *ephash_lookup_proxy_reader_guid (const struct nn_guid *guid)
{
  assert (is_reader_entityid (guid->entityid));
  assert (offsetof (struct proxy_reader, e) == 0);
  return ephash_lookup_guid (gv.guid_hash, guid, EK_PROXY_READER);
}

/* GID-based */

static void ephash_gid_insert (struct ephash *gid_hash, struct generic_endpoint *ep)
{
  if (v_gidIsValid (ep->c.gid))
    ephash_insert (gid_hash, hash_gid (&ep->c.gid, gid_hash->nbitskey), &ep->c.gid_hash_chain, (int) ep->e.kind);
}

static void ephash_gid_remove (struct ephash *gid_hash, struct generic_endpoint *ep)
{
  if (v_gidIsValid (ep->c.gid))
    ephash_remove (gid_hash, hash_gid (&ep->c.gid, gid_hash->nbitskey), &ep->c.gid_hash_chain, (int) ep->e.kind);
}

static struct generic_endpoint *ephash_lookup_gid (const struct ephash *ephash, const struct v_gid_s *gid)
{
  struct ephash_chain_entry *ce;
  int idx = hash_gid (gid, ephash->nbitskey);

  assert (idx >= 0 && idx < (1 << ephash->nbitskey));
  for (ce = ephash->heads[idx]; ce; ce = ce->next)
  {
    struct generic_endpoint *ep = CONTAINER_OF (ce, struct generic_endpoint, c.gid_hash_chain);
    if (gid_eq (gid, &ep->c.gid))
      return ep;
  }
  return NULL;
}

void ephash_insert_writer_gid (struct ephash *gid_hash, struct writer *wr)
{
  ephash_gid_insert (gid_hash, (struct generic_endpoint *) wr);
}

void ephash_insert_reader_gid (struct ephash *gid_hash, struct reader *rd)
{
  ephash_gid_insert (gid_hash, (struct generic_endpoint *) rd);
}

void ephash_remove_writer_gid (struct ephash *gid_hash, struct writer *wr)
{
  ephash_gid_remove (gid_hash, (struct generic_endpoint *) wr);
}

void ephash_remove_reader_gid (struct ephash *gid_hash, struct reader *rd)
{
  ephash_gid_remove (gid_hash, (struct generic_endpoint *) rd);
}

struct writer *ephash_lookup_writer_gid (const struct ephash *gid_hash, const struct v_gid_s *gid)
{
  struct generic_endpoint *ep = ephash_lookup_gid (gid_hash, gid);
  assert (ep == NULL || ep->e.kind == EK_WRITER);
  return (struct writer *) ep;
}

struct reader *ephash_lookup_reader_gid (const struct ephash *gid_hash, const struct v_gid_s *gid)
{
  struct generic_endpoint *ep = ephash_lookup_gid (gid_hash, gid);
  assert (ep == NULL || ep->e.kind == EK_READER);
  return (struct reader *) ep;
}

/* Enumeration */

static void ephash_update_enums_on_delete (struct ephash *ephash, struct ephash_chain_entry *ce)
{
  struct ephash_enum *st;
  ASSERT_MUTEX_HELD (h->lock);
  for (st = ephash->live_enums; st; st = st->next_live)
    if (st->cursor == ce)
      st->cursor = ce->enum_next;
}

static void ephash_enum_init (struct ephash_enum *st, struct ephash *ephash, enum entity_kind kind)
{
  const int listidx = (int) kind;

  assert (0 <= listidx && listidx < (int) (sizeof (ephash->enum_lists) / sizeof (ephash->enum_lists[0])));
  os_mutexLock (&ephash->lock);
  st->ephash = ephash;
  st->next_live = ephash->live_enums;
  st->prev_live = NULL;
  if (st->next_live)
    st->next_live->prev_live = st;
  ephash->live_enums = st;
  st->cursor = ephash->enum_lists[listidx];
  os_mutexUnlock (&ephash->lock);
}

void ephash_enum_writer_init (struct ephash_enum_writer *st)
{
  ephash_enum_init (&st->st, gv.guid_hash, EK_WRITER);
}

void ephash_enum_reader_init (struct ephash_enum_reader *st)
{
  ephash_enum_init (&st->st, gv.guid_hash, EK_READER);
}

void ephash_enum_proxy_writer_init (struct ephash_enum_proxy_writer *st)
{
  ephash_enum_init (&st->st, gv.guid_hash, EK_PROXY_WRITER);
}

void ephash_enum_proxy_reader_init (struct ephash_enum_proxy_reader *st)
{
  ephash_enum_init (&st->st, gv.guid_hash, EK_PROXY_READER);
}

void ephash_enum_participant_init (struct ephash_enum_participant *st)
{
  ephash_enum_init (&st->st, gv.guid_hash, EK_PARTICIPANT);
}

void ephash_enum_proxy_participant_init (struct ephash_enum_proxy_participant *st)
{
  ephash_enum_init (&st->st, gv.guid_hash, EK_PROXY_PARTICIPANT);
}

static void *ephash_perform_enum (struct ephash_enum *st)
{
  void *x;
  /* should be lock-free -- but this takes less development time */
  os_mutexLock (&st->ephash->lock);
  if (st->cursor == NULL)
    x = NULL;
  else
  {
    x = CONTAINER_OF (st->cursor, struct entity_common, guid_hash_chain);
    st->cursor = st->cursor->enum_next;
  }
  os_mutexUnlock (&st->ephash->lock);
  return x;
}

struct writer *ephash_enum_writer_next (struct ephash_enum_writer *st)
{
  assert (offsetof (struct writer, e) == 0);
  return ephash_perform_enum (&st->st);
}

struct reader *ephash_enum_reader_next (struct ephash_enum_reader *st)
{
  assert (offsetof (struct reader, e) == 0);
  return ephash_perform_enum (&st->st);
}

struct proxy_writer *ephash_enum_proxy_writer_next (struct ephash_enum_proxy_writer *st)
{
  assert (offsetof (struct proxy_writer, e) == 0);
  return ephash_perform_enum (&st->st);
}

struct proxy_reader *ephash_enum_proxy_reader_next (struct ephash_enum_proxy_reader *st)
{
  assert (offsetof (struct proxy_reader, e) == 0);
  return ephash_perform_enum (&st->st);
}

struct participant *ephash_enum_participant_next (struct ephash_enum_participant *st)
{
  assert (offsetof (struct participant, e) == 0);
  return ephash_perform_enum (&st->st);
}

struct proxy_participant *ephash_enum_proxy_participant_next (struct ephash_enum_proxy_participant *st)
{
  assert (offsetof (struct proxy_participant, e) == 0);
  return ephash_perform_enum (&st->st);
}

static void ephash_enum_fini (struct ephash_enum *st)
{
  struct ephash *ephash = st->ephash;

  os_mutexLock (&ephash->lock);
  if (st->next_live)
  {
    st->next_live->prev_live = st->prev_live;
  }
  if (st->prev_live)
  {
    st->prev_live->next_live = st->next_live;
  }
  else
  {
    ephash->live_enums = st->next_live;
  }
  os_mutexUnlock (&ephash->lock);
}

void ephash_enum_writer_fini (struct ephash_enum_writer *st)
{
  ephash_enum_fini (&st->st);
}

void ephash_enum_reader_fini (struct ephash_enum_reader *st)
{
  ephash_enum_fini (&st->st);
}

void ephash_enum_proxy_writer_fini (struct ephash_enum_proxy_writer *st)
{
  ephash_enum_fini (&st->st);
}

void ephash_enum_proxy_reader_fini (struct ephash_enum_proxy_reader *st)
{
  ephash_enum_fini (&st->st);
}

void ephash_enum_participant_fini (struct ephash_enum_participant *st)
{
  ephash_enum_fini (&st->st);
}

void ephash_enum_proxy_participant_fini (struct ephash_enum_proxy_participant *st)
{
  ephash_enum_fini (&st->st);
}

/* SHA1 not available (unoffical build.) */

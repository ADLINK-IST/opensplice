#include <ctype.h>
#include <stddef.h>
#include <assert.h>

#include "os_heap.h"
#include "os_mutex.h"
#include "os_socket.h"
#include "os_if.h"

#include "q_osplser.h"
#include "q_protocol.h"
#include "q_rtps.h"
#include "q_misc.h"
#include "q_config.h"
#include "q_log.h"
#include "q_plist.h"
#include "q_unused.h"
#include "q_xevent.h"
#include "q_addrset.h"
#include "q_ddsi_discovery.h"
#include "q_radmin.h"
#include "q_ephash.h"
#include "q_entity.h"
#include "q_globals.h"
#include "q_xmsg.h"
#include "q_bswap.h"
#include "q_transmit.h"
#include "q_lease.h"
#include "q_fibheap.h"

#include "sysdeps.h"

/* This is absolute bottom for signed integers, where -x = x and yet x
   != 0 -- and note that it had better be 2's complement machine! */
#define TSCHED_NOT_ON_HEAP INT64_MIN

struct lease {
  struct fibheap_node heapnode;
  os_int64 tsched;     /* access guarded by leaseheap_lock */
  os_int64 tend;       /* access guarded by lock_lease/unlock_lease */
  os_int64 tdur;       /* constant */
  struct entity_common *entity; /* constant */
};

static int compare_lease_tsched (const void *va, const void *vb)
{
  const struct lease *a = va;
  const struct lease *b = vb;
  return (a->tsched == b->tsched) ? 0 : (a->tsched < b->tsched) ? -1 : 1;
}

void lease_management_init (void)
{
  int i;
  os_mutexInit (&gv.leaseheap_lock, &gv.mattr);
  for (i = 0; i < N_LEASE_LOCKS; i++)
    os_mutexInit (&gv.lease_locks[i], &gv.mattr);
  fh_init (&gv.leaseheap, offsetof (struct lease, heapnode), compare_lease_tsched);
}

void lease_management_term (void)
{
  int i;
  assert (fh_min (&gv.leaseheap) == NULL);
  for (i = 0; i < N_LEASE_LOCKS; i++)
    os_mutexDestroy (&gv.lease_locks[i]);
  os_mutexDestroy (&gv.leaseheap_lock);
}

static os_mutex *lock_lease_addr (struct lease const * const l)
{
  os_uint32 u = (os_ushort) ((os_address) l >> 3);
  os_uint32 v = u * 0xb4817365;
  int idx = v >> (32 - N_LEASE_LOCKS_LG2);
  return &gv.lease_locks[idx];
}

static void lock_lease (const struct lease *l)
{
  os_mutexLock (lock_lease_addr (l));
}

static void unlock_lease (const struct lease *l)
{
  os_mutexUnlock (lock_lease_addr (l));
}

struct lease *lease_new (os_int64 tdur, struct entity_common *e)
{
  struct lease *l;
  if ((l = os_malloc (sizeof (*l))) == NULL)
    return NULL;
  TRACE (("lease_new(tdur %lld guid %x:%x:%x:%x) @ %p\n", tdur, PGUID (e->guid), (void *) l));
  l->tdur = tdur;
  l->tend = add_duration_to_time (now (), tdur);
  l->tsched = TSCHED_NOT_ON_HEAP;
  l->entity = e;
  return l;
}

void lease_register (struct lease *l)
{
  TRACE (("lease_register(l %p guid %x:%x:%x:%x)\n", (void *) l, PGUID (l->entity->guid)));
  assert (l->tsched == TSCHED_NOT_ON_HEAP);
  os_mutexLock (&gv.leaseheap_lock);
  lock_lease (l);
  l->tsched = l->tend;
  unlock_lease (l);
  fh_insert (&gv.leaseheap, l);
  os_mutexUnlock (&gv.leaseheap_lock);
}

void lease_free (struct lease *l)
{
  TRACE (("lease_free(l %p guid %x:%x:%x:%x)\n", (void *) l, PGUID (l->entity->guid)));
  if (l->tsched != TSCHED_NOT_ON_HEAP)
  {
    os_mutexLock (&gv.leaseheap_lock);
    fh_delete (&gv.leaseheap, l);
    os_mutexUnlock (&gv.leaseheap_lock);
  }
  os_free (l);
}

void lease_renew (struct lease *l, os_int64 tnow)
{
  os_int64 tend_new = add_duration_to_time (tnow, l->tdur);
  int did_update;
  lock_lease (l);
  if (tend_new <= l->tend)
    did_update = 0;
  else
  {
    l->tend = tend_new;
    did_update = 1;
  }
  unlock_lease (l);

  if ((config.enabled_logcats & LC_TRACE) && did_update)
  {
    int tsec, tusec;
    TRACE ((" L("));
    if (l->entity->guid.entityid.u == NN_ENTITYID_PARTICIPANT)
      TRACE ((":%x", l->entity->guid.entityid.u));
    else
      TRACE (("%x:%x:%x:%x", PGUID (l->entity->guid)));
    time_to_sec_usec (&tsec, &tusec, tend_new);
    TRACE ((" %d.%06d)", tsec, tusec));
  }
}

void check_and_handle_lease_expiration (UNUSED_ARG (struct thread_state1 *self), os_int64 tnow)
{
  struct lease *l;
  os_mutexLock (&gv.leaseheap_lock);
  while ((l = fh_min (&gv.leaseheap)) != NULL && l->tsched <= tnow)
  {
    nn_guid_t g = l->entity->guid;
    enum entity_kind k = l->entity->kind;

    assert (l->tsched != TSCHED_NOT_ON_HEAP);
    fh_extractmin (&gv.leaseheap);

    lock_lease (l);
    if (tnow < l->tend)
    {
      l->tsched = l->tend;
      unlock_lease (l);
      fh_insert (&gv.leaseheap, l);
      continue;
    }

    TRACE (("lease expired: l %p guid %x:%x:%x:%x tend %lld < now %lld\n", (void *) l, PGUID (g), l->tend, tnow));

    /* If the proxy participant is relying on another participant for
       writing its discovery data (on the privileged participant,
       i.e., its ddsi2 instance), we can't afford to drop it while the
       privileged one is still considered live.  If we do and it was a
       temporary asymmetrical thing and the ddsi2 instance never lost
       its liveliness, we will not rediscover the endpoints of this
       participant because we will not rediscover the ddsi2
       participant.

       So IF it is dependent on another one, we renew the lease for a
       very short while if the other one is still alive.  If it is a
       real case of lost liveliness, the other one will be gone soon
       enough; if not, we should get a sign of life soon enough.

       In this case, we simply abort the current iteration of the loop
       after renewing the lease and continue with the next one.

       This trick would fail if the ddsi2 participant can lose its
       liveliness and regain it before we re-check the liveliness of
       the dependent participants, and so the interval here must
       significantly less than the pruning time for the
       deleted_participants admin.

       I guess that means there is a really good argument for the SPDP
       and SEDP writers to be per-participant! */
    if (k == EK_PROXY_PARTICIPANT)
    {
      struct proxy_participant *proxypp;
      if ((proxypp = ephash_lookup_proxy_participant_guid (&g)) != NULL &&
          ephash_lookup_proxy_participant_guid (&proxypp->privileged_pp_guid) != NULL)
      {
        TRACE (("but postponing because privileged pp %x:%x:%x:%x is still live\n",
                PGUID (proxypp->privileged_pp_guid)));
        l->tsched = l->tend = add_duration_to_time (tnow, 200 * T_MILLISECOND);
        unlock_lease (l);
        fh_insert (&gv.leaseheap, l);
        continue;
      }
    }

    unlock_lease (l);

    l->tsched = TSCHED_NOT_ON_HEAP;
    os_mutexUnlock (&gv.leaseheap_lock);

    switch (k)
    {
      case EK_PARTICIPANT:
        delete_participant (&g);
        break;
      case EK_PROXY_PARTICIPANT:
        delete_proxy_participant_by_guid (&g);
        break;
      case EK_WRITER:
        delete_writer_nolinger (&g);
        break;
      case EK_PROXY_WRITER:
        delete_proxy_writer (&g);
        break;
      case EK_READER:
        delete_reader (&g);
        break;
      case EK_PROXY_READER:
        delete_proxy_reader (&g);
        break;
    }

    os_mutexLock (&gv.leaseheap_lock);
  }
  os_mutexUnlock (&gv.leaseheap_lock);
}

/******/

static void debug_print_rawdata (const char *msg, const void *data, int len)
{
  const unsigned char *c = data;
  int i;
  TRACE (("%s<", msg));
  for (i = 0; i < len; i++)
  {
    if (32 < c[i] && c[i] <= 127)
      TRACE ((" %c", (i > 0 && (i%4) == 0) ? " " : "", c[i]));
    else
      TRACE (("%s\\x%02x", (i > 0 && (i%4) == 0) ? " " : "", c[i]));
  }
  TRACE ((">"));
}

void handle_PMD (UNUSED_ARG (const struct receiver_state *rst), unsigned statusinfo, const void *vdata, int len)
{
  const struct CDRHeader *data = vdata; /* built-ins not deserialized (yet) */
  const int bswap = (data->identifier == CDR_LE) ^ PLATFORM_IS_LITTLE_ENDIAN;
  struct proxy_participant *pp;
  nn_guid_t ppguid;
  TRACE ((" PMD ST%x", statusinfo));
  if (data->identifier != CDR_LE && data->identifier != CDR_BE)
  {
    TRACE ((" PMD data->identifier %d !?\n", ntohs (data->identifier)));
    return;
  }
  switch (statusinfo & (NN_STATUSINFO_DISPOSE | NN_STATUSINFO_UNREGISTER))
  {
    case 0:
      if (offsetof (ParticipantMessageData_t, value) > len - sizeof (struct CDRHeader))
        debug_print_rawdata (" SHORT1", data, len);
      else
      {
        const ParticipantMessageData_t *pmd = (ParticipantMessageData_t *) (data + 1);
        nn_guid_prefix_t p = nn_ntoh_guid_prefix (pmd->participantGuidPrefix);
        unsigned kind = ntohl (pmd->kind);
        unsigned length = bswap ? bswap4u (pmd->length) : pmd->length;
        TRACE ((" pp %x:%x:%x kind %u data %d", p.u[0], p.u[1], p.u[2], kind, length));
        if (len - sizeof (struct CDRHeader) - offsetof (ParticipantMessageData_t, value) < length)
          debug_print_rawdata (" SHORT2", pmd->value, len - sizeof (struct CDRHeader) - offsetof (ParticipantMessageData_t, value));
        else
          debug_print_rawdata ("", pmd->value, length);
        ppguid.prefix = p;
        ppguid.entityid.u = NN_ENTITYID_PARTICIPANT;
        if ((pp = ephash_lookup_proxy_participant_guid (&ppguid)) == NULL)
          TRACE ((" PPunknown"));
        else if (!config.arrival_of_data_asserts_pp_and_ep_liveliness)
          lease_renew (pp->lease, now ());
      }
      break;

    case NN_STATUSINFO_DISPOSE:
    case NN_STATUSINFO_UNREGISTER:
    case NN_STATUSINFO_DISPOSE | NN_STATUSINFO_UNREGISTER:
      /* Serialized key; BE or LE doesn't matter as both fields are
         defined as octets.  */
      if (len < (int) (sizeof (struct CDRHeader) + sizeof (nn_guid_prefix_t)))
        debug_print_rawdata (" SHORT3", data, len);
      else
      {
        ppguid.prefix = nn_ntoh_guid_prefix (*((nn_guid_prefix_t *) (data + 1)));
        ppguid.entityid.u = NN_ENTITYID_PARTICIPANT;
        if (delete_proxy_participant_by_guid (&ppguid) < 0)
          TRACE ((" unknown"));
        else
          TRACE ((" delete"));
      }
      break;
  }
  TRACE (("\n"));
}

/* SHA1 not available (unoffical build.) */

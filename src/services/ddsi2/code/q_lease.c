/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
#include <ctype.h>
#include <stddef.h>
#include <assert.h>

#include "os_heap.h"
#include "os_mutex.h"
#include "os_socket.h"
#include "os_if.h"
#include "os_atomics.h"

#include "ut_fibheap.h"

#include "ddsi_ser.h"
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

#include "sysdeps.h"

/* This is absolute bottom for signed integers, where -x = x and yet x
   != 0 -- and note that it had better be 2's complement machine! */
#define TSCHED_NOT_ON_HEAP INT64_MIN

struct lease {
  ut_fibheapNode_t heapnode;
  nn_etime_t tsched;  /* access guarded by leaseheap_lock */
  nn_etime_t tend;    /* access guarded by lock_lease/unlock_lease */
  os_int64 tdur;       /* constant */
  struct entity_common *entity; /* constant */
};

static int compare_lease_tsched (const void *va, const void *vb);

static const ut_fibheapDef_t lease_fhdef = UT_FIBHEAPDEF_INITIALIZER(offsetof (struct lease, heapnode), compare_lease_tsched);

static int compare_lease_tsched (const void *va, const void *vb)
{
  const struct lease *a = va;
  const struct lease *b = vb;
  return (a->tsched.v == b->tsched.v) ? 0 : (a->tsched.v < b->tsched.v) ? -1 : 1;
}

void lease_management_init (void)
{
  int i;
  os_mutexInit (&gv.leaseheap_lock, NULL);
  for (i = 0; i < N_LEASE_LOCKS; i++)
    os_mutexInit (&gv.lease_locks[i], NULL);
  ut_fibheapInit (&lease_fhdef, &gv.leaseheap);
}

void lease_management_term (void)
{
  int i;
  assert (ut_fibheapMin (&lease_fhdef, &gv.leaseheap) == NULL);
  for (i = 0; i < N_LEASE_LOCKS; i++)
    os_mutexDestroy (&gv.lease_locks[i]);
  os_mutexDestroy (&gv.leaseheap_lock);
}

static os_mutex *lock_lease_addr (struct lease const * const l)
{
  os_uint32 u = (os_ushort) ((os_address) l >> 3);
  os_uint32 v = u * 0xb4817365;
  unsigned idx = v >> (32 - N_LEASE_LOCKS_LG2);
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
  TRACE (("lease_new(tdur %"PA_PRId64" guid %x:%x:%x:%x) @ %p\n", tdur, PGUID (e->guid), (void *) l));
  l->tdur = tdur;
  l->tend = add_duration_to_etime (now_et (), tdur);
  l->tsched.v = TSCHED_NOT_ON_HEAP;
  l->entity = e;
  return l;
}

void lease_register (struct lease *l)
{
  TRACE (("lease_register(l %p guid %x:%x:%x:%x)\n", (void *) l, PGUID (l->entity->guid)));
  assert (l->tsched.v == TSCHED_NOT_ON_HEAP);
  os_mutexLock (&gv.leaseheap_lock);
  lock_lease (l);
  l->tsched = l->tend;
  unlock_lease (l);
  ut_fibheapInsert (&lease_fhdef, &gv.leaseheap, l);
  os_mutexUnlock (&gv.leaseheap_lock);
}

void lease_free (struct lease *l)
{
  TRACE (("lease_free(l %p guid %x:%x:%x:%x)\n", (void *) l, PGUID (l->entity->guid)));
  if (l->tsched.v != TSCHED_NOT_ON_HEAP)
  {
    os_mutexLock (&gv.leaseheap_lock);
    ut_fibheapDelete (&lease_fhdef, &gv.leaseheap, l);
    os_mutexUnlock (&gv.leaseheap_lock);
  }
  os_free (l);
}

void lease_renew (struct lease *l, nn_etime_t tnowE)
{
  nn_etime_t tend_new = add_duration_to_etime (tnowE, l->tdur);
  int did_update;
  lock_lease (l);
  if (tend_new.v <= l->tend.v)
    did_update = 0;
  else
  {
    l->tend = tend_new;
    did_update = 1;
  }
  unlock_lease (l);

  if ((config.enabled_logcats & LC_TRACE) && did_update)
  {
    int tusec;
    os_int64 tsec;
    TRACE ((" L("));
    if (l->entity->guid.entityid.u == NN_ENTITYID_PARTICIPANT)
      TRACE ((":%x", l->entity->guid.entityid.u));
    else
      TRACE (("%x:%x:%x:%x", PGUID (l->entity->guid)));
    etime_to_sec_usec (&tsec, &tusec, tend_new);
    TRACE ((" %"PA_PRId64".%06d)", tsec, tusec));
  }
}

void check_and_handle_lease_expiration (UNUSED_ARG (struct thread_state1 *self), nn_etime_t tnowE)
{
  struct lease *l;
  os_mutexLock (&gv.leaseheap_lock);
  while ((l = ut_fibheapMin (&lease_fhdef, &gv.leaseheap)) != NULL && l->tsched.v <= tnowE.v)
  {
    nn_guid_t g = l->entity->guid;
    enum entity_kind k = l->entity->kind;

    assert (l->tsched.v != TSCHED_NOT_ON_HEAP);
    ut_fibheapExtractMin (&lease_fhdef, &gv.leaseheap);

    lock_lease (l);
    if (tnowE.v < l->tend.v)
    {
      l->tsched = l->tend;
      unlock_lease (l);
      ut_fibheapInsert (&lease_fhdef, &gv.leaseheap, l);
      continue;
    }

    TRACE (("lease expired: l %p guid %x:%x:%x:%x tend %"PA_PRId64" < now %"PA_PRId64"\n", (void *) l, PGUID (g), l->tend.v, tnowE.v));

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
        l->tsched = l->tend = add_duration_to_etime (tnowE, 200 * T_MILLISECOND);
        unlock_lease (l);
        ut_fibheapInsert (&lease_fhdef, &gv.leaseheap, l);
        continue;
      }
    }

    unlock_lease (l);

    l->tsched.v = TSCHED_NOT_ON_HEAP;
    os_mutexUnlock (&gv.leaseheap_lock);

    switch (k)
    {
      case EK_PARTICIPANT:
        delete_participant (&g);
        break;
      case EK_PROXY_PARTICIPANT:
        delete_proxy_participant_by_guid (&g, 1);
        break;
      case EK_WRITER:
        delete_writer_nolinger (&g);
        break;
      case EK_PROXY_WRITER:
        delete_proxy_writer (&g, 1);
        break;
      case EK_READER:
        delete_reader (&g);
        break;
      case EK_PROXY_READER:
        delete_proxy_reader (&g, 1);
        break;
    }

    os_mutexLock (&gv.leaseheap_lock);
  }
  os_mutexUnlock (&gv.leaseheap_lock);
}

/******/

static void debug_print_rawdata (const char *msg, const void *data, size_t len)
{
  const unsigned char *c = data;
  size_t i;
  TRACE (("%s<", msg));
  for (i = 0; i < len; i++)
  {
    if (32 < c[i] && c[i] <= 127)
      TRACE (("%s%c", (i > 0 && (i%4) == 0) ? " " : "", c[i]));
    else
      TRACE (("%s\\x%02x", (i > 0 && (i%4) == 0) ? " " : "", c[i]));
  }
  TRACE ((">"));
}

void handle_PMD (UNUSED_ARG (const struct receiver_state *rst), unsigned statusinfo, const void *vdata, unsigned len)
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
        else
        {
          /* Renew lease if arrival of this message didn't already do so, also renew the lease
             of the virtual participant used for DS-discovered endpoints */
          if (!config.arrival_of_data_asserts_pp_and_ep_liveliness)
            lease_renew (pa_ldvoidp (&pp->lease), now_et ());
        }
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
        if (delete_proxy_participant_by_guid (&ppguid, 0) < 0)
          TRACE ((" unknown"));
        else
          TRACE ((" delete"));
      }
      break;
  }
  TRACE (("\n"));
}

/* SHA1 not available (unoffical build.) */

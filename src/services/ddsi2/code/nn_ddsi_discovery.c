#ifndef _REENTRANT
#define _REENTRANT 1
#endif

#include <ctype.h>
#include <stddef.h>

#include "os_heap.h"
#include "os_mutex.h"
#include "os_cond.h"
#include "os_thread.h"
#include "os_socket.h"
#include "os_if.h"

#include "v_group.h"
#include "v_partition.h"
#include "v_groupSet.h"
#include "v_entity.h"

/* for printing message header info */
#include "v_state.h"
#include "v_topic.h"

#include "nn_avl.h"
#include "nn_osplser.h"
#include "nn_protocol.h"
#include "nn_rtps.h"
#include "nn_misc.h"

#ifndef OS_WIN32_IF_H
#include <netdb.h>
#endif

#include "nn_config.h"
#include "nn_log.h"

#include "nn_mlv.h"
#include "nn_plist.h"
#include "nn_unused.h"
#include "nn_groupset.h"
#include "nn_bswap.h"
#include "nn_lat_estim.h"
#include "nn_bitset.h"
#include "nn_xevent.h"
#include "nn_addrset.h"
#include "nn_ddsi_discovery.h"
#include "nn_radmin.h"

#include "sysdeps.h"

#define ALIGN4(x) (((x) + 3) & -4)

#define PGUIDPREFIX(gp) (gp).u[0], (gp).u[1], (gp).u[2]
#define PGUID(g) PGUIDPREFIX ((g).prefix), (g).entityid.u

struct addrset;
struct xevent;
struct xeventq;
struct proxy_endpoint_common;

#include "nn_rtps_private.h"

static int assert_prd_liveliness_based_on_pp (void *vnode, void *varg)
{
  /* EP liveliness shouldn't be asserted when SPDP gets notified of
     the pp's existence, I think; calling avl_augment_update all the
     time is also not really smart ... */
  struct proxy_reader *prd = vnode;
  const os_int64 *tnow = varg;
  prd->c.tlease_end = add_duration_to_time (*tnow, prd->c.tlease_dur);
  avl_augment_update (&proxyrdtree, prd);
  return AVLWALK_CONTINUE;
}

int assert_pwr_liveliness_based_on_pp (void *vnode, void *varg)
{
  /* see assert_prd_liveliness_based_on_pp() */
  struct proxy_writer *pwr = vnode;
  const os_int64 *tnow = varg;
  pwr->c.tlease_end = add_duration_to_time (*tnow, pwr->c.tlease_dur);
  avl_augment_update (&proxywrtree, pwr);
  return AVLWALK_CONTINUE;
}

void assert_pp_and_all_ep_liveliness (struct proxy_participant *pp)
{
  nn_guid_t min, max;
  os_int64 tnow = now ();
  pp->tlease_end = add_duration_to_time (tnow, pp->tlease_dur);
  avl_augment_update (&proxypptree, pp);
  min = pp->guid; min.entityid.u = 0;
  max = pp->guid; max.entityid.u = ~0;
  avl_walkrange (&proxyrdtree, &min, &max, assert_prd_liveliness_based_on_pp, &tnow);
  avl_walkrange (&proxywrtree, &min, &max, assert_pwr_liveliness_based_on_pp, &tnow);
}

static void maybe_add_pp_as_meta_to_as_disc (const struct proxy_participant *pp)
{
  if (addrset_empty_mc (pp->as_meta))
  {
    nn_locator_udpv4_t loc;
    if (addrset_any_uc (pp->as_meta, &loc))
      add_to_addrset (as_disc, &loc);
  }
}

static int rebuild_as_disc_helper (void *vnode, UNUSED_ARG (void *varg))
{
  const struct proxy_participant *pp = vnode;
  maybe_add_pp_as_meta_to_as_disc (pp);
  return AVLWALK_CONTINUE;
}

static void rebuild_as_disc (void)
{
  nn_log (LC_TRACE, "rebuilding discovery address set\n");
  addrset_purge (as_disc);
  copy_addrset_into_addrset (as_disc, as_disc_init);
  avl_walk (&proxypptree, rebuild_as_disc_helper, NULL);
}

struct proxy_participant *new_proxy_participant (nn_guid_t guid, unsigned bes, struct addrset *as_default, struct addrset *as_meta, os_int64 tlease_dur, nn_vendorid_t vendor)
{
  struct proxy_participant *pp;
  avlparent_t parent;
  if (avl_lookup (&proxypptree, &guid, &parent) != NULL)
    abort ();
  pp = os_malloc (sizeof (*pp));
  avl_init_node (&pp->avlnode, parent);
  pp->guid = guid;
  pp->vendor = vendor;
  pp->refc = 1;
  pp->bes = bes;
  pp->as_default = ref_addrset (as_default);
  pp->as_meta = ref_addrset (as_meta);
  maybe_add_pp_as_meta_to_as_disc (pp);
  pp->tlease_dur = tlease_dur;
  pp->tlease_end = add_duration_to_time (now (), tlease_dur);
  avl_insert (&proxypptree, pp);
  return pp;
}

void augment_proxy_participant (void *vnode)
{
  struct proxy_participant *pp = vnode;
  os_int64 x = pp->tlease_end;
  nn_guid_t xid = pp->guid;
  if (pp->avlnode.left && pp->avlnode.left->min_tlease_end < x)
  {
    x = pp->avlnode.left->min_tlease_end;
    xid = pp->avlnode.left->guid_min_tlease_end;
  }
  if (pp->avlnode.right && pp->avlnode.right->min_tlease_end < x)
  {
    x = pp->avlnode.right->min_tlease_end;
    xid = pp->avlnode.right->guid_min_tlease_end;
  }
  pp->min_tlease_end = x;
  pp->guid_min_tlease_end = xid;
}

struct proxy_participant *ref_proxy_participant (struct proxy_participant *pp)
{
  if (pp)
    pp->refc++;
  return pp;
}

static int free_proxy_participant_helper (UNUSED_ARG_NDEBUG (void *vnode), UNUSED_ARG_NDEBUG (void *varg))
{
#ifndef NDEBUG
  struct proxy_endpoint_common *ep = vnode;
  const nn_guid_t *ppid = varg;
  assert (memcmp (&ppid->prefix, &ep->guid.prefix, sizeof (ep->guid.prefix)) == 0);
#endif
  return AVLWALK_DELETE | AVLWALK_CONTINUE;
}

void free_proxy_participant (void *vpp)
{
  struct proxy_participant *pp = vpp;
  nn_guid_t min, max;
  int tsec, tusec;
  time_to_sec_usec (&tsec, &tusec, now ());
  /* participant dead => endpoints dead - do this the inefficient way */
  nn_log (LC_DISCOVERY, "%d.%06d free_proxy_participant(%x:%x:%x:%x)\n", tsec, tusec, PGUID (pp->guid));
  min = max = pp->guid;
  min.entityid.u = 0;
  max.entityid.u = ~0;
  avl_walkrange (&proxyrdtree, &min, &max, free_proxy_participant_helper, &pp->guid);
  avl_walkrange (&proxywrtree, &min, &max, free_proxy_participant_helper, &pp->guid);
  if (addrset_empty_mc (pp->as_meta))
    rebuild_as_disc ();
  unref_addrset (pp->as_default);
  unref_addrset (pp->as_meta);
  assert (pp->refc == 1);
  os_free (pp);
}

void unref_proxy_participant (struct proxy_participant *pp)
{
  assert (pp->refc > 1);
  --pp->refc;
}

serdata_t construct_spdp_sample_alive (struct participant *pp)
{
  static const nn_vendorid_t myvendorid = MY_VENDOR_ID;
  serdata_t serdata;
  serstate_t serstate;
  struct nn_xmsg *mpayload;
  unsigned payload_sz;
  char *payload_blob;
  struct nn_locators_one def_uni_loc_one, def_multi_loc_one, meta_uni_loc_one, meta_multi_loc_one;
  nn_plist_t ps;
  nn_guid_t kh;

  /* First create a fake message for the payload: we can add plists to
     xmsgs easily, but not to serdata.  But it is rather easy to copy
     the payload of an xmsg over to a serdata ...  Expected size isn't
     terribly important, the msg will grow as needed, address space is
     essentially meaningless because we only use the message to
     construct the payload. */
  mpayload = nn_xmsg_new (xmsgpool, 0, &pp->guid.prefix, 0);

  nn_plist_init_empty (&ps);
  ps.present |= PP_PARTICIPANT_GUID | PP_BUILTIN_ENDPOINT_SET | PP_PROTOCOL_VERSION | PP_VENDORID | PP_DEFAULT_UNICAST_LOCATOR | PP_METATRAFFIC_UNICAST_LOCATOR | PP_PARTICIPANT_LEASE_DURATION;
  ps.participant_guid = pp->guid;
  ps.builtin_endpoint_set = pp->bes;
  ps.protocol_version.major = RTPS_MAJOR;
  ps.protocol_version.minor = RTPS_MINOR;
  ps.vendorid = myvendorid;
  ps.default_unicast_locators.n = 1;
  ps.default_unicast_locators.first =
    ps.default_unicast_locators.last = &def_uni_loc_one;
  ps.metatraffic_unicast_locators.n = 1;
  ps.metatraffic_unicast_locators.first =
    ps.metatraffic_unicast_locators.last = &meta_uni_loc_one;
  def_uni_loc_one.next = NULL;
  meta_uni_loc_one.next = NULL;
  if (config.many_sockets_mode)
  {
    def_uni_loc_one.loc = pp->sockloc;
    meta_uni_loc_one.loc = pp->sockloc;
  }
  else
  {
    def_uni_loc_one.loc = loc_default_uc;
    meta_uni_loc_one.loc = loc_meta_uc;
  }
  if (config.allowMulticast)
  {
    ps.present |= PP_DEFAULT_MULTICAST_LOCATOR | PP_METATRAFFIC_MULTICAST_LOCATOR;
    ps.default_multicast_locators.n = 1;
    ps.default_multicast_locators.first =
      ps.default_multicast_locators.last = &def_multi_loc_one;
    ps.metatraffic_multicast_locators.n = 1;
    ps.metatraffic_multicast_locators.first =
      ps.metatraffic_multicast_locators.last = &meta_multi_loc_one;
    def_multi_loc_one.next = NULL;
    def_multi_loc_one.loc = loc_default_mc;
    meta_multi_loc_one.next = NULL;
    meta_multi_loc_one.loc = loc_meta_mc;
  }
  ps.participant_lease_duration = nn_to_ddsi_duration (pp->lease_duration);
  if (nn_plist_addtomsg (mpayload, &ps, ~0u, ~0u) < 0 ||
      nn_xmsg_addpar_sentinel (mpayload) < 0)
  {
    nn_xmsg_free (mpayload);
    return NULL;
  }

  /* A NULL topic implies a parameter list, now that we do PMD through
     the serializer */
  serstate = serstate_new (serpool, NULL);
  payload_blob = nn_xmsg_payload (&payload_sz, mpayload);
  serstate_append_blob (serstate, 4, payload_sz, payload_blob);
  kh = nn_hton_guid (pp->guid);
  serstate_set_key (serstate, 0, 16, &kh);
  serstate_set_msginfo (serstate, 0, now (), NULL);
  serdata = serstate_fix (serstate);
  nn_xmsg_free (mpayload);
  return serdata;
}

serdata_t construct_spdp_sample_dead (struct participant *pp)
{
  struct nn_xmsg *mpayload;
  unsigned payload_sz;
  char *payload_blob;
  nn_plist_t ps;
  serdata_t serdata;
  serstate_t serstate;
  nn_guid_t kh;

  mpayload = nn_xmsg_new (xmsgpool, 0, &pp->guid.prefix, 0);
  nn_plist_init_empty (&ps);
  ps.present |= PP_PARTICIPANT_GUID;
  ps.participant_guid = pp->guid;
  if (nn_plist_addtomsg (mpayload, &ps, ~0u, ~0u) < 0 ||
      nn_xmsg_addpar_sentinel (mpayload) < 0)
  {
    nn_xmsg_free (mpayload);
    return NULL;
  }

  serstate = serstate_new (serpool, NULL);
  payload_blob = nn_xmsg_payload (&payload_sz, mpayload);
  serstate_append_blob (serstate, 4, payload_sz, payload_blob);
  kh = nn_hton_guid (pp->guid);
  serstate_set_key (serstate, 1, 16, &kh);
  serstate_set_msginfo (serstate, NN_STATUSINFO_DISPOSE | NN_STATUSINFO_UNREGISTER, now (), NULL);
  serdata = serstate_fix (serstate);
  nn_xmsg_free (mpayload);
  return serdata;
}

static int force_spdp_xmit (void *vnode, void *varg)
{
  struct participant *pp = vnode;
  os_int64 tsched = 0;
  if (!config.unicast_response_to_spdp_messages)
    resched_xevent_if_earlier (pp->spdp_xevent, tsched);
  else
  {
    struct proxy_participant *proxypp = varg;
    qxev_spdp (xevents, tsched, pp, proxypp);
  }
  return AVLWALK_CONTINUE;
}

static int handle_spdp_dead (const struct receiver_state *rst, nn_plist_t *qosp, nn_plist_t *datap)
{
  struct proxy_participant *pp;
  unsigned statusinfo;
  nn_guid_t guid;

  statusinfo = (qosp->present & PP_STATUSINFO) ? qosp->statusinfo : 0;
  assert (statusinfo != 0);

  if (datap->present & PP_PARTICIPANT_GUID)
  {
    guid = datap->participant_guid;
    assert (guid.entityid.u == NN_ENTITYID_PARTICIPANT);
  }
  else
  {
    if (datap->present == 0 && datap->qos.present == 0)
      nn_log (LC_TRACE, "no payload? ");
    else
      nn_log (LC_TRACE, "unexpected payload ");
    if (NN_STRICT_P)
    {
      nn_log (LC_WARNING, "data(SPDP, vendor %d.%d): no/invalid payload\n", rst->vendor.id[0], rst->vendor.id[1]);
      return 0;
    }

    if (qosp->present & PP_KEYHASH)
    {
      nn_guid_t *raw_guid = (nn_guid_t *) qosp->keyhash.value;
      guid = nn_ntoh_guid (*raw_guid);
      if (guid.entityid.u != NN_ENTITYID_PARTICIPANT)
      {
        nn_log (LC_TRACE, "keyhash not a valid participant guid");
        return 0;
      }
    }
    else
    {
      nn_log (LC_TRACE, "no keyhash?");
      return 0;
    }
  }

  nn_log (LC_TRACE, "ST%x/%x:%x:%x:%x", statusinfo, PGUID (guid));
  if ((pp = avl_lookup (&proxypptree, &guid, NULL)) == NULL)
    nn_log (LC_TRACE, " unknown");
  else
  {
    nn_log (LC_TRACE, " sched_delete");
    pp->tlease_end = 0;
    avl_augment_update (&proxypptree, pp);
  }
  return 0;
}

static int add_disc_spdp_participant_writer (void *vnode, void *varg)
{
  struct participant *pp = vnode;
  struct proxy_writer *pwr = varg;
  if (pp->spdp_pp_reader)
    add_proxy_writer_to_reader (pp->spdp_pp_reader, pwr);
  return AVLWALK_CONTINUE;
}

static int add_disc_spdp_participant_reader (void *vnode, void *varg)
{
  struct participant *pp = vnode;
  struct proxy_reader *prd = varg;
  add_proxy_reader_to_writer (pp->spdp_pp_writer, prd);
  return AVLWALK_CONTINUE;
}

static int add_disc_sedp_writer_writer (void *vnode, void *varg)
{
  struct participant *pp = vnode;
  struct proxy_writer *pwr = varg;
  if (pp->sedp_writer_reader)
    add_proxy_writer_to_reader (pp->sedp_writer_reader, pwr);
  return AVLWALK_CONTINUE;
}

static int add_disc_sedp_writer_reader (void *vnode, void *varg)
{
  struct participant *pp = vnode;
  struct proxy_reader *prd = varg;
  add_proxy_reader_to_writer (pp->sedp_writer_writer, prd);
  return AVLWALK_CONTINUE;
}

static int add_disc_sedp_reader_writer (void *vnode, void *varg)
{
  struct participant *pp = vnode;
  struct proxy_writer *pwr = varg;
  if (pp->sedp_reader_reader)
    add_proxy_writer_to_reader (pp->sedp_reader_reader, pwr);
  return AVLWALK_CONTINUE;
}

static int add_disc_sedp_reader_reader (void *vnode, void *varg)
{
  struct participant *pp = vnode;
  struct proxy_reader *prd = varg;
  add_proxy_reader_to_writer (pp->sedp_reader_writer, prd);
  return AVLWALK_CONTINUE;
}

static int add_p2p_ppmsg_writer (void *vnode, void *varg)
{
  struct participant *pp = vnode;
  struct proxy_writer *pwr = varg;
  if (pp->participant_message_reader)
    add_proxy_writer_to_reader (pp->participant_message_reader, pwr);
  return AVLWALK_CONTINUE;
}

static int add_p2p_ppmsg_reader (void *vnode, void *varg)
{
  struct participant *pp = vnode;
  struct proxy_reader *prd = varg;
  add_proxy_reader_to_writer (pp->participant_message_writer, prd);
  return AVLWALK_CONTINUE;
}

static int handle_spdp_alive (const struct receiver_state *rst, UNUSED_ARG_NDEBUG (nn_plist_t *qosp), nn_plist_t *datap)
{
  struct addrset *as_meta, *as_default;
  struct proxy_participant *pp;
  const char *stat = "";
  int ignore = 0;

  assert (!(qosp->present & PP_STATUSINFO) || qosp->statusinfo == 0);

  if (datap->present == 0 && datap->qos.present == 0)
  {
    nn_log (LC_TRACE, "no payload?");
    return 0;
  }

  if (!(datap->present & PP_PARTICIPANT_GUID) ||
      !(datap->present & PP_BUILTIN_ENDPOINT_SET) ||
      !(datap->present & PP_DEFAULT_UNICAST_LOCATOR) ||
      !(datap->present & PP_METATRAFFIC_UNICAST_LOCATOR))
  {
    nn_log (LC_TRACE, "parameters missing?");
    return 0;
  }

  /* At some point the RTI implementation didn't mention
     BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER & ...WRITER, or
     so it seemed; and yet they are necessary for correct operation,
     so add them. */
  if (vendor_is_rti (rst->vendor) &&
      ((datap->builtin_endpoint_set &
        (NN_BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER |
         NN_BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER))
       != (NN_BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER |
           NN_BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER)) &&
      config.assume_rti_has_pmd_endpoints)
  {
    nn_log (LC_WARNING, "data(SPDP, vendor %d.%d): assuming unadvertised PMD endpoints do exist\n");
    datap->builtin_endpoint_set |=
      NN_BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER |
      NN_BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER;
  }
  if (avl_lookup (&pptree, &datap->participant_guid, NULL))
  {
    stat = " (local)";
    ignore = 1;
  }
  if ((pp = avl_lookup (&proxypptree, &datap->participant_guid, NULL)) != NULL)
  {
    stat = " (known - FIXME updating lease)";
    assert_pp_and_all_ep_liveliness (pp);
    ignore = 1;
  }
  nn_log (LC_TRACE, "%x:%x:%x:%x bes %x%s", PGUID (datap->participant_guid), datap->builtin_endpoint_set, stat);
  if (ignore)
    return 1;

  nn_log (LC_TRACE, " NEW");
  if (!(datap->present & PID_PARTICIPANT_LEASE_DURATION))
  {
    nn_log (LC_TRACE, " (PARTICIPANT_LEASE_DURATION defaulting to 11s)");
    datap->participant_lease_duration = nn_to_ddsi_duration (11 * T_SECOND);
    datap->present |= PID_PARTICIPANT_LEASE_DURATION;
  }

  {
    nn_locator_udpv4_t loc;
    as_default = new_addrset ();
    as_meta = new_addrset ();
    if (get_udpv4_locator (&loc, &datap->default_unicast_locators))
      add_to_addrset (as_default, &loc);
    if (get_udpv4_locator (&loc, &datap->metatraffic_unicast_locators))
      add_to_addrset (as_meta, &loc);
    if (config.allowMulticast)
    {
      if ((datap->present & PP_DEFAULT_MULTICAST_LOCATOR) &&
          get_udpv4_locator (&loc, &datap->default_multicast_locators))
        add_to_addrset (as_default, &loc);
      if ((datap->present & PP_METATRAFFIC_MULTICAST_LOCATOR) &&
          get_udpv4_locator (&loc, &datap->metatraffic_multicast_locators))
        add_to_addrset (as_meta, &loc);
    }
    nn_log_addrset (LC_TRACE, " (def", as_default);
    nn_log_addrset (LC_TRACE, " meta", as_meta);
    nn_log (LC_TRACE, ")");
  }

  pp = new_proxy_participant (datap->participant_guid, datap->builtin_endpoint_set, as_default, as_meta, nn_from_ddsi_duration (datap->participant_lease_duration), rst->vendor);

  /* Add proxy endpoints based on the advertised (& possibly augmented
     ...) built-in endpoint set. */
  {
#define TE(ap_, a_, bp_, b_, af_) { NN_##ap_##BUILTIN_ENDPOINT_##a_, NN_ENTITYID_##bp_##_BUILTIN_##b_, af_ }
    static const struct bestab {
      unsigned besflag;
      unsigned entityid;
      avlwalk_fun_t add_endpoint_function;
    } bestab[] = {
      TE (DISC_, PARTICIPANT_ANNOUNCER, SPDP, PARTICIPANT_WRITER, add_disc_spdp_participant_writer),
      TE (DISC_, PARTICIPANT_DETECTOR, SPDP, PARTICIPANT_READER, add_disc_spdp_participant_reader),
      TE (DISC_, PUBLICATION_ANNOUNCER, SEDP, PUBLICATIONS_WRITER, add_disc_sedp_writer_writer),
      TE (DISC_, PUBLICATION_DETECTOR, SEDP, PUBLICATIONS_READER, add_disc_sedp_writer_reader),
      TE (DISC_, SUBSCRIPTION_ANNOUNCER, SEDP, SUBSCRIPTIONS_WRITER, add_disc_sedp_reader_writer),
      TE (DISC_, SUBSCRIPTION_DETECTOR, SEDP, SUBSCRIPTIONS_READER, add_disc_sedp_reader_reader),
      TE (, PARTICIPANT_MESSAGE_DATA_WRITER, P2P, PARTICIPANT_MESSAGE_WRITER, add_p2p_ppmsg_writer),
      TE (, PARTICIPANT_MESSAGE_DATA_READER, P2P, PARTICIPANT_MESSAGE_READER, add_p2p_ppmsg_reader)
    };
#undef TE
    int i;
    for (i = 0; i < (int) (sizeof (bestab) / sizeof (*bestab)); i++)
    {
      const struct bestab *te = &bestab[i];
      if (pp->bes & te->besflag)
      {
        nn_guid_t guid1;
        guid1.prefix = pp->guid.prefix;
        guid1.entityid.u = te->entityid;
        if (is_writer_entityid (guid1.entityid))
        {
          struct proxy_writer *pwr;
          nn_xqos_t *xqos = os_malloc (sizeof (*xqos));
          nn_xqos_copy (xqos, &builtin_endpoint_xqos_wr);
          pwr = new_proxy_writer (guid1, pp, pp->as_meta, xqos);
          avl_walk (&pptree, te->add_endpoint_function, pwr);
        }
        else
        {
          struct proxy_reader *prd;
          nn_xqos_t *xqos = os_malloc (sizeof (*xqos));
          nn_xqos_copy (xqos, &builtin_endpoint_xqos_rd);
          prd = new_proxy_reader (guid1, pp, pp->as_meta, xqos);
          avl_walk (&pptree, te->add_endpoint_function, prd);
        }
      }
    }
  }
  unref_addrset (as_meta);
  unref_addrset (as_default);

  /* Force transmission of SPDP messages */
  avl_walk (&pptree, force_spdp_xmit, pp);
  return 1;
}

static int spdp_decode_plist (const struct receiver_state *rst, nn_plist_t *pl, char *base, int off, int len, int isqos)
{
  if (off == 0 || len == 0)
    nn_plist_init_empty (pl);
  else
  {
    nn_plist_src_t src;
    src.protocol_version = rst->protocol_version;
    src.vendorid = rst->vendor;
    switch (isqos)
    {
      case 0:
        src.encoding = ((const struct CDRHeader *) (base + off))->identifier;
        src.buf = base + off + 4;
        src.bufsz = len - 4;
        break;
      case 1:
        src.encoding = PL_CDR_BE;
        src.buf = base + off;
        src.bufsz = len;
        break;
      case 2:
        src.encoding = PL_CDR_LE;
        src.buf = base + off;
        src.bufsz = len;
        break;
    }
    if (nn_plist_init_frommsg (pl, NULL, ~0u, ~0u, &src) < 0)
    {
      nn_log (LC_WARNING, "data(SPDP, vendor %d.%d): invalid %s\n",
              rst->vendor.id[0], rst->vendor.id[1],
              isqos ? "inline qos" : "parameter list");
      return 0;
    }
  }
  return 1;
}

int nn_spdp_dqueue_handler (const struct nn_rsample_info *sampleinfo, const struct nn_rdata *fragchain, UNUSED_ARG (void *qarg))
{
  const struct receiver_state *rst = sampleinfo->rst;
  const Data_DataFrag_common_t *msg;
  int qos_offset, endianness_flag;
  nn_plist_t qos, decoded_data;

  /* header is (partially) byteswapped already (if needed) */
  msg = (Data_DataFrag_common_t *) NN_RMSG_PAYLOADOFF (fragchain->rmsg, NN_RDATA_SUBMSG_OFF (fragchain));
  if (msg->smhdr.flags & DATA_FLAG_INLINE_QOS)
    qos_offset = NN_RDATA_SUBMSG_OFF (fragchain) + offsetof (Data_DataFrag_common_t, octetsToInlineQos) + sizeof (msg->octetsToInlineQos) + msg->octetsToInlineQos;
  else
    qos_offset = 0;
  endianness_flag = (msg->smhdr.flags & SMFLAG_ENDIANNESS) ? 1 : 0;

  /* SPDP is in special mode, not fragmented, and no gaps */
  assert (fragchain->min == 0);
  assert (fragchain->maxp1 == sampleinfo->size);
  assert (fragchain->nextfrag == NULL);
  assert (NN_RDATA_PAYLOAD_OFF (fragchain) > NN_RDATA_SUBMSG_OFF (fragchain));

  if (!spdp_decode_plist (rst, &qos, NN_RMSG_PAYLOAD (fragchain->rmsg), qos_offset, NN_RDATA_PAYLOAD_OFF (fragchain) - qos_offset, 1 + endianness_flag))
  {
    return 0;
  }
  if (!spdp_decode_plist (rst, &decoded_data, NN_RMSG_PAYLOAD (fragchain->rmsg), NN_RDATA_PAYLOAD_OFF (fragchain), fragchain->maxp1, 0))
  {
    return 0;
  }

  os_mutexLock (&lock);
  nn_log (LC_TRACE, "SPDP ");
  if ((qos.present & PP_STATUSINFO) && qos.statusinfo != 0)
    handle_spdp_dead (rst, &qos, &decoded_data);
  else
    handle_spdp_alive (rst, &qos, &decoded_data);
  nn_log (LC_TRACE, "\n");
  os_mutexUnlock (&lock);

  nn_plist_fini (&decoded_data);
  nn_plist_fini (&qos);
  return 0;
}

#include <ctype.h>
#include <stddef.h>
#include <assert.h>

#include "os_heap.h"
#include "os_mutex.h"
#include "os_socket.h"
#include "os_if.h"
#include "os_version.h"
#include "os_gitrev.h"
#include "os_hosttarget.h"

#include "ut_avl.h"
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
#include "q_error.h"

#include "sysdeps.h"

void nn_loc_to_address (os_sockaddr_storage *dst, const nn_locator_t *src)
{
  memset (dst, 0, sizeof (*dst));
  switch (src->kind)
  {
    case NN_LOCATOR_KIND_UDPv4:
    case NN_LOCATOR_KIND_TCPv4:
    {
      os_sockaddr_in *x = (os_sockaddr_in *) dst;
      x->sin_family = AF_INET;
      x->sin_port = htons (src->port);
      memcpy (&x->sin_addr.s_addr, src->address + 12, 4);
      break;
    }
#if OS_SOCKET_HAS_IPV6
    case NN_LOCATOR_KIND_UDPv6:
    case NN_LOCATOR_KIND_TCPv6:
    {
      os_sockaddr_in6 *x = (os_sockaddr_in6 *) dst;
      memset (x, 0, sizeof (*x));
      x->sin6_family = AF_INET6;
      x->sin6_port = htons (src->port);
      memcpy (&x->sin6_addr.s6_addr, src->address, 16);
      if (IN6_IS_ADDR_LINKLOCAL (&x->sin6_addr))
      {
        x->sin6_scope_id = gv.interfaceNo;
      }
      break;
    }
#endif
    default:
      break;
  }
}

static void nn_address_to_loc (nn_locator_t *dst, const os_sockaddr_storage *src)
{
  memset (dst, 0, sizeof (*dst));
  switch (src->ss_family)
  {
    case AF_INET:
    {
      const os_sockaddr_in *x = (const os_sockaddr_in *) src;
      dst->kind = gv.m_factory->m_kind;
      dst->port = ntohs (x->sin_port);
      memcpy (dst->address + 12, &x->sin_addr.s_addr, 4);
      break;
    }
#if OS_SOCKET_HAS_IPV6
    case AF_INET6:
    {
      const os_sockaddr_in6 *x = (const os_sockaddr_in6 *) src;
      dst->kind = gv.m_factory->m_kind;
      dst->port = ntohs (x->sin6_port);
      memcpy (dst->address, &x->sin6_addr.s6_addr, 16);
      break;
    }
#endif
    default:
      NN_FATAL1 ("nn_address_to_loc: family %d unsupported\n", (int) src->ss_family);
  }
}

static int get_address (os_sockaddr_storage * loc, const nn_locators_t * locs)
{
  struct nn_locators_one * l;
  os_sockaddr_storage first, samenet;
  int first_set = 0, samenet_set = 0;
  memset (&first, 0, sizeof (first));
  memset (&samenet, 0, sizeof (samenet));

  /* Preferably an (the first) address that matches a network we are
     on; if none does, pick the first. No multicast locator ever will
     match, so the first one will be used. */

  for (l = locs->first; l != NULL; l = l->next)
  {
    os_sockaddr_storage tmp;
    int i;

    /* Skip locators of the wrong kind */

    if (! ddsi_factory_supports (gv.m_factory, l->loc.kind))
    {
      continue;
    }

    nn_loc_to_address (&tmp, &l->loc);

    if (l->loc.kind == NN_LOCATOR_KIND_UDPv4 && ((os_sockaddr_in *) &gv.extip)->sin_addr.s_addr != 0)
    {
      /* If the examined locator is in the same subnet as our own
         external IP address, this locator will be translated into one
         in the same subnet as our own local ip and selected. */

      os_sockaddr_in *tmp4 = (os_sockaddr_in *) &tmp;
      const os_sockaddr_in *ownip = (os_sockaddr_in *) &gv.ownip;
      const os_sockaddr_in *extip = (os_sockaddr_in *) &gv.extip;

      if ((tmp4->sin_addr.s_addr & gv.extmask.s_addr) == (extip->sin_addr.s_addr & gv.extmask.s_addr))
      {
        /* translate network part of the IP address from the external
           one to the internal one */
        tmp4->sin_addr.s_addr =
          (tmp4->sin_addr.s_addr & ~gv.extmask.s_addr) | (ownip->sin_addr.s_addr & gv.extmask.s_addr);
        *loc = tmp;
        return 1;
      }
    }

#if OS_SOCKET_HAS_IPV6
    if ((l->loc.kind == NN_LOCATOR_KIND_UDPv6) || (l->loc.kind == NN_LOCATOR_KIND_TCPv6))
    {
      /* We (cowardly) refuse to accept advertised link-local
         addresses unles we're in "link-local" mode ourselves.  Then
         we just hope for the best.  */
      if (!gv.ipv6_link_local && IN6_IS_ADDR_LINKLOCAL (&((os_sockaddr_in6 *) &tmp)->sin6_addr))
        continue;
    }
#endif

    if (!first_set)
    {
      first = tmp;
      first_set = 1;
    }
    for (i = 0; i < gv.n_interfaces; i++)
    {
      if (os_sockaddrSameSubnet ((os_sockaddr *) &tmp, (os_sockaddr *) &gv.interfaces[i].addr, (os_sockaddr *) &gv.interfaces[i].netmask))
      {
        if (os_sockaddrIPAddressEqual ((os_sockaddr*) &gv.interfaces[i].addr, (os_sockaddr*) &gv.ownip))
        {
          /* matches the preferred interface -> the very best situation */
          *loc = tmp;
          return 1;
        }
        else if (!samenet_set)
        {
          /* on a network we're connected to */
          samenet = tmp;
          samenet_set = 1;
        }
      }
    }
  }
  if (samenet_set)
  {
    /* prefer a directly connected network */

    *loc = samenet;
    return 1;
  } 
  else if (first_set) 
  {
    /* else any address we found will have to do */

    *loc = first;
    return 1;
  }
  return 0;
}

/******************************************************************************
 ***
 *** SPDP
 ***
 *****************************************************************************/

static void maybe_add_pp_as_meta_to_as_disc (const struct addrset *as_meta)
{
  if (addrset_empty_mc (as_meta))
  {
    os_sockaddr_storage addr;
    if (addrset_any_uc (as_meta, &addr))
      add_to_addrset (gv.as_disc, &addr);
  }
}

int spdp_write (struct participant *pp)
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
  struct writer *wr;
  os_size_t size;
  char node[64];

  TRACE (("spdp_write(%x:%x:%x:%x)\n", PGUID (pp->e.guid)));

  if ((wr = get_builtin_writer (pp, NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER)) == NULL)
  {
    TRACE (("spdp_write(%x:%x:%x:%x) - builtin participant writer not found\n", PGUID (pp->e.guid)));
    return 0;
  }

  /* First create a fake message for the payload: we can add plists to
     xmsgs easily, but not to serdata.  But it is rather easy to copy
     the payload of an xmsg over to a serdata ...  Expected size isn't
     terribly important, the msg will grow as needed, address space is
     essentially meaningless because we only use the message to
     construct the payload. */
  mpayload = nn_xmsg_new (gv.xmsgpool, &pp->e.guid.prefix, 0, NN_XMSG_KIND_DATA);

  nn_plist_init_empty (&ps);
  ps.present |= PP_PARTICIPANT_GUID | PP_BUILTIN_ENDPOINT_SET | 
    PP_PROTOCOL_VERSION | PP_VENDORID | PP_PARTICIPANT_LEASE_DURATION;
  ps.participant_guid = pp->e.guid;
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
    def_uni_loc_one.loc = pp->m_locator;
    meta_uni_loc_one.loc = pp->m_locator;
  }
  else
  {
    def_uni_loc_one.loc = gv.loc_default_uc;
    meta_uni_loc_one.loc = gv.loc_meta_uc;
  }

  if (config.publish_uc_locators)
  {
    ps.present |= PP_DEFAULT_UNICAST_LOCATOR | PP_METATRAFFIC_UNICAST_LOCATOR;
    ps.aliased |= PP_DEFAULT_UNICAST_LOCATOR | PP_METATRAFFIC_UNICAST_LOCATOR;
  }

  if (config.allowMulticast)
  {
    ps.present |= PP_DEFAULT_MULTICAST_LOCATOR | PP_METATRAFFIC_MULTICAST_LOCATOR;
    ps.aliased |= PP_DEFAULT_MULTICAST_LOCATOR | PP_METATRAFFIC_MULTICAST_LOCATOR;
    ps.default_multicast_locators.n = 1;
    ps.default_multicast_locators.first =
      ps.default_multicast_locators.last = &def_multi_loc_one;
    ps.metatraffic_multicast_locators.n = 1;
    ps.metatraffic_multicast_locators.first =
      ps.metatraffic_multicast_locators.last = &meta_multi_loc_one;
    def_multi_loc_one.next = NULL;
    def_multi_loc_one.loc = gv.loc_default_mc;
    meta_multi_loc_one.next = NULL;
    meta_multi_loc_one.loc = gv.loc_meta_mc;
  }
  ps.participant_lease_duration = nn_to_ddsi_duration (pp->lease_duration);

  /* Add PrismTech specific version information */
  {
    ps.present |= PP_PRISMTECH_PARTICIPANT_VERSION_INFO;
    ps.prismtech_participant_version_info.version = 0;

    ps.prismtech_participant_version_info.flags = 0;
    ps.prismtech_participant_version_info.flags |= NN_PRISMTECH_FL_KERNEL_SEQUENCE_NUMBER;

    os_gethostname(node, sizeof(node)-1);
    node[sizeof(node)-1] = '\0';
    size = strlen(node) + strlen(OSPL_VERSION_STR) + strlen(OSPL_INNER_REV_STR) +
            strlen(OSPL_OUTER_REV_STR) + strlen(OSPL_HOST_STR) + strlen(OSPL_TARGET_STR) + 6; /* + /////'\0' */
    ps.prismtech_participant_version_info.internals = os_malloc(size);
    if (ps.prismtech_participant_version_info.internals) {
      snprintf(ps.prismtech_participant_version_info.internals, size, "%s/%s/%s/%s/%s/%s",
              node, OSPL_VERSION_STR, OSPL_INNER_REV_STR, OSPL_OUTER_REV_STR, OSPL_HOST_STR, OSPL_TARGET_STR);
      TRACE (("spdp_write(%x:%x:%x:%x) - internals: %s\n", PGUID (pp->e.guid), ps.prismtech_participant_version_info.internals));
    } else {
      ps.prismtech_participant_version_info.internals = "";
      ps.aliased |= PP_PRISMTECH_PARTICIPANT_VERSION_INFO;
    }
  }

  if (nn_plist_addtomsg (mpayload, &ps, ~0u, ~0u) < 0 ||
      nn_xmsg_addpar_sentinel (mpayload) < 0)
  {
    if (ps.prismtech_participant_version_info.internals != NULL &&
        strcmp(ps.prismtech_participant_version_info.internals,"") != 0)
      os_free(ps.prismtech_participant_version_info.internals);
    nn_xmsg_free (mpayload);
    return ERR_UNSPECIFIED;
  }

  /* A NULL topic implies a parameter list, now that we do PMD through
     the serializer */
  serstate = serstate_new (gv.serpool, NULL);
  payload_blob = nn_xmsg_payload (&payload_sz, mpayload);
  serstate_append_blob (serstate, 4, payload_sz, payload_blob);
  kh = nn_hton_guid (pp->e.guid);
  serstate_set_key (serstate, 0, 16, &kh);
  serstate_set_msginfo (serstate, 0, now (), 1, NULL);
  serdata = serstate_fix (serstate);
  nn_plist_fini(&ps);
  nn_xmsg_free (mpayload);

  return write_sample (NULL, wr, serdata);
}

int spdp_dispose_unregister (struct participant *pp)
{
  struct nn_xmsg *mpayload;
  unsigned payload_sz;
  char *payload_blob;
  nn_plist_t ps;
  serdata_t serdata;
  serstate_t serstate;
  nn_guid_t kh;
  struct writer *wr;

  if ((wr = get_builtin_writer (pp, NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER)) == NULL)
  {
    TRACE (("spdp_dispose_unregister(%x:%x:%x:%x) - builtin participant writer not found\n", PGUID (pp->e.guid)));
    return 0;
  }

  mpayload = nn_xmsg_new (gv.xmsgpool, &pp->e.guid.prefix, 0, NN_XMSG_KIND_DATA);
  nn_plist_init_empty (&ps);
  ps.present |= PP_PARTICIPANT_GUID;
  ps.participant_guid = pp->e.guid;
  if (nn_plist_addtomsg (mpayload, &ps, ~0u, ~0u) < 0 ||
      nn_xmsg_addpar_sentinel (mpayload) < 0)
  {
    nn_xmsg_free (mpayload);
    return ERR_UNSPECIFIED;
  }

  serstate = serstate_new (gv.serpool, NULL);
  payload_blob = nn_xmsg_payload (&payload_sz, mpayload);
  serstate_append_blob (serstate, 4, payload_sz, payload_blob);
  kh = nn_hton_guid (pp->e.guid);
  serstate_set_key (serstate, 1, 16, &kh);
  serstate_set_msginfo (serstate, NN_STATUSINFO_DISPOSE | NN_STATUSINFO_UNREGISTER, now (), 1, NULL);
  serdata = serstate_fix (serstate);
  nn_xmsg_free (mpayload);

  return write_sample (NULL, wr, serdata);
}

static unsigned pseudo_random_delay (const nn_guid_t *x, const nn_guid_t *y, os_int64 tnow)
{
  /* You know, an ordinary random generator would be even better, but
     the C library doesn't have a reentrant one and I don't feel like
     integrating, say, the Mersenne Twister right now. */
#define UINT64_CONST(x, y, z) (((os_uint64) (x) * 1000000 + (y)) * 1000000 + (z))
  static const os_uint64 cs[] = {
    UINT64_CONST (15385148,  50874, 689571),
    UINT64_CONST (17503036, 526311, 582379),
    UINT64_CONST (11075621, 958654, 396447),
    UINT64_CONST ( 9748227, 842331,  24047),
    UINT64_CONST (14689485, 562394, 710107),
    UINT64_CONST (17256284, 993973, 210745),
    UINT64_CONST ( 9288286, 355086, 959209),
    UINT64_CONST (17718429, 552426, 935775),
    UINT64_CONST (10054290, 541876, 311021),
    UINT64_CONST (13417933, 704571, 658407)
  };
#undef UINT64_CONST
  os_uint32 a = x->prefix.u[0], b = x->prefix.u[1], c = x->prefix.u[2], d = x->entityid.u;
  os_uint32 e = y->prefix.u[0], f = y->prefix.u[1], g = y->prefix.u[2], h = y->entityid.u;
  os_uint32 i = (os_uint64) tnow >> 32, j = (os_uint32) tnow;
  os_uint64 m = 0;
  m += (a + cs[0]) * (b + cs[1]);
  m += (c + cs[2]) * (d + cs[3]);
  m += (e + cs[4]) * (f + cs[5]);
  m += (g + cs[6]) * (h + cs[7]);
  m += (i + cs[8]) * (j + cs[9]);
  return m >> 32;
}

static void respond_to_spdp (const nn_guid_t *dest_proxypp_guid)
{
  struct ephash_enum_participant est;
  struct participant *pp;
  os_int64 tnow = now ();
  ephash_enum_participant_init (&est);
  while ((pp = ephash_enum_participant_next (&est)) != NULL)
  {
    /* delay_base has 32 bits, so delay_norm is approximately 1s max;
       delay_max <= 1s by config checks */
    unsigned delay_base = pseudo_random_delay (&pp->e.guid, dest_proxypp_guid, tnow);
    unsigned delay_norm = delay_base >> 2;
    os_int64 delay_max_ms = config.spdp_response_delay_max / 1000000;
    os_int64 delay = (os_int64) delay_norm * delay_max_ms / 1000;
    os_int64 tsched = tnow + delay;
    TRACE ((" %lld", delay));
    if (!config.unicast_response_to_spdp_messages)
      /* pp can't reach gc_delete_participant => can safely reschedule */
      resched_xevent_if_earlier (pp->spdp_xevent, tsched);
    else
      qxev_spdp (tsched, &pp->e.guid, dest_proxypp_guid);
  }
  ephash_enum_participant_fini (&est);
}

static void handle_SPDP_dead (const struct receiver_state *rst, const nn_plist_t *datap)
{
  nn_guid_t guid;

  if (datap->present & PP_PARTICIPANT_GUID)
  {
    guid = datap->participant_guid;
    TRACE ((" %x:%x:%x:%x", PGUID (guid)));
    assert (guid.entityid.u == NN_ENTITYID_PARTICIPANT);
    if (delete_proxy_participant_by_guid (&guid) < 0)
    {
      TRACE ((" unknown"));
    }
    else
    {
      TRACE ((" delete"));
    }
  }
  else
  {
    NN_WARNING2 ("data (SPDP, vendor %d.%d): no/invalid payload\n", rst->vendor.id[0], rst->vendor.id[1]);
  }
}

static void handle_SPDP_alive (const struct receiver_state *rst, const nn_plist_t *datap)
{
  const unsigned bes_sedp_announcer_mask =
    NN_DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER |
    NN_DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER;
  struct addrset *as_meta, *as_default;
  struct proxy_participant *proxypp;
  unsigned builtin_endpoint_set;
  nn_guid_t privileged_pp_guid;
  nn_duration_t lease_duration;
  const char *ignore_msg = NULL;
  unsigned custom_flags = 0;

  if (!(datap->present & PP_PARTICIPANT_GUID) || !(datap->present & PP_BUILTIN_ENDPOINT_SET))
  {
    NN_WARNING2 ("data (SPDP, vendor %d.%d): no/invalid payload\n", rst->vendor.id[0], rst->vendor.id[1]);
    return;
  }

  /* At some point the RTI implementation didn't mention
     BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER & ...WRITER, or
     so it seemed; and yet they are necessary for correct operation,
     so add them. */
  builtin_endpoint_set = datap->builtin_endpoint_set;
  if (vendor_is_rti (rst->vendor) &&
      ((builtin_endpoint_set &
        (NN_BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER |
         NN_BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER))
       != (NN_BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER |
           NN_BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER)) &&
      config.assume_rti_has_pmd_endpoints)
  {
    NN_WARNING2 ("data (SPDP, vendor %d.%d): assuming unadvertised PMD endpoints do exist\n",
                 rst->vendor.id[0], rst->vendor.id[1]);
    builtin_endpoint_set |=
      NN_BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER |
      NN_BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER;
  }

  TRACE ((" %x:%x:%x:%x", PGUID (datap->participant_guid)));

  /* Local SPDP packets may be looped back, and that may include ones
     currently being deleted.  The first thing that happens when
     deleting a participant is removing it from the hash table, and
     consequently the looped back packet may appear to be from an
     unknown participant.  So we handle that, too. */
  if (ephash_lookup_participant_guid (&datap->participant_guid) ||
      is_deleted_participant_guid (&datap->participant_guid))
  {
    TRACE ((" (local or recently deleted)"));
    return;
  }

  if ((proxypp = ephash_lookup_proxy_participant_guid (&datap->participant_guid)) != NULL)
  {
    /* SPDP processing is so different from normal processing that we
       are even skipping the automatic lease renewal.  Therefore do it
       regardless of
       config.arrival_of_data_asserts_pp_and_ep_liveliness. */
    TRACE ((" (known)", ignore_msg));
    lease_renew (proxypp->lease, now ());
    return;
  }

  TRACE ((" bes %x NEW", builtin_endpoint_set));

  if (datap->present & PID_PARTICIPANT_LEASE_DURATION)
  {
    lease_duration = datap->participant_lease_duration;
  }
  else
  {
    TRACE ((" (PARTICIPANT_LEASE_DURATION defaulting to 100s)"));
    lease_duration = nn_to_ddsi_duration (100 * T_SECOND);
  }

  /* If any of the SEDP announcer are missing AND the guid prefix of
     the SPDP writer differs from the guid prefix of the new participant,
     we make it dependent on the writer's participant.  See also the
     lease expiration handling.  Note that the entityid MUST be
     NN_ENTITYID_PARTICIPANT or ephash_lookup will assert.  So we only
     zero the prefix. */
  privileged_pp_guid.prefix = rst->src_guid_prefix;
  privileged_pp_guid.entityid.u = NN_ENTITYID_PARTICIPANT;
  if ((builtin_endpoint_set & bes_sedp_announcer_mask) != bes_sedp_announcer_mask &&
      memcmp (&privileged_pp_guid, &datap->participant_guid, sizeof (nn_guid_t)) != 0)
    TRACE ((" (depends on %x:%x:%x:%x)", PGUID (privileged_pp_guid)));
  else
    memset (&privileged_pp_guid.prefix, 0, sizeof (privileged_pp_guid.prefix));

  if (datap->present & PP_PRISMTECH_PARTICIPANT_VERSION_INFO) {
    if (datap->prismtech_participant_version_info.flags & NN_PRISMTECH_FL_KERNEL_SEQUENCE_NUMBER)
      custom_flags |= CF_INC_KERNEL_SEQUENCE_NUMBERS;

    TRACE ((" (0x%08x-0x%08x-0x%08x-0x%08x-0x%08x %s)",
            datap->prismtech_participant_version_info.version,
            datap->prismtech_participant_version_info.flags,
            datap->prismtech_participant_version_info.unused[0],
            datap->prismtech_participant_version_info.unused[1],
            datap->prismtech_participant_version_info.unused[2],
            datap->prismtech_participant_version_info.internals));
  }

  /* Choose locators */
  {
    os_sockaddr_storage addr;
    as_default = new_addrset ();
    as_meta = new_addrset ();

    /* If unicast locators not present, then try to obtain from connection */

    if 
    (
      (datap->present & PP_DEFAULT_UNICAST_LOCATOR) &&
      (get_address (&addr, &datap->default_unicast_locators))
    )
    {
      add_to_addrset (as_default, &addr);
    }
    else
    {
      if (ddsi_conn_address (rst->conn, &addr))
      {
        add_to_addrset (as_default, &addr);
      }
    }
    if 
    (
      (datap->present & PP_METATRAFFIC_UNICAST_LOCATOR) &&
      (get_address (&addr, &datap->metatraffic_unicast_locators))
    )
    {
      add_to_addrset (as_meta, &addr);
    }
    else
    {
      if (ddsi_conn_address (rst->conn, &addr))
      {
        add_to_addrset (as_meta, &addr);
      }
    }
    if (config.allowMulticast)
    {
      if 
      (
        (datap->present & PP_DEFAULT_MULTICAST_LOCATOR) &&
        (get_address (&addr, &datap->default_multicast_locators))
      )
      {
        add_to_addrset (as_default, &addr);
      }
      if 
      (
        (datap->present & PP_METATRAFFIC_MULTICAST_LOCATOR) &&
        (get_address (&addr, &datap->metatraffic_multicast_locators))
      )
      {
        add_to_addrset (as_meta, &addr);
      }
    }
    nn_log_addrset (LC_TRACE, " (data", as_default);
    nn_log_addrset (LC_TRACE, " meta", as_meta);

    TRACE ((")"));
  }

  maybe_add_pp_as_meta_to_as_disc (as_meta);

  new_proxy_participant 
  (
    &datap->participant_guid,
    builtin_endpoint_set,
    &privileged_pp_guid,
    as_default,
    as_meta,
    nn_from_ddsi_duration (lease_duration),
    rst->vendor,
    custom_flags
  );

  /* Force transmission of SPDP messages - we're not very careful
     in avoiding the processing of SPDP packets addressed to others
     so filter here */
  {
    int have_dst =
      (rst->dst_guid_prefix.u[0] != 0 || rst->dst_guid_prefix.u[1] != 0 || rst->dst_guid_prefix.u[2] != 0);
    if (!have_dst)
    {
      TRACE (("broadcasted SPDP packet -> answering"));
      respond_to_spdp (&datap->participant_guid);
    }
    else
    {
      TRACE (("directed SPDP packet -> not responding"));
    }
  }
}

static void handle_SPDP (const struct receiver_state *rst, unsigned statusinfo, const void *vdata, int len)
{
  const struct CDRHeader *data = vdata; /* built-ins not deserialized (yet) */
  TRACE (("SPDP ST%x", statusinfo));
  if (data == NULL)
  {
    TRACE ((" no payload?\n"));
    return;
  }
  else
  {
    nn_plist_t decoded_data;
    nn_plist_src_t src;
    src.protocol_version = rst->protocol_version;
    src.vendorid = rst->vendor;
    src.encoding = data->identifier;
    src.buf = (char *) data + 4;
    src.bufsz = len - 4;
    if (nn_plist_init_frommsg (&decoded_data, NULL, ~0u, ~0u, &src) < 0)
    {
      NN_WARNING2 ("SPDP (vendor %d.%d): invalid qos/parameters\n", src.vendorid.id[0], src.vendorid.id[1]);
      return;
    }

    switch (statusinfo & (NN_STATUSINFO_DISPOSE | NN_STATUSINFO_UNREGISTER))
    {
      case 0:
        handle_SPDP_alive (rst, &decoded_data);
        break;

      case NN_STATUSINFO_DISPOSE:
      case NN_STATUSINFO_UNREGISTER:
      case (NN_STATUSINFO_DISPOSE | NN_STATUSINFO_UNREGISTER):
        handle_SPDP_dead (rst, &decoded_data);
        break;
    }

    nn_plist_fini (&decoded_data);
    TRACE (("\n"));
  }
}

static void add_sockaddr_to_ps (const os_sockaddr_storage *addr, void *arg)
{
  nn_plist_t *ps = (nn_plist_t *) arg;
  struct nn_locators_one *elem = os_malloc (sizeof (struct nn_locators_one));

  nn_address_to_loc (&elem->loc, addr);
  elem->next = NULL;

  if (is_mcaddr (addr))
  {
    if ( ! (ps->present & PP_MULTICAST_LOCATOR))
    {
      ps->multicast_locators.n = 0;
      ps->multicast_locators.first = NULL;
      ps->multicast_locators.last = NULL;
      ps->present |= PP_MULTICAST_LOCATOR;
    }
    ps->multicast_locators.n++;
    if (ps->multicast_locators.first)
    {
      ps->multicast_locators.last->next = elem;
    }
    else
    {
      ps->multicast_locators.first = elem;
    }
    ps->multicast_locators.last = elem;
  }
  else
  {
    if ( ! (ps->present & PP_UNICAST_LOCATOR))
    {
      ps->unicast_locators.n = 0;
      ps->unicast_locators.first = NULL;
      ps->unicast_locators.last = NULL;
      ps->present |= PP_UNICAST_LOCATOR;
    }
    ps->unicast_locators.n++;
    if (ps->unicast_locators.first)
    {
      ps->unicast_locators.last->next = elem;
    }
    else
    {
      ps->unicast_locators.first = elem;
    }
    ps->unicast_locators.last = elem;
  }
}

/******************************************************************************
 ***
 *** SEDP
 ***
 *****************************************************************************/

static int sedp_write_endpoint (struct writer *wr, int end_of_life, const nn_guid_t *epguid, const nn_xqos_t *xqos, struct addrset *as)
{
  const nn_xqos_t *defqos = is_writer_entityid (epguid->entityid) ? &gv.default_xqos_wr : &gv.default_xqos_rd;
  const nn_vendorid_t my_vendor_id = MY_VENDOR_ID;
  const int just_key = end_of_life;
  struct nn_xmsg *mpayload;
  unsigned qosdiff;
  nn_guid_t kh;
  nn_plist_t ps;
  serstate_t serstate;
  serdata_t serdata;
  void *payload_blob;
  unsigned payload_sz;
  unsigned statusinfo;

  nn_plist_init_empty (&ps);
  ps.present |= PP_ENDPOINT_GUID;
  ps.endpoint_guid = *epguid;

  if (end_of_life)
  {
    assert (xqos == NULL);
    qosdiff = 0;
  }
  else
  {
    assert (xqos != NULL);
    ps.present |= PP_PROTOCOL_VERSION | PP_VENDORID;
    ps.protocol_version.major = RTPS_MAJOR;
    ps.protocol_version.minor = RTPS_MINOR;
    ps.vendorid = my_vendor_id;

    qosdiff = nn_xqos_delta (xqos, defqos, ~0u);
    if (config.explicitly_publish_qos_set_to_default)
      qosdiff = ~0u;
  }

  /* Add all the addresses to the Plist */
  if (as)
  {
    addrset_forall (as, add_sockaddr_to_ps, &ps);
  }

  /* The message is only a temporary thing, used only for encoding
     the QoS and other settings. So the header fields aren't really
     important, except that they need to be set to reasonable things
     or it'll crash */
  mpayload = nn_xmsg_new (gv.xmsgpool, &wr->e.guid.prefix, 0, NN_XMSG_KIND_DATA);
  nn_plist_addtomsg (mpayload, &ps, ~0u, 0);
  if (xqos) nn_xqos_addtomsg (mpayload, xqos, qosdiff);
  nn_xmsg_addpar_sentinel (mpayload);

  /* Then we take the payload from the message and turn it into a
     serdata, and then we can write it as normal data */
  serstate = serstate_new (gv.serpool, NULL);
  payload_blob = nn_xmsg_payload (&payload_sz, mpayload);
  serstate_append_blob (serstate, 4, payload_sz, payload_blob);
  kh = nn_hton_guid (*epguid);
  serstate_set_key (serstate, just_key, 16, &kh);
  if (end_of_life)
    statusinfo = NN_STATUSINFO_DISPOSE | NN_STATUSINFO_UNREGISTER;
  else
    statusinfo = 0;
  serstate_set_msginfo (serstate, statusinfo, now (), 1, NULL);
  serdata = serstate_fix (serstate);
  nn_xmsg_free (mpayload);

  TRACE (("sedp: write for %x:%x:%x:%x via %x:%x:%x:%x\n", PGUID (*epguid), PGUID (wr->e.guid)));
  return write_sample (NULL, wr, serdata);
}

static struct writer *get_sedp_writer (const struct participant *pp, unsigned entityid)
{
  struct writer *sedp_wr = get_builtin_writer (pp, entityid);
  if (sedp_wr == NULL)
    NN_FATAL2 ("sedp_write_writer: no SEDP builtin writer %x for %x:%x:%x:%x\n", entityid, PGUID (pp->e.guid));
  return sedp_wr;
}

int sedp_write_writer (struct writer *wr)
{
  if (is_builtin_entityid (wr->e.guid.entityid))
    return 0;
  else
  {
    struct writer *sedp_wr = get_sedp_writer (wr->c.pp, NN_ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER);
    return sedp_write_endpoint (sedp_wr, 0, &wr->e.guid, wr->xqos, NULL);
  }
}

int sedp_write_reader (struct reader *rd)
{
  if (is_builtin_entityid (rd->e.guid.entityid))
    return 0;
  else
  {
    struct writer *sedp_wr = get_sedp_writer (rd->c.pp, NN_ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER);
    struct addrset *as = NULL;
    return sedp_write_endpoint (sedp_wr, 0, &rd->e.guid, rd->xqos, as);
  }
}

int sedp_dispose_unregister_writer (struct writer *wr)
{
  if (is_builtin_entityid (wr->e.guid.entityid))
    return 0;
  else
  {
    struct writer *sedp_wr = get_sedp_writer (wr->c.pp, NN_ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER);
    return sedp_write_endpoint (sedp_wr, 1, &wr->e.guid, NULL, NULL);
  }
}

int sedp_dispose_unregister_reader (struct reader *rd)
{
  if (is_builtin_entityid (rd->e.guid.entityid))
    return 0;
  else
  {
    struct writer *sedp_wr = get_sedp_writer (rd->c.pp, NN_ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER);
    return sedp_write_endpoint (sedp_wr, 1, &rd->e.guid, NULL, NULL);
  }
}

static const char *durability_to_string (nn_durability_kind_t k)
{
  switch (k)
  {
    case NN_VOLATILE_DURABILITY_QOS: return "volatile";
    case NN_TRANSIENT_LOCAL_DURABILITY_QOS: return "transient-local";
    case NN_TRANSIENT_DURABILITY_QOS: return "transient";
    case NN_PERSISTENT_DURABILITY_QOS: return "persistent";
  }
  abort (); return 0;
}




static int handle_SEDP_alive (nn_plist_t *datap)
{
#define E(msg, lbl) do { nn_log (LC_TRACE, (msg)); goto lbl; } while (0)
  struct proxy_participant *pp;
  nn_guid_t ppguid;
  nn_xqos_t *xqos;
  int reliable;
  struct addrset *as;
  int result = 0;

  assert (datap);

  if (!(datap->present & PP_ENDPOINT_GUID))
    E (" no guid?\n", err);
  TRACE ((" %x:%x:%x:%x", PGUID (datap->endpoint_guid)));

  ppguid.prefix = datap->endpoint_guid.prefix;
  ppguid.entityid.u = NN_ENTITYID_PARTICIPANT;
  if (ephash_lookup_participant_guid (&ppguid) != NULL)
    E (" local pp?\n", err);
  if (is_deleted_participant_guid (&ppguid))
    E (" local dead pp?\n", err);
  if ((pp = ephash_lookup_proxy_participant_guid (&ppguid)) == NULL)
    E (" unknown proxy pp?\n", err);
  if (is_builtin_entityid (datap->endpoint_guid.entityid))
    E (" built-in\n", err);
  if (!(datap->qos.present & QP_TOPIC_NAME))
    E (" no topic?\n", err);
  if (!(datap->qos.present & QP_TYPE_NAME))
    E (" no typename?\n", err);

  xqos = os_malloc (sizeof (*xqos));
  nn_xqos_copy (xqos, &datap->qos);
  if (is_writer_entityid (datap->endpoint_guid.entityid))
    nn_xqos_mergein_missing (xqos, &gv.default_xqos_wr);
  else
    nn_xqos_mergein_missing (xqos, &gv.default_xqos_rd);
  /* After copy + merge, should have at least the ones present in the
     input.  Also verify reliability and durability are present,
     because we explicitly read those. */
  assert ((xqos->present & datap->qos.present) == datap->qos.present);
  assert (xqos->present & QP_RELIABILITY);
  assert (xqos->present & QP_DURABILITY);
  reliable = (xqos->reliability.kind == NN_RELIABLE_RELIABILITY_QOS);

  nn_log (LC_TRACE, " %s %s %s: %s%s.%s/%s",
          reliable ? "reliable" : "best-effort",
          durability_to_string (xqos->durability.kind),
          is_writer_entityid (datap->endpoint_guid.entityid) ? "writer" : "reader",
          ((!(xqos->present & QP_PARTITION) ||
            xqos->partition.n == 0 ||
            *xqos->partition.strs[0] == '\0')
           ? "(default)" : xqos->partition.strs[0]),
          ((xqos->present & QP_PARTITION) && xqos->partition.n > 1) ? "+" : "",
          xqos->topic_name, xqos->type_name);

  if (!is_writer_entityid (datap->endpoint_guid.entityid) &&
      (datap->present & PP_EXPECTS_INLINE_QOS) &&
      datap->expects_inline_qos)
  {
    E ("******* AARGH - it expects inline QoS ********\n", err_xqos);
  }

  {
    int known;
    if (is_writer_entityid (datap->endpoint_guid.entityid))
      known = (ephash_lookup_proxy_writer_guid (&datap->endpoint_guid) != NULL);
    else
      known = (ephash_lookup_proxy_reader_guid (&datap->endpoint_guid) != NULL);
    if (known)
    {
      result = 1;
      TRACE ((" known\n"));
      goto err_xqos;
    }
  }

  TRACE ((" NEW"));

  as = new_addrset ();
  {
    os_sockaddr_storage addr;
    if ((datap->present & PP_UNICAST_LOCATOR) &&
        get_address (&addr, &datap->unicast_locators))
      add_to_addrset (as, &addr);
    else
      copy_addrset_into_addrset_uc (as, pp->as_default);
    if (config.allowMulticast)
    {
      if ((datap->present & PP_MULTICAST_LOCATOR) &&
          get_address (&addr, &datap->multicast_locators))
        add_to_addrset (as, &addr);
      else
        copy_addrset_into_addrset_mc (as, pp->as_default);
    }
  }
  nn_log_addrset (LC_TRACE, " (as", as);
  TRACE ((") QOS={"));
  nn_log_xqos (LC_TRACE, xqos);
  TRACE (("}\n"));

  if (is_writer_entityid (datap->endpoint_guid.entityid))
  {
    /* not supposed to get here for built-in ones, so can determine the channel based on the transport priority */
    new_proxy_writer (&datap->endpoint_guid, as, xqos, gv.user_dqueue, gv.xevents);
  }
  else
  {
    new_proxy_reader (&datap->endpoint_guid, as, xqos);
  }

  unref_addrset (as);
  return 1;

err_xqos:
  nn_xqos_fini (xqos);
  os_free (xqos);
err:
  return result;
#undef E
}

static void handle_SEDP_dead (nn_plist_t *datap)
{
  int res;
  if (!(datap->present & PP_ENDPOINT_GUID))
  {
    TRACE ((" no guid?\n"));
    return;
  }
  TRACE ((" %x:%x:%x:%x", PGUID (datap->endpoint_guid)));
  if (is_writer_entityid (datap->endpoint_guid.entityid))
  {
    res = delete_proxy_writer (&datap->endpoint_guid);
  }
  else
  {
    res = delete_proxy_reader (&datap->endpoint_guid);
  }
  TRACE ((" %s\n", (res < 0) ? " unknown" : " delete"));
}

static void handle_SEDP (const struct receiver_state *rst, unsigned statusinfo, const void *vdata, int len)
{
  const struct CDRHeader *data = vdata; /* built-ins not deserialized (yet) */
  TRACE (("SEDP ST%x", statusinfo));
  if (data == NULL)
  {
    TRACE ((" no payload?\n"));
    return;
  }
  else
  {
    nn_plist_t decoded_data;
    nn_plist_src_t src;
    src.protocol_version = rst->protocol_version;
    src.vendorid = rst->vendor;
    src.encoding = data->identifier;
    src.buf = (char *) data + 4;
    src.bufsz = len - 4;
    if (nn_plist_init_frommsg (&decoded_data, NULL, ~0u, ~0u, &src) < 0)
    {
      NN_WARNING2 ("SEDP (vendor %d.%d): invalid qos/parameters\n", src.vendorid.id[0], src.vendorid.id[1]);
      return;
    }

    switch (statusinfo & (NN_STATUSINFO_DISPOSE | NN_STATUSINFO_UNREGISTER))
    {
      case 0:
        handle_SEDP_alive (&decoded_data);
        break;

      case NN_STATUSINFO_DISPOSE:
      case NN_STATUSINFO_UNREGISTER:
      case (NN_STATUSINFO_DISPOSE | NN_STATUSINFO_UNREGISTER):
        handle_SEDP_dead (&decoded_data);
        break;
    }

    nn_plist_fini (&decoded_data);
  }
}

int builtins_dqueue_handler (const struct nn_rsample_info *sampleinfo, const struct nn_rdata *fragchain, UNUSED_ARG (const nn_guid_t *rdguid), UNUSED_ARG (void *qarg))
{
  struct proxy_writer *pwr;
  struct {
    struct CDRHeader cdr;
    nn_parameter_t p_endpoint_guid;
    char kh[16];
    nn_parameter_t p_sentinel;
  } keyhash_payload;
  const char *datap = NN_RMSG_PAYLOADOFF (fragchain->rmsg, NN_RDATA_PAYLOAD_OFF (fragchain));
  int datasz = fragchain->maxp1;
  unsigned statusinfo;
  int need_keyhash;
  nn_guid_t srcguid;
  Data_DataFrag_common_t *msg;
  unsigned char data_smhdr_flags;
  nn_plist_t qos;

  /* no fragments accepted yet; gaps have been filtered out already */
  assert (fragchain->min == 0);
  assert (fragchain->maxp1 == sampleinfo->size);
  assert (fragchain->nextfrag == NULL);

  /* Luckily, most of the Data and DataFrag headers are the same - and
     in particular, all that we care about here is the same.  The
     key/data flags of DataFrag are different from those of Data, but
     DDSI2 used to treat them all as if they are data :( so now,
     instead of splitting out all the code, we reformat these flags
     from the submsg to always conform to that of the "Data"
     submessage regardless of the input. */
  msg = (Data_DataFrag_common_t *) NN_RMSG_PAYLOADOFF (fragchain->rmsg, NN_RDATA_SUBMSG_OFF (fragchain));
  data_smhdr_flags = normalize_data_datafrag_flags (&msg->smhdr, config.buggy_datafrag_flags_mode);
  srcguid.prefix = sampleinfo->rst->src_guid_prefix;
  srcguid.entityid = msg->writerId;

  pwr = sampleinfo->pwr;
  if (pwr == NULL)
    assert (srcguid.entityid.u == NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER);
  else
  {
    assert (is_builtin_entityid (pwr->e.guid.entityid));
    assert (memcmp (&pwr->e.guid, &srcguid, sizeof (srcguid)) == 0);
    assert (srcguid.entityid.u != NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER);
  }

  /* If there is no payload, it is either a completely invalid message
     or a dispose/unregister in RTI style. We assume the latter,
     consequently expect to need the keyhash.  Then, if sampleinfo
     says it is a complex qos, or the keyhash is required, extract all
     we need from the inline qos. */
  need_keyhash = (datasz == 0 || (data_smhdr_flags & (DATA_FLAG_KEYFLAG | DATA_FLAG_DATAFLAG)) == 0);
  if (!(sampleinfo->complex_qos || need_keyhash))
  {
    nn_plist_init_empty (&qos);
    statusinfo = sampleinfo->statusinfo;
  }
  else
  {
    nn_plist_src_t src;
    int qos_offset = NN_RDATA_SUBMSG_OFF (fragchain) + offsetof (Data_DataFrag_common_t, octetsToInlineQos) + sizeof (msg->octetsToInlineQos) + msg->octetsToInlineQos;
    src.protocol_version = sampleinfo->rst->protocol_version;
    src.vendorid = sampleinfo->rst->vendor;
    src.encoding = (msg->smhdr.flags & SMFLAG_ENDIANNESS) ? PL_CDR_LE : PL_CDR_BE;
    src.buf = NN_RMSG_PAYLOADOFF (fragchain->rmsg, qos_offset);
    src.bufsz = NN_RDATA_PAYLOAD_OFF (fragchain) - qos_offset;
    if (nn_plist_init_frommsg (&qos, NULL, PP_STATUSINFO | PP_KEYHASH, 0, &src) < 0)
    {
      NN_WARNING4 ("data(builtin, vendor %d.%d): %x:%x:%x:%x #%lld: invalid inline qos\n",
                   src.vendorid.id[0], src.vendorid.id[1], PGUID (srcguid), sampleinfo->seq);
      goto done_upd_deliv;
    }
    /* Complex qos bit also gets set when statusinfo bits other than
       dispose/unregister are set.  They are not currently defined,
       but this may save us if they do get defined one day. */
    statusinfo = (qos.present & PP_STATUSINFO) ? qos.statusinfo : 0;
  }

  if (pwr && ut_avlIsEmpty (&pwr->readers))
  {
    /* Wasn't empty when enqueued, but needn't still be; SPDP has no
       proxy writer, and is always accepted */
    goto done_upd_deliv;
  }

  /* Built-ins still do their own deserialization (SPDP <=> pwr ==
     NULL)). */
  assert (pwr == NULL || pwr->c.topic == NULL);
  if (statusinfo == 0)
  {
    if (datasz == 0 || !(data_smhdr_flags & DATA_FLAG_DATAFLAG))
    {
      NN_WARNING4 ("data(builtin, vendor %d.%d): %x:%x:%x:%x #%lld: "
                   "built-in data but no payload\n",
                   sampleinfo->rst->vendor.id[0], sampleinfo->rst->vendor.id[1],
                   PGUID (srcguid), sampleinfo->seq);
      goto done_upd_deliv;
    }
  }
  else if (datasz)
  {
    /* Raw data must be full payload for write, just keys for
       dispose and unregister. First has been checked; the second
       hasn't been checked fully yet. */
    if (!(data_smhdr_flags & DATA_FLAG_KEYFLAG))
    {
      NN_WARNING4 ("data(builtin, vendor %d.%d): %x:%x:%x:%x #%lld: "
                   "dispose/unregister of built-in data but payload not just key\n",
                   sampleinfo->rst->vendor.id[0], sampleinfo->rst->vendor.id[1],
                   PGUID (srcguid), sampleinfo->seq);
      goto done_upd_deliv;
    }
  }
  else if ((qos.present & PP_KEYHASH) && !NN_STRICT_P)
  {
    /* For SPDP/SEDP, fake a parameter list with just a keyhash.  For
       PMD, just use the keyhash directly.  Too hard to fix everything
       at the same time ... */
    if (srcguid.entityid.u == NN_ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_WRITER)
    {
      datap = qos.keyhash.value;
      datasz = sizeof (qos.keyhash);
    }
    else
    {
      nn_parameterid_t pid =
        (srcguid.entityid.u == NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER)
        ? PID_PARTICIPANT_GUID : PID_ENDPOINT_GUID;
      keyhash_payload.cdr.identifier = PLATFORM_IS_LITTLE_ENDIAN ? PL_CDR_LE : PL_CDR_BE;
      keyhash_payload.cdr.options = 0;
      keyhash_payload.p_endpoint_guid.parameterid = pid;
      keyhash_payload.p_endpoint_guid.length = sizeof (nn_keyhash_t);
      memcpy (keyhash_payload.kh, &qos.keyhash, sizeof (qos.keyhash));
      keyhash_payload.p_sentinel.parameterid = PID_SENTINEL;
      keyhash_payload.p_sentinel.length = 0;
      datap = (char *) &keyhash_payload;
      datasz = sizeof (keyhash_payload);
    }
  }
  else
  {
    NN_WARNING4 ("data(builtin, vendor %d.%d): %x:%x:%x:%x #%lld: "
                 "dispose/unregister with no content\n",
                 sampleinfo->rst->vendor.id[0], sampleinfo->rst->vendor.id[1],
                 PGUID (srcguid), sampleinfo->seq);
    goto done_upd_deliv;
  }

  switch (srcguid.entityid.u)
  {
    case NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER:
      handle_SPDP (sampleinfo->rst, statusinfo, datap, datasz);
      break;
    case NN_ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER:
    case NN_ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER:
      handle_SEDP (sampleinfo->rst, statusinfo, datap, datasz);
      break;
    case NN_ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_WRITER:
      handle_PMD (sampleinfo->rst, statusinfo, datap, datasz);
      break;
    default:
      NN_WARNING4 ("data(builtin, vendor %d.%d): %x:%x:%x:%x #%lld: not handled\n",
                   sampleinfo->rst->vendor.id[0], sampleinfo->rst->vendor.id[1],
                   PGUID (srcguid), sampleinfo->seq);
      break;
  }

 done_upd_deliv:
  if (pwr)
  {
    /* No proxy writer for SPDP */
    atomic_store_u32 (&pwr->next_deliv_seq_lowword, (os_uint32) (sampleinfo->seq + 1));
  }
  return 0;
}

/* SHA1 not available (unoffical build.) */

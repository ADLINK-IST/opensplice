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

#include "os_heap.h"
#include "os_mutex.h"
#include "os_cond.h"
#include "os_thread.h"
#include "os_socket.h"
#include "os_stdlib.h"
#include "os_init.h"
#include "os_if.h"
#include "os_atomics.h"

#include "ut_avl.h"
#include "ut_thread_pool.h"

#include "q_md5.h"
#include "q_protocol.h"
#include "q_rtps.h"
#include "q_misc.h"
#include "q_config.h"
#include "q_log.h"
#include "q_plist.h"
#include "q_unused.h"
#include "q_bswap.h"
#include "q_lat_estim.h"
#include "q_bitset.h"
#include "q_xevent.h"
#include "q_align.h"
#include "q_addrset.h"
#include "q_ddsi_discovery.h"
#include "q_radmin.h"
#include "q_error.h"
#include "q_thread.h"
#include "q_ephash.h"
#include "q_lease.h"
#include "q_gc.h"
#include "q_whc.h"
#include "q_entity.h"
#include "q_nwif.h"
#include "q_globals.h"
#include "q_xmsg.h"
#include "q_receive.h"
#include "q_pcap.h"
#include "q_feature_check.h"

#include "sysdeps.h"

#include "ddsi_ser.h"
#include "ddsi_tran.h"
#include "ddsi_udp.h"
#include "ddsi_tcp.h"

static void add_peer_addresses (struct addrset *as, const struct config_peer_listelem *list)
{
  while (list)
  {
    add_addresses_to_addrset (as, list->peer, -1, "add_peer_addresses", 0);
    list = list->next;
  }
}

static int make_uc_sockets (os_uint32 * pdisc, os_uint32 * pdata, int ppid)
{
  if (ppid >= 0)
  {
    /* FIXME: verify port numbers are in range instead of truncating them like this */
    int base = config.port_base + (config.port_dg * config.domainId) + (ppid * config.port_pg);
    *pdisc = (os_uint32) (base + config.port_d1);
    *pdata = (os_uint32) (base + config.port_d3);
  }
  else if (ppid == PARTICIPANT_INDEX_NONE)
  {
    *pdata = 0;
    *pdisc = 0;
  }
  else
  {
    NN_FATAL1 ("make_uc_sockets: invalid participant index %d\n", ppid);
    return -1;
  }

  gv.disc_conn_uc = ddsi_factory_create_conn (gv.m_factory, *pdisc, NULL);
  if (gv.disc_conn_uc)
  {
    /* Check not configured to use same unicast port for data and discovery */

    if (*pdata != 0 && (*pdata != *pdisc))
    {
      gv.data_conn_uc = ddsi_factory_create_conn (gv.m_factory, *pdata, NULL);
    }
    else
    {
      gv.data_conn_uc = gv.disc_conn_uc;
    }
    if (gv.data_conn_uc == NULL)
    {
      ddsi_conn_free (gv.disc_conn_uc);
      gv.disc_conn_uc = NULL;
    }
    else
    {
      /* Set unicast locators */

      ddsi_conn_locator (gv.disc_conn_uc, &gv.loc_meta_uc);
      ddsi_conn_locator (gv.data_conn_uc, &gv.loc_default_uc);
    }
  }

  return gv.data_conn_uc ? 0 : -1;
}

static void make_builtin_endpoint_xqos (nn_xqos_t *q, const nn_xqos_t *template)
{
  nn_xqos_copy (q, template);
  q->reliability.kind = NN_RELIABLE_RELIABILITY_QOS;
  q->reliability.max_blocking_time = nn_to_ddsi_duration (100 * T_MILLISECOND);
  q->durability.kind = NN_TRANSIENT_LOCAL_DURABILITY_QOS;
}

static int set_recvips (void)
{
  gv.recvips = NULL;

  if (config.networkRecvAddressStrings)
  {
    if (os_strcasecmp (config.networkRecvAddressStrings[0], "all") == 0)
    {
#if OS_SOCKET_HAS_IPV6
      if (gv.ipv6_link_local)
      {
        NN_WARNING0 ("DDSI2EService/General/MulticastRecvNetworkInterfaceAddresses: using 'preferred' instead of 'all' because of IPv6 link-local address\n");
        gv.recvips_mode = RECVIPS_MODE_PREFERRED;
      }
      else
#endif
      {
        gv.recvips_mode = RECVIPS_MODE_ALL;
      }
    }
    else if (os_strcasecmp (config.networkRecvAddressStrings[0], "any") == 0)
    {
#if OS_SOCKET_HAS_IPV6
      if (gv.ipv6_link_local)
      {
        NN_ERROR0 ("DDSI2EService/General/MulticastRecvNetworkInterfaceAddresses: 'any' is unsupported in combination with an IPv6 link-local address\n");
        return -1;
      }
#endif
      gv.recvips_mode = RECVIPS_MODE_ANY;
    }
    else if (os_strcasecmp (config.networkRecvAddressStrings[0], "preferred") == 0)
    {
      gv.recvips_mode = RECVIPS_MODE_PREFERRED;
    }
    else if (os_strcasecmp (config.networkRecvAddressStrings[0], "none") == 0)
    {
      gv.recvips_mode = RECVIPS_MODE_NONE;
    }
#if OS_SOCKET_HAS_IPV6
    else if (gv.ipv6_link_local)
    {
      /* If the configuration explicitly includes the selected
       interface, treat it as "preferred", else as "none"; warn if
       interfaces other than the selected one are included. */
      int i, have_selected = 0, have_others = 0;
      for (i = 0; config.networkRecvAddressStrings[i] != NULL; i++)
      {
        os_sockaddr_storage parsedaddr;
        if (!os_sockaddrStringToAddress (config.networkRecvAddressStrings[i], (os_sockaddr *) &parsedaddr, !config.useIpv6))
        {
          NN_ERROR1 ("%s: not a valid IP address\n", config.networkRecvAddressStrings[i]);
          return -1;
        }
        if (os_sockaddrIPAddressEqual ((os_sockaddr *) &gv.interfaces[gv.selected_interface].addr, (os_sockaddr *) &parsedaddr))
          have_selected = 1;
        else
          have_others = 1;
      }
      gv.recvips_mode = have_selected ? RECVIPS_MODE_PREFERRED : RECVIPS_MODE_NONE;
      if (have_others)
      {
        NN_WARNING0 ("DDSI2EService/General/MulticastRecvNetworkInterfaceAddresses: using 'preferred' because of IPv6 local address\n");
      }
    }
#endif
    else
    {
      struct ospl_in_addr_node **recvnode = &gv.recvips;
      int i, j;
      gv.recvips_mode = RECVIPS_MODE_SOME;
      for (i = 0; config.networkRecvAddressStrings[i] != NULL; i++)
      {
        os_sockaddr_storage parsedaddr;
        if (!os_sockaddrStringToAddress (config.networkRecvAddressStrings[i], (os_sockaddr *) &parsedaddr, !config.useIpv6))
        {
          NN_ERROR1 ("%s: not a valid IP address\n", config.networkRecvAddressStrings[i]);
          return -1;
        }
        for (j = 0; j < gv.n_interfaces; j++)
        {
          if (os_sockaddrIPAddressEqual ((os_sockaddr *) &gv.interfaces[j].addr, (os_sockaddr *) &parsedaddr))
            break;
        }
        if (j == gv.n_interfaces)
        {
          NN_ERROR1 ("No interface bound to requested address '%s'\n",
                     config.networkRecvAddressStrings[i]);
          return -1;
        }
        *recvnode = os_malloc (sizeof (struct ospl_in_addr_node));
        (*recvnode)->addr = parsedaddr;
        recvnode = &(*recvnode)->next;
        *recvnode = NULL;
      }
    }
  }
  return 0;
}

/* Apart from returning errors with the 'string' content is not valid, this
 * function can also return errors when a mismatch in multicast/unicast is
 * detected.
 *   return -1 : ddsi is unicast, but 'mc' indicates it expects multicast
 *   return  0 : ddsi is multicast, but 'mc' indicates it expects unicast
 * The return 0 means that the possible changes in 'loc' can be ignored. */
static int string_to_default_locator (nn_locator_t *loc, const char *string, os_uint32 port, int mc, const char *tag)
{
  os_sockaddr_storage addr;
  if (strspn (string, " \t") == strlen (string))
  {
    /* string consisting of just spaces and/or tabs (that includes the empty string) is ignored */
    return 0;
  }
  else if (!os_sockaddrStringToAddress (string, (os_sockaddr *) &addr, !config.useIpv6))
  {
    NN_ERROR2 ("%s: not a valid IP address (%s)\n", string, tag);
    return -1;
  }
  else if (!config.useIpv6 && addr.ss_family != AF_INET)
  {
    NN_ERROR2 ("%s: not a valid IPv4 address (%s)\n", string, tag);
    return -1;
  }
#if OS_SOCKET_HAS_IPV6
  else if (config.useIpv6 && addr.ss_family != AF_INET6)
  {
    NN_ERROR2 ("%s: not a valid IPv6 address (%s)\n", string, tag);
    return -1;
  }
#endif
  else
  {
    nn_address_to_loc (loc, &addr, config.useIpv6 ? NN_LOCATOR_KIND_UDPv6 : NN_LOCATOR_KIND_UDPv4);
    if (port != 0 && !is_unspec_locator(loc))
      loc->port = port;
    assert (mc == -1 || mc == 0 || mc == 1);
    if (mc >= 0)
    {
      const char *unspecstr = config.useIpv6 ? "the IPv6 unspecified address (::0)" : "IPv4 ANY (0.0.0.0)";
      const char *rel = mc ? "must" : "may not";
      const int ismc = is_unspec_locator (loc) || is_mcaddr (loc);
      if (mc != ismc)
      {
        NN_ERROR4 ("%s: %s %s be %s or a multicast address\n", string, tag, rel, unspecstr);
        return -1;
      }
    }

    return 1;
  }
}

static int set_spdp_address (void)
{
  const os_uint32 port = (os_uint32) (config.port_base + config.port_dg * config.domainId + config.port_d0);
  int rc = 0;
  if (strcmp (config.spdpMulticastAddressString, "239.255.0.1") != 0)
  {
    if ((rc = string_to_default_locator (&gv.loc_spdp_mc, config.spdpMulticastAddressString, port, 1, "SPDP address")) < 0)
      return rc;
  }
  if (rc == 0)
  {
    /* There isn't a standard IPv6 multicast group for DDSI. For
       some reason, node-local multicast addresses seem to be
       unsupported (ff01::... would be a node-local one I think), so
       instead do link-local. I suppose we could use the hop limit
       to make it node-local.  If other hosts reach us in some way,
       we'll of course respond. */
    const char *def = config.useIpv6 ? "ff02::ffff:239.255.0.1" : "239.255.0.1";
    rc = string_to_default_locator (&gv.loc_spdp_mc, def, port, 1, "SPDP address");
    assert (rc > 0);
  }
  if (!(config.allowMulticast & AMC_SPDP) || config.suppress_spdp_multicast)
  {
    /* Explicitly disabling SPDP multicasting is always possible */
    set_unspec_locator (&gv.loc_spdp_mc);
  }
  return 0;
}

static int set_default_mc_address (void)
{
  const os_uint32 port = (os_uint32) (config.port_base + config.port_dg * config.domainId + config.port_d2);
  int rc;
  if (!config.defaultMulticastAddressString)
    gv.loc_default_mc = gv.loc_spdp_mc;
  else if ((rc = string_to_default_locator (&gv.loc_default_mc, config.defaultMulticastAddressString, port, 1, "default multicast address")) < 0)
    return rc;
  else if (rc == 0)
    gv.loc_default_mc = gv.loc_spdp_mc;
  if (!(config.allowMulticast & ~AMC_SPDP))
  {
    /* no multicasting beyond SPDP */
    set_unspec_locator (&gv.loc_default_mc);
  }
  gv.loc_meta_mc = gv.loc_default_mc;
  return 0;
}

static int set_ext_address_and_mask (void)
{
  nn_locator_t loc;
  int rc;

  if (!config.externalAddressString)
    gv.extip = gv.ownip;
  else if ((rc = string_to_default_locator (&loc, config.externalAddressString, 0, 0, "external address")) < 0)
    return rc;
  else if (rc == 0) {
    NN_WARNING1 ("Ignoring ExternalNetworkAddress %s\n", config.externalAddressString);
    gv.extip = gv.ownip;
  } else {
    nn_loc_to_address (&gv.extip, &loc);
  }

  if (!config.externalMaskString || strcmp (config.externalMaskString, "0.0.0.0") == 0)
    gv.extmask.s_addr = 0;
  else if (config.useIpv6)
  {
    NN_ERROR0 ("external network masks only supported in IPv4 mode\n");
    return -1;
  }
  else
  {
    os_sockaddr_storage addr;
    if ((rc = string_to_default_locator (&loc, config.externalMaskString, 0, -1, "external mask")) < 0)
      return rc;
    nn_loc_to_address(&addr, &loc);
    assert (addr.ss_family == AF_INET);
    gv.extmask = ((const os_sockaddr_in *) &addr)->sin_addr;
  }
  return 0;
}


static int check_thread_properties (void)
{
  static const char *fixed[] = { "recv", "tev", "gc", "lease", "dq.builtins", "xmit.user", "dq.user", "debmon", NULL };
  const struct config_thread_properties_listelem *e;
  int ok = 1, i;
  for (e = config.thread_properties; e; e = e->next)
  {
    for (i = 0; fixed[i]; i++)
      if (strcmp (fixed[i], e->name) == 0)
        break;
    if (fixed[i] == NULL)
    {
      NN_ERROR1 ("config: DDSI2Service/Threads/Thread[@name=\"%s\"]: unknown thread\n", e->name);
      ok = 0;
    }
  }
  return ok;
}

int rtps_config_open (void)
{
  int status = -1;

  if (status == -1)
  {
    if (config.tracingOutputFileName == NULL || *config.tracingOutputFileName == 0 || config.enabled_logcats == 0)
    {
      config.enabled_logcats = 0;
      config.tracingOutputFile = NULL;
      status = 1;
    }
    else if (os_strcasecmp (config.tracingOutputFileName, "stdout") == 0)
    {
      config.tracingOutputFile = stdout;
      status = 1;
    }
    else if (os_strcasecmp (config.tracingOutputFileName, "stderr") == 0)
    {
      config.tracingOutputFile = stderr;
      status = 1;
    }
    else if ((config.tracingOutputFile = fopen (config.tracingOutputFileName, config.tracingAppendToFile ? "a" : "w")) == NULL)
    {
      NN_ERROR1 ("%s: cannot open for writing\n", config.tracingOutputFileName);
      status = 0;
    }
    else
    {
      status = 1;
    }
  }
  return status;
}

int rtps_config_prep (struct cfgst *cfgst)
{

  /* if the discovery domain id was explicitly set, override the default here */
  if (!config.discoveryDomainId.isdefault)
  {
    config.domainId = config.discoveryDomainId.value;
  }

  /* retry_on_reject_duration default is dependent on late_ack_mode and responsiveness timeout, so fix up */

  if (config.retry_on_reject_duration.isdefault)
  {
    if (config.late_ack_mode)
      config.retry_on_reject_duration.value = 0;
    else
      config.retry_on_reject_duration.value = 4 * (config.responsiveness_timeout / 5);
  }

  if (config.whc_init_highwater_mark.isdefault)
    config.whc_init_highwater_mark.value = config.whc_lowwater_mark;
  if (config.whc_highwater_mark < config.whc_lowwater_mark ||
      config.whc_init_highwater_mark.value < config.whc_lowwater_mark ||
      config.whc_init_highwater_mark.value > config.whc_highwater_mark)
  {
    NN_ERROR0 ("Invalid watermark settings\n");
    goto err_config_late_error;
  }

  if (config.besmode == BESMODE_MINIMAL && config.many_sockets_mode)
  {
    /* These two are incompatible because minimal bes mode can result
       in implicitly creating proxy participants inheriting the
       address set from the ddsi2 participant (which is then typically
       inherited by readers/writers), but in many sockets mode each
       participant has its own socket, and therefore unique address
       set */
    NN_ERROR0 ("Minimal built-in endpoint set mode and ManySocketsMode are incompatible\n");
    goto err_config_late_error;
  }

  /* Dependencies between default values is not handled
   automatically by the config processing (yet) */
  if (config.many_sockets_mode)
  {
    if (config.max_participants == 0)
      config.max_participants = 100;
  }
  if (NN_STRICT_P)
  {
    /* Should not be sending invalid messages when strict */
    config.respond_to_rti_init_zero_ack_with_invalid_heartbeat = 0;
    config.acknack_numbits_emptyset = 1;
  }
  if (config.max_queued_rexmit_bytes == 0)
  {
    config.max_queued_rexmit_bytes = 2147483647u;
  }

  /* Verify thread properties refer to defined threads */
  if (!check_thread_properties ())
  {
    NN_ERROR0 ("Could not initialise configuration\n");
    goto err_config_late_error;
  }

#if ! OS_SOCKET_HAS_IPV6
  /* If the platform doesn't support IPv6, guarantee useIpv6 is
   false. There are two ways of going about it, one is to do it
   silently, the other to let the user fix his config. Clearly, we
   have chosen the latter. */
  if (config.useIpv6)
  {
    NN_ERROR0 ("IPv6 addressing requested but not supported on this platform\n");
    goto err_config_late_error;
  }
#endif


  /* Open tracing file after all possible config errors have been
   printed */
  if (! rtps_config_open ())
  {
    NN_ERROR0 ("Could not initialise configuration\n");
    goto err_config_late_error;
  }

  /* Thread admin: need max threads, which is currently (2 or 3) for each
   configured channel plus 7: main, recv, dqueue.builtin,
   lease, gc, debmon; once thread state admin has been inited, upgrade the
   main thread one participating in the thread tracking stuff as
   if it had been created using create_thread(). */

  {
  /* For Lite - Temporary
    Thread states for each application thread is managed using thread_states structure
  */
#define USER_MAX_THREADS 0

    const unsigned max_threads = 9 + USER_MAX_THREADS + config.ddsi2direct_max_threads;
    thread_states_init (max_threads);
  }

  /* Now the per-thread-log-buffers are set up, so print the configuration */
  config_print_and_free_cfgst (cfgst);


  return 0;

err_config_late_error:
  return -1;
}

struct joinleave_spdp_defmcip_helper_arg {
  int errcount;
  int dojoin;
};

static void joinleave_spdp_defmcip_helper (const nn_locator_t *loc, void *varg)
{
  struct joinleave_spdp_defmcip_helper_arg *arg = varg;
  if (!is_mcaddr (loc))
    return;
  if (arg->dojoin) {
    if (ddsi_conn_join_mc (gv.disc_conn_mc, NULL, loc) < 0 || ddsi_conn_join_mc (gv.data_conn_mc, NULL, loc) < 0)
      arg->errcount++;
  } else {
    if (ddsi_conn_leave_mc (gv.disc_conn_mc, NULL, loc) < 0 || ddsi_conn_leave_mc (gv.data_conn_mc, NULL, loc) < 0)
      arg->errcount++;
  }
}

static int joinleave_spdp_defmcip (int dojoin)
{
  /* Addrset provides an easy way to filter out duplicates */
  struct joinleave_spdp_defmcip_helper_arg arg;
  struct addrset *as = new_addrset ();
  arg.errcount = 0;
  arg.dojoin = dojoin;
  if (config.allowMulticast & AMC_SPDP)
    add_to_addrset (as, &gv.loc_spdp_mc);
  if (config.allowMulticast & ~AMC_SPDP)
    add_to_addrset (as, &gv.loc_default_mc);
  addrset_forall (as, joinleave_spdp_defmcip_helper, &arg);
  unref_addrset (as);
  if (arg.errcount)
  {
    NN_ERROR2 ("rtps_init: failed to join multicast groups for domain %d participant %d\n", config.domainId, config.participantIndex);
    return -1;
  }
  return 0;
}

int rtps_init (void)
{
  os_uint32 port_disc_uc = 0;
  os_uint32 port_data_uc = 0;

  /* Initialize implementation (Lite or OSPL) */

  ddsi_impl_init ();

  gv.tstart = now ();    /* wall clock time, used in logs */

  gv.disc_conn_uc = NULL;
  gv.data_conn_uc = NULL;
  gv.disc_conn_mc = NULL;
  gv.data_conn_mc = NULL;
  gv.tev_conn = NULL;
  gv.listener = NULL;
  gv.thread_pool = NULL;

  /* Print start time for referencing relative times in the remainder
   of the nn_log. */
  {
    os_timeW tv = nn_wctime_to_os_timeW(gv.tstart);
    char str[OS_CTIME_R_BUFSIZE];

    os_ctimeW_r (&tv, str, sizeof(str));
    nn_log (LC_INFO | LC_CONFIG, "started at %" PA_PRItime " -- %s\n", OS_TIMEW_PRINT(tv), str);
  }

  /* Initialize thread pool */

  if (config.tp_enable)
  {
    gv.thread_pool = ut_thread_pool_new
      (config.tp_threads, config.tp_max_threads, 0, NULL);
  }

  /* Initialize UDP or TCP transport and resolve factory */

  if (!config.tcp_enable)
  {
    config.publish_uc_locators = TRUE;
    if (ddsi_udp_init () < 0)
      goto err_udp_tcp_init;
    gv.m_factory = ddsi_factory_find ("udp");
  }
  else
  {
    config.publish_uc_locators = (config.tcp_port == -1) ? FALSE : TRUE;
    /* TCP affects what features are supported/required */
    config.suppress_spdp_multicast = TRUE;
    config.many_sockets_mode = FALSE;
    config.allowMulticast = AMC_FALSE;

    if (ddsi_tcp_init () < 0)
      goto err_udp_tcp_init;
    gv.m_factory = ddsi_factory_find ("tcp");
  }

  if (!find_own_ip (config.networkAddressString))
  {
    NN_ERROR0 ("No network interface selected\n");
    goto err_find_own_ip;
  }
  if (config.allowMulticast)
  {
    int i;
    for (i = 0; i < gv.n_interfaces; i++)
    {
      if (gv.interfaces[i].mc_capable)
        break;
    }
    if (i == gv.n_interfaces)
    {
      NN_WARNING0 ("No multicast capable interfaces: disabling multicast\n");
      config.suppress_spdp_multicast = 1;
      config.allowMulticast = AMC_FALSE;
    }
  }
  if (set_recvips () < 0)
    goto err_set_recvips;
  if (set_spdp_address () < 0)
    goto err_set_ext_address;
  if (set_default_mc_address () < 0)
    goto err_set_ext_address;
  if (set_ext_address_and_mask () < 0)
    goto err_set_ext_address;

  {
    char buf[INET6_ADDRSTRLEN_EXTENDED];
    nn_log (LC_CONFIG, "ownip: %s\n", sockaddr_to_string_no_port (buf, &gv.ownip));
    nn_log (LC_CONFIG, "extip: %s\n", sockaddr_to_string_no_port (buf, &gv.extip));
    nn_log (LC_CONFIG, "extmask: %s%s\n", inet_ntoa (gv.extmask), config.useIpv6 ? " (not applicable)" : "");
    nn_log (LC_CONFIG, "networkid: 0x%lx\n", (unsigned long) gv.myNetworkId);
    nn_log (LC_CONFIG, "SPDP MC: %s\n", locator_to_string_no_port (buf, &gv.loc_spdp_mc));
    nn_log (LC_CONFIG, "default MC: %s\n", locator_to_string_no_port (buf, &gv.loc_default_mc));
  }

  if (gv.ownip.ss_family != gv.extip.ss_family)
    NN_FATAL0 ("mismatch between network address kinds\n");

  gv.startup_mode = (config.startup_mode_duration > 0) ? 1 : 0;
  nn_log (LC_CONFIG, "startup-mode: %s\n", gv.startup_mode ? "enabled" : "disabled");

  (ddsi_plugin.init_fn) ();

  gv.xmsgpool = nn_xmsgpool_new ();
  gv.serpool = ddsi_serstatepool_new ();


  nn_plist_init_default_participant (&gv.default_plist_pp);
  nn_xqos_init_default_reader (&gv.default_xqos_rd);
  nn_xqos_init_default_writer (&gv.default_xqos_wr);
  nn_xqos_init_default_writer_noautodispose (&gv.default_xqos_wr_nad);
  nn_xqos_init_default_topic (&gv.default_xqos_tp);
  nn_xqos_init_default_subscriber (&gv.default_xqos_sub);
  nn_xqos_init_default_publisher (&gv.default_xqos_pub);
  nn_xqos_copy (&gv.spdp_endpoint_xqos, &gv.default_xqos_rd);
  gv.spdp_endpoint_xqos.durability.kind = NN_TRANSIENT_LOCAL_DURABILITY_QOS;
  make_builtin_endpoint_xqos (&gv.builtin_endpoint_xqos_rd, &gv.default_xqos_rd);
  make_builtin_endpoint_xqos (&gv.builtin_endpoint_xqos_wr, &gv.default_xqos_wr);

  os_mutexInit (&gv.participant_set_lock, NULL);
  os_condInit (&gv.participant_set_cond, &gv.participant_set_lock, NULL);
  lease_management_init ();
  deleted_participants_admin_init ();
  gv.guid_hash = ephash_new (config.guid_hash_softlimit);

  os_mutexInit (&gv.privileged_pp_lock, NULL);
  gv.privileged_pp = NULL;


  os_mutexInit (&gv.lock, NULL);
  os_mutexInit (&gv.spdp_lock, NULL);
  gv.spdp_defrag = nn_defrag_new (NN_DEFRAG_DROP_OLDEST, config.defrag_unreliable_maxsamples);
  gv.spdp_reorder = nn_reorder_new (NN_REORDER_MODE_ALWAYS_DELIVER, config.primary_reorder_maxsamples);

  pa_st32 (&gv.last_threshold_warning_sec, 0);
  pa_st32 (&gv.memory_shortage_dropcount, 0);

  if (gv.m_factory->m_connless)
  {
    if (config.participantIndex >= 0 || config.participantIndex == PARTICIPANT_INDEX_NONE)
    {
      if (make_uc_sockets (&port_disc_uc, &port_data_uc, config.participantIndex) < 0)
      {
        NN_ERROR2 ("rtps_init: failed to create unicast sockets for domain %d participant %d\n", config.domainId, config.participantIndex);
        goto err_unicast_sockets;
      }
    }
    else if (config.participantIndex == PARTICIPANT_INDEX_AUTO)
    {
      /* try to find a free one, and update config.participantIndex */
      int ppid;
      nn_log (LC_CONFIG, "rtps_init: trying to find a free participant index\n");
      for (ppid = 0; ppid <= config.maxAutoParticipantIndex; ppid++)
      {
        int r = make_uc_sockets (&port_disc_uc, &port_data_uc, ppid);
        if (r == 0) /* Success! */
          break;
        else if (r == -1) /* Try next one */
          continue;
        else /* Oops! */
        {
          NN_ERROR2 ("rtps_init: failed to create unicast sockets for domain %d participant %d\n", config.domainId, ppid);
          goto err_unicast_sockets;
        }
      }
      if (ppid > config.maxAutoParticipantIndex)
      {
        NN_ERROR1 ("rtps_init: failed to find a free participant index for domain %d\n", config.domainId);
        goto err_unicast_sockets;
      }
      config.participantIndex = ppid;
    }
    else
    {
      assert(0);
    }
    nn_log (LC_CONFIG, "rtps_init: uc ports: disc %d data %d\n", port_disc_uc, port_data_uc);
  }
  nn_log (LC_CONFIG, "rtps_init: domainid %d participantid %d\n", config.domainId, config.participantIndex);

  gv.waitset = os_sockWaitsetNew ();

  if (config.pcap_file && *config.pcap_file)
  {
    gv.pcap_fp = new_pcap_file (config.pcap_file);
    if (gv.pcap_fp)
    {
      os_mutexInit (&gv.pcap_lock, NULL);
    }
  }
  else
  {
    gv.pcap_fp = NULL;
  }

  if (gv.m_factory->m_connless)
  {
    os_uint32 port;

    TRACE (("Unicast Ports: discovery %d data %d \n",
      ddsi_tran_port (gv.disc_conn_uc), ddsi_tran_port (gv.data_conn_uc)));

    if (config.allowMulticast)
    {
      ddsi_tran_qos_t qos = ddsi_tran_create_qos ();
      qos->m_multicast = TRUE;

      /* FIXME: should check for overflow */
      port = (os_uint32) (config.port_base + config.port_dg * config.domainId + config.port_d0);
      gv.disc_conn_mc = ddsi_factory_create_conn (gv.m_factory, port, qos);

      port = (os_uint32) (config.port_base + config.port_dg * config.domainId + config.port_d2);
      gv.data_conn_mc = ddsi_factory_create_conn (gv.m_factory, port, qos);

      ddsi_tran_free_qos (qos);

      if (gv.disc_conn_mc == NULL || gv.data_conn_mc == NULL)
        goto err_mc_conn;

      TRACE (("Multicast Ports: discovery %d data %d \n",
        ddsi_tran_port (gv.disc_conn_mc), ddsi_tran_port (gv.data_conn_mc)));

      /* Set multicast locators */
      if (!is_unspec_locator(&gv.loc_spdp_mc))
        gv.loc_spdp_mc.port = ddsi_tran_port (gv.disc_conn_mc);
      if (!is_unspec_locator(&gv.loc_meta_mc))
        gv.loc_meta_mc.port = ddsi_tran_port (gv.disc_conn_mc);
      if (!is_unspec_locator(&gv.loc_default_mc))
        gv.loc_default_mc.port = ddsi_tran_port (gv.data_conn_mc);

      if (joinleave_spdp_defmcip (1) < 0)
        goto err_mc_conn;
    }
  }
  else
  {
    /* Must have a data_conn_uc/tev_conn/transmit_conn */
    gv.data_conn_uc = ddsi_factory_create_conn (gv.m_factory, 0, NULL);

    if (config.tcp_port != -1)
    {
      gv.listener = ddsi_factory_create_listener (gv.m_factory, config.tcp_port, NULL);
      if (gv.listener == NULL || ddsi_listener_listen (gv.listener) != 0)
      {
        NN_ERROR1 ("Failed to create %s listener\n", gv.m_factory->m_typename);
        if (gv.listener)
          ddsi_listener_free(gv.listener);
        goto err_mc_conn;
      }

      /* Set unicast locators from listener */
      set_unspec_locator (&gv.loc_spdp_mc);
      set_unspec_locator (&gv.loc_meta_mc);
      set_unspec_locator (&gv.loc_default_mc);

      ddsi_listener_locator (gv.listener, &gv.loc_meta_uc);
      ddsi_listener_locator (gv.listener, &gv.loc_default_uc);
    }
  }

  /* Create shared transmit connection */

  gv.tev_conn = gv.data_conn_uc;
  TRACE (("Timed event transmit port: %d\n", (int) ddsi_tran_port (gv.tev_conn)));


  /* Create event queues */

  gv.xevents = xeventq_new
  (
    gv.tev_conn,
    config.max_queued_rexmit_bytes,
    config.max_queued_rexmit_msgs,
    0
  );

  gv.as_disc = new_addrset ();
  add_to_addrset (gv.as_disc, &gv.loc_spdp_mc);
  if (config.peers)
    add_peer_addresses (gv.as_disc, config.peers);
  if (config.peers_group)
  {
    gv.as_disc_group = new_addrset ();
    add_peer_addresses (gv.as_disc_group, config.peers_group);
  }
  else
  {
    gv.as_disc_group = NULL;
  }

  gv.gcreq_queue = gcreq_queue_new ();

  /* We create the rbufpool for the receive thread, and so we'll
     become the initial owner thread. The receive thread will change
     it before it does anything with it. */
  if ((gv.rbufpool = nn_rbufpool_new (config.rbuf_size, config.rmsg_chunk_size)) == NULL)
  {
    NN_FATAL0 ("rtps_init: can't allocate receive buffer pool\n");
  }

  gv.rtps_keepgoing = 1;
  os_rwlockInit (&gv.qoslock, NULL);

  {
    int r;
    gv.builtins_dqueue = nn_dqueue_new ("builtins", config.delivery_queue_maxsamples, builtins_dqueue_handler, NULL);
    if ((r = xeventq_start (gv.xevents, NULL)) < 0)
    {
      NN_FATAL1 ("failed to start global event processing thread (%d)\n", r);
    }
  }

  gv.user_dqueue = nn_dqueue_new ("user", config.delivery_queue_maxsamples, user_dqueue_handler, NULL);

  gv.recv_ts = create_thread ("recv", (void * (*) (void *)) recv_thread, gv.rbufpool);
  if (gv.listener)
  {
    gv.listen_ts = create_thread ("listen", (void * (*) (void *)) listen_thread, gv.listener);
  }

  if (gv.startup_mode)
  {
    qxev_end_startup_mode (add_duration_to_mtime (now_mt (), config.startup_mode_duration));
  }

  return 0;

err_mc_conn:
  if (gv.disc_conn_mc)
    ddsi_conn_free (gv.disc_conn_mc);
  if (gv.data_conn_mc)
    ddsi_conn_free (gv.data_conn_mc);
  if (gv.pcap_fp)
    os_mutexDestroy (&gv.pcap_lock);
  os_sockWaitsetFree (gv.waitset);
  if (gv.disc_conn_uc == gv.data_conn_uc)
    ddsi_conn_free (gv.data_conn_uc);
  else
  {
    ddsi_conn_free (gv.data_conn_uc);
    ddsi_conn_free (gv.disc_conn_uc);
  }
err_unicast_sockets:
  nn_reorder_free (gv.spdp_reorder);
  nn_defrag_free (gv.spdp_defrag);
  os_mutexDestroy (&gv.spdp_lock);
  os_mutexDestroy (&gv.lock);
  os_mutexDestroy (&gv.privileged_pp_lock);
  ephash_free (gv.guid_hash);
  deleted_participants_admin_fini ();
  lease_management_term ();
  os_condDestroy (&gv.participant_set_cond);
  os_mutexDestroy (&gv.participant_set_lock);
  nn_xqos_fini (&gv.builtin_endpoint_xqos_wr);
  nn_xqos_fini (&gv.builtin_endpoint_xqos_rd);
  nn_xqos_fini (&gv.spdp_endpoint_xqos);
  nn_xqos_fini (&gv.default_xqos_pub);
  nn_xqos_fini (&gv.default_xqos_sub);
  nn_xqos_fini (&gv.default_xqos_tp);
  nn_xqos_fini (&gv.default_xqos_wr_nad);
  nn_xqos_fini (&gv.default_xqos_wr);
  nn_xqos_fini (&gv.default_xqos_rd);
  nn_plist_fini (&gv.default_plist_pp);
  ddsi_serstatepool_free (gv.serpool);
  nn_xmsgpool_free (gv.xmsgpool);
  (ddsi_plugin.fini_fn) ();
err_set_ext_address:
  while (gv.recvips)
  {
    struct ospl_in_addr_node *n = gv.recvips;
    gv.recvips = n->next;
    os_free (n);
  }
err_set_recvips:
  {
    int i;
    for (i = 0; i < gv.n_interfaces; i++)
      os_free (gv.interfaces[i].name);
  }
err_find_own_ip:
  ddsi_factory_free (gv.m_factory);
err_udp_tcp_init:
  if (config.tp_enable)
    ut_thread_pool_free (gv.thread_pool);
  return -1;
}

struct dq_builtins_ready_arg {
  os_mutex lock;
  os_cond cond;
  int ready;
};

static void builtins_dqueue_ready_cb (void *varg)
{
  struct dq_builtins_ready_arg *arg = varg;
  os_mutexLock (&arg->lock);
  arg->ready = 1;
  os_condBroadcast (&arg->cond);
  os_mutexUnlock (&arg->lock);
}

void rtps_term_prep (void)
{
  /* Stop all I/O */
  os_mutexLock (&gv.lock);
  if (gv.rtps_keepgoing)
  {
    gv.rtps_keepgoing = 0; /* so threads will stop once they get round to checking */
    pa_fence ();
    /* can't wake up throttle_writer, currently, but it'll check every few seconds */
    os_sockWaitsetTrigger (gv.waitset);
  }
  os_mutexUnlock (&gv.lock);
}

void rtps_term (void)
{
  struct thread_state1 *self = lookup_thread_state ();

  /* Stop all I/O */
  rtps_term_prep ();
  join_thread (gv.recv_ts, NULL);

  if (gv.listener)
  {
    ddsi_listener_unblock(gv.listener);
    join_thread (gv.listen_ts, NULL);
    ddsi_listener_free(gv.listener);
  }

  xeventq_stop (gv.xevents);

  /* Send a bubble through the delivery queue for built-ins, so that any
     pending proxy participant discovery is finished before we start
     deleting them */
  {
    struct dq_builtins_ready_arg arg;
    os_mutexInit (&arg.lock, NULL);
    os_condInit (&arg.cond, &arg.lock, NULL);
    arg.ready = 0;
    nn_dqueue_enqueue_callback(gv.builtins_dqueue, builtins_dqueue_ready_cb, &arg);
    os_mutexLock (&arg.lock);
    while (!arg.ready)
      os_condWait (&arg.cond, &arg.lock);
    os_mutexUnlock (&arg.lock);
    os_condDestroy (&arg.cond);
    os_mutexDestroy (&arg.lock);
  }

  /* Once the receive threads have stopped, defragmentation and
     reorder state can't change anymore, and can be freed safely. */
  nn_reorder_free (gv.spdp_reorder);
  nn_defrag_free (gv.spdp_defrag);
  os_mutexDestroy (&gv.spdp_lock);

  {
    struct ephash_enum_proxy_participant est;
    struct proxy_participant *proxypp;
    /* Clean up proxy readers, proxy writers and proxy
       participants. Deleting a proxy participants deletes all its
       readers and writers automatically */
    thread_state_awake (self);
    ephash_enum_proxy_participant_init (&est);
    while ((proxypp = ephash_enum_proxy_participant_next (&est)) != NULL)
    {
      delete_proxy_participant_by_guid(&proxypp->e.guid, 1);
    }
    ephash_enum_proxy_participant_fini (&est);
    thread_state_asleep (self);
  }

  {
    const nn_vendorid_t ownvendorid = MY_VENDOR_ID;
    struct ephash_enum_writer est_wr;
    struct ephash_enum_reader est_rd;
    struct ephash_enum_participant est_pp;
    struct participant *pp;
    struct writer *wr;
    struct reader *rd;
    /* Delete readers, writers and participants, relying on
       delete_participant to schedule the deletion of the built-in
       rwriters to get all SEDP and SPDP dispose+unregister messages
       out. FIXME: need to keep xevent thread alive for a while
       longer. */
    thread_state_awake (self);
    ephash_enum_writer_init (&est_wr);
    while ((wr = ephash_enum_writer_next (&est_wr)) != NULL)
    {
      if (!is_builtin_entityid (wr->e.guid.entityid, ownvendorid))
        delete_writer_nolinger (&wr->e.guid);
    }
    ephash_enum_writer_fini (&est_wr);
    thread_state_awake (self);
    ephash_enum_reader_init (&est_rd);
    while ((rd = ephash_enum_reader_next (&est_rd)) != NULL)
    {
      if (!is_builtin_entityid (rd->e.guid.entityid, ownvendorid))
        delete_reader (&rd->e.guid);
    }
    ephash_enum_reader_fini (&est_rd);
    thread_state_awake (self);
    ephash_enum_participant_init (&est_pp);
    while ((pp = ephash_enum_participant_next (&est_pp)) != NULL)
    {
      delete_participant (&pp->e.guid);
    }
    ephash_enum_participant_fini (&est_pp);
    thread_state_asleep (self);
  }

  /* Wait until all participants are really gone => by then we can be
     certain that no new GC requests will be added */
  os_mutexLock (&gv.participant_set_lock);
  while (gv.nparticipants > 0)
    os_condWait (&gv.participant_set_cond, &gv.participant_set_lock);
  os_mutexUnlock (&gv.participant_set_lock);

  /* Clean up privileged_pp -- it must be NULL now (all participants
     are gone), but the lock still needs to be destroyed */
  assert (gv.privileged_pp == NULL);
  os_mutexDestroy (&gv.privileged_pp_lock);

  /* Shut down the GC system -- no new requests will be added */
  gcreq_queue_free (gv.gcreq_queue);

  /* No new data gets added to any admin, all synchronous processing
     has ended, so now we can drain the delivery queues to end up with
     the expected reference counts all over the radmin thingummies. */
  nn_dqueue_free (gv.builtins_dqueue);

  nn_dqueue_free (gv.user_dqueue);

  xeventq_free (gv.xevents);


  ut_thread_pool_free (gv.thread_pool);

  os_sockWaitsetFree (gv.waitset);

  (void) joinleave_spdp_defmcip (0);

  ddsi_conn_free (gv.disc_conn_mc);
  ddsi_conn_free (gv.data_conn_mc);
  if (gv.disc_conn_uc == gv.data_conn_uc)
  {
    ddsi_conn_free (gv.data_conn_uc);
  }
  else
  {
    ddsi_conn_free (gv.data_conn_uc);
    ddsi_conn_free (gv.disc_conn_uc);
  }

  /* Not freeing gv.tev_conn: it aliases data_conn_uc */

  ddsi_factory_free (gv.m_factory);

  if (gv.pcap_fp)
  {
    os_mutexDestroy (&gv.pcap_lock);
    fclose (gv.pcap_fp);
  }

  unref_addrset (gv.as_disc);
  unref_addrset (gv.as_disc_group);

  /* Must delay freeing of rbufpools until after *all* references have
     been dropped, which only happens once all receive threads have
     stopped, defrags and reorders have been freed, and all delivery
     queues been drained.  I.e., until very late in the game. */
  nn_rbufpool_free (gv.rbufpool);

  ephash_free (gv.guid_hash);
  deleted_participants_admin_fini ();
  lease_management_term ();
  os_mutexDestroy (&gv.participant_set_lock);
  os_condDestroy (&gv.participant_set_cond);

  nn_xqos_fini (&gv.builtin_endpoint_xqos_wr);
  nn_xqos_fini (&gv.builtin_endpoint_xqos_rd);
  nn_xqos_fini (&gv.spdp_endpoint_xqos);
  nn_xqos_fini (&gv.default_xqos_pub);
  nn_xqos_fini (&gv.default_xqos_sub);
  nn_xqos_fini (&gv.default_xqos_tp);
  nn_xqos_fini (&gv.default_xqos_wr_nad);
  nn_xqos_fini (&gv.default_xqos_wr);
  nn_xqos_fini (&gv.default_xqos_rd);
  nn_plist_fini (&gv.default_plist_pp);

  os_mutexDestroy (&gv.lock);
  os_rwlockDestroy (&gv.qoslock);

  while (gv.recvips)
  {
    struct ospl_in_addr_node *n = gv.recvips;
    gv.recvips = n->next;
    os_free (n);
  }

  {
    int i;
    for (i = 0; i < (int) gv.n_interfaces; i++)
      os_free (gv.interfaces[i].name);
  }

  ddsi_serstatepool_free (gv.serpool);
  nn_xmsgpool_free (gv.xmsgpool);
  (ddsi_plugin.fini_fn) ();
}

/* SHA1 not available (unoffical build.) */

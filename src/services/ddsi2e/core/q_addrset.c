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
#include <string.h>
#include <stddef.h>
#include <assert.h>

#include "os_heap.h"
#include "os_socket.h"
#include "os_stdlib.h"
#include "os_atomics.h"

#include "ut_avl.h"
#include "q_log.h"
#include "q_misc.h"
#include "q_config.h"
#include "q_addrset.h"
#include "q_globals.h" /* gv.mattr */

/* So what does one do with const & mutexes? I need to take lock in a
   pure function just in case some other thread is trying to change
   something. Arguably, that means the thing isn't const; but one
   could just as easily argue that "const" means "this call won't
   change it". If it is globally visible before the call, it may
   change anyway.

   Today, I'm taking the latter interpretation. But all the
   const-discarding casts get moved into LOCK/UNLOCK macros. */
#define LOCK(as) (os_mutexLock (&((struct addrset *) (as))->lock))
#define TRYLOCK(as) (os_mutexTryLock (&((struct addrset *) (as))->lock))
#define UNLOCK(as) (os_mutexUnlock (&((struct addrset *) (as))->lock))

static int compare_locators_vwrap (const void *va, const void *vb);

static const ut_avlCTreedef_t addrset_treedef =
  UT_AVL_CTREEDEF_INITIALIZER (offsetof (struct addrset_node, avlnode), offsetof (struct addrset_node, loc), compare_locators_vwrap, 0);

static int add_addresses_to_addrset_1 (struct addrset *as, const char *ip, int port_mode, const char *msgtag, int req_mc)
{
  char buf[INET6_ADDRSTRLEN_EXTENDED];
  os_sockaddr_storage tmpaddr;
  nn_locator_t loc;
  os_int32 kind;

  if (config.useIpv6)
    kind = config.tcp_enable ? NN_LOCATOR_KIND_TCPv6 : NN_LOCATOR_KIND_UDPv6;
  else
    kind = config.tcp_enable ? NN_LOCATOR_KIND_TCPv4 : NN_LOCATOR_KIND_UDPv4;

  if (!os_sockaddrStringToAddress (ip, (os_sockaddr *) &tmpaddr, !config.useIpv6))
  {
    NN_ERROR2 ("%s: %s: not a valid address\n", msgtag, ip);
    return -1;
  }
  if ((config.useIpv6 && tmpaddr.ss_family != AF_INET6) || (!config.useIpv6 && tmpaddr.ss_family != AF_INET))
  {
    NN_ERROR3 ("%s: %s: not a valid IPv%d address\n", msgtag, ip, config.useIpv6 ? 6 : 4);
    return -1;
  }
  nn_address_to_loc (&loc, &tmpaddr, kind);
  if (req_mc && !is_mcaddr (&loc))
  {
    NN_ERROR2 ("%s: %s: not a multicast address\n", msgtag, ip);
    return -1;
  }

  if (port_mode >= 0)
  {
    loc.port = (unsigned) port_mode;
    nn_log (LC_CONFIG, "%s: add %s", msgtag, locator_to_string_with_port (buf, &loc));
    add_to_addrset (as, &loc);
  }
  else
  {
    nn_log (LC_CONFIG, "%s: add ", msgtag);
    if (!is_mcaddr (&loc))
    {
      int i;
      for (i = 0; i <= config.maxAutoParticipantIndex; i++)
      {
        int port = config.port_base + config.port_dg * config.domainId + i * config.port_pg + config.port_d1;
        loc.port = (unsigned) port;
        if (i == 0)
          nn_log (LC_CONFIG, "%s", locator_to_string_with_port (buf, &loc));
        else
          nn_log (LC_CONFIG, ", :%d", port);
        add_to_addrset (as, &loc);
      }
    }
    else
    {
      int port = port_mode;
      if (port == -1)
        port = config.port_base + config.port_dg * config.domainId + config.port_d0;
      loc.port = (unsigned) port;
      nn_log (LC_CONFIG, "%s", locator_to_string_with_port (buf, &loc));
      add_to_addrset (as, &loc);
    }
  }

  nn_log (LC_CONFIG, "\n");
  return 0;
}

int add_addresses_to_addrset (struct addrset *as, const char *addrs, int port_mode, const char *msgtag, int req_mc)
{
  /* port_mode: -1  => take from string, if 0 & unicast, add for a range of participant indices;
     port_mode >= 0 => always set port to port_mode
  */
  char *addrs_copy, *ip, *cursor, *a;
  int retval = -1;
  addrs_copy = os_strdup (addrs);
  ip = os_malloc (strlen (addrs) + 1);
  cursor = addrs_copy;
  while ((a = os_strsep (&cursor, ",")) != NULL)
  {
    int port = 0, pos;
    if (!config.useIpv6)
    {
      if (port_mode == -1 && sscanf (a, "%[^:]:%d%n", ip, &port, &pos) == 2 && a[pos] == 0)
        ; /* XYZ:PORT */
      else if (sscanf (a, "%[^:]%n", ip, &pos) == 1 && a[pos] == 0)
        port = port_mode; /* XYZ */
      else { /* XY:Z -- illegal, but conversion routine should flag it */
        strcpy (ip, a);
        port = 0;
      }
    }
    else
    {
      if (port_mode == -1 && sscanf (a, "[%[^]]]:%d%n", ip, &port, &pos) == 2 && a[pos] == 0)
        ; /* [XYZ]:PORT */
      else if (sscanf (a, "[%[^]]]%n", ip, &pos) == 1 && a[pos] == 0)
        port = port_mode; /* [XYZ] */
      else { /* XYZ -- let conversion routines handle errors */
        strcpy (ip, a);
        port = 0;
      }
    }

    if ((port > 0 && port <= 65535) || (port_mode == -1 && port == -1)) {
      if (add_addresses_to_addrset_1 (as, ip, port, msgtag, req_mc) < 0)
        goto error;
    } else {
      NN_ERROR3 ("%s: %s: port %d invalid\n", msgtag, a, port);
    }
  }
  retval = 0;
 error:
  os_free (ip);
  os_free (addrs_copy);
  return retval;
}

int compare_locators (const nn_locator_t *a, const nn_locator_t *b)
{
  int c;
  if (a->kind != b->kind)
    return (int) (a->kind - b->kind);
  else if ((c = memcmp (a->address, b->address, sizeof (a->address))) != 0)
    return c;
  else
    return (int) (a->port - b->port);
}

static int compare_locators_vwrap (const void *va, const void *vb)
{
  return compare_locators (va, vb);
}

struct addrset *new_addrset (void)
{
  struct addrset *as = os_malloc (sizeof (*as));
  pa_st32 (&as->refc, 1);
  os_mutexInit (&as->lock, NULL);
  ut_avlCInit (&addrset_treedef, &as->ucaddrs);
  ut_avlCInit (&addrset_treedef, &as->mcaddrs);
  return as;
}

struct addrset *ref_addrset (struct addrset *as)
{
  if (as != NULL)
  {
    pa_inc32 (&as->refc);
  }
  return as;
}

void unref_addrset (struct addrset *as)
{
  if ((as != NULL) && (pa_dec32_nv (&as->refc) == 0))
  {
    ut_avlCFree (&addrset_treedef, &as->ucaddrs, os_free);
    ut_avlCFree (&addrset_treedef, &as->mcaddrs, os_free);
    os_mutexDestroy (&as->lock);
    os_free (as);
  }
}

int is_mcaddr (const nn_locator_t *loc)
{
  os_sockaddr_storage tmp;
  nn_loc_to_address (&tmp, loc);
  switch (loc->kind)
  {
    case NN_LOCATOR_KIND_UDPv4: {
      const os_sockaddr_in *x = (const os_sockaddr_in *) &tmp;
      return IN_MULTICAST (ntohl (x->sin_addr.s_addr));
    }
#if OS_SOCKET_HAS_IPV6
    case NN_LOCATOR_KIND_UDPv6: {
      const os_sockaddr_in6 *x = (const os_sockaddr_in6 *) &tmp;
      return IN6_IS_ADDR_MULTICAST (&x->sin6_addr);
    }
#endif
    default: {
      return 0;
    }
  }
}

void set_unspec_locator (nn_locator_t *loc)
{
  loc->kind = NN_LOCATOR_KIND_INVALID;
  loc->port = NN_LOCATOR_PORT_INVALID;
  memset (loc->address, 0, sizeof (loc->address));
}

int is_unspec_locator (const nn_locator_t *loc)
{
  static const nn_locator_t zloc;
  return (loc->kind == NN_LOCATOR_KIND_INVALID &&
          loc->port == NN_LOCATOR_PORT_INVALID &&
          memcmp (&zloc.address, loc->address, sizeof (zloc.address)) == 0);
}

#ifdef DDSI_INCLUDE_SSM
int is_ssm_mcaddr (const nn_locator_t *loc)
{
  os_sockaddr_storage tmp;
  nn_loc_to_address (&tmp, loc);
  switch (loc->kind)
  {
    case NN_LOCATOR_KIND_UDPv4: {
      const os_sockaddr_in *x = (const os_sockaddr_in *) &tmp;
      return (((os_uint32) ntohl (x->sin_addr.s_addr)) >> 24) == 232;
    }
#if OS_SOCKET_HAS_IPV6
    case NN_LOCATOR_KIND_UDPv6: {
      const os_sockaddr_in6 *x = (const os_sockaddr_in6 *) &tmp;
      return x->sin6_addr.s6_addr[0] == 0xff && (x->sin6_addr.s6_addr[1] & 0xf0) == 0x30;
    }
#endif
    default: {
      return 0;
    }
  }
}

int addrset_contains_ssm (const struct addrset *as)
{
  struct addrset_node *n;
  ut_avlCIter_t it;
  LOCK (as);
  for (n = ut_avlCIterFirst (&addrset_treedef, &as->mcaddrs, &it); n; n = ut_avlCIterNext (&it))
  {
    if (is_ssm_mcaddr (&n->loc))
    {
      UNLOCK (as);
      return 1;
    }
  }
  UNLOCK (as);
  return 0;
}

int addrset_any_ssm (const struct addrset *as, nn_locator_t *dst)
{
  struct addrset_node *n;
  ut_avlCIter_t it;
  LOCK (as);
  for (n = ut_avlCIterFirst (&addrset_treedef, &as->mcaddrs, &it); n; n = ut_avlCIterNext (&it))
  {
    if (is_ssm_mcaddr (&n->loc))
    {
      *dst = n->loc;
      UNLOCK (as);
      return 1;
    }
  }
  UNLOCK (as);
  return 0;
}

int addrset_any_non_ssm_mc (const struct addrset *as, nn_locator_t *dst)
{
  struct addrset_node *n;
  ut_avlCIter_t it;
  LOCK (as);
  for (n = ut_avlCIterFirst (&addrset_treedef, &as->mcaddrs, &it); n; n = ut_avlCIterNext (&it))
  {
    if (!is_ssm_mcaddr (&n->loc))
    {
      *dst = n->loc;
      UNLOCK (as);
      return 1;
    }
  }
  UNLOCK (as);
  return 0;
}
#endif

int addrset_purge (struct addrset *as)
{
  LOCK (as);
  ut_avlCFree (&addrset_treedef, &as->ucaddrs, os_free);
  ut_avlCFree (&addrset_treedef, &as->mcaddrs, os_free);
  UNLOCK (as);
  return 0;
}

void add_to_addrset (struct addrset *as, const nn_locator_t *loc)
{
  if (!is_unspec_locator (loc))
  {
    ut_avlIPath_t path;
    ut_avlCTree_t *tree = is_mcaddr (loc) ? &as->mcaddrs : &as->ucaddrs;
    LOCK (as);
    if (ut_avlCLookupIPath (&addrset_treedef, tree, loc, &path) == NULL)
    {
      struct addrset_node *n = os_malloc (sizeof (*n));
      n->loc = *loc;
      ut_avlCInsertIPath (&addrset_treedef, tree, n, &path);
    }
    UNLOCK (as);
  }
}

void remove_from_addrset (struct addrset *as, const nn_locator_t *loc)
{
  ut_avlDPath_t path;
  ut_avlCTree_t *tree = is_mcaddr (loc) ? &as->mcaddrs : &as->ucaddrs;
  struct addrset_node *n;
  LOCK (as);
  if ((n = ut_avlCLookupDPath (&addrset_treedef, tree, loc, &path)) != NULL)
  {
    ut_avlCDeleteDPath (&addrset_treedef, tree, n, &path);
    os_free (n);
  }
  UNLOCK (as);
}

void copy_addrset_into_addrset_uc (struct addrset *as, const struct addrset *asadd)
{
  struct addrset_node *n;
  ut_avlCIter_t it;
  LOCK (asadd);
  for (n = ut_avlCIterFirst (&addrset_treedef, &asadd->ucaddrs, &it); n; n = ut_avlCIterNext (&it))
    add_to_addrset (as, &n->loc);
  UNLOCK (asadd);
}

void copy_addrset_into_addrset_mc (struct addrset *as, const struct addrset *asadd)
{
  struct addrset_node *n;
  ut_avlCIter_t it;
  LOCK (asadd);
  for (n = ut_avlCIterFirst (&addrset_treedef, &asadd->mcaddrs, &it); n; n = ut_avlCIterNext (&it))
    add_to_addrset (as, &n->loc);
  UNLOCK (asadd);
}

void copy_addrset_into_addrset (struct addrset *as, const struct addrset *asadd)
{
  copy_addrset_into_addrset_uc (as, asadd);
  copy_addrset_into_addrset_mc (as, asadd);
}

#ifdef DDSI_INCLUDE_SSM
void copy_addrset_into_addrset_no_ssm_mc (struct addrset *as, const struct addrset *asadd)
{
  struct addrset_node *n;
  ut_avlCIter_t it;
  LOCK (asadd);
  for (n = ut_avlCIterFirst (&addrset_treedef, &asadd->mcaddrs, &it); n; n = ut_avlCIterNext (&it))
  {
    if (!is_ssm_mcaddr (&n->loc))
      add_to_addrset (as, &n->loc);
  }
  UNLOCK (asadd);

}

void copy_addrset_into_addrset_no_ssm (struct addrset *as, const struct addrset *asadd)
{
  copy_addrset_into_addrset_uc (as, asadd);
  copy_addrset_into_addrset_no_ssm_mc (as, asadd);
}

void addrset_purge_ssm (struct addrset *as)
{
  struct addrset_node *n;
  LOCK (as);
  n = ut_avlCFindMin (&addrset_treedef, &as->mcaddrs);
  while (n)
  {
    struct addrset_node *n1 = n;
    n = ut_avlCFindSucc (&addrset_treedef, &as->mcaddrs, n);
    if (is_ssm_mcaddr (&n1->loc))
    {
      ut_avlCDelete (&addrset_treedef, &as->mcaddrs, n1);
      os_free (n1);
    }
  }
  UNLOCK (as);
}
#endif

os_size_t addrset_count (const struct addrset *as)
{
  if (as == NULL)
    return 0;
  else
  {
    os_size_t count;
    LOCK (as);
    count = ut_avlCCount (&as->ucaddrs) + ut_avlCCount (&as->mcaddrs);
    UNLOCK (as);
    return count;
  }
}

os_size_t addrset_count_uc (const struct addrset *as)
{
  if (as == NULL)
    return 0;
  else
  {
    os_size_t count;
    LOCK (as);
    count = ut_avlCCount (&as->ucaddrs);
    UNLOCK (as);
    return count;
  }
}

int addrset_empty_uc (const struct addrset *as)
{
  int isempty;
  LOCK (as);
  isempty = ut_avlCIsEmpty (&as->ucaddrs);
  UNLOCK (as);
  return isempty;
}

int addrset_empty_mc (const struct addrset *as)
{
  int isempty;
  LOCK (as);
  isempty = ut_avlCIsEmpty (&as->mcaddrs);
  UNLOCK (as);
  return isempty;
}

int addrset_empty (const struct addrset *as)
{
  int isempty;
  LOCK (as);
  isempty = ut_avlCIsEmpty (&as->ucaddrs) && ut_avlCIsEmpty (&as->mcaddrs);
  UNLOCK (as);
  return isempty;
}

int addrset_any_uc (const struct addrset *as, nn_locator_t *dst)
{
  LOCK (as);
  if (ut_avlCIsEmpty (&as->ucaddrs))
  {
    UNLOCK (as);
    return 0;
  }
  else
  {
    const struct addrset_node *n = ut_avlCRoot (&addrset_treedef, &as->ucaddrs);
    *dst = n->loc;
    UNLOCK (as);
    return 1;
  }
}

int addrset_any_mc (const struct addrset *as, nn_locator_t *dst)
{
  LOCK (as);
  if (ut_avlCIsEmpty (&as->mcaddrs))
  {
    UNLOCK (as);
    return 0;
  }
  else
  {
    const struct addrset_node *n = ut_avlCRoot (&addrset_treedef, &as->mcaddrs);
    *dst = n->loc;
    UNLOCK (as);
    return 1;
  }
}

struct addrset_forall_helper_arg
{
  addrset_forall_fun_t f;
  void * arg;
};

static void addrset_forall_helper (void *vnode, void *varg)
{
  const struct addrset_node *n = vnode;
  struct addrset_forall_helper_arg *arg = varg;
  arg->f (&n->loc, arg->arg);
}

os_size_t addrset_forall_count (struct addrset *as, addrset_forall_fun_t f, void *arg)
{
  struct addrset_forall_helper_arg arg1;
  os_size_t count;
  arg1.f = f;
  arg1.arg = arg;
  LOCK (as);
  ut_avlCWalk (&addrset_treedef, &as->mcaddrs, addrset_forall_helper, &arg1);
  ut_avlCWalk (&addrset_treedef, &as->ucaddrs, addrset_forall_helper, &arg1);
  count = ut_avlCCount (&as->ucaddrs) + ut_avlCCount (&as->mcaddrs);
  UNLOCK (as);
  return count;
}

void addrset_forall (struct addrset *as, addrset_forall_fun_t f, void *arg)
{
  (void) addrset_forall_count (as, f, arg);
}

int addrset_forone (struct addrset *as, addrset_forone_fun_t f, void *arg)
{
  unsigned i;
  addrset_node_t n;
  ut_avlCTree_t *trees[2];
  ut_avlCIter_t iter;

  trees[0] = &as->mcaddrs;
  trees[1] = &as->ucaddrs;

  for (i = 0; i < 2u; i++)
  {
    n = (addrset_node_t) ut_avlCIterFirst (&addrset_treedef, trees[i], &iter);
    while (n)
    {
      if ((f) (&n->loc, arg) > 0)
      {
        return 0;
      }
      n = (addrset_node_t) ut_avlCIterNext (&iter);
    }
  }
  return -1;
}

struct log_addrset_helper_arg
{
  logcat_t tf;
};

static void log_addrset_helper (const nn_locator_t *n, void *varg)
{
  const struct log_addrset_helper_arg *arg = varg;
  char buf[INET6_ADDRSTRLEN_EXTENDED];
  if (config.enabled_logcats & arg->tf)
    nn_log (arg->tf, " %s", locator_to_string_with_port (buf, n));
}

void nn_log_addrset (logcat_t tf, const char *prefix, const struct addrset *as)
{
  if (config.enabled_logcats & tf)
  {
    struct log_addrset_helper_arg arg;
    arg.tf = tf;
    nn_log (tf, "%s", prefix);
    addrset_forall ((struct addrset *) as, log_addrset_helper, &arg); /* drop const, we know it is */
  }
}

static int addrset_eq_onesidederr1 (const ut_avlCTree_t *at, const ut_avlCTree_t *bt)
{
  /* Just checking the root */
  if (ut_avlCIsEmpty (at) && ut_avlCIsEmpty (bt)) {
    return 1;
  } else if (ut_avlCIsSingleton (at) && ut_avlCIsSingleton (bt)) {
    const struct addrset_node *a = ut_avlCRoot (&addrset_treedef, at);
    const struct addrset_node *b = ut_avlCRoot (&addrset_treedef, bt);
    return compare_locators (&a->loc, &b->loc) == 0;
  } else {
    return 0;
  }
}

int addrset_eq_onesidederr (const struct addrset *a, const struct addrset *b)
{
  int iseq;
  if (a == b)
    return 1;
  if (a == NULL || b == NULL)
    return 0;
  LOCK (a);
  if (TRYLOCK (b) == os_resultSuccess)
  {
    iseq =
      addrset_eq_onesidederr1 (&a->ucaddrs, &b->ucaddrs) &&
      addrset_eq_onesidederr1 (&a->mcaddrs, &b->mcaddrs);
    UNLOCK (b);
  }
  else
  {
    /* We could try <lock b ; trylock(a)>, in a loop, &c. Or we can
       just decide it isn't worth the bother. Which it isn't because
       it doesn't have to be an exact check on equality. A possible
       improvement would be to use an rwlock. */
    iseq = 0;
  }
  UNLOCK (a);
  return iseq;
}

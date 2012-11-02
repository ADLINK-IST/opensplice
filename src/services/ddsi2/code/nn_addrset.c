#include <string.h>
#include <stddef.h>

#include "os_heap.h"
#include "os_socket.h"

#include "nn_avl.h"
#include "nn_log.h"
#include "nn_config.h"
#include "nn_addrset.h"

static int compare_addr (const void *va, const void *vb)
{
  return memcmp (va, vb, sizeof (nn_locator_udpv4_t));
}

struct addrset *new_addrset (void)
{
  struct addrset *as = os_malloc (sizeof (*as));
  as->refc = 1;
  avl_init (&as->ucaddrs, offsetof (struct addrset_node, avlnode), offsetof (struct addrset_node, addr), compare_addr, 0, os_free);
  avl_init (&as->mcaddrs, offsetof (struct addrset_node, avlnode), offsetof (struct addrset_node, addr), compare_addr, 0, os_free);
  return as;
}

struct addrset *ref_addrset (struct addrset *as)
{
  if (as != NULL)
    pa_increment (&as->refc);
  return as;
}

void unref_addrset (struct addrset *as)
{
  if (pa_decrement (&as->refc) == 0)
  {
    avl_free (&as->ucaddrs);
    avl_free (&as->mcaddrs);
    os_free (as);
  }
}

static int is_mcaddr (const nn_locator_udpv4_t *addr)
{
  return IN_MULTICAST (ntohl (addr->address));
}

int addrset_purge (struct addrset *as)
{
  avl_free (&as->ucaddrs);
  avl_free (&as->mcaddrs);
  return 0;
}

void add_to_addrset (struct addrset *as, const nn_locator_udpv4_t *addr)
{
  struct addrset_node *n;
  avlparent_t parent;
  struct addrset_avltree *tree = is_mcaddr (addr) ? &as->mcaddrs : &as->ucaddrs;
  if ((n = avl_lookup (tree, addr, &parent)) == NULL)
  {
    n = os_malloc (sizeof (*n));
    avl_init_node (&n->avlnode, parent);
    n->addr = *addr;
    avl_insert (tree, n);
  }
}

static int copy_addrset_into_addrset_helper (void *vnode, void *varg)
{
  const struct addrset_node *n = vnode;
  add_to_addrset (varg, &n->addr);
  return AVLWALK_CONTINUE;
}

void copy_addrset_into_addrset_uc (struct addrset *as, const struct addrset *asadd)
{
  avl_walk ((struct addrset_avltree *) &asadd->ucaddrs, copy_addrset_into_addrset_helper, as);
}


void copy_addrset_into_addrset_mc (struct addrset *as, const struct addrset *asadd)
{
  avl_walk ((struct addrset_avltree *) &asadd->mcaddrs, copy_addrset_into_addrset_helper, as);
}

void copy_addrset_into_addrset (struct addrset *as, const struct addrset *asadd)
{
  copy_addrset_into_addrset_uc (as, asadd);
  copy_addrset_into_addrset_mc (as, asadd);
}

int addrset_empty_uc (struct addrset *as)
{
  return avl_empty (&as->ucaddrs);
}

int addrset_empty_mc (struct addrset *as)
{
  return avl_empty (&as->mcaddrs);
}

int addrset_empty (struct addrset *as)
{
  return addrset_empty_uc (as) && addrset_empty_mc (as);
}

int addrset_any_uc (struct addrset *as, nn_locator_udpv4_t *dst)
{
  if (addrset_empty_uc (as))
    return 0;
  else
  {
    *dst = as->ucaddrs.root->addr;
    return 1;
  }
}

int addrset_any_mc (struct addrset *as, nn_locator_udpv4_t *dst)
{
  if (addrset_empty_mc (as))
    return 0;
  else
  {
    *dst = as->mcaddrs.root->addr;
    return 1;
  }
}

struct addrset_forall_addresses_helper_arg {
  addrset_forall_fun_t f;
  void *arg;
  size_t ret;
};

static int addrset_forall_helper (void *vnode, void *varg)
{
  const struct addrset_node *n = vnode;
  struct addrset_forall_addresses_helper_arg *arg = varg;
  arg->ret += arg->f (&n->addr, arg->arg);
  return AVLWALK_CONTINUE;
}

size_t addrset_forall_addresses (struct addrset *as, addrset_forall_fun_t f, void *arg)
{
  struct addrset_forall_addresses_helper_arg arg1;
  arg1.f = f;
  arg1.arg = arg;
  arg1.ret = 0;
  avl_walk (&as->mcaddrs, addrset_forall_helper, &arg1);
  avl_walk (&as->ucaddrs, addrset_forall_helper, &arg1);
  return arg1.ret;
}

static size_t log_addrset_helper (const nn_locator_udpv4_t *n, void *vtf)
{
  const logcat_t *tf = vtf;
  struct in_addr x;
  x.s_addr = n->address;
  if (config.enabled_logcats & *tf)
    nn_log (*tf, " %s:%d", inet_ntoa (x), n->port);
  return 0;
}

void nn_log_addrset (logcat_t tf, const char *prefix, struct addrset *as)
{
  nn_log (tf, prefix);
  addrset_forall_addresses (as, log_addrset_helper, &tf);
}

static int addrset_eq_maybeimprecise1 (const struct addrset_avltree *at, const struct addrset_avltree *bt)
{
  /* Just checking the root */
  if (at->root == NULL)
    return (bt->root == NULL);
  else if (bt->root == NULL)
    return 0;
  else
    return
      (at->root->avlnode.left == NULL && at->root->avlnode.right == NULL &&
       bt->root->avlnode.left == NULL && bt->root->avlnode.right == NULL &&
       memcmp (&at->root->addr, &bt->root->addr, sizeof (at->root->addr)) == 0);
}

int addrset_eq_maybeimprecise (const struct addrset *a, const struct addrset *b)
{
  if (a == b)
    return 1;
  return
    addrset_eq_maybeimprecise1 (&a->ucaddrs, &b->ucaddrs) &&
    addrset_eq_maybeimprecise1 (&a->mcaddrs, &b->mcaddrs);
}

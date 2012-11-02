#ifndef NN_ADDRSET_H
#define NN_ADDRSET_H

#include "nn_avl.h"
#include "nn_protocol.h"

struct addrset_node {
  STRUCT_AVLNODE (addrset_node_avlnode, struct addrset_node *) avlnode;
  nn_locator_udpv4_t addr;
};

struct addrset {
  os_uint32 refc;
  STRUCT_AVLTREE (addrset_avltree, struct addrset_node *) ucaddrs, mcaddrs;
};

typedef size_t (*addrset_forall_fun_t) (const nn_locator_udpv4_t *addr, void *arg);

struct addrset *new_addrset (void);
struct addrset *ref_addrset (struct addrset *as);
void unref_addrset (struct addrset *as);
void add_to_addrset (struct addrset *as, const nn_locator_udpv4_t *addr);
int addrset_purge (struct addrset *as);
void copy_addrset_into_addrset_uc (struct addrset *as, const struct addrset *asadd);
void copy_addrset_into_addrset_mc (struct addrset *as, const struct addrset *asadd);
void copy_addrset_into_addrset (struct addrset *as, const struct addrset *asadd);
int addrset_empty_uc (struct addrset *as);
int addrset_empty_mc (struct addrset *as);
int addrset_empty (struct addrset *as);
int addrset_any_uc (struct addrset *as, nn_locator_udpv4_t *dst);
int addrset_any_mc (struct addrset *as, nn_locator_udpv4_t *dst);
size_t addrset_forall_addresses (struct addrset *as, addrset_forall_fun_t f, void *arg);
void nn_log_addrset (logcat_t tf, const char *prefix, struct addrset *as);
int addrset_eq_maybeimprecise (const struct addrset *a, const struct addrset *b);

#endif /* NN_ADDRSET_H */

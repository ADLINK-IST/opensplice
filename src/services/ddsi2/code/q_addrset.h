#ifndef NN_ADDRSET_H
#define NN_ADDRSET_H

#include "os_mutex.h"
#include "os_socket.h"

#include "q_avl.h"
#include "q_log.h"
#include "q_protocol.h"

struct addrset_node {
  STRUCT_AVLNODE (addrset_node_avlnode, struct addrset_node *) avlnode;
  os_sockaddr_storage addr;
};

struct addrset {
  os_mutex lock;
  os_uint32 refc;
  STRUCT_AVLTREE (addrset_avltree, struct addrset_node *) ucaddrs, mcaddrs;
};

typedef size_t (*addrset_forall_fun_t) (const os_sockaddr_storage *addr, void *arg);

struct addrset *new_addrset (void);
struct addrset *ref_addrset (struct addrset *as);
void unref_addrset (struct addrset *as);
void add_to_addrset (struct addrset *as, const os_sockaddr_storage *addr);
int addrset_purge (struct addrset *as);

/* These lock ASADD, then lock/unlock AS any number of times, then
   unlock ASADD */
void copy_addrset_into_addrset_uc (struct addrset *as, const struct addrset *asadd);
void copy_addrset_into_addrset_mc (struct addrset *as, const struct addrset *asadd);
void copy_addrset_into_addrset (struct addrset *as, const struct addrset *asadd);

int addrset_empty_uc (const struct addrset *as);
int addrset_empty_mc (const struct addrset *as);
int addrset_empty (const struct addrset *as);
int addrset_any_uc (const struct addrset *as, os_sockaddr_storage *dst);
int addrset_any_mc (const struct addrset *as, os_sockaddr_storage *dst);

/* Keeps AS locked */
size_t addrset_forall_addresses (struct addrset *as, addrset_forall_fun_t f, void *arg);
void nn_log_addrset (logcat_t tf, const char *prefix, const struct addrset *as);

/* Tries to lock A then B for a decent check, returning false if
   trylock B fails */
int addrset_eq_onesidederr (const struct addrset *a, const struct addrset *b);

int is_mcaddr (const os_sockaddr_storage *addr);

void init_locator (nn_locator_t *loc, const os_sockaddr_storage *addr, unsigned short port);
int add_addresses_to_addrset (struct addrset *as, const char *addrs, int port_mode, const char *msgtag);

#endif /* NN_ADDRSET_H */

/* SHA1 not available (unoffical build.) */

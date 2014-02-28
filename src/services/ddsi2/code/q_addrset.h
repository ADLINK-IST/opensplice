#ifndef NN_ADDRSET_H
#define NN_ADDRSET_H

#include "os_mutex.h"
#include "os_socket.h"

#include "ut_avl.h"
#include "q_log.h"
#include "q_protocol.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct addrset_node {
  ut_avlNode_t avlnode;
  os_sockaddr_storage addr;
};

struct addrset {
  os_mutex lock;
  os_uint32 refc;
  ut_avlCTree_t ucaddrs, mcaddrs;
};

typedef void (*addrset_forall_fun_t) (const os_sockaddr_storage *addr, void *arg);

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

os_size_t addrset_count (const struct addrset *as);
int addrset_empty_uc (const struct addrset *as);
int addrset_empty_mc (const struct addrset *as);
int addrset_empty (const struct addrset *as);
int addrset_any_uc (const struct addrset *as, os_sockaddr_storage *dst);
int addrset_any_mc (const struct addrset *as, os_sockaddr_storage *dst);

/* Keeps AS locked */
void addrset_forall (struct addrset *as, addrset_forall_fun_t f, void *arg);
void nn_log_addrset (logcat_t tf, const char *prefix, const struct addrset *as);

/* Tries to lock A then B for a decent check, returning false if
   trylock B fails */
int addrset_eq_onesidederr (const struct addrset *a, const struct addrset *b);

int is_mcaddr (const os_sockaddr_storage *addr);

int add_addresses_to_addrset (struct addrset *as, const char *addrs, int port_mode, const char *msgtag);

#if defined (__cplusplus)
}
#endif
#endif /* NN_ADDRSET_H */

/* SHA1 not available (unoffical build.) */

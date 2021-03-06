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
#ifndef NN_ADDRSET_H
#define NN_ADDRSET_H

#include "os_mutex.h"
#include "os_socket.h"

#include "ut_avl.h"
#include "q_log.h"
#include "q_protocol.h"
#include "q_feature_check.h"

#if defined (__cplusplus)
extern "C" {
#endif

typedef struct addrset_node {
  ut_avlNode_t avlnode;
  nn_locator_t loc;
} * addrset_node_t;

struct addrset {
  os_mutex lock;
  pa_uint32_t refc;
  ut_avlCTree_t ucaddrs, mcaddrs;
};

typedef void (*addrset_forall_fun_t) (const nn_locator_t *loc, void *arg);
typedef os_ssize_t (*addrset_forone_fun_t) (const nn_locator_t *loc, void *arg);

struct addrset *new_addrset (void);
struct addrset *ref_addrset (struct addrset *as);
void unref_addrset (struct addrset *as);
void add_to_addrset (struct addrset *as, const nn_locator_t *loc);
void remove_from_addrset (struct addrset *as, const nn_locator_t *loc);
int addrset_purge (struct addrset *as);
int compare_locators (const nn_locator_t *a, const nn_locator_t *b);

/* These lock ASADD, then lock/unlock AS any number of times, then
   unlock ASADD */
void copy_addrset_into_addrset_uc (struct addrset *as, const struct addrset *asadd);
void copy_addrset_into_addrset_mc (struct addrset *as, const struct addrset *asadd);
void copy_addrset_into_addrset (struct addrset *as, const struct addrset *asadd);

os_size_t addrset_count (const struct addrset *as);
os_size_t addrset_count_uc (const struct addrset *as);
int addrset_empty_uc (const struct addrset *as);
int addrset_empty_mc (const struct addrset *as);
int addrset_empty (const struct addrset *as);
int addrset_any_uc (const struct addrset *as, nn_locator_t *dst);
int addrset_any_mc (const struct addrset *as, nn_locator_t *dst);

/* Keeps AS locked */
int addrset_forone (struct addrset *as, addrset_forone_fun_t f, void *arg);
void addrset_forall (struct addrset *as, addrset_forall_fun_t f, void *arg);
os_size_t addrset_forall_count (struct addrset *as, addrset_forall_fun_t f, void *arg);
void nn_log_addrset (logcat_t tf, const char *prefix, const struct addrset *as);

/* Tries to lock A then B for a decent check, returning false if
   trylock B fails */
int addrset_eq_onesidederr (const struct addrset *a, const struct addrset *b);

int is_mcaddr (const nn_locator_t *loc);
int is_unspec_locator (const nn_locator_t *loc);
void set_unspec_locator (nn_locator_t *loc);

int add_addresses_to_addrset (struct addrset *as, const char *addrs, int port_mode, const char *msgtag, int req_mc);

#ifdef DDSI_INCLUDE_SSM
int is_ssm_mcaddr (const nn_locator_t *loc);
int addrset_contains_ssm (const struct addrset *as);
int addrset_any_ssm (const struct addrset *as, nn_locator_t *dst);
int addrset_any_non_ssm_mc (const struct addrset *as, nn_locator_t *dst);
void copy_addrset_into_addrset_no_ssm_mc (struct addrset *as, const struct addrset *asadd);
void copy_addrset_into_addrset_no_ssm (struct addrset *as, const struct addrset *asadd);
void addrset_pruge_ssm (struct addrset *as);
#endif

#if defined (__cplusplus)
}
#endif
#endif /* NN_ADDRSET_H */

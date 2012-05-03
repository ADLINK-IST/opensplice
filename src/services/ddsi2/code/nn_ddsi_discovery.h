#ifndef NN_DDSI_DISCOVERY_H
#define NN_DDSI_DISCOVERY_H

#include "nn_unused.h"
#include "nn_protocol.h"

struct nn_rsample_info;
struct nn_rdata;
struct proxy_participant;
struct addrset;
struct serdata;

void augment_proxy_participant (void *vnode); /* <== shouldn't be here ... */

int nn_spdp_dqueue_handler (const struct nn_rsample_info *sampleinfo, const struct nn_rdata *fragchain, UNUSED_ARG (void *qarg));

int assert_pwr_liveliness_based_on_pp (void *vnode, void *varg);
void assert_pp_and_all_ep_liveliness (struct proxy_participant *pp);

struct serdata *construct_spdp_sample_alive (struct participant *pp);
struct serdata *construct_spdp_sample_dead (struct participant *pp);

struct proxy_participant *new_proxy_participant (nn_guid_t guid, unsigned bes, struct addrset *as_default, struct addrset *as_meta, os_int64 tlease_dur, nn_vendorid_t vendor);
struct proxy_participant *ref_proxy_participant (struct proxy_participant *pp);
void free_proxy_participant (void *vpp);
void unref_proxy_participant (struct proxy_participant *pp);

#endif /* NN_DDSI_DISCOVERY_H */

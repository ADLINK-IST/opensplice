#ifndef Q_LEASE_H
#define Q_LEASE_H

#if defined (__cplusplus)
extern "C" {
#endif

struct receiver_state;
struct participant;
struct lease;
struct entity_common;
struct thread_state1;

void lease_management_init (void);
void lease_management_term (void);
struct lease *lease_new (os_int64 tdur, struct entity_common *e);
void lease_register (struct lease *l);
void lease_free (struct lease *l);
void lease_renew (struct lease *l, os_int64 tnow);
void check_and_handle_lease_expiration (struct thread_state1 *self, os_int64 tnow);

void handle_PMD (const struct receiver_state *rst, unsigned statusinfo, const void *vdata, int len);

#if defined (__cplusplus)
}
#endif

#endif /* Q_LEASE_H */

/* SHA1 not available (unoffical build.) */

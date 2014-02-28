#ifndef Q_EPHASH_H
#define Q_EPHASH_H

#include "os_defs.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct ephash;
struct participant;
struct reader;
struct writer;
struct proxy_participant;
struct proxy_reader;
struct proxy_writer;
struct nn_guid;

struct ephash_chain_entry {
  struct ephash_chain_entry *next;
  struct ephash_chain_entry *prev;

  /* In a bit of a hurry, so use a separate doubly-linked list for
     enumeration and fixed-size hash tables, instead of a dynamically
     resized hash table and iterating over the bins. That'd be
     straightforward if I didn't try to do a lock-free
     lookup. Scanning a 30k entry hash table for a handful of entries
     just doesn't feel right. These are per-tag, we get that for
     free. */
  struct ephash_chain_entry *enum_prev;
  struct ephash_chain_entry *enum_next;
};

struct ephash_enum
{
  struct ephash_chain_entry *cursor;
  struct ephash *ephash;
  struct ephash_enum *next_live;
  struct ephash_enum *prev_live;
};

/* Readers & writers are both in a GUID- and in a GID-keyed table. If
   they are in the GID-based one, they are also in the GUID-based one,
   but not the way around, for two reasons:

   - firstly, there are readers & writers that do not have a GID
     (built-in endpoints, fictitious transient data readers),

   - secondly, they are inserted first in the GUID-keyed one, and then
     in the GID-keyed one.

   The GID is used solely for the interface with the OpenSplice
   kernel, all internal state and protocol handling is done using the
   GUID. So all this means is that, e.g., a writer being deleted
   becomes invisible to the network reader slightly before it
   disappears in the protocol handling, or that a writer might exist
   at the protocol level slightly before the network reader can use it
   to transmit data. */

struct ephash *ephash_new (os_uint32 soft_limit);
void ephash_free (struct ephash *ephash);

void ephash_insert_participant_guid (struct participant *pp);
void ephash_insert_proxy_participant_guid (struct proxy_participant *proxypp);
void ephash_insert_writer_guid (struct writer *wr);
void ephash_insert_reader_guid (struct reader *rd);
void ephash_insert_proxy_writer_guid (struct proxy_writer *pwr);
void ephash_insert_proxy_reader_guid (struct proxy_reader *prd);

void ephash_remove_participant_guid (struct participant *pp);
void ephash_remove_proxy_participant_guid (struct proxy_participant *proxypp);
void ephash_remove_writer_guid (struct writer *wr);
void ephash_remove_reader_guid (struct reader *rd);
void ephash_remove_proxy_writer_guid (struct proxy_writer *pwr);
void ephash_remove_proxy_reader_guid (struct proxy_reader *prd);

struct participant *ephash_lookup_participant_guid (const struct nn_guid *guid);
struct proxy_participant *ephash_lookup_proxy_participant_guid (const struct nn_guid *guid);
struct writer *ephash_lookup_writer_guid (const struct nn_guid *guid);
struct reader *ephash_lookup_reader_guid (const struct nn_guid *guid);
struct proxy_writer *ephash_lookup_proxy_writer_guid (const struct nn_guid *guid);
struct proxy_reader *ephash_lookup_proxy_reader_guid (const struct nn_guid *guid);

struct v_gid_s;
void ephash_insert_writer_gid (struct ephash *gid_hash, struct writer *wr);
void ephash_insert_reader_gid (struct ephash *gid_hash, struct reader *rd);
void ephash_remove_writer_gid (struct ephash *gid_hash, struct writer *wr);
void ephash_remove_reader_gid (struct ephash *gid_hash, struct reader *rd);
struct writer *ephash_lookup_writer_gid (const struct ephash *gid_hash, const struct v_gid_s *gid);
struct reader *ephash_lookup_reader_gid (const struct ephash *gid_hash, const struct v_gid_s *gid);

/* Enumeration of entries in the hash table:

   - "next" visits at least all entries that were in the hash table at
     the time of calling init and that have not subsequently been
     removed;

   - "next" may visit an entry more than once, but will do so only
     because of rare events (i.e., resize or so);

   - the order in which entries are visited is arbitrary;

   - the caller must call init() before it may call next(); it must
     call fini() before it may call init() again. */
struct ephash_enum_participant { struct ephash_enum st; };
struct ephash_enum_writer { struct ephash_enum st; };
struct ephash_enum_reader { struct ephash_enum st; };
struct ephash_enum_proxy_participant { struct ephash_enum st; };
struct ephash_enum_proxy_writer { struct ephash_enum st; };
struct ephash_enum_proxy_reader { struct ephash_enum st; };

void ephash_enum_writer_init (struct ephash_enum_writer *st);
void ephash_enum_reader_init (struct ephash_enum_reader *st);
void ephash_enum_proxy_writer_init (struct ephash_enum_proxy_writer *st);
void ephash_enum_proxy_reader_init (struct ephash_enum_proxy_reader *st);
void ephash_enum_participant_init (struct ephash_enum_participant *st);
void ephash_enum_proxy_participant_init (struct ephash_enum_proxy_participant *st);

struct writer *ephash_enum_writer_next (struct ephash_enum_writer *st);
struct reader *ephash_enum_reader_next (struct ephash_enum_reader *st);
struct proxy_writer *ephash_enum_proxy_writer_next (struct ephash_enum_proxy_writer *st);
struct proxy_reader *ephash_enum_proxy_reader_next (struct ephash_enum_proxy_reader *st);
struct participant *ephash_enum_participant_next (struct ephash_enum_participant *st);
struct proxy_participant *ephash_enum_proxy_participant_next (struct ephash_enum_proxy_participant *st);

void ephash_enum_writer_fini (struct ephash_enum_writer *st);
void ephash_enum_reader_fini (struct ephash_enum_reader *st);
void ephash_enum_proxy_writer_fini (struct ephash_enum_proxy_writer *st);
void ephash_enum_proxy_reader_fini (struct ephash_enum_proxy_reader *st);
void ephash_enum_participant_fini (struct ephash_enum_participant *st);
void ephash_enum_proxy_participant_fini (struct ephash_enum_proxy_participant *st);

#if defined (__cplusplus)
}
#endif

#endif /* Q_EPHASH_H */

/* SHA1 not available (unoffical build.) */

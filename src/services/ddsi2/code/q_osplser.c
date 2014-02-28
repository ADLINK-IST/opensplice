/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include <stddef.h>
#include <ctype.h>
#include <assert.h>

#include "os_stdlib.h"
#include "os_defs.h"
#include "os_thread.h"
#include "os_heap.h"
#include "os_mutex.h"

#include "c_base.h"
#include "v_topic.h"
#include "v_message.h"
#include "v_state.h"

#include "ut_avl.h"
#include "q_osplser.h"
#include "q_osplserModule.h"
#include "q_bswap.h"
#include "q_config.h"
#include "q_log.h"
#include "q_rtps.h"
#include "q_protocol.h"
#include "q_unused.h"
#include "q_misc.h"
#include "q_error.h"
#include "q_globals.h"

#include "sysdeps.h"

/* Keyhash is based on the CDR big-endian encapsulation of all the key
   fields in sequence (DDSI-2.1 9.6.3.3). If this is at most 16 bytes
   long, that's it, with all unfilled bits are zero. If it is
   (potentially) longer than 16 bytes, it is the MD5 hash of it.

   As one would expect, CDR leaves the value of padding bytes
   undefined, and even if one argues that the padding bytes consist of
   "unfilled bits", that strictly speaking only deals with the
   non-MD5-hashed variant. Formally, the MD5 hash has to be computed
   over undefined data ... but that can't work. Presumably all padding
   bytes are supposed to be 0.

   The next question is whether or not there is padding at all ... I
   think there is: "the encapsulation of all key fields in sequence"
   to me reads like the encapsulation of a struct comprised of the
   various key fields in their originally specified order. That means
   padding. The amount of padding differs from that in the normal
   data.

   No sensible person would've invented this! */
#include "q_md5.h"
/* my current guess -- ain't such a sloppily worded spec wonderful? */
#define KEYHASH_HAS_PADDING 0

/* Deserializer loops through arrays, testing whether or not to
   byteswap once fore ach element. Presumably a processor with dynamic
   branch prediction (all normal modern processors except embedded
   stuff) will sail through the test with minimal delay and benefit
   from the smaller code, and a good compiler will duplicate the loop
   and hoist the test out of it for other processors. */
#define CAN_READ_UNALIGNED 1 /* x86 can (and faster than first memcpy'ing) */

/* No key can be at this offset, nor at any offset NOT_A_KEY_OFF +
   sizeof(T) for any struct type T. */
#define NOT_A_KEY_OFF 0x80000000

/* It'd be nice to quickly decide whether COUNT elements of type TYPE
   can fit in NBYTES: for example, for quickly discarding fake CDR
   data meant to consume all available memory.  But the old approach
   based on sizeof(type) is not going to work, because (for example)
   strings in 64-bit mode are 64-bit versus 32-bit + a byte minimum,
   and padding can be different for the very first primitive type of
   the sequence.  This is left as a placeholder, in case some ever
   gets around to really improving the serializer. */
#define GUARANTEED_INSUFFICIENT_BYTES_LEFT(type, count, nbytes) ((void) (type), (void) (count), (void) (nbytes), 0)

#if HAVE_ATOMIC_LIFO && ! defined SERSTATEPOOL_STATISTICS
#define USE_ATOMIC_LIFO 1
#else
#define USE_ATOMIC_LIFO 0
#endif

typedef enum typeclass {
  TC_ONEBYTE,
  TC_TWOBYTES,
  TC_FOURBYTES,
  TC_EIGHTBYTES,
  TC_STRINGREF,
  TC_STRINGINLINE
} typeclass_t;

struct serstate {
  serdata_t data;
  os_int64 twrite; /* time of write - not source timestamp, set post-throttling */
  os_uint32 refcount;
  unsigned pos;
  unsigned size;
  int keyidx; /* current index in topic.keys */
  const struct topic *topic;
  int justkey; /* set when generated using serialize_key */
  serstatepool_t pool;
  struct serstate *next; /* in pool->freelist */
};

struct serstatepool {
#if USE_ATOMIC_LIFO
  os_atomic_lifo_t freelist;
#else
  os_mutex lock;
  int nalloced;
  int nfree;
  serstate_t freelist;
#endif
};

struct keyinfo {
  unsigned long long ord;       /* serialization order (lower values of ord go earlier) */
  unsigned off;                 /* offset from start of (input) sample */
  unsigned keyseroff;           /* offset in serdata.key */
  typeclass_t tc;               /* type class */
  unsigned short align;         /* required alignment */
  unsigned short specord_idx;   /* index in serialization order where current index in specification order can be found (i.e. keys[keys[i].specord_idx] <=> key i in specification order) */
  c_type type;                  /* full type for key (only for (de)serializing keys/keyhashes) */
};

struct topic {
  ut_avlNode_t avlnode;
  char *name_typename;
  char *name;
  char *typename;
  v_topic ospl_topic;
  c_type type;
  /* Array of keys, represented as offset in the OpenSplice internal
     format data blob. Keys must be stored in the order visited by
     serializer (so that the serializer can simply compare the current
     offset with the next key offset). Also: keys[nkeys].off =def=
     ~0u, which won't equal any real offset so that there is no need
     to test for the end of the array.

     Offsets work 'cos only primitive types, enums and strings are
     accepted as keys. So there is no ambiguity if a key happens to
     be inside a nested struct. */
  int nkeys;
  unsigned keysersize;
  struct keyinfo keys[1]; /* C90 does not support flex. array members */
};

static serstate_t serstate_allocnew (serstatepool_t pool, const struct topic *topic);
static void serstate_free (serstate_t st);
static void serstate_ref (serstate_t st);
static void serstate_release (serstate_t st);
static int serialize1 (serstate_t st, C_STRUCT(c_type) const * const type, const char *data, unsigned off);
static int deserialize1 (C_STRUCT(c_type) const * const type, char *dst, const char *src, unsigned srcoff, unsigned srcsize);
static int deserialize1S (C_STRUCT(c_type) const * const type, char *dst, const char *src, unsigned srcoff, unsigned srcsize);
static int deserialize1P (C_STRUCT(c_type) const * const type_x, char **dst, int *dstsize, const char *src, unsigned srcoff, unsigned srcsize, int swap);

static void *serstate_append_specific_alignment (serstate_t st, unsigned n, unsigned a);

topic_t osplser_topic4u;
topic_t osplser_topicpmd;
c_type osplser_topicpmd_type;
c_type osplser_topicpmd_value_type;

static const ut_avlTreedef_t topictree_treedef =
  UT_AVL_TREEDEF_INITIALIZER_INDKEY (offsetof (struct topic, avlnode), offsetof (struct topic, name_typename), (int (*) (const void *, const void *)) strcmp, 0);
static ut_avlTree_t topictree;
static os_mutex topiclock;

#ifndef NDEBUG
static unsigned ispowerof2 (unsigned x)
{
  return x > 0 && !(x & (x-1));
}
#endif

static unsigned alignup (unsigned x, unsigned a)
{
  assert (ispowerof2 (a));
  return -((-x) & (-a));
}

static unsigned ceiling_lg2 (unsigned x)
{
  if (x >= (1u << 31))
    return 32;
  else
  {
    unsigned l = 0;
    while ((1u << l) < x)
      l++;
    return l;
  }
}

static typeclass_t size_to_tc (os_address sz)
{
  switch (sz)
  {
    case 1: return TC_ONEBYTE;
    case 2: return TC_TWOBYTES;
    case 4: return TC_FOURBYTES;
    case 8: return TC_EIGHTBYTES;
    default: assert (0); return TC_ONEBYTE;
  }
}

static unsigned tc_to_size (typeclass_t tc)
{
  switch (tc)
  {
    case TC_ONEBYTE: return 1;
    case TC_TWOBYTES: return 2;
    case TC_FOURBYTES: return 4;
    case TC_EIGHTBYTES: return 8;
    case TC_STRINGREF: return 4;
    case TC_STRINGINLINE: assert (0); return 0;
    default: assert (0); return 0;
  }
}

/*****************************************************************************
 **
 **  topic_t: deftopic, freetopic
 **
 *****************************************************************************/

static int iskeytype (C_STRUCT(c_type) const * const type)
{
  switch (c_baseObjectKind (type))
  {
    case M_PRIMITIVE:
      switch (c_primitiveKind ((c_object) type))
      {
        case P_BOOLEAN:
        case P_CHAR:
        case P_OCTET:
        case P_SHORT: case P_USHORT:
        case P_LONG: case P_ULONG:
        case P_LONGLONG: case P_ULONGLONG:
          return 1;
        default:
          return 0;
      }
      /* NOTREACHED */
    case M_ENUMERATION:
      return 1;
    case M_COLLECTION:
      switch (c_collectionTypeKind ((c_object) type))
      {
        case C_STRING:
          return 1;
        default:
          return 0;
      }
      /* NOTREACHED */
    default:
      return 0;
  }
}

static int findkey (struct keyinfo *ki, C_STRUCT(c_type) const *type, const char *key, unsigned *maxsz)
{
  /* Looks up dot-separated field names in type (accepting only
     structures & ending on a primitive type or a string); sets
     ki->{ord,off,tc,align} and *maxsz. */
  char *keycopy = os_strdup (key), *cursor = keycopy, *name;
  C_STRUCT (c_member) const *member = NULL;
  int ordpos = 63; /* So we never try to compute (uint64_t)0 << 64 */
  ki->ord = 0;
  ki->off = 0;
  type = c_typeActualType ((c_type) type); /* WT? not const? */
  nn_log (LC_TOPIC, "findkey: looking for %s ...\n", key);
  while ((name = os_strsep (&cursor, ".")) != NULL && *name != '\0' &&
         c_baseObjectKind (type) == M_STRUCTURE)
  {
    C_STRUCT(c_structure) const * const structure = c_structure ((c_object) type);
    unsigned i, n = c_arraySize (structure->members);
    nn_log (LC_TOPIC, "  %s:", name);
    for (i = 0; i < n; i++)
    {
      C_STRUCT(c_member) const * const m = structure->members[i];
      C_STRUCT(c_specifier) const * const ms = c_specifier (m);
      nn_log (LC_TOPIC, " %s", ms->name);
      if (strcmp (name, ms->name) == 0)
      {
        member = m;
        type = c_typeActualType (ms->type);
        ki->off += (unsigned) m->offset;
        /* The obvious ord = (ord * n) + i instead of all this
           shifting & power-of-2 stuff won't do: then a deeply nested
           key followed by a shallowly nested one will be reversed in
           the sort. (I do wonder if all this is necessary -- most
           likely the offsets are strictly ascending in the type
           definition ... anyway, this technique requires no
           guarantees from the kernel's type definitions) */
        ordpos -= ceiling_lg2 (n);
        if (ordpos < 0)
        {
          nn_log (LC_TOPIC, " - too deeply nested (n %u -> %u ordpos' %d)\n", n, ceiling_lg2 (n), ordpos);
          goto too_deeply_nested;
        }
        ki->ord |= ((unsigned long long) i) << ordpos;
        nn_log (LC_TOPIC, " - %llx", ki->ord);
        break;
      }
    }
    if (i == n)
    {
      nn_log (LC_TOPIC, " - not found\n");
      goto not_found;
    }
    nn_log (LC_TOPIC, "\n");
  }
  if (name != NULL || member == NULL || !iskeytype (type))
  {
    /* failed to locate member, weirdness in key, attempting to locate
       a member in something other than a struct, ... */
    nn_log (LC_TOPIC, "  not found or not a key type (%p, %p, %d)\n",
            (void *) name, (void *) member, (int) iskeytype (type));
    goto unhappy;
  }
  os_free (keycopy);
  ki->type = (c_type) type;
  switch (c_baseObjectKind ((c_type) type))
  {
    case M_PRIMITIVE:
    case M_ENUMERATION:
      ki->tc = size_to_tc (type->size);
      ki->align = KEYHASH_HAS_PADDING ? type->size : 1;
      *maxsz = 0;
      break;
    case M_COLLECTION:
      assert (c_collectionTypeKind ((c_type) type) == C_STRING);
      /* Usually a ref into data as 32-bit offset from the key buffer;
         but it may be an inlined CDR string, which happens to have
         (yeehaa!) the same alignment because of the included length
         field. */
      ki->tc = TC_STRINGREF;
      ki->align = sizeof (unsigned);
      *maxsz = c_collectionTypeMaxSize ((c_type) type);
      if (*maxsz == 0)
        ; /* leave it */
      else if (*maxsz < sizeof (((struct serdata *) 0)->key) - sizeof (unsigned))
        /* might fit, increment by one to account for terminating '\0' */
        (*maxsz)++;
      else
        /* doesn't fit - this also avoids potential numerical overflows */
        *maxsz = 0;
      break;
    default:
      assert (0);
  }
  return 1;
 unhappy:
 not_found:
 too_deeply_nested:
  os_free (keycopy);
  return 0;
}

static int calc_keyseroff (struct topic *tp, const unsigned *maxstrlengths)
{
  /* Determines offsets in serdata.key & inlines strings when possible */
  unsigned new_keyseroff = 0;
  int i;
#ifndef NDEBUG
  for (i = 0; i < tp->nkeys; i++)
  {
    tp->keys[i].keyseroff = ~0u;
    assert (tp->keys[i].tc != TC_STRINGINLINE);
    if (maxstrlengths[i] != 0)
      assert (tp->keys[i].tc == TC_STRINGREF);
  }
#endif
  nn_log (LC_TOPIC, "calc_keyseroff: %d keys\n", (int) tp->nkeys);
  for (i = 0; i < tp->nkeys; i++)
  {
    unsigned size = tc_to_size (tp->keys[i].tc);
    nn_log (LC_TOPIC, "  key %d kso %u typecode %d size %u align %u\n",
            i, new_keyseroff, (int) tp->keys[i].tc, size, tp->keys[i].align);
    if (maxstrlengths[i] > 0)
    {
      const unsigned inline_align = KEYHASH_HAS_PADDING ? 4 : 1;
      const unsigned inline_size = sizeof (unsigned) + maxstrlengths[i];
      unsigned so = alignup (new_keyseroff, inline_align) + inline_size;
      int j;
      for (j = i + 1; j < tp->nkeys; j++)
        so = alignup (so, tp->keys[j].align) + tc_to_size (tp->keys[j].tc);
      nn_log (LC_TOPIC, "    bounded-string: inline_size %u so %u\n", inline_size, so);
      if (so <= sizeof (((struct serdata *) 0)->key))
      {
        tp->keys[i].tc = TC_STRINGINLINE;
        tp->keys[i].align = inline_align;
        size = inline_size;
        nn_log (LC_TOPIC, "    inlining: size %u align %u\n", size, tp->keys[i].align);
      }
    }
    tp->keys[i].keyseroff = alignup (new_keyseroff, tp->keys[i].align);
    new_keyseroff = tp->keys[i].keyseroff + size;
    nn_log (LC_TOPIC, "    keyseroff %u kso %u\n", tp->keys[i].keyseroff, new_keyseroff);
  }
  tp->keysersize = new_keyseroff;
#ifndef NDEBUG
  /* Weak check if all is well */
  if (tp->nkeys > 0)
  {
    assert (tp->keys[0].keyseroff == 0);
    for (i = 1; i < tp->nkeys; i++)
    {
      assert (tp->keys[i].keyseroff > tp->keys[i-1].keyseroff);
      assert (tp->keys[i].keyseroff < sizeof (((struct serdata *) 0)->key));
    }
  }
#endif
  return (tp->keysersize <= sizeof (((struct serdata *) 0)->key));
}

static int keyinfo_cmp_ord (const struct keyinfo *a, const struct keyinfo *b)
{
  return (a->ord == b->ord) ? 0 : (a->ord < b->ord) ? -1 : 1;
}

static topic_t deftopic_unl (const char *name, C_STRUCT(v_topic) const * const ospl_topic, const char *typename, C_STRUCT(c_type) const * const type, int nkeys, char const * const *keys)
{
  topic_t tp;
  int i;
  unsigned *maxstrlengths; /* for fixed-length strings: maximum string length; anything else: 0 */
  ut_avlIPath_t path;
  char *name_typename;

  /* Reuse known definition, if available; compare is on name+typeame,
     just in case. But we forget about the keys ... */
  if ((name_typename = os_malloc (strlen (name) + 1 + strlen (typename) + 1)) == NULL)
    return NULL;
  os_sprintf (name_typename, "%s/%s", name, typename);
  tp = ut_avlLookupIPath (&topictree_treedef, &topictree, name_typename, &path);
  if (tp != NULL)
  {
    nn_log (LC_TOPIC, "deftopic_unl: reusing definition for %s\n", name_typename);
    os_free (name_typename);
    return tp;
  }

  /* Define new one */
  nn_log (LC_TOPIC, "deftopic_unl: new topic %s\n", name_typename);
  if ((maxstrlengths = os_malloc (nkeys * sizeof (*maxstrlengths))) == NULL && nkeys > 0)
    goto fail00;
  if ((tp = os_malloc (offsetof (struct topic, keys) + (nkeys+1) * sizeof (tp->keys[0]))) == NULL)
    goto fail0;
  tp->name_typename = name_typename;
  if ((tp->name = os_strdup (name)) == NULL)
    goto fail1;
  if ((tp->typename = os_strdup (typename)) == NULL)
    goto fail2;
  tp->ospl_topic = c_keep ((v_topic) ospl_topic); /* drop const */
  tp->type = c_keep ((c_type) type); /* drop const */
  tp->nkeys = nkeys;
  for (i = 0; i < nkeys; i++)
  {
    if (!findkey (&tp->keys[i], type, keys[i], &maxstrlengths[i]))
      goto fail3;
    tp->keys[i].specord_idx = i;
  }
  if (!calc_keyseroff (tp, maxstrlengths))
  {
    /* Keys don't fit in available space */
    goto fail3;
  }
  /* sort on serialization order */
  qsort (tp->keys, nkeys, sizeof (*tp->keys), (int (*) (const void *, const void *)) keyinfo_cmp_ord);
  /* permute specord_idx: each key now contains its original position,
     but we want the reverse */
  {
    unsigned short *tmp;
    if ((tmp = os_malloc (nkeys * sizeof (*tmp))) == NULL && nkeys > 0)
      goto fail4;
    for (i = 0; i < nkeys; i++)
      tmp[tp->keys[i].specord_idx] = i;
    for (i = 0; i < nkeys; i++)
      tp->keys[i].specord_idx = tmp[i];
    os_free (tmp);
  }
  /* sentinel (only "off" really matters, the others are never read) */
  tp->keys[tp->nkeys].off = ~0u;
  os_free (maxstrlengths);
  ut_avlInsertIPath (&topictree_treedef, &topictree, tp, &path);
  return tp;
 fail4:
 fail3:
  if (tp->ospl_topic)
    c_free (tp->ospl_topic);
  c_free (tp->type);
  os_free (tp->typename);
 fail2:
  os_free (tp->name);
 fail1:
  os_free (tp);
 fail0:
  os_free (maxstrlengths);
 fail00:
  os_free (name_typename);
  return NULL;
}

topic_t deftopic (C_STRUCT(v_topic) const * const ospl_topic, const char *keystr)
{
#define MAX_NKEYS 32 /* arbitrary limit out of laziness */
  char *keystr_copy, *pos, *k;
  int nkeys = 0;
  char *keys[MAX_NKEYS];
  topic_t tp;
  C_STRUCT (c_type) const * ospl_type;
  char *ospl_topicname;
  char *ospl_typename;
  if (keystr == NULL)
    keystr = ospl_topic->keyExpr;
  if (keystr == NULL)
    keystr = "";
  pos = keystr_copy = os_strdup (keystr);
  if (*keystr != 0)
  {
    while ((k = os_strsep (&pos, ", \t")) != NULL)
    {
      if (nkeys == MAX_NKEYS)
        goto fail;
      keys[nkeys++] = k;
    }
  }
  ospl_type = v_topicDataType ((v_topic) ospl_topic);
  ospl_typename = c_metaScopedName (c_metaObject (ospl_type));
  ospl_topicname = v_entity ((v_topic) ospl_topic)->name;
  os_mutexLock (&topiclock);
  tp = deftopic_unl (ospl_topicname, ospl_topic, ospl_typename, ospl_type, nkeys, (char const * const *) keys);
  os_mutexUnlock (&topiclock);
  os_free (ospl_typename);
  os_free (keystr_copy);
  return tp;
 fail:
  os_free (keystr_copy);
  return NULL;
#undef MAX_NKEYS
}

const char *topic_name (const struct topic *tp)
{
  return tp->name;
}

const char *topic_typename (const struct topic *tp)
{
  return tp->typename;
}

c_type topic_type (const struct topic *tp)
{
  return (c_type) tp->type;
}

v_topic topic_ospl_topic (const struct topic *tp)
{
  return (v_topic) tp->ospl_topic;
}

int topic_haskey (const struct topic *tp)
{
  return tp->nkeys > 0;
}

static void freetopic_helper (void *vtp)
{
  topic_t tp = vtp;
  c_free (tp->type);
  if (tp->ospl_topic)
    c_free (tp->ospl_topic);
  os_free (tp->name);
  os_free (tp->typename);
  os_free (tp->name_typename);
  os_free (tp);
}

void freetopic (UNUSED_ARG (topic_t tp))
{
#if 0
  /* Never deleting them: the xmit events queue may still contain
     references to a topic via the serialized data when the writer is
     deleted, the easiest fix is to never free a topic. Another option
     is to track references in the queue, yet another is to defer
     freeing a writer until a bubble has passed through the queue,
     &c. &c. */
  ut_avlDelete (&topictree_treedef, &topictree, tp);
  freetopic_helper (tp);
#endif
}

static topic_t deftopic4u (c_base base)
{
  /* Special case for generating serdata in SEDP messages, which
     itself is only required because the rtps.c expects serdata to be
     present in Data messages. To be removed eventually. A topic
     consisting of a 4 unsigned 32-bit integers, all part of the
     key. This corresponds exactly to the key of SEDP ... */
  char const *keys[] = { "a", "b", "c", "d" };
  topic_t tp;
  os_mutexLock (&topiclock);
  tp = deftopic_unl ("....4u....", NULL, "q_osplserModule::type4u", c_resolve (base, "q_osplserModule::type4u"), 4, keys);
  os_mutexUnlock (&topiclock);
  return tp;
}

static topic_t deftopicpmd (c_base base)
{
  /* Special case for generating serdata in PMD */
  char const *keys[] = { "a", "b", "c", "kind" };
  topic_t tp;
  osplser_topicpmd_type = c_resolve (base, "q_osplserModule::pmd");
  osplser_topicpmd_value_type = c_specifier (c_metaResolve ((c_metaObject) osplser_topicpmd_type, "value"))->type;
  os_mutexLock (&topiclock);
  tp = deftopic_unl ("....pmd....", NULL, "q_osplserModule::pmd", osplser_topicpmd_type, 4, keys);
  os_mutexUnlock (&topiclock);
  return tp;
}

int osplser_init (void)
{
  c_base base = gv.ospl_base;
  os_mutexAttr attr;
  if (!loadq_osplserModule (base))
    return ERR_UNSPECIFIED;
  os_mutexAttrInit (&attr);
  attr.scopeAttr = OS_SCOPE_PRIVATE;
  os_mutexInit (&topiclock, &attr);
  ut_avlInit (&topictree_treedef, &topictree);
  osplser_topic4u = deftopic4u (base);
  osplser_topicpmd = deftopicpmd (base);
  return 0;
}

void osplser_fini (void)
{
  ut_avlFree (&topictree_treedef, &topictree, freetopic_helper);
  os_mutexDestroy (&topiclock);
}

/*****************************************************************************
 **
 ** serdata_t: keyhash generation, compare, serialize, release
 **
 *****************************************************************************/

static void gen_keyhash_md5 (struct serdata const * const d, char keyhash[16])
{
  const struct topic *tp = d->u.st->topic;
#if KEYHASH_HAS_PADDING
  /* 64-bit int & double-prec float are largest => never padding
     with more than 7 bytes */
  static const char zeros[7];
#endif
  /* Process the keys in specified order, as opposed to serialization
     order. (Even though the two are almost certainly going to be the
     same, I haven't found a spot where it is guaranteed.) */
  unsigned off = 0;
  int i;
  md5_state_t md5st;
  md5_init (&md5st);
  for (i = 0; i < tp->nkeys; i++)
  {
    const struct keyinfo *ki = &tp->keys[tp->keys[i].specord_idx];
    const char *src = d->key + ki->keyseroff;
#if KEYHASH_HAS_PADDING
    /* Re-do the padding because it may change following a string
       reference (could special case everything up to the first string
       reference, I s'pose -- wouldn't be hard, actually) */
    {
      unsigned off1 = alignup (off, ki->align);
      assert (off1 - off <= sizeof (zeros));
      md5_append (&md5st, zeros, off1 - off);
      off = off1;
    }
#endif
    switch (ki->tc)
    {
      case TC_ONEBYTE:
        md5_append (&md5st, (const unsigned char *) src, 1);
        off += 1;
        break;
      case TC_TWOBYTES:
        md5_append (&md5st, (const unsigned char *) src, 2);
        off += 2;
        break;
      case TC_FOURBYTES:
        md5_append (&md5st, (const unsigned char *) src, 4);
        off += 4;
        break;
      case TC_EIGHTBYTES:
        md5_append (&md5st, (const unsigned char *) src, 8);
        off += 8;
        break;
      case TC_STRINGREF:
        {
          const struct cdrstring *str;
          assert (d->isstringref & (1u << ki->keyseroff));
          str = (const struct cdrstring *) (d->key + *((unsigned *) src));
#if PLATFORM_IS_LITTLE_ENDIAN
          {
            unsigned lenBE = toBE4u (str->length);
            md5_append (&md5st, (const unsigned char *) &lenBE, sizeof (lenBE));
          }
          md5_append (&md5st, (const unsigned char *) str->contents, str->length);
#else
          md5_append (&md5st, (const unsigned char *) str, sizeof (unsigned) + str->length);
#endif
          off += sizeof (unsigned) + str->length;
        }
        break;
      case TC_STRINGINLINE:
        {
          unsigned len;
          assert (!(d->isstringref & (1u << ki->keyseroff)));
#if KEYHASH_HAS_PADDING
          len = fromBE4u (*((const unsigned *) src));
#else
          memcpy (&len, src, sizeof (unsigned));
          len = fromBE4u (len);
#endif
          md5_append (&md5st, (const unsigned char *) src, sizeof (unsigned) + len);
          off += sizeof (unsigned) + len;
        }
        break;
    }
  }
  md5_finish (&md5st, (unsigned char *) keyhash);
}

void serdata_keyhash (const struct serdata *d, char keyhash[16])
{
  /* If any ref'd strings remain, that is because they couldn't be fit
     into d->key. If d->key is at least as long as keyhash, that means
     that won't fit in keyhash either, and so the presence of string
     references and a serialized key larger than keyhash both call for
     the use of MD5. */
  assert (sizeof (d->key) >= 16);
  if (d->isstringref == 0 && (d->u.st->topic == NULL || d->u.st->topic->keysersize <= 16))
    memcpy (keyhash, d->key, 16);
  else
    gen_keyhash_md5 (d, keyhash);
}

/* Find the first bit set in I. (From glibc, obviously correct by
   inspection.) */
static int myffs (int i)
{
  static const unsigned char table[] =
    {
      0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
      6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
      7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
      7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
      8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
      8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
      8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
      8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
    };
  unsigned int a;
  unsigned int x = i & -i;
  a = x <= 0xffff ? (x <= 0xff ? 0 : 8) : (x <= 0xffffff ?  16 : 24);
  return table[x >> a] + a;
}

int serdata_cmp (const struct serdata *a, const struct serdata *b)
{
  unsigned x = a->isstringref;
  unsigned p = 0;
  unsigned keysersize = a->u.st->topic ? a->u.st->topic->keysersize : 16;
  int c;
  assert (a->isstringref == b->isstringref);
  assert (a->u.st->topic == b->u.st->topic);
  while (x)
  {
    unsigned q = myffs (x) - 1;
    x ^= x & -x; /*/ x & -x: lowest set bit => xor clears bit q */
    assert (q >= p);
    assert (p < keysersize);
    assert (q < keysersize);
    if (q > p)
    {
      /* if q > p, one or more scalars precede the next string */
      if ((c = memcmp (a->key + p, b->key + p, q - p)) != 0)
        return c;
      p = q;
    }
    /* compare strings (not lexicographically: no need for that!) */
    {
      const struct cdrstring *astr =
        (const struct cdrstring *) (a->key + *((unsigned *) (a->key + p)));
      const struct cdrstring *bstr =
        (const struct cdrstring *) (b->key + *((unsigned *) (b->key + p)));
      if (astr->length != bstr->length)
        /* just use string length, ignore the text */
        return (astr->length < bstr->length) ? -1 : 1;
      else if ((c = memcmp (astr->contents, bstr->contents, astr->length)) != 0)
        return c;
      p += sizeof (unsigned);
    }
  }
  if (p < keysersize)
    return memcmp (a->key + p, b->key + p, keysersize - p);
  else
    return 0;
}

int serdata_refcount_is_1 (serdata_t serdata)
{
  return (serdata->u.st->refcount == 1);
}

serdata_t serdata_ref (serdata_t serdata)
{
  serstate_ref (serdata->u.st);
  return serdata;
}

void serdata_unref (serdata_t serdata)
{
  serstate_release (serdata->u.st);
}

os_uint32 serdata_size (const struct serdata *serdata)
{
  const struct serstate *st = serdata->u.st;
  return (sizeof (struct CDRHeader) + (os_uint32) st->pos);
}

os_int64 serdata_twrite (const struct serdata *serdata)
{
  return serstate_twrite (serdata->u.st);
}

void serdata_set_twrite (serdata_t serdata, os_int64 twrite)
{
  serstate_set_twrite (serdata->u.st, twrite);
}

int serdata_keyhash_exact_p (const struct serdata *serdata)
{
  return
    serdata->u.st->topic == NULL ||
    (serdata->u.st->topic->keysersize <= 16 && serdata->isstringref == 0);
}

const struct topic *serdata_topic (const struct serdata *serdata)
{
  return serdata->u.st->topic;
}

static serdata_t serialize_raw_private (serstatepool_t pool, const struct topic *tp, const void *data)
{
  serstate_t st = serstate_new (pool, tp);
  if (serialize1 (st, tp->type, data, 0) < 0)
  {
    serstate_release (st);
    return NULL;
  }
  /* Pad to a multiple of 4, the size of the serialized data isn't
     very important so we can safely include some padding bytes.  But
     the padding is pretty much required if we don't want to send
     garbage through nn_xmsg_serdata().  Padding may be garbage, but
     valgrind doesn't like it ... */
  serstate_append_specific_alignment (st, 0, 4);
  return st->data;
}

serdata_t serialize_raw (serstatepool_t pool, const struct topic *tp, const void *data, unsigned statusinfo, os_int64 timestamp, int timestamp_is_now, const struct nn_prismtech_writer_info *wri)
{
  serdata_t d = serialize_raw_private (pool, tp, data);
  if (d)
    serstate_set_msginfo (d->u.st, statusinfo, timestamp, timestamp_is_now, wri);
  return d;
}

static unsigned statusinfo_from_msg (C_STRUCT (v_message) const *msg)
{
  switch (v_nodeState ((v_message) msg))
  {
    case 0:
      /* kernel doesn't produce it state = 0, but we do temporarily
         for pretty-printing incoming data */
    case L_WRITE:
      return 0;
    case L_WRITE | L_DISPOSED:
    case L_DISPOSED:
      return  NN_STATUSINFO_DISPOSE;
    case L_UNREGISTER:
      return NN_STATUSINFO_UNREGISTER;
    default:
      NN_WARNING1 ("statusinfo_from_msg: unhandled message state: %u\n", (unsigned) v_nodeState ((v_message) msg));
      return 0;
  }
}

serdata_t serialize (serstatepool_t pool, const struct topic *tp, C_STRUCT (v_message) const *msg)
{
  const void *udata = C_DISPLACE (msg, tp->ospl_topic->dataField->offset);
  serdata_t d = serialize_raw_private (pool, tp, udata);
  if (d)
  {
    d->v.msginfo.statusinfo = statusinfo_from_msg (msg);
    d->v.msginfo.timestamp = msg->writeTime.seconds * T_SECOND + msg->writeTime.nanoseconds;
    d->v.msginfo.timestamp_is_now = 0;
    d->v.msginfo.have_wrinfo = 1;
    d->v.msginfo.wrinfo.transactionId = msg->transactionId;
    d->v.msginfo.wrinfo.writerGID = msg->writerGID;
    d->v.msginfo.wrinfo.writerInstanceGID = msg->writerInstanceGID;
    d->v.msginfo.wrinfo.sequenceNumber = msg->sequenceNumber;
  }
  return d;
}

serdata_t serialize_key (serstatepool_t pool, const struct topic *tp, C_STRUCT (v_message) const *msg)
{
  /* Only reads key fields from data, have to fiddle a bit with the
     various indices to serialize in specification order without
     messing up key field detection for initializing key array. */
  serstate_t st = serstate_new (pool, tp);
  const void *udata = C_DISPLACE (msg, tp->ospl_topic->dataField->offset);
  int i;
  st->justkey = 1;
  for (i = 0; i < tp->nkeys; i++)
  {
    const int specidx = tp->keys[i].specord_idx;
    const struct keyinfo *ki = &tp->keys[specidx];
    st->keyidx = specidx;
    if (serialize1 (st, ki->type, (const char *) udata + ki->off, ki->off) < 0)
    {
      serstate_release (st);
      return NULL;
    }
  }
  /* set msginfo */
  {
    serdata_t d = st->data;
    d->v.msginfo.statusinfo = statusinfo_from_msg (msg);
    d->v.msginfo.timestamp = msg->writeTime.seconds * T_SECOND + msg->writeTime.nanoseconds;
    d->v.msginfo.timestamp_is_now = 0;
    d->v.msginfo.have_wrinfo = 1;
    d->v.msginfo.wrinfo.transactionId = msg->transactionId;
    d->v.msginfo.wrinfo.writerGID = msg->writerGID;
    d->v.msginfo.wrinfo.writerInstanceGID = msg->writerInstanceGID;
    d->v.msginfo.wrinfo.sequenceNumber = msg->sequenceNumber;
  }
  return st->data;
}

int serdata_is_key (const struct serdata *serdata)
{
  return serdata->u.st->justkey;
}

/*****************************************************************************
 **
 **  serstatepool_t: new, free
 **
 *****************************************************************************/

serstatepool_t serstatepool_new (void)
{
  serstatepool_t pool;
  /* I have to be absolutely certain that a c_bool matches the boolean
     in the CDR spec in its format, or else the serializer will
     produce garbage */
  assert (sizeof (c_bool) == 1);
  pool = os_malloc (sizeof (*pool));

#if USE_ATOMIC_LIFO
  os_atomic_lifo_init (&pool->freelist);
#else
  {
    os_mutexAttr attr;
    os_mutexAttrInit (&attr);
    attr.scopeAttr = OS_SCOPE_PRIVATE;
    os_mutexInit (&pool->lock, &attr);
    pool->freelist = NULL;
    pool->nalloced = 0;
    pool->nfree = 0;
  }
#endif
  return pool;
}

void serstatepool_free (serstatepool_t pool)
{
#if USE_ATOMIC_LIFO
  serstate_t st;
  while ((st = os_atomic_lifo_pop (&pool->freelist, offsetof (struct serstate, next))) != NULL)
    serstate_free (st);
  TRACE (("serstatepool_free(%p)\n", pool));
#else
  while (pool->freelist)
  {
    serstate_t st = pool->freelist;
    pool->freelist = st->next;
    serstate_free (st);
  }
  os_mutexDestroy (&pool->lock);
  nn_log (LC_TOPIC, "serstatepool_free(%p) nalloced %d nfree %d\n", pool, pool->nalloced, pool->nfree);
#endif /* USE_ATOMIC_LIFO */
  os_free (pool);
}

/*****************************************************************************
 **
 **  serstate_t (private)
 **
 *****************************************************************************/

static void serstate_ref (serstate_t st)
{
  atomic_inc_u32_nv (&st->refcount);
}

static void serstate_release (serstate_t st)
{
  if (atomic_dec_u32_nv (&st->refcount) == 0)
  {
    serstatepool_t pool = st->pool;
#if USE_ATOMIC_LIFO
    os_atomic_lifo_push (&pool->freelist, st, offsetof (struct serstate, next));
#else
    os_mutexLock (&pool->lock);
#if 0
#ifndef NDEBUG
    {
      serstate_t b;
      for (b = pool->freelist; b && b != st; b = b->next)
        ;
      assert (b == NULL);
    }
#endif
#endif
    st->next = pool->freelist;
    pool->freelist = st;
    pool->nfree++;
    os_mutexUnlock (&pool->lock);
#endif /* USE_ATOMIC_LIFO */
  }
}

static void serstate_init (serstate_t st, const struct topic *topic)
{
  st->pos = 0;
  st->keyidx = 0;
  st->topic = topic;
  st->data->isstringref = 0;
  st->refcount = 1;
  st->justkey = 0;
  st->twrite = -1;
  if (topic)
    st->data->hdr.identifier = PLATFORM_IS_LITTLE_ENDIAN ? CDR_LE : CDR_BE;
  else
    st->data->hdr.identifier = PLATFORM_IS_LITTLE_ENDIAN ? PL_CDR_LE : PL_CDR_BE;
#ifndef NDEBUG
  st->data->v.msginfo.have_wrinfo = 2; /* invalid value */
#endif
  memset (st->data->key, 0, sizeof (st->data->key));
}

serstate_t serstate_new (serstatepool_t pool, const struct topic *topic)
{
  serstate_t st;
#if USE_ATOMIC_LIFO
  if ((st = os_atomic_lifo_pop (&pool->freelist, offsetof (struct serstate, next))) != NULL)
    serstate_init (st, topic);
  else
    st = serstate_allocnew (pool, topic);
#else
  os_mutexLock (&pool->lock);
  if (pool->freelist == NULL)
  {
    os_mutexUnlock (&pool->lock);
    st = serstate_allocnew (pool, topic);
  }
  else
  {
    st = pool->freelist;
    pool->freelist = st->next;
    pool->nfree--;
    os_mutexUnlock (&pool->lock);
    serstate_init (st, topic);
  }
#endif
  return st;
}

serdata_t serstate_fix (serstate_t st)
{
  /* see serialize_raw_private() */
  serstate_append_specific_alignment (st, 0, 4);
  return st->data;
}

os_int64 serstate_twrite (const struct serstate *serstate)
{
  assert (serstate->twrite >= 0);
  return serstate->twrite;
}

void serstate_set_twrite (serstate_t st, os_int64 twrite)
{
  st->twrite = twrite;
}

static serstate_t serstate_allocnew (serstatepool_t pool, const struct topic *topic)
{
  serstate_t st = os_malloc (sizeof (*st));
#if ! USE_ATOMIC_LIFO
  pool->nalloced++;
#endif
  st->size = 128;
  st->data = os_malloc (offsetof (struct serdata, data) + st->size);
  st->pool = pool;
  st->data->u.st = st;
  st->data->hdr.options = 0;
  serstate_init (st, topic);
  return st;
}

static void serstate_free (serstate_t st)
{
#if ! USE_ATOMIC_LIFO
#ifndef NDEBUG
  {
    serstate_t b;
    for (b = st->pool->freelist; b && b != st; b = b->next)
      ;
    assert (b == NULL);
  }
#endif
#endif
  os_free (st->data);
  os_free (st);
}

static void *serstate_append (serstate_t st, unsigned n)
{
  char *p;
  if (st->pos + n > st->size)
  {
    unsigned size1 = alignup (st->pos + n, 128);
    serdata_t data1 = os_realloc (st->data, offsetof (struct serdata, data) + size1);
    if (data1 == NULL)
    {
      /* out of memory, how unfortunate */
      return NULL;
    }
    else
    {
      st->data = data1;
      st->size = size1;
    }
  }
  assert (st->pos + n <= st->size);
  p = st->data->data + st->pos;
  st->pos += n;
  return p;
}

static void *serstate_append_specific_alignment (serstate_t st, unsigned n, unsigned a)
{
  /* Simply align st->pos, without verifying it fits in the allocated
     buffer: serstate_append() is called immediately afterward and will
     grow the buffer as soon as the end of the requested space no
     longer fits. */
  unsigned pos0 = st->pos;
  char *p;
  assert (ispowerof2 (a));
  st->pos = alignup (st->pos, a);
  p = serstate_append (st, n);
  if (p && st->pos > pos0)
    memset (st->data->data + pos0, 0, st->pos - pos0);
  return p;
}

void *ddsi2direct_serstate_append_specific_alignment (serstate_t st, unsigned n, unsigned a)
{
  return serstate_append_specific_alignment (st, n, a);
}

static void *serstate_append_aligned (serstate_t st, unsigned sz)
{
  /* For primitive types where alignment == size */
  return serstate_append_specific_alignment (st, sz, sz);
}

int serstate_append_blob (serstate_t st, unsigned align, unsigned sz, const void *data)
{
  char *p = serstate_append_specific_alignment (st, sz, align);
  if (p == NULL)
    return ERR_OUT_OF_MEMORY;
  else
  {
    memcpy (p, data, sz);
    return 0;
  }
}

int serstate_set_key (serstate_t st, int justkey, unsigned sz, const void *key)
{
  assert (sz <= 16);
  st->justkey = justkey;
  st->data->isstringref = 0;
  memcpy (st->data->key, key, sz);
  memset (st->data->key + sz, 0, sizeof (st->data->key) - sz);
  return 0;
}

void serstate_set_msginfo (serstate_t st, unsigned statusinfo, os_int64 timestamp, int timestamp_is_now, const struct nn_prismtech_writer_info *wri)
{
  serdata_t d = st->data;
  assert (timestamp_is_now == !!timestamp_is_now);
  d->v.msginfo.statusinfo = statusinfo;
  d->v.msginfo.timestamp = timestamp;
  d->v.msginfo.timestamp_is_now = timestamp_is_now;
  if (wri == NULL)
    d->v.msginfo.have_wrinfo = 0;
  else
  {
    d->v.msginfo.have_wrinfo = 1;
    d->v.msginfo.wrinfo = *wri;
  }
}

/*****************************************************************************
 **
 **  serializer
 **
 *****************************************************************************/

static void copykey (serstate_t st, const char *p)
{
  const struct keyinfo *ki = &st->topic->keys[st->keyidx++];
  char *dst = st->data->key + ki->keyseroff;
#if KEYHASH_HAS_PADDING
  switch (ki->tc)
  {
    case TC_ONEBYTE:
      *dst = *p;
      break;
    case TC_TWOBYTES:
      *((unsigned short *) dst) = toBE2u (*((const unsigned short *) p));
      break;
    case TC_FOURBYTES:
      *((unsigned *) dst) = toBE4u (*((const unsigned *) p));
      break;
    case TC_EIGHTBYTES:
      *((unsigned long long *) dst) = toBE8u (*((const unsigned long long *) p));
      break;
    case TC_STRINGREF:
      *((unsigned *) dst) = (unsigned) (p - st->data.key);
      st->data.isstringref |= (1u << st->keyseroff);
      break;
    case TC_STRINGINLINE:
      {
        unsigned len = *((const unsigned *) p);
#if PLATFORM_IS_LITTLE_ENDIAN
        *((unsigned *) dst) = toBE4u (len);
        memcpy (dst + sizeof (unsigned), p + sizeof (unsigned), len);
#else
        memcpy (dst, p, sizeof (unsigned) + len);
#endif
      }
      break;
  }
#else
  switch (ki->tc)
  {
    case TC_ONEBYTE:
      *dst = *p;
      break;
    case TC_TWOBYTES:
      {
        unsigned short x = toBE2u (*((const unsigned short *) p));
        memcpy (dst, &x, sizeof (x));
      }
      break;
    case TC_FOURBYTES:
      {
        unsigned x = toBE4u (*((const unsigned *) p));
        memcpy (dst, &x, sizeof (x));
      }
      break;
    case TC_EIGHTBYTES:
      {
        unsigned long long x = toBE8u (*((const unsigned long long *) p));
        memcpy (dst, &x, sizeof (x));
      }
      break;
    case TC_STRINGREF:
      assert ((ki->keyseroff % sizeof (unsigned)) == 0);
      *((unsigned *) dst) = (unsigned) (p - st->data->key);
      st->data->isstringref |= (1u << ki->keyseroff);
      break;
    case TC_STRINGINLINE:
      {
        unsigned len = *((const unsigned *) p);
#if PLATFORM_IS_LITTLE_ENDIAN
        unsigned x = toBE4u (len);
        memcpy (dst, &x, sizeof (x));
        memcpy (dst + sizeof (unsigned), p + sizeof (unsigned), len);
#else
        memcpy (dst, p, sizeof (unsigned) + len);
#endif
      }
      break;
  }
#endif /* KEYHASH_HAS_PADDING */
}

static int serprim (serstate_t st, C_STRUCT(c_type) const * const type, const char *data, unsigned off)
{
  char *p;
  if ((p = serstate_append_aligned (st, (os_uint32) type->size)) == NULL)
    return ERR_OUT_OF_MEMORY;
  switch (c_primitiveKind ((c_type) type))
  {
    case P_BOOLEAN: case P_CHAR: case P_OCTET:
      *((char *) p) = *((const char *) data);
      break;
    case P_SHORT: case P_USHORT:
      *((unsigned short *) p) = *((const unsigned short *) data);
      break;
    case P_LONG: case P_ULONG: case P_FLOAT:
      *((unsigned *) p) = *((const unsigned *) data);
      break;
    case P_LONGLONG: case P_ULONGLONG: case P_DOUBLE:
      *((unsigned long long *) p) = *((const unsigned long long *) data);
      break;
    case P_UNDEFINED: case P_ADDRESS: case P_VOIDP: case P_MUTEX:
    case P_LOCK: case P_COND: case P_COUNT:
      abort ();
    case P_WCHAR:
      /* Too big a mess to worry about it now */
      abort ();
  }
  if (off == st->topic->keys[st->keyidx].off)
    copykey (st, p);
  return (int) type->size;
}

static int serenum (serstate_t st, UNUSED_ARG (C_STRUCT(c_type) const * const type), const char *data, unsigned off)
{
  char *p;
  if ((p = serstate_append_aligned (st, sizeof (c_long))) == NULL)
    return ERR_OUT_OF_MEMORY;
  *((c_long *) p) = *((const c_long *) data);
  if (off == st->topic->keys[st->keyidx].off)
    copykey (st, p);
  return (int) sizeof (c_long);
}

static int sercoll (serstate_t st, C_STRUCT(c_type) const * const type, const char *data, unsigned off)
{
  C_STRUCT(c_collectionType) const * const ctype = c_collectionType ((c_type) type);
  int sersize = 0;
  switch (ctype->kind)
  {
    case C_STRING:
      {
        /* data is a c_string* is an os_char** is a char** (it's gotta
           be 'cos it's passed to strlen -- O! the joys of defining
           aliases for primitive types in C ... FIXME: wstrings? */
        const char *s = (const char *) (*((const c_string *) data));
        char *p;
        if (s == NULL)
        {
          /* NULL pointer allowed in OpenSplice, map it onto length-0
             CDR string (which is illegal) */
          sersize = (int) (sizeof (unsigned));
          if ((p = serstate_append_specific_alignment (st, sersize, sizeof (unsigned))) == NULL)
            return ERR_OUT_OF_MEMORY;
          *((unsigned *) p) = 0;
        }
        else
        {
          unsigned n = (int) strlen (s) + 1;
          if (ctype->maxSize > 0 && n-1 > (unsigned) ctype->maxSize)
            return 0;
          sersize = (int) (sizeof (unsigned) + n);
          if ((p = serstate_append_specific_alignment (st, sersize, sizeof (unsigned))) == NULL)
            return ERR_OUT_OF_MEMORY;
          *((unsigned *) p) = n;
          memcpy (p + sizeof (unsigned), s, n);
        }
        if (off == st->topic->keys[st->keyidx].off)
          copykey (st, p);
      }
      break;

    case C_ARRAY:
    case C_SEQUENCE:
      {
        /* Array: maybe reftype (reftype if vla <=> maxSize == 0, I
           think); Sequence: always reftype. (Do I really need to
           bother? Existing DDSI service does, so maybe I do. This is
           *their* logic, not mine!) */
        C_STRUCT(c_type) const * const subtype = ctype->subType;
        const c_metaKind subtypekind = c_baseObjectKind (c_baseObject (subtype));
        const void *array;
        char *p;
        unsigned length;
        if (ctype->kind == C_ARRAY && ctype->maxSize > 0) {
          array = data;
          length = ctype->maxSize;
        } else {
          array = *((void const * const *) data);
          length = c_arraySize ((void *) array); /* drop const */
        }
        /* C_SEQUENCE and by-ref C_ARRAYs (those have unknown static
           size) are both serialized as a CDR sequence, i.e., first
           the number of elements, then the elements themselves. Only
           C_ARRAY with statically determined length can be serialized
           as a CDR array. */
        if (ctype->kind == C_SEQUENCE || ctype->maxSize == 0)
        {
          if ((p = serstate_append_aligned (st, sizeof (unsigned))) == NULL)
            return ERR_OUT_OF_MEMORY;
          *((unsigned *) p) = length;
          sersize += sizeof (unsigned);
        }
        if (subtypekind == M_PRIMITIVE || subtypekind == M_ENUMERATION)
        {
          unsigned size1 = (unsigned) subtype->size;
          unsigned sizeN = length * size1;
          if ((p = serstate_append_specific_alignment (st, sizeN, size1)) == NULL)
            return ERR_OUT_OF_MEMORY;
          memcpy (p, array, sizeN);
          sersize += sizeN;
        }
        else
        {
          unsigned datasize1 = (c_typeIsRef ((c_type) subtype)) ? (unsigned) sizeof (void *) : (unsigned) subtype->size;
          const char *src1 = array;
          unsigned i;
          for (i = 0; i < length; i++)
          {
            int sersize1 = serialize1 (st, subtype, src1, NOT_A_KEY_OFF);
            if (sersize1 < 0)
              return sersize1;
            sersize += sersize1;
            src1 += datasize1;
          }
        }
      }
      break;

    default:
      abort ();
  }
  return sersize;
}

static int serstruct (serstate_t st, C_STRUCT(c_type) const * const type, const char *data, unsigned off)
{
  C_STRUCT(c_structure) const * const structure = c_structure ((c_type) type);
  unsigned n = c_arraySize (structure->members);
  unsigned i;
  int sersize = 0;
  for (i = 0; i < n; i++)
  {
    C_STRUCT(c_member) const * const member = structure->members[i];
    const unsigned disp = (unsigned) member->offset;
    C_STRUCT(c_type) const * const subtype = c_typeActualType (c_specifierType (member));
    int sersize1 = serialize1 (st, subtype, data + disp, off + disp);
    if (sersize1 < 0)
      return sersize1;
    sersize += sersize1;
  }
  return sersize;
}

static c_value get_discriminant_value (C_STRUCT(c_type) const * const dtype, const char *data)
{
  c_value dvalue;
  switch (c_baseObjectKind (dtype))
  {
    case M_PRIMITIVE:
      switch (c_primitiveKind ((c_type) dtype))
      {
#define X(prim, type) case prim: dvalue = type##Value (*((const type *) data)); break
        X (P_BOOLEAN, c_bool);
        X (P_CHAR, c_char);
        X (P_SHORT, c_short);
        X (P_USHORT, c_ushort);
        X (P_LONG, c_long);
        X (P_ULONG, c_ulong);
        X (P_LONGLONG, c_longlong);
        X (P_ULONGLONG, c_ulonglong);
#undef X
        default:
          /* Unsupported type */
          abort ();
      }
      break;
    case M_ENUMERATION:
      dvalue = c_longValue (*(const c_long *) data);
      break;
    default:
      /* Unsupported type */
      abort ();
  }
  return dvalue;
}

static C_STRUCT(c_unionCase) const *active_union_case (C_STRUCT(c_union) const * const utype, const c_value dvalue)
{
  C_STRUCT(c_unionCase) const *defcase = NULL;
  int i, n = (int) c_arraySize (utype->cases);
  for (i = 0; i < n; i++)
  {
    C_STRUCT(c_unionCase) const * const c = c_unionCase (utype->cases[i]);
    unsigned nlab = c_arraySize (c->labels);
    if (nlab == 0)
    {
      /* Only one default would be reasonable, wouldn't it? Except I
         don't know if it is guaranteed ... Maybe the default case is
         always the last one -- that's what I'd have done, especially
         if a default case is required (or is it? first
         an assertion, then a test ...) */
      defcase = c;
    }
    else
    {
      int j;
      for (j = 0; j < (int) nlab; j++)
      {
        C_STRUCT(c_literal) const * const label = c_literal (c->labels[j]);
        if (c_valueCompare (dvalue, label->value) == C_EQ)
          return c;
      }
    }
  }
  return defcase;
}

static int serunion (serstate_t st, C_STRUCT(c_type) const * const type, const char *data, unsigned off)
{
  C_STRUCT(c_union) const * const utype = c_union ((c_type) type);
  C_STRUCT(c_type) const * const dtype = c_typeActualType (utype->switchType);
  const c_value dvalue = get_discriminant_value (dtype, data);
  C_STRUCT(c_unionCase) const * const activecase = active_union_case (utype, dvalue);
  int sersize = 0;
  int sersize1;
  if ((sersize1 = serialize1 (st, dtype, data, off)) < 0)
    return sersize1;
  sersize += sersize1;
  if (activecase)
  {
    const unsigned disp = alignup ((unsigned) dtype->size, (unsigned) c_type (utype)->alignment);
    C_STRUCT(c_type) const * const subtype = c_typeActualType (c_specifierType (activecase));
    sersize1 = serialize1 (st, subtype, data + disp, off + disp);
    if (sersize1 < 0)
      return sersize1;
    sersize += sersize1;
  }
  return sersize;
}

static int serialize1 (serstate_t st, C_STRUCT(c_type) const * const type_x, const char *data, unsigned off)
{
  c_type type = c_typeActualType ((c_type) type_x);
  switch (c_baseObjectKind (type))
  {
    case M_PRIMITIVE:
      return serprim (st, type, data, off);
    case M_ENUMERATION:
      return serenum (st, type, data, off);
    case M_COLLECTION:
      return sercoll (st, type, data, off);
    case M_STRUCTURE:
      return serstruct (st, type, data, off);
    case M_UNION:
      return serunion (st, type, data, off);
    default:
      /* Do any others need to be supported? */
      abort ();
      return 0;
  }
}

/*****************************************************************************
 **
 **  deserializer (everything twice: straight & swapped)
 **
 *****************************************************************************/

static void mysnprintf (char **dst, int *dstsize, const char *fmt, ...)
{
  va_list ap;
  int n;
  if (*dstsize <= 0)
  {
    /* OS abstraction wrapper has issues with size-0 buffers, and there
       may well be other implementations that cause issues. We don't
       really care, and just pretend one character was output. This may
       cause a spurious "(trunc)" in the output, but only if a call causes
       the string to reach its maximum length, and all further calls
       really have a zero-length output. That issue is negligible. */
    (*dstsize)--;
  }
  else
  {
    va_start (ap, fmt);
    n = os_vsnprintf (*dst, *dstsize, fmt, ap);
    va_end (ap);
    if (n < *dstsize)
      *dst += n;
    else
      *dst += *dstsize;
    *dstsize -= n;
  }
}

static int deserprim (C_STRUCT(c_type) const * const type, char *dst, const char *src, unsigned srcoff, unsigned srcsize)
{
  srcoff = alignup (srcoff, (unsigned) type->size);
  if (srcoff + (unsigned) type->size > srcsize)
    return ERR_INVALID_DATA;
  switch (c_primitiveKind ((c_type) type))
  {
    case P_BOOLEAN: case P_CHAR: case P_OCTET:
      *((char *) dst) = *((const char *) (src + srcoff));
      break;
    case P_SHORT: case P_USHORT:
      memcpy (dst, src + srcoff, sizeof (unsigned short));
      break;
    case P_LONG: case P_ULONG: case P_FLOAT:
      memcpy (dst, src + srcoff, sizeof (unsigned));
      break;
    case P_LONGLONG: case P_ULONGLONG: case P_DOUBLE:
      memcpy (dst, src + srcoff, sizeof (unsigned long long));
      break;
    case P_UNDEFINED: case P_ADDRESS: case P_VOIDP: case P_MUTEX:
    case P_LOCK: case P_COND: case P_COUNT:
      abort ();
    case P_WCHAR:
      /* Too big a mess to worry about it now */
      abort ();
  }
  return srcoff + (unsigned) type->size;
}

static int deserprimS (C_STRUCT(c_type) const * const type, char *dst, const char *src, unsigned srcoff, unsigned srcsize)
{
  srcoff = alignup (srcoff, (unsigned) type->size);
  if (srcoff + (unsigned) type->size > srcsize)
    return ERR_INVALID_DATA;
  switch (c_primitiveKind ((c_type) type))
  {
    case P_BOOLEAN: case P_CHAR: case P_OCTET:
      *((char *) dst) = *((const char *) (src + srcoff));
      break;
    case P_SHORT: case P_USHORT:
      {
        unsigned short tmp;
        memcpy (&tmp, src + srcoff, sizeof (tmp));
        *((unsigned short *) dst) = bswap2u (tmp);
      }
      break;
    case P_LONG: case P_ULONG: case P_FLOAT:
      {
        unsigned tmp;
        memcpy (&tmp, src + srcoff, sizeof (tmp));
        *((unsigned *) dst) = bswap4u (tmp);
      }
      break;
    case P_LONGLONG: case P_ULONGLONG: case P_DOUBLE:
      {
        unsigned long long tmp;
        memcpy (&tmp, src + srcoff, sizeof (tmp));
        *((unsigned long long *) dst) = bswap8u (tmp);
      }
      break;
    case P_UNDEFINED: case P_ADDRESS: case P_VOIDP: case P_MUTEX:
    case P_LOCK: case P_COND: case P_COUNT:
      abort ();
    case P_WCHAR:
      /* Too big a mess to worry about it now */
      abort ();
  }
  return srcoff + (unsigned) type->size;
}

static int deserprimP (C_STRUCT(c_type) const * const type, char **dst, int *dstsize, const char *src, unsigned srcoff, unsigned srcsize, int swap)
{
  srcoff = alignup (srcoff, (unsigned) type->size);
  if (srcoff + (unsigned) type->size > srcsize)
  {
    mysnprintf (dst, dstsize, "(short)");
    return ERR_INVALID_DATA;
  }
  switch (c_primitiveKind ((c_type) type))
  {
    case P_BOOLEAN:
      {
        signed char v = *((const signed char *) (src + srcoff));
        mysnprintf (dst, dstsize, "%s (%u)", v ? "true" : "false", (unsigned) v);
        break;
      }
    case P_CHAR:
      {
        signed char v = *((const signed char *) (src + srcoff));
        mysnprintf (dst, dstsize, "%d", v);
        if (isprint ((unsigned char) v))
          mysnprintf (dst, dstsize, " '%c'", (unsigned char) v);
        break;
      }
    case P_OCTET:
      {
        signed char v = *((const signed char *) (src + srcoff));
        mysnprintf (dst, dstsize, "%u", (unsigned char) v);
        if (isprint ((unsigned char) v))
          mysnprintf (dst, dstsize, " '%c'", (unsigned char) v);
        break;
      }
    case P_SHORT:
      {
        short v;
        memcpy (&v, src + srcoff, sizeof (v));
        if (swap) v = (short) bswap2u (v);
        mysnprintf (dst, dstsize, "%d", v);
        break;
      }
    case P_USHORT:
      {
        unsigned short v;
        memcpy (&v, src + srcoff, sizeof (v));
        if (swap) v = bswap2u (v);
        mysnprintf (dst, dstsize, "%u", v);
        break;
      }
    case P_LONG:
      {
        int v;
        memcpy (&v, src + srcoff, sizeof (v));
        if (swap) v = (int) bswap4u (v);
        mysnprintf (dst, dstsize, "%d", v);
        break;
      }
    case P_ULONG:
      {
        unsigned v;
        memcpy (&v, src + srcoff, sizeof (v));
        if (swap) v = bswap4u (v);
        mysnprintf (dst, dstsize, "%u", v);
        break;
      }
    case P_LONGLONG:
      {
        long long v;
        memcpy (&v, src + srcoff, sizeof (v));
        if (swap) v = (long long) bswap8u (v);
        mysnprintf (dst, dstsize, "%lld", v);
        break;
      }
    case P_ULONGLONG:
      {
        unsigned long long v;
        memcpy (&v, src + srcoff, sizeof (v));
        if (swap) v = bswap8u (v);
        mysnprintf (dst, dstsize, "%llu", v);
        break;
      }
    case P_FLOAT:
      {
        union { unsigned u; float f; } v;
        memcpy (&v, src + srcoff, sizeof (v));
        if (swap) v.u = bswap4u (v.u);
        mysnprintf (dst, dstsize, "%f", v.f);
        break;
      }
    case P_DOUBLE:
      {
        union { unsigned long long u; double f; } v;
        memcpy (&v, src + srcoff, sizeof (v));
        if (swap) v.u = bswap8u (v.u);
        mysnprintf (dst, dstsize, "%f", v.f);
        break;
      }
    case P_UNDEFINED: case P_ADDRESS: case P_VOIDP: case P_MUTEX:
    case P_LOCK: case P_COND: case P_COUNT:
      abort ();
    case P_WCHAR:
      /* Too big a mess to worry about it now */
      abort ();
  }
  return srcoff + (unsigned) type->size;
}

static int deserenum (C_STRUCT(c_type) const * const type, char *dst, const char *src, unsigned srcoff, unsigned srcsize)
{
  srcoff = alignup (srcoff, sizeof (unsigned));
  if (srcoff + (unsigned) type->size > srcsize)
    return ERR_INVALID_DATA;
  memcpy ((unsigned *) dst, src + srcoff, sizeof (unsigned));
  return srcoff + sizeof (unsigned);
}

static int deserenumS (C_STRUCT(c_type) const * const type, char *dst, const char *src, unsigned srcoff, unsigned srcsize)
{
  unsigned tmp;
  srcoff = alignup (srcoff, sizeof (unsigned));
  if (srcoff + (unsigned) type->size > srcsize)
    return ERR_INVALID_DATA;
  memcpy (&tmp, src + srcoff, sizeof (unsigned));
  *((unsigned *) dst) = bswap4u (tmp);
  return srcoff + sizeof (unsigned);
}

static int deserenumP (C_STRUCT(c_type) const * const type, char **dst, int *dstsize, const char *src, unsigned srcoff, unsigned srcsize, int swap)
{
  unsigned v;
  srcoff = alignup (srcoff, sizeof (unsigned));
  if (srcoff + (unsigned) type->size > srcsize)
  {
    mysnprintf (dst, dstsize, "(short)");
    return ERR_INVALID_DATA;
  }
  memcpy (&v, src + srcoff, sizeof (v));
  if (swap)
    v = bswap4u (v);
  mysnprintf (dst, dstsize, "%u", v); /* can't be bother to look up label */
  return srcoff + sizeof (unsigned);
}

static int deserlengthG_unaligned (unsigned *n, const char *src, unsigned *srcoff, unsigned srcsize, int swap)
{
  unsigned tmp;
  if (*srcoff + sizeof (unsigned) > srcsize)
    return 0;
  memcpy (&tmp, src + *srcoff, sizeof (unsigned));
  *n = swap ? bswap4u (tmp) : tmp;
  *srcoff += sizeof (unsigned);
  assert (*srcoff <= srcsize);
  return 1;
}

static int deserlengthG (unsigned *n, const char *src, unsigned *srcoff, unsigned srcsize, int swap)
{
  *srcoff = alignup (*srcoff, sizeof (unsigned));
  return deserlengthG_unaligned (n, src, srcoff, srcsize, swap);
}

static int deserprimarray (C_STRUCT(c_type) const * const subtype, char *array, const char *src, unsigned srcoff, unsigned srcsize, unsigned n)
{
  const unsigned size1 = (unsigned) subtype->size;
  if (n == 0) /* no array elements => no need for alignment */
    return srcoff;
  srcoff = alignup (srcoff, size1);
  /* if n was read from src, we already know size1 * n <= srcsize -
     srcoff before alignment (cf desercollG()); if not, the OpenSplice
     kernel accepted the type ... */
  if (srcoff > srcsize || size1 * n > srcsize - srcoff)
    return ERR_INVALID_DATA;
  memcpy (array, src + srcoff, n * size1);
  srcoff += n * size1;
  return srcoff;
}

static int deserprimarrayS (C_STRUCT(c_type) const * const subtype, char *array, const char *src, unsigned srcoff, unsigned srcsize, unsigned n)
{
  const unsigned size1 = (unsigned) subtype->size;
  unsigned i;
  if (n == 0)
    return srcoff;
  srcoff = alignup (srcoff, size1);
  if (srcoff > srcsize || size1 * n > srcsize - srcoff)
    return ERR_INVALID_DATA;
  switch (size1)
  {
    case 1:
      if (n > srcsize - srcoff)
        return ERR_INVALID_DATA;
      memcpy (array, src + srcoff, n);
      srcoff += n;
      break;
    case 2:
      {
        unsigned short *d = (unsigned short *) array;
        const unsigned short *s = (const unsigned short *) (src + srcoff);
        for (i = 0; i < n; i++)
        {
          unsigned short tmp;
          /* Hopefully the compiler knows memcpy intimately and is
             smart enough to figure out that it can be omitted on an
             x86 (or any other processor that can natively read
             unaligned data) */
          memcpy (&tmp, &s[i], sizeof (tmp));
          d[i] = bswap2u (tmp);
        }
        srcoff += 2 * n;
      }
      break;
    case 4:
      {
        unsigned *d = (unsigned *) array;
        const unsigned *s = (const unsigned *) (src + srcoff);
        for (i = 0; i < n; i++)
        {
          unsigned tmp;
          memcpy (&tmp, &s[i], sizeof (tmp));
          d[i] = bswap4u (tmp);
        }
        srcoff += 4 * n;
      }
      break;
    case 8:
      {
        unsigned long long *d = (unsigned long long *) array;
        const unsigned long long *s = (const unsigned long long *) (src + srcoff);
        for (i = 0; i < n; i++)
        {
          unsigned long long tmp;
          memcpy (&tmp, &s[i], sizeof (tmp));
          d[i] = bswap8u (tmp);
        }
        srcoff += 8 * n;
      }
      break;
    default:
      assert (0);
  }
  return srcoff;
}

static int deserstringG_unaligned (C_STRUCT(c_collectionType) const * const ctype, char *dst, const char *src, unsigned srcoff, unsigned srcsize, int swap)
{
  unsigned n;
  c_string str;
  if (!deserlengthG_unaligned (&n, src, &srcoff, srcsize, swap))
    return ERR_INVALID_DATA;
  else if (n == 0)
  {
    /* Illegal CDR, abused to support NULL pointers in string fields
       that get used in internal topics; ought to accept these only if
       the source is OpenSplice */
    *((c_string *) dst) = NULL;
    return srcoff;
  }
  else if (n > srcsize - srcoff)
    return ERR_INVALID_DATA;
  else if (ctype->maxSize > 0 && n-1 > (unsigned) ctype->maxSize)
    return ERR_INVALID_DATA;
  else if (src[srcoff + n-1] != '\0')
    /* don't care so much if there are NULs embedded in the
       input, but it *must* end with one */
    return ERR_INVALID_DATA;
  if ((str = c_stringMalloc (c_getBase ((c_type) ctype), n)) == NULL)
    return ERR_OUT_OF_MEMORY;
  memcpy (str, src + srcoff, n);
  srcoff += n;
  *((c_string *) dst) = str;
  return srcoff;
}

static int deserstringP_unaligned (C_STRUCT(c_collectionType) const * const ctype, char **dst, int *dstsize, const char *src, unsigned srcoff, unsigned srcsize, int swap)
{
  unsigned i, n;
  if (!deserlengthG_unaligned (&n, src, &srcoff, srcsize, swap))
  {
    mysnprintf (dst, dstsize, "(short)");
    return ERR_INVALID_DATA;
  }
  else if (n == 0) /* see deserstringG_unaligned */
  {
    mysnprintf (dst, dstsize, "(null)");
    return srcoff;
  }
  else if (n > srcsize - srcoff)
  {
    mysnprintf (dst, dstsize, "(string length %u out of bounds wrt serdata)", n-1);
    return ERR_INVALID_DATA;
  }
  else if (ctype->maxSize > 0 && n-1 > (unsigned) ctype->maxSize)
  {
    mysnprintf (dst, dstsize, "(string length %u out of bounds wrt type)", n-1);
    return ERR_INVALID_DATA;
  }
  else if (src[srcoff + n-1] != '\0')
  {
    /* don't care so much if there are NULs embedded in the
       input, but it *must* end with one */
    mysnprintf (dst, dstsize, "(string not terminated)");
    return ERR_INVALID_DATA;
  }
  mysnprintf (dst, dstsize, "\"");
  for (i = 0; i < n-1; i++)
  {
    unsigned char v = (src + srcoff)[i];
    if (isprint (v))
      mysnprintf (dst, dstsize, "%c", v);
    else
      mysnprintf (dst, dstsize, "\\x%02x", v);
  }
  mysnprintf (dst, dstsize, "\"");
  srcoff += n;
  return srcoff;
}

static int desercollG (C_STRUCT(c_type) const * const type, char *dst, const char *src, unsigned srcoff, unsigned srcsize, int swap)
{
  C_STRUCT(c_collectionType) const * const ctype = c_collectionType ((c_type) type);
  switch (ctype->kind)
  {
    case C_STRING:
      {
        int off1;
        srcoff = alignup (srcoff, sizeof (unsigned));
        if ((off1 = deserstringG_unaligned (ctype, dst, src, srcoff, srcsize, swap)) < 0)
          return ERR_INVALID_DATA;
        srcoff = off1;
      }
      break;

    case C_ARRAY:
    case C_SEQUENCE:
      {
        /* Array: maybe reftype (reftype if vla <=> maxSize == 0, I
           think); Sequence: always reftype. (Do I really need to
           bother? Existing DDSI service does, so maybe I do. This is
           *their* logic, not mine!) */
        C_STRUCT(c_type) const * const subtype = ctype->subType;
        const c_metaKind subtypekind = c_baseObjectKind (c_baseObject (subtype));
        const unsigned size1 = (c_typeIsRef ((c_type) subtype)) ? (unsigned) sizeof (void *) : (unsigned) subtype->size;
        void *array;
        unsigned n;
        if (ctype->kind == C_ARRAY && ctype->maxSize > 0)
        {
          n = ctype->maxSize;
          assert (n > 0);
        }
        else /* C_SEQUENCE or C_ARRAY with ctype->maxSize == 0 (cf sercoll()) */
        {
          if (!deserlengthG (&n, src, &srcoff, srcsize, swap))
            return ERR_INVALID_DATA;
          /* c_arrayNew takes a c_long (which is signed), and frowns
             upon negative lengths; and bailing out because the
             deserialized sequence length exceeds the bytes remaining
             in the serialized form prevents attempts at tricking the
             code into needlessly allocating huge amounts of memory
             with the potential for denial-of-service */
          if ((c_long) n < 0 || GUARANTEED_INSUFFICIENT_BYTES_LEFT (subtype, n, srcsize - srcoff))
            return ERR_INVALID_DATA;
        }
        if (ctype->kind == C_ARRAY && ctype->maxSize > 0)
        {
          if (n > (unsigned) ctype->maxSize)
            return ERR_INVALID_DATA;
          array = dst;
        }
        else if (*(void **) dst)
        {
          /* ???Q?Q?Q?Q??? who might've done that? or it all behaves
             differently from what I expect ... */
          abort ();
          array = *((void **) dst);
        }
        else
        {
          array = c_arrayNew ((c_type) subtype, (c_long) n);
          if (n != 0 && array == NULL)
            /* length-0 array yields a null pointer, any other null
               pointer is an out-of-memory condition */
            return ERR_OUT_OF_MEMORY;
          *((void **) dst) = array;
        }
        /* Load array contents; we know 'array' is large enough, but
           we don't know the source is. For primitive types, check
           before starting to copy, and rely on the deserializing of
           each single element to fail when it runs out of data for
           all other types. */
        if (subtypekind == M_PRIMITIVE || subtypekind == M_ENUMERATION)
        {
          int off1;
          if (swap)
            off1 = deserprimarrayS (subtype, array, src, srcoff, srcsize, n);
          else
            off1 = deserprimarray (subtype, array, src, srcoff, srcsize, n);
          if (off1 < 0)
            return off1;
          srcoff = off1;
        }
        else
        {
          unsigned i;
          char *dst1 = array;
          /* For bad compilers and processors with static branch
             prediction, maybe the following loop ought to be
             duplicated, one for the swapped & one for the non-swapped
             case */
          for (i = 0; i < n; i++)
          {
            int off1;
            if (swap)
              off1 = deserialize1S (subtype, dst1, src, srcoff, srcsize);
            else
              off1 = deserialize1 (subtype, dst1, src, srcoff, srcsize);
            if (off1 < 0)
              return off1;
            srcoff = off1;
            dst1 += size1;
          }
        }
      }
      break;

    default:
      abort ();
  }
  return srcoff;
}

static unsigned isprint_runlen (const unsigned char *s, unsigned n)
{
  unsigned m;
  for (m = 0; m < n && isprint (s[m]); m++)
    ;
  return m;
}

static int desercollP (C_STRUCT(c_type) const * const type, char **dst, int *dstsize, const char *src, unsigned srcoff, unsigned srcsize, int swap)
{
  C_STRUCT(c_collectionType) const * const ctype = c_collectionType ((c_type) type);
  switch (ctype->kind)
  {
    case C_STRING:
      {
        int off1;
        srcoff = alignup (srcoff, sizeof (unsigned));
        if ((off1 = deserstringP_unaligned (ctype, dst, dstsize, src, srcoff, srcsize, swap)) < 0)
          return off1;
        srcoff = off1;
      }
      break;

    case C_ARRAY:
    case C_SEQUENCE:
      {
        /* Array: maybe reftype (reftype if vla <=> maxSize == 0, I
           think); Sequence: always reftype. (Do I really need to
           bother? Existing DDSI service does, so maybe I do. This is
           *their* logic, not mine!) */
        C_STRUCT(c_type) const * const subtype = ctype->subType;
        const unsigned size1 = (c_typeIsRef ((c_type) subtype)) ? (unsigned) sizeof (void *) : (unsigned) subtype->size;
        unsigned n;
        mysnprintf (dst, dstsize, "{");
        if (ctype->kind == C_ARRAY && ctype->maxSize > 0)
        {
          n = ctype->maxSize;
          assert (n > 0);
        }
        else /* C_SEQUENCE or C_ARRAY with ctype->maxSize == 0 (cf sercoll()) */
        {
          if (!deserlengthG (&n, src, &srcoff, srcsize, swap))
          {
            mysnprintf (dst, dstsize, "(short)");
            return ERR_INVALID_DATA;
          }
          /* c_arrayNew takes a c_long (which is signed), and frowns
             upon negative lengths; and bailing out because the
             deserialized sequence length exceeds the bytes remaining
             in the serialized form prevents attempts at tricking the
             code into needlessly allocating huge amounts of memory
             with the potential for denial-of-service */
          if ((c_long) n < 0 || GUARANTEED_INSUFFICIENT_BYTES_LEFT (size1, n, srcsize - srcoff))
          {
            mysnprintf (dst, dstsize, "(coll length %u out of bounds wrt serdata)", n);
            return ERR_INVALID_DATA;
          }
        }
        if (ctype->maxSize > 0)
        {
          if (n > (unsigned) ctype->maxSize)
          {
            mysnprintf (dst, dstsize, "(coll length %u out of bounds wrt type)", n);
            return ERR_INVALID_DATA;
          }
        }
        /* not special-casing primitive types, except for an array/sequence of bytes */
        if (c_baseObjectKind ((c_type) subtype) == M_PRIMITIVE &&
            (c_primitiveKind ((c_type) subtype) == P_CHAR ||
             c_primitiveKind ((c_type) subtype) == P_OCTET))
        {
          unsigned i = 0, j;
          while (i < n)
          {
            unsigned m = isprint_runlen ((unsigned char *) (src + srcoff), n);
            if (m >= 4)
            {
              mysnprintf (dst, dstsize, "%s\"", i != 0 ? "," : "");
              for (j = 0; j < m; j++)
                mysnprintf (dst, dstsize, "%c", src[srcoff + j]);
              mysnprintf (dst, dstsize, "\"");
              srcoff += m;
              i += m;
            }
            else
            {
              int off1;
              if (i != 0)
                mysnprintf (dst, dstsize, ",");
              off1 = deserialize1P (subtype, dst, dstsize, src, srcoff, srcsize, swap);
              if (off1 < 0)
                return off1;
              srcoff = off1;
              i++;
            }
          }
        }
        else
        {
          unsigned i;
          for (i = 0; i < n; i++)
          {
            int off1;
            if (i != 0)
              mysnprintf (dst, dstsize, ",");
            off1 = deserialize1P (subtype, dst, dstsize, src, srcoff, srcsize, swap);
            if (off1 < 0)
              return off1;
            srcoff = off1;
          }
        }
        mysnprintf (dst, dstsize, "}");
      }
      break;

    default:
      abort ();
  }
  return srcoff;
}

static int desercoll (C_STRUCT(c_type) const * const type, char *dst, const char *src, unsigned srcoff, unsigned srcsize)
{
  return desercollG (type, dst, src, srcoff, srcsize, 0);
}

static int desercollS (C_STRUCT(c_type) const * const type, char *dst, const char *src, unsigned srcoff, unsigned srcsize)
{
  return desercollG (type, dst, src, srcoff, srcsize, 1);
}

static int deserstruct (C_STRUCT(c_type) const * const type, char *dst, const char *src, unsigned srcoff, unsigned srcsize)
{
  C_STRUCT(c_structure) const * const structure = c_structure ((c_type) type);
  const int n = (int) c_arraySize (structure->members);
  int i;
  for (i = 0; i < n; i++)
  {
    C_STRUCT(c_member) const * const member = structure->members[i];
    C_STRUCT(c_type) const * const subtype = c_typeActualType (c_specifierType (member));
    void *dst1 = C_DISPLACE (dst, (c_address) member->offset);
    int off1;
    if ((off1 = deserialize1 (subtype, dst1, src, srcoff, srcsize)) < 0)
      return off1;
    srcoff = off1;
  }
  return srcoff;
}

static int deserstructS (C_STRUCT(c_type) const * const type, char *dst, const char *src, unsigned srcoff, unsigned srcsize)
{
  C_STRUCT(c_structure) const * const structure = c_structure ((c_type) type);
  const int n = (int) c_arraySize (structure->members);
  int i;
  for (i = 0; i < n; i++)
  {
    C_STRUCT(c_member) const * const member = structure->members[i];
    C_STRUCT(c_type) const * const subtype = c_typeActualType (c_specifierType (member));
    void *dst1 = C_DISPLACE (dst, (c_address) member->offset);
    int off1;
    if ((off1 = deserialize1S (subtype, dst1, src, srcoff, srcsize)) < 0)
      return off1;
    srcoff = off1;
  }
  return srcoff;
}

static int deserstructP (C_STRUCT(c_type) const * const type, char **dst, int *dstsize, const char *src, unsigned srcoff, unsigned srcsize, int swap)
{
  C_STRUCT(c_structure) const * const structure = c_structure ((c_type) type);
  const int n = (int) c_arraySize (structure->members);
  int i;
  mysnprintf (dst, dstsize, "{");
  for (i = 0; i < n; i++)
  {
    C_STRUCT(c_member) const * const member = structure->members[i];
    C_STRUCT(c_type) const * const subtype = c_typeActualType (c_specifierType (member));
    int off1;
    if (i != 0)
      mysnprintf (dst, dstsize, ",");
    if ((off1 = deserialize1P (subtype, dst, dstsize, src, srcoff, srcsize, swap)) < 0)
      return off1;
    srcoff = off1;
  }
  mysnprintf (dst, dstsize, "}");
  return srcoff;
}

static int deserunion (C_STRUCT(c_type) const * const type, char *dst, const char *src, unsigned srcoff, unsigned srcsize)
{
  C_STRUCT(c_union) const * const utype = c_union ((c_type) type);
  C_STRUCT(c_type) const * const dtype = c_typeActualType (utype->switchType);
  c_value dvalue;
  C_STRUCT(c_unionCase) const *activecase;
  int off1;
  if ((off1 = deserialize1 (dtype, dst, src, srcoff, srcsize)) < 0)
    return off1;
  srcoff = off1;
  dvalue = get_discriminant_value (dtype, dst);
  activecase = active_union_case (utype, dvalue);
  if (activecase)
  {
    const unsigned disp = alignup ((unsigned) dtype->size, (unsigned) c_type (utype)->alignment);
    C_STRUCT(c_type) const * const subtype = c_typeActualType (c_specifierType (activecase));
    if ((off1 = deserialize1 (subtype, dst + disp, src, srcoff, srcsize)) < 0)
      return off1;
    srcoff = off1;
  }
  return srcoff;
}

static int deserunionS (C_STRUCT(c_type) const * const type, char *dst, const char *src, unsigned srcoff, unsigned srcsize)
{
  C_STRUCT(c_union) const * const utype = c_union ((c_type) type);
  C_STRUCT(c_type) const * const dtype = c_typeActualType (utype->switchType);
  c_value dvalue;
  C_STRUCT(c_unionCase) const *activecase;
  int off1;
  if ((off1 = deserialize1S (dtype, dst, src, srcoff, srcsize)) < 0)
    return off1;
  srcoff = off1;
  dvalue = get_discriminant_value (dtype, dst);
  activecase = active_union_case (utype, dvalue);
  if (activecase)
  {
    const unsigned disp = alignup ((unsigned) dtype->size, (unsigned) c_type (utype)->alignment);
    C_STRUCT(c_type) const * const subtype = c_typeActualType (c_specifierType (activecase));
    if ((off1 = deserialize1S (subtype, dst + disp, src, srcoff, srcsize)) < 0)
      return off1;
    srcoff = off1;
  }
  return srcoff;
}

static int deserunionP (C_STRUCT(c_type) const * const type, char **dst, int *dstsize, const char *src, unsigned srcoff, unsigned srcsize, int swap)
{
  C_STRUCT(c_union) const * const utype = c_union ((c_type) type);
  C_STRUCT(c_type) const * const dtype = c_typeActualType (utype->switchType);
  char tmp[sizeof (c_value)];
  c_value dvalue;
  C_STRUCT(c_unionCase) const *activecase;
  int off1;
  if ((off1 = deserialize1P (dtype, dst, dstsize, src, srcoff, srcsize, swap)) < 0)
    return off1;
  assert (dtype->size <= sizeof (tmp));
  if (swap)
    deserialize1S (dtype, tmp, src, srcoff, srcsize);
  else
    deserialize1 (dtype, tmp, src, srcoff, srcsize);
  srcoff = off1;
  mysnprintf (dst, dstsize, ":");
  dvalue = get_discriminant_value (dtype, tmp);
  activecase = active_union_case (utype, dvalue);
  if (activecase)
  {
    C_STRUCT(c_type) const * const subtype = c_typeActualType (c_specifierType (activecase));
    if ((off1 = deserialize1P (subtype, dst, dstsize, src, srcoff, srcsize, swap)) < 0)
      return off1;
    srcoff = off1;
  }
  return srcoff;
}

static int deserialize1 (C_STRUCT(c_type) const * const type_x, char *dst, const char *src, unsigned srcoff, unsigned srcsize)
{
  c_type type = c_typeActualType ((c_type) type_x);
  switch (c_baseObjectKind (type))
  {
    case M_PRIMITIVE:
      return deserprim (type, dst, src, srcoff, srcsize);
    case M_ENUMERATION:
      return deserenum (type, dst, src, srcoff, srcsize);
    case M_COLLECTION:
      return desercoll (type, dst, src, srcoff, srcsize);
    case M_STRUCTURE:
      return deserstruct (type, dst, src, srcoff, srcsize);
    case M_UNION:
      return deserunion (type, dst, src, srcoff, srcsize);
    default:
      /* Do any others need to be supported? */
      abort ();
      return ERR_UNSPECIFIED;
  }
}

static int deserialize1S (C_STRUCT(c_type) const * const type_x, char *dst, const char *src, unsigned srcoff, unsigned srcsize)
{
  c_type type = c_typeActualType ((c_type) type_x);
  switch (c_baseObjectKind (type))
  {
    case M_PRIMITIVE:
      return deserprimS (type, dst, src, srcoff, srcsize);
    case M_ENUMERATION:
      return deserenumS (type, dst, src, srcoff, srcsize);
    case M_COLLECTION:
      return desercollS (type, dst, src, srcoff, srcsize);
    case M_STRUCTURE:
      return deserstructS (type, dst, src, srcoff, srcsize);
    case M_UNION:
      return deserunionS (type, dst, src, srcoff, srcsize);
    default:
      /* Do any others need to be supported? */
      abort ();
      return ERR_UNSPECIFIED;
  }
}

static int deserialize1P (C_STRUCT(c_type) const * const type_x, char **dst, int *dstsize, const char *src, unsigned srcoff, unsigned srcsize, int swap)
{
  c_type type = c_typeActualType ((c_type) type_x);
  switch (c_baseObjectKind (type))
  {
    case M_PRIMITIVE:
      return deserprimP (type, dst, dstsize, src, srcoff, srcsize, swap);
    case M_ENUMERATION:
      return deserenumP (type, dst, dstsize, src, srcoff, srcsize, swap);
    case M_COLLECTION:
      return desercollP (type, dst, dstsize, src, srcoff, srcsize, swap);
    case M_STRUCTURE:
      return deserstructP (type, dst, dstsize, src, srcoff, srcsize, swap);
    case M_UNION:
      return deserunionP (type, dst, dstsize, src, srcoff, srcsize, swap);
    default:
      /* Do any others need to be supported? */
      abort ();
      return ERR_UNSPECIFIED;
  }
}

typedef int (*deserialize1_t) (C_STRUCT(c_type) const * const, char *, const char *, unsigned, unsigned);

static int deserialize_prep (v_message *msg, char **dst, deserialize1_t *df, char const **src, unsigned *srcsize, const struct topic *topic, const void *vsrc, int vsrcsize)
{
  C_STRUCT(v_topic) const * const ospl_topic = topic->ospl_topic;
  const struct CDRHeader *hdr = vsrc;
  int swap;
  *msg = NULL;
#ifndef NDEBUG
  {
    C_STRUCT(c_type) const * const type = topic->type;
    assert (type == v_topicDataType ((v_topic) ospl_topic));
  }
#endif
  if (vsrcsize < (int) sizeof (*hdr))
    return 0;
  switch (hdr->identifier)
  {
    case CDR_BE:
      swap = PLATFORM_IS_LITTLE_ENDIAN;
      break;
    case CDR_LE:
      swap = ! PLATFORM_IS_LITTLE_ENDIAN;
      break;
    default:
      return 0;
  }
  if (hdr->options != 0)
  {
    /* Unless I know they're harmless or support them, can't allow
       them. */
    return 0;
  }
  if ((*msg = v_topicMessageNew ((v_topic) ospl_topic)) == NULL)
    return 0;
  (*msg)->qos = NULL;
  *dst = (char *) (*msg) + v_topicDataOffset ((v_topic) ospl_topic);
  *df = swap ? deserialize1S : deserialize1;
  assert (vsrcsize >= (int) sizeof (struct CDRHeader));
  *src = (const char *) vsrc + sizeof (struct CDRHeader);
  *srcsize = (unsigned) vsrcsize - sizeof (struct CDRHeader);
  return 1;
}

v_message deserialize (const struct topic *topic, const void *vsrc, int vsrcsize)
{
  C_STRUCT(c_type) const * const type = topic->type;
  deserialize1_t df;
  v_message msg;
  char *dst;
  const char *src;
  unsigned srcsize;
  if (!deserialize_prep (&msg, &dst, &df, &src, &srcsize, topic, vsrc, vsrcsize))
    goto fail;
  /* Would prefer df to return srcsize, but there may be up to three
     trailing padding bytes due to alignment restrictions. Being
     sloppy and only checking whether the deserialized grokked the
     bytes up to wherever, and ignoring any trailing stuff. */
  if (df (type, dst, src, 0, srcsize) < 0)
  {
    /* possibly allow superfluous bytes at the end? */
    goto fail;
  }
  return msg;
 fail:
  if (msg) c_free (msg);
  return NULL;
}

v_message deserialize_from_key (const struct topic *topic, const void *vsrc, int vsrcsize)
{
  /* v_topic has a messageKeyList which is an array of 'c_field's,
     each of which contains an offset that presumably is an offset
     into the data part of a message. If not, we can always roll our
     own ... */
  deserialize1_t df;
  v_message msg;
  char *dst;
  const char *src;
  unsigned srcsize;
  int i, off;
  if (!deserialize_prep (&msg, &dst, &df, &src, &srcsize, topic, vsrc, vsrcsize))
    goto fail;
  assert (c_arraySize (topic->ospl_topic->messageKeyList) == topic->nkeys);
  off = 0;
  for (i = 0; i < topic->nkeys; i++)
  {
    const struct keyinfo *ki = &topic->keys[topic->keys[i].specord_idx];
    if ((off = df (ki->type, dst + ki->off, src, off, srcsize)) < 0)
      goto fail;
  }
#if 0 /* may end before srcsize */
  if (off != (int) srcsize)
    goto fail;
#endif
  return msg;
 fail:
  if (msg) c_free (msg);
  return NULL;
}

v_message deserialize_from_keyhash (const struct topic *topic, const void *vsrc, int srcsize)
{
  C_STRUCT(v_topic) const * const ospl_topic = topic->ospl_topic;
  const char *src = vsrc;
  v_message msg;
  char *dst;
  int i;
  /* If keys don't fit in 16 bytes (strings-by-ref handled lazily) the
     keyhash can't be decoded into a sample */
  if (topic->keysersize > 16)
    return NULL;
  if (srcsize != 16)
    return NULL;
#ifndef NDEBUG
  {
    C_STRUCT(c_type) const * const type = topic->type;
    assert (type == v_topicDataType ((v_topic) ospl_topic));
  }
#endif
  if ((msg = v_topicMessageNew ((v_topic) ospl_topic)) == NULL)
    return NULL;
  msg->qos = NULL;
  dst = (char *) msg + v_topicDataOffset ((v_topic) ospl_topic);
  for (i = 0; i < topic->nkeys; i++)
  {
    const struct keyinfo *ki = &topic->keys[i]; /* order is irrelevant */
    switch (ki->tc)
    {
      case TC_ONEBYTE:
        *(dst + ki->off) = *(src + ki->keyseroff);
        break;
      case TC_TWOBYTES:
        {
          unsigned short tmp;
          memcpy (&tmp, src + ki->keyseroff, sizeof (tmp));
          *((unsigned short *) (dst + ki->off)) = fromBE2u (tmp);
        }
        break;
      case TC_FOURBYTES:
        {
          unsigned tmp;
          memcpy (&tmp, src + ki->keyseroff, sizeof (tmp));
          *((unsigned *) (dst + ki->off)) = fromBE4u (tmp);
        }
        break;
      case TC_EIGHTBYTES:
        {
          unsigned long long tmp;
          memcpy (&tmp, src + ki->keyseroff, sizeof (tmp));
          *((unsigned long long *) (dst + ki->off)) = fromBE8u (tmp);
        }
        break;
      case TC_STRINGREF:
        /* Oops! keyhash can't faithfully represent the key! How
           naughty a sender (I can't imagine I can do any harm, so
           late detection is good enough) */
        goto fail;
      case TC_STRINGINLINE:
        {
          C_STRUCT(c_collectionType) const * const ctype = c_collectionType (ki->type);
          assert (ctype->kind == C_STRING);
          if (deserstringG_unaligned (ctype, dst + ki->off, src, ki->keyseroff, srcsize, PLATFORM_IS_LITTLE_ENDIAN) < 0)
            goto fail;
        }
        break;
    }
  }
  return msg;
 fail:
  if (msg) c_free (msg);
  return NULL;
}

static int prettyprint_prep (char **dst, int *dstsize, int *swap, const char **src, int *srcsize, const void *vsrc, int vsrcsize)
{
  const struct CDRHeader *hdr = vsrc;
  if (vsrcsize < (int) sizeof (*hdr))
  {
    mysnprintf (dst, dstsize, "(short)");
    goto fail;
  }
  switch (hdr->identifier)
  {
    case CDR_BE:
      *swap = PLATFORM_IS_LITTLE_ENDIAN;
      break;
    case CDR_LE:
      *swap = ! PLATFORM_IS_LITTLE_ENDIAN;
      break;
    default:
      mysnprintf (dst, dstsize, "(unknown encoding)");
      *swap = 0;
      goto fail;
  }
  if (hdr->options != 0)
  {
    /* Serdata is from my serializer, which doesn't do options */
    mysnprintf (dst, dstsize, "(no options supported)");
    goto fail;
  }
  assert (vsrcsize >= (int) sizeof (struct CDRHeader));
  *src = (const char *) (hdr + 1);
  *srcsize = (unsigned) vsrcsize - sizeof (struct CDRHeader);
  return 1;
 fail:
  return 0;
}

int prettyprint_raw (char *dst, const int dstsize, const struct topic *topic, const void *vsrc, int vsrcsize)
{
  int dstsize1 = dstsize;
  const char *src;
  int swap = 0;
  int srcsize;
  if (!prettyprint_prep (&dst, &dstsize1, &swap, &src, &srcsize, vsrc, vsrcsize))
    goto fail;
  if (deserialize1P (topic->type, &dst, &dstsize1, src, 0, srcsize, swap) < 0)
  {
    mysnprintf (&dst, &dstsize1, "(fail)");
    goto fail;
  }
  return dstsize - dstsize1;
 fail:
  return -(dstsize - dstsize1);
}

static int prettyprint_key (char *dst, const int dstsize, const struct topic *topic, const void *vsrc, int vsrcsize)
{
  int dstsize1 = dstsize;
  const char *src;
  int swap = 0;
  int srcsize;
  int i, off;
  if (!prettyprint_prep (&dst, &dstsize1, &swap, &src, &srcsize, vsrc, vsrcsize))
    goto fail;
  assert (c_arraySize (topic->ospl_topic->messageKeyList) == topic->nkeys);
  mysnprintf (&dst, &dstsize1, "k:{");
  off = 0;
  for (i = 0; i < topic->nkeys; i++)
  {
    const struct keyinfo *ki = &topic->keys[topic->keys[i].specord_idx];
    if (i > 0)
      mysnprintf (&dst, &dstsize1, ",");
    if ((off = deserialize1P (ki->type, &dst, &dstsize1, src, off, srcsize, swap)) < 0)
    {
      mysnprintf (&dst, &dstsize1, "(fail)");
      goto fail;
    }
  }
  mysnprintf (&dst, &dstsize1, "}");
  return dstsize - dstsize1;
 fail:
  return -(dstsize - dstsize1);
}

int prettyprint_serdata (char *dst, const int dstsize, const struct serdata *serdata)
{
  if (serdata->u.st->topic == NULL)
  {
    int dstsize1 = dstsize;
    mysnprintf (&dst, &dstsize1, "%s(blob)", serdata->u.st->justkey ? "k:" : "");
    return dstsize - dstsize1;
  }
  else if (!serdata->u.st->justkey)
  {
    return prettyprint_raw (dst, dstsize, serdata->u.st->topic, &serdata->hdr, serdata_size (serdata));
  }
  else
  {
    return prettyprint_key (dst, dstsize, serdata->u.st->topic, &serdata->hdr, serdata_size (serdata));
  }
}

/*************************************/

static int verdata (C_STRUCT(c_type) const * const type_x, const char *a, const char *b);

static int verprim (C_STRUCT(c_type) const * const type, const char *a, const char *b)
{
  return memcmp (a, b, type->size) == 0;
}

static int verenum (C_STRUCT(c_type) const * const type, const char *a, const char *b)
{
  return verprim (type, a, b);
}

static int vercoll (C_STRUCT(c_type) const * const type, const char *a, const char *b)
{
  C_STRUCT(c_collectionType) const * const ctype = c_collectionType ((c_type) type);
  switch (ctype->kind)
  {
    case C_STRING:
      {
        const char *a1 = (const char *) (*((char **) a));
        const char *b1 = (const char *) (*((char **) b));
        if (a1 == NULL)
          return b1 == NULL;
        else if (b1 == NULL)
          return 0;
        else
          return strcmp (a1, b1) == 0;
      }

    case C_ARRAY:
    case C_SEQUENCE:
      {
        /* Array: maybe reftype (reftype if vla <=> maxSize == 0, I
           think); Sequence: always reftype. (Do I really need to
           bother? Existing DDSI service does, so maybe I do. This is
           *their* logic, not mine!) */
        C_STRUCT(c_type) const * const subtype = ctype->subType;
        const char *a1;
        const char *b1;
        unsigned length_a, length_b;
        unsigned datasize1 = (c_typeIsRef ((c_type) subtype)) ? (unsigned) sizeof (void *) : (unsigned) subtype->size;
        unsigned i;
        if (ctype->kind == C_ARRAY && ctype->maxSize > 0) {
          a1 = a;
          b1 = b;
          length_a = length_b = ctype->maxSize;
        } else {
          a1 = *((char const * const *) a);
          b1 = *((char const * const *) b);
          length_a = c_arraySize ((void *) a1);
          length_b = c_arraySize ((void *) b1);
          if ((length_a != 0) && (a1 == NULL))
            return 0;
          if ((length_b != 0) && (b1 == NULL))
            return 0;
        }
        if (length_a != length_b)
          return 0;
        for (i = 0; i < length_a; i++)
        {
          if (!verdata (subtype, a1, b1))
            return 0;
          a1 += datasize1;
          b1 += datasize1;
        }
        return 1;
      }

    default:
      abort ();
  }
  return 0;
}

static int verstruct (C_STRUCT(c_type) const * const type, const char *a, const char *b)
{
  C_STRUCT(c_structure) const * const structure = c_structure ((c_type) type);
  unsigned i, n = c_arraySize (structure->members);
  for (i = 0; i < n; i++)
  {
    C_STRUCT(c_member) const * const member = structure->members[i];
    const unsigned disp = (unsigned) member->offset;
    C_STRUCT(c_type) const * const subtype = c_typeActualType (c_specifierType (member));
    if (!verdata (subtype, a + disp, b + disp))
      return 0;
  }
  return 1;
}

static int verunion (C_STRUCT(c_type) const * const type, const char *a, const char *b)
{
  C_STRUCT(c_union) const * const utype = c_union ((c_type) type);
  C_STRUCT(c_type) const * const dtype = c_typeActualType (utype->switchType);
  const c_value dvalue_a = get_discriminant_value (dtype, a);
  C_STRUCT(c_unionCase) const * const activecase_a = active_union_case (utype, dvalue_a);
#ifndef NDEBUG
  const c_value dvalue_b = get_discriminant_value (dtype, b);
  C_STRUCT(c_unionCase) const * const activecase_b = active_union_case (utype, dvalue_b);
#endif
  if (!verdata (dtype, a, b))
    return 0;
  assert (activecase_a == activecase_b);
  if (activecase_a)
  {
    const unsigned disp = (unsigned) alignup ((unsigned) dtype->size, (unsigned) c_type (utype)->alignment);
    C_STRUCT(c_type) const * const subtype = c_typeActualType (c_specifierType (activecase_a));
    if (!verdata (subtype, a + disp, b + disp))
      return 0;
  }
  return 1;
}

static int verdata (C_STRUCT(c_type) const * const type_x, const char *a, const char *b)
{
  c_type type = c_typeActualType ((c_type) type_x);
  switch (c_baseObjectKind (type))
  {
    case M_PRIMITIVE:
      return verprim (type, a, b);
    case M_ENUMERATION:
      return verenum (type, a, b);
    case M_COLLECTION:
      return vercoll (type, a, b);
    case M_STRUCTURE:
      return verstruct (type, a, b);
    case M_UNION:
      return verunion (type, a, b);
    default:
      /* Do any others need to be supported? */
      abort ();
      return 0;
  }
}

int serdata_verify (serdata_t serdata, C_STRUCT (v_message) const *srcmsg)
{
  int res = 0;
  const void *a;
  const void *b;
  v_message msg = deserialize (serdata->u.st->topic, &serdata->hdr, serdata_size (serdata));
  if (msg == NULL)
    goto fail;
  a = C_DISPLACE (srcmsg, serdata->u.st->topic->ospl_topic->dataField->offset);
  b = C_DISPLACE (msg, serdata->u.st->topic->ospl_topic->dataField->offset);
  res = verdata (serdata->u.st->topic->type, a, b);
 fail:
  c_free (msg);
  return res;
}

/* SHA1 not available (unoffical build.) */

/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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
#include "os_atomics.h"

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
#include "q_murmurhash3.h"

#include "sd_cdr.h"

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

static int serialize1 (serstate_t st, C_STRUCT(c_type) const * const type, const char *data, unsigned off);
static int deserialize1 (C_STRUCT(c_type) const * const type, char *dst, const char *src, size_t srcoff, size_t srcsize);
static int deserialize1S (C_STRUCT(c_type) const * const type, char *dst, const char *src, size_t srcoff, size_t srcsize);
static int deserialize1P (C_STRUCT(c_type) const * const type_x, char **dst, int *dstsize, const char *src, size_t srcoff, size_t srcsize, int swap);

#if ! USE_PRIVATE_SERIALIZER
static int cdr_serdata_init (void *vst, char **dst, os_uint32 size_hint);
static int cdr_serdata_grow (void *vst, char **dst, os_uint32 size_hint);
static void cdr_serdata_finalize (void *vst, char *dst);
static os_uint32 cdr_serdata_getpos (const void *vst, const char *dst);
static int cdr_serdata_tag (os_uint32 *tag, void *vtp, enum sd_cdrTagType type, os_uint32 srcoff);
static int cdr_serdata_process (void *vtp, void *vst, os_uint32 tag, UNUSED_ARG (os_uint32 cdroff), const void *cdr);
#endif

sertopic_t osplser_topic4u;
sertopic_t osplser_topicpmd;
c_type osplser_topicpmd_type;
c_type osplser_topicpmd_value_type;

static const ut_avlTreedef_t topictree_treedef =
  UT_AVL_TREEDEF_INITIALIZER_INDKEY (offsetof (struct sertopic, avlnode), offsetof (struct sertopic, name_typename), (int (*) (const void *, const void *)) strcmp, 0);
static ut_avlTree_t topictree;
static os_mutex topiclock;

#ifndef NDEBUG
static int ispowerof2 (unsigned x)
{
  return x > 0 && !(x & (x-1));
}

static int ispowerof2_size (size_t x)
{
  return x > 0 && !(x & (x-1));
}
#endif

static dds_keytype_t sertype_size_to_tc (os_address sz)
{
  switch (sz)
  {
    case 1: return DDS_KEY_ONEBYTE;
    case 2: return DDS_KEY_TWOBYTES;
    case 4: return DDS_KEY_FOURBYTES;
    case 8: return DDS_KEY_EIGHTBYTES;
    default: assert (0); return DDS_KEY_ONEBYTE;
  }
}

static unsigned sertype_tc_to_size (dds_keytype_t tc)
{
  switch (tc)
  {
    case DDS_KEY_ONEBYTE: return 1;
    case DDS_KEY_TWOBYTES: return 2;
    case DDS_KEY_FOURBYTES: return 4;
    case DDS_KEY_EIGHTBYTES: return 8;
    case DDS_KEY_STRINGREF: return 4;
    case DDS_KEY_STRINGINLINE: assert (0); return 0;
    default: assert (0); return 0;
  }
}

static unsigned alignup (unsigned x, unsigned a)
{
  unsigned m = a-1;
  assert (ispowerof2 (a));
  return (x+m) & ~m;
}

static size_t alignup_size (size_t x, size_t a)
{
  size_t m = a-1;
  assert (ispowerof2_size (a));
  return (x+m) & ~m;
}

static int ceiling_lg2 (unsigned x)
{
  if (x >= (1u << 31))
    return 32;
  else
  {
    int l = 0;
    while ((1u << l) < x)
      l++;
    return l;
  }
}

static void serstate_copykey_internal (serstate_t st, const struct dds_key_descriptor *ki, const char *p)
{
  char *dst = st->data->v.key + ki->m_seroff;
  switch (ki->m_keytype)
  {
    case DDS_KEY_ONEBYTE:
      *dst = *p;
      break;
    case DDS_KEY_TWOBYTES:
    {
      unsigned short x = toBE2u (*((const unsigned short *) p));
      memcpy (dst, &x, sizeof (x));
      break;
    }
    case DDS_KEY_FOURBYTES:
    {
      unsigned x = toBE4u (*((const unsigned *) p));
      memcpy (dst, &x, sizeof (x));
      break;
    }
    case DDS_KEY_EIGHTBYTES:
    {
      unsigned long long x = toBE8u (*((const unsigned long long *) p));
      memcpy (dst, &x, sizeof (x));
      break;
    }
    case DDS_KEY_STRINGREF:
      assert ((ki->m_seroff % sizeof (unsigned)) == 0);
      *((unsigned *) dst) = (unsigned) (p - st->data->v.key);
      st->data->v.isstringref |= (1u << ki->m_seroff);
      break;
    case DDS_KEY_STRINGINLINE:
    {
      unsigned len = *((const unsigned *) p);
#if PLATFORM_IS_LITTLE_ENDIAN
      unsigned x = toBE4u (len);
      memcpy (dst, &x, sizeof (x));
      memcpy (dst + sizeof (unsigned), p + sizeof (unsigned), len);
#else
      memcpy (dst, p, sizeof (unsigned) + len);
#endif
      break;
    }
  }
}

static void serstate_copykey (serstate_t st, const char *p)
{
  const struct dds_key_descriptor *ki = &st->topic->keys[st->keyidx++];
  serstate_copykey_internal (st, ki, p);
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
        case P_FLOAT: case P_DOUBLE:
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
        case OSPL_C_STRING:
          return 1;
        default:
          return 0;
      }
      /* NOTREACHED */
    default:
      return 0;
  }
}

static int findkey (struct dds_key_descriptor *ki, C_STRUCT(c_type) const *type, const char *key, unsigned *maxsz)
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
      ki->m_keytype = sertype_size_to_tc (type->size);
      ki->align = KEYHASH_HAS_PADDING ? (unsigned short) type->size : 1;
      *maxsz = 0;
      break;
    case M_COLLECTION:
      assert (c_collectionTypeKind ((c_type) type) == OSPL_C_STRING);
      /* Usually a ref into data as 32-bit offset from the key buffer;
         but it may be an inlined CDR string, which happens to have
         (yeehaa!) the same alignment because of the included length
         field. */
      ki->m_keytype = DDS_KEY_STRINGREF;
      ki->align = sizeof (unsigned);
      *maxsz = c_collectionTypeMaxSize ((c_type) type);
      if (*maxsz == 0)
        ; /* leave it */
      else if (*maxsz < sizeof (((struct serdata *) 0)->v.key) - sizeof (unsigned))
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

static int calc_m_seroff (sertopic_t tp, const unsigned *maxstrlengths)
{
  /* Determines offsets in serdata.key & inlines strings when possible */
  unsigned new_m_seroff = 0;
  unsigned i;
#ifndef NDEBUG
  for (i = 0; i < tp->nkeys; i++)
  {
    tp->keys[i].m_seroff = ~0u;
    assert (tp->keys[i].m_keytype != DDS_KEY_STRINGINLINE);
    if (maxstrlengths[i] != 0)
      assert (tp->keys[i].m_keytype == DDS_KEY_STRINGREF);
  }
#endif
  nn_log (LC_TOPIC, "calc_m_seroff: %d keys\n", (int) tp->nkeys);
  for (i = 0; i < tp->nkeys; i++)
  {
    unsigned size = sertype_tc_to_size (tp->keys[i].m_keytype);
    nn_log (LC_TOPIC, "  key %u kso %u typecode %d size %u align %u\n",
            i, new_m_seroff, (int) tp->keys[i].m_keytype, size, tp->keys[i].align);
    if (maxstrlengths[i] > 0)
    {
      const unsigned short inline_align = KEYHASH_HAS_PADDING ? 4 : 1;
      const unsigned inline_size = (unsigned)sizeof(unsigned) + maxstrlengths[i];
      unsigned so = alignup (new_m_seroff, inline_align) + inline_size;
      unsigned j;
      for (j = i + 1; j < tp->nkeys; j++)
        so = alignup (so, tp->keys[j].align) + sertype_tc_to_size (tp->keys[j].m_keytype);
      nn_log (LC_TOPIC, "    bounded-string: inline_size %u so %u\n", inline_size, so);
      if (so <= sizeof (((struct serdata *) 0)->v.key))
      {
        tp->keys[i].m_keytype = DDS_KEY_STRINGINLINE;
        tp->keys[i].align = inline_align;
        size = inline_size;
        nn_log (LC_TOPIC, "    inlining: size %u align %u\n", size, tp->keys[i].align);
      }
    }
    tp->keys[i].m_seroff = alignup (new_m_seroff, tp->keys[i].align);
    new_m_seroff = tp->keys[i].m_seroff + size;
    nn_log (LC_TOPIC, "    m_seroff %u kso %u\n", tp->keys[i].m_seroff, new_m_seroff);
  }
  tp->keysersize = new_m_seroff;
#ifndef NDEBUG
  /* Weak check if all is well */
  if (tp->nkeys > 0)
  {
    assert (tp->keys[0].m_seroff == 0);
    for (i = 1; i < tp->nkeys; i++)
    {
      assert (tp->keys[i].m_seroff > tp->keys[i-1].m_seroff);
      assert (tp->keys[i].m_seroff < sizeof (((struct serdata *) 0)->v.key));
    }
  }
#endif
  return (tp->keysersize <= sizeof (((struct serdata *) 0)->v.key));
}

static int dds_key_descriptor_cmp_ord (const struct dds_key_descriptor *a, const struct dds_key_descriptor *b)
{
  return (a->ord == b->ord) ? 0 : (a->ord < b->ord) ? -1 : 1;
}

#if ! USE_PRIVATE_SERIALIZER
static int cdr_serdata_tag (os_uint32 *tag, void *vtp, enum sd_cdrTagType type, os_uint32 srcoff)
{
  sertopic_t tp = vtp;
  unsigned i;
#ifdef NDEBUG
  OS_UNUSED_ARG (type);
#endif
  for (i = 0; i < tp->nkeys; i++)
    if (srcoff == tp->keys[i].off)
      break;
  if (i == tp->nkeys)
    return 0;
  /* sanity check */
  switch (tp->keys[i].m_keytype)
  {
    case DDS_KEY_ONEBYTE: assert (type == SD_CDR_TT_PRIM1); break;
    case DDS_KEY_TWOBYTES: assert (type == SD_CDR_TT_PRIM2); break;
    case DDS_KEY_FOURBYTES: assert (type == SD_CDR_TT_PRIM4); break;
    case DDS_KEY_EIGHTBYTES: assert (type == SD_CDR_TT_PRIM8); break;
    case DDS_KEY_STRINGREF: assert (type == SD_CDR_TT_STRING); break;
    case DDS_KEY_STRINGINLINE: assert (type == SD_CDR_TT_STRING); break;
  }
#if CHECK_TAGS
  assert (!tp->keys[i].tagged);
  tp->keys[i].tagged = 1;
#endif
  *tag = (os_uint32) i;
  return 1;
}
#endif

static sertopic_t deftopic_unl (const char *name, C_STRUCT(v_topic) const * const ospl_topic, const char *typename, C_STRUCT(c_type) const * const type, unsigned nkeys, char const * const *keys)
{
  sertopic_t tp;
  unsigned i;
  unsigned *maxstrlengths; /* for fixed-length strings: maximum string length; anything else: 0 */
  ut_avlIPath_t path;
  char *name_typename;

  /* Reuse known definition, if available; compare is on name+typeame,
     just in case. But we forget about the keys ... */
  name_typename = os_malloc (strlen (name) + 1 + strlen (typename) + 1);
  os_sprintf (name_typename, "%s;%s", name, typename);
  tp = ut_avlLookupIPath (&topictree_treedef, &topictree, name_typename, &path);
  if (tp != NULL)
  {
    nn_log (LC_TOPIC, "deftopic_unl: reusing definition for %s\n", name_typename);
    os_free (name_typename);
    return tp;
  }

  /* Define new one */
  nn_log (LC_TOPIC, "deftopic_unl: new topic %s\n", name_typename);
  if (nkeys == 0)
    maxstrlengths = NULL;
  else
    maxstrlengths = os_malloc (nkeys * sizeof (*maxstrlengths));
  tp = os_malloc (offsetof (struct sertopic, keys) + (nkeys+1) * sizeof (tp->keys[0]));
  tp->name_typename = name_typename;
  tp->name = os_strdup (name);
  tp->typename = os_strdup (typename);
  tp->ospl_topic = c_keep ((v_topic) ospl_topic); /* drop const */
  tp->type = c_keep ((c_type) type); /* drop const */
  tp->nkeys = nkeys;
  for (i = 0; i < nkeys; i++)
  {
    if (!findkey (&tp->keys[i], type, keys[i], &maxstrlengths[i]))
      goto fail3;
    tp->keys[i].specord_idx = (unsigned short) i;
  }
  if (!calc_m_seroff (tp, maxstrlengths))
  {
    /* Keys don't fit in available space */
    goto fail3;
  }
  /* sort on serialization order */
  qsort (tp->keys, nkeys, sizeof (*tp->keys), (int (*) (const void *, const void *)) dds_key_descriptor_cmp_ord);
  /* permute specord_idx: each key now contains its original position, but we want the reverse */
  if (tp->nkeys)
  {
    unsigned short *tmp = os_malloc (nkeys * sizeof (*tmp));
    for (i = 0; i < nkeys; i++)
      tmp[tp->keys[i].specord_idx] = (unsigned short) i;
    for (i = 0; i < nkeys; i++)
      tp->keys[i].specord_idx = tmp[i];
    os_free (tmp);
  }
  /* sentinel (only "off" really matters, the others are never read) */
  tp->keys[tp->nkeys].off = ~0u;

#if ! USE_PRIVATE_SERIALIZER
  {
    struct sd_cdrControl control;
    control.init = cdr_serdata_init;
    control.grow = cdr_serdata_grow;
    control.finalize = cdr_serdata_finalize;
    control.getpos = cdr_serdata_getpos;
    control.tag = cdr_serdata_tag;
    control.tag_arg = tp;
    control.process = cdr_serdata_process;
    control.process_arg = tp;
#if CHECK_TAGS
    for (i = 0; i < tp->nkeys; i++)
      tp->keys[i].tagged = 0;
#endif
    if ((tp->ci = sd_cdrInfoNewControl (tp->type, &control)) == NULL)
      goto fail4;
    if (sd_cdrCompile (tp->ci) < 0)
      goto fail5;
#if CHECK_TAGS
    for (i = 0; i < tp->nkeys; i++)
      assert (tp->keys[i].tagged);
#endif
  }
#endif

  os_free (maxstrlengths);
  ut_avlInsertIPath (&topictree_treedef, &topictree, tp, &path);
  return tp;
 fail5:
  sd_cdrInfoFree (tp->ci);
 fail4:
 fail3:
  if (tp->ospl_topic)
    c_free (tp->ospl_topic);
  c_free (tp->type);
  os_free (tp->typename);
  os_free (tp->name);
  os_free (tp);
  os_free (maxstrlengths);
  os_free (name_typename);
  return NULL;
}

sertopic_t deftopic (C_STRUCT(v_topic) const * const ospl_topic)
{
#define MAX_NKEYS 32 /* arbitrary limit out of laziness */
  char *keystr_copy, *pos, *k;
  unsigned nkeys = 0;
  char *keys[MAX_NKEYS];
  sertopic_t tp;
  C_STRUCT (c_type) const * ospl_type;
  char *ospl_topicname;
  char *ospl_typename;
  pos = keystr_copy = os_strdup (v_topicKeyExpr(ospl_topic) ? v_topicKeyExpr(ospl_topic) : "");

  if (*keystr_copy != 0)
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

c_type topic_type (const struct sertopic * tp)
{
  return (c_type) tp->type;
}

v_topic topic_ospl_topic (const struct sertopic * tp)
{
  return (v_topic) tp->ospl_topic;
}

static void freetopic_helper (void *vtp)
{
  sertopic_t tp = vtp;
  sd_cdrInfoFree (tp->ci);
  c_free (tp->type);
  if (tp->ospl_topic)
    c_free (tp->ospl_topic);
  os_free (tp->name);
  os_free (tp->typename);
  os_free (tp->name_typename);
  os_free (tp);
}

void freetopic (UNUSED_ARG (sertopic_t tp))
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

static sertopic_t deftopic4u (c_base base)
{
  /* Special case for generating serdata in SEDP messages, which
     itself is only required because the rtps.c expects serdata to be
     present in Data messages. To be removed eventually. A topic
     consisting of a 4 unsigned 32-bit integers, all part of the
     key. This corresponds exactly to the key of SEDP ... */
  char const *keys[] = { "a", "b", "c", "d" };
  sertopic_t tp;
  os_mutexLock (&topiclock);
  tp = deftopic_unl ("....4u....", NULL, "q_osplserModule::type4u", c_resolve (base, "q_osplserModule::type4u"), 4, keys);
  os_mutexUnlock (&topiclock);
  return tp;
}

static sertopic_t deftopicpmd (c_base base)
{
  /* Special case for generating serdata in PMD */
  char const *keys[] = { "a", "b", "c", "kind" };
  sertopic_t tp;
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
  if (!loadq_osplserModule (base))
    return ERR_UNSPECIFIED;
  os_mutexInit (&topiclock, NULL);
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

static serdata_t serialize_raw_private (serstatepool_t pool, const struct sertopic * tp, const void *data)
{
  serstate_t st = ddsi_serstate_new (pool, tp);
#if USE_PRIVATE_SERIALIZER
  if (serialize1 (st, tp->type, data, 0) < 0)
  {
    ddsi_serstate_release (st);
    return NULL;
  }
#else
  if (sd_cdrSerializeControl (tp->ci, st, data) < 0)
  {
    ddsi_serstate_release (st);
    return NULL;
  }
#endif
  /* Pad to a multiple of 4, the size of the serialized data isn't
     very important so we can safely include some padding bytes.  But
     the padding is pretty much required if we don't want to send
     garbage through nn_xmsg_serdata().  Padding may be garbage, but
     valgrind doesn't like it ... */
  ddsi_serstate_append_aligned (st, 0, 4);
  return st->data;
}

serdata_t serialize_raw
(
  serstatepool_t pool, const sertopic_t tp, const void *data, unsigned statusinfo,
  nn_wctime_t timestamp, const struct nn_prismtech_writer_info *wri
)
{
  serdata_t d = serialize_raw_private (pool, tp, data);
  if (d)
    ddsi_serstate_set_msginfo (d->v.st, statusinfo, timestamp, wri);
  return d;
}

static unsigned statusinfo_from_msg (C_STRUCT (v_message) const *msg)
{
  switch (v_nodeState ((v_message) msg) & ~(L_SYNCHRONOUS | L_TRANSACTION | L_ENDOFTRANSACTION))
  {
    case 0:
      /* kernel doesn't produce it state = 0, but we do temporarily
         for pretty-printing incoming data */
    case L_WRITE:
      return 0;
    case L_WRITE | L_DISPOSED:
    case L_DISPOSED:
      return NN_STATUSINFO_DISPOSE;
    case L_UNREGISTER:
      return NN_STATUSINFO_UNREGISTER;
    default:
      NN_WARNING1 ("statusinfo_from_msg: unhandled message state: %u\n", (unsigned) v_nodeState ((v_message) msg));
      return 0;
  }
}

static void set_msginfo (serdata_t d, unsigned statusinfo, C_STRUCT (v_message) const *msg)
{
  d->v.msginfo.statusinfo = statusinfo;
  d->v.msginfo.timestamp.v = (os_int64)OS_TIMEW_GET_VALUE(msg->writeTime);
  d->v.msginfo.have_wrinfo = 1;
  d->v.msginfo.wrinfo.transactionId = msg->transactionId;
  d->v.msginfo.wrinfo.writerGID = msg->writerGID;
  d->v.msginfo.wrinfo.writerInstanceGID = msg->writerInstanceGID;
  d->v.msginfo.wrinfo.sequenceNumber = msg->sequenceNumber;
}

serdata_t serialize (serstatepool_t pool, const struct sertopic * tp, C_STRUCT (v_message) const *msg)
{
  const void *udata = msg + 1;
  serdata_t d = serialize_raw_private (pool, tp, udata);
  if (d) set_msginfo (d, statusinfo_from_msg (msg), msg);
  return d;
}

serdata_t serialize_key (serstatepool_t pool, const struct sertopic * tp, C_STRUCT (v_message) const *msg)
{
  /* Only reads key fields from data, have to fiddle a bit with the
     various indices to serialize in specification order without
     messing up key field detection for initializing key array. */
  serstate_t st = ddsi_serstate_new (pool, tp);
  const void *udata = msg + 1;

  unsigned i;
  st->kind = STK_KEY;
  for (i = 0; i < tp->nkeys; i++)
  {
    const int specidx = tp->keys[i].specord_idx;
    const struct dds_key_descriptor *ki = &tp->keys[specidx];
    st->keyidx = specidx;
    if (serialize1 (st, ki->type, (const char *) udata + ki->off, ki->off) < 0)
    {
      ddsi_serstate_release (st);
      return NULL;
    }
  }
  set_msginfo (st->data, statusinfo_from_msg (msg), msg);
  return st->data;
}

serdata_t serialize_empty (serstatepool_t pool, unsigned statusinfo, C_STRUCT (v_message) const *msg)
{
  serstate_t st = ddsi_serstate_new (pool, NULL);
  st->kind = STK_EMPTY;
  set_msginfo (st->data, statusinfo, msg);
  return st->data;
}

#if ! USE_PRIVATE_SERIALIZER
static int cdr_serdata_init (void *vst, char **dst, UNUSED_ARG (os_uint32 size_hint))
{
  serstate_t st = vst;
  *dst = st->data->data;
  return (int) st->size;
}

static int cdr_serdata_grow (void *vst, char **dst, os_uint32 size_hint)
{
  serstate_t st = vst;
  size_t osize = st->size;
  assert (*dst == st->data->data + st->size);
  st->pos = st->size;
  *dst = ddsi_serstate_append (st, size_hint);
  return (int) (st->size - osize);
}

static void cdr_serdata_finalize (void *vst, char *dst)
{
  serstate_t st = vst;
  st->pos = (unsigned) (dst - st->data->data);
}

static os_uint32 cdr_serdata_getpos (const void *vst, const char *dst)
{
  const struct serstate *st = vst;
  return (os_uint32) (dst - st->data->data);
}
#endif

/*****************************************************************************
 **
 **  serializer
 **
 *****************************************************************************/

#if ! USE_PRIVATE_SERIALIZER
static int cdr_serdata_process (void *vtp, void *vst, os_uint32 tag, os_uint32 cdroff, UNUSED_ARG (const void *cdr))
{
  sertopic_t tp = vtp;
  serstate_t st = vst;
  assert (tag < (os_uint32) tp->nkeys);
  /* Note: must compute address from base & offset because the data
     may have been reallocated in the meantime */
  serstate_copykey_internal (st, &tp->keys[tag], st->data->data + cdroff);
  return 0;
}
#endif

static int serprim (serstate_t st, C_STRUCT(c_type) const * const type, const char *data, unsigned off)
{
  char *p;
  if ((p = ddsi_serstate_append_align (st, (os_uint32) type->size)) == NULL)
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
    case P_LOCK: case P_COND:
    case P_PA_UINT32: case P_PA_UINTPTR: case P_PA_VOIDP:
    case P_COUNT:
      abort ();
    case P_WCHAR:
      /* Too big a mess to worry about it now */
      abort ();
  }
  if (off == st->topic->keys[st->keyidx].off)
  {
    serstate_copykey (st, p);
  }
  return (int) type->size;
}

static int serenum (serstate_t st, UNUSED_ARG (C_STRUCT(c_type) const * const type), const char *data, unsigned off)
{
  char *p;
  if ((p = ddsi_serstate_append_align (st, sizeof (c_long))) == NULL)
    return ERR_OUT_OF_MEMORY;
  *((c_long *) p) = *((const c_long *) data);
  if (off == st->topic->keys[st->keyidx].off)
  {
    serstate_copykey (st, p);
  }
  return (int) sizeof (c_long);
}

static int sercoll (serstate_t st, C_STRUCT(c_type) const * const type, const char *data, unsigned off)
{
  C_STRUCT(c_collectionType) const * const ctype = c_collectionType ((c_type) type);
  size_t sersize = 0;
  switch (ctype->kind)
  {
    case OSPL_C_STRING:
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
          sersize = sizeof (unsigned);
          if ((p = ddsi_serstate_append_aligned (st, sersize, sizeof (unsigned))) == NULL)
            return ERR_OUT_OF_MEMORY;
          *((unsigned *) p) = 0;
        }
        else
        {
          unsigned n = (unsigned) strlen (s) + 1;
          if (ctype->maxSize > 0 && n-1 > ctype->maxSize)
            return 0;
          sersize = sizeof (unsigned) + n;
          if ((p = ddsi_serstate_append_aligned (st, sersize, sizeof (unsigned))) == NULL)
            return ERR_OUT_OF_MEMORY;
          *((unsigned *) p) = n;
          memcpy (p + sizeof (unsigned), s, n);
        }
        if (off == st->topic->keys[st->keyidx].off)
        {
          serstate_copykey (st, p);
        }
      }
      break;

#if USE_PRIVATE_SERIALIZER
    case OSPL_C_ARRAY:
    case OSPL_C_SEQUENCE:
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
        if (ctype->kind == OSPL_C_ARRAY && ctype->maxSize > 0) {
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
        if (ctype->kind == OSPL_C_SEQUENCE || ctype->maxSize == 0)
        {
          if ((p = ddsi_serstate_append_align (st, sizeof (unsigned))) == NULL)
            return ERR_OUT_OF_MEMORY;
          *((unsigned *) p) = length;
          sersize += sizeof (unsigned);
        }
        if (length == 0)
        {
          /* do nothing */
        }
        else if (subtypekind == M_PRIMITIVE || subtypekind == M_ENUMERATION)
        {
          unsigned size1 = (unsigned) subtype->size;
          unsigned sizeN = length * size1;
          if ((p = ddsi_serstate_append_aligned (st, sizeN, size1)) == NULL)
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
            sersize += (unsigned) sersize1;
            src1 += datasize1;
          }
        }
      }
      break;
#endif

    default:
      abort ();
  }
  return (int) sersize;
}

#if USE_PRIVATE_SERIALIZER
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
#endif

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

#if USE_PRIVATE_SERIALIZER
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
#endif

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
#if USE_PRIVATE_SERIALIZER
    case M_STRUCTURE:
      return serstruct (st, type, data, off);
    case M_UNION:
      return serunion (st, type, data, off);
#endif
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
    n = os_vsnprintf (*dst, (size_t) *dstsize, fmt, ap);
    va_end (ap);
    if (n < *dstsize)
      *dst += n;
    else
      *dst += *dstsize;
    *dstsize -= n;
  }
}

static int deserprim (C_STRUCT(c_type) const * const type, char *dst, const char *src, size_t srcoff, size_t srcsize)
{
  srcoff = alignup_size (srcoff, type->size);
  if (srcoff + type->size > srcsize)
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
    case P_LOCK: case P_COND:
    case P_PA_UINT32: case P_PA_UINTPTR: case P_PA_VOIDP:
    case P_COUNT:
      abort ();
    case P_WCHAR:
      /* Too big a mess to worry about it now */
      abort ();
  }
  return (int) (srcoff + type->size);
}

static int deserprimS (C_STRUCT(c_type) const * const type, char *dst, const char *src, size_t srcoff, size_t srcsize)
{
  srcoff = alignup_size (srcoff, type->size);
  if (srcoff + type->size > srcsize)
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
    case P_LOCK: case P_COND:
    case P_PA_UINT32: case P_PA_UINTPTR: case P_PA_VOIDP:
    case P_COUNT:
      abort ();
    case P_WCHAR:
      /* Too big a mess to worry about it now */
      abort ();
  }
  return (int) (srcoff + type->size);
}

static int deserprimP (C_STRUCT(c_type) const * const type, char **dst, int *dstsize, const char *src, size_t srcoff, size_t srcsize, int swap)
{
  srcoff = alignup_size (srcoff, type->size);
  if (srcoff + type->size > srcsize)
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
        if (swap) v = bswap2 (v);
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
        if (swap) v = bswap4 (v);
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
        if (swap) v = bswap8 (v);
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
    case P_LOCK: case P_COND:
    case P_PA_UINT32: case P_PA_UINTPTR: case P_PA_VOIDP:
    case P_COUNT:
      abort ();
    case P_WCHAR:
      /* Too big a mess to worry about it now */
      abort ();
  }
  return (int) (srcoff + type->size);
}

static int deserenum (C_STRUCT(c_type) const * const type, char *dst, const char *src, size_t srcoff, size_t srcsize)
{
  srcoff = alignup_size (srcoff, sizeof (unsigned));
  if (srcoff + type->size > srcsize)
    return ERR_INVALID_DATA;
  memcpy ((unsigned *) dst, src + srcoff, sizeof (unsigned));
  return (int) (srcoff + sizeof (unsigned));
}

static int deserenumS (C_STRUCT(c_type) const * const type, char *dst, const char *src, size_t srcoff, size_t srcsize)
{
  unsigned tmp;
  srcoff = alignup_size (srcoff, sizeof (unsigned));
  if (srcoff + type->size > srcsize)
    return ERR_INVALID_DATA;
  memcpy (&tmp, src + srcoff, sizeof (unsigned));
  *((unsigned *) dst) = bswap4u (tmp);
  return (int) (srcoff + sizeof (unsigned));
}

static int deserenumP (C_STRUCT(c_type) const * const type, char **dst, int *dstsize, const char *src, size_t srcoff, size_t srcsize, int swap)
{
  unsigned v;
  srcoff = alignup_size (srcoff, sizeof (unsigned));
  if (srcoff + type->size > srcsize)
  {
    mysnprintf (dst, dstsize, "(short)");
    return ERR_INVALID_DATA;
  }
  memcpy (&v, src + srcoff, sizeof (v));
  if (swap)
    v = bswap4u (v);
  mysnprintf (dst, dstsize, "%u", v); /* can't be bother to look up label */
  return (int) (srcoff + sizeof (unsigned));
}

static int deserlengthG_unaligned (unsigned *n, const char *src, size_t *srcoff, size_t srcsize, int swap)
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

static int deserlengthG (unsigned *n, const char *src, size_t *srcoff, size_t srcsize, int swap)
{
  *srcoff = alignup_size (*srcoff, sizeof (unsigned));
  return deserlengthG_unaligned (n, src, srcoff, srcsize, swap);
}

#if USE_PRIVATE_SERIALIZER
static int deserprimarray (C_STRUCT(c_type) const * const subtype, char *array, const char *src, size_t srcoff, size_t srcsize, unsigned n)
{
  const size_t size1 = subtype->size;
  if (n == 0) /* no array elements => no need for alignment */
    return (int) srcoff;
  srcoff = alignup_size (srcoff, size1);
  /* if n was read from src, we already know size1 * n <= srcsize -
     srcoff before alignment (cf desercollG()); if not, the OpenSplice
     kernel accepted the type ... */
  if (srcoff > srcsize || size1 * n > srcsize - srcoff)
    return ERR_INVALID_DATA;
  memcpy (array, src + srcoff, n * size1);
  srcoff += n * size1;
  return (int) srcoff;
}

static int deserprimarrayS (C_STRUCT(c_type) const * const subtype, char *array, const char *src, size_t srcoff, size_t srcsize, unsigned n)
{
  const size_t size1 = subtype->size;
  unsigned i;
  if (n == 0)
    return (int) srcoff;
  srcoff = alignup_size (srcoff, size1);
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
  return (int) srcoff;
}
#endif

static int deserstringG_unaligned (C_STRUCT(c_collectionType) const * const ctype, char *dst, const char *src, size_t srcoff, size_t srcsize, int swap)
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
    return (int) srcoff;
  }
  else if (n > srcsize - srcoff)
    return ERR_INVALID_DATA;
  else if (ctype->maxSize > 0 && n-1 > ctype->maxSize)
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
  return (int) srcoff;
}

static int deserstringP_unaligned (C_STRUCT(c_collectionType) const * const ctype, char **dst, int *dstsize, const char *src, size_t srcoff, size_t srcsize, int swap)
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
    return (int) srcoff;
  }
  else if (n > srcsize - srcoff)
  {
    mysnprintf (dst, dstsize, "(string length %u out of bounds wrt serdata)", n-1);
    return ERR_INVALID_DATA;
  }
  else if (ctype->maxSize > 0 && n-1 > ctype->maxSize)
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
    unsigned char v = (unsigned char) (src + srcoff)[i];
    if (isprint (v))
      mysnprintf (dst, dstsize, "%c", v);
    else
      mysnprintf (dst, dstsize, "\\x%02x", v);
  }
  mysnprintf (dst, dstsize, "\"");
  srcoff += n;
  return (int) srcoff;
}

static int desercollG (C_STRUCT(c_type) const * const type, char *dst, const char *src, size_t srcoff, size_t srcsize, int swap)
{
  C_STRUCT(c_collectionType) const * const ctype = c_collectionType ((c_type) type);
  switch (ctype->kind)
  {
    case OSPL_C_STRING:
      {
        int off1;
        srcoff = alignup_size (srcoff, sizeof (unsigned));
        if ((off1 = deserstringG_unaligned (ctype, dst, src, srcoff, srcsize, swap)) < 0)
          return ERR_INVALID_DATA;
        srcoff = (unsigned) off1;
      }
      break;

#if USE_PRIVATE_SERIALIZER
    case OSPL_C_ARRAY:
    case OSPL_C_SEQUENCE:
      {
        /* Array: maybe reftype (reftype if vla <=> maxSize == 0, I
           think); Sequence: always reftype. (Do I really need to
           bother? Existing DDSI service does, so maybe I do. This is
           *their* logic, not mine!) */
        C_STRUCT(c_type) const * const subtype = ctype->subType;
        const c_metaKind subtypekind = c_baseObjectKind (c_baseObject (subtype));
        const size_t size1 = (c_typeIsRef ((c_type) subtype)) ? sizeof (void *) : subtype->size;
        void *array;
        unsigned n;
        if (ctype->kind == OSPL_C_ARRAY && ctype->maxSize > 0)
        {
          n = ctype->maxSize;
          assert (n > 0);
        }
        else /* C_SEQUENCE or C_ARRAY with ctype->maxSize == 0 (cf sercoll()) */
        {
          if (!deserlengthG (&n, src, &srcoff, srcsize, swap))
            return ERR_INVALID_DATA;
          if (GUARANTEED_INSUFFICIENT_BYTES_LEFT (subtype, n, srcsize - srcoff))
            return ERR_INVALID_DATA;
          if (ctype->maxSize > 0 && n > ctype->maxSize)
            return ERR_INVALID_DATA;
        }
        if (ctype->kind == OSPL_C_ARRAY && ctype->maxSize > 0)
        {
          array = dst;
        }
        else if (*(void **) dst)
        {
          /* ???Q?Q?Q?Q??? who might've done that? or it all behaves
             differently from what I expect ... */
          array = NULL;
          (void) array;
          assert(0);
        }
        else if (n == 0)
        {
          /* c_newSequence doesn't like n = 0 when the sequence is internally
             represented as a C_ARRAY with maxSize = 0 (which it sometimes is
             for historical reasons, and can't easily be changed becasue of
             backwards compatibility */
          *((void **) dst) = NULL;
          break;
        }
        else
        {
          array = c_newBaseArrayObject ((c_collectionType) ctype, n);
          if (array == NULL)
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
          srcoff = (unsigned) off1;
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
            srcoff = (unsigned) off1;
            dst1 += size1;
          }
        }
      }
      break;
#endif

    default:
      abort ();
  }
  return (int) srcoff;
}

static unsigned isprint_runlen (const unsigned char *s, unsigned n)
{
  unsigned m;
  for (m = 0; m < n && isprint (s[m]); m++)
    ;
  return m;
}

static int desercollP (C_STRUCT(c_type) const * const type, char **dst, int *dstsize, const char *src, size_t srcoff, size_t srcsize, int swap)
{
  C_STRUCT(c_collectionType) const * const ctype = c_collectionType ((c_type) type);
  switch (ctype->kind)
  {
    case OSPL_C_STRING:
      {
        int off1;
        srcoff = alignup_size (srcoff, sizeof (unsigned));
        if ((off1 = deserstringP_unaligned (ctype, dst, dstsize, src, srcoff, srcsize, swap)) < 0)
          return off1;
        srcoff = (unsigned) off1;
      }
      break;

    case OSPL_C_ARRAY:
    case OSPL_C_SEQUENCE:
      {
        /* Array: maybe reftype (reftype if vla <=> maxSize == 0, I
           think); Sequence: always reftype. (Do I really need to
           bother? Existing DDSI service does, so maybe I do. This is
           *their* logic, not mine!) */
        C_STRUCT(c_type) const * const subtype = ctype->subType;
        const unsigned size1 = (c_typeIsRef ((c_type) subtype)) ? (unsigned) sizeof (void *) : (unsigned) subtype->size;
        unsigned n;
        mysnprintf (dst, dstsize, "{");
        if (ctype->kind == OSPL_C_ARRAY && ctype->maxSize > 0)
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
          if (n > ctype->maxSize)
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
              srcoff = (unsigned) off1;
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
            srcoff = (unsigned) off1;
          }
        }
        mysnprintf (dst, dstsize, "}");
      }
      break;

    default:
      abort ();
  }
  return (int) srcoff;
}

static int desercoll (C_STRUCT(c_type) const * const type, char *dst, const char *src, size_t srcoff, size_t srcsize)
{
  return desercollG (type, dst, src, srcoff, srcsize, 0);
}

static int desercollS (C_STRUCT(c_type) const * const type, char *dst, const char *src, size_t srcoff, size_t srcsize)
{
  return desercollG (type, dst, src, srcoff, srcsize, 1);
}

#if USE_PRIVATE_SERIALIZER
static int deserstruct (C_STRUCT(c_type) const * const type, char *dst, const char *src, size_t srcoff, size_t srcsize)
{
  C_STRUCT(c_structure) const * const structure = c_structure ((c_type) type);
  const int n = (int) c_arraySize (structure->members);
  int i;
  for (i = 0; i < n; i++)
  {
    C_STRUCT(c_member) const * const member = structure->members[i];
    C_STRUCT(c_type) const * const subtype = c_typeActualType (c_specifierType (member));
    void *dst1 = C_DISPLACE (dst, member->offset);
    int off1;
    if ((off1 = deserialize1 (subtype, dst1, src, srcoff, srcsize)) < 0)
      return off1;
    srcoff = (unsigned)off1;
  }
  return (int)srcoff;
}

static int deserstructS (C_STRUCT(c_type) const * const type, char *dst, const char *src, size_t srcoff, size_t srcsize)
{
  C_STRUCT(c_structure) const * const structure = c_structure ((c_type) type);
  const int n = (int) c_arraySize (structure->members);
  int i;
  for (i = 0; i < n; i++)
  {
    C_STRUCT(c_member) const * const member = structure->members[i];
    C_STRUCT(c_type) const * const subtype = c_typeActualType (c_specifierType (member));
    void *dst1 = C_DISPLACE (dst, member->offset);
    int off1;
    if ((off1 = deserialize1S (subtype, dst1, src, srcoff, srcsize)) < 0)
      return off1;
    srcoff = (unsigned) off1;
  }
  return (int)srcoff;
}
#endif

static int deserstructP (C_STRUCT(c_type) const * const type, char **dst, int *dstsize, const char *src, size_t srcoff, size_t srcsize, int swap)
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
    srcoff = (unsigned)off1;
  }
  mysnprintf (dst, dstsize, "}");
  return (int)srcoff;
}

#if USE_PRIVATE_SERIALIZER
static int deserunion (C_STRUCT(c_type) const * const type, char *dst, const char *src, size_t srcoff, size_t srcsize)
{
  C_STRUCT(c_union) const * const utype = c_union ((c_type) type);
  C_STRUCT(c_type) const * const dtype = c_typeActualType (utype->switchType);
  c_value dvalue;
  C_STRUCT(c_unionCase) const *activecase;
  int off1;
  if ((off1 = deserialize1 (dtype, dst, src, srcoff, srcsize)) < 0)
    return off1;
  srcoff = (unsigned)off1;
  dvalue = get_discriminant_value (dtype, dst);
  activecase = active_union_case (utype, dvalue);
  if (activecase)
  {
    const unsigned disp = alignup ((unsigned) dtype->size, (unsigned) c_type (utype)->alignment);
    C_STRUCT(c_type) const * const subtype = c_typeActualType (c_specifierType (activecase));
    if ((off1 = deserialize1 (subtype, dst + disp, src, srcoff, srcsize)) < 0)
      return off1;
    srcoff = (unsigned)off1;
  }
  return (int)srcoff;
}

static int deserunionS (C_STRUCT(c_type) const * const type, char *dst, const char *src, size_t srcoff, size_t srcsize)
{
  C_STRUCT(c_union) const * const utype = c_union ((c_type) type);
  C_STRUCT(c_type) const * const dtype = c_typeActualType (utype->switchType);
  c_value dvalue;
  C_STRUCT(c_unionCase) const *activecase;
  int off1;
  if ((off1 = deserialize1S (dtype, dst, src, srcoff, srcsize)) < 0)
    return off1;
  srcoff = (unsigned)off1;
  dvalue = get_discriminant_value (dtype, dst);
  activecase = active_union_case (utype, dvalue);
  if (activecase)
  {
    const unsigned disp = alignup ((unsigned) dtype->size, (unsigned) c_type (utype)->alignment);
    C_STRUCT(c_type) const * const subtype = c_typeActualType (c_specifierType (activecase));
    if ((off1 = deserialize1S (subtype, dst + disp, src, srcoff, srcsize)) < 0)
      return off1;
    srcoff = (unsigned)off1;
  }
  return (int)srcoff;
}
#endif

static int deserunionP (C_STRUCT(c_type) const * const type, char **dst, int *dstsize, const char *src, size_t srcoff, size_t srcsize, int swap)
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
  srcoff = (unsigned)off1;
  mysnprintf (dst, dstsize, ":");
  dvalue = get_discriminant_value (dtype, tmp);
  activecase = active_union_case (utype, dvalue);
  if (activecase)
  {
    C_STRUCT(c_type) const * const subtype = c_typeActualType (c_specifierType (activecase));
    if ((off1 = deserialize1P (subtype, dst, dstsize, src, srcoff, srcsize, swap)) < 0)
      return off1;
    srcoff = (unsigned)off1;
  }
  return (int)srcoff;
}

static int deserialize1 (C_STRUCT(c_type) const * const type_x, char *dst, const char *src, size_t srcoff, size_t srcsize)
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
#if USE_PRIVATE_SERIALIZER
    case M_STRUCTURE:
      return deserstruct (type, dst, src, srcoff, srcsize);
    case M_UNION:
      return deserunion (type, dst, src, srcoff, srcsize);
#endif
    default:
      /* Do any others need to be supported? */
      abort ();
      return ERR_UNSPECIFIED;
  }
}

static int deserialize1S (C_STRUCT(c_type) const * const type_x, char *dst, const char *src, size_t srcoff, size_t srcsize)
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
#if USE_PRIVATE_SERIALIZER
    case M_STRUCTURE:
      return deserstructS (type, dst, src, srcoff, srcsize);
    case M_UNION:
      return deserunionS (type, dst, src, srcoff, srcsize);
#endif
    default:
      /* Do any others need to be supported? */
      abort ();
      return ERR_UNSPECIFIED;
  }
}

static int deserialize1P (C_STRUCT(c_type) const * const type_x, char **dst, int *dstsize, const char *src, size_t srcoff, size_t srcsize, int swap)
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

typedef int (*deserialize1_t) (C_STRUCT(c_type) const * const, char *, const char *, size_t, size_t);

static int deserialize_prep
(
  v_message *msg, char **dst, deserialize1_t *df, char const **src, size_t *srcsize,
  const struct sertopic * topic, const void *vsrc, size_t vsrcsize
)
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
  if (vsrcsize < sizeof (*hdr))
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

  if ((*msg = v_topicMessageNew_s ((v_topic) ospl_topic)) == NULL)
    return 0;
  (*msg)->qos = NULL;
  *dst = (char *) (*msg + 1);
  *df = swap ? deserialize1S : deserialize1;
  assert (vsrcsize >= sizeof (struct CDRHeader));
  *src = (const char *) vsrc + sizeof (struct CDRHeader);
  *srcsize = vsrcsize - sizeof (struct CDRHeader);
  return 1;
}

v_message deserialize (const struct sertopic * topic, const void *vsrc, size_t vsrcsize)
{
  deserialize1_t df;
  v_message msg;
  char *dst;
  const char *src;
  size_t srcsize;
  if (!deserialize_prep (&msg, &dst, &df, &src, &srcsize, topic, vsrc, vsrcsize))
    goto fail;
#if USE_PRIVATE_SERIALIZER
  if (df (topic->type, dst, src, 0, srcsize) < 0)
  {
    /* possibly allow superfluous bytes at the end? */
    goto fail;
  }
#else
  if (df == deserialize1)
  {
    if (sd_cdrDeserializeRaw (dst, topic->ci, (os_uint32) srcsize, src) < 0)
    {
      /* possibly allow superfluous bytes at the end? */
      goto fail;
    }
  }
  else
  {
    if (sd_cdrDeserializeRawBSwap (dst, topic->ci, (os_uint32) srcsize, src) < 0)
    {
      /* possibly allow superfluous bytes at the end? */
      goto fail;
    }
  }
#endif
  return msg;
 fail:
  if (msg) c_free (msg);
  return NULL;
}

v_message deserialize_from_key (const struct sertopic * topic, const void *vsrc, size_t vsrcsize)
{
  /* v_topic has a messageKeyList which is an array of 'c_field's,
     each of which contains an offset that presumably is an offset
     into the data part of a message. If not, we can always roll our
     own ... */
  deserialize1_t df;
  v_message msg;
  char *dst;
  const char *src;
  size_t srcsize;
  unsigned i;
  int off;
  if (!deserialize_prep (&msg, &dst, &df, &src, &srcsize, topic, vsrc, vsrcsize))
    goto fail;
  assert ((unsigned) c_arraySize (v_topicMessageKeyList(topic->ospl_topic)) == topic->nkeys);
  off = 0;
  for (i = 0; i < topic->nkeys; i++)
  {
    const struct dds_key_descriptor *ki = &topic->keys[topic->keys[i].specord_idx];
    if ((off = df (ki->type, dst + ki->off, src, (unsigned) off, srcsize)) < 0)
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

v_message deserialize_from_keyhash (const struct sertopic * topic, const void *vsrc, size_t srcsize)
{
  C_STRUCT(v_topic) const * const ospl_topic = topic->ospl_topic;
  const char *src = vsrc;
  v_message msg;
  char *dst;
  unsigned i;
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
  if ((msg = v_topicMessageNew_s ((v_topic) ospl_topic)) == NULL)
    return NULL;
  msg->qos = NULL;
  dst = (char *) (msg + 1);
  for (i = 0; i < topic->nkeys; i++)
  {
    const struct dds_key_descriptor *ki = &topic->keys[i]; /* order is irrelevant */
    switch (ki->m_keytype)
    {
      case DDS_KEY_ONEBYTE:
        *(dst + ki->off) = *(src + ki->m_seroff);
        break;
      case DDS_KEY_TWOBYTES:
        {
          unsigned short tmp;
          memcpy (&tmp, src + ki->m_seroff, sizeof (tmp));
          *((unsigned short *) (dst + ki->off)) = fromBE2u (tmp);
        }
        break;
      case DDS_KEY_FOURBYTES:
        {
          unsigned tmp;
          memcpy (&tmp, src + ki->m_seroff, sizeof (tmp));
          *((unsigned *) (dst + ki->off)) = fromBE4u (tmp);
        }
        break;
      case DDS_KEY_EIGHTBYTES:
        {
          unsigned long long tmp;
          memcpy (&tmp, src + ki->m_seroff, sizeof (tmp));
          *((unsigned long long *) (dst + ki->off)) = fromBE8u (tmp);
        }
        break;
      case DDS_KEY_STRINGREF:
        /* Oops! keyhash can't faithfully represent the key! How
           naughty a sender (I can't imagine I can do any harm, so
           late detection is good enough) */
        goto fail;
      case DDS_KEY_STRINGINLINE:
        {
          C_STRUCT(c_collectionType) const * const ctype = c_collectionType (ki->type);
          assert (ctype->kind == OSPL_C_STRING);
          if (deserstringG_unaligned (ctype, dst + ki->off, src, ki->m_seroff, srcsize, PLATFORM_IS_LITTLE_ENDIAN) < 0)
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

static int prettyprint_prep (char **dst, int *dstsize, int *swap, const char **src, size_t *srcsize, const void *vsrc, size_t vsrcsize)
{
  const struct CDRHeader *hdr = vsrc;
  if (vsrcsize < sizeof (*hdr))
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
  assert (vsrcsize >= sizeof (struct CDRHeader));
  *src = (const char *) (hdr + 1);
  *srcsize = vsrcsize - sizeof (struct CDRHeader);
  return 1;
 fail:
  return 0;
}

int prettyprint_raw (char *dst, const int dstsize, const struct sertopic * topic, const void *vsrc, size_t vsrcsize)
{
  int dstsize1 = dstsize;
  const char *src;
  int swap = 0;
  size_t srcsize;
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

static int prettyprint_key (char *dst, const int dstsize, const struct sertopic * topic, const void *vsrc, size_t vsrcsize)
{
  int dstsize1 = dstsize;
  const char *src;
  int swap = 0;
  size_t srcsize;
  unsigned i;
  int off;
  if (!prettyprint_prep (&dst, &dstsize1, &swap, &src, &srcsize, vsrc, vsrcsize))
    goto fail;
  assert (c_arraySize (v_topicMessageKeyList(topic->ospl_topic)) == topic->nkeys);
  mysnprintf (&dst, &dstsize1, "k:{");
  off = 0;
  for (i = 0; i < topic->nkeys; i++)
  {
    const struct dds_key_descriptor *ki = &topic->keys[topic->keys[i].specord_idx];
    if (i > 0)
      mysnprintf (&dst, &dstsize1, ",");
    if ((off = deserialize1P (ki->type, &dst, &dstsize1, src, (unsigned) off, srcsize, swap)) < 0)
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
  if (serdata->v.st->topic == NULL)
  {
    const char *kindstr = "?:";
    int dstsize1 = dstsize;
    switch (serdata->v.st->kind)
    {
      case STK_EMPTY: kindstr = "e:"; break;
      case STK_KEY: kindstr = "k:"; break;
      case STK_DATA: kindstr = ""; break;
    }
    mysnprintf (&dst, &dstsize1, "%s(blob)", kindstr);
    return dstsize - dstsize1;
  }
  else
  {
    switch (serdata->v.st->kind)
    {
      case STK_EMPTY:
        assert (0);
        return 0;
      case STK_KEY:
        return prettyprint_key
          (dst, dstsize, serdata->v.st->topic, &serdata->hdr, ddsi_serdata_size (serdata));
      case STK_DATA:
        return prettyprint_raw
          (dst, dstsize, serdata->v.st->topic, &serdata->hdr, ddsi_serdata_size (serdata));
    }
  }
  assert(0);
  return 0;
}

/*************************************/

static int verdata (C_STRUCT(c_type) const * const type_x, const char *a, const char *b);

static int verret (int x)
{
  return x;
}

static int verprim (C_STRUCT(c_type) const * const type, const char *a, const char *b)
{
  return verret (memcmp (a, b, type->size) == 0);
}

static int verenum (C_STRUCT(c_type) const * const type, const char *a, const char *b)
{
  return verret (verprim (type, a, b));
}

static int vercoll (C_STRUCT(c_type) const * const type, const char *a, const char *b)
{
  C_STRUCT(c_collectionType) const * const ctype = c_collectionType ((c_type) type);
  switch (ctype->kind)
  {
    case OSPL_C_STRING:
      {
        const char *a1 = (const char *) (*((char **) a));
        const char *b1 = (const char *) (*((char **) b));
        if (a1 == NULL)
          return verret (b1 == NULL || *b1 == '\0');
        else if (b1 == NULL)
          return verret (0);
        else
          return verret (strcmp (a1, b1) == 0);
      }

    case OSPL_C_ARRAY:
    case OSPL_C_SEQUENCE:
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
        if (ctype->kind == OSPL_C_ARRAY && ctype->maxSize > 0) {
          a1 = a;
          b1 = b;
          length_a = length_b = ctype->maxSize;
        } else {
          a1 = *((char const * const *) a);
          b1 = *((char const * const *) b);
          length_a = c_arraySize ((void *) a1);
          length_b = c_arraySize ((void *) b1);
          if ((length_a != 0) && (a1 == NULL))
            return verret (0);
          if ((length_b != 0) && (b1 == NULL))
            return verret (0);
        }
        if (length_a != length_b)
          return verret (0);
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
  return verret (0);
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
  const void *a;
  const void *b;
  deserialize1_t df;
  v_message msg;
  char *dst;
  const char *src;
  size_t srcsize;
  int rc, res = 0;

  if (!deserialize_prep (&msg, &dst, &df, &src, &srcsize, serdata->v.st->topic, &serdata->hdr, ddsi_serdata_size (serdata)))
  {
    goto fail;
  }
#if USE_PRIVATE_SERIALIZER
  rc = df (serdata->v.st->topic->type, dst, src, 0, srcsize);
  if (rc == ERR_OUT_OF_MEMORY)
    return 1;
#else
  rc = sd_cdrDeserializeRaw (dst, serdata->v.st->topic->ci, (os_uint32) srcsize, src);
  if (rc == SD_CDR_OUT_OF_MEMORY)
    return 1;
#endif
  if (rc < 0)
  {
    NN_ERROR2 ("serdata_verify: can't even deserialize my own data ... (serdata %p srcmsg %p)\n",
               (void *) serdata, (void *) srcmsg);
    goto fail;
  }

  a = srcmsg + 1;
  b = msg + 1;
  res = verdata (serdata->v.st->topic->type, a, b);
 fail:
  c_free (msg);
  return res;
}

static void gen_keyhash_md5 (struct serdata const * const d, char keyhash[16])
{
  const struct sertopic * tp = d->v.st->topic;

  /* Process the keys in specified order, as opposed to serialization
     order. (Even though the two are almost certainly going to be the
     same, I haven't found a spot where it is guaranteed.) */

  size_t off = 0;
  unsigned i;
  md5_state_t md5st;
  md5_init (&md5st);
  for (i = 0; i < tp->nkeys; i++)
  {
    const struct dds_key_descriptor *ki = &tp->keys[tp->keys[i].specord_idx];
    const char *src = d->v.key + ki->m_seroff;
    switch (ki->m_keytype)
    {
      case DDS_KEY_ONEBYTE:
        md5_append (&md5st, (const unsigned char *) src, 1);
        off += 1;
        break;
      case DDS_KEY_TWOBYTES:
        md5_append (&md5st, (const unsigned char *) src, 2);
        off += 2;
        break;
      case DDS_KEY_FOURBYTES:
        md5_append (&md5st, (const unsigned char *) src, 4);
        off += 4;
        break;
      case DDS_KEY_EIGHTBYTES:
        md5_append (&md5st, (const unsigned char *) src, 8);
        off += 8;
        break;
      case DDS_KEY_STRINGREF:
        {
          const struct cdrstring *str;
          assert (d->v.isstringref & (1u << ki->m_seroff));
          str = (const struct cdrstring *) (d->v.key + *((unsigned *) src));
#if PLATFORM_IS_LITTLE_ENDIAN
          {
            unsigned lenBE = toBE4u (str->length);
            md5_append (&md5st, (const unsigned char *) &lenBE, sizeof (lenBE));
          }
          md5_append (&md5st, (const unsigned char *) str->contents, str->length);
#else
          md5_append (&md5st, (const unsigned char *) str, (unsigned) (sizeof (unsigned) + str->length));
#endif
          off += sizeof (unsigned) + str->length;
        }
        break;
      case DDS_KEY_STRINGINLINE:
        {
          unsigned len;
          assert (!(d->v.isstringref & (1u << ki->m_seroff)));
          memcpy (&len, src, sizeof (unsigned));
          len = fromBE4u (len);
          md5_append (&md5st, (const unsigned char *) src, (unsigned) (sizeof (unsigned) + len));
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
     into d->v.key. If d->v.key is at least as long as keyhash, that means
     that won't fit in keyhash either, and so the presence of string
     references and a serialized key larger than keyhash both call for
     the use of MD5. */
  assert (sizeof (d->v.key) >= 16);
  if (d->v.isstringref == 0 && (d->v.st->topic == NULL || d->v.st->topic->keysersize <= 16))
    memcpy (keyhash, d->v.key, 16);
  else
    gen_keyhash_md5 (d, keyhash);
}

/* Find the first bit set in I. (From glibc, obviously correct by
   inspection.)
*/

static unsigned myffs (unsigned i)
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
  unsigned a;
  unsigned x = i & -i;
  a = x <= 0xffff ? (x <= 0xff ? 0 : 8) : (x <= 0xffffff ?  16 : 24);
  return table[x >> a] + a;
}

int serdata_cmp (const struct serdata *a, const struct serdata *b)
{
  unsigned x = a->v.isstringref;
  unsigned p = 0;
  unsigned keysersize = a->v.st->topic ? a->v.st->topic->keysersize : 16;
  int c;
  assert (a->v.isstringref == b->v.isstringref);
  assert (a->v.st->topic == b->v.st->topic);
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
      if ((c = memcmp (a->v.key + p, b->v.key + p, q - p)) != 0)
        return c;
      p = q;
    }
    /* compare strings (not lexicographically: no need for that!) */
    {
      const struct cdrstring *astr =
        (const struct cdrstring *) (a->v.key + *((unsigned *) (a->v.key + p)));
      const struct cdrstring *bstr =
        (const struct cdrstring *) (b->v.key + *((unsigned *) (b->v.key + p)));
      if (astr->length != bstr->length)
        /* just use string length, ignore the text */
        return (astr->length < bstr->length) ? -1 : 1;
      else if ((c = memcmp (astr->contents, bstr->contents, astr->length)) != 0)
        return c;
      p += (unsigned) (sizeof (unsigned));
    }
  }
  if (p < keysersize)
    return memcmp (a->v.key + p, b->v.key + p, keysersize - p);
  else
    return 0;
}

unsigned serdata_hash (const struct serdata *a)
{
  if (a->v.hash_valid)
  {
    pa_fence();
    return a->v.hash;
  }
  else
  {
    const size_t keysersize = a->v.st->topic ? a->v.st->topic->keysersize : 16;
    /* I believe this is legal C, provided whatever a points to is not a
     originally defined as a const-qualified object */
    volatile struct serdata *va = (volatile struct serdata *) a;
    os_uint32 h;

    /* a->key is _always_ zero-padded, and we rely on that */
#ifndef NDEBUG
    {
      size_t i;
      for (i = keysersize; i < sizeof (a->v.key); i++)
        assert(a->v.key[i] == 0);
    }
#endif

    if (!a->v.isstringref)
    {
#define UINT64_CONST(x, y, z) (((os_uint64) (x) * 1000000 + (y)) * 1000000 + (z))
      const os_uint64 hc0 = UINT64_CONST (16292676, 669999, 574021);
      const os_uint64 hc1 = UINT64_CONST (10242350, 189706, 880077);
#undef UINT64_CONST
      if (keysersize <= sizeof (os_uint32))
      {
        /* simple universal hashing of a 32-bit int, key is char[] and properly
         aligned, so the cast is valid */
        const os_uint32 *k = (const os_uint32 *) a->v.key;
        h = (os_uint32) ((k[0] * hc0) >> 32);
      }
      else if (keysersize <= 2 * sizeof (os_uint32))
      {
        const os_uint32 *k = (const os_uint32 *) a->v.key;
        h = (os_uint32) (((k[0] + hc0) * (k[1] + hc1)) >> 32);
      }
      else
      {
        /* should benchmark to find out at what point switching to a fancy hash
         function is sensible -- but reality is that more than 2 keys is rare
         and that 64-bit integers as keys are rare, so once we get here it is
         probably going to be a string key, which one shouldn't expect to be
         as fast anyway. */
        h = murmurhash3(a->v.key, keysersize, 0);
      }
    }
    else
    {
      unsigned x = a->v.isstringref;
      unsigned p = 0;
      h = 0;
      while (x)
      {
        unsigned q = myffs (x) - 1;
        x ^= x & -x; /* x & -x: lowest set bit => xor clears bit q */
        assert (q >= p);
        assert (p < keysersize);
        assert (q < keysersize);

        /* hash any bytes preceding next referenced string */
        if (q > p)
        {
          h = murmurhash3(a->v.key + p, q - p, h);
          p = q;
        }

        /* hash referenced string */
        {
          const struct cdrstring *astr = (const struct cdrstring *) (a->v.key + *((unsigned *) (a->v.key + p)));
          h = murmurhash3(astr, 4 + astr->length, h);
          p += sizeof (unsigned);
        }
      }
      if (p < keysersize)
      {
        h = murmurhash3(a->v.key + p, keysersize - p, h);
      }
    }

    va->v.hash = h;
    pa_fence();
    va->v.hash_valid = 1;
    return h;
  }
}

void serstate_set_key (serstate_t st, int justkey, const void *key)
{
  st->kind = justkey ? STK_KEY : STK_DATA;
  st->data->v.isstringref = 0;
  memcpy (st->data->v.key, key, 16);
}

void serstate_init (serstate_t st, const struct sertopic * topic)
{
  st->pos = 0;
  st->keyidx = 0;
  st->topic = topic;
  pa_st32 (&st->refcount, 1);
  st->kind = STK_DATA; /* key and empty are rare cases and set separately */
  st->twrite.v = -1;
  st->data->v.isstringref = 0;
  if (topic)
    st->data->hdr.identifier = PLATFORM_IS_LITTLE_ENDIAN ? CDR_LE : CDR_BE;
  else
    st->data->hdr.identifier = PLATFORM_IS_LITTLE_ENDIAN ? PL_CDR_LE : PL_CDR_BE;

#ifndef NDEBUG
  st->data->v.msginfo.have_wrinfo = 2; /* invalid value */
#endif

  if (topic == NULL || topic->keysersize > 0)
  {
    st->data->v.hash_valid = 0;
  }
  else
  {
    st->data->v.hash_valid = 1;
    st->data->v.hash = 0;
  }
  memset (st->data->v.key, 0, sizeof (st->data->v.key));
}

void serstate_free (serstate_t st)
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

/* SHA1 not available (unoffical build.) */

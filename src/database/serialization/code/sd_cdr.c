/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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

#include "os_abstract.h"
#include "os_stdlib.h"
#include "os_defs.h"
#include "os_thread.h"
#include "os_heap.h"
#include "os_mutex.h"

#include "c_base.h"
#include "c_collection.h"
#include "c_typebase.h"

#include "sd_cdr.h"

#ifdef __GNUC__
#define UNUSED_ARG(x) x __attribute__ ((unused))
#else
#define UNUSED_ARG(x) x
#endif

#ifndef NDEBUG
#define UNUSED_ARG_NDEBUG(x) x
#else
#define UNUSED_ARG_NDEBUG(x) UNUSED_ARG (x)
#endif

#if defined PA_LITTLE_ENDIAN
#define BE_NEEDS_BSWAP 1
#else
#define BE_NEEDS_BSWAP 0
#endif

/* Defines for debugging purposes : */
/*#define PRINTTYPE*/
/*#define PRINTSIZE*/
/*#define PRINTPROG*/
/*#define PRINTALLOC*/
/*#define PROGEXEC_TRACE*/

#define NOT_A_SRCOFF (~0u)

enum sd_cdrInfoStatus {
  SD_CIS_FRESH,
  SD_CIS_UNSUPPORTED,
  SD_CIS_READY
};

struct sd_cdrInfo {
  enum sd_cdrInfoStatus status;
  c_type ktype;
  int clear_padding;

  struct sd_catsstac *catsstac_head;
  struct sd_catsstac *catsstac_tail;

  struct sd_quietref *quietref_head;
  struct sd_quietref *quietref_tail;

  size_t minsize;
  size_t maxsize;
  size_t initial_alloc;
  int dynalloc; /* use realloc */
  struct serprog *prog;
  struct sd_cdrControl control;
};

enum ser_typekind {
  TK_NONE,
  TK_PRIM1,
  TK_PRIM2,
  TK_PRIM4,
  TK_PRIM8,
  TK_ARRAY,
  TK_STRING,
  TK_STRING_TO_ARRAY,
  TK_ARRAY_TO_STRING,
  TK_SEQUENCE,
  TK_RSEQUENCE,
  TK_STRUCT,
  TK_UNION_LIST,
  TK_CLASS /* unique => guaranteed not a reference to an already serialised object; probably not fully supported */
};

struct ser_type;

struct ser_structmember {
  size_t off;
  struct ser_type *type;
};

struct ser_unionmember {
  unsigned long long dv;
  struct ser_type *type;
};

struct ser_cdralign {
  size_t align;               /* 1, 2, 4 or 8 */
  size_t off;                 /* 0 <= off < align */
};

#define MAX_TAG 0xfffff /* 20 bits, see tag field in struct ser_type */

struct ser_type {
  enum ser_typekind kind;
  unsigned label;
  int recuse; /* set if used recursively */
  size_t width;
  struct ser_cdralign align;
  os_uint32 has_tag: 1;
  os_uint32 tag: 20; /* tags must fit in insn.count, see MAX_TAG */
  union ser_typeunion {
    int dummy;
    struct {
      unsigned maxn; /* 0 => unbounded */
    } string;
    struct {
      unsigned n;
      struct ser_type *subtype;
    } array;
    struct {
      unsigned n;
    } string_to_array;
    struct {
      unsigned n;
    } array_to_string;
    struct {
      unsigned maxn; /* 0 => unbounded */
      struct ser_type *subtype;
      const struct c_collectionType_s *ospl_type; /* needed for deserializing sequence */
    } sequence;
    struct {
      unsigned maxn; /* 0 => unbounded */
      unsigned sublabel;
      const struct c_collectionType_s *ospl_type; /* needed for deserializing sequence */
    } rsequence;
    struct {
      unsigned n;
      struct ser_structmember ms[1 /* n */];
    } strukt;
    struct {
      unsigned n;
      unsigned hasdefault: 1; /* whether original has a default value */
      enum ser_typekind dkind;
      size_t moff; /* member offset */
      struct ser_unionmember ms[1 /* n+1; last is default; aliasing of ms[].type.u allowed */];
    } union_list;
    struct {
      const struct c_type_s *ospl_type; /* needed for instantiating object when deserializing */
      int quietref;
      struct ser_type *subtype; /* guaranteed to be a TK_STRUCT */
    } class;
  } u;
};

static unsigned char bswap1u (unsigned char x)
{
  return x;
}

static unsigned short bswap2u (unsigned short x)
{
  return (unsigned short) ((x >> 8) | (x << 8));
}

static unsigned bswap4u (unsigned x)
{
  return (x >> 24) | ((x >> 8) & 0xff00) | ((x << 8) & 0xff0000) | (x << 24);
}

static unsigned long long bswap8u (unsigned long long x)
{
  return
    ( x << 56) |
    ((x << 40) & ((unsigned long long) 0xff << 48)) |
    ((x << 24) & ((unsigned long long) 0xff << 40)) |
    ((x << 8)  & ((unsigned long long) 0xff << 32)) |
    ((x >> 8)  & ((unsigned long long) 0xff << 24)) |
    ((x >> 24) & ((unsigned long long) 0xff << 16)) |
    ((x >> 40) & ((unsigned long long) 0xff << 8)) |
    ( x >> 56);
}

/************** SPECIAL-PURPOSE ALLOCATOR **************/

struct convtype_allocator {
  void *blocks;
  size_t total;
};

static void convtype_allocator_init (struct convtype_allocator *alloc)
{
  alloc->blocks = NULL;
  alloc->total = 0;
}

static void convtype_allocator_fini (struct convtype_allocator *alloc)
{
  while (alloc->blocks)
  {
    void **b = alloc->blocks;
    alloc->blocks = *b;
    os_free (b);
  }
#ifdef PRINTSIZE
  fprintf (stderr, "convtype alloc total: %u\n", alloc->total);
#endif
}

static struct ser_type *convtype_alloc (struct convtype_allocator *alloc, enum ser_typekind kind, unsigned label, size_t width, size_t allocextra)
{
  const size_t hsz = (sizeof (void *) + 7) & (size_t)-8;
  const size_t asz = hsz + sizeof (struct ser_type) + allocextra;
  struct ser_type *t;
  char *p;
  if ((p = os_malloc (asz)) == NULL)
    return NULL;
  *((void **) p) = alloc->blocks;
  alloc->blocks = p;
  alloc->total += asz;
  t = (struct ser_type *) (p + hsz);
  t->kind = kind;
  t->label = label;
  t->width = width;
  t->recuse = 0;
  t->align.align = 0;
  t->align.off = 0;
  t->has_tag = 0;
  t->tag = 0;
  return t;
}

static os_uint32 srcoff_add (os_uint32 srcoff, os_size_t add)
{
  assert (add <= 0x80000000);
  return (srcoff == NOT_A_SRCOFF) ? srcoff : (srcoff + (os_uint32) add);
}

/******************** TYPE CONVERSION ********************/

struct sd_catsstac {
  struct sd_catsstac *next;
  unsigned n;
  const struct c_type_s *typestack[1];
};

struct sd_quietref {
  struct sd_quietref *next;
  unsigned n;
  const struct c_type_s *typestack[1];
};

struct convtype_context {
  struct sd_cdrInfo *ci;
  struct sd_catsstac *catsstac;
  struct sd_quietref *quietref;
  struct convtype_allocator alloc;
  unsigned next_label;
  unsigned typestack_depth;
  const struct c_type_s *typestack[128];
  unsigned labels[128];
  int recuse[128];
};

static int convtype (struct convtype_context *ctx, struct ser_type **rt, const struct c_type_s *type0, os_uint32 srcoff, const struct sd_cdrControl *control);

static enum sd_cdrTagType map_tag_type (enum ser_typekind kind)
{
  switch (kind)
  {
    case TK_PRIM1: return SD_CDR_TT_PRIM1;
    case TK_PRIM2: return SD_CDR_TT_PRIM2;
    case TK_PRIM4: return SD_CDR_TT_PRIM4;
    case TK_PRIM8: return SD_CDR_TT_PRIM8;
    case TK_STRING: return SD_CDR_TT_STRING;
    default: abort (); return SD_CDR_TT_PRIM1;
  }
}

static int kind_is_primN (enum ser_typekind kind)
{
  switch (kind)
  {
    case TK_PRIM1: case TK_PRIM2: case TK_PRIM4: case TK_PRIM8:
      return 1;
    default:
      return 0;
  }
}

static enum ser_typekind convprimtype (const struct c_type_s *type0)
{
  const struct c_type_s *type = c_typeActualType ((c_type) type0);
  switch (c_baseObjectKind ((c_type) type))
  {
    case M_PRIMITIVE:
      switch (type->size)
      {
        case 1: return TK_PRIM1;
        case 2: return TK_PRIM2;
        case 4: return TK_PRIM4;
        case 8: return TK_PRIM8;
        default: abort (); return TK_NONE;
      }
    case M_ENUMERATION:
      return TK_PRIM4;
    default:
      return TK_NONE;
  }
}

static int isprimtype (const struct c_type_s *type0)
{
  return convprimtype (type0) != TK_NONE;
}

static int mk_array (struct convtype_context *ctx, struct ser_type **rt, unsigned label, unsigned n, struct ser_type *subtype)
{
  if (n == 1)
  {
    *rt = subtype;
  }
  else if (subtype->kind == TK_ARRAY)
  {
    *rt = subtype;
    (*rt)->width *= n;
    (*rt)->u.array.n *= n;
  }
  else
  {
    if ((*rt = convtype_alloc (&ctx->alloc, TK_ARRAY, label, n * subtype->width, 0)) == NULL)
      return SD_CDR_OUT_OF_MEMORY;
    (*rt)->u.array.n = n;
    (*rt)->u.array.subtype = subtype;
  }
  return 0;
}

static int mk_sequence (struct convtype_context *ctx, struct ser_type **rt, unsigned label, unsigned maxn, struct ser_type *subtype, const struct c_collectionType_s *ospl_type)
{
  if ((*rt = convtype_alloc (&ctx->alloc, TK_SEQUENCE, label, sizeof (void *), 0)) == NULL)
    return SD_CDR_OUT_OF_MEMORY;
  (*rt)->u.sequence.maxn = maxn;
  (*rt)->u.sequence.subtype = subtype;
  (*rt)->u.sequence.ospl_type = ospl_type;
  return 0;
}

static int convstruct_collapse_substructs (struct convtype_context *ctx, struct ser_type **rt)
{
  const unsigned n = (*rt)->u.strukt.n;
  struct ser_structmember *ms = (*rt)->u.strukt.ms;
  unsigned i, ncollapse = 0;
  int have_substructs = 0;
  for (i = 0; i < n; i++)
  {
    if (ms[i].type->kind != TK_STRUCT || ms[i].type->recuse)
      ncollapse++;
    else
    {
      ncollapse += ms[i].type->u.strukt.n;
      have_substructs = 1;
    }
  }
  if (have_substructs)
  {
    unsigned j;
    struct ser_type *rtnew;
    struct ser_structmember *msnew;
    if ((rtnew = convtype_alloc (&ctx->alloc, TK_STRUCT, (*rt)->label, (*rt)->width, ncollapse * sizeof (rtnew->u.strukt.ms[0]))) == NULL)
      return SD_CDR_OUT_OF_MEMORY;
    rtnew->u.strukt.n = ncollapse;
    msnew = rtnew->u.strukt.ms;
    j = 0;
    for (i = 0; i < n; i++)
    {
      if (ms[i].type->kind != TK_STRUCT || ms[i].type->recuse)
        msnew[j++] = ms[i];
      else
      {
        const size_t off = ms[i].off;
        const unsigned subn = ms[i].type->u.strukt.n;
        unsigned k;
        memmove (&msnew[j], ms[i].type->u.strukt.ms, subn * sizeof (*ms));
        for (k = 0; k < subn; k++)
          msnew[j++].off += off;
      }
    }
    assert (j == ncollapse);
    *rt = rtnew;
  }
  return 0;
}

static enum ser_typekind ser_prim_or_primarray_kind (const struct ser_type *type)
{
  switch (type->kind)
  {
    case TK_PRIM1: case TK_PRIM2: case TK_PRIM4: case TK_PRIM8:
      return type->has_tag ? TK_NONE : type->kind;
    case TK_ARRAY:
      return kind_is_primN (type->u.array.subtype->kind) ? type->u.array.subtype->kind : TK_NONE;
    default:
      return TK_NONE;
  }
}

static unsigned ser_primwidth (enum ser_typekind kind)
{
  switch (kind)
  {
    case TK_PRIM1: return 1;
    case TK_PRIM2: return 2;
    case TK_PRIM4: return 4;
    case TK_PRIM8: return 8;
    default: abort (); return 0;
  }
}

static int convstruct_discover_primarray (struct convtype_context *ctx, struct ser_type **rt)
{
  struct ser_structmember * const ms = (*rt)->u.strukt.ms;
  unsigned i, j;
  int rc;
  i = 0;
  while (i < (*rt)->u.strukt.n)
  {
    const unsigned minlength = 2;
    enum ser_typekind kind;
    unsigned width;
    size_t noff;
    unsigned totn;
    if ((kind = ser_prim_or_primarray_kind (ms[i].type)) == TK_NONE)
    {
      i++;
      continue;
    }
    width = ser_primwidth (kind);
    totn = (ms[i].type->kind == TK_ARRAY) ? ms[i].type->u.array.n : 1;
    noff = ms[i].off + width * totn;
    for (j = i + 1; j < (*rt)->u.strukt.n; j++)
    {
      unsigned n;
      if (ser_prim_or_primarray_kind (ms[j].type) != kind || ms[j].off != noff)
        break;
      n = (ms[j].type->kind == TK_ARRAY) ? ms[j].type->u.array.n : 1;
      totn += n;
      noff += width * n;
    }
    if (totn < minlength)
      i = j;
    else
    {
      struct ser_type *subtype;
      assert ((*rt)->u.strukt.n > j - i - 1);
      if ((subtype = convtype_alloc (&ctx->alloc, kind, 0, width, 0)) == NULL)
        return SD_CDR_OUT_OF_MEMORY;
      if ((rc = mk_array (ctx, &ms[i].type, 0, totn, subtype)) < 0)
        return rc;
      memmove (&ms[i + 1], &ms[j], ((*rt)->u.strukt.n - j) * sizeof (*ms));
      (*rt)->u.strukt.n -= j - i - 1;
      i++;
    }
  }
  return 0;
}

static int convstruct_elide_singleton (struct ser_type **rt)
{
  if ((*rt)->u.strukt.n == 1)
  {
    assert ((*rt)->u.strukt.ms[0].off == 0);
    *rt = (*rt)->u.strukt.ms[0].type;
  }
  return 0;
}

static int convstruct (struct convtype_context *ctx, struct ser_type **rt, unsigned label, const struct c_structure_s *stype, os_uint32 srcoff, const struct sd_cdrControl *control)
{
  const unsigned n = c_arraySize (stype->members);
  struct ser_structmember *ms;
  unsigned i;
  int rc;
  /* deliberately allocate a little more memory than needed, for readability */
  if ((*rt = convtype_alloc (&ctx->alloc, TK_STRUCT, label, stype->_parent.size, n * sizeof ((*rt)->u.strukt.ms[0]))) == NULL)
    return SD_CDR_OUT_OF_MEMORY;
  (*rt)->u.strukt.n = n;
  ms = (*rt)->u.strukt.ms;
  for (i = 0; i < n; i++)
  {
    const struct c_member_s *member = stype->members[i];
    ms[i].off = member->offset;
    assert (i == 0 || ms[i].off > ms[i-1].off);
    if ((rc = convtype (ctx, &ms[i].type, member->_parent.type, srcoff_add (srcoff, member->offset), control)) < 0)
      return rc;
  }
  /* collapse substructs */
  if ((rc = convstruct_collapse_substructs (ctx, rt)) < 0)
    return rc;
  if ((rc = convstruct_discover_primarray (ctx, rt)) < 0)
    return rc;
  if ((rc = convstruct_elide_singleton (rt)) < 0)
    return rc;
  return 0;
}

struct convclass_count_arg {
  unsigned n;
};

struct convclass_convert_arg {
  struct convtype_context *ctx;
  struct ser_type *st;
  unsigned n; /* const, determined by _count */
  unsigned i;
  os_uint32 srcoff;
  const struct sd_cdrControl *control;
  int error;
};

static c_bool convclass_count (const void *object, void *varg)
{
  struct convclass_count_arg *arg = varg;
  if (c_baseObjectKind (object) == M_ATTRIBUTE)
    arg->n++;
  return 1;
}

static c_bool convclass_convert (const void *object, void *varg)
{
  struct convclass_convert_arg *arg = varg;
  assert (arg->st->kind == TK_STRUCT);
  if (c_baseObjectKind (object) == M_ATTRIBUTE)
  {
    const struct c_property_s *prop = (const struct c_property_s *) object;
    struct ser_structmember * const ms = arg->st->u.strukt.ms;
    int rc;
    assert (arg->i < arg->n);
    ms[arg->i].off = prop->offset;
    if ((rc = convtype (arg->ctx, &ms[arg->i].type, prop->type, srcoff_add (arg->srcoff, prop->offset), arg->control)) < 0)
    {
      arg->error = rc;
      return 0;
    }
    arg->i++;
  }
  return 1;
}

static int structmember_off_cmp (const void *va, const void *vb)
{
  const struct ser_structmember *a = va;
  const struct ser_structmember *b = vb;
  return (a->off == b->off) ? 0 : (a->off < b->off) ? -1 : 1;
}

static int convclass_body (struct convtype_context *ctx, struct ser_type **rt, unsigned label, const struct c_class_s *cls, os_uint32 srcoff, const struct sd_cdrControl *control)
{
  struct convclass_count_arg count_arg;
  struct convclass_convert_arg convert_arg;
  int rc;

  count_arg.n = (cls->extends == NULL) ? 0 : 1;
  c_metaWalk (c_metaObject (cls), (c_metaWalkAction) convclass_count, &count_arg);
  assert (count_arg.n > 0);

  /* For our purposes, a class really is like a struct, but the
   * the meta data is represented very differently.
   */
  if ((*rt = convtype_alloc (&ctx->alloc, TK_STRUCT, label, cls->_parent._parent.size, count_arg.n * sizeof ((*rt)->u.strukt.ms[0]))) == NULL)
    return SD_CDR_OUT_OF_MEMORY;
  (*rt)->u.strukt.n = count_arg.n;

  convert_arg.ctx = ctx;
  convert_arg.st = *rt;
  convert_arg.n = count_arg.n;
  convert_arg.i = 0;
  convert_arg.srcoff = srcoff;
  convert_arg.control = control;
  convert_arg.error = 0;

  /* First field is the parent class (if any) -- so convert that one
   * first.  Then call metaWalk to walk the attributes of the class.
   * The resulting "struct conversion" is unordered in offset, hence
   * the sorting.
   */
  if (cls->extends)
  {
    (*rt)->u.strukt.ms[convert_arg.i].off = 0;
    if ((rc = convclass_body (ctx, &(*rt)->u.strukt.ms[convert_arg.i].type, label, cls->extends, srcoff, control)) < 0)
      return rc;
    convert_arg.i++;
  }
  c_metaWalk (c_metaObject (cls), (c_metaWalkAction) convclass_convert, &convert_arg);
  qsort ((*rt)->u.strukt.ms, (*rt)->u.strukt.n, sizeof ((*rt)->u.strukt.ms[0]), structmember_off_cmp);

  /* collapse substructs */
  if ((rc = convstruct_collapse_substructs (ctx, rt)) < 0)
    return rc;
  if ((rc = convstruct_discover_primarray (ctx, rt)) < 0)
    return rc;
  if ((rc = convstruct_elide_singleton (rt)) < 0)
    return rc;
  return 0;
}

static int convclass (struct convtype_context *ctx, struct ser_type **rt, unsigned label, const struct c_class_s *cls, int quietref, os_uint32 srcoff, const struct sd_cdrControl *control)
{
  int rc;
  if ((*rt = convtype_alloc (&ctx->alloc, TK_CLASS, label, sizeof (void *), 0)) == NULL)
    return SD_CDR_OUT_OF_MEMORY;
  (*rt)->u.class.ospl_type = (const struct c_type_s *) cls;
  (*rt)->u.class.quietref = quietref;
  if ((rc = convclass_body (ctx, &(*rt)->u.class.subtype, label, cls, srcoff, control)) < 0)
    return rc;
  return 0;
}

static int unionmember_cmpdv (const void *va, const void *vb)
{
  const struct ser_unionmember *a = va;
  const struct ser_unionmember *b = vb;
  return (a->dv == b->dv) ? 0 : (a->dv < b->dv) ? -1 : 1;
}

static unsigned long long convunion_discvalue (const struct c_type_s *dtype0, const struct c_literal_s *lab)
{
  const struct c_type_s *dtype = c_typeActualType ((c_type) dtype0);
  switch (c_baseObjectKind ((c_type) dtype))
  {
    case M_PRIMITIVE:
      switch (c_primitiveKind ((c_type) dtype))
      {
        /* All types are treated as unsigned by serializer when
         * reading from source, so should also treat label values as
         * unsigned even when they aren't.
         */
        case P_BOOLEAN: return lab->value.is.Boolean;
        case P_CHAR: return lab->value.is.Octet;
        case P_SHORT: return lab->value.is.UShort;
        case P_USHORT: return lab->value.is.UShort;
        case P_LONG: return lab->value.is.ULong;
        case P_ULONG: return lab->value.is.ULong;
        case P_LONGLONG: return lab->value.is.ULongLong;
        case P_ULONGLONG: return lab->value.is.ULongLong;
        default: abort (); return 0; /* Unsupported type */
      }
    case M_ENUMERATION:
      return lab->value.is.ULong;
    default:
      abort ();
      return 0;
  }
}

static unsigned convunion_countcases (const struct c_union_s *utype)
{
  const unsigned ncases = c_arraySize (((c_union) utype)->cases);
  unsigned i, ntot;
  ntot = 1; /* default case, which we always have */
  for (i = 0; i < ncases; i++)
  {
    struct c_unionCase_s const * const c = c_unionCase (utype->cases[i]);
    const unsigned nlab = c_arraySize (((c_unionCase) c)->labels);
    ntot += nlab;
  }
  return ntot;
}

static int convunion_getcases (struct convtype_context *ctx, int *hasdefault, struct ser_unionmember *ms, const struct c_union_s *utype, const struct sd_cdrControl *control)
{
  const unsigned ncases = c_arraySize (((c_union) utype)->cases);
  unsigned i, idx = 0, defidx = 0;
  int rc;

  /* extract discriminator values, convert types, add default if there was none */
  *hasdefault = 0;
  for (i = 0; i < ncases; i++)
  {
    struct c_unionCase_s const * const c = c_unionCase (utype->cases[i]);
    const unsigned nlab = c_arraySize (((c_unionCase) c)->labels);
    if ((rc = convtype (ctx, &ms[idx].type, c->_parent.type, NOT_A_SRCOFF, control)) < 0)
      return rc;
    if (nlab == 0)
    {
      *hasdefault = 1;
      ms[idx].dv = 0; /* irrelevant */
      defidx = idx;
    }
    else
    {
      unsigned j;
      for (j = 0; j < nlab; j++)
      {
        ms[idx + j].dv = convunion_discvalue (utype->switchType, (const struct c_literal_s *) c->labels[j]);
        ms[idx + j].type = ms[idx].type;
      }
    }
    idx += (nlab == 0) ? 1 : nlab;
  }
  if (! *hasdefault)
  {
    ms[idx].dv = 0; /* irrelevant */
    if ((ms[idx].type = convtype_alloc (&ctx->alloc, TK_NONE, 0, 0, 0)) == NULL)
      return SD_CDR_OUT_OF_MEMORY;
    defidx = idx;
    idx++;
  }

  /* move default to the end if it isn't, then sort on increasing
   * label value (when interpreted as unsigned 64-bit)
   */
  if (defidx != idx - 1)
  {
    struct ser_unionmember tmp = ms[idx - 1];
    ms[idx - 1] = ms[defidx];
    ms[defidx] = tmp;
  }
  qsort (ms, idx - 1, sizeof (*ms), unionmember_cmpdv);
  return (int) idx;
}

static size_t alignup_size_t (size_t x, size_t a)
{
  size_t m = a-1;
  return (x + m) & ~m;
}

static os_address alignup_address (os_address x, os_address a)
{
  os_address m = a-1;
  return (x + m) & ~m;
}

static char *alignup_pchar (const char *x, os_address a)
{
  return (char *) alignup_address ((os_address) x, a);
}

static int convunion_isdense (unsigned n, const struct ser_unionmember *ms)
{
  /* accept ~50% overhead (arbitrarily chosen) */
  unsigned i;
  for (i = 0; i < n; i++)
    if (ms[i].dv > 0xffffffff || ms[i].dv >= 3 * n / 2)
      return 0;
  return 1;
}

static int convunion (struct convtype_context *ctx, struct ser_type **rt, unsigned label, const struct c_union_s *utype, const struct sd_cdrControl *control)
{
  const struct c_type_s *dtype = c_typeActualType ((c_type) utype->switchType);
  unsigned ntot = convunion_countcases (utype);
  int hasdefault, rc;
  assert (ntot > 0);
  if ((*rt = convtype_alloc (&ctx->alloc, TK_UNION_LIST, label, utype->_parent.size, ntot * sizeof ((*rt)->u.union_list.ms[0]))) == NULL)
    return SD_CDR_OUT_OF_MEMORY;
  (*rt)->u.union_list.n = ntot - 1; /* real cases = total cases less default */
  (*rt)->u.union_list.dkind = convprimtype (dtype);
  (*rt)->u.union_list.moff = alignup_size_t (dtype->size, utype->_parent.alignment);
  assert (utype->_parent.size >= (*rt)->u.union_list.moff);
  if ((rc = convunion_getcases (ctx, &hasdefault, (*rt)->u.union_list.ms, utype, control)) < 0)
    return rc;
  assert ((unsigned) rc == ntot);
  (*rt)->u.union_list.hasdefault = !!hasdefault;
  return 0;
}

static int catsstac_match (struct convtype_context *ctx)
{
  if (ctx->catsstac && ctx->catsstac->n == ctx->typestack_depth &&
      memcmp (ctx->catsstac->typestack, ctx->typestack, ctx->typestack_depth * sizeof (*ctx->typestack)) == 0)
  {
    ctx->catsstac = ctx->catsstac->next;
    return 1;
  }
  else
  {
    return 0;
  }
}

static int quietref_match (struct convtype_context *ctx)
{
  if (ctx->quietref && ctx->quietref->n == ctx->typestack_depth &&
      memcmp (ctx->quietref->typestack, ctx->typestack, ctx->typestack_depth * sizeof (*ctx->typestack)) == 0)
  {
    ctx->quietref = ctx->quietref->next;
    return 1;
  }
  else
  {
    return 0;
  }
}

static int isrecursive (unsigned *label, struct convtype_context *ctx, const struct c_type_s *type0)
{
  const struct c_type_s *type = c_typeActualType ((c_type) type0);
  unsigned i;
  for (i = 0; i < ctx->typestack_depth; i++)
    if (ctx->typestack[i] == type)
    {
      *label = ctx->labels[i];
      ctx->recuse[i] = 1;
      return 1;
    }
  return 0;
}

OSPL_DIAG_OFF(conversion)
static int ser_type_set_tag (struct ser_type *rt, unsigned tag)
{
  if (tag > MAX_TAG)
    return SD_CDR_INVALID_TAG;
  rt->has_tag = 1;
  rt->tag = tag;
  return 0;
}
OSPL_DIAG_ON(conversion)

static int convtype (struct convtype_context *ctx, struct ser_type **rt, const struct c_type_s *type0, os_uint32 srcoff, const struct sd_cdrControl *control)
{
  const struct c_type_s *type = c_typeActualType ((c_type) type0);
  const unsigned label = ctx->next_label++;
  int rc;
  ctx->typestack[ctx->typestack_depth] = type;
  ctx->labels[ctx->typestack_depth] = label;
  ctx->recuse[ctx->typestack_depth] = 0;
  ctx->typestack_depth++;
  switch (c_baseObjectKind ((c_type) type))
  {
    case M_PRIMITIVE:
    case M_ENUMERATION:
      {
        enum ser_typekind kind = convprimtype (type);
        size_t width = ser_primwidth (kind);
        os_uint32 tag;
        if ((*rt = convtype_alloc (&ctx->alloc, kind, label, width, 0)) == NULL)
          return SD_CDR_OUT_OF_MEMORY;
        if (control->tag && control->tag (&tag, control->tag_arg, map_tag_type (kind), srcoff))
        {
          if ((rc = ser_type_set_tag (*rt, tag)) < 0)
            return rc;
        }
      }
      break;
    case M_COLLECTION:
      {
        const struct c_collectionType_s *ctype = (const struct c_collectionType_s *) type;
        os_uint32 tag;
        switch (ctype->kind)
        {
          case OSPL_C_STRING:
            if (!catsstac_match (ctx))
            {
              if ((*rt = convtype_alloc (&ctx->alloc, TK_STRING, label, sizeof (char *), 0)) == NULL)
                return SD_CDR_OUT_OF_MEMORY;
              (*rt)->u.string.maxn = ctype->maxSize;
            }
            else
            {
#ifdef PRINTTYPE
              fprintf (stderr, "convtype: CATS\n");
#endif
              assert (ctype->maxSize > 0);
              if ((*rt = convtype_alloc (&ctx->alloc, TK_STRING_TO_ARRAY, label, sizeof (char *), 0)) == NULL)
                return SD_CDR_OUT_OF_MEMORY;
              (*rt)->u.string_to_array.n = ctype->maxSize;
            }
            if (control->tag && control->tag (&tag, control->tag_arg, SD_CDR_TT_STRING, srcoff))
            {
              if ((rc = ser_type_set_tag (*rt, tag)) < 0)
                return rc;
            }
            break;
          case OSPL_C_ARRAY:
          case OSPL_C_SEQUENCE:
            if (isprimtype (ctype->subType))
            {
              struct ser_type *subtype;
              if ((subtype = convtype_alloc (&ctx->alloc, convprimtype (ctype->subType), 0, ctype->subType->size, 0)) == NULL)
                return SD_CDR_OUT_OF_MEMORY;
              if (ctype->kind != OSPL_C_ARRAY || ctype->maxSize == 0)
                rc = mk_sequence (ctx, rt, label, ctype->maxSize, subtype, ctype);
              else if (subtype->kind != TK_PRIM1 || !catsstac_match (ctx))
                rc = mk_array (ctx, rt, label, ctype->maxSize, subtype);
              else
              {
#ifdef PRINTTYPE
                fprintf (stderr, "convtype: STAC\n");
#endif
                assert (ctype->maxSize > 0);
                assert ((c_address) ctype->maxSize == type->size);
                if ((*rt = convtype_alloc (&ctx->alloc, TK_ARRAY_TO_STRING, label, type->size, 0)) == NULL)
                  return SD_CDR_OUT_OF_MEMORY;
                (*rt)->u.array_to_string.n = ctype->maxSize;
                rc = 0;
              }
            }
            else
            {
              struct ser_type *subtype;
              unsigned sublabel;
              if (isrecursive (&sublabel, ctx, ctype->subType))
              {
                assert (ctype->kind == OSPL_C_SEQUENCE || (ctype->kind == OSPL_C_ARRAY && ctype->maxSize == 0));
                if ((*rt = convtype_alloc (&ctx->alloc, TK_RSEQUENCE, label, sizeof (void *), 0)) == NULL)
                  return SD_CDR_OUT_OF_MEMORY;
                (*rt)->u.rsequence.maxn = ctype->maxSize;
                (*rt)->u.rsequence.sublabel = sublabel;
                (*rt)->u.rsequence.ospl_type = ctype;
                rc = 0;
              }
              else
              {
                if ((rc = convtype (ctx, &subtype, ctype->subType, NOT_A_SRCOFF, control)) < 0)
                  return rc;
                if (ctype->kind != OSPL_C_ARRAY || ctype->maxSize == 0)
                  rc = mk_sequence (ctx, rt, label, ctype->maxSize, subtype, ctype);
                else
                  rc = mk_array (ctx, rt, label, ctype->maxSize, subtype);
              }
            }
            if (rc < 0)
              return rc;
            break;
          default:
            abort ();
        }
        break;
      }
    case M_STRUCTURE:
      if ((rc = convstruct (ctx, rt, label, (const struct c_structure_s *) type, srcoff, control)) < 0)
        return rc;
      break;
    case M_UNION:
      if ((rc = convunion (ctx, rt, label, (const struct c_union_s *) type, control)) < 0)
        return rc;
      break;
    case M_CLASS:
      {
        int quietref = quietref_match(ctx);
        if ((rc = convclass (ctx, rt, label, (const struct c_class_s *) type, quietref, srcoff, control)) < 0)
          return rc;
        break;
      }
    default:
      abort ();
  }
  ctx->typestack_depth--;
  (*rt)->recuse = ctx->recuse[ctx->typestack_depth];
  return 0;
}

/****************** SERIALIZED SIZE *******************/

/* Stream alignment is represented as (a,k), where:
 * - stream position = k mod a
 * - a `elem` {1,2,4,8}
 * - k `elem` N && k < a
 *
 * If a < A, where A is the alignment requirement of the next type to be
 * appended to (or read from) the stream, then the stream must be aligned
 * dynamically by moving forward:
 *
 * align A pos = if pos `mod` A == 0 then 0 else A - (pos `mod` A))
 *
 * If a >= A, then the required shift is known statically:
 *
 * if k `mod` A == 0 then 0 else A - (k `mod` A)
 *
 * For arrays and sequences, the alignment steps in the first element may
 * be different from those in the second and further elements: the stream
 * alignment (a,k) for the second and further elements is determined by
 * the one before the first element and the type of the elements, whereas
 * that for the first element is determined by what precedes the array of
 * sequence.
 *
 * merge (a_i,k_i) = (a',k'), where:
 * - a' is the maximum a' <= a_i s.t.
 * - forall i. (k_i mod a_i) mod a' = k' mod a'
 */
struct mmsz_context {
  size_t minsize;
  size_t maxsize;
  int unbounded;
  int depth;
  struct ser_cdralign align;
};

static struct ser_cdralign merge_cdralign (struct ser_cdralign a0, unsigned n, const struct ser_cdralign *as)
{
  /* At all times, all ser_cdralign have 0 <= off < align, so each
   * time we reduce a.align, we recalculate a.off.
   */
  struct ser_cdralign a = a0;
  unsigned i;
  for (i = 0; i < n && a.align > 1; i++)
  {
    if (as[i].align < a.align)
    {
      a.align = as[i].align;
      a.off = a.off % a.align;
    }
    /* a.align <= as[i].align, so a.off and as[i].off may be congruent
     * mod a.align (in which case all is well) or not (in which case
     * we know less about the alignment than a.align); the lazy way is
     * to just do this by trial and error, as we can halve a.align at
     * most 3 times to reach a={.align=1,.off=0}, or nothing known.
     */
    while (a.off != (as[i].off % a.align))
    {
      a.align /= 2;
      a.off = a.off % a.align;
    }
  }
  return a;
}

static void mmsz_calc1 (struct mmsz_context *ctx, const struct ser_type *type);

static void mmsz_update (struct mmsz_context *ctx, size_t addmin, size_t addmax, size_t align)
{
  /* If required alignment higher than we our current basis, re-align
   * (and do so conservatively: e.g., ctx->align=={4,3} and align==8
   * => need 1 or 5 bytes, not something in 0 .. 7), else if the
   * offset makes it misaligned, add a known amount of padding
   */
#ifdef PRINTSIZE
  fprintf (stderr, "mmsz_update: %u,%u [%zu,%zu] %zu => ", ctx->align.align, ctx->align.off, addmin, addmax, align);
#endif
  assert (align <= 0x80000000);
  if (align > ctx->align.align) {
    ctx->align.align = align;
    ctx->align.off = 0;
    ctx->maxsize += align - 1;
#ifdef PRINTSIZE
    fprintf (stderr, "align");
#endif
  } else if ((ctx->align.off % align) != 0) {
    size_t pad = (align - (ctx->align.off % align));
    ctx->align.off = (ctx->align.off + pad) % ctx->align.align;
    ctx->maxsize += pad;
#ifdef PRINTSIZE
    fprintf (stderr, "pad(%u)", pad);
#endif
  } else {
#ifdef PRINTSIZE
    fprintf (stderr, "ok");
#endif
  }
  ctx->minsize += addmin;
  ctx->maxsize += addmax;
  /* If min and max sizes are equal, i.e., a definite size, then we
   * compute the exact effect on the alignment; else we simply assume
   * there is no alignment guarantee beyond "align"
   */
  if (addmin == addmax) {
    ctx->align.off = (ctx->align.off + addmin) % ctx->align.align;
#ifdef PRINTSIZE
    fprintf (stderr, " => %u,%u\n", ctx->align.align, ctx->align.off);
#endif
  } else {
    ctx->align.align = align;
    ctx->align.off = 0;
#ifdef PRINTSIZE
    fprintf (stderr, " => reset => %u,%u\n", ctx->align.align, ctx->align.off);
#endif
  }
}

static void mmsz_unbounded (struct mmsz_context *ctx)
{
  ctx->unbounded = 1;
}

static void mmsz_context_init (struct mmsz_context *ctx, struct ser_cdralign init_align)
{
  ctx->minsize = 0;
  ctx->maxsize = 0;
  ctx->unbounded = 0;
  ctx->depth = 0;
  ctx->align = init_align;
}

static void mmsz_context_update_ctx (struct mmsz_context *ctx, const struct mmsz_context *sub)
{
  ctx->minsize += sub->minsize;
  ctx->maxsize += sub->maxsize;
  if (sub->unbounded)
    ctx->unbounded = 1;
  ctx->align = sub->align;
}

static void mmsz_context_subsume_fixed (struct mmsz_context *ctx, unsigned n, const struct ser_type *subtype)
{
  struct mmsz_context ctx1;
  /* For the 2nd and later array elements, the size is exactly the
   * same (if it is fixed in the first place), and the alignment at
   * the end is also the same, except for the possibility that the
   * offset can be different for N vs N+1 copies.  However, the offset
   * necessarily repeats itself at least every ALIGN copies.  E.g.,
   * array[]{octet} starting with (8,0) results in (8,n mod 8); and
   * array[]{struct{short;octet}} starting with (8,0) toggles between
   * (8,3) and (8,7) as alignment.
   */
  assert (n > 0);
#ifdef PRINTSIZE
  fprintf (stderr, "mmsz_context_subsume_fixed %u\n", n);
#endif
  mmsz_context_init (&ctx1, ctx->align);
  mmsz_calc1 (&ctx1, subtype);
  ctx->minsize += n * ctx1.minsize;
  ctx->maxsize += n * ctx1.maxsize;
  if (ctx1.unbounded)
    ctx->unbounded = 1;
  if (n == 1)
    ctx->align = ctx1.align;
  else
  {
    unsigned i;
    struct mmsz_context ctx2;
    mmsz_context_init (&ctx2, ctx1.align);
    for (i = 1; i < n % ctx1.align.align; i++)
      mmsz_calc1 (&ctx2, subtype);
    ctx->align = ctx2.align;
  }
#ifdef PRINTSIZE
  fprintf (stderr, "mmsz_context_subsume_fixed => %u,%u\n", ctx->align.align, ctx->align.off);
#endif
}

static void mmsz_context_subsume_variable (struct mmsz_context *ctx, unsigned nmax0, const struct ser_type *subtype)
{
  struct mmsz_context ctx1;
  unsigned i, nmax, nxoffs;
#ifdef PRINTSIZE
  fprintf (stderr, "mmsz_context_subsume_variable %u\n", nmax0);
#endif
  if (nmax0 != 0)
    nmax = nmax0;
  else
  {
    /* alignment guaranteed to repeat itself after 8 copies, so
     * unbounded can be treated as 8
     */
    nmax = 8;
  }
  mmsz_context_init (&ctx1, ctx->align);
  mmsz_calc1 (&ctx1, subtype);
  ctx->maxsize += ctx1.maxsize;
  if (ctx1.unbounded)
    ctx->unbounded = 1;
  ctx->align = merge_cdralign (ctx->align, 1, &ctx1.align);
  if (nmax > 1)
  {
    mmsz_context_init (&ctx1, ctx->align);
    mmsz_calc1 (&ctx1, subtype);
    ctx->maxsize += (nmax - 1) * ctx1.maxsize;
    ctx->align = merge_cdralign (ctx->align, 1, &ctx1.align);
  }
  /* Like _fixed, but here we have to allow for the effects of 2, 3,... nmax copies. */
  if (nmax > 2)
  {
    nxoffs = (nmax < 8) ? nmax : 8;
    for (i = 2; i < nxoffs; i++)
    {
      mmsz_calc1 (&ctx1, subtype);
      ctx->align = merge_cdralign (ctx->align, 1, &ctx1.align);
    }
  }
#ifdef PRINTSIZE
  fprintf (stderr, "mmsz_context_subsume_variable => %u,%u\n", ctx->align.align, ctx->align.off);
#endif
}

static void mmsz_context_merge_minmax (struct mmsz_context *ctx, const struct mmsz_context *sub)
{
  if (sub->minsize < ctx->minsize)
    ctx->minsize = sub->minsize;
  if (sub->maxsize > ctx->maxsize)
    ctx->maxsize = sub->maxsize;
  if (sub->unbounded)
    ctx->unbounded = 1;
  ctx->align = merge_cdralign (ctx->align, 1, &sub->align);
}

static void mmsz_calc1 (struct mmsz_context *ctx, const struct ser_type *type)
{
  static const struct ser_type prim4 = { TK_PRIM4, 0, 0, 4, { 0, 0 }, 0, 0, { 0 } };
  static const struct ser_type prim1 = { TK_PRIM1, 0, 0, 1, { 0, 0 }, 0, 0, { 0 } };
  unsigned i;
  ctx->depth++;
  switch (type->kind)
  {
    case TK_NONE:
      break;
    case TK_PRIM1:
      mmsz_update (ctx, 1, 1, 1);
      break;
    case TK_PRIM2:
      mmsz_update (ctx, 2, 2, 2);
      break;
    case TK_PRIM4:
      mmsz_update (ctx, 4, 4, 4);
      break;
    case TK_PRIM8:
      mmsz_update (ctx, 8, 8, 8);
      break;
    case TK_ARRAY:
      {
        assert (type->u.array.n > 1);
        mmsz_calc1 (ctx, type->u.array.subtype);
        if (type->u.array.n > 1)
          mmsz_context_subsume_fixed (ctx, type->u.array.n - 1, type->u.array.subtype);
      }
      break;
    case TK_STRING:
      mmsz_calc1 (ctx, &prim4);
      if (type->u.string.maxn > 0)
        mmsz_update (ctx, 1, type->u.string.maxn + 1, 1);
      else
      {
        /* addmin, addmax have to be different, but when it is
         * unbounded the actual value of addmax is irrelevant
         */
        mmsz_unbounded (ctx);
        mmsz_update (ctx, 1, 0, 1);
      }
      break;
    case TK_ARRAY_TO_STRING:
      mmsz_calc1 (ctx, &prim4);
      mmsz_update (ctx, 1, type->u.array_to_string.n + 1, 1);
      break;
    case TK_STRING_TO_ARRAY:
      mmsz_update (ctx, type->u.string_to_array.n, type->u.string_to_array.n, 1);
      break;
    case TK_SEQUENCE:
      {
        if (type->u.sequence.maxn == 0)
          mmsz_unbounded (ctx);
        mmsz_calc1 (ctx, &prim4);
        mmsz_context_subsume_variable (ctx, type->u.sequence.maxn, type->u.sequence.subtype);
      }
      break;
    case TK_RSEQUENCE:
      mmsz_unbounded (ctx);
      mmsz_calc1 (ctx, &prim4);
      /* drop all alignment info, rather than trying to be precise: at
       * worst we underestimate the minimum serialized size by a few
       * bytes (note that precise alignment tracking is primarily
       * useful for recognising memcpy opportunities, but those don't
       * exist with recursive sequences anyway).
       */
      ctx->align.align = 1;
      ctx->align.off = 0;
      break;
    case TK_STRUCT:
      for (i = 0; i < type->u.strukt.n; i++)
        mmsz_calc1 (ctx, type->u.strukt.ms[i].type);
      break;
    case TK_CLASS:
      if (ctx->depth == 1 || type->u.class.quietref)
      {
        /* class at top level and non-null references serialized simply
           as the struct representing its members */
        mmsz_calc1 (ctx, type->u.class.subtype);
      }
      else
      {
        /* class serialized as-if union switch(bool) { case true: T }
         * (though computed as-if sequence<T,1> with a 8-bit length
         * rather than a 32-bit length)
         */
        mmsz_calc1 (ctx, &prim1);
        mmsz_context_subsume_variable (ctx, 1, type->u.class.subtype);
      }
      break;
    case TK_UNION_LIST:
      {
        struct mmsz_context ctxu;
        struct ser_type dtype;
        dtype.kind = type->u.union_list.dkind;
        mmsz_calc1 (ctx, &dtype);
        mmsz_context_init (&ctxu, ctx->align);
        /* Set ctxu.minsize to something ridiculously large, then let
         * the iteration over the union clauses bring it down
         */
        ctxu.minsize = ~ (size_t) 0;
        for (i = 0; i < type->u.union_list.n + type->u.union_list.hasdefault; i++)
        {
          struct mmsz_context ctx1;
          mmsz_context_init (&ctx1, ctxu.align);
          mmsz_calc1 (&ctx1, type->u.union_list.ms[i].type);
          mmsz_context_merge_minmax (&ctxu, &ctx1);
        }
        mmsz_context_update_ctx (ctx, &ctxu);
      }
      break;
  }
  ctx->depth--;
}

static void mmsz_calc (size_t *minsize, size_t *maxsize, const struct ser_type *type)
{
  struct ser_cdralign init_align = { 8, 0 };
  struct mmsz_context ctx;
  mmsz_context_init (&ctx, init_align);
  mmsz_calc1 (&ctx, type);
#ifdef PRINTSIZE
  fprintf (stderr, "maxsize: min %lu max %lu unbounded %d align %u,%u\n", (unsigned long) ctx.minsize, (unsigned long) ctx.maxsize, ctx.unbounded, ctx.align.align, ctx.align.off);
#endif
  *minsize = ctx.minsize;
  *maxsize = ctx.unbounded ? 0 : ctx.maxsize;
}

static const char *kindstr (enum ser_typekind kind)
{
  switch (kind)
  {
    case TK_NONE: return "none";
    case TK_PRIM1: return "prim1";
    case TK_PRIM2: return "prim2";
    case TK_PRIM4: return "prim4";
    case TK_PRIM8: return "prim8";
    case TK_ARRAY: return "array";
    case TK_STRING: return "string";
    case TK_STRING_TO_ARRAY: return "string_to_array";
    case TK_ARRAY_TO_STRING: return "array_to_string";
    case TK_SEQUENCE: return "sequence";
    case TK_RSEQUENCE: return "rsequence";
    case TK_STRUCT: return "struct";
    case TK_UNION_LIST: return "union/list";
    case TK_CLASS: return "class";
    default: return "?";
  }
}

static void printtype1 (FILE *fp, int indent, const struct ser_type *type)
{
  unsigned i;

  fprintf (fp, "%*.*s%s srcsz=%lu", indent, indent, "", kindstr (type->kind), (unsigned long) type->width);
  if (type->has_tag)
    fprintf (fp, " tag=%u", type->tag);
  if (type->recuse)
    fprintf (fp, " label=%u", type->label);
  switch (type->kind)
  {
    case TK_NONE:
    case TK_PRIM1:
    case TK_PRIM2:
    case TK_PRIM4:
    case TK_PRIM8:
      fprintf (fp, "\n");
      break;
    case TK_ARRAY:
      fprintf (fp, " n=%u\n", type->u.array.n);
      printtype1 (fp, indent+2, type->u.array.subtype);
      break;
    case TK_STRING:
      fprintf (fp, " maxn=%u\n", type->u.sequence.maxn);
      break;
    case TK_STRING_TO_ARRAY:
      fprintf (fp, " n=%u\n", type->u.string_to_array.n);
      break;
    case TK_ARRAY_TO_STRING:
      fprintf (fp, " n=%u\n", type->u.array_to_string.n);
      break;
    case TK_SEQUENCE:
      fprintf (fp, " maxn=%u\n", type->u.sequence.maxn);
      printtype1 (fp, indent+2, type->u.sequence.subtype);
      break;
    case TK_RSEQUENCE:
      fprintf (fp, " maxn=%u sublabel=%u\n", type->u.rsequence.maxn, type->u.rsequence.sublabel);
      break;
    case TK_STRUCT:
      fprintf (fp, " n=%u\n", type->u.strukt.n);
      indent += 2;
      for (i = 0; i < type->u.strukt.n; i++)
      {
        fprintf (fp, "%*.*soffset %lu\n", indent, indent, "", (unsigned long) type->u.strukt.ms[i].off);
        printtype1 (fp, indent+2, type->u.strukt.ms[i].type);
      }
      break;
    case TK_UNION_LIST:
      fprintf (fp, " n=%u moff=%lu dkind=%s\n", type->u.union_list.n, (unsigned long) type->u.union_list.moff, kindstr (type->u.union_list.dkind));
      indent += 2;
      for (i = 0; i < type->u.union_list.n; i++)
      {
        fprintf (fp, "%*.*scase %llu:\n", indent, indent, "", type->u.union_list.ms[i].dv);
        printtype1 (fp, indent+2, type->u.union_list.ms[i].type);
      }
      if (type->u.union_list.hasdefault) {
        fprintf (fp, "%*.*sdefault:\n", indent, indent, "");
        printtype1 (fp, indent+2, type->u.union_list.ms[i].type);
      }
      break;
    case TK_CLASS:
      fprintf (fp, "%s\n", type->u.class.quietref ? " quietref" : "");
      printtype1 (fp, indent+2, type->u.class.subtype);
      break;
  }
}

static void printtype (FILE *fp, const struct ser_type *type)
{
  printtype1 (fp, 0, type);
}

/* Undocumented interface for debugging */
void sd_cdrPrintType (FILE *fp, const struct ser_type *type)
{
  printtype (fp, type);
}

/******************** LOWERING ********************/

struct insn_dispatch_list4 {
  unsigned dv;
  unsigned off;
};

struct insn_dispatch_list8 {
  unsigned long long dv;
  unsigned off;
};

struct insn_dispatch_direct {
  unsigned off;
};

enum insn_opcode {
  INSN_DONE,
  INSN_SRCADV,
  INSN_PRIM1,
  INSN_PRIM2,
  INSN_PRIM4,
  INSN_PRIM8,
  INSN_STRING,
  INSN_BSTRING, /* followed by: maxn */
  INSN_PRIM1_POPSRC,
  INSN_PRIM2_POPSRC,
  INSN_PRIM4_POPSRC,
  INSN_PRIM8_POPSRC,
  INSN_STRING_POPSRC,
  INSN_PRIM1_LOOP,
  INSN_PRIM2_LOOP,
  INSN_PRIM4_LOOP,
  INSN_PRIM8_LOOP,
  INSN_POPSRC,
  INSN_PUSHCOUNT,
  INSN_PUSHSTAR, /* followed by: maxn, c_type pointer; insn.count is jump relative to opcode not including c_type pointer */
  INSN_LOOP,
  INSN_LOOPSTAR,
  INSN_PRIM1_LOOPSTAR,
  INSN_PRIM2_LOOPSTAR,
  INSN_PRIM4_LOOPSTAR,
  INSN_PRIM8_LOOPSTAR,
  INSN_PRIM1_DISPATCH_LIST, /* all dispatch followed by srcadv + table */
  INSN_PRIM2_DISPATCH_LIST,
  INSN_PRIM4_DISPATCH_LIST,
  INSN_PRIM8_DISPATCH_LIST,
  INSN_PRIM1_DISPATCH_DIRECT,
  INSN_PRIM2_DISPATCH_DIRECT,
  INSN_PRIM4_DISPATCH_DIRECT,
  INSN_PRIM8_DISPATCH_DIRECT,
  INSN_STRING_TO_ARRAY,
  INSN_ARRAY_TO_STRING,
  INSN_PRIM1_CONST,
  INSN_JUMP, /* invoke code at a higher address */
  INSN_CALL, /* invoke code at a lower address */
  INSN_REF_UNIQ, /* followed by c_type pointer for object type */
  INSN_QUIETREF,
  INSN_RETURN,
  INSN_TAG_PREP, /* insn.count = dstalign */
  INSN_TAG /* insn.count = tag */
};

struct insn_enc {
  unsigned opcode: 6; /* see enum insn_opcode */
  unsigned srcadv: 3;
  unsigned count: 23;
};

struct insnstream_marker {
  size_t pos;
};

struct insnstream {
  size_t pos; /* pos in stream, units of 4 bytes */
  size_t size; /* size of stream, units of 4 bytes */
  unsigned srcpos_tos;
  os_size_t srcpos_stk[128];
  int last_is_insn;
  int error;
  char *buf; /* output buffer */
};

struct serprog {
  c_base base; /* for c_stringMalloc */
  c_type ospl_type; /* for deserializing objects */
  const struct sd_cdrControl *control;
  size_t size;
  char buf[1];
};

struct lowertype_context {
  struct insnstream *st;
  unsigned depth;
  unsigned typestack_depth;
  unsigned labels[128];
  struct insnstream_marker markers[128];
};

static int lowertype1 (struct lowertype_context *ctx, const struct ser_type *type);

OSPL_DIAG_OFF(conversion)
static void insn_set_opcode (struct insn_enc *insn, enum insn_opcode opcode)
{
  insn->opcode = (unsigned) opcode;
}

static void insn_set_srcadv (struct insn_enc *insn, unsigned srcadv)
{
  assert(srcadv < (1u << 3));
  insn->srcadv = srcadv;
}

static void insn_set_count (struct insn_enc *insn, size_t count)
{
  assert(count < (1u << 23));
  insn->count = count;
}
OSPL_DIAG_ON(conversion)

static struct insnstream *is_new (void)
{
  struct insnstream *st;
  st = os_malloc (sizeof (*st));
  st->pos = 0;
  st->size = 1024;
  st->srcpos_tos = 0;
  st->srcpos_stk[st->srcpos_tos] = 0;
  st->last_is_insn = 0;
  st->error = 0;
  st->buf = os_malloc (4 * st->size);
  return st;
}

static void is_free (struct insnstream *st)
{
  os_free (st->buf);
  os_free (st);
}

static struct serprog *is_program (const struct insnstream *st, c_type objtype, const struct sd_cdrControl *control)
{
  struct serprog *p;
  assert (st->srcpos_tos == 0);
  if ((p = os_malloc (offsetof (struct serprog, buf) + 4 * st->pos)) == NULL)
    return NULL;
  p->base = c_getBase (objtype);
  p->ospl_type = objtype;
  p->control = control;
  p->size = st->pos;
  memcpy (p->buf, st->buf, 4 * st->pos);
  return p;
}

static int is_maybe_resize (struct insnstream *st, size_t nbytes)
{
  assert ((nbytes % 4) == 0);
  if (st->pos + nbytes / 4 > st->size)
  {
    st->size = (unsigned) alignup_size_t (st->pos + nbytes / 4, 1024);
    st->buf = os_realloc (st->buf, 4 * st->size);
  }
  return 0;
}

static void is_barrier (struct insnstream *st)
{
  st->last_is_insn = 0;
}

static void is_mark_off (struct insnstream_marker *m, struct insnstream *st, int off)
{
  m->pos = (off >= 0) ? (st->pos + (size_t) off) : (st->pos - (size_t) -off);
}

static void is_mark (struct insnstream_marker *m, struct insnstream *st)
{
  is_barrier (st);
  is_mark_off (m, st, 0);
}

static unsigned is_mark_pos (struct insnstream_marker m)
{
  return (unsigned) m.pos;
}

static int is_reserve (struct insnstream_marker *m, struct insnstream *st, size_t nbytes)
{
  is_mark (m, st);
  st->pos += nbytes / 4;
  return is_maybe_resize (st, 0);
}

static void is_patch (struct insnstream *st, struct insnstream_marker m, size_t off, size_t nbytes, const void *data)
{
  assert ((off % 4) == 0);
  assert ((nbytes % 4) == 0);
  assert (m.pos + off / 4 + nbytes / 4 <= st->size);
  memcpy (st->buf + 4 * m.pos + off, data, nbytes);
}

static char *is_append (struct insnstream *st, size_t nbytes)
{
  char *p;
  assert ((nbytes % 4) == 0);
  if (is_maybe_resize (st, nbytes) < 0)
    return NULL;
  p = st->buf + 4 * st->pos;
  st->pos += nbytes / 4;
  return p;
}

static void is_adv_srcpos (struct insnstream *st, os_size_t adv)
{
  st->srcpos_stk[st->srcpos_tos] += adv;
}

static unsigned maybe_subsume_srcadv (struct insnstream *st)
{
  struct insn_enc *px;
  if (!st->last_is_insn)
    return 0;
  assert (st->pos > 0);
  px = (struct insn_enc *) st->buf + (st->pos - 1);
  if (px->opcode != INSN_SRCADV || px->count >= 8)
    return 0;
  st->pos--;
  return px->count;
}

static int is_append_count (struct insnstream *st, unsigned srcadv, enum insn_opcode opcode, os_size_t count)
{
  struct insn_enc *px = (struct insn_enc *) st->buf + (st->pos - 1);
  if (st->last_is_insn && opcode == INSN_SRCADV && px->opcode == INSN_SRCADV)
  {
    insn_set_count (px, px->count + count);
  }
  else
  {
    struct insn_enc x;
    char *p;
    insn_set_opcode (&x, opcode);
    insn_set_srcadv (&x, maybe_subsume_srcadv (st));
    insn_set_count (&x, count);
    assert (x.srcadv == 0 || opcode != INSN_SRCADV);
    if ((p = is_append (st, sizeof (x))) == NULL)
      return SD_CDR_OUT_OF_MEMORY;
    memcpy (p, &x, sizeof (x));
    is_adv_srcpos (st, srcadv);
    st->last_is_insn = 1;
  }
  return 0;
}

static int is_append_literal (struct insnstream *st, unsigned value)
{
  char *p;
  if ((p = is_append (st, sizeof (value))) == NULL)
    return SD_CDR_OUT_OF_MEMORY;
  memcpy (p, &value, sizeof (value));
  st->last_is_insn = 0;
  return 0;
}

static int is_append_pointer (struct insnstream *st, const void *ptr)
{
  char *p;
  if ((p = is_append (st, sizeof (void *))) == NULL)
    return SD_CDR_OUT_OF_MEMORY;
  memcpy (p, &ptr, sizeof (ptr));
  st->last_is_insn = 0;
  return 0;
}

static int is_append_0 (struct insnstream *st, unsigned srcadv, enum insn_opcode opcode)
{
  return is_append_count (st, srcadv, opcode, 0);
}

static int can_set_popsrc (enum insn_opcode opcode)
{
  switch (opcode)
  {
    case INSN_PRIM1:
    case INSN_PRIM2:
    case INSN_PRIM4:
    case INSN_PRIM8:
    case INSN_STRING:
      return 1;
    default:
      return 0;
  }
}

static enum insn_opcode set_popsrc (enum insn_opcode opcode)
{
  switch (opcode)
  {
    case INSN_PRIM1: return INSN_PRIM1_POPSRC;
    case INSN_PRIM2: return INSN_PRIM2_POPSRC;
    case INSN_PRIM4: return INSN_PRIM4_POPSRC;
    case INSN_PRIM8: return INSN_PRIM8_POPSRC;
    case INSN_STRING: return INSN_STRING_POPSRC;
    default: abort (); return (enum insn_opcode) 0;
  }
}

static int insn_isnop (const struct insn_enc *xs)
{
  return xs->opcode == INSN_SRCADV && xs->srcadv == 0 && xs->count == 0;
}

static unsigned adjust_dest_for_pop (struct insnstream *st, unsigned dest)
{
  struct insn_enc *xs = (struct insn_enc *) st->buf;
  while (insn_isnop (&xs[dest]))
    dest++;
  return dest;
}

static void is_make_jumps_relative (struct insnstream *st)
{
  struct insn_enc *xs = (struct insn_enc *) st->buf;
  unsigned i, pos = 0;
  while (pos < st->pos)
  {
    switch (xs[pos].opcode)
    {
      case INSN_PRIM1_POPSRC:
      case INSN_PRIM2_POPSRC:
      case INSN_PRIM4_POPSRC:
      case INSN_PRIM8_POPSRC:
      case INSN_STRING_POPSRC:
        insn_set_count (&xs[pos], adjust_dest_for_pop (st, xs[pos].count) - (pos+1));
        break;
      case INSN_BSTRING:
        pos += 1;
        break;
      case INSN_POPSRC:
      case INSN_JUMP:
        assert (xs[pos].count > pos);
        insn_set_count (&xs[pos], adjust_dest_for_pop (st, xs[pos].count) - (pos+1));
        break;
      case INSN_PUSHSTAR:
        assert (xs[pos].count > pos);
        insn_set_count (&xs[pos], adjust_dest_for_pop (st, xs[pos].count) - (pos+1));
        pos += 1 + (unsigned) (sizeof (void *) / 4);
        break;
      case INSN_CALL:
      case INSN_LOOP:
      case INSN_LOOPSTAR:
        assert (xs[pos].count < pos);
        insn_set_count (&xs[pos], (pos+1) - adjust_dest_for_pop (st, xs[pos].count));
        break;
      case INSN_PRIM1_DISPATCH_LIST:
      case INSN_PRIM2_DISPATCH_LIST:
      case INSN_PRIM4_DISPATCH_LIST:
        {
          struct insn_dispatch_list4 *dl =
            (struct insn_dispatch_list4 *) (st->buf + 4 * (pos + 2));
          for (i = 0; i <= xs[pos].count; i++)
            dl[i].off = adjust_dest_for_pop (st, dl[i].off) - (pos+1);
          pos += 1 + i * (unsigned) (sizeof (*dl) / 4);
          break;
        }
      case INSN_PRIM8_DISPATCH_LIST:
        {
          struct insn_dispatch_list8 *dl =
            (struct insn_dispatch_list8 *) (st->buf + 4 * (pos + 2));
          for (i = 0; i <= xs[pos].count; i++)
            dl[i].off = adjust_dest_for_pop (st, dl[i].off) - (pos+1);
          pos += 1 + i * (unsigned) (sizeof (*dl) / 4);
          break;
        }
      case INSN_PRIM1_DISPATCH_DIRECT:
      case INSN_PRIM2_DISPATCH_DIRECT:
      case INSN_PRIM4_DISPATCH_DIRECT:
      case INSN_PRIM8_DISPATCH_DIRECT:
        {
          struct insn_dispatch_direct *dl =
            (struct insn_dispatch_direct *) (st->buf + 4 * (pos + 2));
          for (i = 0; i <= xs[pos].count; i++)
            dl[i].off = adjust_dest_for_pop (st, dl[i].off) - (pos+1);
          pos += 1 + i * (unsigned) (sizeof (*dl) / 4);
          break;
        }
      case INSN_REF_UNIQ:
        assert (xs[pos].count > pos);
        insn_set_count (&xs[pos], adjust_dest_for_pop (st, xs[pos].count) - (pos+1));
        pos += (unsigned) (sizeof (void *) / 4);
        break;
      default:
        break;
    }
    pos++;
  }
}

static int is_append_marker (struct insnstream *st, enum insn_opcode opcode, struct insnstream_marker m)
{
  return is_append_count (st, 0, opcode, m.pos);
}

static int is_append_marker_placeholder (struct insnstream_marker *m, struct insnstream *st, enum insn_opcode opcode)
{
  struct insn_enc *x = (struct insn_enc *) st->buf + st->pos;
  int rc;
  if (opcode == INSN_POPSRC && st->last_is_insn && can_set_popsrc (x[-1].opcode))
  {
    insn_set_opcode (&x[-1], set_popsrc (x[-1].opcode));
    is_mark_off (m, st, -1);
    return 0;
  }
  else if ((rc = is_append_count (st, 0, opcode, 0)) < 0)
  {
    return rc;
  }
  else
  {
    is_mark_off (m, st, -1);
    return 0;
  }
}

static void is_patch_marker_placeholder (struct insnstream *st, struct insnstream_marker m, struct insnstream_marker m1)
{
  struct insn_enc *x = (struct insn_enc *) st->buf + m.pos;
  assert (x->count == 0);
  insn_set_count (x, m1.pos);
}

static int is_append_srcadv (struct insnstream *st, os_size_t srcadv)
{
  assert (st->pos > 0 || srcadv == 0);
  assert (srcadv < 8);
  if (srcadv > 0)
    return is_append_count (st, (unsigned) srcadv, INSN_SRCADV, (unsigned) srcadv);
  else
    return 0;
}

static os_size_t is_push_srcpos (struct insnstream *st)
{
  os_size_t pos;
  assert (st->srcpos_tos + 1 < sizeof (st->srcpos_stk) / sizeof (*st->srcpos_stk));
  pos = st->srcpos_stk[st->srcpos_tos];
  st->srcpos_stk[++st->srcpos_tos] = 0;
  return pos;
}

static os_size_t is_pop_srcpos (struct insnstream *st)
{
  assert (st->srcpos_tos > 0);
  return st->srcpos_stk[st->srcpos_tos--];
}

static os_size_t is_get_srcpos (struct insnstream *st)
{
  return st->srcpos_stk[st->srcpos_tos];
}

static int is_align2 (struct insnstream *st)
{
  int rc;
  if ((st->pos % 2) == 1)
  {
    struct insn_enc *x = (struct insn_enc *) st->buf + st->pos - 1;
    if (st->last_is_insn && x->opcode == INSN_SRCADV && x->count < 8) {
      /* skip: next instruction will be merged */
    } else {
      if ((rc = is_append_count (st, 0, INSN_SRCADV, 0)) < 0)
        return rc;
      is_barrier (st);
    }
  }
  else
  {
    struct insn_enc *x = (struct insn_enc *) st->buf + st->pos - 1;
    if (st->last_is_insn && x->opcode == INSN_SRCADV && x->count < 8) {
      struct insn_enc xcopy = *x;
      /* next instruction will be merged if we don't take measures (but we want it merged!) */
      x->count = 0;
      is_barrier (st);
      if ((rc = is_append_count (st, 0, INSN_SRCADV, xcopy.count)) < 0)
        return rc;
    }
  }
  return 0;
}

static int lowertype1_multiple (struct lowertype_context *ctx, const struct ser_type *subtype)
{
  os_size_t srcpos0, srcadv;
  int rc;
  srcpos0 = is_push_srcpos (ctx->st);
  if ((rc = lowertype1 (ctx, subtype)) < 0)
    return rc;
  srcadv = is_pop_srcpos (ctx->st) - srcpos0;
  assert (srcadv <= subtype->width);
  return is_append_srcadv (ctx->st, subtype->width - srcadv);
}

static int lowertype1 (struct lowertype_context *ctx, const struct ser_type *type)
{
  /* PRE: source positioned at start of type; srcadv tracking is
   * entirely internal to lowering function, VM always advances source
   * for every primitive converted, as well as for every srcadvance
   * instruction
   */
  struct insnstream *st = ctx->st;
  struct insnstream_marker mjump = { 0 };
  os_size_t srcpos_before, srcpos_after;
  unsigned i;
  int rc;

  /* If recursively used, we "call" the code as if it were a function,
   * rather than jump into it. There's no need for a function prologue
   * in the code (we do need an epilogue: INSN_RETURN), but we do need
   * to insert a INSN_CALL.  Note that we never call lowertype1 for a
   * recursive use of the type, only for the first use (thanks to the
   * simple-mindedness of IDL type recursion).
   */
  if (type->recuse)
  {
    if ((rc = is_append_marker_placeholder (&mjump, st, INSN_JUMP)) < 0)
      return rc;
    ctx->labels[ctx->typestack_depth] = type->label;
    is_mark (&ctx->markers[ctx->typestack_depth], st);
    ctx->typestack_depth++;
  }
  ctx->depth++;

  srcpos_before = is_get_srcpos (st);
  if (type->has_tag)
  {
    os_size_t align;
    assert (kind_is_primN (type->kind) || type->kind == TK_STRING);
    align = kind_is_primN (type->kind) ? type->width : (type->kind == TK_STRING) ? 4u : 1u;
    if ((rc = is_append_count (st, 0, INSN_TAG_PREP, align)) < 0)
      return rc;
  }
  switch (type->kind)
  {
    case TK_NONE:
      break;
    case TK_PRIM1:
      if (is_append_0 (st, 1, INSN_PRIM1) < 0)
        return SD_CDR_OUT_OF_MEMORY;
      break;
    case TK_PRIM2:
      if (is_append_0 (st, 2, INSN_PRIM2) < 0)
        return SD_CDR_OUT_OF_MEMORY;
      break;
    case TK_PRIM4:
      if (is_append_0 (st, 4, INSN_PRIM4) < 0)
        return SD_CDR_OUT_OF_MEMORY;
      break;
    case TK_PRIM8:
      if (is_append_0 (st, 8, INSN_PRIM8) < 0)
        return SD_CDR_OUT_OF_MEMORY;
      break;
    case TK_ARRAY:
      if (kind_is_primN (type->u.array.subtype->kind))
      {
        unsigned n = type->u.array.n;
        switch (type->u.array.subtype->kind)
        {
          case TK_PRIM1: rc = is_append_count (st, 1 * n, INSN_PRIM1_LOOP, n); break;
          case TK_PRIM2: rc = is_append_count (st, 2 * n, INSN_PRIM2_LOOP, n); break;
          case TK_PRIM4: rc = is_append_count (st, 4 * n, INSN_PRIM4_LOOP, n); break;
          case TK_PRIM8: rc = is_append_count (st, 8 * n, INSN_PRIM8_LOOP, n); break;
          default: abort (); rc = SD_CDR_OUT_OF_MEMORY;
        }
        if (rc < 0)
          return rc;
      }
      else
      {
        struct insnstream_marker m;
        if ((rc = is_append_count (st, 0, INSN_PUSHCOUNT, type->u.array.n)) < 0)
          return rc;
        is_push_srcpos (st);
        is_mark (&m, st);
        if ((rc = lowertype1_multiple (ctx, type->u.array.subtype)) < 0)
          return rc;
        if ((rc = is_append_marker (st, INSN_LOOP, m)) < 0)
          return rc;
        is_pop_srcpos (st);
        is_adv_srcpos (st, type->u.array.n * type->u.array.subtype->width);
      }
      break;
    case TK_STRING:
      if (type->u.string.maxn == 0 || type->u.string.maxn == ~(os_uint32)0)
      {
        if ((rc = is_append_0 (st, sizeof (void *), INSN_STRING)) < 0)
          return rc;
      }
      else
      {
        if ((rc = is_append_0 (st, sizeof (void *), INSN_BSTRING)) < 0)
          return rc;
        if ((rc = is_append_literal (st, type->u.string.maxn + 1)) < 0)
          return rc;
      }
      break;
    case TK_STRING_TO_ARRAY:
      if ((rc = is_append_count (st, sizeof (char *), INSN_STRING_TO_ARRAY, type->u.string_to_array.n)) < 0)
        return rc;
      break;
    case TK_ARRAY_TO_STRING:
      if ((rc = is_append_count (st, type->u.array_to_string.n, INSN_ARRAY_TO_STRING, type->u.array_to_string.n)) < 0)
        return rc;
      break;
    case TK_SEQUENCE:
      {
        /* Type uses 0 to encode maxn, but CDR has a hard limit of
         * 2**32-1 on sequence lengths because it encodes them as
         * 32-bit unsigned.  Transforming unbounded to 2**32-1 means
         * we need only a single check when deserialising
         */
        const os_uint32 maxn = (type->u.sequence.maxn == 0) ? ~(os_uint32)0 : type->u.sequence.maxn;
        struct insnstream_marker mpush, m;
        if ((rc = is_append_marker_placeholder (&mpush, st, INSN_PUSHSTAR)) < 0)
          return rc;
        if ((rc = is_append_literal (st, maxn)) < 0)
          return rc;
        if ((rc = is_append_pointer (st, type->u.sequence.ospl_type)) < 0)
          return rc;
        is_adv_srcpos (st, sizeof (void *));
        is_push_srcpos (st);
        if (kind_is_primN (type->u.sequence.subtype->kind))
        {
          switch (type->u.sequence.subtype->kind)
          {
            case TK_PRIM1: rc = is_append_0 (st, 0, INSN_PRIM1_LOOPSTAR); break;
            case TK_PRIM2: rc = is_append_0 (st, 0, INSN_PRIM2_LOOPSTAR); break;
            case TK_PRIM4: rc = is_append_0 (st, 0, INSN_PRIM4_LOOPSTAR); break;
            case TK_PRIM8: rc = is_append_0 (st, 0, INSN_PRIM8_LOOPSTAR); break;
            default: abort (); rc = SD_CDR_OUT_OF_MEMORY;
          }
          if (rc < 0)
            return rc;
        }
        else
        {
          is_mark (&m, st);
          if ((rc = lowertype1_multiple (ctx, type->u.sequence.subtype)) < 0)
            return rc;
          if ((rc = is_append_marker (st, INSN_LOOPSTAR, m)) < 0)
            return rc;
        }
        is_mark (&m, st);
        is_patch_marker_placeholder (st, mpush, m);
        is_pop_srcpos (st);
      }
      break;
    case TK_RSEQUENCE:
      {
        /* Type uses 0 to encode maxn, but CDR has a hard limit of
         * 2**32-1 on sequence lengths because it encodes them as
         * 32-bit unsigned.  Transforming unbounded to 2**32-1 means
         * we need only a single check when deserialising
         */
        const os_uint32 maxn = (type->u.sequence.maxn == 0) ? ~(os_uint32)0 : type->u.sequence.maxn;
        /* label is in context, along with the instruction marker */
        struct insnstream_marker mpush, m;
        unsigned j;
        for (j = 0; j < ctx->typestack_depth; j++)
          if (ctx->labels[j] == type->u.rsequence.sublabel)
            break;
        assert (j < ctx->typestack_depth);
        if ((rc = is_append_marker_placeholder (&mpush, st, INSN_PUSHSTAR)) < 0)
          return rc;
        if ((rc = is_append_literal (st, maxn)) < 0)
          return rc;
        if ((rc = is_append_pointer (st, type->u.rsequence.ospl_type)) < 0)
          return rc;
        is_adv_srcpos (st, sizeof (void *));
        is_push_srcpos (st);
        is_mark (&m, st);
        if ((rc = is_append_marker (st, INSN_CALL, ctx->markers[j])) < 0)
          return rc;
        if ((rc = is_append_marker (st, INSN_LOOPSTAR, m)) < 0)
          return rc;
        is_mark (&m, st);
        is_patch_marker_placeholder (st, mpush, m);
        is_pop_srcpos (st);
      }
      break;
    case TK_STRUCT:
      for (i = 0; i < type->u.strukt.n; i++)
      {
        os_size_t srcpos = is_get_srcpos (st);
        assert (srcpos <= type->u.strukt.ms[i].off);
        if ((rc = is_append_srcadv (st, (unsigned) type->u.strukt.ms[i].off - srcpos)) < 0)
          return rc;
        if ((rc = lowertype1 (ctx, type->u.strukt.ms[i].type)) < 0)
          return rc;
      }
      break;
    case TK_UNION_LIST:
      {
        struct insnstream_marker mtab, *ms, m;
        size_t discsz = 0;
        ms = os_malloc ((type->u.union_list.n+1) * sizeof (*ms));
        is_push_srcpos (st);
        if (convunion_isdense (type->u.union_list.n, type->u.union_list.ms))
        {
          const unsigned max = (unsigned) type->u.union_list.ms[type->u.union_list.n-1].dv;
          const unsigned count = max + 1;
          struct insn_dispatch_direct d;
          unsigned j;
          switch (type->u.union_list.dkind)
          {
            case TK_PRIM1: rc = is_append_count (st, 1, INSN_PRIM1_DISPATCH_DIRECT, count); discsz = 1; break;
            case TK_PRIM2: rc = is_append_count (st, 2, INSN_PRIM2_DISPATCH_DIRECT, count); discsz = 2; break;
            case TK_PRIM4: rc = is_append_count (st, 4, INSN_PRIM4_DISPATCH_DIRECT, count); discsz = 4; break;
            case TK_PRIM8: rc = is_append_count (st, 8, INSN_PRIM8_DISPATCH_DIRECT, count); discsz = 8; break;
            default: abort (); rc = SD_CDR_OUT_OF_MEMORY;
          }
          if (rc < 0)
            goto union_return_rc;
          if ((rc = is_append_literal (st, (unsigned) type->width)) < 0)
            goto union_return_rc;
          if ((rc = is_reserve (&mtab, st, (count+1) * sizeof (struct insn_dispatch_direct))) < 0)
            goto union_return_rc;
          for (i = 0; i <= type->u.union_list.n; i++)
          {
            unsigned idx;
            if (i < type->u.union_list.n)
              idx = (unsigned) type->u.union_list.ms[i].dv;
            else
              idx = count;
            is_mark (&m, st);
            d.off = is_mark_pos (m);
            is_patch (st, mtab, idx * sizeof (d), sizeof (d), &d);
            if ((rc = is_append_srcadv (st, (unsigned) (type->u.union_list.moff - discsz))) < 0)
              goto union_return_rc;
            /* push_srcpos this late so that recursive call sees
             * offset 0; at the end of the union we pop the source pos
             * and advance it beyond the union, so there is no harm in
             * the successive append_srcadv's here.
             */
            is_push_srcpos (st);
            if ((rc = lowertype1 (ctx, type->u.union_list.ms[i].type)) < 0)
              goto union_return_rc;
            is_pop_srcpos (st);
            if ((rc = is_append_marker_placeholder (&ms[i], st, INSN_POPSRC)) < 0)
              goto union_return_rc;
          }
          /* patch holes, default */
          d.off = is_mark_pos (ms[type->u.union_list.n]);
          for (j = i = 0; i < type->u.union_list.n; i++, j++)
            while (j < type->u.union_list.ms[i].dv)
              is_patch (st, mtab, j++ * sizeof (d), sizeof (d), &d);
          is_patch (st, mtab, j * sizeof (d), sizeof (d), &d);
        }
        else
        {
          const unsigned count = type->u.union_list.n;
          const unsigned dl_size =
            (type->u.union_list.dkind == TK_PRIM8)
            ? sizeof (struct insn_dispatch_list8) : sizeof (struct insn_dispatch_list4);
          switch (type->u.union_list.dkind)
          {
            case TK_PRIM1:
              if ((rc = is_append_count (st, 1, INSN_PRIM1_DISPATCH_LIST, count)) < 0)
                goto union_return_rc;
              discsz = 1;
              break;
            case TK_PRIM2:
              if ((rc = is_append_count (st, 2, INSN_PRIM2_DISPATCH_LIST, count)) < 0)
                goto union_return_rc;
              discsz = 2;
              break;
            case TK_PRIM4:
              if ((rc = is_append_count (st, 4, INSN_PRIM4_DISPATCH_LIST, count)) < 0)
                goto union_return_rc;
              discsz = 4;
              break;
            case TK_PRIM8:
              /* insn_dispatch_list8 uses prim8, and therefore has to
               * be 8-byte aligned.  That means the dispatch
               * instruction itself must be on an even position
               */
              if ((rc = is_align2 (st)) < 0)
                goto union_return_rc;
              if ((rc = is_append_count (st, 8, INSN_PRIM8_DISPATCH_LIST, count)) < 0)
                goto union_return_rc;
              discsz = 8;
              break;
            default:
              abort ();
          }
          if ((rc = is_append_literal (st, (unsigned) type->width)) < 0)
            goto union_return_rc;
          if ((rc = is_reserve (&mtab, st, (type->u.union_list.n+1) * dl_size)) < 0)
            goto union_return_rc;
          for (i = 0; i <= type->u.union_list.n; i++)
          {
            is_mark (&m, st);
            if (type->u.union_list.dkind != TK_PRIM8)
            {
              struct insn_dispatch_list4 d;
              d.dv = (unsigned) type->u.union_list.ms[i].dv;
              d.off = is_mark_pos (m);
              is_patch (st, mtab, i * sizeof (d), sizeof (d), &d);
            }
            else
            {
              struct insn_dispatch_list8 d;
              d.dv = type->u.union_list.ms[i].dv;
              d.off = is_mark_pos (m);
              is_patch (st, mtab, i * sizeof (d), sizeof (d), &d);
            }
            if ((rc = is_append_srcadv (st, (unsigned) (type->u.union_list.moff - discsz))) < 0)
              goto union_return_rc;
            is_push_srcpos (st);
            if ((rc = lowertype1 (ctx, type->u.union_list.ms[i].type)) < 0)
              goto union_return_rc;
            is_pop_srcpos (st);
            if ((rc = is_append_marker_placeholder (&ms[i], st, INSN_POPSRC)) < 0)
              goto union_return_rc;
          }
        }
        /* patch destination addresses of POPSRC insns */
        is_mark (&m, st);
        for (i = 0; i <= type->u.union_list.n; i++)
          is_patch_marker_placeholder (st, ms[i], m);
        is_pop_srcpos (st);
        is_adv_srcpos (st, type->width);
        os_free (ms);
        break;
      union_return_rc:
        os_free (ms);
        return rc;
      }
    case TK_CLASS:
      if (ctx->depth == 1)
      {
        /* top-level class is ignored: that is handled outside the
         * (de)serializer VM, so all we need to do is process the
         * struct representing the class's members.  SRCPOS is
         * essentially irrelevant for the top-level struct, the
         * fiddling here is just to keep the code handling trailing
         * padding happy.
         */
        is_adv_srcpos (st, sizeof (void *));
        is_push_srcpos (st);
        if ((rc = lowertype1 (ctx, type->u.class.subtype)) < 0)
          return rc;
        is_pop_srcpos (st);
      }
      else if (type->u.class.quietref)
      {
        struct insnstream_marker mpop, m;
        if ((rc = is_append_0 (st, 0, INSN_QUIETREF)) < 0)
          return rc;
        is_adv_srcpos (st, sizeof (void *));
        is_push_srcpos (st);
        if ((rc = lowertype1 (ctx, type->u.class.subtype)) < 0)
          return rc;
        is_pop_srcpos (st);
        if ((rc = is_append_marker_placeholder (&mpop, st, INSN_POPSRC)) < 0)
          return rc;
        is_mark (&m, st);
        is_patch_marker_placeholder (st, mpop, m);
      }
      else
      {
        struct insnstream_marker mobj, mpop, m;
        if ((rc = is_append_marker_placeholder (&mobj, st, INSN_REF_UNIQ)) < 0)
          return rc;
        if ((rc = is_append_pointer (st, type->u.class.ospl_type)) < 0)
          return rc;
        is_adv_srcpos (st, sizeof (void *));
        is_push_srcpos (st);
        if ((rc = lowertype1 (ctx, type->u.class.subtype)) < 0)
          return rc;
        is_pop_srcpos (st);
        if ((rc = is_append_marker_placeholder (&mpop, st, INSN_POPSRC)) < 0)
          return rc;
        is_mark (&m, st);
        is_patch_marker_placeholder (st, mobj, m);
        is_patch_marker_placeholder (st, mpop, m);
      }
      break;
  }
  if (type->has_tag)
  {
    if ((rc = is_append_count (st, 0, INSN_TAG, type->tag)) < 0)
      return rc;
  }
  srcpos_after = is_get_srcpos (st);
  if ((rc = is_append_srcadv (st, (unsigned) (type->width - (srcpos_after - srcpos_before)))) < 0)
    return rc;

  ctx->depth--;
  if (type->recuse)
  {
    struct insnstream_marker mcall;
    ctx->typestack_depth--;
    if ((rc = is_append_0 (st, 0, INSN_RETURN)) < 0)
      return rc;
    is_mark (&mcall, st);
    is_patch_marker_placeholder (st, mjump, mcall);
    if ((rc = is_append_marker (st, INSN_CALL, ctx->markers[ctx->typestack_depth])) < 0)
      return rc;
  }
  return 0;
}

static int lowertype (struct serprog **prog, const struct ser_type *type, c_type ospl_type, const struct sd_cdrControl *control)
{
  struct lowertype_context ctx;
  int rc = 0;
  if ((ctx.st = is_new ()) == NULL)
    return SD_CDR_OUT_OF_MEMORY;
  ctx.depth = 0;
  ctx.typestack_depth = 0;
  if ((rc = lowertype1 (&ctx, type)) < 0)
    goto err;
  assert (ctx.typestack_depth == 0);
  if ((rc = is_append_0 (ctx.st, 0, INSN_DONE)) < 0)
    goto err;
  is_make_jumps_relative (ctx.st);
  if ((*prog = is_program (ctx.st, ospl_type, control)) == NULL)
  {
    rc = SD_CDR_OUT_OF_MEMORY;
    goto err;
  }
 err:
  is_free (ctx.st);
  return rc;
}

static void *extract_pointer_from_code (const char *cptr)
{
  /* ptr is at least 4 byte aligned, so on 32-bits platforms and on
   * those with hardware-supported misaligned loads, it is best to
   * just load it (but we don't have an easy way of knowing whether
   * our platforms supports misaligned loads)
   */
  void *ptr;
  memcpy (&ptr, cptr, sizeof (void *));
  return ptr;
}

static const char *opcodestr (enum insn_opcode opcode)
{
  switch (opcode)
  {
    case INSN_DONE: return "done";
    case INSN_SRCADV: return "srcadv";
    case INSN_PRIM1: return "prim1";
    case INSN_PRIM2: return "prim2";
    case INSN_PRIM4: return "prim4";
    case INSN_PRIM8: return "prim8";
    case INSN_PRIM1_POPSRC: return "prim1_popsrc";
    case INSN_PRIM2_POPSRC: return "prim2_popsrc";
    case INSN_PRIM4_POPSRC: return "prim4_popsrc";
    case INSN_PRIM8_POPSRC: return "prim8_popsrc";
    case INSN_PRIM1_LOOP: return "prim1_loop";
    case INSN_PRIM2_LOOP: return "prim2_loop";
    case INSN_PRIM4_LOOP: return "prim4_loop";
    case INSN_PRIM8_LOOP: return "prim8_loop";
    case INSN_PRIM1_LOOPSTAR: return "prim1_loop*";
    case INSN_PRIM2_LOOPSTAR: return "prim2_loop*";
    case INSN_PRIM4_LOOPSTAR: return "prim4_loop*";
    case INSN_PRIM8_LOOPSTAR: return "prim8_loop*";
    case INSN_STRING: return "string";
    case INSN_BSTRING: return "bstring";
    case INSN_STRING_POPSRC: return "string_popsrc";
    case INSN_PUSHCOUNT: return "pushcount";
    case INSN_PUSHSTAR: return "push*";
    case INSN_LOOP: return "loop";
    case INSN_LOOPSTAR: return "loop*";
    case INSN_PRIM1_DISPATCH_LIST: return "prim1_dispatch_list";
    case INSN_PRIM2_DISPATCH_LIST: return "prim2_dispatch_list";
    case INSN_PRIM4_DISPATCH_LIST: return "prim4_dispatch_list";
    case INSN_PRIM8_DISPATCH_LIST: return "prim8_dispatch_list";
    case INSN_PRIM1_DISPATCH_DIRECT: return "prim1_dispatch_direct";
    case INSN_PRIM2_DISPATCH_DIRECT: return "prim2_dispatch_direct";
    case INSN_PRIM4_DISPATCH_DIRECT: return "prim4_dispatch_direct";
    case INSN_PRIM8_DISPATCH_DIRECT: return "prim8_dispatch_direct";
    case INSN_POPSRC: return "popsrc";
    case INSN_STRING_TO_ARRAY: return "string_to_array";
    case INSN_ARRAY_TO_STRING: return "array_to_string";
    case INSN_PRIM1_CONST: return "prim1_const";
    case INSN_JUMP: return "jump";
    case INSN_CALL: return "call";
    case INSN_RETURN: return "return";
    case INSN_REF_UNIQ: return "ref_uniq";
    case INSN_QUIETREF: return "quietref";
    case INSN_TAG_PREP: return "tag_prep";
    case INSN_TAG: return "tag";
  }
  return "?";
}

static unsigned printinsn (FILE *fp, int *indent, int **outdentdelay, const struct serprog *prog, unsigned pos, int isexec, int isloopend)
{
  const struct insn_enc *xs = (const struct insn_enc *) prog->buf;
  fprintf (fp, "%*.*s%5d", *indent, *indent, "", pos);
  if (insn_isnop (&xs[pos]))
  {
    fprintf (fp, " nop\n");
    return 1;
  }
  if (xs[pos].srcadv)
    fprintf (fp, " srcadv %u ;", xs[pos].srcadv);
  fprintf (fp, " %s", opcodestr (xs[pos].opcode));
  switch (xs[pos].opcode)
  {
    case INSN_PRIM1:
    case INSN_PRIM2:
    case INSN_PRIM4:
    case INSN_PRIM8:
    case INSN_STRING:
    case INSN_RETURN:
    case INSN_QUIETREF:
      fprintf (fp, "\n");
      return 1;
    case INSN_BSTRING:
      {
        unsigned maxn = *((const unsigned *) &xs[pos+1]);
        fprintf (fp, " [%u]\n", maxn);
      }
      return 2;
    case INSN_PUSHSTAR:
      {
        unsigned maxn = *((const unsigned *) &xs[pos+1]);
        void *seqtype = extract_pointer_from_code ((const char *) &xs[pos+2]);
        fprintf (fp, " %u (%u) [%u] %p\n", xs[pos].count, pos + 1 + xs[pos].count, maxn, seqtype);
        *indent += 2;
      }
      return 2 + sizeof (void *) / 4;
    case INSN_REF_UNIQ:
      {
        void *subtype = extract_pointer_from_code ((const char *) &xs[pos+1]);
        fprintf (fp, " %u (%u) %p\n", xs[pos].count, pos + 1 + xs[pos].count, subtype);
      }
      return 1 + sizeof (void *) / 4;
    case INSN_PRIM1_POPSRC:
    case INSN_PRIM2_POPSRC:
    case INSN_PRIM4_POPSRC:
    case INSN_PRIM8_POPSRC:
    case INSN_STRING_POPSRC:
      fprintf (fp, " %u (%u)\n", xs[pos].count, pos + 1 + xs[pos].count);
      if (isexec)
        *indent -= 2;
      else if (--(**outdentdelay) == 0) {
        *indent -= 2;
        --(*outdentdelay);
      }
      return 1;
    case INSN_DONE:
      fprintf (fp, " %u\n", xs[pos].count);
      if (isexec)
        *indent -= 2;
      else if (--(**outdentdelay) == 0) {
        *indent -= 2;
        --(*outdentdelay);
      }
      return 1;
    case INSN_SRCADV:
    case INSN_PRIM1_LOOP:
    case INSN_PRIM2_LOOP:
    case INSN_PRIM4_LOOP:
    case INSN_PRIM8_LOOP:
    case INSN_STRING_TO_ARRAY:
    case INSN_ARRAY_TO_STRING:
    case INSN_TAG_PREP:
    case INSN_TAG:
      fprintf (fp, " %u\n", xs[pos].count);
      return 1;
    case INSN_POPSRC:
      fprintf (fp, " %u (%u)\n", xs[pos].count, pos + 1 + xs[pos].count);
      if (isexec)
        *indent -= 2;
      else if (--(**outdentdelay) == 0) {
        *indent -= 2;
        --(*outdentdelay);
      }
      return 1;
    case INSN_JUMP:
      fprintf (fp, " %u (%u)\n", xs[pos].count, pos + 1 + xs[pos].count);
      return 1;
    case INSN_PUSHCOUNT:
      fprintf (fp, " %u\n", xs[pos].count);
      *indent += 2;
      return 1;
    case INSN_LOOP:
    case INSN_LOOPSTAR:
      fprintf (fp, " %u (%u)\n", xs[pos].count, pos + 1 - xs[pos].count);
      if (isloopend)
        *indent -= 2;
      return 1;
    case INSN_PRIM1_LOOPSTAR:
    case INSN_PRIM2_LOOPSTAR:
    case INSN_PRIM4_LOOPSTAR:
    case INSN_PRIM8_LOOPSTAR:
      fprintf (fp, " %u (%u)\n", xs[pos].count, pos + 1 - xs[pos].count);
      *indent -= 2;
      return 1;
    case INSN_CALL:
      fprintf (fp, " %u (%u)\n", xs[pos].count, pos + 1 - xs[pos].count);
      return 1;
    case INSN_PRIM1_DISPATCH_LIST:
    case INSN_PRIM2_DISPATCH_LIST:
    case INSN_PRIM4_DISPATCH_LIST:
      {
        const struct insn_dispatch_list4 *ds =
          (const struct insn_dispatch_list4 *) (prog->buf + 4 * (pos + 2));
        unsigned i, n = xs[pos].count;
        fprintf (fp, " %u %u\n", n, *((unsigned *) prog->buf + pos + 1));
        *indent += 2;
        if (!isexec)
        {
          for (i = 0; i <= n; i++)
            fprintf (fp, "%*.*s    %u => %u (%u)\n", *indent, *indent, "", ds[i].dv, ds[i].off, pos + ds[i].off + 1);
          ++(*outdentdelay);
          **outdentdelay = (int) (n+1);
        }
        return 2 + (n + 1) * (unsigned) (sizeof (*ds) / 4);
      }
    case INSN_PRIM8_DISPATCH_LIST:
      {
        const struct insn_dispatch_list8 *ds =
          (const struct insn_dispatch_list8 *) (prog->buf + 4 * (pos + 2));
        unsigned i, n = xs[pos].count;
        fprintf (fp, " %u %u\n", n, *((unsigned *) prog->buf + pos + 1));
        *indent += 2;
        if (!isexec)
        {
          for (i = 0; i <= n; i++)
            fprintf (fp, "%*.*s    %llu => %u (%u)\n", *indent, *indent, "", ds[i].dv, ds[i].off, pos + ds[i].off + 1);
          ++(*outdentdelay);
          **outdentdelay = (int) (n+1);
        }
        return 2 + (n + 1) * (unsigned) (sizeof (*ds) / 4);
      }
    case INSN_PRIM1_DISPATCH_DIRECT:
    case INSN_PRIM2_DISPATCH_DIRECT:
    case INSN_PRIM4_DISPATCH_DIRECT:
    case INSN_PRIM8_DISPATCH_DIRECT:
      {
        const struct insn_dispatch_direct *ds =
          (const struct insn_dispatch_direct *) (prog->buf + 4 * (pos + 2));
        unsigned i, n = xs[pos].count;
        fprintf (fp, " %u %u\n", n, *((unsigned *) prog->buf + pos + 1));
        *indent += 2;
        if (!isexec)
        {
          for (i = 0; i <= n; i++)
            fprintf (fp, "%*.*s    (%u) => %u (%u)\n", *indent, *indent, "", i, ds[i].off, pos + ds[i].off + 1);
          ++(*outdentdelay);
          **outdentdelay = (int) (n+1);
        }
        return 2 + (n + 1) * (unsigned) (sizeof (*ds) / 4);
      }
    case INSN_PRIM1_CONST:
      fprintf (stderr, "%u\n", xs[pos].count);
      return 1;
  }
  return 1;
}

static void printprog (FILE *fp, const struct serprog *prog)
{
  unsigned pos = 0;
  int indent = 0;
  int outdentdelaybuf[128], *outdentdelay = &outdentdelaybuf[0];
  fprintf (fp, "PROG: %" PA_PRIuSIZE " (base %p type %p)\n", prog->size, (void *) prog->base, (void *) prog->ospl_type);
  *outdentdelay = 0;
  while (pos < prog->size)
  {
    pos += printinsn (fp, &indent, &outdentdelay, prog, pos, 0, 1);
  }
}

/* Undocumented interface for debugging */
void sd_cdrPrintProg (FILE *fp, const struct serprog *prog)
{
  printprog (fp, prog);
}

/************** SERIALIZED DATA *************/

struct sd_cdrSerdataBlock {
  union {
    struct {
      struct sd_cdrSerdataBlock *next;
      char *endp;
    } st;
    double align_f;
    long long align_i;
  } u;
  char data[1];
};

struct sd_cdrSerdata {
  struct sd_cdrSerdataBlock *last;
  int clear_padding;
  size_t sersize;
  struct sd_cdrSerdataBlock *first;
  char *blob;
};

static struct sd_cdrSerdataBlock *sdblock_new (size_t size, int clear)
{
  struct sd_cdrSerdataBlock *sb;
  sb = os_malloc (offsetof (struct sd_cdrSerdataBlock, data) + size);
  if (clear)
    memset (sb->data, 0, size);
  sb->u.st.next = NULL;
  sb->u.st.endp = sb->data + size;
  return sb;
}

static struct sd_cdrSerdata *sd_new (const struct sd_cdrInfo *ci)
{
  struct sd_cdrSerdata *sd;
  sd = os_malloc (sizeof (*sd));
  sd->clear_padding = ci->clear_padding;
  sd->sersize = 0;
  sd->blob = NULL;
  sd->first = sd->last = NULL;
  return sd;
}

static struct sd_cdrSerdataBlock *sd_addblock (struct sd_cdrSerdata *sd, char * const dst, size_t size)
{
  struct sd_cdrSerdataBlock *sb;
  if ((sb = sdblock_new (size, sd->clear_padding)) == NULL)
    return NULL;
  sd->last->u.st.endp = dst;
  sd->sersize += (size_t) (dst - sd->last->data);
  sd->last->u.st.next = sb;
  sd->last = sb;
  return sb;
}

static void sd_finalize (struct sd_cdrSerdata *sd, char * const dst)
{
  sd->last->u.st.endp = dst;
  sd->sersize += (size_t) (dst - sd->last->data);
}

static void sd_free (struct sd_cdrSerdata *sd)
{
  if (sd->blob && sd->blob != sd->first->data)
    os_free (sd->blob);
  while (sd->first)
  {
    struct sd_cdrSerdataBlock *sb = sd->first;
    sd->first = sb->u.st.next;
    os_free (sb);
  }
  os_free (sd);
}

static int sd_blob (struct sd_cdrSerdata *sd, const void **blob, size_t *sz)
{
#ifdef PRINTALLOC
  static int x = 0;
#endif
  *sz = sd->sersize;
  if (sd->first == sd->last)
  {
#ifdef PRINTALLOC
    if (!x) { fprintf (stderr, "sd_blob: alias %u bytes\n", (unsigned) *sz); x = 1; }
#endif
    *blob = sd->first->data;
    return 0;
  }
  else
  {
    const struct sd_cdrSerdataBlock *sb;
    char *dst = os_malloc (sd->sersize);
    *blob = sd->blob = dst;
    for (sb = sd->first; sb; sb = sb->u.st.next)
    {
      size_t n = (size_t) (sb->u.st.endp - sb->data);
#ifdef PRINTALLOC
      if (!x) { fprintf (stderr, "sd_blob: %p copy %u bytes\n", (void *) sd, (unsigned) n); }
#endif
      memcpy (dst, sb->data, n);
      dst += n;
    }
#ifdef PRINTALLOC
    x = 1;
#endif
    return 0;
  }
}

/******************** SERIALIZE VM (STRAIGHT) ********************/

typedef unsigned char serprog_uint1_t;
typedef os_ushort serprog_uint2_t;
typedef os_uint32 serprog_uint4_t;
typedef os_uint64 serprog_uint8_t;

typedef serprog_uint1_t serprog_uint_x0_t;
typedef serprog_uint2_t serprog_uint_x1_t;
typedef serprog_uint4_t serprog_uint_x2_t;
typedef serprog_uint8_t serprog_uint_x3_t;

#ifdef __GNUC__
#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define LIKELY(x)   (x)
#define UNLIKELY(x) (x)
#endif

static void align_dst_0 (UNUSED_ARG (char **dst)) { }
static void align_dst_1 (char **dst) { *dst = alignup_pchar (*dst, 2); }
static void align_dst_2 (char **dst) { *dst = alignup_pchar (*dst, 4); }
static void align_dst_3 (char **dst) { *dst = alignup_pchar (*dst, 8); }

#define SER_OPER_CLEAR_MULTIPLE_0(n) memset (*dst, 0, (n))
#define SER_OPER_COPY_MULTIPLE_0(n) memcpy (*dst, *src, (n))
#define SER_OPER_COPY_MULTIPLE_1(n) memcpy (*dst, *src, (n) << 1)
#define SER_OPER_COPY_MULTIPLE_2(n) memcpy (*dst, *src, (n) << 2)
#define SER_OPER_COPY_MULTIPLE_3(n) memcpy (*dst, *src, (n) << 3)

#define SER_SLOWPATH_MULTIPLE(OPER, oper, lg2_width) static int ser_slowpath_##oper##_multiple_##lg2_width ( \
          const struct sd_cdrControl *control,                          \
          struct sd_cdrSerdata *sd, char **dst, char **dstlimit, const char **src,  \
          os_uint32 n) {                                                \
    assert ((lg2_width) >= 0 && (lg2_width) <= 3);                      \
    assert (n > 0);                                                     \
    do {                                                                \
      os_uint32 mb, mm, m;                                              \
      assert (*dst <= *dstlimit);                                       \
      if (*dst == *dstlimit) {                                          \
        int rc;                                                         \
        if ((rc = control->grow (sd, dst, 8192)) < 0)                   \
          return rc;                                                    \
        assert ((((os_address) *dst) & 7) == 0);                        \
        assert (rc > 0 && (rc & 7) == 0);                               \
        *dstlimit = *dst + rc;                                          \
      }                                                                 \
      mb = (os_uint32) (*dstlimit - *dst);  /* available bytes */       \
      assert ((mb % (1 << (lg2_width))) == 0);                          \
      mm = mb >> (lg2_width); /* available prims */                     \
      m = (mm < n) ? mm : n; /* min(avail, actual) */                   \
      SER_OPER_##OPER##_MULTIPLE_##lg2_width (m);                       \
      n -= m;                                                           \
      *dst += m << (lg2_width);                                         \
      *src += m << (lg2_width);                                         \
    } while (n > 0);                                                    \
    return 0;                                                           \
  }

SER_SLOWPATH_MULTIPLE (CLEAR, clear, 0)
SER_SLOWPATH_MULTIPLE (COPY, copy, 0)
SER_SLOWPATH_MULTIPLE (COPY, copy, 1)
SER_SLOWPATH_MULTIPLE (COPY, copy, 2)
SER_SLOWPATH_MULTIPLE (COPY, copy, 3)

#define SER_MULTIPLE(OPER, oper, lg2_width) static int ser_##oper##_multiple_##lg2_width ( \
          const struct sd_cdrControl *control,                          \
          struct sd_cdrSerdata *sd, char **dst, char **dstlimit, const char **src, \
          os_uint32 n) {                                                \
    const os_uint32 needed = n << (lg2_width);                          \
    align_dst_##lg2_width (dst);                                        \
    assert (*dst <= *dstlimit);                                         \
    if (LIKELY (*dst + needed <= *dstlimit))                            \
    {                                                                   \
      SER_OPER_##OPER##_MULTIPLE_##lg2_width (n);                       \
      *src += needed;                                                   \
      *dst += needed;                                                   \
      return 0;                                                         \
    }                                                                   \
    else                                                                \
    {                                                                   \
      return ser_slowpath_##oper##_multiple_##lg2_width (control, sd, dst, dstlimit, src, n); \
    }                                                                   \
  }

SER_MULTIPLE (CLEAR, clear, 0)
SER_MULTIPLE (COPY, copy, 0)
SER_MULTIPLE (COPY, copy, 1)
SER_MULTIPLE (COPY, copy, 2)
SER_MULTIPLE (COPY, copy, 3)

#define SERPROG_EXEC_MULTIPLE(lg2_width, n) do {                       \
    int rc;                                                             \
    if ((rc = ser_copy_multiple_##lg2_width (prog->control, sd, &dst, &dstlimit, &src, (n))) < 0) \
      return rc;                                                        \
  } while (0)
#define SERPROG_EXEC_SINGLE(lg2_width) SERPROG_EXEC_MULTIPLE (lg2_width, 1)

static int ser_write_0 (
        const struct sd_cdrControl *control,
        struct sd_cdrSerdata *sd, char **dst, char **dstlimit,
        char v)
{
  assert (*dst <= *dstlimit);
  if (LIKELY (*dst + 1 <= *dstlimit))
  {
    **dst = v;
    (*dst)++;
    return 0;
  }
  else
  {
    char *src = &v;
    return ser_slowpath_copy_multiple_0 (control, sd, dst, dstlimit, (const char **) &src, 1);
  }
}

static int ser_write_2 (
        const struct sd_cdrControl *control,
        struct sd_cdrSerdata *sd, char **dst, char **dstlimit,
        os_uint32 v)
{
  align_dst_2 (dst);
  assert (*dst <= *dstlimit);
  if (LIKELY (*dst + 4 <= *dstlimit))
  {
    *((serprog_uint4_t *) *dst) = v;
    *dst += 4;
    return 0;
  }
  else
  {
    char *src = (char *) &v;
    return ser_slowpath_copy_multiple_2 (control, sd, dst, dstlimit, (const char **) &src, 1);
  }
}

#define SERPROG_WRITE_COUNT(count) do {                                 \
    int rc;                                                             \
    if ((rc = ser_write_2 (prog->control, sd, &dst, &dstlimit, (count))) < 0) \
      return rc;                                                        \
  } while (0)

#define SERPROG_EXEC_MULTIPLE1(n) SERPROG_EXEC_MULTIPLE (0, n)
#define SERPROG_EXEC_SINGLE1() SERPROG_EXEC_SINGLE (0)

#define SERPROG_EXEC_DISPATCH_list do {         \
    unsigned n = insn.count;                    \
    unsigned i;                                 \
    for (i = 0; i < n && disc != ds[i].dv; i++) \
      /* skip */ ;                              \
    xs += ds[i].off;                            \
  } while (0)
#define SERPROG_EXEC_DISPATCH_direct do {       \
    unsigned n = insn.count;                    \
    xs += ds[disc > n ? n : disc].off;          \
  } while (0)
#define SERPROG_EXEC_DISPATCH_list4 SERPROG_EXEC_DISPATCH_list
#define SERPROG_EXEC_DISPATCH_list8 SERPROG_EXEC_DISPATCH_list
#define SERPROG_EXEC_DISPATCH(lg2_width, mode) do {     \
    const struct insn_dispatch_##mode *ds =             \
      (const struct insn_dispatch_##mode *) (xs + 1);   \
    const unsigned srcadv = *((const unsigned *) xs);   \
    serprog_uint_x##lg2_width##_t disc =                \
      *((const serprog_uint_x##lg2_width##_t *) src);   \
    *stk++ = (os_address) src + srcadv;                 \
    SERPROG_EXEC_SINGLE (lg2_width);                    \
    SERPROG_EXEC_DISPATCH_##mode;                       \
  } while (0)

#ifdef PROGEXEC_TRACE
static void serprog_exec_trace (FILE *fp, int *indent, const struct serprog *prog, const struct insn_enc *xs, unsigned loopcount)
{
  unsigned pos = (unsigned) (xs - (const struct insn_enc *) prog->buf);
  printinsn (fp, indent, NULL, prog, pos, 1, loopcount == 1);
}
#endif

static const char *getstring (const char *src)
{
  /* data is a c_string* is an os_char** is a char** (it's
   * gotta be 'cos it's passed to strlen -- O! the joys of
   * defining aliases for primitive types in C ...
   * FIXME: wstrings?
   */
  const char *s = (const char *) (*((const c_string *) src));
  return s;
}

static int serprog_exec (struct sd_cdrSerdata *sd, const struct serprog *prog, const char *data, os_uint32 size_hint)
{
  /* os_address really means uintptr_t: unsigned int, or a pointer;
   * stack is unified for loop counts, source pointers and code
   * pointers; when both a loop and a source pointer are are pushed,
   * src goes first (and therefore when both are popped, src comes
   * last)
   */
  const struct insn_enc *xs = (const struct insn_enc *) prog->buf;
  os_address stk_storage[128];
  os_address *stk = &stk_storage[0];
  const char *src = data;
  char *dst, *dstlimit;
  unsigned loopcount = 0;
  char *cdrptr = 0;
  os_uint32 cdrpos = 0;
#ifdef PROGEXEC_TRACE
  int indent = 0;
  fprintf (stderr, "EXEC SER: data %p\n", (void *) data);
#endif
#ifdef __clang_analyzer__
  memset(stk_storage, 0, sizeof(stk_storage));
#endif

  {
    int rc;
    dst = NULL;
    if ((rc = prog->control->init (sd, &dst, size_hint)) < 0)
      return rc;
    dstlimit = dst + rc;
  }

  while (1)
  {
    const struct insn_enc insn = *xs++;
#ifdef PROGEXEC_TRACE
    fprintf (stderr, "%p %5u - ", src, (unsigned) (dst - sd->last->data));
    serprog_exec_trace (stderr, &indent, prog, xs - 1, loopcount);
#endif
    src += insn.srcadv;
    switch ((enum insn_opcode) insn.opcode)
    {
      case INSN_DONE:
        prog->control->finalize (sd, dst);
        return - (int) insn.count;
      case INSN_PRIM1:
        SERPROG_EXEC_SINGLE1 ();
        break;
      case INSN_PRIM2:
        SERPROG_EXEC_SINGLE (1);
        break;
      case INSN_PRIM4:
        SERPROG_EXEC_SINGLE (2);
        break;
      case INSN_PRIM8:
        SERPROG_EXEC_SINGLE (3);
        break;
      case INSN_PRIM1_POPSRC:
        SERPROG_EXEC_SINGLE1 ();
        src = (const char *) *--stk;
        xs += insn.count;
        break;
      case INSN_PRIM2_POPSRC:
        SERPROG_EXEC_SINGLE (1);
        src = (const char *) *--stk;
        xs += insn.count;
        break;
      case INSN_PRIM4_POPSRC:
        SERPROG_EXEC_SINGLE (2);
        src = (const char *) *--stk;
        xs += insn.count;
        break;
      case INSN_PRIM8_POPSRC:
        SERPROG_EXEC_SINGLE (3);
        src = (const char *) *--stk;
        xs += insn.count;
        break;
      case INSN_PRIM1_LOOP:
        SERPROG_EXEC_MULTIPLE1 (insn.count);
        break;
      case INSN_PRIM2_LOOP:
        SERPROG_EXEC_MULTIPLE (1, insn.count);
        break;
      case INSN_PRIM4_LOOP:
        SERPROG_EXEC_MULTIPLE (2, insn.count);
        break;
      case INSN_PRIM8_LOOP:
        SERPROG_EXEC_MULTIPLE (3, insn.count);
        break;
      case INSN_STRING:
      case INSN_BSTRING:
      case INSN_STRING_POPSRC:
        {
          const char *s = getstring (src);
          const unsigned n = s ? (unsigned) (strlen (s) + 1) : 0;
          int rc;
          SERPROG_WRITE_COUNT (n);
          if ((rc = ser_copy_multiple_0 (prog->control, sd, &dst, &dstlimit, &s, n)) < 0)
            return rc;
          if (insn.opcode != INSN_STRING_POPSRC)
          {
            xs += (insn.opcode == INSN_BSTRING);
            src += sizeof (char *);
          }
          else
          {
            src = (const char *) *--stk;
            xs += insn.count;
          }
        }
        break;
      case INSN_POPSRC:
        src = (const char *) *--stk;
        xs += insn.count;
        break;
      case INSN_SRCADV:
        src += insn.count;
        break;
      case INSN_PRIM1_LOOPSTAR:
        SERPROG_EXEC_MULTIPLE1 (loopcount);
        loopcount = (unsigned) *--stk;
        src = (const char *) *--stk;
        break;
      case INSN_PRIM2_LOOPSTAR:
        SERPROG_EXEC_MULTIPLE (1, loopcount);
        loopcount = (unsigned) *--stk;
        src = (const char *) *--stk;
        break;
      case INSN_PRIM4_LOOPSTAR:
        SERPROG_EXEC_MULTIPLE (2, loopcount);
        loopcount = (unsigned) *--stk;
        src = (const char *) *--stk;
        break;
      case INSN_PRIM8_LOOPSTAR:
        SERPROG_EXEC_MULTIPLE (3, loopcount);
        loopcount = (unsigned) *--stk;
        src = (const char *) *--stk;
        break;
      case INSN_PUSHCOUNT:
        *stk++ = loopcount;
        loopcount = insn.count;
        break;
      case INSN_PUSHSTAR:
        {
          const void *ary = *((const void **) src);
          unsigned n = c_arraySize ((void **) ary);
          SERPROG_WRITE_COUNT (n);
          if (n == 0)
          {
            src += sizeof (void *);
            xs += insn.count;
#ifdef PROGEXEC_TRACE
            indent -= 2;
#endif
          }
          else
          {
            xs += 1 + sizeof (void *) / 4; /* maxn, c_type */
            *stk++ = (os_address) src + sizeof (void *);
            *stk++ = loopcount;
            src = ary;
            loopcount = n;
          }
        }
        break;
      case INSN_LOOP:
        if (--loopcount != 0)
          xs -= insn.count;
        else
          loopcount = (unsigned) *--stk;
        break;
      case INSN_LOOPSTAR:
        if (--loopcount != 0)
          xs -= insn.count;
        else
        {
          loopcount = (unsigned) *--stk;
          src = (const char *) *--stk;
        }
        break;
      case INSN_PRIM1_DISPATCH_LIST:
        SERPROG_EXEC_DISPATCH (0, list4);
        break;
      case INSN_PRIM2_DISPATCH_LIST:
        SERPROG_EXEC_DISPATCH (1, list4);
        break;
      case INSN_PRIM4_DISPATCH_LIST:
        SERPROG_EXEC_DISPATCH (2, list4);
        break;
      case INSN_PRIM8_DISPATCH_LIST:
        SERPROG_EXEC_DISPATCH (3, list8);
        break;
      case INSN_PRIM1_DISPATCH_DIRECT:
        SERPROG_EXEC_DISPATCH (0, direct);
        break;
      case INSN_PRIM2_DISPATCH_DIRECT:
        SERPROG_EXEC_DISPATCH (1, direct);
        break;
      case INSN_PRIM4_DISPATCH_DIRECT:
        SERPROG_EXEC_DISPATCH (2, direct);
        break;
      case INSN_PRIM8_DISPATCH_DIRECT:
        SERPROG_EXEC_DISPATCH (3, direct);
        break;
      case INSN_STRING_TO_ARRAY:
        {
          const char *s1 = getstring (src);
          const char *s = s1 ? s1 : "";
          unsigned n;
          int rc;
          n = (unsigned) (strlen (s) + 1);
          if (n >= insn.count)
            n = insn.count;
          if ((rc = ser_copy_multiple_0 (prog->control, sd, &dst, &dstlimit, &s, n)) < 0)
            return rc;
          if (n < insn.count)
          {
            /* note that the src is not really needed for clearing,
             * but the function does advance the source pointer,
             * needed or not
             */
            ser_clear_multiple_0 (prog->control, sd, &dst, &dstlimit, &s, insn.count - n);
          }
          src += sizeof (char *);
        }
        break;
      case INSN_ARRAY_TO_STRING:
        {
          const unsigned n = (unsigned) os_strnlen (src, insn.count) + 1;
          const char *tmpsrc;
          int rc;
          SERPROG_WRITE_COUNT (n);
          tmpsrc = src;
          if ((rc = ser_copy_multiple_0 (prog->control, sd, &dst, &dstlimit, &tmpsrc, n - 1)) < 0)
            return rc;
          if ((rc = ser_write_0 (prog->control, sd, &dst, &dstlimit, 0)) < 0)
            return rc;
          src += insn.count;
        }
        break;
      case INSN_PRIM1_CONST:
        {
          int rc;
          if ((rc = ser_write_0 (prog->control, sd, &dst, &dstlimit, (char) insn.count)) < 0)
            return rc;
        }
        break;
      case INSN_JUMP:
        xs += insn.count;
        break;
      case INSN_CALL:
        *stk++ = (os_address) xs;
        xs -= insn.count;
        break;
      case INSN_REF_UNIQ:
        {
          const void *obj = *((const void **) src);
          int rc;
          if ((rc = ser_write_0 (prog->control, sd, &dst, &dstlimit, (obj != NULL))) < 0)
            return rc;
          if (obj == NULL)
          {
            src += sizeof (void *);
            xs += insn.count;
          }
          else
          {
            xs += sizeof (void *) / 4; /* c_type */
            *stk++ = (os_address) src + sizeof (void *);
            src = obj;
          }
        }
        break;
      case INSN_QUIETREF:
        *stk++ = (os_address) src + sizeof (void *);
        src = *((const void **) src);
        break;
      case INSN_RETURN:
        xs = (const struct insn_enc *) *--stk;
        break;
      case INSN_TAG_PREP:
        dst = alignup_pchar (dst, insn.count);
        if (dst == dstlimit) {
          int rc;
          if ((rc = prog->control->grow (sd, &dst, 8192)) < 0)
            return rc;
          assert ((((os_address) dst) & 7) == 0);
          assert (rc > 0 && (rc & 7) == 0);
          dstlimit = dst + rc;
        }
        cdrpos = prog->control->getpos (sd, dst);
        cdrptr = dst;
        break;
      case INSN_TAG:
        {
          int rc;
          if ((rc = prog->control->process (prog->control->process_arg, sd, insn.count, cdrpos, cdrptr)) < 0)
            return rc;
        }
        break;
    }
  }
}

/******************** SERIALIZE VM (SWAPPED) ********************/

#define SER_OPER_COPY_MULTIPLE_SWAP(width) static void ser_oper_copy_multiple_swap_##width (char *dst, const char *src, os_uint32 n) { \
    const serprog_uint##width##_t *tsrc =       \
      (const serprog_uint##width##_t *) src;    \
    serprog_uint##width##_t *tdst;              \
    os_uint32 i;                                \
    tdst = (serprog_uint##width##_t *) dst;     \
    for (i = 0; i < n; i++)                     \
      *tdst++ = bswap##width##u (*tsrc++);      \
  }
SER_OPER_COPY_MULTIPLE_SWAP (2)
SER_OPER_COPY_MULTIPLE_SWAP (4)
SER_OPER_COPY_MULTIPLE_SWAP (8)
#define SER_OPER_COPY_MULTIPLE_SWAP_0(n) memcpy (*dst, *src, (n))
#define SER_OPER_COPY_MULTIPLE_SWAP_1(n) ser_oper_copy_multiple_swap_2 (*dst, *src, n)
#define SER_OPER_COPY_MULTIPLE_SWAP_2(n) ser_oper_copy_multiple_swap_4 (*dst, *src, n)
#define SER_OPER_COPY_MULTIPLE_SWAP_3(n) ser_oper_copy_multiple_swap_8 (*dst, *src, n)

#define SER_SLOWPATH_MULTIPLE_SWAP(OPER, oper, lg2_width) static int ser_slowpath_##oper##_multiple_swap_##lg2_width ( \
          const struct sd_cdrControl *control,                          \
          struct sd_cdrSerdata *sd, char **dst, char **dstlimit, const char **src,  \
          os_uint32 n) {                                                \
    assert ((lg2_width) >= 0 && (lg2_width) <= 3);                      \
    assert (n > 0);                                                     \
    do {                                                                \
      os_uint32 mb, mm, m;                                              \
      assert (*dst <= *dstlimit);                                       \
      if (*dst == *dstlimit) {                                          \
        int rc;                                                         \
        if ((rc = control->grow (sd, dst, 8192)) < 0)                   \
          return rc;                                                    \
        assert ((((os_address) *dst) & 7) == 0);                        \
        assert (rc > 0 && (rc & 7) == 0);                               \
        *dstlimit = *dst + rc;                                          \
      }                                                                 \
      mb = (os_uint32) (*dstlimit - *dst);  /* available bytes */       \
      assert ((mb % (1 << (lg2_width))) == 0);                          \
      mm = mb >> (lg2_width); /* available prims */                     \
      m = (mm < n) ? mm : n; /* min(avail, actual) */                   \
      SER_OPER_##OPER##_MULTIPLE_SWAP_##lg2_width (m);                  \
      n -= m;                                                           \
      *dst += m << (lg2_width);                                         \
      *src += m << (lg2_width);                                         \
    } while (n > 0);                                                    \
    return 0;                                                           \
  }

SER_SLOWPATH_MULTIPLE_SWAP (COPY, copy, 0)
SER_SLOWPATH_MULTIPLE_SWAP (COPY, copy, 1)
SER_SLOWPATH_MULTIPLE_SWAP (COPY, copy, 2)
SER_SLOWPATH_MULTIPLE_SWAP (COPY, copy, 3)

#define SER_MULTIPLE_SWAP(OPER, oper, lg2_width) static int ser_##oper##_multiple_swap_##lg2_width ( \
          const struct sd_cdrControl *control,                          \
          struct sd_cdrSerdata *sd, char **dst, char **dstlimit, const char **src, \
          os_uint32 n) {                                                \
    const os_uint32 needed = n << (lg2_width);                          \
    align_dst_##lg2_width (dst);                                        \
    assert (*dst <= *dstlimit);                                         \
    if (LIKELY (*dst + needed <= *dstlimit))                            \
    {                                                                   \
      SER_OPER_##OPER##_MULTIPLE_SWAP_##lg2_width (n);                  \
      *src += needed;                                                   \
      *dst += needed;                                                   \
      return 0;                                                         \
    }                                                                   \
    else                                                                \
    {                                                                   \
      return ser_slowpath_##oper##_multiple_swap_##lg2_width (control, sd, dst, dstlimit, src, n); \
    }                                                                   \
  }

SER_MULTIPLE_SWAP (COPY, copy, 0)
SER_MULTIPLE_SWAP (COPY, copy, 1)
SER_MULTIPLE_SWAP (COPY, copy, 2)
SER_MULTIPLE_SWAP (COPY, copy, 3)

#undef SERPROG_EXEC_MULTIPLE
#undef SERPROG_EXEC_SINGLE

#define SERPROG_EXEC_MULTIPLE(lg2_width, n) do {                       \
    int rc;                                                             \
    if ((rc = ser_copy_multiple_swap_##lg2_width (prog->control, sd, &dst, &dstlimit, &src, (n))) < 0) \
      return rc;                                                        \
  } while (0)
#define SERPROG_EXEC_SINGLE(lg2_width) SERPROG_EXEC_MULTIPLE (lg2_width, 1)

static int ser_write_swap_2 (
        const struct sd_cdrControl *control,
        struct sd_cdrSerdata *sd, char **dst, char **dstlimit,
        os_uint32 v)
{
  align_dst_2 (dst);
  assert (*dst <= *dstlimit);
  if (LIKELY (*dst + 4 <= *dstlimit))
  {
    *((serprog_uint4_t *) *dst) = bswap4u (v);
    *dst += 4;
    return 0;
  }
  else
  {
    char *src = (char *) &v;
    return ser_slowpath_copy_multiple_swap_2 (control, sd, dst, dstlimit, (const char **) &src, 1);
  }
}

#undef SERPROG_WRITE_COUNT
#define SERPROG_WRITE_COUNT(count) do {                                 \
    int rc;                                                             \
    if ((rc = ser_write_swap_2 (prog->control, sd, &dst, &dstlimit, (count))) < 0) \
      return rc;                                                        \
  } while (0)

static int serprog_exec_swap (struct sd_cdrSerdata *sd, const struct serprog *prog, const char *data, os_uint32 size_hint)
{
  /* os_address really means uintptr_t: unsigned int, or a pointer;
   * stack is unified for loop counts, source pointers and code
   * pointers; when both a loop and a source pointer are are pushed,
   * src goes first (and therefore when both are popped, src comes
   * last)
   */
  const struct insn_enc *xs = (const struct insn_enc *) prog->buf;
  os_address stk_storage[128];
  os_address *stk = &stk_storage[0];
  const char *src = data;
  char *dst, *dstlimit;
  unsigned loopcount = 0;
  char *cdrptr = 0;
  os_uint32 cdrpos = 0;
#ifdef PROGEXEC_TRACE
  int indent = 0;
  fprintf (stderr, "EXEC SER: data %p\n", (void *) data);
#endif
#ifdef __clang_analyzer__
  memset(stk_storage, 0, sizeof(stk_storage));
#endif

  {
    int rc;
    dst = NULL;
    if ((rc = prog->control->init (sd, &dst, size_hint)) < 0)
      return rc;
    dstlimit = dst + rc;
  }

  while (1)
  {
    const struct insn_enc insn = *xs++;
#ifdef PROGEXEC_TRACE
    fprintf (stderr, "%p %5u - ", src, (unsigned) (dst - sd->last->data));
    serprog_exec_trace (stderr, &indent, prog, xs - 1, loopcount);
#endif
    src += insn.srcadv;
    switch ((enum insn_opcode) insn.opcode)
    {
      case INSN_DONE:
        prog->control->finalize (sd, dst);
        return - (int) insn.count;
      case INSN_PRIM1:
        SERPROG_EXEC_SINGLE1 ();
        break;
      case INSN_PRIM2:
        SERPROG_EXEC_SINGLE (1);
        break;
      case INSN_PRIM4:
        SERPROG_EXEC_SINGLE (2);
        break;
      case INSN_PRIM8:
        SERPROG_EXEC_SINGLE (3);
        break;
      case INSN_PRIM1_POPSRC:
        SERPROG_EXEC_SINGLE1 ();
        src = (const char *) *--stk;
        xs += insn.count;
        break;
      case INSN_PRIM2_POPSRC:
        SERPROG_EXEC_SINGLE (1);
        src = (const char *) *--stk;
        xs += insn.count;
        break;
      case INSN_PRIM4_POPSRC:
        SERPROG_EXEC_SINGLE (2);
        src = (const char *) *--stk;
        xs += insn.count;
        break;
      case INSN_PRIM8_POPSRC:
        SERPROG_EXEC_SINGLE (3);
        src = (const char *) *--stk;
        xs += insn.count;
        break;
      case INSN_PRIM1_LOOP:
        SERPROG_EXEC_MULTIPLE1 (insn.count);
        break;
      case INSN_PRIM2_LOOP:
        SERPROG_EXEC_MULTIPLE (1, insn.count);
        break;
      case INSN_PRIM4_LOOP:
        SERPROG_EXEC_MULTIPLE (2, insn.count);
        break;
      case INSN_PRIM8_LOOP:
        SERPROG_EXEC_MULTIPLE (3, insn.count);
        break;
      case INSN_STRING:
      case INSN_BSTRING:
      case INSN_STRING_POPSRC:
        {
          const char *s = getstring (src);
          const unsigned n = s ? (unsigned) (strlen (s) + 1) : 0;
          int rc;
          SERPROG_WRITE_COUNT (n);
          if ((rc = ser_copy_multiple_0 (prog->control, sd, &dst, &dstlimit, &s, n)) < 0)
            return rc;
          if (insn.opcode != INSN_STRING_POPSRC)
          {
            xs += (insn.opcode == INSN_BSTRING);
            src += sizeof (char *);
          }
          else
          {
            src = (const char *) *--stk;
            xs += insn.count;
          }
        }
        break;
      case INSN_POPSRC:
        src = (const char *) *--stk;
        xs += insn.count;
        break;
      case INSN_SRCADV:
        src += insn.count;
        break;
      case INSN_PRIM1_LOOPSTAR:
        SERPROG_EXEC_MULTIPLE1 (loopcount);
        loopcount = (unsigned) *--stk;
        src = (const char *) *--stk;
        break;
      case INSN_PRIM2_LOOPSTAR:
        SERPROG_EXEC_MULTIPLE (1, loopcount);
        loopcount = (unsigned) *--stk;
        src = (const char *) *--stk;
        break;
      case INSN_PRIM4_LOOPSTAR:
        SERPROG_EXEC_MULTIPLE (2, loopcount);
        loopcount = (unsigned) *--stk;
        src = (const char *) *--stk;
        break;
      case INSN_PRIM8_LOOPSTAR:
        SERPROG_EXEC_MULTIPLE (3, loopcount);
        loopcount = (unsigned) *--stk;
        src = (const char *) *--stk;
        break;
      case INSN_PUSHCOUNT:
        *stk++ = loopcount;
        loopcount = insn.count;
        break;
      case INSN_PUSHSTAR:
        {
          const void *ary = *((const void **) src);
          unsigned n = c_arraySize ((void **) ary);
          SERPROG_WRITE_COUNT (n);
          if (n == 0)
          {
            src += sizeof (void *);
            xs += insn.count;
#ifdef PROGEXEC_TRACE
            indent -= 2;
#endif
          }
          else
          {
            xs += 1 + sizeof (void *) / 4; /* maxn, c_type */
            *stk++ = (os_address) src + sizeof (void *);
            *stk++ = loopcount;
            src = ary;
            loopcount = n;
          }
        }
        break;
      case INSN_LOOP:
        if (--loopcount != 0)
          xs -= insn.count;
        else
          loopcount = (unsigned) *--stk;
        break;
      case INSN_LOOPSTAR:
        if (--loopcount != 0)
          xs -= insn.count;
        else
        {
          loopcount = (unsigned) *--stk;
          src = (const char *) *--stk;
        }
        break;
      case INSN_PRIM1_DISPATCH_LIST:
        SERPROG_EXEC_DISPATCH (0, list4);
        break;
      case INSN_PRIM2_DISPATCH_LIST:
        SERPROG_EXEC_DISPATCH (1, list4);
        break;
      case INSN_PRIM4_DISPATCH_LIST:
        SERPROG_EXEC_DISPATCH (2, list4);
        break;
      case INSN_PRIM8_DISPATCH_LIST:
        SERPROG_EXEC_DISPATCH (3, list8);
        break;
      case INSN_PRIM1_DISPATCH_DIRECT:
        SERPROG_EXEC_DISPATCH (0, direct);
        break;
      case INSN_PRIM2_DISPATCH_DIRECT:
        SERPROG_EXEC_DISPATCH (1, direct);
        break;
      case INSN_PRIM4_DISPATCH_DIRECT:
        SERPROG_EXEC_DISPATCH (2, direct);
        break;
      case INSN_PRIM8_DISPATCH_DIRECT:
        SERPROG_EXEC_DISPATCH (3, direct);
        break;
      case INSN_STRING_TO_ARRAY:
        {
          const char *s1 = getstring (src);
          const char *s = s1 ? s1 : "";
          unsigned n = (unsigned) (strlen (s) + 1);
          int rc;
          if (n >= insn.count)
            n = insn.count;
          if ((rc = ser_copy_multiple_0 (prog->control, sd, &dst, &dstlimit, &s, n)) < 0)
            return rc;
          if (n < insn.count)
          {
            /* note that the src is not really needed for clearing,
             * but the function does advance the source pointer,
             * needed or not
             */
            ser_clear_multiple_0 (prog->control, sd, &dst, &dstlimit, &s, insn.count - n);
          }
          src += sizeof (char *);
        }
        break;
      case INSN_ARRAY_TO_STRING:
        {
          const unsigned n = (unsigned) (os_strnlen (src, insn.count) + 1);
          const char *tmpsrc;
          int rc;
          SERPROG_WRITE_COUNT (n);
          tmpsrc = src;
          if ((rc = ser_copy_multiple_0 (prog->control, sd, &dst, &dstlimit, &tmpsrc, n - 1)) < 0)
            return rc;
          if ((rc = ser_write_0 (prog->control, sd, &dst, &dstlimit, 0)) < 0)
            return rc;
          src += insn.count;
        }
        break;
      case INSN_PRIM1_CONST:
        {
          int rc;
          if ((rc = ser_write_0 (prog->control, sd, &dst, &dstlimit, (char) insn.count)) < 0)
            return rc;
        }
        break;
      case INSN_JUMP:
        xs += insn.count;
        break;
      case INSN_CALL:
        *stk++ = (os_address) xs;
        xs -= insn.count;
        break;
      case INSN_REF_UNIQ:
        {
          const void *obj = *((const void **) src);
          int rc;
          if ((rc = ser_write_0 (prog->control, sd, &dst, &dstlimit, (obj != NULL))) < 0)
            return rc;
          if (obj == NULL)
          {
            src += sizeof (void *);
            xs += insn.count;
          }
          else
          {
            xs += sizeof (void *) / 4; /* c_type */
            *stk++ = (os_address) src + sizeof (void *);
            src = obj;
          }
        }
        break;
      case INSN_QUIETREF:
        *stk++ = (os_address) src + sizeof (void *);
        src = *((const void **) src);
        break;
      case INSN_RETURN:
        xs = (const struct insn_enc *) *--stk;
        break;
      case INSN_TAG_PREP:
        dst = alignup_pchar (dst, insn.count);
        if (dst == dstlimit) {
          int rc;
          if ((rc = prog->control->grow (sd, &dst, 8192)) < 0)
            return rc;
          assert ((((os_address) dst) & 7) == 0);
          assert (rc > 0 && (rc & 7) == 0);
          dstlimit = dst + rc;
        }
        cdrpos = prog->control->getpos (sd, dst);
        cdrptr = dst;
        break;
      case INSN_TAG:
        {
          int rc;
          if ((rc = prog->control->process (prog->control->process_arg, sd, insn.count, cdrpos, cdrptr)) < 0)
            return rc;
        }
        break;
    }
  }
}

/******************** DESERIALIZE VM (STRAIGHT) ********************/

/* FIXME: natural alignment on input guaranteed to be 4,
 * so can't do address-based alignment and need to use
 * memcpy (on some platforms) for 8 byte objects
 */
#define SRC_CHECK_ALIGN(amount, align) do {                             \
    src = blob + alignup_address ((os_address) (src - blob), (align));  \
    if (src + amount > srclimit)                                        \
      return SD_CDR_INVALID;                                            \
  } while (0)

#define DESERPROG_EXEC_MULTIPLE(width, n) do {  \
    SRC_CHECK_ALIGN (width*(n), width);         \
    memcpy (dst, src, width*(n));               \
    src += width*(n);                           \
    dst += width*(n);                           \
  } while (0)
#define DESERPROG_EXEC_SINGLE(width) do {               \
    const serprog_uint##width##_t *tsrc;                \
    serprog_uint##width##_t *tdst =                     \
      (serprog_uint##width##_t *) dst;                  \
    SRC_CHECK_ALIGN (width, width);                     \
    tsrc = (const serprog_uint##width##_t *) src;       \
    if ((width) > 4)                                    \
      memcpy (tdst, tsrc, (width));                     \
    else                                                \
      *tdst = *tsrc;                                    \
    src = (const char *) ++tsrc;                        \
    dst = (char *) ++tdst;                              \
  } while (0)
#define DESERPROG_READ_COUNT(countvar) do {     \
    const serprog_uint4_t *tsrc;                \
    SRC_CHECK_ALIGN (4, 4);                     \
    tsrc = (const serprog_uint4_t *) src;       \
    countvar = *tsrc++;                         \
    src = (const char *) tsrc;                  \
  } while (0)
#define DESERPROG_EXEC_MULTIPLE1(n) do {        \
    SRC_CHECK_ALIGN ((n), 1);                   \
    memcpy (dst, src, (n));                     \
    src += (n);                                 \
    dst += (n);                                 \
  } while (0)
#define DESERPROG_EXEC_SINGLE1() do {           \
    const serprog_uint1_t *tsrc;                \
    serprog_uint1_t *tdst =                     \
      (serprog_uint1_t *) dst;                  \
    SRC_CHECK_ALIGN (1, 1);                     \
    tsrc = (const serprog_uint1_t *) src;       \
    *tdst++ = *tsrc++;                          \
    src = (const char *) tsrc;                  \
    dst = (char *) tdst;                        \
  } while (0)

#define DESERPROG_EXEC_DISPATCH_list do {       \
    unsigned n = insn.count;                    \
    unsigned i;                                 \
    for (i = 0; i < n && disc != ds[i].dv; i++) \
      /* skip */ ;                              \
    xs += ds[i].off;                            \
  } while (0)
#define DESERPROG_EXEC_DISPATCH_direct do {     \
    unsigned n = insn.count;                    \
    xs += ds[disc > n ? n : disc].off;          \
  } while (0)
#define DESERPROG_EXEC_DISPATCH_list4 DESERPROG_EXEC_DISPATCH_list
#define DESERPROG_EXEC_DISPATCH_list8 DESERPROG_EXEC_DISPATCH_list
/* NOTE: sneakily reading just-deserialized discriminator from result,
 * rather than reading discriminator from source.  Also note the
 * confusingly named "srcadv".
 */
#define DESERPROG_EXEC_DISPATCH(width, mode) do {               \
    const struct insn_dispatch_##mode *ds =                     \
      (const struct insn_dispatch_##mode *) (xs + 1);           \
    const unsigned srcadv = *((const unsigned *) xs);           \
    serprog_uint##width##_t disc;                               \
    *stk++ = (os_address) dst + srcadv;                         \
    DESERPROG_EXEC_SINGLE (width);                              \
    disc = *((const serprog_uint##width##_t *) dst - 1);        \
    DESERPROG_EXEC_DISPATCH_##mode;                             \
  } while (0)

static int deserprog_exec (char *dst, const struct serprog *prog, os_uint32 sz, const char *blob)
{
  const struct insn_enc *xs = (const struct insn_enc *) prog->buf;
  os_address stk_storage[128];
  os_address *stk = &stk_storage[0];
  const char *src = blob;
  const char *srclimit = blob + sz;
  unsigned loopcount = 0;
#ifdef PROGEXEC_TRACE
  int indent = 0;
  fprintf (stderr, "EXEC DESER: data %p\n", (void *) dst);
#endif
#ifdef __clang_analyzer__
  memset(stk_storage, 0, sizeof(stk_storage));
#endif
  while (1)
  {
    const struct insn_enc insn = *xs++;
#ifdef PROGEXEC_TRACE
    fprintf (stderr, "%p %5u - ", dst, (unsigned) (src - blob));
    serprog_exec_trace (stderr, &indent, prog, xs - 1, loopcount);
#endif
    /* FIXME: instruction fields are named for serialisation, so
     * "source advance" suddenly becomes "destination advance" when
     * deserializing
     */
    dst += insn.srcadv;
    switch ((enum insn_opcode) insn.opcode)
    {
      case INSN_DONE:
        return - (int) insn.count;
      case INSN_PRIM1:
        DESERPROG_EXEC_SINGLE1 ();
        break;
      case INSN_PRIM2:
        DESERPROG_EXEC_SINGLE (2);
        break;
      case INSN_PRIM4:
        DESERPROG_EXEC_SINGLE (4);
        break;
      case INSN_PRIM8:
        DESERPROG_EXEC_SINGLE (8);
        break;
      case INSN_PRIM1_POPSRC:
        DESERPROG_EXEC_SINGLE1 ();
        dst = (char *) *--stk;
        xs += insn.count;
        break;
      case INSN_PRIM2_POPSRC:
        DESERPROG_EXEC_SINGLE (2);
        dst = (char *) *--stk;
        xs += insn.count;
        break;
      case INSN_PRIM4_POPSRC:
        DESERPROG_EXEC_SINGLE (4);
        dst = (char *) *--stk;
        xs += insn.count;
        break;
      case INSN_PRIM8_POPSRC:
        DESERPROG_EXEC_SINGLE (8);
        dst = (char *) *--stk;
        xs += insn.count;
        break;
      case INSN_PRIM1_LOOP:
        DESERPROG_EXEC_MULTIPLE1 (insn.count);
        break;
      case INSN_PRIM2_LOOP:
        DESERPROG_EXEC_MULTIPLE (2u, insn.count);
        break;
      case INSN_PRIM4_LOOP:
        DESERPROG_EXEC_MULTIPLE (4u, insn.count);
        break;
      case INSN_PRIM8_LOOP:
        DESERPROG_EXEC_MULTIPLE (8u, insn.count);
        break;
      case INSN_STRING:
      case INSN_BSTRING:
      case INSN_STRING_POPSRC:
        {
          os_uint32 maxn, n;
          char *str;
          if (insn.opcode != INSN_BSTRING)
            maxn = ~(os_uint32)0;
          else
          {
            maxn = *((unsigned *) xs);
            xs++;
          }
          DESERPROG_READ_COUNT (n);
          SRC_CHECK_ALIGN (n, 1);
          if (n == 0) {
            str = NULL;
          } else if (n > maxn || n > (os_uint32) (srclimit - src) || src[n-1] != 0) {
            return SD_CDR_INVALID;
          } else if ((str = c_stringMalloc_s (prog->base, n)) == NULL) {
            return SD_CDR_OUT_OF_MEMORY;
          } else {
            memcpy (str, src, n);
            src += n;
          }
          *((c_string *) dst) = str;
          if (insn.opcode != INSN_STRING_POPSRC)
            dst += sizeof (char *);
          else
          {
            dst = (char *) *--stk;
            xs += insn.count;
          }
        }
        break;
      case INSN_POPSRC:
        dst = (char *) *--stk;
        xs += insn.count;
        break;
      case INSN_SRCADV:
        src += insn.count;
        break;
      case INSN_PRIM1_LOOPSTAR:
        DESERPROG_EXEC_MULTIPLE1 (loopcount);
        loopcount = (unsigned) *--stk;
        dst = (char *) *--stk;
        break;
      case INSN_PRIM2_LOOPSTAR:
        DESERPROG_EXEC_MULTIPLE (2u, loopcount);
        loopcount = (unsigned) *--stk;
        dst = (char *) *--stk;
        break;
      case INSN_PRIM4_LOOPSTAR:
        DESERPROG_EXEC_MULTIPLE (4u, loopcount);
        loopcount = (unsigned) *--stk;
        dst = (char *) *--stk;
        break;
      case INSN_PRIM8_LOOPSTAR:
        DESERPROG_EXEC_MULTIPLE (8u, loopcount);
        loopcount = (unsigned) *--stk;
        dst = (char *) *--stk;
        break;
      case INSN_PUSHCOUNT:
        *stk++ = loopcount;
        loopcount = insn.count;
        break;
      case INSN_PUSHSTAR:
        {
          unsigned n;
          DESERPROG_READ_COUNT (n);
          if (n == 0)
          {
            *((void **) dst) = NULL;
            dst += sizeof (void *);
            /* offset includes c_type pointer */
            xs += insn.count;
#ifdef PROGEXEC_TRACE
            indent -= 2;
#endif
          }
          else
          {
            const unsigned maxn = *((const unsigned *) xs);
            const struct c_collectionType_s *seqtype = extract_pointer_from_code ((const char *) (xs + 1));
            void *ary;
            if (n > maxn || n > (unsigned) (srclimit - src))
              return SD_CDR_INVALID;
            if ((ary = c_newBaseArrayObject_s ((c_collectionType) seqtype, n)) == NULL)
              return SD_CDR_OUT_OF_MEMORY;
            xs += 1 + sizeof (void *) / 4;
            *((void **) dst) = ary;
            *stk++ = (os_address) dst + sizeof (void *);
            *stk++ = loopcount;
            dst = ary;
            loopcount = n;
          }
        }
        break;
      case INSN_LOOP:
        if (--loopcount != 0)
          xs -= insn.count;
        else
          loopcount = (unsigned) *--stk;
        break;
      case INSN_LOOPSTAR:
        if (--loopcount != 0)
          xs -= insn.count;
        else
        {
          loopcount = (unsigned) *--stk;
          dst = (char *) *--stk;
        }
        break;
      case INSN_PRIM1_DISPATCH_LIST:
        DESERPROG_EXEC_DISPATCH (1, list4);
        break;
      case INSN_PRIM2_DISPATCH_LIST:
        DESERPROG_EXEC_DISPATCH (2, list4);
        break;
      case INSN_PRIM4_DISPATCH_LIST:
        DESERPROG_EXEC_DISPATCH (4, list4);
        break;
      case INSN_PRIM8_DISPATCH_LIST:
        DESERPROG_EXEC_DISPATCH (8, list8);
        break;
      case INSN_PRIM1_DISPATCH_DIRECT:
        DESERPROG_EXEC_DISPATCH (1, direct);
        break;
      case INSN_PRIM2_DISPATCH_DIRECT:
        DESERPROG_EXEC_DISPATCH (2, direct);
        break;
      case INSN_PRIM4_DISPATCH_DIRECT:
        DESERPROG_EXEC_DISPATCH (4, direct);
        break;
      case INSN_PRIM8_DISPATCH_DIRECT:
        DESERPROG_EXEC_DISPATCH (8, direct);
        break;
      case INSN_STRING_TO_ARRAY:
        {
          char *str;
          unsigned n;
          SRC_CHECK_ALIGN (insn.count, 1);
          if (insn.count > 0) {
            n = (unsigned) os_strnlen (src, insn.count);
            if ((str = c_stringMalloc_s (prog->base, n + 1)) == NULL)
              return SD_CDR_OUT_OF_MEMORY;
            memcpy (str, src, n);
            str[n] = 0;
            src += insn.count;
          } else {
            str = NULL;
          }
          *((c_string *) dst) = str;
          dst += sizeof (char *);
        }
        break;
      case INSN_ARRAY_TO_STRING:
        {
          unsigned n;
          DESERPROG_READ_COUNT (n);
          SRC_CHECK_ALIGN (n, 1);
          memcpy (dst, src, (n <= insn.count) ? n : insn.count);
          dst += insn.count;
          src += n;
        }
        break;
      case INSN_PRIM1_CONST:
        /* Constants are not represented in the deserialized
         * representation, so we can skip them -- or we can require
         * that they have the expected value.  Given it is currently
         * only being used for an optional endianness marker, perhaps
         * it'd be wise to check
         */
        SRC_CHECK_ALIGN (1, 1);
        if (*src != insn.count)
          return SD_CDR_INVALID;
        src++;
        break;
      case INSN_JUMP:
        xs += insn.count;
        break;
      case INSN_CALL:
        *stk++ = (os_address) xs;
        xs -= insn.count;
        break;
      case INSN_REF_UNIQ:
        {
          unsigned char flag = (unsigned char) *src++;
          SRC_CHECK_ALIGN (1, 1);
          if (flag == 0)
          {
            *((void **) dst) = NULL;
            dst += sizeof (void *);
            /* offset includes c_type pointer */
            xs += insn.count;
          }
          else if (flag == 1)
          {
            const struct c_type_s *subtype = extract_pointer_from_code ((const char *) xs);
            void *obj;
            if ((obj = c_new_s ((c_type) subtype)) == NULL)
              return SD_CDR_OUT_OF_MEMORY;
            xs += sizeof (void *) / 4;
            *((void **) dst) = obj;
            *stk++ = (os_address) dst + sizeof (void *);
            dst = obj;
          }
          else
          {
            return SD_CDR_INVALID;
          }
        }
        break;
      case INSN_QUIETREF:
        *stk++ = (os_address) dst + sizeof (void *);
        dst = *((void **) dst);
        break;
      case INSN_RETURN:
        xs = (const struct insn_enc *) *--stk;
        break;
      case INSN_TAG_PREP:
      case INSN_TAG:
        break;
    }
  }
}

#undef DESERPROG_EXEC_MULTIPLE
#undef DESERPROG_EXEC_SINGLE
#undef DESERPROG_READ_COUNT

/******************** DESERIALIZE VM (SWAPPED) ********************/

#define DESERPROG_EXEC_MULTIPLE(width, n) do {          \
    const serprog_uint##width##_t *tsrc;                \
    serprog_uint##width##_t *tdst =                     \
      (serprog_uint##width##_t *) dst;                  \
    unsigned i;                                         \
    SRC_CHECK_ALIGN (width*(n), width);                 \
    tsrc = (const serprog_uint##width##_t *) src;       \
    for (i = 0; i < n; i++) {                           \
      serprog_uint##width##_t srcval;                   \
      if ((width) > 4)                                  \
        memcpy (&srcval, tsrc, (width));                \
      else                                              \
        srcval = *tsrc;                                 \
      tsrc++;                                           \
      *tdst++ = bswap##width##u (srcval);               \
    }                                                   \
    src = (const char *) tsrc;                          \
    dst = (char *) tdst;                                \
  } while (0)
#define DESERPROG_EXEC_SINGLE(width) do {               \
    const serprog_uint##width##_t *tsrc;                \
    serprog_uint##width##_t srcval;                     \
    serprog_uint##width##_t *tdst =                     \
      (serprog_uint##width##_t *) dst;                  \
    SRC_CHECK_ALIGN (width, width);                     \
    tsrc = (const serprog_uint##width##_t *) src;       \
    if ((width) > 4)                                    \
      memcpy (&srcval, tsrc, (width));                  \
    else                                                \
      srcval = *tsrc;                                   \
    *tdst = bswap##width##u (srcval);                   \
    src = (const char *) ++tsrc;                        \
    dst = (char *) ++tdst;                              \
  } while (0)
#define DESERPROG_READ_COUNT(countvar) do {     \
    const serprog_uint4_t *tsrc;                \
    SRC_CHECK_ALIGN (4, 4);                     \
    tsrc = (const serprog_uint4_t *) src;       \
    countvar = bswap4u (*tsrc++);               \
    src = (const char *) tsrc;                  \
  } while (0)

static int deserprog_exec_swap (char *dst, const struct serprog *prog, os_uint32 sz, const char *blob)
{
  const struct insn_enc *xs = (const struct insn_enc *) prog->buf;
  os_address stk_storage[128];
  os_address *stk = &stk_storage[0];
  const char *src = blob;
  const char *srclimit = blob + sz;
  unsigned loopcount = 0;
#ifdef PROGEXEC_TRACE
  int indent = 0;
  fprintf (stderr, "EXEC DESER: data %p\n", (void *) dst);
#endif
#ifdef __clang_analyzer__
  memset(stk_storage, 0, sizeof(stk_storage));
#endif
  while (1)
  {
    const struct insn_enc insn = *xs++;
#ifdef PROGEXEC_TRACE
    fprintf (stderr, "%p %5u - ", dst, (unsigned) (src - blob));
    serprog_exec_trace (stderr, &indent, prog, xs - 1, loopcount);
#endif
    /* FIXME: instruction fields are named for serialisation, so
     * "source advance" suddenly becomes "destination advance" when
     * deserializing
     */
    dst += insn.srcadv;
    switch ((enum insn_opcode) insn.opcode)
    {
      case INSN_DONE:
        return - (int) insn.count;
      case INSN_PRIM1:
        DESERPROG_EXEC_SINGLE1 ();
        break;
      case INSN_PRIM2:
        DESERPROG_EXEC_SINGLE (2);
        break;
      case INSN_PRIM4:
        DESERPROG_EXEC_SINGLE (4);
        break;
      case INSN_PRIM8:
        DESERPROG_EXEC_SINGLE (8);
        break;
      case INSN_PRIM1_POPSRC:
        DESERPROG_EXEC_SINGLE1 ();
        dst = (char *) *--stk;
        xs += insn.count;
        break;
      case INSN_PRIM2_POPSRC:
        DESERPROG_EXEC_SINGLE (2);
        dst = (char *) *--stk;
        xs += insn.count;
        break;
      case INSN_PRIM4_POPSRC:
        DESERPROG_EXEC_SINGLE (4);
        dst = (char *) *--stk;
        xs += insn.count;
        break;
      case INSN_PRIM8_POPSRC:
        DESERPROG_EXEC_SINGLE (8);
        dst = (char *) *--stk;
        xs += insn.count;
        break;
      case INSN_PRIM1_LOOP:
        DESERPROG_EXEC_MULTIPLE1 (insn.count);
        break;
      case INSN_PRIM2_LOOP:
        DESERPROG_EXEC_MULTIPLE (2, insn.count);
        break;
      case INSN_PRIM4_LOOP:
        DESERPROG_EXEC_MULTIPLE (4, insn.count);
        break;
      case INSN_PRIM8_LOOP:
        DESERPROG_EXEC_MULTIPLE (8, insn.count);
        break;
      case INSN_STRING:
      case INSN_BSTRING:
      case INSN_STRING_POPSRC:
        {
          os_uint32 maxn, n;
          char *str;
          if (insn.opcode != INSN_BSTRING)
            maxn = ~(os_uint32)0;
          else
          {
            maxn = *((unsigned *) xs);
            xs++;
          }
          DESERPROG_READ_COUNT (n);
          SRC_CHECK_ALIGN (n, 1);
          if (n == 0) {
            str = NULL;
          } else if (n > maxn || n > (os_uint32) (srclimit - src) || src[n-1] != 0) {
            return SD_CDR_INVALID;
          } else if ((str = c_stringMalloc_s (prog->base, n)) == NULL) {
            return SD_CDR_OUT_OF_MEMORY;
          } else {
            memcpy (str, src, n);
            src += n;
          }
          *((c_string *) dst) = str;
          if (insn.opcode != INSN_STRING_POPSRC)
            dst += sizeof (char *);
          else
          {
            dst = (char *) *--stk;
            xs += insn.count;
          }
        }
        break;
      case INSN_POPSRC:
        dst = (char *) *--stk;
        xs += insn.count;
        break;
      case INSN_SRCADV:
        src += insn.count;
        break;
      case INSN_PRIM1_LOOPSTAR:
        DESERPROG_EXEC_MULTIPLE1 (loopcount);
        loopcount = (unsigned) *--stk;
        dst = (char *) *--stk;
        break;
      case INSN_PRIM2_LOOPSTAR:
        DESERPROG_EXEC_MULTIPLE (2, loopcount);
        loopcount = (unsigned) *--stk;
        dst = (char *) *--stk;
        break;
      case INSN_PRIM4_LOOPSTAR:
        DESERPROG_EXEC_MULTIPLE (4, loopcount);
        loopcount = (unsigned) *--stk;
        dst = (char *) *--stk;
        break;
      case INSN_PRIM8_LOOPSTAR:
        DESERPROG_EXEC_MULTIPLE (8, loopcount);
        loopcount = (unsigned) *--stk;
        dst = (char *) *--stk;
        break;
      case INSN_PUSHCOUNT:
        *stk++ = loopcount;
        loopcount = insn.count;
        break;
      case INSN_PUSHSTAR:
        {
          unsigned n;
          DESERPROG_READ_COUNT (n);
          if (n == 0)
          {
            *((void **) dst) = NULL;
            dst += sizeof (void *);
            /* offset includes c_type pointer */
            xs += insn.count;
#ifdef PROGEXEC_TRACE
            indent -= 2;
#endif
          }
          else
          {
            const unsigned maxn = *((const unsigned *) xs);
            const struct c_collectionType_s *seqtype = extract_pointer_from_code ((const char *) (xs + 1));
            void *ary;
            if (n > maxn || n > (unsigned) (srclimit - src))
              return SD_CDR_INVALID;
            if ((ary = c_newBaseArrayObject_s ((c_collectionType) seqtype, n)) == NULL)
              return SD_CDR_OUT_OF_MEMORY;
            xs += 1 + sizeof (void *) / 4;
            *((void **) dst) = ary;
            *stk++ = (os_address) dst + sizeof (void *);
            *stk++ = loopcount;
            dst = ary;
            loopcount = n;
          }
        }
        break;
      case INSN_LOOP:
        if (--loopcount != 0)
          xs -= insn.count;
        else
          loopcount = (unsigned) *--stk;
        break;
      case INSN_LOOPSTAR:
        if (--loopcount != 0)
          xs -= insn.count;
        else
        {
          loopcount = (unsigned) *--stk;
          dst = (char *) *--stk;
        }
        break;
      case INSN_PRIM1_DISPATCH_LIST:
        DESERPROG_EXEC_DISPATCH (1, list4);
        break;
      case INSN_PRIM2_DISPATCH_LIST:
        DESERPROG_EXEC_DISPATCH (2, list4);
        break;
      case INSN_PRIM4_DISPATCH_LIST:
        DESERPROG_EXEC_DISPATCH (4, list4);
        break;
      case INSN_PRIM8_DISPATCH_LIST:
        DESERPROG_EXEC_DISPATCH (8, list8);
        break;
      case INSN_PRIM1_DISPATCH_DIRECT:
        DESERPROG_EXEC_DISPATCH (1, direct);
        break;
      case INSN_PRIM2_DISPATCH_DIRECT:
        DESERPROG_EXEC_DISPATCH (2, direct);
        break;
      case INSN_PRIM4_DISPATCH_DIRECT:
        DESERPROG_EXEC_DISPATCH (4, direct);
        break;
      case INSN_PRIM8_DISPATCH_DIRECT:
        DESERPROG_EXEC_DISPATCH (8, direct);
        break;
      case INSN_STRING_TO_ARRAY:
        {
          char *str;
          unsigned n;
          SRC_CHECK_ALIGN (insn.count, 1);
          if (insn.count > 0) {
            n = (unsigned) os_strnlen (src, insn.count);
            if ((str = c_stringMalloc_s (prog->base, n + 1)) == NULL)
              return SD_CDR_OUT_OF_MEMORY;
            memcpy (str, src, n);
            str[n] = 0;
            src += insn.count;
          } else {
            str = NULL;
          }
          *((c_string *) dst) = str;
          dst += sizeof (char *);
        }
        break;
      case INSN_ARRAY_TO_STRING:
        {
          unsigned n;
          DESERPROG_READ_COUNT (n);
          SRC_CHECK_ALIGN (n, 1);
          memcpy (dst, src, (n <= insn.count) ? n : insn.count);
          dst += insn.count;
          src += n;
        }
        break;
      case INSN_PRIM1_CONST:
        /* Constants are not represented in the deserialized
         * representation, so we can skip them -- or we can require
         * that they have the expected value.  Given it is currently
         * only being used for an optional endianness marker, perhaps
         * it'd be wise to check
         */
        SRC_CHECK_ALIGN (1, 1);
        if (*src != insn.count)
          return SD_CDR_INVALID;
        src++;
        break;
      case INSN_JUMP:
        xs += insn.count;
        break;
      case INSN_CALL:
        *stk++ = (os_address) xs;
        xs -= insn.count;
        break;
      case INSN_REF_UNIQ:
        {
          unsigned char flag = (unsigned char) *src++;
          SRC_CHECK_ALIGN (1, 1);
          if (flag == 0)
          {
            *((void **) dst) = NULL;
            dst += sizeof (void *);
            /* offset includes c_type pointer */
            xs += insn.count;
          }
          else if (flag == 1)
          {
            const struct c_type_s *subtype = extract_pointer_from_code ((const char *) xs);
            void *obj;
            if ((obj = c_new_s ((c_type) subtype)) == NULL)
              return SD_CDR_OUT_OF_MEMORY;
            xs += sizeof (void *) / 4;
            *((void **) dst) = obj;
            *stk++ = (os_address) dst + sizeof (void *);
            dst = obj;
          }
          else
          {
            return SD_CDR_INVALID;
          }
        }
        break;
      case INSN_QUIETREF:
        *stk++ = (os_address) dst + sizeof (void *);
        dst = *((void **) dst);
        break;
      case INSN_RETURN:
        xs = (const struct insn_enc *) *--stk;
        break;
      case INSN_TAG_PREP:
      case INSN_TAG:
        break;
    }
  }
}

/****************** INTERFACE ********************/

static int sd_cdrSerdataInit (void *vsd, char **dst, os_uint32 size_hint)
{
  struct sd_cdrSerdata *sd = vsd;
  if ((sd->first = sd->last = sdblock_new (size_hint, sd->clear_padding)) == NULL)
    return SD_CDR_OUT_OF_MEMORY;
  *dst = sd->last->data;
  return (int) size_hint;
}

static int sd_cdrSerdataGrow (void *vsd, char **dst, os_uint32 size_hint)
{
  struct sd_cdrSerdata *sd = vsd;
  struct sd_cdrSerdataBlock *sb;
  const int size = (int) alignup_size_t (size_hint, 8192);
  if ((sb = sd_addblock (sd, *dst, (unsigned) size)) == NULL)
    return SD_CDR_OUT_OF_MEMORY;
  *dst = sb->data;
  return size;
}

static void sd_cdrSerdataFinalize (void *vsd, char *dst)
{
  sd_finalize (vsd, dst);
}

static os_uint32 sd_cdrSerdataGetpos (const void *vsd, const char *dst)
{
  const struct sd_cdrSerdata *sd = vsd;
  return (os_uint32) (sd->sersize + dst - sd->last->data);
}

static int sd_cdrTagField_notag (UNUSED_ARG (os_uint32 *tag), UNUSED_ARG (void *arg), UNUSED_ARG (enum sd_cdrTagType type), UNUSED_ARG (os_uint32 srcoff))
{
  return 0;
}

struct sd_cdrInfo *sd_cdrInfoNewControl (const struct c_type_s *type, const struct sd_cdrControl *control)
{
  struct sd_cdrInfo *ci = os_malloc (sizeof (*ci));
  ci->status = SD_CIS_FRESH;
  ci->clear_padding = 0;
  ci->ktype = c_keep ((c_type) type);
  ci->catsstac_head = ci->catsstac_tail = NULL;
  ci->quietref_head = ci->quietref_tail = NULL;
  ci->control = *control;
  return ci;
}

struct sd_cdrInfo *sd_cdrInfoNew (const struct c_type_s *type)
{
  struct sd_cdrControl ctrl;
  ctrl.init = sd_cdrSerdataInit;
  ctrl.grow = sd_cdrSerdataGrow;
  ctrl.finalize = sd_cdrSerdataFinalize;
  ctrl.getpos = sd_cdrSerdataGetpos;
  ctrl.tag = sd_cdrTagField_notag;
  ctrl.tag_arg = NULL;
  ctrl.process = 0; /* not needed if no fields tagged */
  ctrl.process_arg = NULL;
  return sd_cdrInfoNewControl (type, &ctrl);
}

void sd_cdrInfoClearPadding (struct sd_cdrInfo *ci)
{
  ci->clear_padding = 1;
}

void sd_cdrInfoFree (struct sd_cdrInfo *ci)
{
  while (ci->catsstac_head)
  {
    struct sd_catsstac *cs = ci->catsstac_head;
    ci->catsstac_head = cs->next;
    os_free (cs);
  }
  while (ci->quietref_head)
  {
    struct sd_quietref *cs = ci->quietref_head;
    ci->quietref_head = cs->next;
    os_free (cs);
  }
  if (ci->status == SD_CIS_READY)
  {
    os_free (ci->prog);
  }
  c_free (ci->ktype);
  os_free (ci);
}

int sd_cdrNoteCatsStac (struct sd_cdrInfo *ci, unsigned n, struct c_type_s const * const *typestack)
{
  /* typestack contains: structs, unions, arrays, sequences +
   * string(cats)/array(stac), i.e., the full path without
   * typedefs.
   */
  struct sd_catsstac *cs;
  cs = os_malloc (offsetof (struct sd_catsstac, typestack) + n * sizeof (*cs->typestack));
  cs->next = NULL;
  cs->n = n;
  memcpy ((void *) cs->typestack, typestack, n * sizeof (*cs->typestack));
  if (ci->catsstac_head)
    ci->catsstac_tail->next = cs;
  else
    ci->catsstac_head = cs;
  ci->catsstac_tail = cs;
  return 0;
}

int sd_cdrNoteQuietRef (struct sd_cdrInfo *ci, unsigned int n, struct c_type_s const * const *typestack)
{
    struct sd_quietref *cs;
    cs = os_malloc (offsetof (struct sd_quietref, typestack) + n * sizeof (*cs->typestack));
    cs->next = NULL;
    cs->n = n;
    memcpy ((void *) cs->typestack, typestack, n * sizeof (*cs->typestack));
    if (ci->quietref_head)
      ci->quietref_tail->next = cs;
    else
      ci->quietref_head = cs;
    ci->quietref_tail = cs;
    return 0;
}

int sd_cdrCompile (struct sd_cdrInfo *ci)
{
  struct ser_type *sertype;
  struct convtype_context ctx;
  int rc;

  switch (ci->status)
  {
    case SD_CIS_FRESH:
      break; /* first time */
    case SD_CIS_UNSUPPORTED:
      return SD_CDR_INVALID; /* no point in trying again */
    case SD_CIS_READY:
      return 0; /* doing it again would yield the same result */
  }

  ctx.ci = ci;
  ctx.catsstac = ci->catsstac_head;
  ctx.quietref = ci->quietref_head;
  ctx.next_label = 1;
  ctx.typestack_depth = 0;
  convtype_allocator_init (&ctx.alloc);

  if ((rc = convtype (&ctx, &sertype, ci->ktype, 0, &ci->control)) < 0)
  {
    convtype_allocator_fini (&ctx.alloc);
    return rc;
  }
#ifdef PRINTTYPE
  {
    char *n = c_metaScopedName (c_metaObject (ci->ktype));
    fprintf (stderr, "TYPE %s\n", n);
    os_free (n);
    printtype (stderr, sertype);
  }
#endif

  mmsz_calc (&ci->minsize, &ci->maxsize, sertype);
  ci->dynalloc = (ci->maxsize == 0 || (ci->maxsize / 4 > ci->minsize && ci->maxsize > 8192));
  if (ci->dynalloc)
    ci->initial_alloc = alignup_size_t (2 * ci->minsize, 8192);
  else
    ci->initial_alloc = alignup_size_t (ci->maxsize + 7, 8);
#if PRINTCOPY
  fprintf (stderr, "initial alloc: %"PA_PRIuSIZE"\n", ci->initial_alloc);
#endif
  if ((rc = lowertype (&ci->prog, sertype, ci->ktype, &ci->control)) < 0)
  {
    convtype_allocator_fini (&ctx.alloc);
    return rc;
  }
#ifdef PRINTPROG
  printprog (stderr, ci->prog);
#endif
  convtype_allocator_fini (&ctx.alloc);
  ci->status = SD_CIS_READY;
  return 0;
}

os_uint32 sd_cdrSerdataBlob (const void **blob, struct sd_cdrSerdata *serdata)
{
  size_t sz;
  if (sd_blob (serdata, blob, &sz) >= 0)
    return (os_uint32) sz;
  return 0;
}

void sd_cdrSerdataFree (struct sd_cdrSerdata *sd)
{
  sd_free (sd);
}

int sd_cdrSerializeControl (const struct sd_cdrInfo *ci, void *serdata, const void *data)
{
  return serprog_exec (serdata, ci->prog, data, (os_uint32) ci->initial_alloc);
}

int sd_cdrSerializeControlBSwap (const struct sd_cdrInfo *ci, void *serdata, const void *data)
{
  return serprog_exec_swap (serdata, ci->prog, data, (os_uint32) ci->initial_alloc);
}

int sd_cdrSerializeControlBE (const struct sd_cdrInfo *ci, void *serdata, const void *data)
{
#if BE_NEEDS_BSWAP
  return serprog_exec_swap (serdata, ci->prog, data, (os_uint32) ci->initial_alloc);
#else
  return serprog_exec (serdata, ci->prog, data, (os_uint32) ci->initial_alloc);
#endif
}

int sd_cdrSerializeControlLE (const struct sd_cdrInfo *ci, void *serdata, const void *data)
{
#if BE_NEEDS_BSWAP
  return serprog_exec (serdata, ci->prog, data, (os_uint32) ci->initial_alloc);
#else
  return serprog_exec_swap (serdata, ci->prog, data, (os_uint32) ci->initial_alloc);
#endif
}

struct sd_cdrSerdata *sd_cdrSerializeInternal (int (*f) (const struct sd_cdrInfo *, void *, const void *), const struct sd_cdrInfo *ci, const void *data)
{
  struct sd_cdrSerdata *sd;
  int rc;
  if ((sd = sd_new (ci)) == NULL)
    return NULL;
  if ((rc = f (ci, sd, data)) < 0)
  {
    sd_free (sd);
    return NULL;
  }
  return sd;
}

struct sd_cdrSerdata *sd_cdrSerialize (const struct sd_cdrInfo *ci, const void *data)
{
  return sd_cdrSerializeInternal (sd_cdrSerializeControl, ci, data);
}

struct sd_cdrSerdata *sd_cdrSerializeBSwap (const struct sd_cdrInfo *ci, const void *data)
{
  return sd_cdrSerializeInternal (sd_cdrSerializeControlBSwap, ci, data);
}

struct sd_cdrSerdata *sd_cdrSerializeBE (const struct sd_cdrInfo *ci, const void *data)
{
  return sd_cdrSerializeInternal (sd_cdrSerializeControlBE, ci, data);
}

struct sd_cdrSerdata *sd_cdrSerializeLE (const struct sd_cdrInfo *ci, const void *data)
{
  return sd_cdrSerializeInternal (sd_cdrSerializeControlLE, ci, data);
}


int sd_cdrDeserializeRaw (void *dst, const struct sd_cdrInfo *ci, os_uint32 sz, const void *src)
{
  return deserprog_exec (dst, ci->prog, sz, src);
}

int sd_cdrDeserializeRawBSwap (void *dst, const struct sd_cdrInfo *ci, os_uint32 sz, const void *src)
{
  return deserprog_exec_swap (dst, ci->prog, sz, src);
}

int sd_cdrDeserializeRawBE (void *dst, const struct sd_cdrInfo *ci, os_uint32 sz, const void *src)
{
#if BE_NEEDS_BSWAP
  return sd_cdrDeserializeRawBSwap (dst, ci, sz, src);
#else
  return sd_cdrDeserializeRaw (dst, ci, sz, src);
#endif
}

int sd_cdrDeserializeRawLE (void *dst, const struct sd_cdrInfo *ci, os_uint32 sz, const void *src)
{
#if BE_NEEDS_BSWAP
  return sd_cdrDeserializeRaw (dst, ci, sz, src);
#else
  return sd_cdrDeserializeRawBSwap (dst, ci, sz, src);
#endif
}

static int sd_cdrDeserializeObjectInternal (int (*f) (char *, const struct serprog *, os_uint32, const char *), void **dst, const struct sd_cdrInfo *ci, os_uint32 sz, const void *src)
{
  void *obj;
  int rc;
  if ((obj = c_new_s (ci->prog->ospl_type)) == NULL)
    return SD_CDR_OUT_OF_MEMORY;
  if ((rc = f (obj, ci->prog, sz, src)) < 0)
  {
    c_free (obj);
    return rc;
  }
  *dst = obj;
  return 0;
}

int sd_cdrDeserializeObject (void **dst, const struct sd_cdrInfo *ci, os_uint32 sz, const void *src)
{
  return sd_cdrDeserializeObjectInternal (deserprog_exec, dst, ci, sz, src);
}

int sd_cdrDeserializeObjectBSwap (void **dst, const struct sd_cdrInfo *ci, os_uint32 sz, const void *src)
{
  return sd_cdrDeserializeObjectInternal (deserprog_exec_swap, dst, ci, sz, src);
}

int sd_cdrDeserializeObjectBE (void **dst, const struct sd_cdrInfo *ci, os_uint32 sz, const void *src)
{
#if BE_NEEDS_BSWAP
  return sd_cdrDeserializeObjectInternal (deserprog_exec_swap, dst, ci, sz, src);
#else
  return sd_cdrDeserializeObjectInternal (deserprog_exec, dst, ci, sz, src);
#endif
}

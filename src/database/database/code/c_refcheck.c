#include <stddef.h>
#include "os_stdlib.h"
#include "os_report.h"
#include "os_abstract.h"
#include "os_atomics.h"
#include "ut_avl.h"
#include "c_mmbase.h"
#include "c__mmbase.h"
#include "c__scope.h"
#include "c__base.h"
#include "c_collection.h"
#include "c__collection.h"
#include "c__field.h"
#include "c__metabase.h"
#include "c__refcheck.h"
#include "q__parser.h"
#include "c_module.h"

struct mlnode {
    ut_avlNode_t avlnode;
    const void *obj; /* object address */
    c_ulong refc; /* expected reference count */
    c_ulong objc; /* object count, for types */
};

struct mlnode_block {
    struct mlnode_block *next;
    int n;
    struct mlnode nodes[4096];
};

struct mlnode_allocator {
    struct mlnode_block *first, *last;
};

struct marker_state {
    ut_avlTree_t mltree;
    struct mlnode_allocator allocator;
    unsigned depth;
};

static int voidp_compare (const void *va, const void *vb)
{
    return (va == vb) ? 0 : ((char *) va < (char *) vb) ? -1 : 1;
}

static const ut_avlTreedef_t mltree_td =
    UT_AVL_TREEDEF_INITIALIZER_INDKEY (offsetof (struct mlnode, avlnode),
                                       offsetof (struct mlnode, obj),
                                       voidp_compare, 0);

static struct mlnode_block *mlnode_block_new (void)
{
    struct mlnode_block *b = os_malloc (sizeof (*b));
    b->next = NULL;
    b->n = 0;
    return b;
}

static void mlnode_allocator_init (struct mlnode_allocator *mla)
{
    mla->first = mla->last = mlnode_block_new ();
}

static void mlnode_allocator_fini (struct mlnode_allocator *mla)
{
    struct mlnode_block *b;
    while ((b = mla->first) != NULL) {
        mla->first = b->next;
        os_free (b);
    }
}

static struct mlnode *mlnode_new (struct mlnode_allocator *mla)
{
    if (mla->last->n == (int) (sizeof (mla->last->nodes) / sizeof (mla->last->nodes[0]))) {
        mla->last = mla->last->next = mlnode_block_new ();
    }
    return &mla->last->nodes[mla->last->n++];
}

static void error (const char *fmt, ...)
{
    char buf[1024];
    va_list ap;
    va_start (ap, fmt);
    (void)os_vsnprintf (buf, sizeof (buf), fmt, ap);
    va_end (ap);
    fprintf (stderr, "%s\n", buf);
}

static char *fqtypename (const struct c_type_s *consttype)
{
    c_type type = (c_type) consttype;
    if (/* DISABLES CODE */ (1) || c_baseObjectKind (type) != M_CLASS) {
        return c_metaScopedName (c_metaObject (type));
    } else {
        c_class class = (c_class) type;
        char *ns[32], *n, *p;
        int l = 0, i;
        size_t sz;
        do {
            ns[l++] = c_metaScopedName (c_metaObject (class));
        } while ((class = class->extends) != NULL && l < (int) (sizeof (ns) / sizeof (ns[0])));
        sz = 0;
        for (i = 0; i < l; i++) {
            sz += strlen (ns[i]) + 2;
        }
        if (class) {
            sz += 5;
        }
        n = p = os_malloc (sz);
        for (i = 0; i < l; i++) {
            p += sprintf (p, "%s<:", ns[i]);
            os_free (ns[i]);
        }
        if (class) {
            p += sprintf (p, "...<:");
        }
        p -= 2;
        *p = 0;
        return n;
    }
}

static void calc_objcount (struct marker_state *state)
{
    struct mlnode_block *b;
    for (b = state->allocator.first; b; b = b->next) {
        int i;
        for (i = 0; i < b->n; i++) {
            const struct c_type_s *type = c_getType ((void *) b->nodes[i].obj);
            struct mlnode *n = ut_avlLookup (&mltree_td, &state->mltree, type);
            if (n == NULL) {
                char *tn = fqtypename (type);
                error ("objc: object %p (type %p %s): type not visited", b->nodes[i].obj, (void *) type, tn);
                os_free (tn);
            } else {
                n->objc++;
            }
        }
    }
}

static void check_refcount (struct marker_state *state)
{
    struct mlnode_block *b;
    for (b = state->allocator.first; b; b = b->next) {
        int i;
        for (i = 0; i < b->n; i++) {
            struct mlnode *n = &b->nodes[i];
            const struct c_type_s *type = c_getType ((void *) n->obj);
            c_long refc = c_refCount ((void *) n->obj);
            char *tn = NULL;
            if (refc < 0 || (c_ulong) refc != n->refc + n->objc) {
                if (tn == NULL) { tn = fqtypename (type); }
                error ("refc: object %p (type %p %s): refcount %u != refc %u + objc %u", n->obj, (void *) type, tn, refc, n->refc, n->objc);
                if (strcmp (tn, "c_constant") == 0) {
                    const struct c_constant_s *c = n->obj;
                    error ("    : defs %s = %d", c->_parent.name, c_literal(c->operand)->value.is.Long);
                }
            }
            if (n->objc != 0) {
                if (!c_instanceOf ((void *) n->obj, "c_type")) {
                    if (tn == NULL) { tn = fqtypename (type); }
                    error ("refc: object %p (type %p %s): objc %u type not an instance of c_type", n->obj, (void *) type, tn, n->objc);
                } else if (n->objc != pa_ld32 (&c_type(n->obj)->objectCount)) {
                    if (tn == NULL) { tn = fqtypename (type); }
                    error ("refc: object %p (type %p %s): objcount %u != objc %u", n->obj, (void *) type, tn, pa_ld32 (&c_type(n->obj)->objectCount), n->objc);
                }
            }
            if (tn != NULL) { os_free (tn); }
        }
    }
}

static int note_ref (struct marker_state *state, const void *obj, int isobj);
static void collect_refs (struct marker_state *state, const struct c_type_s *type, const void *obj, int isobj);

static void collect_refs_array_raw (struct marker_state *state, const struct c_type_s *st, c_ulong n, const void *obj)
{
    os_size_t sz = c_typeIsRef ((c_type) st) ? sizeof (void *) : st->size, i;
    const char *ptr = obj;
    for (i = 0; i < n; i++) {
        collect_refs (state, st, ptr, 0);
        ptr += sz;
    }
}

static void collect_refs_array (struct marker_state *state, const struct c_collectionType_s *type, const void *obj)
{
    c_type st = type->subType;
    if (c_typeHasRef (st)) {
        collect_refs_array_raw (state, st, type->maxSize, obj);
    }
}

static void collect_refs_sequence (struct marker_state *state, const struct c_collectionType_s *type, const void *obj)
{
    c_type st = type->subType;
    if (c_typeHasRef (st)) {
        collect_refs_array_raw (state, st, c_arraySize ((c_array) obj), obj);
    }
}

static void collect_refs_scope_helper (c_metaObject obj, void *varg)
{
    const char *name = c_metaName (obj);
    struct marker_state *arg = varg;
    printf ("%*.*sm %s\n", 4*arg->depth, 4*arg->depth, "", name);
    (void) note_ref (arg, name, 1);
    collect_refs (arg, c_getType (c_object(obj)), obj, 1);
}

static c_bool collect_refs_coll_helper (void *obj, void *varg)
{
    collect_refs (varg, c_getType (obj), obj, 1);
    return 1;
}

static const void *deref (const void *obj)
{
    return *((void **) obj);
}

static void collect_refs_coll (struct marker_state *state, const struct c_collectionType_s *type, const void *obj, int isobj)
{
    switch (c_collectionType(type)->kind) {
        case OSPL_C_STRING:
            if (!isobj) { obj = deref (obj); }
            (void) note_ref (state, obj, 1);
            break;
        case OSPL_C_SEQUENCE:
            if (!isobj) { obj = deref (obj); }
            if (note_ref (state, obj, 1)) {
                collect_refs_sequence (state, type, obj);
            }
            break;
        case OSPL_C_ARRAY:
            if (!isobj) { obj = deref (obj); }

            if (type->maxSize) {
                collect_refs_array (state, type, obj);
            } else {
                if (note_ref (state, obj, 1)) {
                    collect_refs_sequence (state, type, obj);
                }
            }
            break;
        case OSPL_C_SCOPE:
            if (!isobj) { obj = deref (obj); }
            if (note_ref (state, obj, 1)) {
                c_scopeWalk ((c_scope) obj, collect_refs_scope_helper, state);
            }
            break;
        case OSPL_C_LIST:
        case OSPL_C_BAG:
        case OSPL_C_SET:
        case OSPL_C_DICTIONARY:
        case OSPL_C_QUERY:
            if (!isobj) { obj = deref (obj); }
            if (note_ref (state, obj, 1)) {
                (void)c_walk ((c_collection) obj, collect_refs_coll_helper, state);
            }
            break;
        default:
            break;
    }
}

static void collect_refs_typedef (struct marker_state *state, const struct c_typeDef_s *type, const void *obj, int isobj)
{
    /*printf ("TYPEDEF\n");*/
    if (note_ref (state, type->alias, isobj)) {
        collect_refs (state, type->alias, obj, isobj);
    }
}

static void collect_refs_struct (struct marker_state *state, const struct c_structure_s *type, const void *obj, int isobj)
{
    c_ulong i, size;
    c_member m;
    if (note_ref (state, obj, isobj)) {
        size = c_arraySize (type->members);
        for (i = 0; i < size; i++) {
            m = type->members[i];
            collect_refs (state, m->_parent.type, (char *) obj + m->offset, 0);
        }
    }
}

static c_value get_discriminant_value (const struct c_type_s * const dtype, const char *data)
{
    static c_value bug;
    switch (c_baseObjectKind (dtype)) {
        case M_PRIMITIVE:
            switch (c_primitiveKind ((c_type) dtype)) {
                case P_BOOLEAN:   return c_boolValue      (*((const c_bool *)      data));
                case P_CHAR:      return c_charValue      (*((const c_char *)      data));
                case P_SHORT:     return c_shortValue     (*((const c_short *)     data));
                case P_USHORT:    return c_ushortValue    (*((const c_ushort *)    data));
                case P_LONG:      return c_longValue      (*((const c_long *)      data));
                case P_ULONG:     return c_ulongValue     (*((const c_ulong *)     data));
                case P_LONGLONG:  return c_longlongValue  (*((const c_longlong *)  data));
                case P_ULONGLONG: return c_ulonglongValue (*((const c_ulonglong *) data));
                default: /* Unsupported type */ assert (0); return bug;
            }
            break;
        case M_ENUMERATION:
            return c_longValue (*(const c_long *) data);
        default: /* Unsupported type */
            assert (0);
            return bug;
    }
}

static const struct c_unionCase_s *active_union_case (const struct c_union_s *utype, const c_value dvalue)
{
    const struct c_unionCase_s *defcase = NULL;
    int i, j, n = (int) c_arraySize (utype->cases);
    for (i = 0; i < n; i++) {
        const struct c_unionCase_s * const c = c_unionCase(utype->cases[i]);
        unsigned nlab = c_arraySize (c->labels);
        if (nlab == 0) {
            defcase = c;
        } else {
            for (j = 0; j < (int) nlab; j++) {
                const struct c_literal_s * const label = c_literal(c->labels[j]);
                if (c_valueCompare (dvalue, label->value) == C_EQ) {
                    return c;
                }
            }
        }
    }
    return defcase;
}

static unsigned alignup (unsigned x, unsigned a)
{
  return -((-x) & (-a));
}

static void collect_refs_union (struct marker_state *state, const struct c_union_s *type, const void *obj, int isobj)
{
    if (note_ref (state, obj, isobj)) {
        const struct c_union_s * const utype = c_union ((c_type) type);
        const struct c_type_s * const dtype = c_typeActualType (utype->switchType);
        const c_value dvalue = get_discriminant_value (dtype, obj);
        const struct c_unionCase_s * const activecase = active_union_case (utype, dvalue);
        if (activecase) {
            const unsigned disp = alignup ((unsigned) dtype->size, (unsigned) c_type (utype)->alignment);
            const struct c_type_s * const subtype = c_typeActualType (c_specifierType(activecase));
            collect_refs (state, subtype, (const char *) obj + disp, 0);
        }
    }
}

struct collect_refs_class_helper_arg {
    struct marker_state *state;
    const char *obj;
};

static void collect_refs_class_helper (c_metaObject o, void *varg)
{
    struct collect_refs_class_helper_arg *arg = varg;
    switch (c_baseObjectKind (o)) {
        case M_ATTRIBUTE: case M_RELATION: case M_MEMBER: case M_UNIONCASE: {
                const struct c_property_s *p = c_property(o);
                ++arg->state->depth;
                printf ("%*.*sprop %s\n", 4*arg->state->depth, 4*arg->state->depth, "", p->_parent.name);
                collect_refs (arg->state, p->type, arg->obj + p->offset, 0);
                --arg->state->depth;
            }
            break;
        default:
            /*printf ("XX %d XX\n", (int) c_baseObjectKind (o));*/
            break;
    }
}

static void collect_refs_class (struct marker_state *state, const struct c_class_s *type, const void *obj, int isobj)
{
    if (!isobj) {
        /*printf ("(deref)\n");*/
        if ((obj = deref (obj)) != NULL) {
            type = (const struct c_class_s *) c_getType ((void *) obj);
        }
    }
    if (note_ref (state, obj, 1)) {
        struct collect_refs_class_helper_arg arg;
        arg.state = state;
        arg.obj = obj;
        /*printf ("(walk)\n");*/
        do {
            c_scopeWalk (c_interface(type)->scope, collect_refs_class_helper, &arg);
        } while ((type = type->extends) != NULL);
    } else {
        /*printf ("(skip)\n");*/
    }
}

static void collect_refs (struct marker_state *state, const struct c_type_s *type, const void *obj, int isobj)
{
    ++state->depth;
    switch (c_baseObjectKind (type)) {
        case M_PRIMITIVE:
            /*printf ("PRIM\n");*/
            break;
        case M_ENUMERATION:
            /*printf ("ENUM\n");*/
            break;
        case M_EXCEPTION:
        case M_STRUCTURE:
            /*printf ("EXCEPT or STRUCT\n");*/
            collect_refs_struct (state, c_structure(type), obj, isobj);
            break;
        case M_UNION:
            /*printf ("UNION\n");*/
            collect_refs_union (state, c_union(type), obj, isobj);
            break;
        case M_COLLECTION:
            /*printf ("COLL\n");*/
            collect_refs_coll (state, c_collectionType(type), obj, isobj);
            break;
        case M_CLASS:
            /*printf ("CLASS\n");*/
            collect_refs_class (state, c_class(type), obj, isobj);
            break;
        case M_TYPEDEF:
            /*printf ("TYPEDEF\n");*/
            collect_refs_typedef (state, c_typeDef(type), obj, isobj);
            break;
        case M_INTERFACE:
        case M_ANNOTATION:
            /* Objects should not be of an interface-type */
            assert(0);
            break;
        default:
            /* Function must only be called for type-objects */
            assert(0);
            break;
    }
    --state->depth;
}

static int note_ref (struct marker_state *state, const void *obj, int isobj)
{
    ut_avlIPath_t ipath;
    struct mlnode *n;

    if (obj == NULL) {
        assert (isobj);
        return 0;
    } else if (!isobj) {
        return 1;
    } else {
#if 0
        assert (c_isValidDatabaseObject ((void *) obj));
#endif
        if ((n = ut_avlLookupIPath (&mltree_td, &state->mltree, obj, &ipath)) != NULL) {
            n->refc++;
#if 1
            {
                c_type type = c_getType ((void *) obj);
                char *tn = fqtypename (type);
                int ismeta = c_instanceOf ((void *) obj, "c_metaObject");
                printf ("%*.*s[object %p (type %p %s)]%s%s\n", 4*state->depth, 4*state->depth, "", (void *) obj, (void *) type, tn, ismeta ? " name=" : "", ismeta ? ((c_metaObject)obj)->name : "");
                os_free (tn);
            }
#endif
            return 0;
        } else {
            c_type type = c_getType ((void *) obj);
#if 1
            {
                char *tn = fqtypename (type);
                int ismeta = c_instanceOf ((void *) obj, "c_metaObject");
                printf ("%*.*sobject %p (type %p %s)%s%s\n", 4*state->depth, 4*state->depth, "", (void *) obj, (void *) type, tn, ismeta ? " name=" : "", ismeta ? ((c_metaObject)obj)->name : "");
                os_free (tn);
            }
#endif
            n = mlnode_new (&state->allocator);
            n->obj = obj;
            n->refc = 1;
            n->objc = 0;
            ut_avlInsertIPath (&mltree_td, &state->mltree, n, &ipath);
            return 1;
        }
    }
}

void c__refcheckWalkRefs (int nroots, c_object roots[])
{
    struct marker_state state;
    int i;

    ut_avlInit (&mltree_td, &state.mltree);
    mlnode_allocator_init (&state.allocator);
    state.depth = 0;

    for (i = 0; i < nroots; i++) {
        collect_refs (&state, c_getType (roots[i]), roots[i], 1);
    }
    calc_objcount (&state);
    check_refcount (&state);

    mlnode_allocator_fini (&state.allocator);
}

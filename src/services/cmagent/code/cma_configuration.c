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
#include "vortex_os.h"

#include "cma__configuration.h"
#include "cma__service.h"
#include "cma__util.h"
#include "cma__log.h"

#include "ut_collection.h"
#include "cfg_parser.h"
#include "u_domain.h"
#include "u_cfNode.h"
#include "u_cfData.h"

#include "ut_avl.h"

#include "stddef.h"

struct cfgst; /* forward decl */

typedef enum q__schedPrioClass {
    Q__SCHED_PRIO_RELATIVE,
    Q__SCHED_PRIO_ABSOLUTE
} q__schedPrioClass;

C_STRUCT(cma_configuration)
{
    C_EXTENDS(cma_object);
    int valid;
    struct cfgst *cfgst;
    const os_char *serviceName;
    u_participant participant;

    /* Lease options */
    struct {
        os_timeReal expiry;
        os_float updateFactor;
    } serviceLease;

    /* Tracing options */
    struct {
        os_char *outputFileName;
        c_bool appendToFile;
        cma_logcat categories;
    } tracing;

    /* Watchdog sched options,
     * not used by service; user layer directly accesses
     * the configuration tree */
    os_schedClass watchdog_sched_class;
    os_int32 watchdog_sched_priority;
    q__schedPrioClass watchdog_sched_priority_class;
};

/******************* DDSI-DERIVED CONFIG STUFF *******************/
#ifdef __GNUC__
#define UNUSED_ARG(x) x __attribute__ ((unused))
#else
#define UNUSED_ARG(x) x
#endif

#define WARN_DEPRECATED_ALIAS 1
#define WARN_DEPRECATED_UNIT 1
#define MAX_PATH_DEPTH 10 /* max nesting level of configuration elements */

struct cfgelem;
struct cfgst;

typedef int (*init_fun_t) (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem);
typedef int (*update_fun_t) (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int first, const char *value);
typedef void (*free_fun_t) (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem);
typedef void (*print_fun_t) (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default);

struct unit {
    const char *name;
    os_int64 multiplier;
};

struct cfgelem {
    const char *name;
    const struct cfgelem *children;
    const struct cfgelem *attributes;
    int multiplicity;
    const char *defvalue; /* NULL -> no default */
    int relative_offset;
    int elem_offset;
    init_fun_t init;
    update_fun_t update;
    free_fun_t free;
    print_fun_t print;
    const char *description;
};

struct cfgst_nodekey {
    const struct cfgelem *e;
};

/* config_listelem must be an overlay for all used listelem types */
struct config_listelem {
    struct config_listelem *next;
};

struct cfgst_node {
    ut_avlNode_t avlnode;
    struct cfgst_nodekey key;
    int count;
    int failed;
    int is_default;
};

struct cfgst {
    ut_avlTree_t found;
    cma_configuration cfg;

    /* Servicename is used by uf_service_name to use as a default value
     when the supplied string is empty, which happens when the service
     is started without a Agent configuration item, i.e. when
     everything is left at the default. */
    const char *servicename;

    /* path_depth, isattr and path together control the formatting of
     error messages by cfg_error() */
    int path_depth;
    int isattr[MAX_PATH_DEPTH];
    const struct cfgelem *path[MAX_PATH_DEPTH];
    void *parent[MAX_PATH_DEPTH];
};

/* "trace" is special: it enables (nearly) everything */
static const char *logcat_names[] = {
    "fatal", "error", "warning", "config", "info", "trace", NULL
};
static const cma_logcat logcat_codes[] = {
    LOG_FATAL, LOG_ERROR, LOG_WARNING, LOG_CONFIG, LOG_INFO, LOG_TRACE, 0
};

static int cfgst_node_cmp (const void *va, const void *vb);
static const ut_avlTreedef_t cfgst_found_treedef = UT_AVL_TREEDEF_INITIALIZER (offsetof (struct cfgst_node, avlnode), offsetof (struct cfgst_node, key), cfgst_node_cmp, 0);

#define DU(fname) static int fname (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int first, const char *value)
DU (uf_nopstring);
DU (uf_boolean);
DU (uf_tracingOutputFileName);
DU (uf_verbosity);
DU (uf_logcat);
DU (uf_float);
DU (uf_timeReal);
DU (uf_int32);
DU (uf_sched_prio_class);
DU (uf_sched_class);
#undef DU

#define DF(fname) static void fname (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem)
DF (ff_free);
#undef DF

#define PF(fname) static void fname (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
PF (pf_nop);
PF (pf_string);
PF (pf_logcat);
PF (pf_boolean);
PF (pf_float);
PF (pf_timeReal);
PF (pf_int32);
PF (pf_sched_prio_class);
PF (pf_sched_class);
#undef PF

#define CO(name) ((int) offsetof (struct cma_configuration_s, name))
#define ABSOFF(name) 0, CO (name)
#define RELOFF(parent,name) 1, ((int) offsetof (struct parent, name))
#define NODATA 1, NULL, 0, 0, 0, 0, 0, 0
#define END_MARKER { NULL, NULL, NULL, NODATA, NULL }
#define WILDCARD { "*", NULL, NULL, NODATA, NULL }
#define LEAF(name) name, NULL, NULL
#define LEAF_W_ATTRS(name, attrs) name, NULL, attrs
#define GROUP(name, children) name, children, NULL, 1, NULL, 0, 0, 0, 0, 0, 0
#define MGROUP(name, children, attrs) name, children, attrs
#define ATTR(name) name, NULL, NULL

static const struct cfgelem tracing_cfgelems[] = {
    { LEAF ("EnableCategory"), 1, "", 0, 0, 0, uf_logcat, 0, pf_logcat,
        "<p>This element enables individual logging categories. These are enabled in addition to those enabled by Tracing/Verbosity. Recognised categories are:\n\
<ul><li><i>fatal</i>: all fatal errors, errors causing immediate termination</li>\n\
<li><i>error</i>: failures probably impacting correctness but not necessarily causing immediate termination</li>\n\
<li><i>warning</i>: abnormal situations that will likely not impact correctness</li>\n\
<li><i>config</i>: full dump of the configuration</li>\n\
<li><i>info</i>: general informational notices</li></ul>\n\
In addition, there is the keyword <i>trace</i> that enables all categories</p>" },
    { LEAF ("Verbosity"), 1, "none", 0, 0, 0, uf_verbosity, 0, pf_nop,
        "<p>This element enables standard groups of categories, based on a desired verbosity level. This is in addition to the categories enabled by the Tracing/EnableCategory setting. Recognised verbosity levels and the categories they map to are:\n\
<ul><li><i>none</i>: no Control and Monitoring Agent log</li>\n\
<li><i>severe</i>: error and fatal</li>\n\
<li><i>warning</i>: <i>severe</i> + warning</li>\n\
<li><i>info</i>: <i>warning</i> + general information messages</li>\n\
<li><i>config</i>: <i>info</i> + config</li>\n\
<li><i>fine</i>: equivalent to <i>config</i></li>\n\
<li><i>finest</i>: <i>fine</i> + trace</li></ul>\n\
While <i>none</i>prevents any message from being written to a Control and Monitoring Agent log file, warnings and errors are still logged in the ospl-info.log and ospl-error.log files.</p>" },
    { LEAF ("OutputFile"), 1, "cmagent.log", ABSOFF (tracing.outputFileName), 0, uf_tracingOutputFileName, ff_free, pf_string,
        "<p>This option specifies where the logging is printed to. Note that <i>stdout</i> and <i>stderr</i> are treated as special values, representing \"standard out\" and \"standard error\" respectively. No file is created unless logging categories are enabled using the Tracing/Verbosity or Tracing/EnabledCategory settings.</p>" },
    { LEAF ("AppendToFile"), 1, "false", ABSOFF (tracing.appendToFile), 0, uf_boolean, 0, pf_boolean,
        "<p>This option specifies whether the output is to be appended to an existing log file. The default is to create a new log file each time, which is generally the best option if a detailed log is generated.</p>" },
    END_MARKER
};

static const struct cfgelem sched_prio_cfgattrs[] = {
    { ATTR ("priority_kind"), 1, "relative", ABSOFF (watchdog_sched_priority_class), 0, uf_sched_prio_class, 0, pf_sched_prio_class,
        "<p>This attribute specifies whether the specified Priority is a relative or absolute priority.</p>" },
    END_MARKER
};

static const struct cfgelem sched_cfgelems[] = {
    { LEAF ("Class"), 1, "default", ABSOFF (watchdog_sched_class), 0, uf_sched_class, 0, pf_sched_class,
        "<p>This element specifies the thread scheduling class that will be used by the watchdog thread. The user may need the appropriate privileges from the underlying operating system to be able to assign some of the privileged scheduling classes.</p>" },
    { LEAF_W_ATTRS ("Priority", sched_prio_cfgattrs), 1, "0", ABSOFF (watchdog_sched_priority), 0, uf_int32, 0, pf_int32,
        "<p>This element specifies the thread priority. Only priorities that are supported by the underlying operating system can be assigned to this element. The user may need special privileges from the underlying operating system to be able to assign some of the privileged priorities.</p>" },
    END_MARKER
};

static const struct cfgelem watchdog_cfgelems[] = {
    { GROUP ("Scheduling", sched_cfgelems),
        "<p>This element specifies the type of OS scheduling class will be used by the thread that announces its liveliness periodically.</p>" },
    END_MARKER
};

static const struct cfgelem cma_cfgelems[] = {
    { GROUP ("Tracing", tracing_cfgelems),
        "<p>The Tracing element controls the amount and type of information that is written into the tracing log by the Control and Monitoring Agent service. This is useful to track the service during application development.</p>" },
    { GROUP ("Watchdog", watchdog_cfgelems),
        "<p>This element specifies the type of OS scheduling class will be used by the thread that announces its liveliness periodically.</p>" },
    END_MARKER
};

/* Note: using 2e-1 instead of 0.2 to avoid use of the decimal
 separator, which is locale dependent. */
static const struct cfgelem lease_expiry_time_cfgattrs[] = {
    { ATTR ("update_factor"), 1, "2e-1", ABSOFF (serviceLease.updateFactor), 0, uf_float, 0, pf_float, NULL },
    END_MARKER
};

static const struct cfgelem lease_cfgelems[] = {
    { LEAF_W_ATTRS ("ExpiryTime", lease_expiry_time_cfgattrs), 1, "10", ABSOFF (serviceLease.expiry), 0, uf_timeReal, 0, pf_timeReal, NULL },
    END_MARKER
};

static const struct cfgelem domain_cfgelems[] = {
    { GROUP ("Lease", lease_cfgelems), NULL },
    WILDCARD,
    END_MARKER
};

static const struct cfgelem cma_cfgattrs[] = {
    { ATTR ("name"), 1, "cmagent", ABSOFF (serviceName), 0, uf_nopstring, 0, pf_string,
        "<p>This attribute identifies the configuration for the Control and Monitoring Agent. Multiple service configurations can be specified in one single XML file. The actual applicable configuration is determined by the value of the name attribute, which must match the string specified in the element OpenSplice/Domain/Service[@name] in the Domain Service configuration.</p>" },
    END_MARKER
};

static const struct cfgelem root_cfgelems[] = {
    { "Agent", cma_cfgelems, cma_cfgattrs, NODATA,
        "<p>The root element of a Control and Monitoring Agent configuration.</p>" },
    { "Domain", domain_cfgelems, NULL, NODATA, NULL },
    END_MARKER
};

static const struct cfgelem root_cfgelem =
{ NULL, root_cfgelems, NULL, NODATA, "root" };

#undef ATTR
#undef GROUP
#undef LEAF_W_ATTRS
#undef LEAF
#undef WILDCARD
#undef END_MARKER
#undef NODATA
#undef RELOFF
#undef ABSOFF
#undef CO

static void cfgst_push (struct cfgst *cfgst, int isattr, const struct cfgelem *elem, void *parent)
{
    assert (cfgst->path_depth < MAX_PATH_DEPTH);
    assert (isattr == 0 || isattr == 1);
    cfgst->isattr[cfgst->path_depth] = isattr;
    cfgst->path[cfgst->path_depth] = elem;
    cfgst->parent[cfgst->path_depth] = parent;
    cfgst->path_depth++;
}

static void cfgst_pop (struct cfgst *cfgst)
{
    assert (cfgst->path_depth > 0);
    cfgst->path_depth--;
}

static const struct cfgelem *cfgst_tos (const struct cfgst *cfgst)
{
    assert (cfgst->path_depth > 0);
    return cfgst->path[cfgst->path_depth-1];
}

static void *cfgst_parent (const struct cfgst *cfgst)
{
    assert (cfgst->path_depth > 0);
    return cfgst->parent[cfgst->path_depth-1];
}

struct cfg_note_buf {
    size_t bufpos;
    size_t bufsize;
    char *buf;
};

static size_t cfg_note_vsnprintf (struct cfg_note_buf *bb, const char *fmt, va_list ap)
{
    int x;
    x = os_vsnprintf (bb->buf + bb->bufpos, bb->bufsize - bb->bufpos, fmt, ap);
    if (x >= 0 && (size_t)x >= bb->bufsize - bb->bufpos)
    {
        size_t nbufsize = ((bb->bufsize + (size_t)x + 1) + 1023) & (size_t)-1024;
        char *nbuf = os_realloc (bb->buf, nbufsize);
        bb->buf = nbuf;
        bb->bufsize = nbufsize;
        return nbufsize;
    }
    if (x < 0)
        CMA_FATAL ("cmagent_config", "cfg_note_vsnprintf: os_vsnprintf failed\n");
    else
        bb->bufpos += (size_t) x;
    return 0;
}

static void cfg_note_snprintf (struct cfg_note_buf *bb, const char *fmt, ...)
{
    /* The reason the 2nd call to os_vsnprintf is here and not inside
     cfg_note_vsnprintf is because I somehow doubt that all platforms
     implement va_copy() */
    va_list ap;
    size_t r;
    va_start (ap, fmt);
    r = cfg_note_vsnprintf (bb, fmt, ap);
    va_end (ap);
    if (r > 0)
    {
        int s;
        va_start (ap, fmt);
        s = os_vsnprintf (bb->buf + bb->bufpos, bb->bufsize - bb->bufpos, fmt, ap);
        if (s < 0 || (size_t) s >= bb->bufsize - bb->bufpos)
            CMA_FATAL ("cmagent_config", "cfg_note_snprintf: os_vsnprintf failed\n");
        va_end (ap);
        bb->bufpos += (size_t) s;
    }
}

static size_t cfg_note (struct cfgst *cfgst, cma_logcat cat, size_t bsz, const char *fmt, va_list ap)
{
    /* Have to snprintf our way to a single string so we can OS_REPORT
     as well as nn_log.  Otherwise configuration errors will be lost
     completely on platforms where stderr doesn't actually work for
     outputting error messages (this includes Windows because of the
     way "ospl start" does its thing). */
    struct cfg_note_buf bb;
    int i, sidx;
    size_t r;

    bb.bufpos = 0;
    bb.bufsize = (bsz == 0) ? 1024 : bsz;
    if ((bb.buf = os_malloc (bb.bufsize)) == NULL)
        CMA_FATAL ("cmagent_config", "cfg_note: out of memory\n");

    cfg_note_snprintf (&bb, "config: ");

    /* Path to element/attribute causing the error. Have to stop once an
     attribute is reached: a NULL marker may have been pushed onto the
     stack afterward in the default handling. */
    sidx = 0;
    while (sidx < cfgst->path_depth && cfgst->path[sidx]->name == NULL)
        sidx++;
    for (i = sidx; i < cfgst->path_depth && (i == sidx || !cfgst->isattr[i-1]); i++)
    {
        if (cfgst->path[i] == NULL)
        {
            assert (i > sidx);
            cfg_note_snprintf (&bb, "/#text");
        }
        else if (cfgst->isattr[i])
        {
            cfg_note_snprintf (&bb, "[@%s]", cfgst->path[i]->name);
        }
        else
        {
            const char *p = strchr (cfgst->path[i]->name, '|');
            int n = p ? (int) (p - cfgst->path[i]->name) : (int) strlen (cfgst->path[i]->name);
            cfg_note_snprintf (&bb, "%s%*.*s", (i == sidx) ? "" : "/", n, n, cfgst->path[i]->name);
        }
    }

    cfg_note_snprintf (&bb, ": ");
    if ((r = cfg_note_vsnprintf (&bb, fmt, ap)) > 0)
    {
        /* Can't reset ap ... and va_copy isn't widely available - so
         instead abort and hope the caller tries again with a larger
         initial buffer */
        os_free (bb.buf);
        return r;
    }

    switch (cat)
    {
        case LOG_CONFIG:
            cma_log (cat, "%s\n", bb.buf);
            break;
        case LOG_WARNING:
            CMA_WARNING ("cmagent_config", "%s\n", bb.buf);
            break;
        case LOG_ERROR:
            CMA_ERROR ("cmagent_config", "%s\n", bb.buf);
            break;
        default:
            CMA_FATAL ("cmagent_config", "cfg_note unhandled category %u for message %s\n", (unsigned) cat, bb.buf);
            break;
    }

    os_free (bb.buf);
    return 0;
}

#if WARN_DEPRECATED_ALIAS || WARN_DEPRECATED_UNIT
static void cfg_warning (struct cfgst *cfgst, const char *fmt, ...)
{
    va_list ap;
    size_t bsz = 0;
    do {
        va_start (ap, fmt);
        bsz = cfg_note (cfgst, LOG_WARNING, bsz, fmt, ap);
        va_end (ap);
    } while (bsz > 0);
}
#endif

static int cfg_error (struct cfgst *cfgst, const char *fmt, ...)
{
    va_list ap;
    size_t bsz = 0;
    do {
        va_start (ap, fmt);
        bsz = cfg_note (cfgst, LOG_ERROR, bsz, fmt, ap);
        va_end (ap);
    } while (bsz > 0);
    return 0;
}

static int cfg_log (struct cfgst *cfgst, const char *fmt, ...)
{
    va_list ap;
    size_t bsz = 0;
    do {
        va_start (ap, fmt);
        bsz = cfg_note (cfgst, LOG_CONFIG, bsz, fmt, ap);
        va_end (ap);
    } while (bsz > 0);
    return 0;
}

static int list_index (const char *list[], const char *elem)
{
    int i;
    for (i = 0; list[i] != NULL; i++)
    {
        if (os_strcasecmp (list[i], elem) == 0)
            return i;
    }
    return -1;
}

static void *cfg_address (UNUSED_ARG (struct cfgst *cfgst), void *parent, struct cfgelem const * const cfgelem)
{
    assert (cfgelem->multiplicity == 1);
    return (char *) parent + cfgelem->elem_offset;
}

static void *cfg_deref_address (UNUSED_ARG (struct cfgst *cfgst), void *parent, struct cfgelem const * const cfgelem)
{
    assert (cfgelem->multiplicity != 1);
    return *((void **) ((char *) parent + cfgelem->elem_offset));
}

static void ff_free (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem)
{
    void **elem = cfg_address (cfgst, parent, cfgelem);
    os_free (*elem);
}

static int uf_nopstring (UNUSED_ARG (struct cfgst *cfgst), UNUSED_ARG (void *parent), UNUSED_ARG (struct cfgelem const * const cfgelem), UNUSED_ARG (int first), UNUSED_ARG (const char *value))
{
    return 1;
}

static int uf_boolean (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
    static const char *vs[] = { "false", "true", NULL };
    int *elem = cfg_address (cfgst, parent, cfgelem);
    int idx = list_index (vs, value);
    if (idx < 0)
        return cfg_error (cfgst, "'%s': undefined value", value);
    else
    {
        *elem = idx;
        return 1;
    }
}

static int uf_float (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
    float *elem = cfg_address (cfgst, parent, cfgelem);
    char *endptr;
    float f = (float) strtod (value, &endptr);
    if (*value == 0 || *endptr != 0)
        return cfg_error (cfgst, "%s: not a floating point number", value);
    *elem = f;
    return 1;
}

static int uf_timeReal (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
    os_timeReal *elem = cfg_address (cfgst, parent, cfgelem);
    char *endptr;
    os_timeReal f = (os_timeReal) strtod (value, &endptr);
    if (*value == 0 || *endptr != 0)
        return cfg_error (cfgst, "%s: not a floating point number", value);
    *elem = f;
    return 1;
}

static int uf_int32 (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
    os_int32 *elem = cfg_address (cfgst, parent, cfgelem);
    char *endptr;
    long v = strtol (value, &endptr, 10);
    if (*value == 0 || *endptr != 0)
        return cfg_error (cfgst, "%s: not a decimal integer", value);
    if (v != (int) v)
        return cfg_error (cfgst, "%s: value out of range", value);
    *elem = (int) v;
    return 1;
}

static int uf_logcat (struct cfgst *cfgst, UNUSED_ARG (void *parent), UNUSED_ARG (struct cfgelem const * const cfgelem), UNUSED_ARG (int first), const char *value)
{
    static const char **vs = logcat_names;
    static const cma_logcat *lc = logcat_codes;
    char *copy = os_strdup (value), *cursor = copy, *tok;
    while ((tok = os_strsep (&cursor, ",")) != NULL)
    {
        int idx = list_index (vs, tok);
        if (idx < 0)
        {
            int ret = cfg_error (cfgst, "'%s' in '%s' undefined", tok, value);
            os_free (copy);
            return ret;
        }
        cfgst->cfg->tracing.categories |= lc[idx];
    }
    os_free (copy);
    return 1;
}

static int uf_verbosity (struct cfgst *cfgst, UNUSED_ARG (void *parent), UNUSED_ARG (struct cfgelem const * const cfgelem), UNUSED_ARG (int first), const char *value)
{
    static const char *vs[] = {
        "finest", "fine", "config", "info", "warning", "severe", "none", NULL
    };
    static const cma_logcat lc[] = {
        LOG_TRACE, 0, LOG_CONFIG, LOG_INFO, LOG_WARNING, LOG_ERROR | LOG_FATAL, 0, 0
    };
    int idx = list_index (vs, value);
    assert (sizeof (vs) / sizeof (*vs) == sizeof (lc) / sizeof (*lc));
    if (idx < 0)
        return cfg_error (cfgst, "'%s': undefined value", value);
    else
    {
        int i;
        for (i = (int) (sizeof (vs) / sizeof (*vs)) - 1; i >= idx; i--)
            cfgst->cfg->tracing.categories |= lc[i];
        return 1;
    }
}

static int uf_tracingOutputFileName (struct cfgst *cfgst, UNUSED_ARG (void *parent), UNUSED_ARG (struct cfgelem const * const cfgelem), UNUSED_ARG (int first), const char *value)
{
    cma_configuration cfg = cfgst->cfg;
    if (os_strcasecmp (value, "stdout") != 0 && os_strcasecmp (value, "stderr") != 0) {
        cfg->tracing.outputFileName = os_fileNormalize (value);
    } else {
        cfg->tracing.outputFileName = os_strdup (value);
    }
    return 1;
}

static int uf_sched_prio_class (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem,UNUSED_ARG (int first), const char *value)
{
    int ret;
    q__schedPrioClass *prio;

    assert (value != NULL);

    prio = cfg_address (cfgst, parent, cfgelem);

    if (os_strcasecmp (value, "relative") == 0) {
        *prio = Q__SCHED_PRIO_RELATIVE;
        ret = 1;
    } else if (os_strcasecmp (value, "absolute") == 0) {
        *prio = Q__SCHED_PRIO_ABSOLUTE;
        ret = 1;
    } else {
        ret = cfg_error (cfgst, "'%s': undefined value", value);
    }

    return ret;
}

static void pf_sched_prio_class (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
    char *str;
    q__schedPrioClass *prio = cfg_address (cfgst, parent, cfgelem);

    if (*prio == Q__SCHED_PRIO_RELATIVE) {
        str = "relative";
    } else if (*prio == Q__SCHED_PRIO_ABSOLUTE) {
        str = "absolute";
    } else {
        str = "INVALID";
    }

    cfg_log (cfgst, "%s%s", str, is_default ? " [def]" : "");
}

static int uf_sched_class (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
    static const char *vs[] = { "realtime", "timeshare", "default" };
    static const os_schedClass ms[] = { OS_SCHED_REALTIME, OS_SCHED_TIMESHARE, OS_SCHED_DEFAULT };
    int idx = list_index (vs, value);
    os_schedClass *elem = cfg_address (cfgst, parent, cfgelem);
    assert (sizeof (vs) / sizeof (*vs) == sizeof (ms) / sizeof (*ms));
    if (idx < 0)
        return cfg_error (cfgst, "'%s': undefined value", value);
    *elem = ms[idx];
    return 1;
}

static void pf_sched_class (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
    os_schedClass *p = cfg_address (cfgst, parent, cfgelem);
    const char *str = "INVALID";
    switch (*p)
    {
        case OS_SCHED_DEFAULT: str = "default"; break;
        case OS_SCHED_TIMESHARE: str = "timeshare"; break;
        case OS_SCHED_REALTIME: str = "realtime"; break;
    }
    cfg_log (cfgst, "%s%s", str, is_default ? " [def]" : "");
}


static void pf_float (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
    float *p = cfg_address (cfgst, parent, cfgelem);
    cfg_log (cfgst, "%f%s", *p, is_default ? " [def]" : "");
}

static void pf_timeReal (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
    os_timeReal *p = cfg_address (cfgst, parent, cfgelem);
    cfg_log (cfgst, "%f%s", *p, is_default ? " [def]" : "");
}

static void pf_int32 (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
    os_int32 *p = cfg_address (cfgst, parent, cfgelem);
    cfg_log (cfgst, "%d%s", *p, is_default ? " [def]" : "");
}

static int do_update (struct cfgst *cfgst, update_fun_t upd, void *parent, struct cfgelem const * const cfgelem, const char *value, int is_default)
{
    struct cfgst_node *n;
    struct cfgst_nodekey key;
    ut_avlIPath_t np;
    int ok;
    key.e = cfgelem;
    if ((n = ut_avlLookupIPath (&cfgst_found_treedef, &cfgst->found, &key, &np)) == NULL)
    {
        if ((n = os_malloc (sizeof (*n))) == NULL)
            return cfg_error (cfgst, "out of memory");

        n->key = key;
        n->count = 0;
        n->failed = 0;
        n->is_default = is_default;
        ut_avlInsertIPath (&cfgst_found_treedef, &cfgst->found, n, &np);
    }
    if (cfgelem->multiplicity == 0 || n->count < cfgelem->multiplicity)
        ok = upd (cfgst, parent, cfgelem, (n->count == n->failed), value);
    else
        ok = cfg_error (cfgst, "only %d instance(s) allowed",cfgelem->multiplicity);
    n->count++;
    if (!ok)
    {
        n->failed++;
    }
    return ok;
}

static int set_default (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem)
{
    if (cfgelem->defvalue == NULL)
        return cfg_error (cfgst, "element missing in configuration");
    return do_update (cfgst, cfgelem->update, parent, cfgelem, cfgelem->defvalue, 1);
}

static int set_defaults (struct cfgst *cfgst, void *parent, int isattr, struct cfgelem const * const cfgelem, int clear_found)
{
    const struct cfgelem *ce;
    int ok = 1;
    for (ce = cfgelem; ce && ce->name; ce++)
    {
        struct cfgst_node *n;
        struct cfgst_nodekey key;
        key.e = ce;
        cfgst_push (cfgst, isattr, ce, parent);
        if (ce->multiplicity == 1)
        {
            if (ut_avlLookup (&cfgst_found_treedef, &cfgst->found, &key) == NULL)
            {
                if (ce->update)
                {
                    int ok1;
                    cfgst_push (cfgst, 0, NULL, NULL);
                    ok1 = set_default (cfgst, parent, ce);
                    cfgst_pop (cfgst);
                    ok = ok && ok1;
                }
            }
            if ((n = ut_avlLookup (&cfgst_found_treedef, &cfgst->found, &key)) != NULL)
            {
                if (clear_found)
                {
                    ut_avlDelete (&cfgst_found_treedef, &cfgst->found, n);
                    os_free (n);
                }
            }
            if (ce->children)
            {
                int ok1 = set_defaults (cfgst, parent, 0, ce->children, clear_found);
                ok = ok && ok1;
            }
            if (ce->attributes)
            {
                int ok1 = set_defaults (cfgst, parent, 1, ce->attributes, clear_found);
                ok = ok && ok1;
            }
        }
        cfgst_pop (cfgst);
    }
    return ok;
}

static void pf_nop (UNUSED_ARG (struct cfgst *cfgst), UNUSED_ARG (void *parent), UNUSED_ARG (struct cfgelem const * const cfgelem), UNUSED_ARG (int is_default))
{
}

static void pf_string (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
    char **p = cfg_address (cfgst, parent, cfgelem);
    cfg_log (cfgst, "%s%s", *p ? *p : "(null)", is_default ? " [def]" : "");
}

static void pf_boolean (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
    int *p = cfg_address (cfgst, parent, cfgelem);
    cfg_log (cfgst, "%s%s", *p ? "true" : "false", is_default ? " [def]" : "");
}

static void pf_logcat (struct cfgst *cfgst, UNUSED_ARG (void *parent), UNUSED_ARG (struct cfgelem const * const cfgelem), UNUSED_ARG (int is_default))
{
    cma_configuration config = cfgst->cfg;
    cma_logcat remaining = config->tracing.categories;
    char res[256] = "", *resp = res;
    const char *prefix = "";
    size_t i;
#ifndef NDEBUG
    {
        size_t max;
        for (i = 0, max = 0; i < sizeof (logcat_codes) / sizeof (*logcat_codes); i++)
            max += logcat_codes[i] ? (1 + strlen (logcat_names[i])) : 0;
        max += 11; /* ,0x%x */
        max += 1; /* \0 */
        assert (max <= sizeof (res));
    }
#endif
    /* TRACE enables ALLCATS, all the others just one */
    if ((remaining & LOG_TRACE) == LOG_TRACE)
    {
        resp += sprintf (resp, "%strace", prefix);
        remaining &= ~LOG_TRACE;
        prefix = ",";
    }
    for (i = 0; i < sizeof (logcat_codes) / sizeof (*logcat_codes); i++)
    {
        if (remaining & logcat_codes[i])
        {
            resp += sprintf (resp, "%s%s", prefix, logcat_names[i]);
            remaining &= ~logcat_codes[i];
            prefix = ",";
        }
    }
    if (remaining)
    {
        resp += sprintf (resp, "%s0x%x", prefix, (unsigned) remaining);
    }
    assert (resp <= res + sizeof (res));
    /* can't do default indicator: user may have specified Verbosity, in
     which case EnableCategory is at default, but for these two
     settings, I don't mind. */
    cfg_log (cfgst, "%s", res);
}

static void print_configitems (struct cfgst *cfgst, void *parent, int isattr, struct cfgelem const * const cfgelem, int unchecked)
{
    const struct cfgelem *ce;
    for (ce = cfgelem; ce && ce->name; ce++)
    {
        struct cfgst_nodekey key;
        struct cfgst_node *n;
        key.e = ce;
        cfgst_push (cfgst, isattr, ce, parent);
        if (ce->multiplicity == 1)
        {
            if ((n = ut_avlLookup (&cfgst_found_treedef, &cfgst->found, &key)) != NULL)
            {
                cfgst_push (cfgst, 0, NULL, NULL);
                ce->print (cfgst, parent, ce, n->is_default);
                cfgst_pop (cfgst);
            }
            else
            {
                if (unchecked && ce->print)
                {
                    cfgst_push (cfgst, 0, NULL, NULL);
                    ce->print (cfgst, parent, ce, 0);
                    cfgst_pop (cfgst);
                }
            }

            if (ce->children)
                print_configitems (cfgst, parent, 0, ce->children, unchecked);
            if (ce->attributes)
                print_configitems (cfgst, parent, 1, ce->attributes, unchecked);
        }
        else
        {
            struct config_listelem *p = cfg_deref_address (cfgst, parent, ce);
            while (p)
            {
                cfgst_push (cfgst, 0, NULL, NULL);
                if (ce->print)
                {
                    ce->print (cfgst, p, ce, 0);
                }
                cfgst_pop (cfgst);
                if (ce->attributes)
                    print_configitems (cfgst, p, 1, ce->attributes, 1);
                if (ce->children)
                    print_configitems (cfgst, p, 0, ce->children, 1);
                p = p->next;
            }
        }
        cfgst_pop (cfgst);
    }
}

static void free_all_elements (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem)
{
    const struct cfgelem *ce;

    for (ce = cfgelem; ce && ce->name; ce++)
    {
        if (ce->free)
            ce->free (cfgst, parent, ce);

        if (ce->multiplicity == 1)
        {
            if (ce->children)
                free_all_elements (cfgst, parent, ce->children);
            if (ce->attributes)
                free_all_elements (cfgst, parent, ce->attributes);
        }
        else
        {
            struct config_listelem *p = cfg_deref_address (cfgst, parent, ce);
            struct config_listelem *r ;
            while (p) {
                if (ce->attributes)
                    free_all_elements (cfgst, p, ce->attributes);
                if (ce->children)
                    free_all_elements (cfgst, p, ce->children);
                r = p;
                p = p->next;
                os_free(r);
            }
        }
    }
}

static void free_configured_elements (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem)
{
    const struct cfgelem *ce;
    for (ce = cfgelem; ce && ce->name; ce++)
    {
        struct cfgst_nodekey key;
        struct cfgst_node *n;
        key.e = ce;
        if ((n = ut_avlLookup (&cfgst_found_treedef, &cfgst->found, &key)) != NULL)
        {
            if (ce->free && n->count > n->failed)
                ce->free (cfgst, parent, ce);
        }

        if (ce->multiplicity == 1)
        {
            if (ce->children)
                free_configured_elements (cfgst, parent, ce->children);
            if (ce->attributes)
                free_configured_elements (cfgst, parent, ce->attributes);
        }
        else
        {
            struct config_listelem *p = cfg_deref_address (cfgst, parent, ce);
            struct config_listelem *r;
            while (p)
            {
                if (ce->attributes)
                    free_all_elements (cfgst, p, ce->attributes);
                if (ce->children)
                    free_all_elements (cfgst, p, ce->children);
                r = p;
                p = p->next;
                os_free(r);
            }
        }
    }
}

static int matching_name_index (const char *name_w_aliases, const char *name)
{
    const char *ns = name_w_aliases, *p = strchr (ns, '|');
    int idx = 0;
    while (p)
    {
        if (os_strncasecmp (ns, name, (os_uint32) (p-ns)) == 0 && name[p-ns] == 0)
        {
            /* ns upto the pipe symbol is a prefix of name, and name is terminated at that point */
            return idx;
        }
        /* If primary name followed by '||' instead of '|', aliases are non-warning */
        ns = p + 1 + (idx == 0 && p[1] == '|');
        p = strchr (ns, '|');
        idx++;
    }
    return (os_strcasecmp (ns, name) == 0) ? idx : -1;
}

static int proc_elem_open (void *varg, const char *name)
{
    struct cfgst * const cfgst = varg;
    const struct cfgelem *cfgelem = cfgst_tos (cfgst);
    const struct cfgelem *cfg_subelem;
    if (cfgelem == NULL)
    {
        /* Ignoring, but do track the structure so we can know when to stop ignoring */
        cfgst_push (cfgst, 0, NULL, NULL);
        return 1;
    }
    for (cfg_subelem = cfgelem->children;
         cfg_subelem && cfg_subelem->name && strcmp (cfg_subelem->name, "*") != 0;
         cfg_subelem++)
    {
        int idx = matching_name_index (cfg_subelem->name, name);
#if WARN_DEPRECATED_ALIAS
        if (idx > 0)
        {
            int n = (int) (strchr (cfg_subelem->name, '|') - cfg_subelem->name);
            if (cfg_subelem->name[n+1] != '|')
            {
                cfg_warning (cfgst, "'%s': deprecated alias for '%*.*s'", name, n, n, cfg_subelem->name);
            }
        }
#endif
        if (idx >= 0)
        {
            break;
        }
    }
    if (cfg_subelem == NULL || cfg_subelem->name == NULL)
        return cfg_error (cfgst, "%s: unknown element", name);
    else if (strcmp (cfg_subelem->name, "*") == 0)
    {
        /* Push a marker that we are to ignore this part of the DOM tree */
        cfgst_push (cfgst, 0, NULL, NULL);
        return 1;
    }
    else
    {
        void *parent = cfgst_parent (cfgst);
        void *dynparent;

        assert (cfgelem->init || cfgelem->multiplicity == 1); /*multi-items must have an init-func */

        if (cfg_subelem->init)
        {
            if (cfg_subelem->init (cfgst, parent, cfg_subelem) < 0)
                return 0;
        }

        if (cfg_subelem->multiplicity != 1)
            dynparent = cfg_deref_address (cfgst, parent, cfg_subelem);
        else
            dynparent = parent;

        cfgst_push (cfgst, 0, cfg_subelem, dynparent);
        return 1;
    }
}

static int proc_attr (void *varg, const char *name, const char *value)
{
    /* All attributes are processed immediately after opening the element */
    struct cfgst * const cfgst = varg;
    const struct cfgelem *cfgelem = cfgst_tos (cfgst);
    const struct cfgelem *cfg_attr;
    if (cfgelem == NULL)
        return 1;
    for (cfg_attr = cfgelem->attributes; cfg_attr && cfg_attr->name; cfg_attr++)
    {
        if (os_strcasecmp (cfg_attr->name, name) == 0)
            break;
    }
    if (cfg_attr == NULL || cfg_attr->name == NULL)
        return cfg_error (cfgst, "%s: unknown attribute", name);
    else
    {
        void *parent = cfgst_parent (cfgst);
        int ok;
        cfgst_push (cfgst, 1, cfg_attr, parent);
        ok = do_update (cfgst, cfg_attr->update, parent, cfg_attr, value, 0);
        cfgst_pop (cfgst);
        return ok;
    }
}

static int proc_elem_data (void *varg, const char *value)
{
    struct cfgst * const cfgst = varg;
    const struct cfgelem *cfgelem = cfgst_tos (cfgst);
    if (cfgelem == NULL)
        return 1;
    if (cfgelem->update == 0)
        return cfg_error (cfgst, "%s: no data expected", value);
    else
    {
        void *parent = cfgst_parent (cfgst);
        int ok;
        cfgst_push (cfgst, 0, NULL, parent);
        ok = do_update (cfgst, cfgelem->update, parent, cfgelem, value, 0);
        cfgst_pop (cfgst);
        return ok;
    }
}

static int proc_elem_close (void *varg)
{
    struct cfgst * const cfgst = varg;
    const struct cfgelem * cfgelem = cfgst_tos (cfgst);
    int ok = 1;
    if (cfgelem && cfgelem->multiplicity != 1)
    {
        void *parent = cfgst_parent (cfgst);
        int ok1;
        ok1 = set_defaults (cfgst, parent, 1, cfgelem->attributes, 1);
        ok = ok && ok1;
        ok1 = set_defaults (cfgst, parent, 0, cfgelem->children, 1);
        ok = ok && ok1;
    }
    cfgst_pop (cfgst);
    return ok;
}

static int walk_element (struct cfgst *cfgst, const char *name, u_cfElement elem);

static int walk_attributes (struct cfgst *cfgst, u_cfElement base)
{
    c_iter iter;
    u_cfNode child;
    int ok = 1;
    iter = u_cfElementGetAttributes (base);
    child = u_cfNode (c_iterTakeFirst (iter));
    while (child)
    {
        u_cfAttribute attr;
        c_char *name, *value;
        int ok1 = 0;
        name = u_cfNodeName (child);
        assert (name != NULL);
        assert (u_cfNodeKind (child) == V_CFATTRIBUTE);
        attr = u_cfAttribute (child);
        if (!u_cfAttributeStringValue (attr, &value))
            ok1 = cfg_error (cfgst, "failed to extract data");
        else
        {
            ok1 = proc_attr (cfgst, name, value);
            os_free (value);
        }
        ok = ok && ok1;
        os_free (name);
        u_cfNodeFree (child);
        child = u_cfNode (c_iterTakeFirst (iter));
    }
    c_iterFree (iter);
    return ok;
}

static int walk_children (struct cfgst *cfgst, u_cfElement base)
{
    c_iter iter;
    u_cfNode child;
    int ok = 1;
    iter = u_cfElementGetChildren (base);
    child = u_cfNode (c_iterTakeFirst (iter));
    while (child)
    {
        c_char *child_name;
        int ok1 = 0;
        child_name = u_cfNodeName (child);
        assert (child_name != NULL);
        switch (u_cfNodeKind (child))
        {
            case V_CFELEMENT:
            {
                u_cfElement elem = u_cfElement (child);
                ok1 = walk_element (cfgst, child_name, elem);
                break;
            }
            case V_CFDATA:
            {
                u_cfData data = u_cfData (child);
                c_char *value;
                if (!u_cfDataStringValue (data, &value))
                    ok1 = cfg_error (cfgst, "failed to extract data");
                else
                {
                    if (strspn (value, " \t\r\n") != strlen (value))
                        ok1 = proc_elem_data (cfgst, value);
                    else
                        ok1 = 1;
                    os_free (value);
                }
                break;
            }
            default:
                abort ();
        }
        ok = ok && ok1;
        os_free (child_name);
        u_cfNodeFree (child);
        child = u_cfNode (c_iterTakeFirst (iter));
    }
    c_iterFree (iter);
    return ok;
}

static int walk_element (struct cfgst *cfgst, const char *name, u_cfElement elem)
{
    if (!proc_elem_open (cfgst, name))
        return 0;
    else
    {
        int ok;
        ok = walk_attributes (cfgst, elem) && walk_children (cfgst, elem);
        if (!proc_elem_close (cfgst))
            ok = 0;
        return ok;
    }
}

static int cfgst_node_cmp (const void *va, const void *vb)
{
    return memcmp (va, vb, sizeof (struct cfgst_nodekey));
}

struct cfgst * config_init(u_participant participant, const char *servicename, cma_configuration config)
{
    /* pre: all parameters in config should be 0 */
    int ok = 1;
    struct cfgst *cfgst;
    u_cfElement root, elem;
    c_iter iter;
    int rootidx;
    assert (participant != NULL);

    config->valid = 0;

    cfgst = os_malloc (sizeof (*cfgst));
    memset (cfgst, 0, sizeof (*cfgst));

    ut_avlInit (&cfgst_found_treedef, &cfgst->found);
    cfgst->cfg = config;
    cfgst->servicename = servicename;

    if ((root = u_participantGetConfiguration ((u_participant) participant)) == NULL)
    {
        CMA_ERROR ("cmagent_config", "config_init: u_participantGetConfiguration failed");
        ut_avlFree (&cfgst_found_treedef, &cfgst->found, os_free);
        os_free (cfgst);
        return NULL;
    }

    /* Only suitable for Domain (without a attributes) and a service
     with a matching name attribute */
    cfgst_push (cfgst, 0, &root_cfgelem, config);
    for (rootidx = 0; root_cfgelems[rootidx].name; rootidx++)
    {
        const struct cfgelem *root_cfgelem = &root_cfgelems[rootidx];
        char *copy = os_strdup (root_cfgelem->name), *cursor = copy, *tok;
        while ((tok = os_strsep (&cursor, "|")) != NULL)
        {
            iter = u_cfElementXPath (root, tok);
            elem = u_cfElement (c_iterTakeFirst (iter));
            while (elem)
            {
                c_char *str;
                if (root_cfgelem->attributes == NULL)
                {
                    /* Domain element */
                    int ok1;
                    char *name = u_cfNodeName (u_cfNode (elem));
                    ok1 = walk_element (cfgst, name, elem);
                    os_free (name);
                    ok = ok && ok1;
                }
                else if (u_cfElementAttributeStringValue (elem, "name", &str))
                {
                    int ok1;
                    if (os_strcasecmp (servicename, str) != 0)
                        ok1 = 1;
                    else
                    {
                        char *name = u_cfNodeName (u_cfNode (elem));
                        ok1 = walk_element (cfgst, name, elem);
                        os_free (name);
                    }
                    ok = ok && ok1;
                    os_free (str);
                }
                u_cfElementFree (elem);
                elem = u_cfElement (c_iterTakeFirst (iter));
            }
            c_iterFree (iter);
        }
        os_free (copy);
    }
    cfgst_pop (cfgst);
    u_cfElementFree (root);

    /* Set defaults for everything not set that we have a default value
     for, signal errors for things unset but without a default. */
    {
        int ok1 = set_defaults (cfgst, cfgst->cfg, 0, root_cfgelems, 0);
        ok = ok && ok1;
    }

    if (!ok)
    {
        free_configured_elements (cfgst, cfgst->cfg, root_cfgelems);
    }

    if (ok)
    {
        config->valid = 1;
        return cfgst;
    }
    else
    {
        ut_avlFree (&cfgst_found_treedef, &cfgst->found, os_free);
        os_free (cfgst);
        return NULL;
    }
}

void config_print_and_free_cfgst (struct cfgst *cfgst)
{
    if (cfgst == NULL)
        return;
    print_configitems (cfgst, cfgst->cfg, 0, root_cfgelems, 0);
    ut_avlFree (&cfgst_found_treedef, &cfgst->found, os_free);
    os_free (cfgst);
}

void config_fini (cma_configuration config)
{
    if (config->valid)
    {
        struct cfgst cfgst;
        cfgst.cfg = config;
        free_all_elements (&cfgst, cfgst.cfg, root_cfgelems);
    }
}

/***************** END DDSI-DERIVED CONFIG STUFF *****************/

static void
cma__configurationDeinit(
    cma_configuration _this) __nonnull_all__;

cma_configuration
cma_configurationNew(
    cma_service service)
{
    cma_configuration _this;

    cma_objectIsValidKind(service, CMA_OBJECT_SERVICE);

    _this = os_malloc(sizeof(*_this));
    cma__objectInit(cma_object(_this), CMA_OBJECT_CONFIGURATION, (cma_objectDeinitFunc)cma__configurationDeinit);

    _this->valid = 0;
    _this->participant = cma_serviceParticipant(service);
    _this->serviceName = cma_serviceName(service); /* weak ref, cma_service outlives cma_configuration */

    _this->cfgst = config_init(_this->participant, _this->serviceName, _this);
    if (!_this->cfgst) {
        os_free(_this);
        return NULL;
    } else {
        return _this;
    }
}

static void
cma__configurationDeinit(
    cma_configuration _this)
{
    assert(_this);

    /* TODO deinit config */

    cma__objectDeinit(cma_object(_this));
}

void
cma_configurationPrint(
    cma_configuration _this)
{
    config_print_and_free_cfgst(_this->cfgst);
    _this->cfgst = NULL;
}

void
cma_logConfigInit(
    cma_logConfig _this)
{
    assert(_this);

    /* Trace output to stderr until configuration is read */
    _this->tracing.categories = LOG_ERROR | LOG_FATAL;
    _this->tracing.file = stderr;
}

void
cma_logConfigDeinit(
    cma_logConfig _this)
{
    int rc;

    assert(_this);

    if (_this->tracing.file) {
        rc = fclose(_this->tracing.file);
        if (rc != 0) {
            os_int err = os_getErrno();
            OS_REPORT(OS_WARNING, "cma_logConfigDeinit", err,
                "Failed to close cmagent tracing file: %s",
                err ? os_strError(err) : "(unknown error)");

        }
    }
}

/***** Lease params *****/

os_duration
cma_configurationLeaseUpdateInterval(
    cma_configuration _this)
{
    cma_objectIsValidKind(_this, CMA_OBJECT_CONFIGURATION);

    return os_realToDuration(_this->serviceLease.updateFactor * _this->serviceLease.expiry);
}

os_duration
cma_configurationLeaseExpiryTime(
    cma_configuration _this)
{
    cma_objectIsValidKind(_this, CMA_OBJECT_CONFIGURATION);

    return os_realToDuration(_this->serviceLease.expiry);
}

/***** Tracing params *****/

os_char*
cma_configurationTracingFileName(
    cma_configuration _this)
{
    cma_objectIsValidKind(_this, CMA_OBJECT_CONFIGURATION);
    return _this->tracing.outputFileName;
}

cma_logcat
cma_configurationTracingCategories(
    cma_configuration _this)
{
    cma_objectIsValidKind(_this, CMA_OBJECT_CONFIGURATION);
    return _this->tracing.categories;
}

c_bool
cma_configurationTracingAppend(
    cma_configuration _this)
{
    cma_objectIsValidKind(_this, CMA_OBJECT_CONFIGURATION);
    return _this->tracing.appendToFile;
}


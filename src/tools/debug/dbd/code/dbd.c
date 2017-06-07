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

#include <ctype.h>

#include "../../common/code/dbg_common.c"
#include "v_entity.h"
#include "v_group.h"
#include "v_groupSet.h"
#include "u_observable.h"
#include "u_domain.h"
#include "os_report.h"

#include "c__mmbase.h"
#include "c__refcheck.h"

static void toolAction(v_public entity, c_voidp args);
static void initAction(v_public entity, c_voidp args);
static void lookupServicesAction(v_public entity, c_voidp arg);
static void lookupTopicAction(v_public entity, c_voidp arg);
static void lookupPartitionAction(v_public entity, c_voidp arg);
static void lookupParticipantsAction(v_public entity, c_voidp arg);
static void lookupGroupsAction(v_public entity, c_voidp arg);
static void lookupReadersWithFilterAction(v_public public, c_voidp arg);
static void lookupWritersWithFilterAction(v_public public, c_voidp arg);
static void lookupReadersAction(v_public entity, c_voidp arg);
static void lookupWritersAction(v_public entity, c_voidp arg);
static void lookupEntitiesAction(v_public entity, c_voidp arg);
static void lookupAddressAction(v_public entity, c_voidp arg);
static void lookupObjectsOfTypeAction(v_public entity, c_voidp arg);
static void print_object_history(v_public public, c_voidp arg);
static void scan_object(v_public public, c_voidp arg);
static void resolve_type(v_public public, c_voidp arg);
static void trace_object(v_public public, c_voidp arg);
static void trace_type(v_public public, c_voidp arg);

static void print_usage()
{
    printf("***************************************************************************************************\n");
    printf("DataBase Dump (dbd) is an experimental tool to print shm content to stdout.                        \n");
    printf("This tool operates read only on shm and doesn't perform any locking.                               \n");
    printf("Subsequently it may crash when reading unstable data.                                              \n");
    printf("---------------------------------------------------------------------------------------------------\n");
    printf("Usage:\n");
    printf("dbd> <addr> <enter>                                  /* show object content                      */\n");
    printf("dbd> s <enter>                                       /* show services                            */\n");
    printf("dbd> t [<expr>] <enter>                              /* show topics if name matches <expr>       */\n");
    printf("dbd> d [<expr>] <enter>                              /* show partitions if name matches <expr>   */\n");
    printf("dbd> p [<expr>] <enter>                              /* show participants if name matches <expr> */\n");
    printf("dbd> g [<expr>][<expr>] <enter>                      /* show groups if topic and partition       */\n");
    printf("                                                     /* names matches the expressions            */\n");
    printf("dbd> r [<addr>] <enter>                              /* show readers associated to entity <addr> */\n");
    printf("dbd> rf [<string>] <enter>                           /* show readers matching given readername   */\n");
    printf("dbd> w [<addr>] <enter>                              /* show writers associated to entity <addr> */\n");
    printf("dbd> wf [<string>] <enter>                           /* show writers matching given writername   */\n");
    printf("dbd> e [<addr>] <enter>                              /* show contained entities                  */\n");
    printf("dbd> f [<addr>] <enter>                              /* find all references to <addr>            */\n");
    printf("dbd> F [<addr>] <enter>                              /* find all objects of type <addr>          */\n");
    printf("dbd> l <enter>                                       /* show <address> history                   */\n");
    printf("dbd> i [<systemId>][<localId>] [<serial>] <enter>    /* show entry belonging to that gid         */\n");
    printf("dbd> ? <addr> <enter>                                /* show allocation context for <address>    */\n");
    printf("dbd> R type                                          /* resolve type                             */\n");
    printf("dbd> trace <addr>                                    /* trace free/keep of object at <addr>      */\n");
    printf("dbd> tracetype <addr>                                /* set trace on new objects of type <addr>  */\n");
    printf("dbd> q <enter>                                       /* quit                                     */\n");
    printf("Example:\n");
    printf("dbd> 0x200a2df8 <enter>                              /* Print content of shm address             */\n");
    printf("***************************************************************************************************\n");
}

C_CLASS(lookupGroupsArg);
C_STRUCT(lookupGroupsArg) {
    char *partitions;
    char *topics;
};

struct tgtdesc;

typedef void (*act_fn_t) (const struct tgtdesc *td, void (*action) (v_public p, void *arg), void *arg);

enum tgtkind {
    TK_LIVE,
    TK_DUMP
};

struct tgtdesc {
    c_mm mm;
    c_base base;
    v_kernel kernel;
    act_fn_t act;
    unsigned systemId;
    enum tgtkind kind;
    union {
        struct {
            char *uri;
            u_participant p;
        } live;
        struct {
            void *mem;
            size_t size;
        } dump;
    } u;
};

static void tgtactlive(const struct tgtdesc *td, void (*action) (v_public p, void *arg), void *arg)
{
    (void)u_observableAction(u_observable(td->u.live.p), action, arg);
}

static struct tgtdesc *tgtopenlive(const char *uri)
{
    struct tgtdesc *td;
    u_participantQos pqos;

    if (u_userInitialise() != U_RESULT_OK) {
        printf("Failed to initialise process.\n");
        return NULL;
    }

    td = os_malloc(sizeof(*td));
    td->act = tgtactlive;
    td->kind = TK_LIVE;
    td->u.live.uri = uri ? os_strdup(uri) : NULL;

    pqos = u_participantQosNew(NULL);
    td->u.live.p = u_participantNew(uri, U_DOMAIN_ID_ANY, 30, "dbd", pqos, TRUE);
    u_participantQosFree(pqos);
    if (td->u.live.p == NULL) {
        printf("Creation of participant failed. Is the OpenSplice system running?\n");
        OS_REPORT(OS_ERROR, "tool", 0, "Creation of participant failed.");
        os_free(td);
        return NULL;
    }

    {
        u_cfElement element;
        u_domain domain;
        v_gid gid;

        if ((element = u_participantGetConfiguration(td->u.live.p)) != NULL) {
            char *n = u_cfNodeName((u_cfNode)element);
            printf("Node name: %s\n", n);
            printf("Node kind: %d\n", u_cfNodeKind((u_cfNode)element));
            os_free(n);
            u_cfElementFree(element);
        } else {
            printf("No configuration for: %s\n", uri);
        }

        if ((domain = u_participantDomain(td->u.live.p)) != NULL) {
            printf("Domain memory address: 0x%" PA_PRIxADDR "\n", (os_address)u_domainMemoryAddress(domain));
            printf("Domain memory size: 0x%" PA_PRIxADDR "\n", (os_address)u_domainMemorySize(domain));
        } else {
            printf("No domain for Participant\n");
        }

        (void)u_observableAction(u_observable(td->u.live.p), initAction, td);
        printf("Kernel Address <0x%" PA_PRIxADDR ">\n", (os_address) td->kernel);
        gid = u_observableGid(u_observable(td->u.live.p));
        printf("Node GID: %u %u %u \n", gid.systemId, gid.localId, gid.serial);
        td->systemId = gid.systemId;
    }

    return td;
}

static void tgtclose(struct tgtdesc *td)
{
    switch (td->kind)
    {
        case TK_LIVE:
            u_objectFree(u_object(td->u.live.p));
            break;
        case TK_DUMP:
            abort();
            break;
    }
}

struct dbd_getline {
    char *buf;
    size_t size;
};

static void dbd_getline_init(struct dbd_getline *gl)
{
    gl->buf = NULL;
    gl->size = 0;
}

static void dbd_getline_fini(struct dbd_getline *gl)
{
    os_free(gl->buf);
}

static const char *dbd_getline(struct dbd_getline *gl)
{
    size_t pos = 0;
    int c;
    if (gl->buf == NULL) {
        gl->size = 512;
        gl->buf = os_malloc(gl->size);
    }
    while ((c = getchar()) != EOF && c != '\n') {
        if (pos + 1 == gl->size) {
            gl->buf = os_realloc(gl->buf, gl->size += 512);
        }
        gl->buf[pos++] = (char)c;
    }
    gl->buf[pos] = 0;
    return (c == EOF && pos == 0) ? NULL : gl->buf;
}

static char *skipspaces(const char *s)
{
    while (*s && isspace((unsigned char)*s)) {
        s++;
    }
    return (char*)s;
}

static char *skipword(const char *s)
{
    while (*s && !isspace((unsigned char)*s)) {
        s++;
    }
    return (char*)s;
}

struct word {
    char *buf;
    size_t pos;
};

static size_t getwords(size_t max, char **buf, struct word *ws, const char *s)
{
    char *ptr;
    size_t i, n = 0;
    *buf = ptr = os_strdup(s);
    ptr = skipspaces(ptr);
    while (*ptr && n < max) {
        ws[n].buf = ptr;
        ws[n].pos = (size_t)(ptr - *buf);
        n++;
        ptr = skipword(ptr);
        if (*ptr) {
            *ptr++ = 0;
            ptr = skipspaces(ptr);
        }
    }
    if (n == 0) {
        os_free(*buf);
        *buf = NULL;
    }
    for (i = n; i < max; i++) {
        ws[i].buf = NULL;
        ws[i].pos = 0;
    }
    return n;
}

static int gwaddr(void **addr, const struct word w)
{
    os_address x;
    int pos;
    if (w.buf == NULL) {
        return 0;
    } else if (sscanf(w.buf, "0x%" PA_PRIxADDR "%n", &x, &pos) != 1 || w.buf[pos] != 0) {
        return 0;
    } else {
        *addr = (void *)x;
        return 1;
    }
}

static int gwaddr_opt(void **addr, const struct word w)
{
    if (w.buf == NULL) {
        *addr = NULL;
        return 1;
    } else {
        return gwaddr(addr, w);
    }
}

static int gwuint(unsigned *uint, const struct word w)
{
    int pos;
    if (w.buf == NULL || sscanf(w.buf, "%u%n", uint, &pos) != 1 || w.buf[pos] != 0) {
        return 0;
    } else {
        return 1;
    }
}

static int gwgid(v_gid *gid, size_t nws, const struct word *ws)
{
    if (nws < 3) {
        return 0;
    } else if (gwuint(&gid->systemId, ws[0]) && gwuint(&gid->localId, ws[1]) && gwuint(&gid->serial, ws[2])) {
        return 1;
    } else {
        return 0;
    }
}

#ifdef INTEGRITY
int dbd_main(int argc, char *argv[])
#else
OPENSPLICE_MAIN(ospl_dbd)
#endif
{
    char *uri = NULL;

    struct tgtdesc *tgt;
    c_iter history;
    struct dbd_getline gl;

    int stop = 0;
    void *addr;

    if (argc > 1) {
        uri = argv[1];
    }

    if ((tgt = tgtopenlive(uri)) == NULL) {
        return 1;
    }

    history = NULL;
    dbd_getline_init(&gl);
    while (!stop) {
        const char *line;
        char *wsbuf;
        struct word ws[4];
        size_t nws;
        printf("dbd> "); fflush(stdout);
        if ((line = dbd_getline(&gl)) == NULL) {
            stop = 1;
            continue;
        } else if ((nws = getwords(sizeof(ws)/sizeof(ws[0]), &wsbuf, ws, line)) == 0) {
            continue;
        } else if (strncmp(ws[0].buf, "q", 1) == 0) {
            stop = 1;
        } else if (strcmp(ws[0].buf, "s") == 0) {
            tgt->act(tgt, lookupServicesAction, NULL);
        } else if (strcmp(ws[0].buf, "t") == 0) {
            tgt->act(tgt, lookupTopicAction, nws > 1 ? ws[1].buf : "*");
        } else if (strcmp(ws[0].buf, "d") == 0) {
            tgt->act(tgt, lookupPartitionAction, nws > 1 ? ws[1].buf : "*");
        } else if (strcmp(ws[0].buf, "p") == 0) {
            tgt->act(tgt, lookupParticipantsAction, nws > 1 ? ws[1].buf : "*");
        } else if (strcmp(ws[0].buf, "g") == 0) {
            C_STRUCT(lookupGroupsArg) arg;
            arg.partitions = nws > 1 ? ws[1].buf : "*";
            arg.topics = nws > 2 ? ws[2].buf : "*";
            tgt->act(tgt, lookupGroupsAction, &arg);
        } else if (strcmp(ws[0].buf, "r") == 0) {
            if (gwaddr_opt(&addr, ws[1])) {
                tgt->act(tgt, lookupReadersAction, addr);
            } else {
                printf("usage: r <address>\n");
            }
        } else if (strcmp(ws[0].buf, "rf") == 0) {
            if (nws == 1) {
                printf("usage: rf <string>\n");
            } else {
                tgt->act(tgt, lookupReadersWithFilterAction, (void *)(line + ws[1].pos));
            }
        } else if (strcmp(ws[0].buf, "w") == 0) {
            if (gwaddr_opt(&addr, ws[1])) {
                tgt->act(tgt, lookupWritersAction, addr);
            } else {
                printf("usage: w <address>\n");
            }
        } else if (strcmp(ws[0].buf, "wf") == 0) {
            if (nws == 1) {
                printf("usage: rf <string>\n");
            } else {
                tgt->act(tgt, lookupWritersWithFilterAction, (void *)(line + ws[1].pos));
            }
        } else if (strcmp(ws[0].buf, "e") == 0) {
            if (gwaddr(&addr, ws[1])) {
                tgt->act(tgt, lookupEntitiesAction, addr);
            } else {
                printf("usage: e <address>\n");
            }
        } else if (strcmp(ws[0].buf, "l") == 0) {
            c_iterWalk(history, printWalkHistory, NULL);
        } else if (strcmp(ws[0].buf, "i") == 0) {
            v_gid gid;
            if (tgt->kind != TK_LIVE) {
                printf("usage: i <systemId> <localId> <serial>: only supported when connected to a live domain\n");
            } else if (!gwgid(&gid, nws-1, ws+1)) {
                printf("usage: i <systemId> <localId> <serial>\n");
            } else {
                v_public vp = NULL;
                vp = v_gidClaim(gid, tgt->kernel);
                if (vp != NULL) {
                    tgt->act(tgt, toolAction, vp);
                    c_iterTake(history, vp); /* remove if already exist */
                    history = c_iterInsert(history, vp);
                    v_gidRelease(gid, tgt->kernel);
                } else {
                    if (tgt->systemId != gid.systemId) {
                        printf("Requested entity with systemId: %u is not present on local machine with systemId %u\n", gid.systemId, tgt->systemId);
                    } else {
                        printf("Could not find an entry for gid: %u %u %u\n", gid.systemId, gid.localId, gid.serial);
                    }
                }
            }
        } else if (strcmp(ws[0].buf, "?") == 0) {
            if (gwaddr(&addr, ws[1])) {
                tgt->act(tgt, print_object_history, addr);
            } else {
                printf("usage: ? <address>\n");
            }
        } else if (strcmp(ws[0].buf, "??") == 0) {
            if (gwaddr(&addr, ws[1])) {
                tgt->act(tgt, scan_object, addr);
            } else {
                printf("usage: ?? <address>\n");
            }
        } else if (strcmp(ws[0].buf, "f") == 0) {
            if (gwaddr(&addr, ws[1])) {
                tgt->act(tgt, lookupAddressAction, addr);
            } else {
                printf("usage: f <address>\n");
            }
        } else if (strcmp(ws[0].buf, "F") == 0) {
            if (gwaddr(&addr, ws[1])) {
                tgt->act(tgt, lookupObjectsOfTypeAction, addr);
            } else {
                printf("usage: F <address>\n");
            }
        } else if (strcmp(ws[0].buf, "R") == 0) {
            if (nws > 1) {
                tgt->act(tgt, resolve_type, (void *)(line + ws[1].pos));
            } else {
                printf("usage: R <typename>\n");
            }
        } else if (strcmp(ws[0].buf, "trace") == 0) {
            if (gwaddr(&addr, ws[1])) {
                tgt->act(tgt, trace_object, addr);
            } else {
                printf("usage: trace <address>\n");
            }
        } else if (strcmp(ws[0].buf, "tracetype") == 0) {
            if (gwaddr(&addr, ws[1])) {
                tgt->act(tgt, trace_type, addr);
            } else {
                printf("usage: trace <address>\n");
            }
        } else if (gwaddr(&addr, ws[0])) {
            tgt->act(tgt, toolAction, addr);
            c_iterTake(history, addr); /* remove if already exist */
            history = c_iterInsert(history, addr);
        } else {
            print_usage();
        }

        os_free(wsbuf);
    }
    dbd_getline_fini(&gl);
    tgtclose(tgt);
    return 0;
}

#if 0
static void printHistory (c_iter history, c_long cursor)
{
    c_type type;
    c_object o;
    c_char *name, *ename;

    o = c_iterObject(history,cursor-1);
    type = c_getType(o);
    name = c_metaScopedName(c_metaObject(type));
    printf("<0x%"PA_PRIxADDR"> %s",(os_address)o,name);
    if (c_checkType(o, "v_entity") == o) {
        ename = v_entityName(o);
        if (ename != NULL) {
            printf(" /* %s */", ename);
        }
    }
    printf("\n");
}
#endif

static void toolAction(v_public entity, c_voidp args)
{
    c_base base;
    c_type type;
    c_char *name;
    c_object o;
    c_address offset;
    os_size_t size;
    struct toolActionData actionData;

    actionData.fin = stdin;
    actionData.fout = stdout;
    actionData.depth = 0;
    actionData.stack = NULL;

    base = c_getBase(entity);
    o = c_baseCheckPtr(base, args);
    if (o) {
        type = c_getType(o);
        size = c_typeSize(type);
        if (o != args) {
            offset = C_ADDRESS(args) - C_ADDRESS(o);
            if (offset < (c_address)size) {
                printf("Warning: address is %" PA_PRIuADDR
                       " bytes in %s object starting at 0x%" PA_PRIxADDR "\n",
                       offset, _METANAME(type), (os_address)o);
                OBJECT_PUSH(&actionData, o);
                tryPrintOffset(o, &actionData, offset);
                OBJECT_POP(&actionData);
            } else {
                printf("Warning: address is %" PA_PRIuADDR
                       " bytes in memory starting at 0x%" PA_PRIxADDR "\n",
                       offset, (os_address)o);
            }
        } else {
            name = c_metaScopedName(c_metaObject(type));
            printf("Object <0x%" PA_PRIxADDR "> refCount=%d size=%" PA_PRIuSIZE " type is: <0x%" PA_PRIxADDR "> %s\n",
                   (os_address)o, c_refCount(o), size, (os_address)type, name);
            os_free(name);
            OBJECT_PUSH(&actionData, o);
            printType(type, &actionData);
            printf("\n");
            OBJECT_POP(&actionData);
        }
    } else {
        printf("Address <0x%" PA_PRIxADDR "> is not a Database Object\n", (os_address)args);
    }
}

static void initAction(v_public entity, c_voidp varg)
{
    struct tgtdesc *arg = varg;

    arg->kernel = v_objectKernel(entity);
    arg->base = c_getBase(arg->kernel);
    arg->mm = c_baseMM(arg->base);

    /* caching of type references for fast comparison. */
    v_handle_t = c_resolve(arg->base, "kernelModuleI::v_handle_s");
    v_topic_t = v_kernelType(arg->kernel, K_TOPIC);
    v_partition_t = v_kernelType(arg->kernel, K_DOMAIN);
}

static c_bool serviceAction(c_object _this, c_voidp arg)
{
    v_participant p = (v_participant)_this;

    OS_UNUSED_ARG(arg);
    if (_this) {
        if (v_objectKind(p) != K_PARTICIPANT) {
            printf("<0x%" PA_PRIxADDR "> /* %s */\n", (os_address)_this, v_entityName2(p));
        }
    }
    return TRUE;
}

static void lookupServicesAction(v_public entity, c_voidp arg)
{
    v_kernel kernel;

    OS_UNUSED_ARG(arg);
    kernel = v_objectKernel(entity);
    c_walk(kernel->participants, serviceAction, NULL);
}

static void lookupTopicAction(v_public entity, c_voidp arg)
{
    v_kernel kernel;
    c_iter list;
    c_voidp addr;

    kernel = v_objectKernel(entity);
    list = v_resolveTopics(kernel, arg);
    addr = c_iterTakeFirst(list);
    while (addr != NULL) {
        printf("<0x%" PA_PRIxADDR "> /* %s */\n", (os_address)addr, v_entityName2(addr));
        c_free(addr);
        addr = c_iterTakeFirst(list);
    }
    c_iterFree(list);
}

static void lookupPartitionAction(v_public entity, c_voidp arg)
{
    v_kernel kernel;
    c_iter list;
    c_voidp addr;

    kernel = v_objectKernel(entity);
    list = v_resolvePartitions(kernel, arg);
    addr = c_iterTakeFirst(list);
    while (addr != NULL) {
        printf("<0x%" PA_PRIxADDR "> /* %s */\n", (os_address)addr, v_entityName2(addr));
        c_free(addr);
        addr = c_iterTakeFirst(list);
    }
    c_iterFree(list);
}

static void lookupParticipantsAction(v_public entity, c_voidp arg)
{
    v_kernel kernel;
    c_iter list;
    c_voidp addr;

    kernel = v_objectKernel(entity);
    list = v_resolveParticipants(kernel, arg);
    addr = c_iterTakeFirst(list);
    while (addr != NULL) {
        printf("<0x%" PA_PRIxADDR "> /* %s */\n", (os_address)addr, v_entityName2(addr));
        c_free(addr);
        addr = c_iterTakeFirst(list);
    }
    c_iterFree(list);
}

static void lookupGroupsAction(v_public entity, c_voidp arg)
{
    v_kernel kernel;
    c_iter list;
    c_voidp addr;
    lookupGroupsArg a = (lookupGroupsArg)arg;

    kernel = v_objectKernel(entity);
    list = v_groupSetLookup(kernel->groupSet, a->partitions, a->topics);
    addr = c_iterTakeFirst(list);
    while (addr != NULL) {
        printf("<0x%" PA_PRIxADDR "> /* %s */\n", (os_address)addr, v_groupName(addr));
        c_free(addr);
        addr = c_iterTakeFirst(list);
    }
    c_iterFree(list);
}

static c_bool lookupReader(v_entry _this, c_voidp arg)
{
    c_iter *list = (c_iter *)arg;

    if ((_this) && (list != NULL)) {
        *list = c_iterInsert(*list, _this->reader);
    }
    return TRUE;
}

static c_bool lookupReaderAction(v_entity e, c_voidp arg)
{
    c_iter *list = (c_iter *)arg;

    if (e) {
        if (c_checkType(e, "v_reader") == e) {
            *list = c_iterInsert(*list, e);
        }
    }
    return TRUE;
}

static c_bool lookupSubscriberAction(v_entity e, c_voidp arg)
{
    c_iter *list = (c_iter *)arg;

    if (e) {
        if (v_objectKind(e) == K_SUBSCRIBER) {
            *list = c_iterInsert(*list, e);
        }
    }
    return TRUE;
}

static void lookupReadersWithFilterAction(v_public public, c_voidp arg)
{
    v_kernel kernel;
    c_iter list, alist, slist;
    c_object o;
    c_voidp addr;
    v_subscriber s;
    v_entity entity = v_entity(public);
    int found = 0;

    kernel = v_objectKernel(entity);

    list = NULL;
    alist = v_resolveParticipants(kernel, "*");
    o = c_iterTakeFirst(alist);
    while (o != NULL) {
        slist = NULL;
        v_entityWalkEntities(o, lookupSubscriberAction, &slist);
        s = c_iterTakeFirst(slist);
        while (s != NULL) {
            v_entityWalkEntities(v_entity(s), lookupReaderAction, &list);
            s = c_iterTakeFirst(slist);
        }
        c_iterFree(slist);
        c_free(o);
        o = c_iterTakeFirst(alist);
    }
    c_iterFree(alist);
    addr = c_iterTakeFirst(list);

    while (addr != NULL) {
        if (strstr(v_entityName2(addr), (char *)arg) != NULL) {
            printf("<0x%" PA_PRIxADDR "> /* %s */\n", (os_address)addr, v_entityName2(addr));
            found = 1;
        }
        addr = c_iterTakeFirst(list);
    }
    if (found == 0) {
        printf("No readers found matching: %s\n", (char *)arg);
    }
    c_iterFree(list);
    return;
}

static void lookupReadersAction(v_public entity, c_voidp arg)
{
    v_kernel kernel;
    c_iter list, alist, glist, slist;
    c_object o;
    c_base base;
    c_voidp addr;
    v_group g;
    v_subscriber s;

    kernel = v_objectKernel(entity);
    base = c_getBase(entity);
    if (arg == NULL) {
        list = NULL;
        alist = v_resolveParticipants(kernel, "*");
        o = c_iterTakeFirst(alist);
        while (o != NULL) {
            slist = NULL;
            v_entityWalkEntities(o, lookupSubscriberAction, &slist);
            s = c_iterTakeFirst(slist);
            while (s != NULL) {
                v_entityWalkEntities(v_entity(s), lookupReaderAction, &list);
                s = c_iterTakeFirst(slist);
            }
            c_iterFree(slist);
            c_free(o);
            o = c_iterTakeFirst(alist);
        }
        c_iterFree(alist);
        addr = c_iterTakeFirst(list);
        while (addr != NULL) {
            printf("<0x%" PA_PRIxADDR "> /* %s */\n", (os_address)addr, v_entityName2(addr));
            addr = c_iterTakeFirst(list);
        }
        c_iterFree(list);
        return;
    }
    o = c_baseCheckPtr(base, arg);
    if (o) {
        if (o == arg) {
            list = NULL;
            switch (v_objectKind(o)) {
                case K_GROUP:
                    v_groupWalkEntries(o, lookupReader, &list);
                    break;
                case K_DOMAIN:
                    glist = v_groupSetLookup(kernel->groupSet, v_partitionName(o), "*");
                    g = c_iterTakeFirst(glist);
                    while (g != NULL) {
                        v_groupWalkEntries(g, lookupReader, &list);
                        c_free(g);
                        g = c_iterTakeFirst(glist);
                    }
                    c_iterFree(glist);
                    break;
                case K_TOPIC:
                    glist = v_groupSetLookup(kernel->groupSet, "*", v_topicName(o));
                    g = c_iterTakeFirst(glist);
                    while (g != NULL) {
                        v_groupWalkEntries(g, lookupReader, &list);
                        c_free(g);
                        g = c_iterTakeFirst(glist);
                    }
                    c_iterFree(glist);
                    break;
                case K_SUBSCRIBER:
                    v_entityWalkEntities(o, lookupReaderAction, &list);
                    break;
                case K_PARTICIPANT:
                case K_SERVICE:
                case K_SPLICED:
                    slist = NULL;
                    v_entityWalkEntities(o, lookupSubscriberAction, &slist);
                    s = c_iterTakeFirst(slist);
                    while (s != NULL) {
                        v_entityWalkEntities(v_entity(s), lookupReaderAction, &list);
                        s = c_iterTakeFirst(slist);
                    }
                    c_iterFree(slist);
                    break;
                default:
                    printf("Lookup Reader for provided address 0x%" PA_PRIxADDR " is not supported\n", (os_address)arg);
                    break;
            }
            addr = c_iterTakeFirst(list);
            while (addr != NULL) {
                printf("<0x%" PA_PRIxADDR "> /* %s */\n", (os_address)addr, v_entityName2(addr));
                addr = c_iterTakeFirst(list);
            }
            c_iterFree(list);
        } else {
            printf("Given address 0x%" PA_PRIxADDR " is invalid\n", (os_address)arg);
        }
    } else {
        printf("Given address 0x%" PA_PRIxADDR " is invalid\n", (os_address)arg);
    }
}

static c_bool lookupWriterAction(v_entity e, c_voidp arg)
{
    c_iter *list = (c_iter *)arg;

    if (e) {
        if (c_checkType(e, "v_writer") == e) {
            *list = c_iterInsert(*list, e);
        }
    }
    return TRUE;
}

static c_bool lookupPublisherAction(v_entity e, c_voidp arg)
{
    c_iter *list = (c_iter *)arg;

    if (e) {
        if (v_objectKind(e) == K_PUBLISHER) {
            *list = c_iterInsert(*list, e);
        }
    }
    return TRUE;
}

static void lookupWritersWithFilterAction(v_public public, c_voidp arg)
{
    v_kernel kernel;
    c_iter list, plist, alist;
    c_object o;
    c_voidp addr;
    v_entity entity = v_entity(public);
    v_publisher p;
    int found = 0;

    kernel = v_objectKernel(entity);
    list = NULL;
    alist = v_resolveParticipants(kernel, "*");
    o = c_iterTakeFirst(alist);
    while (o != NULL) {
        plist = NULL;
        v_entityWalkEntities(o, lookupPublisherAction, &plist);
        p = c_iterTakeFirst(plist);
        while (p != NULL) {
            v_entityWalkEntities(v_entity(p), lookupWriterAction, &list);
            p = c_iterTakeFirst(plist);
        }
        c_iterFree(plist);
        c_free(o);
        o = c_iterTakeFirst(alist);
    }
    c_iterFree(alist);
    addr = c_iterTakeFirst(list);
    while (addr != NULL) {
        if (strstr(v_entityName2(addr), (char *)arg) != NULL) {
            printf("<0x%" PA_PRIxADDR "> /* %s */\n", (os_address)addr, v_entityName2(addr));
            found = 1;
        }
        addr = c_iterTakeFirst(list);
    }
    if (found == 0) {
        printf("No writers found matching: %s\n", (char *)arg);
    }
    c_iterFree(list);
    return;
}

static void lookupWritersAction(v_public entity, c_voidp arg)
{
    v_kernel kernel;
    c_iter list, glist, plist, alist;
    c_object o;
    c_base base;
    c_voidp addr;
    v_group g;
    v_publisher p;

    kernel = v_objectKernel(entity);
    base = c_getBase(entity);
    if (arg == NULL) {
        list = NULL;
        alist = v_resolveParticipants(kernel, "*");
        o = c_iterTakeFirst(alist);
        while (o != NULL) {
            plist = NULL;
            v_entityWalkEntities(o, lookupPublisherAction, &plist);
            p = c_iterTakeFirst(plist);
            while (p != NULL) {
                v_entityWalkEntities(v_entity(p), lookupWriterAction, &list);
                p = c_iterTakeFirst(plist);
            }
            c_iterFree(plist);
            c_free(o);
            o = c_iterTakeFirst(alist);
        }
        c_iterFree(alist);
        addr = c_iterTakeFirst(list);
        while (addr != NULL) {
            printf("<0x%" PA_PRIxADDR "> /* %s */\n", (os_address)addr, v_entityName2(addr));
            addr = c_iterTakeFirst(list);
        }
        c_iterFree(list);
        return;
    }
    o = c_baseCheckPtr(base, arg);
    if (o) {
        if (o == arg) {
            list = NULL;
            switch (v_objectKind(o)) {
                case K_GROUP:
                    /*                v_groupWalkWriters(o, lookupWriter,
                     * &list); */
                    printf("Lookup writer for Group not yet implemented\n");
                    break;
                case K_DOMAIN:
                    glist = v_groupSetLookup(kernel->groupSet, v_partitionName(o), "*");
                    g = c_iterTakeFirst(glist);
                    while (g != NULL) {
                        /*                    v_groupWalkWriters(g,
                         * lookupWriter, &list); */
                        printf("Lookup writer for Partition not yet implemented\n");
                        c_free(g);
                        g = c_iterTakeFirst(glist);
                    }
                    c_iterFree(glist);
                    break;
                case K_TOPIC:
                    glist = v_groupSetLookup(kernel->groupSet, "*", v_topicName(o));
                    g = c_iterTakeFirst(glist);
                    while (g != NULL) {
                        /*                    v_groupWalkWriters(g,
                         * lookupWriter, &list); */
                        printf("Lookup writer for Topic not yet implemented\n");
                        c_free(g);
                        g = c_iterTakeFirst(glist);
                    }
                    c_iterFree(glist);
                    break;
                case K_PUBLISHER:
                    v_entityWalkEntities(o, lookupWriterAction, &list);
                    break;
                case K_PARTICIPANT:
                case K_SERVICE:
                case K_SPLICED:
                    plist = NULL;
                    v_entityWalkEntities(o, lookupPublisherAction, &plist);
                    p = c_iterTakeFirst(plist);
                    while (p != NULL) {
                        v_entityWalkEntities(v_entity(p), lookupWriterAction, &list);
                        p = c_iterTakeFirst(plist);
                    }
                    c_iterFree(plist);
                    break;
                default:
                    printf("Lookup Writer for provided address 0x%" PA_PRIxADDR " is not supported\n", (os_address)arg);
                    break;
            }
            addr = c_iterTakeFirst(list);
            while (addr != NULL) {
                printf("<0x%" PA_PRIxADDR "> /* %s */\n", (os_address)addr, v_entityName2(addr));
                addr = c_iterTakeFirst(list);
            }
            c_iterFree(list);
        } else {
            printf("Given address 0x%" PA_PRIxADDR " is invalid\n", (os_address)arg);
        }
    } else {
        printf("Given address 0x%" PA_PRIxADDR " is invalid\n", (os_address)arg);
    }
}

static c_bool lookupEntityAction(v_entity e, c_voidp arg)
{
    c_iter *list = (c_iter *)arg;

    if (e) {
        if (c_checkType(e, "v_entity") == e) {
            *list = c_iterInsert(*list, e);
        }
    }
    return TRUE;
}

static void lookupEntitiesAction(v_public entity, c_voidp arg)
{
    c_iter list;
    c_object o;
    c_base base;
    c_voidp addr;

    base = c_getBase(entity);
    o = c_baseCheckPtr(base, arg);
    if (o) {
        if (o == arg) {
            list = NULL;
            v_entityWalkEntities(o, lookupEntityAction, &list);
            addr = c_iterTakeFirst(list);
            while (addr != NULL) {
                printf("<0x%" PA_PRIxADDR "> /* %s */\n", (os_address)addr, v_entityName2(addr));
                addr = c_iterTakeFirst(list);
            }
            c_iterFree(list);
        } else {
            printf("Given address 0x%" PA_PRIxADDR " is invalid\n", (os_address)arg);
        }
    } else {
        printf("Given address 0x%" PA_PRIxADDR " is invalid\n", (os_address)arg);
    }
}

struct lookupOffsetArg {
    os_address object;
    os_address container;
    os_size_t offset;
    c_type type;
    int printed;
};

static os_size_t propertySize(c_type type)
{
    switch (c_baseObjectKind(type)) {
        case M_COLLECTION:
            if ((c_collectionType(type)->kind == OSPL_C_ARRAY) && (c_collectionType(type)->maxSize != 0)) {
                assert(type->size > 0);
                return type->size;
            } else {
                return sizeof(void *);
            }
        case M_CLASS:
            return sizeof(void *);
        case M_TYPEDEF:
            return propertySize(c_typeDef(type)->alias);
        default:
            return type->size;
    }
}

static void lookupOffset(c_metaObject o, c_scopeWalkActionArg arg)
{
    struct lookupOffsetArg *a = (struct lookupOffsetArg *)arg;
    char *name, *typeName;

    switch (c_baseObjectKind(o)) {
        case M_CLASS:
            if (c_class(o)->extends != NULL) {
                c_metaWalk(c_metaObject(c_class(o)->extends), lookupOffset, arg);
            }
        case M_INTERFACE:
            c_metaWalk(c_metaObject(o), lookupOffset, arg);
            break;
        case M_STRUCTURE: {
            c_ulong length, i;
            length = c_arraySize(c_structure(o)->members);
            for (i = 0; i < length; i++) {
                c_member m = c_member(c_structure(o)->members[i]);
                if (m->offset <= a->offset && a->offset < m->offset + m->_parent.type->size) {
                    printf("Member %s +%" PA_PRIdSIZE "\n", m->_parent.name, a->offset - m->offset);
                    a->printed = 1;
                }
            }
            break;
        }
        case M_ATTRIBUTE:
        case M_RELATION:
            if (c_property(o)->offset <= a->offset &&
                a->offset < c_property(o)->offset + propertySize(c_property(o)->type)) {
                typeName = c_metaName(c_metaObject(a->type));
                name = c_metaScopedName(o);
                if (c_checkType(a->type, "kernelModuleI::v_entity")) {
                    printf("Used in %s %s (0x%" PA_PRIxADDR ") at offset %" PA_PRIuADDR " (member %s +%" PA_PRIdSIZE
                           ")\n",
                           v_entityName(o), typeName, a->container, a->offset, name, a->offset - c_property(o)->offset);
                    a->printed = 1;
                } else {
                    printf("Used in %s (0x%" PA_PRIxADDR ") at offset %" PA_PRIuADDR " (attribute %s +%" PA_PRIdSIZE
                           ")\n",
                           typeName, a->container, a->offset, name, a->offset - c_property(o)->offset);
                    a->printed = 1;
                }
                os_free(name);
                c_free(typeName);
            }
            break;
        case M_MEMBER:
            if (c_member(o)->offset <= a->offset && a->offset < c_member(o)->offset + c_member(o)->_parent.type->size) {
                typeName = c_metaName(c_metaObject(a->type));
                name = c_metaScopedName(o);
                if (c_checkType(a->type, "kernelModuleI::v_entity")) {
                    printf("Used in %s %s (0x%" PA_PRIxADDR ") at offset %" PA_PRIuADDR " (member %s +%" PA_PRIdSIZE
                           ")\n",
                           v_entityName(o), typeName, a->container, a->offset, name, a->offset - c_member(o)->offset);
                    a->printed = 1;
                } else {
                    printf("Used in %s (0x%" PA_PRIxADDR ") at offset %" PA_PRIuADDR " (member %s +%" PA_PRIdSIZE ")\n",
                           typeName, a->container, a->offset, name, a->offset - c_member(o)->offset);
                    a->printed = 1;
                }
                os_free(name);
                c_free(typeName);
            }
            break;
        case M_UNIONCASE:
        default:
            break;
    }
}

static void lookupAddressAction(v_public entity, c_voidp arg)
{
    c_object o, co;
    c_base base;
    os_address baseAddr, endAddr, cursor;

    base = c_getBase(entity);
    baseAddr = (os_address)c_baseMM(base);
    endAddr = baseAddr + (os_address)c_mmSize((c_mm)baseAddr);

    o = c_baseCheckPtr(base, arg);
    if (o) {
        if (o == arg) {
            cursor = baseAddr;
            while (cursor < endAddr) {
                if (*((c_object *)cursor) == o) {
                    co = c_baseCheckPtr(base, (c_object)cursor);
                    if (co != NULL) {
                        c_type type;
                        char *name = NULL;
                        os_address offset = cursor - (os_address)co;
                        type = c_getType(co);
                        if (type && (offset < c_typeSize(type))) {
                            name = c_metaName(c_metaObject(type));
                            if (name) {
                                struct lookupOffsetArg loArg;
                                loArg.object = (os_address)o;
                                loArg.offset = offset;
                                loArg.type = type;
                                loArg.container = (os_address)co;
                                loArg.printed = 0;
                                lookupOffset(c_metaObject(type), &loArg);
                                c_free(name);
                                if (!loArg.printed) {
                                    char *n = c_metaScopedName(c_metaObject(type));
                                    printf("<0x%" PA_PRIxADDR
                                           "> /* found in object of type %s at offset %" PA_PRIuADDR " */\n",
                                           (os_address)co, n, offset);
                                    os_free(n);
                                }
                            } else {
                                printf("<0x%" PA_PRIxADDR
                                       "> /* found in unknown type at offset %" PA_PRIuADDR " */\n",
                                       (os_address)co, offset);
                            }
                        }
                    }
                }
                cursor = cursor + sizeof(os_address);
            }
        } else {
            printf("Given address 0x%" PA_PRIxADDR " is invalid\n", (os_address)arg);
        }
    } else {
        printf("Given address 0x%" PA_PRIxADDR " is invalid\n", (os_address)arg);
    }
}

static void lookupObjectsOfTypeAction(v_public entity, c_voidp arg)
{
    c_object o, co, co1;
    c_base base;
    os_address baseAddr, endAddr, cursor;

    base = c_getBase(entity);
    baseAddr = (os_address)c_baseMM(base);
    endAddr = baseAddr + (os_address)c_mmSize((c_mm)baseAddr);

    o = c_baseCheckPtr(base, arg);
    if (o) {
        if (o == arg) {
            cursor = baseAddr;
            while (cursor < endAddr) {
                if (*((c_object *)cursor) == o) {
                    co1 = c_mmCheckPtr(c_baseMM(base), (c_object)cursor);
                    if (co1 == c_mmCheckPtr(c_baseMM(base), (c_object)(cursor + sizeof(os_address)))) {
                        co = c_baseCheckPtr(base, (c_object)(cursor + sizeof(os_address)));
                        if (co != NULL && c_getType(co) == o) {
                            printf("<0x%" PA_PRIxADDR ">\n", (os_address)co);
                        }
                    }
                }
                cursor = cursor + sizeof(os_address);
            }
        } else {
            printf("Given address 0x%" PA_PRIxADDR " is invalid\n", (os_address)arg);
        }
    } else {
        printf("Given address 0x%" PA_PRIxADDR " is invalid\n", (os_address)arg);
    }
}

static void print_object_history(v_public public, c_voidp arg)
{
    v_entity entity = v_entity(public);
    c_base base = c_getBase(entity);
    c_mm mm = c_baseMM(base);
    c_mmPrintObjectHistory(stdout, mm, arg);
}

static void scan_object(v_public public, c_voidp arg)
{
    v_entity entity = v_entity(public);
    c_base base = c_getBase(entity);
    c_object o = c_baseCheckPtr(base, arg);
    if (o == NULL) {
        printf("Given address 0x%" PA_PRIxADDR " is not a database object\n", (os_address)arg);
    } else {
        if ((c_voidp)o != arg) {
            os_address delta = (os_address)o - (os_address)arg;
            printf("Given address 0x%" PA_PRIxADDR " is %" PA_PRIuADDR " bytes into an object at 0x%" PA_PRIxADDR "\n",
                   (os_address)arg, delta, (os_address)o);
        }
        c__refcheckWalkRefs(1, &o);
    }
}

static void resolve_type(v_public public, c_voidp arg)
{
    v_entity entity = v_entity(public);
    c_base base = c_getBase(entity);
    c_object o = c_resolve(base, arg);
    if (o == NULL) {
        printf("%s not found\n", (char *)arg);
    } else {
        toolAction(public, o);
        c_free(o);
    }
}

static void trace_object(v_public public, c_voidp arg)
{
    v_entity entity = v_entity(public);
    c_base base = c_getBase(entity);
    c_object o = c_baseCheckPtr(base, arg);
    if (o == NULL) {
        printf("Given address 0x%" PA_PRIxADDR " is not a database object\n", (os_address)arg);
    } else {
        c_baseTraceObject(o);
    }
}

static void trace_type(v_public public, c_voidp arg)
{
    v_entity entity = v_entity(public);
    c_base base = c_getBase(entity);
    c_object o = c_baseCheckPtr(base, arg);
    if (o == NULL) {
        printf("Given address 0x%" PA_PRIxADDR " is not a database object\n", (os_address)arg);
    } else if (!c_instanceOf(o, "c_type")) {
        c_type t = c_getType(o);
        char *n = t ? c_metaScopedName(c_metaObject(t)) : "(no type?)";
        printf("Given address 0x%" PA_PRIxADDR " is not a type (it is of type %s)\n", (os_address)arg, n);
        os_free(n);
    } else {
        c_baseTraceObjectsOfType(o);
    }
}

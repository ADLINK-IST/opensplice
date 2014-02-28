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

#include "../../../common/code/dbg_common.c"
#include "v_group.h"
#include "v_groupSet.h"
#include "u_domain.h"
#include "os_report.h"

#include "c__mmbase.h"

#define _METANAME(m) c_metaObject(m)->name
#define iprintf { int i; for (i=0; i<actionData->depth; i++) printf("    "); } printf


static void
toolAction (
    v_entity entity,
    c_voidp args);

static void
initAction (
    v_entity entity,
    c_voidp args);

static void
lookupServicesAction (
    v_entity entity,
    c_voidp arg);

static void
lookupTopicAction (
    v_entity entity,
    c_voidp arg);

static void
lookupPartitionAction (
    v_entity entity,
    c_voidp arg);

static void
lookupParticipantsAction (
    v_entity entity,
    c_voidp arg);

static void
lookupGroupsAction (
    v_entity entity,
    c_voidp arg);

static void
lookupReadersWithFilterAction (
    v_entity entity,
    c_voidp arg);

static void
lookupWritersWithFilterAction (
    v_entity entity,
    c_voidp arg);

static void
lookupReadersAction (
    v_entity entity,
    c_voidp arg);

static void
lookupWritersAction (
    v_entity entity,
    c_voidp arg);

static void
lookupEntitiesAction (
    v_entity entity,
    c_voidp arg);

static void
print_object_history (
    v_entity entity,
    c_voidp arg);

static void
print_usage ()
{
    printf ("***************************************************************************************************\n");
    printf ("DataBase Dump (dbd) is an experimental tool to print shm content to stdout.                        \n");
    printf ("This tool operates read only on shm and doesn't perform any locking.                               \n");
    printf ("Subsequently it may crash when reading unstable data.                                              \n");
    printf ("---------------------------------------------------------------------------------------------------\n");
    printf ("Usage:\n");
    printf ("dbd> <addr> <enter>                                  /* show object content                      */\n");
    printf ("dbd> s <enter>                                       /* show services                            */\n");
    printf ("dbd> t [<expr>] <enter>                              /* show topics if name matches <expr>       */\n");
    printf ("dbd> d [<expr>] <enter>                              /* show partitions if name matches <expr>   */\n");
    printf ("dbd> p [<expr>] <enter>                              /* show participants if name matches <expr> */\n");
    printf ("dbd> g [<expr>][<expr>] <enter>                      /* show groups if topic and partition       */\n");
    printf ("                                                     /* names matches the expressions            */\n");
    printf ("dbd> r [<addr>] <enter>                              /* show readers associated to entity <addr> */\n");
    printf ("dbd> rf [<string>] <enter>                           /* show readers matching given readername   */\n");
    printf ("dbd> w [<addr>] <enter>                              /* show writers associated to entity <addr> */\n");
    printf ("dbd> wf [<string>] <enter>                           /* show writers matching given writername   */\n");
    printf ("dbd> e [<addr>] <enter>                              /* show contained entities                  */\n");
    printf ("dbd> l <enter>                                       /* show <address> history                   */\n");
    printf ("dbd> i [<systemId>][<localId>] [<serial>] <enter>    /* show entry belonging to that gid         */\n");
    printf ("dbd> ? <addr> <enter>                                /* show allocation context for <address>    */\n");
    printf ("dbd> q <enter>                                       /* quit                                     */\n");
    printf ("Example:\n");
    printf ("dbd> 0x200a2df8 <enter>                              /* Print content of shm address             */\n");
    printf ("***************************************************************************************************\n");
}


C_CLASS(lookupGroupsArg);
C_STRUCT(lookupGroupsArg) {
    char *partitions;
    char *topics;
};

/* The Main function
 * Performes initialization and enters the main loop until the a
 * stop condition becomes true.
 * A mail loop iteration reads a command from stdin and calls
 * the operation that implements the command. The result is
 * written to a stream.
 */
#ifdef INTEGRITY
int dbd_main (int argc,
              char * argv[])
#else
OPENSPLICE_MAIN (ospl_dbd)
#endif
{
    char *uri = "";

    u_result         ur;
    u_participant    participant;
    v_participantQos pqos;
    c_iter history;
    c_long cursor;

    if (argc > 1) {
       uri = argv[1];
    }

    if((!uri) || (strlen(uri) == 0))
    {
        uri = os_getenv("OSPL_URI");

        if(!uri)
        {
            uri = (c_char*)os_malloc(19);
            sprintf(uri, "%s", "The default Domain");
        }
    }

    ur = u_userInitialise();

    if (ur == U_RESULT_OK)
    {
        pqos = u_participantQosNew(NULL);
        participant = u_participantNew(uri, 30, "dbd", (v_qos)pqos, TRUE);
        u_participantQosFree(pqos);

        if(participant)
        {
            u_cfElement element;
            u_domain domain;
            char buf1[100];
            char buf2[100];
            char buf3[100];
            char buf4[100];
            int stop = 0;
            int len;
            void *addr;
            int i, c;
            v_gid g;

            element = u_participantGetConfiguration(participant);
            if (element) {
                printf("Node name: %s\n", u_cfNodeName((u_cfNode)element));
                printf("Node kind: %d\n", u_cfNodeKind((u_cfNode)element));
            } else {
                printf("No configuration for: %s\n", uri);
            }

            domain = u_participantDomain(participant);
            if (domain) {
                printf("Domain memory address: 0x"PA_ADDRFMT"\n",
                       (os_address) u_domainMemoryAddress(domain));
                printf("Domain memory size: 0x"PA_ADDRFMT"\n",
                       (os_address) u_domainMemorySize(domain));
            } else {
                printf("No domain for Participant\n");
            }

            print_usage();

            ur = u_entityAction(u_entity(participant),
                                initAction,
                                &addr);
            printf("Kernel Address <0x"PA_ADDRFMT">\n",(os_address)addr);
            g = u_entityGid(u_entity(participant));
            printf("Node GID: %u %u %u \n",g.systemId,g.localId,g.serial);
            cursor = 0;
            history = NULL;
            while (!stop) {
                printf("dbd> ");
                i = 0;
                c = getchar();
                while ((c!=0) && (c!=' ') && (c!='\t') && (c!='\n')) {
                   buf1[i++] = (c_char)c;
                   c = getchar();
                }
                buf1[i] = 0;
                while ((c!=0) && ((c==' ') || (c=='\t')) && (c!='\n')) {
                    c = getchar();
                }
                i=0;

                while ((c!=0) && (c!=' ') && (c!='\t') && (c!='\n')) {
                   buf2[i++] = (c_char)c;
                   c = getchar();
                }
                buf2[i] = 0;
                while ((c!=0) && ((c==' ') || (c=='\t')) && (c!='\n')) {
                    c = getchar();
                }
                i=0;

                while ((c!=0) && (c!=' ') && (c!='\t') && (c!='\n')) {
                   buf3[i++] = (c_char)c;
                   c = getchar();
                }
                buf3[i] = 0;
                i=0;

                while ((c!=0) && ((c==' ') || (c=='\t')) && (c!='\n')) {
                    c = getchar();
                }
                while ((c!=0) && (c!=' ') && (c!='\t') && (c!='\n')) {
                   buf4[i++] = (c_char)c;
                   c = getchar();
                }
                buf4[i] = 0;
                while ((c!=0) && (c!='\n')) {
                    c = getchar();
                }
                if (strlen(buf1) > 0) {
                    if (strncmp(buf1, "q", 1) == 0) {
                        stop = 1;
                    } else if (strcmp(buf1, "s") == 0) {
                        ur = u_entityAction(u_entity(participant),
                                            lookupServicesAction,
                                            NULL);
                    } else if (strcmp(buf1, "t") == 0) {
                        if (strlen(buf2) == 0) {
                            strcpy(buf2,"*");
                        }
                        ur = u_entityAction(u_entity(participant),
                                            lookupTopicAction,
                                            buf2);
                    } else if (strcmp(buf1, "d") == 0) {
                        if (strlen(buf2) == 0) {
                            strcpy(buf2,"*");
                        }
                        ur = u_entityAction(u_entity(participant),
                                            lookupPartitionAction,
                                            buf2);
                    } else if (strcmp(buf1, "p") == 0) {
                        if (strlen(buf2) == 0) {
                            strcpy(buf2,"*");
                        }
                        ur = u_entityAction(u_entity(participant),
                                            lookupParticipantsAction,
                                            buf2);
                    } else if (strcmp(buf1, "g") == 0) {
                        C_STRUCT(lookupGroupsArg) arg;

                        if (strlen(buf2) == 0) {
                            strcpy(buf2,"*");
                        }
                        if (strlen(buf3) == 0) {
                            strcpy(buf3,"*");
                        }
                        arg.partitions = buf2;
                        arg.topics = buf3;
                        ur = u_entityAction(u_entity(participant),
                                            lookupGroupsAction,
                                            &arg);
                    } else if (strcmp(buf1, "r") == 0) {
                        if (strlen(buf2) == 0) {
                            ur = u_entityAction(u_entity(participant),
                                                lookupReadersAction,
                                                NULL);
                        } else {
                            len = sscanf(buf2,"0x"PA_ADDRFMT"",(os_address *)&addr);
                            if (len == 1) {
                                ur = u_entityAction(u_entity(participant),
                                                    lookupReadersAction,
                                                    addr);
                            } else {
                                printf("usage: r <address>\n");
                            }
                        }
                    } else if (strcmp(buf1, "rf") == 0) {
                        if (strlen(buf2) == 0) {
                            printf("usage: rf <string>\n");
                        } else {
                            ur = u_entityAction(u_entity(participant),
                                                lookupReadersWithFilterAction,
                                                (c_voidp)buf2);
                        }
                    } else if (strcmp(buf1, "w") == 0) {
                        if (strlen(buf2) == 0) {
                            ur = u_entityAction(u_entity(participant),
                                                lookupWritersAction,
                                                NULL);
                        } else {
                            len = sscanf(buf2,"0x"PA_ADDRFMT"",(os_address *)&addr);
                            if (len == 1) {
                                ur = u_entityAction(u_entity(participant),
                                                    lookupWritersAction,
                                                    addr);
                            } else {
                                printf("usage: w <address>\n");
                            }
                        }
                    } else if (strcmp(buf1, "wf") == 0) {
                        if (strlen(buf2) == 0) {
                            printf("usage: wf <string>\n");
                        } else {
                            ur = u_entityAction(u_entity(participant),
                                                lookupWritersWithFilterAction,
                                                (c_voidp)buf2);
                        }
                    } else if (strcmp(buf1, "e") == 0) {
                        len = sscanf(buf2,"0x"PA_ADDRFMT"",(os_address *)&addr);
                        if (len == 1) {
                            ur = u_entityAction(u_entity(participant),
                                                lookupEntitiesAction,
                                                addr);
                        } else {
                            printf("usage: e <address>\n");
                        }
                    } else if (strcmp(buf1, "l") == 0) {
                        c_iterWalk(history,printWalkHistory,NULL);

                    } else if (strcmp(buf1, "i") == 0) {
                        c_ulong a1,a2,a3;
                        int len1,len2,len3;
                        len1  = sscanf(buf2,"%u",&a1);
                        len2  = sscanf(buf3,"%u",&a2);
                        len3  = sscanf(buf4,"%u",&a3);
                        if (len1 == 1 && len2 == 1 && len3 == 1) {
                            v_public vp = NULL;
                            v_gid gid ;
                            gid.systemId = a1;
                            gid.localId = a2;
                            gid.serial =a3;
                            vp = v_gidClaim(gid,(v_kernel)addr);
                            if (vp != NULL) {
                                ur = u_entityAction(u_entity(participant),
                                                    toolAction,
                                                    vp);
                                c_iterTake(history,vp); /* remove if already exist */
                                history = c_iterInsert(history,vp);
                                v_gidRelease(gid,(v_kernel)addr);
                            } else {
                                if (g.systemId != gid.systemId) {
                                    printf("Requested entity with systemId: %u is not present on local machine with systemId %u\n",gid.systemId,g.systemId);
                                } else {
                                    printf("Could not find an entry for gid: %u %u %u\n",gid.systemId,gid.localId,gid.serial);
                                }
                            }
                        } else {
                            printf("usage: i <systemId> <localId> <serial>\n");
                        }
                    } else if (strcmp(buf1, "?") == 0) {
                        len = sscanf(buf2,"0x"PA_ADDRFMT"",(os_address *)&addr);
                        if (len == 1) {
                            ur = u_entityAction (u_entity (participant), print_object_history, addr);
                        } else {
                            printf("usage: e <address>\n");
                        }
                    } else {
                        len = sscanf(buf1,"0x"PA_ADDRFMT"",(os_address *)&addr);
                        if (len > 0) {
                            ur = u_entityAction(u_entity(participant),
                                                toolAction,
                                                addr);
                            c_iterTake(history,addr); /* remove if already exist */
                            history = c_iterInsert(history,addr);
                        } else {
                            print_usage();
                        }
                    }
                }
            }
            u_participantFree(participant);
        }
        else
        {
            printf("Creation of participant failed.\n");
            printf("Is the OpenSplice system running?\n");
            OS_REPORT(OS_ERROR,"tool", 0, "Creation of participant failed.");
        }

        u_userDetach();

    }
    else
    {
        printf("Failed to initialise process.\n");
        OS_REPORT(OS_ERROR,"tool", 0, "Failed to initialise process.");
    }

    printf("\nExiting now...\n");

    return 0;
}

static void
printHistory (
    c_iter history,
    c_long cursor)
{
    c_type type;
    c_object o;
    c_char *name, *ename;

    o = c_iterObject(history,cursor-1);
    type = c_getType(o);
    name = c_metaScopedName(c_metaObject(type));
    printf("<0x"PA_ADDRFMT"> %s",(os_address)o,name);
    if (c_checkType(o, "v_entity") == o) {
        ename = v_entityName(o);
        if (ename != NULL) {
            printf(" /* %s */", ename);
        }
    }
    printf("\n");
}

static void
toolAction (
    v_entity entity,
    c_voidp args)
{
    c_base base;
    c_type type;
    c_char *name;
    c_object o;
    c_address offset;
    c_long size;
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
                printf("Warning: address is %lu bytes in %s "
                       "object starting at 0x"PA_ADDRFMT"\n",
                       offset, _METANAME(type), (os_address)o);
                OBJECT_PUSH(&actionData, o);
                tryPrintOffset(o,&actionData,offset);
                OBJECT_POP(&actionData);
            } else {
                printf("Warning: address is %lu bytes in "
                       "memory starting at 0x"PA_ADDRFMT"\n",
                   offset, (os_address)o);
            }
        } else {
            name = c_metaScopedName(c_metaObject(type));
            printf("Object <0x"PA_ADDRFMT"> refCount=%d size=%d type is: <0x"PA_ADDRFMT"> %s\n",
                   (os_address)o, c_refCount(o), size, (os_address)type, name);
            os_free(name);
            OBJECT_PUSH(&actionData, o);
            printType(type, &actionData);
            printf("\n");
            OBJECT_POP(&actionData);
        }
    } else {
        printf("Address <0x"PA_ADDRFMT"> is not a Database Object\n",
               (os_address)args);
    }
}

static void
initAction (
    v_entity entity,
    c_voidp args)
{
    c_base base;
    c_object *kernel = (c_object *)args;

    *kernel = v_objectKernel(entity);
    base = c_getBase(*kernel);

    /* caching of type references for fast comparisson. */
    v_handle_t = c_resolve(base,"kernelModule::v_handle_s");
    v_topic_t = v_kernelType(*kernel,K_TOPIC);
    v_partition_t = v_kernelType(*kernel,K_DOMAIN);
}

static c_bool
serviceAction(
    c_object _this,
    c_voidp arg)
{
    v_participant p = (v_participant)_this;
    if (_this) {
        if (v_objectKind(p) != K_PARTICIPANT) {
            printf("<0x"PA_ADDRFMT"> /* %s */\n",(os_address)_this, v_entityName2(p));
        }
    }
    return TRUE;
}

static void
lookupServicesAction (
    v_entity entity,
    c_voidp arg)
{
    v_kernel kernel;

    kernel = v_objectKernel(entity);
    c_walk(kernel->participants,serviceAction,NULL);
}

static void
lookupTopicAction (
    v_entity entity,
    c_voidp arg)
{
    v_kernel kernel;
    c_iter list;
    c_voidp addr;

    kernel = v_objectKernel(entity);
    list = v_resolveTopics(kernel,arg);
    addr = c_iterTakeFirst(list);
    while (addr != NULL) {
        printf("<0x"PA_ADDRFMT"> /* %s */\n",(os_address)addr, v_entityName2(addr));
        c_free(addr);
        addr = c_iterTakeFirst(list);
    }
    c_iterFree(list);
}

static void
lookupPartitionAction (
    v_entity entity,
    c_voidp arg)
{
    v_kernel kernel;
    c_iter list;
    c_voidp addr;

    kernel = v_objectKernel(entity);
    list = v_resolvePartitions(kernel,arg);
    addr = c_iterTakeFirst(list);
    while (addr != NULL) {
        printf("<0x"PA_ADDRFMT"> /* %s */\n",(os_address)addr, v_entityName2(addr));
        c_free(addr);
        addr = c_iterTakeFirst(list);
    }
    c_iterFree(list);
}

static void
lookupParticipantsAction (
    v_entity entity,
    c_voidp arg)
{
    v_kernel kernel;
    c_iter list;
    c_voidp addr;

    kernel = v_objectKernel(entity);
    list = v_resolveParticipants(kernel,arg);
    addr = c_iterTakeFirst(list);
    while (addr != NULL) {
        printf("<0x"PA_ADDRFMT"> /* %s */\n",(os_address)addr, v_entityName2(addr));
        c_free(addr);
        addr = c_iterTakeFirst(list);
    }
    c_iterFree(list);
}

static void
lookupGroupsAction (
    v_entity entity,
    c_voidp arg)
{
    v_kernel kernel;
    c_iter list;
    c_voidp addr;
    lookupGroupsArg a = (lookupGroupsArg)arg;

    kernel = v_objectKernel(entity);
    list = v_groupSetLookup(kernel->groupSet, a->partitions, a->topics);
    addr = c_iterTakeFirst(list);
    while (addr != NULL) {
        printf("<0x"PA_ADDRFMT"> /* %s */\n",(os_address)addr, v_entityName2(addr));
        c_free(addr);
        addr = c_iterTakeFirst(list);
    }
    c_iterFree(list);
}

static c_bool
lookupReader(
    v_entry _this,
    c_voidp arg)
{
    c_iter *list = (c_iter *)arg;

    if ((_this) && (list != NULL)) {
        *list = c_iterInsert(*list,_this->reader);
    }
    return TRUE;
}

static c_bool
lookupReaderAction (
    v_entity e,
    c_voidp arg)
{
    c_iter *list = (c_iter *)arg;

    if (e) {
        if (c_checkType(e,"v_reader") == e) {
            *list = c_iterInsert(*list,e);
        }
    }
    return TRUE;
}

static c_bool
lookupSubscriberAction (
    v_entity e,
    c_voidp arg)
{
    c_iter *list = (c_iter *)arg;

    if (e) {
        if (v_objectKind(e) == K_SUBSCRIBER) {
            *list = c_iterInsert(*list,e);
        }
    }
    return TRUE;
}

static void
lookupReadersWithFilterAction (
    v_entity entity,
    c_voidp arg)
{
    v_kernel kernel;
    c_iter list, alist, slist;
    c_object o;
    c_base base;
    c_voidp addr;
    v_subscriber s;
    int found =0;

    kernel = v_objectKernel(entity);
    base = c_getBase(entity);

    list = NULL;
    alist = v_resolveParticipants(kernel,"*");
    o = c_iterTakeFirst(alist);
    while (o != NULL) {
        slist = NULL;
        v_entityWalkEntities(o,lookupSubscriberAction,&slist);
        s = c_iterTakeFirst(slist);
        while (s != NULL) {
            v_entityWalkEntities(v_entity(s),lookupReaderAction,&list);
            s = c_iterTakeFirst(slist);
        }
        c_iterFree(slist);
        c_free(o);
        o = c_iterTakeFirst(alist);
    }
    c_iterFree(alist);
    addr = c_iterTakeFirst(list);

    while (addr != NULL) {
        if (strstr(v_entityName2(addr),(char *)arg)  != NULL) {
            printf("<0x"PA_ADDRFMT"> /* %s */\n",(os_address)addr, v_entityName2(addr));
            found =1;
        }
        addr = c_iterTakeFirst(list);
    }
    if (found ==0) {
        printf("No readers found matching: %s\n",(char *)arg);
    }
    c_iterFree(list);
    return;

}

static void
lookupReadersAction (
    v_entity entity,
    c_voidp arg)
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
        alist = v_resolveParticipants(kernel,"*");
        o = c_iterTakeFirst(alist);
        while (o != NULL) {
            slist = NULL;
            v_entityWalkEntities(o,lookupSubscriberAction,&slist);
            s = c_iterTakeFirst(slist);
            while (s != NULL) {
                v_entityWalkEntities(v_entity(s),lookupReaderAction,&list);
                s = c_iterTakeFirst(slist);
            }
            c_iterFree(slist);
            c_free(o);
            o = c_iterTakeFirst(alist);
        }
        c_iterFree(alist);
        addr = c_iterTakeFirst(list);
        while (addr != NULL) {
            printf("<0x"PA_ADDRFMT"> /* %s */\n",(os_address)addr, v_entityName2(addr));
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
                v_entityWalkEntities(o,lookupReaderAction,&list);
            break;
            case K_PARTICIPANT:
            case K_SERVICE:
            case K_SPLICED:
                slist = NULL;
                v_entityWalkEntities(o,lookupSubscriberAction,&slist);
                s = c_iterTakeFirst(slist);
                while (s != NULL) {
                    v_entityWalkEntities(v_entity(s),lookupReaderAction,&list);
                    s = c_iterTakeFirst(slist);
                }
                c_iterFree(slist);
            break;
            default :
                printf("Lookup Reader for provided address 0x"PA_ADDRFMT" is not supported\n",
                       (os_address)arg);
            break;
            }
            addr = c_iterTakeFirst(list);
            while (addr != NULL) {
                printf("<0x"PA_ADDRFMT"> /* %s */\n",(os_address)addr, v_entityName2(addr));
                addr = c_iterTakeFirst(list);
            }
            c_iterFree(list);
        } else {
            printf("Given address 0x"PA_ADDRFMT" is invalid\n", (os_address)arg);
        }
    } else {
        printf("Given address 0x"PA_ADDRFMT" is invalid\n", (os_address)arg);
    }
}

static c_bool
lookupWriterAction (
    v_entity e,
    c_voidp arg)
{
    c_iter *list = (c_iter *)arg;

    if (e) {
        if (c_checkType(e,"v_writer") == e) {
            *list = c_iterInsert(*list,e);
        }
    }
    return TRUE;
}

static c_bool
lookupPublisherAction (
    v_entity e,
    c_voidp arg)
{
    c_iter *list = (c_iter *)arg;

    if (e) {
        if (v_objectKind(e) == K_PUBLISHER) {
            *list = c_iterInsert(*list,e);
        }
    }
    return TRUE;
}

static void
lookupWritersWithFilterAction (
    v_entity entity,
    c_voidp arg)
{
    v_kernel kernel;
    c_iter list, plist, alist;
    c_object o;
    c_base base;
    c_voidp addr;
    v_publisher p;
    int found =0;

    kernel = v_objectKernel(entity);
    base = c_getBase(entity);
    list = NULL;
    alist = v_resolveParticipants(kernel,"*");
    o = c_iterTakeFirst(alist);
    while (o != NULL) {
        plist = NULL;
        v_entityWalkEntities(o,lookupPublisherAction,&plist);
        p = c_iterTakeFirst(plist);
        while (p != NULL) {
            v_entityWalkEntities(v_entity(p),lookupWriterAction,&list);
            p = c_iterTakeFirst(plist);
        }
        c_iterFree(plist);
        c_free(o);
        o = c_iterTakeFirst(alist);
    }
    c_iterFree(alist);
    addr = c_iterTakeFirst(list);
    while (addr != NULL) {
        if (strstr(v_entityName2(addr),(char *)arg)  != NULL) {
            printf("<0x"PA_ADDRFMT"> /* %s */\n",(os_address)addr, v_entityName2(addr));
            found =1;
        }
        addr = c_iterTakeFirst(list);
    }
    if (found == 0) {
        printf("No writers found matching: %s\n",(char *)arg);
    }
    c_iterFree(list);
    return;
}

static void
lookupWritersAction (
    v_entity entity,
    c_voidp arg)
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
        alist = v_resolveParticipants(kernel,"*");
        o = c_iterTakeFirst(alist);
        while (o != NULL) {
            plist = NULL;
            v_entityWalkEntities(o,lookupPublisherAction,&plist);
            p = c_iterTakeFirst(plist);
            while (p != NULL) {
                v_entityWalkEntities(v_entity(p),lookupWriterAction,&list);
                p = c_iterTakeFirst(plist);
            }
            c_iterFree(plist);
            c_free(o);
            o = c_iterTakeFirst(alist);
        }
        c_iterFree(alist);
        addr = c_iterTakeFirst(list);
        while (addr != NULL) {
            printf("<0x"PA_ADDRFMT"> /* %s */\n",(os_address)addr, v_entityName2(addr));
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
/*                v_groupWalkWriters(o, lookupWriter, &list); */
printf("Lookup writer for Group not yet implemented\n");
            break;
            case K_DOMAIN:
                glist = v_groupSetLookup(kernel->groupSet, v_partitionName(o), "*");
                g = c_iterTakeFirst(glist);
                while (g != NULL) {
/*                    v_groupWalkWriters(g, lookupWriter, &list); */
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
/*                    v_groupWalkWriters(g, lookupWriter, &list); */
printf("Lookup writer for Topic not yet implemented\n");
                    c_free(g);
                    g = c_iterTakeFirst(glist);
                }
                c_iterFree(glist);
            break;
            case K_PUBLISHER:
                v_entityWalkEntities(o,lookupWriterAction,&list);
            break;
            case K_PARTICIPANT:
            case K_SERVICE:
            case K_SPLICED:
                plist = NULL;
                v_entityWalkEntities(o,lookupPublisherAction,&plist);
                p = c_iterTakeFirst(plist);
                while (p != NULL) {
                    v_entityWalkEntities(v_entity(p),lookupWriterAction,&list);
                    p = c_iterTakeFirst(plist);
                }
                c_iterFree(plist);
            break;
            default :
                printf("Lookup Writer for provided address 0x"PA_ADDRFMT" is not supported\n",
                       (os_address)arg);
            break;
            }
            addr = c_iterTakeFirst(list);
            while (addr != NULL) {
                printf("<0x"PA_ADDRFMT"> /* %s */\n",(os_address)addr, v_entityName2(addr));
                addr = c_iterTakeFirst(list);
            }
            c_iterFree(list);
        } else {
            printf("Given address 0x"PA_ADDRFMT" is invalid\n", (os_address)arg);
        }
    } else {
        printf("Given address 0x"PA_ADDRFMT" is invalid\n", (os_address)arg);
    }
}

static c_bool
lookupEntityAction (
    v_entity e,
    c_voidp arg)
{
    c_iter *list = (c_iter *)arg;

    if (e) {
        if (c_checkType(e, "v_entity") == e) {
            *list = c_iterInsert(*list,e);
        }
    }
    return TRUE;
}

static void
lookupEntitiesAction (
    v_entity entity,
    c_voidp arg)
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
            v_entityWalkEntities(o,lookupEntityAction,&list);
            addr = c_iterTakeFirst(list);
            while (addr != NULL) {
                printf("<0x"PA_ADDRFMT"> /* %s */\n",(os_address)addr, v_entityName2(addr));
                addr = c_iterTakeFirst(list);
            }
            c_iterFree(list);
        } else {
            printf("Given address 0x"PA_ADDRFMT" is invalid\n", (os_address)arg);
        }
    } else {
        printf("Given address 0x"PA_ADDRFMT" is invalid\n", (os_address)arg);
    }
}

static void
print_object_history (
    v_entity entity,
    c_voidp arg)
{
    c_base base = c_getBase (entity);
    c_mm mm = c_baseMM (base);
    c_mmPrintObjectHistory (stdout, mm, arg);
}

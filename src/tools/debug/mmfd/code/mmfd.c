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
#ifndef _WIN32
#include <unistd.h>
#endif
#include "os_memMapFile.h"
#include "d_storeMMFKernel.h"
#include "d_topicInfo.h"
#include "d_groupInfo.h"
#include "os_report.h"

#define MMF_STORE_DATABASE_NAME "OsplDurabilityDatabase"
#define MMF_STORE_KERNEL_NAME   "OsplDurabilityKernel"


#define d_nameSpaceInfo(o) (C_CAST(o,d_nameSpaceInfo))

const char* MMFD_CMD_OPTIONS = "hf:a:";

static d_storeMMFKernel
attachToMMFKernel(
    c_base base,
    const c_char *name);

static void 
printMemoryUsage(
    c_base base);

static void
toolAction (
    d_storeMMFKernel kernel,
    c_voidp addr);


static void
lookupTopicAction (
   d_storeMMFKernel kernel,
   const c_char *filter);

static void
lookupNameSpacesAction (
    d_storeMMFKernel kernel,
    const c_char *filter);

static void
lookupGroupsAction (
    c_set groups,
    const c_char *partitions,
    const c_char *topics);


static void
print_help ()
{
    printf ("Usage: mmfd [-h] <-f mmfFile> <-a mappingAddress>\n\n");
    printf ("Where:\t[-h]\t\tPrints this help message.\n");
    printf ("\t<-f mmfFile>\tIdentifies the mmf file for which to dump the contents.\n");
    printf ("\t<-a mmfAddress>\tIdentifies the mapping address that is needed to map this file in memory.\n");
    printf ("\t\t\tNote: this must be the same mapping address that was used by the \n");
    printf ("\t\t\tdurability service that generated this persistence file.\n\n");

}

static void
print_usage ()
{
    printf ("********************************************************************************\n");
    printf ("MMF Dump (mmfd) is an experimental tool to print mmf content to stdout.         \n");
    printf ("This tool operates read only on mmf and doesn't perform any locking.            \n");
    printf ("Subsequently it may crash when reading unstable data.                           \n");
    printf ("--------------------------------------------------------------------------------\n");
    printf ("Usage:\n");
    printf ("mmfd> <addr> <enter>               /* show object content                      */\n");
    printf ("mmfd> t [<expr>] <enter>           /* show topics if name matches <expr>       */\n");
    printf ("mmfd> n [<expr>] <enter>           /* show namespaces if name matches <expr>   */\n");
    printf ("mmfd> g [<expr>][<expr>] <enter>   /* show groups if topic and partition       */\n");
    printf ("                                   /* names matches the expressions            */\n");
    printf ("mmfd> b [<expr>][<expr>] <enter>   /* show backup if topic and partition       */\n");
    printf ("                                   /* names matches the expressions            */\n");
    printf ("mmfd> l <enter>                    /* show <address> history                   */\n");
    printf ("mmfd> m <enter>                    /* show memory usage                        */\n");
    printf ("mmfd> h <enter>                    /* print this help                          */\n");
    printf ("mmfd> q <enter>                    /* quit                                     */\n");
    printf ("Example:\n");
    printf ("mmfd> 0x200a2df8 <enter>           /* Print content of shm address             */\n");
    printf ("********************************************************************************\n");
}

/* The Main function
 * Performes initialization and enters the main loop until the a
 * stop condition becomes true.
 * A mail loop iteration reads a command from stdin and calls
 * the operation that implements the command. The result is
 * written to a stream.
 */
#ifdef INTEGRITY
int mmfd_main (int argc,
              char * argv[])
#else
OPENSPLICE_MAIN(ospl_mmfd)
#endif
{
    char *mmfFile = NULL;
    c_address mmfAddr = 0x0;
    os_mmfAttr mmfAttr;
    os_mmfHandle mmfHandle;
    os_result mmfOpenResult, mmfAttachResult;
    c_base base;
    d_storeMMFKernel kernel;

    c_iter history;
    c_long cursor;
    char buf1[100];
    char buf2[100];
    char buf3[100];
    int stop = 0;
    int len;
    void *addr;
    int i, c, scanCount;
    u_result ur;

    int opt;

    while ((opt = getopt(argc, argv, MMFD_CMD_OPTIONS)) != -1) {
        switch (opt) {
        case 'h':
            printf ("mmfd is a tool that can dump the contents of an OpenSplice MemoryMapped persistence file (mmf) to <stdout>.\n\n");
            print_help();
            exit(0);
            break;
        case 'f':
            mmfFile = optarg;
            break;
        case 'a':
            scanCount = sscanf(optarg, PA_ADDRFMT, &mmfAddr);
            if (scanCount != 1)
            {
                printf("%s is not a valid address.\n", optarg);
                print_help();
                exit(1);
            }
            break;
        case '?':
            print_help();
            exit(1);
            break;
        }
    }

    if (!mmfFile)
    {
        printf ("Missing the mandatory mmfFile argument.\n\n");
        print_help();
        exit(1);
    }

    if (!mmfAddr)
    {
        printf ("Missing the mandatory mmfAddress argument.\n\n");
        print_help();
        exit(1);
    }

    ur = u_userInitialise();

    os_mmfAttrInit(&mmfAttr);
    mmfAttr.map_address = (void*) mmfAddr;
    mmfHandle = os_mmfCreateHandle(mmfFile, &mmfAttr);
    if (!os_mmfFileExist(mmfHandle))
    {
        printf("mmfFile '%s' does not exist.\n\n", mmfFile);
        print_help();
        exit(1);
    }

    mmfOpenResult = os_mmfOpen(mmfHandle);
    if (mmfOpenResult != os_resultSuccess)
    {
        printf("Failed to open mmfFile '%s'.\n\n", mmfFile);
        exit(1);
    }

    mmfAttachResult = os_mmfAttach(mmfHandle);
    if (mmfAttachResult != os_resultSuccess)
    {
        printf("Failed to attach to mmfFile '%s'.\n\n", mmfFile);
        exit(1);
    }

    base = c_open(MMF_STORE_DATABASE_NAME, os_mmfAddress(mmfHandle));
    if (base == NULL)
    {
        printf("Failed to open database in mmfFile '%s'.\n\n", mmfFile);
        exit(1);
    }

    kernel = attachToMMFKernel(base, MMF_STORE_KERNEL_NAME);
    if (kernel == NULL)
    {
        printf("Kernel object not found in mmfFile '%s'.\n\n", mmfFile);
        exit(1);
    }

    c_mmResume(c_baseMM(base));

    print_usage();

    printf("Kernel Address <0x"PA_ADDRFMT">\n",(os_address)kernel);
    printMemoryUsage(base);

    cursor = 0;
    history = NULL;
    while (!stop) {
        printf("mmfd> ");
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
        while ((c!=0) && (c!='\n')) {
            c = getchar();
        }
        if (strlen(buf1) > 0) {
            if (strncmp(buf1, "q", 1) == 0) {
                stop = 1;
            } else if (strcmp(buf1, "t") == 0) {
                if (strlen(buf2) == 0) {
                    strcpy(buf2,"*");
                }
                lookupTopicAction(kernel, buf2);
            } else if (strcmp(buf1, "b") == 0) {
                if (strlen(buf2) == 0) {
                    strcpy(buf2,"*");
                }
                if (strlen(buf3) == 0) {
                    strcpy(buf3,"*");
                }
                lookupGroupsAction(kernel->backup, buf2, buf3);
            } else if (strcmp(buf1, "n") == 0) {
                if (strlen(buf2) == 0) {
                    strcpy(buf2,"*");
                }
                lookupNameSpacesAction(kernel, buf2);
            } else if (strcmp(buf1, "g") == 0) {
                if (strlen(buf2) == 0) {
                    strcpy(buf2,"*");
                }
                if (strlen(buf3) == 0) {
                    strcpy(buf3,"*");
                }
                lookupGroupsAction(kernel->groups, buf2, buf3);
            } else if (strcmp(buf1, "l") == 0) {
                c_iterWalk(history,printWalkHistory,NULL);
            } else if (strcmp(buf1, "m") == 0) {
                printMemoryUsage(base);
            } else {
                len = sscanf(buf1,"0x"PA_ADDRFMT"",(os_address *)&addr);
                if (len > 0) {
                    toolAction(kernel, addr);
                    c_iterTake(history,addr); /* remove if already exist */
                    history = c_iterInsert(history,addr);
                } else {
                    print_usage();
                }
            }
        }
    }

    c_mmSuspend(c_baseMM(base));
    u_userDetach();

    printf("\nExiting now...\n");

    return 0;
}

static d_storeMMFKernel
attachToMMFKernel(
    c_base base,
    const c_char *name)
{
    d_storeMMFKernel kernel = NULL;

    if (name == NULL) {
        OS_REPORT(OS_ERROR,
                  "attachToMMFKernel",0,
                  "Failed to lookup kernel, specified kernel name = <NULL>");
    } else {
        kernel = c_lookup(base,name);
        if (kernel == NULL) {
            OS_REPORT_1(OS_ERROR,
                        "attachToMMFKernel",0,
                        "Failed to lookup kernel '%s' in Database",
                        name);
        } else if (c_checkType(kernel,"d_storeMMFKernel") != kernel) {
            c_free(kernel);
            kernel = NULL;
            OS_REPORT_1(OS_ERROR,
                        "attachToMMFKernel",0,
                        "Object '%s' is apparently not of type 'd_storeMMFKernel'",
                        name);
        }
    }
    return kernel;
}

static void
printMemoryUsage(
    c_base base)
{
    c_mm mm;
    c_mmStatus s;

    mm = c_baseMM(base);
    s = c_mmState(mm, FALSE);

    printf("Total size: %lld, Available: %lld, Reusable: %lld.\n", s.used + s.size + s.garbage, s.size + s.garbage, s.garbage);
}

static void
toolAction (
    d_storeMMFKernel kernel,
    c_voidp addr)
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

    base = c_getBase(kernel);
    o = c_baseCheckPtr(base, addr);
    if (o) {
        type = c_getType(o);
        size = c_typeSize(type);
        if (o != addr) {
            offset = C_ADDRESS(addr) - C_ADDRESS(o);
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
               (os_address)addr);
    }
}


static c_iter
v_resolveMMFTopics(
    d_storeMMFKernel kernel,
    const c_char *name)
{
    c_iter list;
    c_collection q;
    q_expr expr;
    c_value params[1];

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));

    expr = (q_expr)q_parse("name like %0");
    params[0] = c_stringValue((char *)name);
    q = c_queryNew(kernel->topics,expr,params);
    q_dispose(expr);
    list = ospl_c_select(q,0);
    c_free(q);
    return list;
}

static void
lookupTopicAction (
    d_storeMMFKernel kernel,
    const c_char *filter)
{
    c_iter list;
    c_voidp addr;

    list = v_resolveMMFTopics(kernel, filter);
    addr = c_iterTakeFirst(list);
    while (addr != NULL) {
        printf("<0x"PA_ADDRFMT"> /* %s */\n",(os_address)addr, d_topicInfo(addr)->name);
        c_free(addr);
        addr = c_iterTakeFirst(list);
    }
    c_iterFree(list);
}

static c_iter
v_resolveMMFNameSpaces(
    d_storeMMFKernel kernel,
    const c_char *name)
{
    c_iter list;
    c_collection q;
    q_expr expr;
    c_value params[1];

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));

    expr = (q_expr)q_parse("name like %0");
    params[0] = c_stringValue((char *)name);
    q = c_queryNew(kernel->nameSpaces,expr,params);
    q_dispose(expr);
    list = ospl_c_select(q,0);
    c_free(q);
    return list;
}

static void
lookupNameSpacesAction (
    d_storeMMFKernel kernel,
    const c_char *filter)
{
    c_iter list;
    c_voidp addr;

    list = v_resolveMMFNameSpaces(kernel,filter);
    addr = c_iterTakeFirst(list);
    while (addr != NULL) {
        printf("<0x"PA_ADDRFMT"> /* %s */\n",(os_address)addr, d_nameSpaceInfo(addr)->name);
        c_free(addr);
        addr = c_iterTakeFirst(list);
    }
    c_iterFree(list);
}

c_iter
v_resolveMMFGroups(
    c_set groups,
    const c_char *partitionExpr,
    const c_char *topicExpr)
{
    c_collection q;
    q_expr expr;
    c_iter list;
    c_value params[2];

    assert(groups != NULL);

    expr = (q_expr)q_parse("partition.name like %0 and topic.name like %1");
    if (expr == NULL) {
        assert(expr != NULL);
        return NULL;
    }
    params[0] = c_stringValue((c_string)partitionExpr);
    params[1] = c_stringValue((c_string)topicExpr);
    q = c_queryNew(groups, expr, params);
    if (q == NULL) {
        list = NULL;
    } else {
        list = ospl_c_select(q,0);
    }
    assert(q != NULL);
    c_free(q);
    q_dispose(expr);

    return list;
}

static void
lookupGroupsAction (
    c_set groups,
    const c_char *partitions,
    const c_char *topics)
{
    c_iter list;
    c_voidp addr;

    list = v_resolveMMFGroups(groups, partitions, topics);
    addr = c_iterTakeFirst(list);
    while (addr != NULL) {
        printf("<0x"PA_ADDRFMT"> /* topic,partition = %s,%s */\n",(os_address)addr, d_groupInfo(addr)->topic->name, d_groupInfo(addr)->partition);
        c_free(addr);
        addr = c_iterTakeFirst(list);
    }
    c_iterFree(list);
}


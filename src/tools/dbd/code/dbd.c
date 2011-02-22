/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

#include <assert.h>
#include <stdio.h>
#include <time.h>

#include "os.h"
#include "c_misc.h"
#include "v_kernel.h"
#include "v_entity.h"
#include "v_group.h"
#include "v_groupSet.h"
#include "v_topic.h"
#include "v_partition.h"
#include "u_user.h"
#include "u_domain.h"
#include "os_report.h"

#define HEXVALUE unsigned int
#define _METANAME(m) c_metaObject(m)->name
#define iprintf { int i; for (i=0; i<actionData->depth; i++) printf("    "); } printf

typedef struct toolActionData {
    FILE *fin;
    FILE *fout;
    c_long depth;
    c_iter stack;
    c_long size;
} *toolActionData;

static void printType (c_type type, toolActionData actionData);

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
print_usage ()
{
    printf ("********************************************************************************\n");
    printf ("DataBase Dump (dbd) is an experimental tool to print shm content to stdout.     \n");
    printf ("This tool operates read only on shm and doesn't perform any locking.            \n");
    printf ("Subsequently it may crash when reading unstable data.                           \n");
    printf ("--------------------------------------------------------------------------------\n");
    printf ("Usage:\n");
    printf ("dbd> <addr> <enter>               /* show object content                      */\n");
    printf ("dbd> s <enter>                    /* show services                            */\n");
    printf ("dbd> t [<expr>] <enter>           /* show topics if name matches <expr>       */\n");
    printf ("dbd> d [<expr>] <enter>           /* show partitions if name matches <expr>   */\n");
    printf ("dbd> p [<expr>] <enter>           /* show participants if name matches <expr> */\n");
    printf ("dbd> g [<expr>][<expr>] <enter>   /* show groups if topic and partition       */\n");
    printf ("                                  /* names matches the expressions            */\n");
    printf ("dbd> r [<addr>] <enter>           /* show readers associated to entity <addr> */\n");
    printf ("dbd> w [<addr>] <enter>           /* show writers associated to entity <addr> */\n");
    printf ("dbd> e [<addr>] <enter>           /* show contained entities                  */\n");
    printf ("dbd> l <enter>                    /* show <address> history                   */\n");
    printf ("dbd> q <enter>                    /* quit                                     */\n");
    printf ("Example:\n");
    printf ("dbd> 0x200a2df8 <enter>           /* Print content of shm address             */\n");
    printf ("********************************************************************************\n");
}

static void
printWalkHistory (
    c_object o,
    c_iterActionArg arg);

static void
printHistory (
    c_iter history,
    c_long cursor);

#define OBJECT_PUSH(actionData, obj) \
        (actionData)->stack = c_iterInsert((actionData)->stack, obj); \
        (actionData)->depth++

#define OBJECT_POP(actionData) \
        c_iterTakeFirst((actionData)->stack); \
        (actionData)->depth--

static c_type v_handle_t = NULL;
static c_type v_topic_t = NULL;
static c_type v_partition_t = NULL;

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
OPENSPLICE_MAIN(ospl_dbd)
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
            int stop = 0;
            int len;
            void *addr;
            int i, c;

            element = u_participantGetConfiguration(participant);
            if (element) {
                printf("Node name: %s\n", u_cfNodeName((u_cfNode)element));
                printf("Node kind: %d\n", u_cfNodeKind((u_cfNode)element));
            } else {
                printf("No configuration for: %s\n", uri);
            }

            domain = u_participantDomain(participant);
            if (domain) {
                printf("Domain memory address: 0x%x\n",
                       (HEXVALUE)u_domainMemoryAddress(domain));
                printf("Domain memory size: 0x%x\n",
                       (HEXVALUE)u_domainMemorySize(domain));
            } else {
                printf("No domain for Participant\n");
            }

            print_usage();

            ur = u_entityAction(u_entity(participant),
                                initAction,
                                &addr);
            printf("Kernel Address <0x%x>\n",(HEXVALUE)addr);

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
                            len = sscanf(buf2,"0x%x",(HEXVALUE *)&addr);
                            if (len == 1) {
                                ur = u_entityAction(u_entity(participant),
                                                    lookupReadersAction,
                                                    addr);
                            } else {
                                printf("usage: r <address>\n");
                            }
                        }
                    } else if (strcmp(buf1, "w") == 0) {
                        if (strlen(buf2) == 0) {
                            ur = u_entityAction(u_entity(participant),
                                                lookupWritersAction,
                                                NULL);
                        } else {
                            len = sscanf(buf2,"0x%x",(HEXVALUE *)&addr);
                            if (len == 1) {
                                ur = u_entityAction(u_entity(participant),
                                                    lookupWritersAction,
                                                    addr);
                            } else {
                                printf("usage: w <address>\n");
                            }
                        }
                    } else if (strcmp(buf1, "e") == 0) {
                        len = sscanf(buf2,"0x%x",(HEXVALUE *)&addr);
                        if (len == 1) {
                            ur = u_entityAction(u_entity(participant),
                                                lookupEntitiesAction,
                                                addr);
                        } else {
                            printf("usage: e <address>\n");
                        }
                    } else if (strcmp(buf1, "l") == 0) {
                        c_iterWalk(history,printWalkHistory,NULL);
                    } else { 
                        len = sscanf(buf1,"0x%x",(HEXVALUE *)&addr);
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
printWalkHistory (
    c_object o,
    c_iterActionArg arg)
{
    c_type type;
    c_char *name, *ename;

    if (o) {
        type = c_getType(o);
        name = c_metaScopedName(c_metaObject(type));
        printf("<0x%x> %s",(HEXVALUE)o,name);
        if (c_checkType(o, "v_entity") == o) {
            ename = v_entityName(o);
            if (ename != NULL) {
                printf(" /* %s */", ename);
            }
        } else if (c_checkType(o, "c_metaObject") == o) {
            ename = c_metaScopedName(o);
            if (ename != NULL) {
                printf(" /* %s */", ename);
                os_free(ename);
            }
        }
        printf("\n");
    } else {
        printf("<0x0>\n");
    }
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
    printf("<0x%x> %s",(HEXVALUE)o,name);
    if (c_checkType(o, "v_entity") == o) {
        ename = v_entityName(o);
        if (ename != NULL) {
            printf(" /* %s */", ename);
        }
    }
    printf("\n");
}

static c_bool
printCollectionAction(
    c_object o,
    c_voidp arg)
{
    c_type type;
    toolActionData actionData = (toolActionData)arg;
    c_char *name;

    if (o != NULL) {
        type = c_getType(o);
        name = c_metaScopedName(c_metaObject(type));
        iprintf("Object <0x%x> type is: <0x%x> %s\n",
                (HEXVALUE)o, (HEXVALUE)type, name);
        os_free(name);
        OBJECT_PUSH(actionData, o);
        printType(type, actionData);
        printf("\n");
        OBJECT_POP(actionData);
    }
    return TRUE;
}

static void
printCollection(
    c_collectionType type,
    toolActionData actionData)
{
    c_long size, i, offset, esize;
    c_object o, *p;
    c_object arrayElement;
    c_type subtype;
    c_bool isRef;

    o = c_iterObject(actionData->stack, 0);
    switch (type->kind) {
    case C_ARRAY:
    case C_SEQUENCE:
        /* Walk over all entries */
        switch (type->kind) {
        case C_ARRAY:
            if (type->maxSize == 0) {
                size = c_arraySize((c_array)o);
            } else {
                size = type->maxSize;
            }
        break;
        case C_SEQUENCE:
            size = c_arraySize((c_array)o);
        break;
        default:
            size = 0;
            assert(FALSE);
        break;
        }

        if (c_typeIsRef(type->subType)) {
            esize = sizeof(c_voidp);
            isRef = TRUE;
        } else {
            esize = type->subType->size;
            isRef = FALSE;
        }
        p = (c_object *)o;
        offset = 0;
        for (i=0; i<size; i++) {
            iprintf("Element (%d) Offset (%d)\n",i,offset);
            arrayElement = *p;
            if (arrayElement != NULL) {
                OBJECT_PUSH(actionData, arrayElement);
                if (isRef) {
                    subtype = c_getType(arrayElement);
                    printType(subtype, actionData);
                } else {
                    iprintf(" ");
                    printType(type->subType, actionData);
                }
                printf("\n");
                OBJECT_POP(actionData);
            } else {
                iprintf("    <0x0>\n");
            }
            p = C_DISPLACE(p, esize);
            offset += esize;
        }
    break;
    case C_STRING:
        printf(" \"%s\"",(c_char *)o);
    break;
    case C_SET:
    case C_LIST:
    case C_BAG:
    case C_DICTIONARY:
    case C_QUERY:
    {
        if (o != NULL) {
            /* Walk over the elements */
            c_walk(o, (c_action)printCollectionAction, actionData);
            if (c_count(o) == 0) {
                iprintf("<EMPTY>");
            }
        } else {
            iprintf("<NULL>");
        }
    }
    break;
    case C_SCOPE:
        c_scopeWalk(o, printCollectionAction, actionData);
    break;
    default:
        printf("Specified type <0x%x> is not a valid collection type\n",
               (HEXVALUE)type);
    break;
    }
}

static void
printPrimitive(
    c_type _this,
    toolActionData actionData)
{
    c_object o;
    c_value v;
    c_char *str;

    #define _CASE_(l,f,t) \
            case P_##l: \
            { \
                v.is.f = *(t *)o; \
                v.kind = V_##l; \
                str = c_valueImage(v); \
                printf("%s",c_valueImage(v)); \
                os_free(str); \
            } \
            break

    o = c_iterObject(actionData->stack, 0);
    switch(c_primitiveKind(_this)) {
    _CASE_(BOOLEAN,Boolean,c_bool);
    _CASE_(SHORT,Short,c_short);
    _CASE_(LONG,Long,c_long);
    _CASE_(LONGLONG,LongLong,c_longlong);
    _CASE_(OCTET,Octet,c_octet);
    _CASE_(USHORT,UShort,c_ushort);
    _CASE_(ULONG,ULong,c_ulong);
    _CASE_(ULONGLONG,ULongLong,c_ulonglong);
    _CASE_(CHAR,Char,c_char);
    _CASE_(WCHAR,WChar,c_wchar);
    _CASE_(FLOAT,Float,c_float);
    _CASE_(DOUBLE,Double,c_double);
    case P_ADDRESS:
        v.is.Address = *(c_address *)o;
        v.kind = V_ADDRESS;
        str = c_valueImage(v);
        printf("<%s>",c_valueImage(v));
        os_free(str);
    break;
    case P_VOIDP:
        v.is.Voidp = *(c_voidp *)o;
        v.kind = V_VOIDP;
        str = c_valueImage(v);
        printf("<%s>",c_valueImage(v));
        os_free(str);
    break;
    case P_MUTEX:
    case P_LOCK:
    case P_COND:
        printf("<******>");
    break;
    default:
        printf("Specified type <0x%x> is not a valid primitive type\n",
               (HEXVALUE)_this);
    break;
    }
    #undef _CASE_
}

static void
printEnum(
    c_enumeration _this,
    toolActionData actionData)
{
    c_object o;
    c_long i, size;
    c_constant constant;

    o = c_iterObject(actionData->stack, 0);
    size = c_enumerationCount(_this);
    i = *(c_long *)o;
    if ((i < 0) || (i > size)) {
        printf("(%d) \"Bad enumeration value\"\n", i);
    } else {
        constant = c_enumeration(_this)->elements[i];
        printf("%s",_METANAME(constant));
    }
}

static void
printStructure(
    c_structure _this,
    toolActionData actionData)
{
    c_object o;
    c_object object;
    c_long i, size;
    c_member member;

    o = c_iterObject(actionData->stack, 0);

    if (o) {
        if (c_iterLength(actionData->stack) == 1) {
            printf("%s ", _METANAME(_this));
        } else {
        }
        printf("{\n");
        size = c_structureMemberCount(_this);
        for (i=0; i<size; i++) {
            member = c_structureMember(_this, i);
            object = C_DISPLACE(o, (c_address)member->offset);
            if (c_typeIsRef(c_memberType(member))) {
                iprintf("    %s <0x%x>\n",
                       c_specifierName(member),
                       *(HEXVALUE *)object);
            } else {
                OBJECT_PUSH(actionData, object);
                iprintf("%s ",c_specifierName(member));
                printType(c_memberType(member), actionData);
                printf("\n");
                OBJECT_POP(actionData);
            }
        }
        if (c_iterLength(actionData->stack) == 1) {
            iprintf("};");
        } else {
            iprintf("} %s;", _METANAME(_this));
        }
        if (c_type(_this) == v_handle_t) {
            v_object vo;
            v_handleClaim(*(v_handle *)o,&vo);
            v_handleRelease(*(v_handle *)o);
            printf(" /* Handle's object: <0x%x> */", (HEXVALUE)vo);
        }
    }
}

static c_bool
walkProperty(
    c_metaObject object,
    c_metaWalkActionArg actionArg)
{
    toolActionData actionData = (toolActionData)actionArg;
    c_object o;
    c_voidp addr;

    o = c_iterObject(actionData->stack, 0);

    if (c_baseObjectKind(object) == M_ATTRIBUTE) {
        addr = C_DISPLACE(o, (c_address)c_property(object)->offset);
        if (c_typeIsRef(c_property(object)->type)) {
            addr = *(c_voidp *)addr;
            if (c_baseObjectKind(c_property(object)->type) == M_COLLECTION) {
                if (addr) {
                    if (c_iterContains(actionData->stack, addr)) {
                        printf("Ignore cyclic reference 0x%x\n",
                               (HEXVALUE)addr);
                    } else {
                        OBJECT_PUSH(actionData, addr);
                        iprintf("%s ",_METANAME(object));
                        printType(c_property(object)->type, actionData);
                        OBJECT_POP(actionData);
                    }
                } else {
                    iprintf("    %s <0x%x>", _METANAME(object), (HEXVALUE)addr);
                } 
            } else {
                iprintf("    %s <0x%x>", _METANAME(object), (HEXVALUE)addr);
                if (addr) {
                    /* Section for code to print additional type specific info. */
                    if (c_property(object)->type == v_topic_t) {
                        printf(" /* topic name is: %s */", v_topicName(addr));
                    }
                    if (c_property(object)->type == v_partition_t) {
                        printf(" /* Partition: %s */", v_partitionName(addr));
                    }
                    if (c_property(object)->type == c_type_t(c_getBase(object))) {
                        printf(" /* Type: %s */", _METANAME(c_type(addr)));
                    }
                }
            }
            printf("\n");
        } else {
            OBJECT_PUSH(actionData, addr);
            iprintf("%s ",_METANAME(object));
            printType(c_property(object)->type, actionData);
            printf("\n");
            OBJECT_POP(actionData);
        }
    }
    return TRUE;
}

static void
printInterface(
    c_interface interf,
    toolActionData actionData)
{
    c_object o;

    o = c_iterObject(actionData->stack, 0);
    /* Only walk over the properties if the reference is valid. */
    if (o != NULL) {
        iprintf("%s {\n", _METANAME(interf));
        c_metaWalk(c_metaObject(interf),
                   (c_metaWalkAction)walkProperty,
                   actionData);
        iprintf("};");
    }
}

static void
printClass(
    c_class class,
    toolActionData actionData)
{
    if (class->extends) {
        printClass(class->extends, actionData);
        printf("\n");
    }
    printInterface(c_interface(class), actionData);
}

static void
printUnion(
    c_union v_union,
    toolActionData actionData)
{
    c_type switchType;
    c_type caseType;
    c_unionCase deflt;
    c_unionCase activeCase;
    c_unionCase currentCase;
    c_object unionData;
    c_value switchValue;
    c_literal label;
    c_object o;
    int i,j, nCases, nLabels;

    c_long dataOffset;

    o = c_iterObject(actionData->stack, 0);
    /* action for the union itself */
    /* No action, but separate actions for the switch and the data */
    /* action(c_type(v_union), o, actionArg); */
    /* action for the switch */
    printType(v_union->switchType, actionData);
    printf("\n");

    switchType = c_typeActualType(v_union->switchType);
    /* Determine value of the switch field */
    switch (c_baseObjectKind(switchType)) {
    case M_PRIMITIVE:
        switch (c_primitive(switchType)->kind) {
#define __CASE__(prim, type) \
        case prim: switchValue = type##Value(((type)o)); break;
        __CASE__(P_BOOLEAN,c_bool)
        __CASE__(P_CHAR,c_char)
        __CASE__(P_SHORT,c_short)
        __CASE__(P_USHORT,c_ushort)
        __CASE__(P_LONG,c_long)
        __CASE__(P_ULONG,c_ulong)
        __CASE__(P_LONGLONG,c_longlong)
        __CASE__(P_ULONGLONG,c_ulonglong)
#undef __CASE__
        default:
            switchValue = c_undefinedValue();
            assert(FALSE);
        break;
        }
    break;
    case M_ENUMERATION:
        switchValue = c_longValue((c_long)o);
    break;
    default:
        switchValue = c_undefinedValue();
        assert(FALSE);
    break;
    }

    /* Determine the label corresponding to this field */

    activeCase = NULL;
    deflt = NULL;
    nCases = c_arraySize(v_union->cases);

    for (i=0; (i<nCases) && !activeCase; i++) {
        currentCase = c_unionCase(v_union->cases[i]);
        nLabels = c_arraySize(currentCase->labels);
        if (nLabels > 0) {
            for (j=0; (j<nLabels) && !activeCase; j++) {
                label = c_literal(currentCase->labels[j]);
                if (c_valueCompare(switchValue, label->value) == C_EQ) {
                    activeCase = currentCase;
                }
            }
        } else {
            deflt = currentCase;
        }
    }
    if (!activeCase) {
        activeCase = deflt;
    }
    assert(activeCase);
    if (activeCase) {
        caseType = c_specifier(activeCase)->type;
        if (c_type(v_union)->alignment >= v_union->switchType->size) {
            dataOffset = c_type(v_union)->alignment;
        } else {
            dataOffset = v_union->switchType->size;
        }
        unionData = C_DISPLACE(o, (c_address)dataOffset);
        OBJECT_PUSH(actionData, unionData);
        printType(caseType, actionData);
        printf("\n");
        OBJECT_POP(actionData);
    }
}


static void
printCollectionKind(
    c_collectionType type,
    toolActionData actionData)
{
    c_long size;
    c_object o;

    o = c_iterObject(actionData->stack, 0);
    if (o != NULL) {
        switch (type->kind) {
        case C_ARRAY:
            size = c_collectionType(type)->maxSize;
            if (size == 0) {
                printf("<0x%x> /* C_ARRAY(%d) */", (HEXVALUE)o, c_arraySize(o));
            } else {
                printf("<0x%x> /* Bounded C_ARRAY(%d) */", (HEXVALUE)o, size);
            }
        break;
        case C_SEQUENCE:
            printf("<0x%x> /* C_SEQUENCE(%d) */", (HEXVALUE)o, c_sequenceSize(o));
        break;
        case C_STRING:
            printf(" \"%s\"",(c_char *)o);
        break;
        case C_SET:
            printf("<0x%x> /* C_SET(%d) */", (HEXVALUE)o, c_count(o));
        break;
        case C_LIST:
            printf("<0x%x> /* C_LIST(%d) */", (HEXVALUE)o, c_count(o));
        break;
        case C_BAG:
            printf("<0x%x> /* C_BAG(%d) */", (HEXVALUE)o, c_count(o));
        break;
        case C_DICTIONARY:
            printf("<0x%x> /* C_DICTIONARY(%d) */", (HEXVALUE)o, c_count(o));
        break;
        case C_QUERY:
            printf("<0x%x> /* C_QUERY(%d) */", (HEXVALUE)o, c_count(o));
        break;
        case C_SCOPE:
            printf("<0x%x> /* C_SCOPE(%d) */", (HEXVALUE)o, c_scopeCount(o));
        break;
        default:
            printf("Specified type <0x%x> is not a valid collection type\n",
                   (HEXVALUE)type);
        break;
        }
    } else {
        printf("<0x%x> \"Unexpected Internal Stack Error\"", (HEXVALUE)o);
    }
}

static void
printType (
    c_type type,
    toolActionData actionData)
{
    c_type actualType;

    actualType = c_typeActualType(type);
    /* Determine which action to take */
    switch (c_baseObjectKind(actualType)) {
    case M_PRIMITIVE:
        printPrimitive(actualType, actionData);
    break;
    case M_ENUMERATION:
        printEnum(c_enumeration(actualType), actionData);
    break;
    case M_STRUCTURE:
    case M_EXCEPTION:
        printStructure(c_structure(actualType), actionData);
    break;
    case M_COLLECTION:
        if (c_iterLength(actionData->stack) == 1) {
            printCollection(c_collectionType(actualType), actionData);
        } else {
            printCollectionKind(c_collectionType(actualType), actionData);
        }
    break;
    case M_CLASS:
        printClass(c_class(actualType), actionData);
    break;
    case M_INTERFACE:
        printInterface(c_interface(actualType), actionData);
    break;
    case M_UNION:
        printUnion(c_union(actualType), actionData);
    break;
    default:
        printf("Specified type <0x%x> is not a valid type\n",
               (HEXVALUE)type);
        assert(FALSE); /* Only descendants of type possible */
    break;
    }
}

static void
printStructureMember(
    c_structure _this,
    toolActionData actionData,
    c_address offset)
{
}

static void
printCollectionElement(
    c_collectionType _this,
    toolActionData actionData,
    c_address offset)
{
}


typedef struct lookupMetaData {
    c_long offset;
    c_metaObject metaObject;
} *lookupMetaData;

static c_bool
lookupProperty(
    c_metaObject object,
    c_metaWalkActionArg actionArg)
{
    lookupMetaData actionData = (lookupMetaData)actionArg;
    c_long offset, size;
    c_bool proceed = TRUE;

    if (c_baseObjectKind(object) == M_ATTRIBUTE) {
        offset = actionData->offset - c_property(object)->offset;
        size = c_typeSize(c_property(object)->type);
        if ((offset >= 0) && (offset < size)) {
            assert(actionData->metaObject == NULL);
            actionData->metaObject = object;
            actionData->offset -= offset; /* calculate correct offset */
            proceed = FALSE;
        }
    }
    return proceed;
}

static void
printInterfaceAttribute(
    c_interface _this,
    toolActionData actionData,
    c_address offset)
{
    struct lookupMetaData MetaData;
    c_object o, object;
    c_string name;

    o = c_iterObject(actionData->stack, 0);
    /* Only walk over the properties if the reference is valid. */
    if (o != NULL) {
        MetaData.offset = offset;
        MetaData.metaObject = NULL;
        c_metaWalk(c_metaObject(_this),
                   (c_metaWalkAction)lookupProperty,
                   &MetaData);
        if (MetaData.metaObject != NULL) {
            c_iter stack;

            name = c_metaScopedName(MetaData.metaObject);
            if (MetaData.offset < (c_long)offset) {
                printf("Address is %lu bytes in property %s\n",
                       (offset - MetaData.offset), name);
            } else {
                printf("Address refers to property %s\n",
                       name);
            }
            os_free(name);

            stack = actionData->stack;
            actionData->stack = NULL;
            object = C_DISPLACE(o, (c_address)MetaData.offset);
            OBJECT_PUSH(actionData, object);
            printType(c_property(MetaData.metaObject)->type, actionData);
            printf("\n");
            OBJECT_POP(actionData);
            c_iterFree(actionData->stack);
            actionData->stack = stack;
        }
    }
}

static void
printClassAttribute(
    c_class _this,
    toolActionData actionData,
    c_address offset)
{
    c_long size;

    size = 0;
    if (_this->extends) {
        size = c_typeSize(c_type(_this->extends));
        if (offset < (c_address)size) {
            printClassAttribute(_this->extends, actionData, offset);
        }
    }
    printInterfaceAttribute(c_interface(_this), actionData, offset);
}

static void
printUnionMember(
    c_union _this,
    toolActionData actionData,
    c_address offset)
{
}

static void
tryPrintOffset(
    c_voidp o,
    toolActionData actionData,
    c_address offset)
{
    c_type type, actualType;

    type = c_getType(o);
    actualType = c_typeActualType(type);
        
    /* Determine which action to take */
    switch (c_baseObjectKind(actualType)) {
    case M_PRIMITIVE:
        printPrimitive(actualType, actionData);
    break;
    case M_ENUMERATION:
        printEnum(c_enumeration(actualType), actionData);
    break;
    case M_STRUCTURE:
    case M_EXCEPTION:
        printStructureMember(c_structure(actualType), actionData, offset);
    break;
    case M_COLLECTION:
        printCollectionElement(c_collectionType(actualType), actionData, offset);
    break;
    case M_CLASS:
        printClassAttribute(c_class(actualType), actionData, offset);
    break;
    case M_INTERFACE:
        printInterfaceAttribute(c_interface(actualType), actionData, offset);
    break;
    case M_UNION:
        printUnionMember(c_union(actualType), actionData, offset);
    break;
    default:
        printf("Specified type <0x%x> is not a valid type\n",
               (HEXVALUE)type);
        assert(FALSE); /* Only descendants of type possible */
    break;
    }
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
                       "object starting at 0x%x\n",
                       offset, _METANAME(type), (HEXVALUE)o);
                OBJECT_PUSH(&actionData, o);
                tryPrintOffset(o,&actionData,offset);
                OBJECT_POP(&actionData);
            } else {
                printf("Warning: address is %lu bytes in "
                       "memory starting at 0x%x\n",
                   offset, (HEXVALUE)o);
            }
        } else {
            name = c_metaScopedName(c_metaObject(type));
            printf("Object <0x%x> size=%d type is: <0x%x> %s\n", 
                   (HEXVALUE)o, size, (HEXVALUE)type, name);
            os_free(name);
            OBJECT_PUSH(&actionData, o);
            printType(type, &actionData);
            printf("\n");
            OBJECT_POP(&actionData);
        }
    } else {
        printf("Address <0x%x> is not a Database Object\n",
               (HEXVALUE)args);
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
            printf("<0x%x> /* %s */\n",(HEXVALUE)_this, v_entityName2(p));
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
        printf("<0x%x> /* %s */\n",(HEXVALUE)addr, v_entityName2(addr));
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
        printf("<0x%x> /* %s */\n",(HEXVALUE)addr, v_entityName2(addr));
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
        printf("<0x%x> /* %s */\n",(HEXVALUE)addr, v_entityName2(addr));
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
        printf("<0x%x> /* %s */\n",(HEXVALUE)addr, v_entityName2(addr));
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
            printf("<0x%x> /* %s */\n",(HEXVALUE)addr, v_entityName2(addr));
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
                printf("Lookup Reader for provided address 0x%x is not supported\n",
                       (HEXVALUE)arg);
            break;
            }
            addr = c_iterTakeFirst(list);
            while (addr != NULL) {
                printf("<0x%x> /* %s */\n",(HEXVALUE)addr, v_entityName2(addr));
                addr = c_iterTakeFirst(list);
            }
            c_iterFree(list);
        } else {
            printf("Given address 0x%x is invalid\n", (HEXVALUE)arg);
        }
    } else {
        printf("Given address 0x%x is invalid\n", (HEXVALUE)arg);
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
            printf("<0x%x> /* %s */\n",(HEXVALUE)addr, v_entityName2(addr));
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
                printf("Lookup Writer for provided address 0x%x is not supported\n",
                       (HEXVALUE)arg);
            break;
            }
            addr = c_iterTakeFirst(list);
            while (addr != NULL) {
                printf("<0x%x> /* %s */\n",(HEXVALUE)addr, v_entityName2(addr));
                addr = c_iterTakeFirst(list);
            }
            c_iterFree(list);
        } else {
            printf("Given address 0x%x is invalid\n", (HEXVALUE)arg);
        }
    } else {
        printf("Given address 0x%x is invalid\n", (HEXVALUE)arg);
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
    v_kernel kernel;
    c_iter list;
    c_object o;
    c_base base;
    c_voidp addr;

    kernel = v_objectKernel(entity);
    base = c_getBase(entity);
    o = c_baseCheckPtr(base, arg);
    if (o) {
        if (o == arg) {
            list = NULL;
            v_entityWalkEntities(o,lookupEntityAction,&list);
            addr = c_iterTakeFirst(list);
            while (addr != NULL) {
                printf("<0x%x> /* %s */\n",(HEXVALUE)addr, v_entityName2(addr));
                addr = c_iterTakeFirst(list);
            }
            c_iterFree(list);
        } else {
            printf("Given address 0x%x is invalid\n", (HEXVALUE)arg);
        }
    } else {
        printf("Given address 0x%x is invalid\n", (HEXVALUE)arg);
    }
}


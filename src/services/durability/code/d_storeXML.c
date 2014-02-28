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
#include "os.h"
#include "d_misc.h"
#include "d__storeXML.h"
#include "d_storeXML.h"
#include "d_lock.h"
#include "d_store.h"
#include "d_nameSpace.h"
#include "d_admin.h"
#include "d_configuration.h"
#include "d_table.h"
#include "d_actionQueue.h"
#include "d_object.h"
#include "u_user.h"
#include "u_group.h"
#include "u_entity.h"
#include "u_partition.h"
#include "u_topic.h"
#include "v_public.h"
#include "v_entity.h"
#include "v_time.h"
#include "v_topicQos.h"
#include "v_topic.h"
#include "v_message.h"
#include "v_partition.h"
#include "v_state.h"
#include "sd_serializer.h"
#include "sd_serializerXML.h"
#include "sd_serializerXMLMetadata.h"
#include "sd_serializerXMLTypeinfo.h"
#include "c_base.h"
#include "c_laptime.h"
#include "os_heap.h"
#include "os_time.h"
#include "os_stdlib.h"
#include "os_report.h"

#include <limits.h>

#ifdef WIN32
#define D_SEEK_LENGTH (-10)
#else
#define D_SEEK_LENGTH (-9)
#endif

C_CLASS(d_topicMetadata);

C_STRUCT(d_topicMetadata) {
    c_char* name;
    c_char* typeName;
    c_char* keyList;
    v_topicQos qos;
    c_type type;
};

#define d_topicMetadata(t) ((d_topicMetadata)(t))

C_CLASS(persistentInstance);

C_STRUCT(persistentInstance) {
    c_char* keyValue;
    v_groupAction newMessage;
    c_iter messages; /* v_message */
    d_storeResult result;
    c_long writeCount;
    c_long disposeCount;
    c_long registerCount;
    c_long unregisterCount;
};

C_CLASS(groupExpungeActions);

C_STRUCT(groupExpungeActions){
    c_char* partition;
    c_char* topic;
    d_table instances; /*<persistentInstanceCompare, persistentInstanceFree>*/
};

#define groupExpungeActions(i) ((groupExpungeActions)(i))
#define persistentInstance(i) ((persistentInstance)(i))

static d_storeResult
groupsReadXMLUnsafe(
    const d_store store,
    d_groupList *list);

static c_char*
getDataFileNameBasedOnPath(
    const c_char* storeDir,
    const c_char* partition,
    const c_char* topic);

static c_char*
getBakFileNameBasedOnPath(
    const c_char* storeDir,
    const c_char* partition,
    const c_char* topic);

static c_char*
getMetaFileNameBasedOnPath(
    const c_char* storeDir,
    const c_char* partition,
    const c_char* topic);

static d_storeResult
d_storeNsMarkCompleteXMLBasedOnPath(
    const c_char* storeDir,
    const d_nameSpace nameSpace,
    c_bool isComplete);

static d_storeResult
getDataVersion(
    const d_storeXML persistentStore,
    const c_char* partition,
    const c_char* topic,
    c_ulong* version);

static d_storeFile
d_storeFileNew(
    const c_char* path,
    FILE* fdes,
    const c_char* mode)
{
    d_storeFile f;

    assert(path);

    f = d_storeFile(os_malloc(C_SIZEOF(d_storeFile)));
    f->path = os_strdup(path);

    if(fdes){
        f->fdes = fdes;
    } else {
        f->fdes = NULL;
    }
    if(mode){
        f->mode = os_strdup(mode);
    } else {
        f->mode = NULL;
    }

    return f;
}

static void
d_storeFileFree(
    d_storeFile f)
{
    if(f){
        if(f->path){
            os_free(f->path);
        }
        if(f->mode){
            os_free(f->mode);
        }
        if(f->fdes){
            fclose(f->fdes);
        }
        os_free(f);
    }
    return;
}

static int
d_storeFileCompare(
    d_storeFile f1,
    d_storeFile f2)
{
    int result;

    if(f1 && f2){
        if(f1->path && f2->path){
            result = strcmp(f1->path, f2->path);
        } else if(f1->path){
            result = 1;
        } else if(f2->path){
            result = -1;
        } else {
            result = 0;
        }
    } else if(f1){
        result = 1;
    } else if(f2){
        result = -1;
    } else {
        result = 0;
    }
    return result;
}

static d_storeResult
actionStopUnlocked(
    d_storeXML store)
{
    d_storeXML(store)->sessionAlive = FALSE;
    d_tableFree(store->openedFiles);
    store->openedFiles = NULL;
    return D_STORE_RESULT_OK;
}

static c_bool
isDeprecatedMetadataFormat(
    c_char* meta)
{
    c_bool result;

    if(meta && strlen(meta) > 26){
        if(os_strncasecmp(meta, "<MetaData version=\"1.0.0\">", 26) == 0){
            result = FALSE;
        } else {
            result = TRUE;
        }
    } else {
        result = TRUE;
    }
    return result;
}


static void
setKernelGroup(
    v_entity entity,
    c_voidp args)
{
    d_group group;

    group = d_group(args);
    d_groupSetKernelGroup(group,v_group(entity));
}


static c_char*
stringKeyEscape(
    const c_char* str,
    c_bool escapeComma)
{
    os_size_t i, length, index, allocLength;
    c_char c;
    c_char* result = NULL;

    if(str){
        length = strlen(str);
        allocLength = length;
        result = (c_char*)(os_malloc(length + 1));
        index = 0;

        for(i=0; i<length; i++){
            if(index >= allocLength){
                allocLength += length;
                result = os_realloc(result, allocLength + 1);
            }
            if(result == NULL){
                break;
            }
            c = str[i];

            switch(c){
                case '\n':
                    result[index++] = '%';
                    result[index++] = '0';
                    result[index++] = '1';
                    result[index++] = '0';
                    break;
                case '%':
                    result[index++] = '%';
                    result[index++] = '0';
                    result[index++] = '3';
                    result[index++] = '7';
                    break;
                case '&':
                    result[index++] = '%';
                    result[index++] = '0';
                    result[index++] = '3';
                    result[index++] = '8';
                    break;
                case ',':
                    if(escapeComma){
                        result[index++] = '%';
                        result[index++] = '0';
                        result[index++] = '4';
                        result[index++] = '4';
                    } else {
                        result[index++] = c;
                    }
                    break;
                case '<':
                    result[index++] = '%';
                    result[index++] = '0';
                    result[index++] = '6';
                    result[index++] = '0';
                    break;
                case '>':
                    result[index++] = '%';
                    result[index++] = '0';
                    result[index++] = '6';
                    result[index++] = '2';
                    break;
                default:
                    result[index++] = c;
                    break;
            }
        }
        if(result){
            result[index++] = '\0';
        }
    }
    return result;
}

/*
static c_char*
stringKeyUnescape(
    const c_char* escaped)
{
    int i, length, index, code;
    c_char c;
    c_char *tmp;
    c_char *str = NULL;
    c_char codeString[4];

    if(escaped){
        length = (int)strlen(escaped);
        index = 0;

        tmp = os_malloc(length+1);

        for(i=0; i<length; i++){
            c = escaped[i];

            switch(c){
                case '%':
                    i++;
                    codeString[0] = escaped[i++];
                    codeString[1] = escaped[i++];
                    codeString[2] = escaped[i];
                    codeString[3] = '\0';

                    code = atoi(codeString);

                    switch(code){
                        case 10:
                            tmp[index++] = '\n';
                            break;
                        case 37:
                            tmp[index++] = '%';
                            break;
                        case 38:
                            tmp[index++] = '&';
                            break;
                        case 44:
                            tmp[index++] = ',';
                            break;
                        case 60:
                            tmp[index++] = '<';
                            break;
                        case 62:
                            tmp[index++] = '>';
                            break;
                        default:
                            OS_REPORT(OS_ERROR, "durability", 0, "Found unsupported ASCII code.");
                            tmp[index++] = c;
                            break;
                    }
                    break;
                default:
                    tmp[index++] = c;
                    break;
            }
        }
        tmp[index++] = '\0';
        str = (c_char*)(os_malloc(strlen(tmp) + 1));

        if(str){
            os_sprintf(str, "%s", tmp);
        }
        os_free(tmp);
    }
    return str;
}
*/

static c_bool
convertOldToNewKeyList(
    const c_char* oldKeyValues,
    c_char** newKeyList)
{
    c_char *escapedKeyValues;
    os_size_t length;
    c_bool converted = FALSE;

    if(newKeyList && oldKeyValues){
        escapedKeyValues = stringKeyEscape(oldKeyValues, FALSE);

        if(escapedKeyValues){
            length = strlen(KEY_START_TAG) +
                     strlen(escapedKeyValues) +
                     strlen(KEY_END_TAG) + 1;

            *newKeyList = (c_char *)os_malloc(length);

            if(*newKeyList){
                os_sprintf(*newKeyList, "%s%s%s",
                        KEY_START_TAG, escapedKeyValues, KEY_END_TAG);
                converted = TRUE;
            }
            os_free(escapedKeyValues);
        }
    }
    return converted;
}

/*****************************************************************************
       0 NUL   1 SOH    2 STX    3 ETX    4 EOT    5 ENQ    6 ACK    7 BEL
       8 BS    9 HT    10 NL    11 VT    12 NP    13 CR    14 SO    15 SI
      16 DLE  17 DC1   18 DC2   19 DC3   20 DC4   21 NAK   22 SYN   23 ETB
      24 CAN  25 EM    26 SUB   27 ESC   28 FS    29 GS    30 RS    31 US
      32 SP   33 !     34 "     35 #     36 $     37 %     38 &     39 '
      40 (    41 )     42 *     43 +     44 ,     45 -     46 .     47 /
      48 0    49 1     50 2     51 3     52 4     53 5     54 6     55 7
      56 8    57 9     58 :     59 ;     60 <     61 =     62 >     63 ?
      64 @    65 A     66 B     67 C     68 D     69 E     70 F     71 G
      72 H    73 I     74 J     75 K     76 L     77 M     78 N     79 O
      80 P    81 Q     82 R     83 S     84 T     85 U     86 V     87 W
      88 X    89 Y     90 Z     91 [     92 \     93 ]     94 ^     95 _
      96 `    97 a     98 b     99 c    100 d    101 e    102 f    103 g
     104 h   105 i    106 j    107 k    108 l    109 m    110 n    111 o
     112 p   113 q    114 r    115 s    116 t    117 u    118 v    119 w
     120 x   121 y    122 z    123 {    124 |    125 }    126 ~    127 DEL

*****************************************************************************/

static c_char*
stringToURI(
    const c_char* str)
{
    int i, length, index;
    c_char c;
    c_char tmp[512];
    c_char* uri = NULL;

    if(str){
        length = (int)strlen(str);
        index = 0;

        for(i=0; i<length; i++){
            c = str[i];

            switch(c){
                case ' ':
                    tmp[index++] = '%';
                    tmp[index++] = '0';
                    tmp[index++] = '2';
                    tmp[index++] = '0';
                    break;
                case '!':
                    tmp[index++] = '%';
                    tmp[index++] = '0';
                    tmp[index++] = '3';
                    tmp[index++] = '3';
                    break;
                case '"':
                    tmp[index++] = '%';
                    tmp[index++] = '0';
                    tmp[index++] = '3';
                    tmp[index++] = '4';
                    break;
                case '#':
                    tmp[index++] = '%';
                    tmp[index++] = '0';
                    tmp[index++] = '3';
                    tmp[index++] = '5';
                    break;
                case '$':
                    tmp[index++] = '%';
                    tmp[index++] = '0';
                    tmp[index++] = '3';
                    tmp[index++] = '6';
                    break;
                case '%':
                    tmp[index++] = '%';
                    tmp[index++] = '0';
                    tmp[index++] = '3';
                    tmp[index++] = '7';
                    break;
                case '&':
                    tmp[index++] = '%';
                    tmp[index++] = '0';
                    tmp[index++] = '3';
                    tmp[index++] = '8';
                    break;
                case '\'':
                    tmp[index++] = '%';
                    tmp[index++] = '0';
                    tmp[index++] = '3';
                    tmp[index++] = '9';
                    break;
                case ',':
                    tmp[index++] = '%';
                    tmp[index++] = '0';
                    tmp[index++] = '4';
                    tmp[index++] = '4';
                    break;
                case '.':
                    tmp[index++] = '%';
                    tmp[index++] = '0';
                    tmp[index++] = '4';
                    tmp[index++] = '6';
                    break;
                case '/':
                    tmp[index++] = '%';
                    tmp[index++] = '0';
                    tmp[index++] = '4';
                    tmp[index++] = '7';
                    break;
                case ':':
                    tmp[index++] = '%';
                    tmp[index++] = '0';
                    tmp[index++] = '5';
                    tmp[index++] = '8';
                    break;
                case ';':
                    tmp[index++] = '%';
                    tmp[index++] = '0';
                    tmp[index++] = '5';
                    tmp[index++] = '9';
                    break;
                case '<':
                    tmp[index++] = '%';
                    tmp[index++] = '0';
                    tmp[index++] = '6';
                    tmp[index++] = '0';
                    break;
                case '=':
                    tmp[index++] = '%';
                    tmp[index++] = '0';
                    tmp[index++] = '6';
                    tmp[index++] = '1';
                    break;
                case '>':
                    tmp[index++] = '%';
                    tmp[index++] = '0';
                    tmp[index++] = '6';
                    tmp[index++] = '2';
                    break;
                case '@':
                    tmp[index++] = '%';
                    tmp[index++] = '0';
                    tmp[index++] = '6';
                    tmp[index++] = '4';
                    break;
                case '[':
                    tmp[index++] = '%';
                    tmp[index++] = '0';
                    tmp[index++] = '9';
                    tmp[index++] = '1';
                    break;
                case '\\':
                    tmp[index++] = '%';
                    tmp[index++] = '0';
                    tmp[index++] = '9';
                    tmp[index++] = '2';
                    break;
                case ']':
                    tmp[index++] = '%';
                    tmp[index++] = '0';
                    tmp[index++] = '9';
                    tmp[index++] = '3';
                    break;
                case '^':
                    tmp[index++] = '%';
                    tmp[index++] = '0';
                    tmp[index++] = '9';
                    tmp[index++] = '4';
                    break;
                case '`':
                    tmp[index++] = '%';
                    tmp[index++] = '0';
                    tmp[index++] = '9';
                    tmp[index++] = '6';
                    break;
                case '{':
                    tmp[index++] = '%';
                    tmp[index++] = '1';
                    tmp[index++] = '2';
                    tmp[index++] = '3';
                    break;
                case '|':
                    tmp[index++] = '%';
                    tmp[index++] = '1';
                    tmp[index++] = '2';
                    tmp[index++] = '4';
                    break;
                case '}':
                    tmp[index++] = '%';
                    tmp[index++] = '1';
                    tmp[index++] = '2';
                    tmp[index++] = '5';
                    break;
                case '~':
                    tmp[index++] = '%';
                    tmp[index++] = '1';
                    tmp[index++] = '2';
                    tmp[index++] = '6';
                    break;
                default:
                    tmp[index++] = c;
                    break;
            }
        }
        tmp[index++] = '\0';
        uri = (c_char*)(os_malloc(strlen(tmp) + 1));
        os_sprintf(uri, "%s", tmp);
    }
    return uri;
}

static c_char*
URIToString(
    const c_char* uri)
{
    int i, length, index, code;
    c_char c;
    c_char *tmp;
    c_char *str = NULL;
    c_char codeString[4];

    if(uri){
        length = (int)strlen(uri);
        index = 0;

        tmp = os_malloc(length+1);

        for(i=0; i<length; i++){
            c = uri[i];

            switch(c){
                case '%':
                    i++;
                    codeString[0] = uri[i++];
                    codeString[1] = uri[i++];
                    codeString[2] = uri[i];
                    codeString[3] = '\0';

                    code = atoi(codeString);

                    switch(code){
                        case 20:
                            tmp[index++] = ' ';
                            break;
                        case 33:
                            tmp[index++] = '!';
                            break;
                        case 34:
                            tmp[index++] = '"';
                            break;
                        case 35:
                            tmp[index++] = '#';
                            break;
                        case 36:
                            tmp[index++] = '$';
                            break;
                        case 37:
                            tmp[index++] = '%';
                            break;
                        case 38:
                            tmp[index++] = '&';
                            break;
                        case 39:
                            tmp[index++] = '\'';
                            break;
                        case 44:
                            tmp[index++] = ',';
                            break;
                        case 46:
                            tmp[index++] = '.';
                            break;
                        case 47:
                            tmp[index++] = '/';
                            break;
                        case 58:
                            tmp[index++] = ':';
                            break;
                        case 59:
                            tmp[index++] = ';';
                            break;
                        case 60:
                            tmp[index++] = '<';
                            break;
                        case 61:
                            tmp[index++] = '=';
                            break;
                        case 62:
                            tmp[index++] = '>';
                            break;
                        case 64:
                            tmp[index++] = '@';
                            break;
                        case 91:
                            tmp[index++] = '[';
                            break;
                        case 92:
                            tmp[index++] = '\\';
                            break;
                        case 93:
                            tmp[index++] = ']';
                            break;
                        case 94:
                            tmp[index++] = '^';
                            break;
                        case 96:
                            tmp[index++] = '`';
                            break;
                        case 123:
                            tmp[index++] = '{';
                            break;
                        case 124:
                            tmp[index++] = '|';
                            break;
                        case 125:
                            tmp[index++] = '}';
                            break;
                        case 126:
                            tmp[index++] = '~';
                            break;
                        default:
                            OS_REPORT(OS_ERROR, "durability", 0, "Found unknown ASCII code.");
                            tmp[index++] = c;
                            break;
                    }
                    break;
                default:
                    tmp[index++] = c;
                    break;
            }
        }
        tmp[index++] = '\0';
        str = (c_char*)(os_malloc(strlen(tmp) + 1));
        os_sprintf(str, "%s", tmp);
        os_free(tmp);
    }
    return str;
}

static void
readLine(
    FILE* fdes,
    int len,
    c_char* data)
{
    int slen;

    assert(data);
    assert(fdes);

    data[0] = '\0';
    fgets(data, len, fdes);
    slen = strlen(data);

    if(slen > 0){
        data[slen-1] = '\0';
    }
    return;
}


static void
readObject(
    FILE* fdes,
    int len,
    c_char* data)
{
    int slen;
    c_char* buf;
    c_char* readData;

    assert(data);
    assert(fdes);

    data[0] = '\0';
    fgets(data, len, fdes);
    slen = strlen(data);

    if(slen > 0){
        buf = data;
        readData = "";

        while((strncmp(&(buf[slen-10]), "</object>\n", 10) != 0) && (readData != NULL)) {
            buf += slen;
            readData = fgets(buf, len, fdes);
            slen = strlen(buf);

            while((slen < 10) && (readData != NULL)){
                buf += slen;
                readData = fgets(buf, len, fdes);
                slen = strlen(buf);
            }
        }
        buf[slen-1] = '\0';
    }
    return;
}

static c_char*
getSubString(
    const c_char* str,
    int startIndex,
    int endIndex)
{
    c_char* result;
    c_char *tmp;

    result = NULL;

    if(str){
        if((startIndex >= 0) && (endIndex >= startIndex)){
            if(((int)(strlen(str))) >= endIndex){
                result = os_malloc(endIndex - startIndex + 1);
                tmp = (c_char*)(str + startIndex);

               os_strncpy(result, tmp, (endIndex-startIndex));
                result[(endIndex-startIndex)] = '\0';
            }
        }
    }
    return result;
}

static c_char*
getDirectoryNameForStoreDir(
    const c_char* storeDir,
    const c_char* partition)
{
    c_char* fileName;
    c_char* partitionURI;
    const c_char* filesep;

    assert(storeDir);
    assert(partition);

    fileName = NULL;
    if(strlen(partition) == 0)
    {
        fileName = (c_char*)(os_malloc(strlen(storeDir) + 1));
        os_sprintf(fileName, "%s", storeDir);
    } else
    {
        filesep = os_fileSep();
        partitionURI = stringToURI(partition);
        fileName = (c_char*)(os_malloc(strlen(storeDir) + 1 + strlen(partitionURI) + 1));
        os_sprintf(fileName, "%s%s%s", storeDir, filesep, partitionURI);
        os_free(partitionURI);
    }
    return fileName;
}

static c_bool
createDirectoryIfNecessaryForStoreDir(
    const c_char* storeDir,
    const c_char* partition)
{
    c_bool result;
    c_char *dirName;
    os_result status;
    struct os_stat statBuf;

    if(storeDir)
    {
        dirName = getDirectoryNameForStoreDir(storeDir, partition);
    } else
    {
        dirName = NULL;
    }
    status = os_stat(dirName, &statBuf);

    if (status != os_resultSuccess) {
        status = os_mkdir(dirName, S_IRWXU | S_IRWXG | S_IRWXO);
    }
    os_free(dirName);

    if(status == os_resultSuccess){
        result = TRUE;
    } else {
        result = FALSE;
    }
    return result;
}

static c_bool
createDirectoryIfNecessary(
    const d_storeXML store,
    const c_char* partition)
{
    c_bool result;
    c_char *storeDir;

    if(store)
    {
        storeDir = d_store(store)->config->persistentStoreDirectory;
    } else
    {
        storeDir = NULL;
    }
    result = createDirectoryIfNecessaryForStoreDir(storeDir, partition);
    return result;
}

static c_char*
getDataFileName(
    const d_storeXML store,
    const c_char* partition,
    const c_char* topic)
{
    c_char* fileName = NULL;
    c_char* storeDir;

    assert(partition);
    assert(topic);
    assert(store);

    if(store)
    {
        storeDir = d_store(store)->config->persistentStoreDirectory;
        fileName = getDataFileNameBasedOnPath(storeDir, partition, topic);
    }

    return fileName;
}

c_char*
getDataFileNameBasedOnPath(
    const c_char* storeDir,
    const c_char* partition,
    const c_char* topic)
{
    c_char* fileName = NULL;
    c_char* partitionURI;
    const c_char* filesep;

    assert(partition);
    assert(topic);
    assert(storeDir);

    filesep = os_fileSep();
    if(strlen(partition) == 0)
    {
        fileName = (c_char*)(os_malloc(strlen(storeDir) + 1 + strlen(topic) + 5));
        if(fileName)
        {
            os_sprintf(fileName, "%s%s%s.xml", storeDir, filesep, topic);
        }
    } else
    {
        partitionURI = stringToURI(partition);
        if(partitionURI)
        {
            fileName = (c_char*)(os_malloc(strlen(storeDir) + 1 + strlen(partitionURI) + 1 + strlen(topic) + 5));
            if(fileName)
            {
                os_sprintf(fileName, "%s%s%s%s%s.xml", storeDir, filesep, partitionURI, filesep, topic);
            }
            os_free(partitionURI);
        }
    }
    return fileName;
}

static c_char*
getOptimizeFileName(
    const d_storeXML store,
    const c_char* partition,
    const c_char* topic)
{
    c_char* fileName;
    c_char* storeDir;
    c_char* partitionURI;
    const c_char* filesep;

    assert(partition);
    assert(topic);
    assert(store);

    fileName = NULL;

    if(store && topic && partition){
        filesep = os_fileSep();
        storeDir = d_store(store)->config->persistentStoreDirectory;

        if(strlen(partition) == 0){
            fileName = (c_char*)(os_malloc(strlen(storeDir) + 1 + strlen(topic) + 14));

            if(fileName){
                os_sprintf(fileName, "%s%s%s_optimize.txt", storeDir, filesep, topic);
            }
        } else {
            partitionURI = stringToURI(partition);

            if(partitionURI){
                fileName = (c_char*)(os_malloc(strlen(storeDir) + 1 + strlen(partitionURI) + 1 + strlen(topic) + 14));

                if(fileName){
                    os_sprintf(fileName, "%s%s%s%s%s_optimize.txt", storeDir, filesep, partitionURI, filesep, topic);
                }
                os_free(partitionURI);
            }
        }
    }
    return fileName;
}

static c_char*
getTmpFileName(
    const d_storeXML store,
    const c_char* partition,
    const c_char* topic)
{
    c_char* fileName;
    c_char* storeDir;
    c_char* partitionURI;
    const c_char* filesep;

    assert(partition);
    assert(topic);
    assert(store);

    fileName = NULL;

    if(store){
        filesep = os_fileSep();
        storeDir = d_store(store)->config->persistentStoreDirectory;

        if(strlen(partition) == 0){
            fileName = (c_char*)(os_malloc(strlen(storeDir) + 1 + strlen(topic) + 5));
            os_sprintf(fileName, "%s%s%s.tmp", storeDir, filesep, topic);
        } else {
            partitionURI = stringToURI(partition);
            fileName = (c_char*)(os_malloc(strlen(storeDir) + 1 + strlen(partitionURI) + 1 + strlen(topic) + 5));
            os_sprintf(fileName, "%s%s%s%s%s.tmp", storeDir, filesep, partitionURI, filesep, topic);
            os_free(partitionURI);
        }

    }
    return fileName;
}

static c_char*
getBakFileName(
    const d_storeXML store,
    const c_char* partition,
    const c_char* topic)
{
    c_char* fileName = NULL;
    c_char* storeDir;

    assert(partition);
    assert(topic);
    assert(store);

    if(store)
    {
        storeDir = d_store(store)->config->persistentStoreDirectory;
        fileName = getBakFileNameBasedOnPath(storeDir, partition, topic);
    }

    return fileName;
}

c_char*
getBakFileNameBasedOnPath(
    const c_char* storeDir,
    const c_char* partition,
    const c_char* topic)
{
    c_char* fileName;
    c_char* partitionURI;
    const c_char* filesep;

    assert(partition);
    assert(topic);
    assert(storeDir);

    fileName = NULL;

    if(storeDir)
    {
        filesep = os_fileSep();

        if(strlen(partition) == 0){
            fileName = (c_char*)(os_malloc(strlen(storeDir) + 1 + strlen(topic) + 5));
            os_sprintf(fileName, "%s%s%s.bak", storeDir, filesep, topic);
        } else {
            partitionURI = stringToURI(partition);
            fileName = (c_char*)(os_malloc(strlen(storeDir) + 1 + strlen(partitionURI) + 1 + strlen(topic) + 5));
            os_sprintf(fileName, "%s%s%s%s%s.bak", storeDir, filesep, partitionURI, filesep, topic);
            os_free(partitionURI);
        }

    }
    return fileName;
}

static c_char*
getMetaFileName(
    const d_storeXML store,
    const c_char* partition,
    const c_char* topic)
{
    c_char* fileName = NULL;
    c_char* storeDir;

    assert(partition);
    assert(topic);
    assert(store);

    if(store)
    {
        storeDir = d_store(store)->config->persistentStoreDirectory;
        fileName = getMetaFileNameBasedOnPath(storeDir, partition, topic);
    }

    return fileName;
}

c_char*
getMetaFileNameBasedOnPath(
    const c_char* storeDir,
    const c_char* partition,
    const c_char* topic)
{
    c_char* fileName;
    c_char* partitionURI;
    const c_char* filesep;

    assert(partition);
    assert(topic);
    assert(storeDir);

    fileName = NULL;

    if(storeDir){
        filesep = os_fileSep();

        if(strlen(partition) == 0){
            fileName = (c_char*)(os_malloc(strlen(storeDir) + 1 + strlen(topic) + 10));
            os_sprintf(fileName, "%s%s%s_meta.xml", storeDir, filesep, topic);
        } else {
            partitionURI = stringToURI(partition);
            fileName = (c_char*)(os_malloc(strlen(storeDir) + 1 + strlen(partitionURI) + 1 + strlen(topic) + 10));
            os_sprintf(fileName, "%s%s%s%s%s_meta.xml", storeDir, filesep, partitionURI, filesep, topic);
            os_free(partitionURI);
        }

    }
    return fileName;
}


static c_char *
getMessageMetadata(
    const c_type type )
{
    c_char *          str;
    c_type            metaType;
    sd_serializer     serializer;
    sd_serializedData serData;
    c_property userDataAttr;

    metaType = c_getType(type);
    userDataAttr = c_property(c_metaResolve(c_metaObject(type), "userData"));
    serializer = sd_serializerXMLTypeinfoNew(c_getBase(metaType), FALSE);
    serData = sd_serializerSerialize(serializer, userDataAttr->type);
    str = sd_serializerToString(serializer, serData);
    sd_serializedDataFree(serData);
    sd_serializerFree(serializer);
    c_free(userDataAttr);
    return(str);
}

static c_char *
getMessageMetadataDeprecated(
    const c_type type )
{
    c_char *          str;
    c_type            metaType;
    sd_serializer     serializer;
    sd_serializedData serData;
    c_property userDataAttr;

    metaType = c_getType(type);
    userDataAttr = c_property(c_metaResolve(c_metaObject(type), "userData"));
    serializer = sd_serializerXMLMetadataNew(c_getBase(metaType));
    serData = sd_serializerSerialize(serializer, userDataAttr->type);
    str = sd_serializerToString(serializer, serData);
    sd_serializedDataFree(serData);
    sd_serializerFree(serializer);
    c_free(userDataAttr);
    return(str);
}

static d_topicMetadata
readTopicMetadata(
    d_storeXML persistentStore,
    u_participant participant,
    const c_char* partitionName,
    const c_char* topicName)
{
    d_topicMetadata metadata;
    struct os_stat   statBuf;
    FILE *        fdes;
    c_char *      data;
    c_char *      fileStorePath;
    c_char *      tmp;
    os_result     ret;
    c_ulong       size;
    c_bool useDeprecatedSerializer;
    sd_serializer serializer;
    sd_serializedData serData;
    struct baseFind f;
    c_type qosType;
    sd_validationResult valResult;

    metadata      = NULL;
    fileStorePath = getMetaFileName(persistentStore, partitionName, topicName);
    ret           = os_stat(fileStorePath, &statBuf);

    if (ret == os_resultSuccess) {
        char * filename = os_fileNormalize(fileStorePath);
        fdes = fopen(filename, "r");
        os_free(filename);

        if (fdes != NULL) {
            size = EXTRA_BACKSLS;
            size = (c_ulong)statBuf.stat_size + size;
            data = (c_char *)os_malloc(size);

            if (data) {
                readLine(fdes, size, data);

                if(strncmp(data, "<METADATA>", 10) == 0){
                    readLine(fdes, size, data);

                    if(strncmp(data, "<name>", 6) == 0){
                        metadata = d_topicMetadata(os_malloc(C_SIZEOF(d_topicMetadata)));
                        metadata->name = getSubString(data, 6, strlen(data)-7);

                        readLine(fdes, size, data);

                        if(strncmp(data, "<keyList>", 9) == 0){
                            metadata->keyList = getSubString(data, 9, strlen(data)-10);

                            readLine(fdes, size, data);

                            if(strncmp(data, "<qos>", 5) == 0){
                                tmp = getSubString(data, 5, strlen(data)-6);

                                u_entityAction(u_entity(participant), d_storeGetBase, &f);
                                qosType = c_resolve(f.base, "kernelModule::v_topicQos");

                                serializer = sd_serializerXMLNewTyped(qosType);
                                serData = sd_serializerFromString(serializer, tmp);
                                metadata->qos = (v_topicQos)(sd_serializerDeserializeValidated(serializer, serData));
                                valResult = sd_serializerLastValidationResult(serializer);
                                os_free(tmp);

                                if(valResult == SD_VAL_SUCCESS){
                                    sd_serializedDataFree(serData);
                                    sd_serializerFree(serializer);
                                    c_free(qosType);

                                    readLine(fdes, size, data);

                                    if(strncmp(data, "<type>", 6) == 0){
                                        tmp = getSubString(data, 6, (strlen(data) - 7));

                                        useDeprecatedSerializer = isDeprecatedMetadataFormat(tmp);

                                        /* Instantiate deprecated serializer only if meta-data format is old. */
                                        if(useDeprecatedSerializer){
                                            serializer = sd_serializerXMLMetadataNew(f.base);
                                        } else{
                                            serializer = sd_serializerXMLTypeinfoNew(f.base, FALSE);
                                        }
                                        serData = sd_serializerFromString(serializer, tmp);
                                        metadata->type = (c_type)(sd_serializerDeserializeValidated(serializer, serData));
                                        os_free(tmp);

                                        if(!metadata->type){
                                            valResult = sd_serializerLastValidationResult(serializer);

                                            if(valResult != SD_VAL_SUCCESS){
                                                d_storeReport(d_store(persistentStore),
                                                    D_LEVEL_SEVERE,
                                                    "Reconstruction of Topic failed. Topic type on disk is not valid.\nReason: %s\nError: %s\n",
                                                    sd_serializerLastValidationMessage(serializer),
                                                    sd_serializerLastValidationLocation(serializer));
                                                OS_REPORT_2(OS_ERROR, D_CONTEXT, 0,
                                                        "Reconstruction of Topic failed. Topic type on disk is not valid.\nReason: %s\nError: %s\n",
                                                        sd_serializerLastValidationMessage(serializer),
                                                        sd_serializerLastValidationLocation(serializer));
                                                os_free(metadata->name);
                                                os_free(metadata->keyList);
                                                v_topicQosFree(metadata->qos);
                                                os_free(metadata);
                                                metadata = NULL;
                                            }
                                        } else if(useDeprecatedSerializer){
                                            metadata->typeName = c_metaScopedName(c_metaObject(metadata->type));
                                        } else {
                                            readLine(fdes, size, data);

                                            if(strncmp(data, "<typeName>", 10) == 0){
                                                metadata->typeName = getSubString(data, 10, strlen(data)-11);
                                            } else {
                                                os_free(metadata->name);
                                                os_free(metadata->keyList);
                                                v_topicQosFree(metadata->qos);
                                                c_free(metadata->type);
                                                os_free(metadata);
                                                metadata = NULL;
                                            }
                                        }
                                        sd_serializedDataFree(serData);
                                        sd_serializerFree(serializer);
                                    } else {
                                        os_free(metadata->name);
                                        os_free(metadata->keyList);
                                        v_topicQosFree(metadata->qos);
                                        os_free(metadata);
                                        metadata = NULL;
                                    }
                                } else {
                                    d_storeReport(d_store(persistentStore),
                                                D_LEVEL_SEVERE,
                                                "Reconstruction of TopicQos failed.\nReason: %s\nError: %s\n",
                                                sd_serializerLastValidationMessage(serializer),
                                                sd_serializerLastValidationLocation(serializer));
                                    sd_serializedDataFree(serData);
                                    sd_serializerFree(serializer);
                                    c_free(qosType);

                                    os_free(metadata->name);
                                    os_free(metadata->keyList);
                                    os_free(metadata);
                                    metadata = NULL;
                                }
                            } else {
                                os_free(metadata->name);
                                os_free(metadata->keyList);
                                os_free(metadata);
                                metadata = NULL;
                            }
                        } else {
                            os_free(metadata->name);
                            os_free(metadata);
                            metadata = NULL;
                        }
                    } else {
                        os_free(metadata);
                        metadata = NULL;
                    }
                }
                os_free(data);
            }
            fclose(fdes);
        }
    }
    d_free(fileStorePath);

    return metadata;
}

static c_bool
storeTopicMetadata(
    d_storeXML persistentStore,
    v_group group)
{
    FILE   *    fdes = NULL;
    c_char *    topicName = NULL;
    c_char *    typeName = NULL;
    c_char *    keyExpr = NULL;
    c_char *    partitionName = NULL;
    c_char *    fileStorePath;
    c_char *    qos;
    c_char*     str;
    c_bool      result;
    sd_serializer serializer;
    sd_serializedData data;
    v_topic topic;

    topic = v_groupTopic(group);
    topicName = v_topicName(topic);
    partitionName = v_partitionName(v_groupPartition(group));
    keyExpr = v_topicKeyExpr(topic);
    typeName = c_metaScopedName(c_metaObject(v_topicGetUserType(topic)));

    if(!keyExpr){
        keyExpr = "";
    }
    if(topicName && partitionName && typeName){
        char * filename;

        serializer = sd_serializerXMLNewTyped(c_getType(v_topicQosRef(topic)));
        data = sd_serializerSerialize(serializer, v_topicQosRef(topic));
        qos = sd_serializerToString(serializer, data);

        sd_serializedDataFree(data);
        sd_serializerFree(serializer);

        fileStorePath = getMetaFileName(persistentStore,
                                        partitionName,
                                        topicName);


        filename = os_fileNormalize(fileStorePath);
        if(filename){
            fdes = fopen(filename, "w+");
            os_free(filename);
        }
        if (fdes == NULL) {
            d_storeReport(d_store(persistentStore),
                          D_LEVEL_SEVERE,
                          RR_COULD_NOT_WRITE_META_DATA,
                          topicName);
            OS_REPORT_1(OS_ERROR,
                        STORE_STORE_TOPIC_XML, 0,
                        RR_COULD_NOT_WRITE_META_DATA,
                        topicName);
            result = FALSE;
        } else {
            str = getMessageMetadata(v_topicMessageType(topic));

            if(str){
                fprintf(fdes, "<METADATA>\n");
                fprintf(fdes, "<name>%s</name>\n", topicName);
                fprintf(fdes, "<keyList>%s</keyList>\n", keyExpr);
                fprintf(fdes, "<qos>%s</qos>\n", qos);
                fprintf(fdes, "<type>%s</type>\n", str);
                fprintf(fdes, "<typeName>%s</typeName>\n", typeName);
                fprintf(fdes, "</METADATA>\n");
                os_fsync(fdes);
                os_free(str);
                result = TRUE;
            } else {
                result = FALSE;
            }
            fclose(fdes);
        }
        if(qos){
            os_free(qos);
        }
        if(fileStorePath){
            os_free(fileStorePath);
        }
    } else {
        result = FALSE;
    }
    if(typeName){
        os_free(typeName);
    }
    return result;
}

/* TODO: Update to support new format*/
static c_bool
metaDataIsCorrect(
    const c_type            type,
    const d_storeXML        persistentStore,
    c_char *                topicName,
    c_char *                partitionName,
    c_char *                keyList,
    v_topicQos              qos )
{
    struct os_stat      statBuf;
    FILE *              fdes;
    c_char *            data;
    c_char *            fileStorePath;
    os_result           ret;
    c_ulong             size;
    c_char *            str;
    c_char *            tmpKeys;
    c_char *            tmpQos;
    c_char*             tmpType;
    c_bool              result;
    c_bool              noError;
    c_bool              isDeprecated;
    c_type              qosType;
    sd_serializer       ser;
    sd_serializedData   serData;
    c_char*             xmlQos;

    result        = FALSE;
    fileStorePath = getMetaFileName(persistentStore, partitionName, topicName);
    ret           = os_stat(fileStorePath, &statBuf);

    if (ret == 0) {
        char * filename = os_fileNormalize(fileStorePath);
        fdes = fopen(filename, "r");
        os_free(filename);

        if (fdes != NULL) {
            size = EXTRA_BACKSLS;
            size = (c_ulong)statBuf.stat_size + size;
            data = (c_char *)os_malloc(size);

            if (data) {
                readLine(fdes, size, data);

                if(strncmp(data, "<METADATA>", 10) == 0){
                    readLine(fdes, size, data);

                    if(strncmp(data, "<name>", 6) == 0){
                        readLine(fdes, size, data);

                            if(strncmp(data, "<keyList>", 9) == 0){
                                tmpKeys = getSubString(data, 9, (strlen(data) - 10));

                                if(strcmp(tmpKeys, keyList) == 0){
                                    readLine(fdes, size, data);

                                    if(strncmp(data, "<qos>", 5) == 0){
                                        tmpQos = getSubString(data, 5, (strlen(data) - 6));
                                        qosType = c_getType(qos);
                                        ser = sd_serializerXMLNewTyped(qosType);
                                        serData = sd_serializerSerialize(ser, qos);
                                        xmlQos = sd_serializerToString(ser, serData);
                                        sd_serializedDataFree(serData);
                                        sd_serializerFree(ser);
                                        /* c_free(qosType); c_getType() does not keep type; */

                                        if(strcmp(xmlQos, tmpQos) == 0){
                                            readLine(fdes, size, data); /* <type> */

                                            if(strncmp(data, "<type>", 6) == 0){
                                                tmpType = getSubString(data, 6, (strlen(data) - 7));
                                                isDeprecated = isDeprecatedMetadataFormat(tmpType);

                                                if(isDeprecated){
                                                    str = getMessageMetadataDeprecated(type);
                                                } else {
                                                    str = getMessageMetadata(type);
                                                }
                                                if(str) {
                                                    size = strlen(str);

                                                    if(strncmp(str, tmpType, size) == 0) {
                                                        noError = TRUE;
                                                        /* The new format also has a line
                                                         * to store the typeName
                                                         */
                                                        if(isDeprecated == FALSE){
                                                            readLine(fdes, size, data); /* <typeName> */

                                                            if(strncmp(data, "<typeName>", 10) != 0){
                                                                noError = FALSE;
                                                                d_storeReport(d_store(persistentStore),
                                                                    D_LEVEL_SEVERE,
                                                                    "type-name of topic with name '%s' is missing\n",
                                                                    topicName);
                                                            }
                                                        }
                                                        if(noError){
                                                            readLine(fdes, size, data); /* </METADATA> */

                                                            if(strncmp(data, "</METADATA>", 11) == 0){
                                                                result = TRUE;
                                                                d_storeReport(d_store(persistentStore),
                                                                    D_LEVEL_FINE,
                                                                    "metaDataIsCorrect for '%s' OK\n", topicName);
                                                            }  else {
                                                                d_storeReport(d_store(persistentStore),
                                                                    D_LEVEL_SEVERE,
                                                                    "</METADATA> missing for topic '%s'\n", topicName);
                                                            }
                                                        }
                                                    } else {
                                                        d_storeReport(d_store(persistentStore),
                                                            D_LEVEL_SEVERE,
                                                            "types of topics with name '%s' do not match\n", topicName);
                                                    }
                                                    os_free(str);

                                                } else {
                                                    d_storeReport(d_store(persistentStore),
                                                        D_LEVEL_SEVERE,
                                                        "type missing for topic '%s'\n", topicName);
                                                }
                                                os_free(tmpType);
                                            } else {
                                                d_storeReport(d_store(persistentStore),
                                                    D_LEVEL_SEVERE,
                                                    "<type> missing for topic '%s'\n", topicName);
                                            }
                                        } else {
                                            d_storeReport(d_store(persistentStore),
                                                D_LEVEL_SEVERE,
                                                "qos-ses of topics with name '%s' do not match.\n", topicName);
                                        }
                                        os_free(xmlQos);
                                        os_free(tmpQos);
                                    } else {
                                        d_storeReport(d_store(persistentStore),
                                            D_LEVEL_SEVERE,
                                            "<qos> missing for topic '%s'\n", topicName);
                                    }
                                } else {
                                    d_storeReport(d_store(persistentStore),
                                        D_LEVEL_SEVERE,
                                        "keylists of topics with name '%s' not equal ('%s' versus '%s')\n",
                                        topicName, keyList, tmpKeys);
                                }
                                os_free(tmpKeys);
                            } else {
                                d_storeReport(d_store(persistentStore),
                                    D_LEVEL_SEVERE,
                                    "<keyList> missing for topic '%s'\n", topicName);
                            }
                        }  else {
                            d_storeReport(d_store(persistentStore),
                                D_LEVEL_SEVERE,
                                "<name> missing for topic '%s'\n", topicName);
                        }
                } else {
                d_storeReport(d_store(persistentStore),
                    D_LEVEL_SEVERE,
                    "<METADATA> missing for topic '%s'\n", topicName);
                }
                os_free(data);
            } else {
                d_storeReport(d_store(persistentStore),
                    D_LEVEL_SEVERE,
                    "Could not allocate.\n");
            }
            fclose(fdes);
        } else {
            d_storeReport(d_store(persistentStore),
                            D_LEVEL_SEVERE,
                            "Metadata not accessible for topic '%s'\n", topicName);
        }
    } else {
        d_storeReport(d_store(persistentStore),
                            D_LEVEL_SEVERE,
                            "Metadata missing on disk for topic '%s'\n", topicName);
    }
    d_free(fileStorePath);

    return result;
}

static void
forAllDirectoryEntries (
    d_storeXML persistentStore,
    const char *      workingDir,
    const char *      parent,
                      /* QAC EXPECT 3672; non-const pointer to function */
    void (*           processEntry )(d_storeXML persistentStore,
                                     const char * path,
                                     const char * parent,
                                     const char * entryName,
                                     struct os_stat * status),
    c_bool usePWD)
{
    char             path[OS_PATH_MAX + 1];
    os_dirHandle     d_descr;
    struct os_dirent d_entry;
    struct os_stat   status;
    os_result        r;

    r = os_opendir(workingDir, &d_descr);
    if (r == os_resultSuccess) { /* accessable */
        r = os_readdir(d_descr, &d_entry);
        while (r == os_resultSuccess) {
            /* QAC EXPECT 5007; use of strcpy*/
            os_strcpy(path, workingDir);
            /* QAC EXPECT 5007; use of os_strcat */
            os_strcat(path, os_fileSep());
            /* QAC EXPECT 5007; use of os_strcat */
            os_strcat(path, d_entry.d_name);

            if (os_stat (path, &status) == os_resultSuccess) { /* accessable */
                /* QAC EXPECT 5007; use of strcmp */
                if ((strcmp(d_entry.d_name, ".") != 0) &&
                    /* QAC EXPECT 5007; use of strcmp */
                    (strcmp(d_entry.d_name, "..") != 0)) {
                    processEntry(persistentStore, path, parent, d_entry.d_name, &status);
                }
            }
            r = os_readdir(d_descr, &d_entry);
        }
        if(usePWD == TRUE){
            if(os_stat(workingDir, &status) == os_resultSuccess){
                processEntry(persistentStore, workingDir, parent, "", &status);
            }
        }
        os_closedir (d_descr);
    }
}

static c_bool
topicIsProperlyTagged(
    FILE *        fDes,
    struct os_stat * status )
{
    /* QAC EXPECT 1259; */
    c_char testText[STRLEN_NEEDED_FOR_A_BIG_TAG];
    c_bool isProperlyTagged;
    int    result;

    /* check the file contents */
    testText[0] = '\0';
    isProperlyTagged = FALSE;
    /* QAC EXPECT 0702,1259; */
    (void) fread ((c_voidp) &testText[0], sizeof (c_char), STRLEN_TOPIC_TAG_OPEN, fDes);
    /* QAC EXPECT 1259; */
    result = strncmp(testText, TOPIC_TAG_OPEN, STRLEN_TOPIC_TAG_OPEN);
    if (result == 0) {
        /* QAC EXPECT 3892,3784,1259; */
        fseek(fDes, status->stat_size - STRLEN_TOPIC_TAG_CLOSE - STRLEN_BACKSLASH_N, SEEK_SET);
        fscanf(fDes, "%s", testText);
        /* QAC EXPECT 1259; */
        result = strncmp(testText, TOPIC_TAG_CLOSE, STRLEN_TOPIC_TAG_CLOSE);
        if  (result == 0) {
            isProperlyTagged = TRUE;
        }
    }
    return isProperlyTagged;
}

static c_bool
fileNameIsOK(
    const char * topic,         /* input  : name with extension */
    c_char *     topicName )    /* output : name without extension */
{
    c_bool       nameIsOK;
    c_bool       result;
    os_uint32      length;
    const char * tail;

    /* Name must end with ".xml"  */
    nameIsOK = FALSE;
    length = strlen(topic);
    /* QAC EXPECT 1259; */
    if (length > STRLEN_DOT_XML) {
        /* QAC EXPECT 0488,1259; */
        tail = topic + length - STRLEN_DOT_XML;
        /* QAC EXPECT 3757,1259; */
        result = strncmp(tail, DOT_XML, STRLEN_DOT_XML);
        /* QAC EXPECT 2100; */
        if (result == 0) {
            nameIsOK = TRUE;
           os_strncpy(topicName, topic, length);
            /* QAC EXPECT 1259; */
            topicName[length - STRLEN_DOT_XML] = '\0';
        }
    }
    return nameIsOK;
}

static d_groupList
getPartitionTopic(
    d_storeXML persistentStore,
    const char *      partition,
    const char *      topic )
{
    d_groupList persistentGroup;
    d_groupList foundGroup;
    int               result;

    foundGroup = NULL;
    persistentGroup = persistentStore->groups;

    while((persistentGroup) && (foundGroup == NULL)) {
        /* QAC EXPECT 5007; */
        result = strcmp(persistentGroup->topic, topic);
        if (result == 0) {
            /* QAC EXPECT 5007; */
            result = strcmp(persistentGroup->partition, partition);
            if (result == 0) {
                foundGroup = persistentGroup;
            }
        }
        persistentGroup = persistentGroup->next;
    }
    return foundGroup;
}

static void
setPartitionTopicQuality(
    d_storeXML persistentStore,
    const char *      partition,
    const char *      topic,
    d_quality         quality,
    d_completeness    completeness )
{
    d_groupList persistentGroup;

    persistentGroup = getPartitionTopic(persistentStore, partition, topic);

    if (persistentGroup) {
        if( (quality.seconds > persistentGroup->quality.seconds) ||
            ((quality.seconds == persistentGroup->quality.seconds) &&
             (quality.nanoseconds > persistentGroup->quality.nanoseconds)))
        {
            persistentGroup->quality.seconds = quality.seconds;
            persistentGroup->quality.nanoseconds = quality.nanoseconds;

            d_storeReport(d_store(persistentStore), D_LEVEL_FINE, "Group '%s.%s' has quality: %d\n",
                partition, topic, quality.seconds);
        }
    } else {
        persistentGroup = d_groupList(os_malloc(C_SIZEOF(d_groupList)));

        if (persistentGroup) {
            persistentGroup->partition           = os_strdup(partition);
            persistentGroup->topic               = os_strdup(topic);
            persistentGroup->quality.seconds     = quality.seconds;
            persistentGroup->quality.nanoseconds = quality.nanoseconds;
            persistentGroup->completeness        = completeness;
            persistentGroup->next                = persistentStore->groups;
            persistentGroup->optimized           = FALSE;
            persistentStore->groups              = persistentGroup;
        }
    }
}

static void
processFile(
    d_storeXML persistentStore,
    FILE *            fDes,
    const char *      partition,     /* = <partition>                  */
    const char *      topic,         /* = <topic>_meta.xml or <topic>.xml */
    struct os_stat *  status )
{
    c_char    topicName[OS_PATH_MAX + 1];
    c_char*   topicDataFileName;
    c_char*   strPartition;
    struct os_stat s;
    d_quality quality;

    topicName[0] = '\0';
    /* QAC EXPECT 2100; */

    if ((fileNameIsOK(topic, topicName) == TRUE)) {


        if ((topicIsProperlyTagged(fDes, status) == TRUE)) {
            strPartition = URIToString(partition);
            topicDataFileName = getDataFileName(persistentStore, strPartition, topicName);

            if(os_stat(topicDataFileName, &s) == os_resultSuccess){
                quality.seconds     = s.stat_mtime.tv_sec; /* last data modification (time_t) */
                quality.nanoseconds = s.stat_mtime.tv_nsec;

                d_storeReport(d_store(persistentStore), D_LEVEL_FINER,
                            "Found group '%s.%s' with quality %d.\n",
                            strPartition, topicName, quality.seconds);
            } else {
                quality.seconds     = 0;
                quality.nanoseconds = 0;

                d_storeReport(d_store(persistentStore), D_LEVEL_FINER,
                            "Found group '%s.%s' that does not have data on disk so quality %d.\n",
                            strPartition, topicName, quality.seconds);
            }
            os_free(topicDataFileName);


            setPartitionTopicQuality(persistentStore, strPartition, topicName,
                                     quality, D_GROUP_INCOMPLETE);
            os_free(strPartition);
        } else {
            d_storeReport(d_store(persistentStore), D_LEVEL_FINEST, "No <METADATA> tag.\n");
            d_storeReport(d_store(persistentStore), D_LEVEL_FINEST, "Is not a meta file\n");
        }
    }
}

static void
processTopic(
    d_storeXML persistentStore,
    const char *      path,
    const char *      partition,
    const char *      topic,
    struct os_stat *     status )
{
    FILE * fDes;

    /* QAC EXPECT 1253,1277; */
    if (OS_ISREG(status->stat_mode)) {
        char * filename = os_fileNormalize(path);

        if(filename){
            fDes = fopen(filename, "r");
            os_free(filename);
            if (fDes == NULL) {
                /* (INACCESSABLE */
            } else {
                /* (REGULAR_FILE */
                processFile(persistentStore, fDes, partition, topic, status);
                fclose (fDes);
            }
        }
    }
}

static void
processPartition(
    d_storeXML persistentStore,
    const char *      path,
    const char *      parent__UNUSED_PARAMETER,
    const char *      partition,
    struct os_stat *  status )
{
    if (parent__UNUSED_PARAMETER == NULL) {
    }
    /* QAC EXPECT 1253,1277; */
    if (OS_ISDIR(status->stat_mode) || OS_ISLNK(status->stat_mode)) {
        d_storeReport(d_store(persistentStore), D_LEVEL_FINE, "partition: %s\n", path);
        forAllDirectoryEntries(persistentStore, path, partition, processTopic, FALSE);
    }
}


/* TODO: Update implementation to use stringKeyEscape() for each key-value
 * that is a string
 */
static c_char*
determineInstance(
    const v_groupAction msg)
{
    v_message message;
    v_group group;
    c_long nrOfKeys, i;
    c_char* keyValues;
    c_value value;
    c_char* valueImage, *notEscaped;
    c_array keyList;
    c_iter keyValuesIter;
    os_size_t totalSize;
    c_bool proceed = TRUE;

    keyValues   = NULL;

    if(msg){
        message  = msg->message;
        group    = msg->group;
        keyList  = v_topicMessageKeyList(group->topic);
        nrOfKeys = c_arraySize(keyList);

        if (nrOfKeys < 1) {
            keyValues = (c_char *)os_malloc(strlen(KEY_START_TAG) +
                                        + strlen(KEY_END_TAG) + 1);
            if(keyValues){
                os_sprintf(keyValues, "%s%s", KEY_START_TAG, KEY_END_TAG);
            }
        } else {
            totalSize = 0;
            keyValuesIter = c_iterNew(NULL);

            if(keyValuesIter){
                for (i = 0; (i < nrOfKeys) && (proceed == TRUE); i++) {
                    value = c_fieldValue(keyList[i], (c_object)message);

                    switch(value.kind){
                    case V_STRING:
                        /*fallthrough on purpose */
                    case V_CHAR:
                        notEscaped = c_valueImage(value);
                        valueImage = stringKeyEscape(notEscaped, TRUE);
                        os_free(notEscaped);
                        break;
                    default:
                        valueImage = c_valueImage(value);
                        break;
                    }
                    if(valueImage){
                        totalSize += strlen(valueImage) + 1;
                        c_iterAppend(keyValuesIter, valueImage);
                    } else {
                        proceed = FALSE;
                        break;
                    }
                    c_valueFreeRef(value);
                }
                if(proceed == TRUE){
                    keyValues = (c_char *)os_malloc(strlen(KEY_START_TAG) +
                            totalSize + strlen(KEY_END_TAG) + 1);

                    if(keyValues){
                        keyValues[0] = '\0';
                        os_strncat(keyValues, KEY_START_TAG, strlen(KEY_START_TAG));
                        assert(c_iterLength(keyValuesIter) == nrOfKeys);

                        for (i=0;i<nrOfKeys;i++) {
                            valueImage = (c_char*)c_iterTakeFirst(keyValuesIter);

                            if(valueImage){
                                os_strncat(keyValues, valueImage, strlen(valueImage));
                                os_free(valueImage);

                                if (i<(nrOfKeys-1)) {
                                    os_strncat(keyValues, ",", 1);
                                }
                            }
                        }
                        os_strncat(keyValues, KEY_END_TAG, strlen(KEY_END_TAG));
                    }
                } else {
                    valueImage = (c_char*)c_iterTakeFirst(keyValuesIter);

                    while(valueImage){
                        os_free(valueImage);
                        valueImage = (c_char*)c_iterTakeFirst(keyValuesIter);
                    }
                }
                c_iterFree(keyValuesIter);
            }
        }
    }
    return keyValues;
}

static persistentInstance
persistentInstanceNew(
    v_groupAction message)
{
    persistentInstance instance;

    assert(message);

    instance = persistentInstance(os_malloc(C_SIZEOF(persistentInstance)));

    instance->newMessage      = c_keep(message);
    instance->messages        = c_iterNew(NULL);
    instance->keyValue        = determineInstance(message);
    instance->result          = D_STORE_RESULT_OK;
    instance->writeCount      = 0;
    instance->disposeCount    = 0;
    instance->registerCount   = 0;
    instance->unregisterCount = 0;

    return instance;
}

static persistentInstance
persistentInstanceDummyNew(
    c_char* keyValue)
{
    persistentInstance instance;

    instance = persistentInstance(os_malloc(C_SIZEOF(persistentInstance)));

    instance->newMessage      = NULL;
    instance->messages        = c_iterNew(NULL);

    if(keyValue){
        instance->keyValue    = os_strdup(keyValue);
    } else {
        instance->keyValue    = NULL;
    }
    instance->result          = D_STORE_RESULT_OK;
    instance->writeCount      = 0;
    instance->disposeCount    = 0;
    instance->registerCount   = 0;
    instance->unregisterCount = 0;

    return instance;
}

static c_bool
persistentInstanceInsertUnregister(
    persistentInstance instance,
    v_message message)
{
    v_message reg, tmp;
    c_long i;
    c_bool added;

    assert(v_stateTest(v_nodeState(message), L_UNREGISTER));

    if(instance->registerCount != 0){
        reg = NULL;

        for(i=0; i<c_iterLength(instance->messages) && !reg; i++){
            reg = c_iterObject(instance->messages, i);

            if( (!v_stateTest(v_nodeState(reg), L_REGISTER)) ||
                ((v_gidCompare(reg->writerGID, message->writerGID) != C_EQ)))
            {
                reg = NULL;
            }
        }
        if(!reg){
            /*No registration found for unregistration, no need to insert*/
            added = FALSE;
        } else if(c_timeCompare(message->writeTime, reg->writeTime) != C_LT){
            tmp = c_iterTake(instance->messages, reg);
            assert(tmp == reg);
            c_free(reg);
            instance->registerCount--;

            /* There is no need to store the unregister if there is no data
             * anymore for the instance
             */
            if( (instance->registerCount == 0) &&
                (instance->writeCount == 0) &&
                (instance->disposeCount == 0))
            {
                added = FALSE;
            } else {
                instance->messages = c_iterAppend(instance->messages, message);
                instance->unregisterCount++;
                added = TRUE;
            }
        } else {
            /*No need to add unregistration, since there is a newer registration*/
            added = FALSE;
        }
    } else {
        /*No registration found for unregistration, no need to insert if there is no data*/
        if( (instance->writeCount == 0) &&
            (instance->disposeCount == 0))
        {
            added = FALSE;
        } else {
            instance->messages = c_iterAppend(instance->messages, message);
            instance->unregisterCount++;
            added = TRUE;
        }
    }
    return added;
}

static c_bool
persistentInstanceInsertRegister(
    persistentInstance instance,
    v_message message)
{
    v_message unreg, tmp;
    c_long i;
    c_bool added;

    assert(v_stateTest(v_nodeState(message), L_REGISTER));

    if(instance->unregisterCount != 0){
        unreg = NULL;

        for(i=0; i<c_iterLength(instance->messages) && !unreg; i++){
            unreg = c_iterObject(instance->messages, i);

            if( (!v_stateTest(v_nodeState(unreg), L_UNREGISTER)) ||
                ((v_gidCompare(unreg->writerGID, message->writerGID) != C_EQ)))
            {
                unreg = NULL;
            }
        }
        if(!unreg){
            instance->messages = c_iterAppend(instance->messages, message);
            instance->registerCount++;
            added = TRUE;
        } else if(c_timeCompare(message->writeTime, unreg->writeTime) != C_LT){
            tmp = c_iterTake(instance->messages, unreg);
            assert(tmp == unreg);
            c_free(unreg);
            instance->messages = c_iterAppend(instance->messages, message);
            instance->unregisterCount--;
            instance->registerCount++;
            added = TRUE;
        } else {
            added = FALSE;
        }
    } else {
        instance->messages = c_iterAppend(instance->messages, message);
        instance->registerCount++;
        added = TRUE;
    }
    return added;
}

static c_bool
persistentInstanceReplaceLastTransactionMessage(
    persistentInstance instance,
    v_message message)
{
    v_message current;
    c_bool replaced = FALSE;
    c_iter newList = c_iterNew(NULL);

    current = v_message(c_iterTakeFirst(instance->messages));

    while(current && !replaced){
        if(current->sequenceNumber == message->sequenceNumber){
            /* Same transactionId */
            if(V_MESSAGE_GET_TRANSACTION_UNIQUE_ID(current->transactionId)
                == V_MESSAGE_GET_TRANSACTION_UNIQUE_ID(message->transactionId))
            {
                /* Same DataWriter */
                if(v_gidCompare(current->writerGID, message->writerGID) == C_EQ)
                {
                    newList = c_iterAppend(newList, message);
                    c_free(current);
                    replaced = TRUE;
                }
            }
        }
        if(!replaced){
            newList = c_iterAppend(newList, current);
        }
        current = v_message(c_iterTakeFirst(instance->messages));
    }
    /* make sure rest of the list is stored as well in case something has been replaced. */
    while(current){
        newList = c_iterAppend(newList, current);
        current = v_message(c_iterTakeFirst(instance->messages));
    }
    c_iterFree(instance->messages);
    instance->messages = newList;

    return replaced;
}

static c_bool
persistentInstanceInsert(
    persistentInstance instance,
    v_topicQos topicQos,
    v_message message)
{
    c_bool added, checkCounters, replaced;
    v_message tmp, oldest;
    c_iter newList;

    added = FALSE;
    checkCounters = FALSE;

    if(v_stateTest(v_nodeState(message), L_REGISTER)){
        added = persistentInstanceInsertRegister(instance, message);
    } else if(v_stateTest(v_nodeState(message), L_UNREGISTER)){
        added = persistentInstanceInsertUnregister(instance, message);
    } else if(instance->writeCount == 0) { /*no write messages for instance yet; add it*/
        instance->messages = c_iterAppend(instance->messages, message);
        added = TRUE;
        checkCounters = TRUE;
    } else if(topicQos->durabilityService.history_kind == V_HISTORY_KEEPALL){ /*KEEP_ALL; add it*/
        instance->messages = c_iterAppend(instance->messages, message);
        added = TRUE;
        checkCounters = TRUE;
    } else { /*KEEP_LAST*/
        if(v_stateTest(v_nodeState(message), L_TRANSACTION)){
            replaced = persistentInstanceReplaceLastTransactionMessage(instance, message);
        } else {
            replaced = FALSE;
        }
        if(replaced){
            added = TRUE;
            checkCounters = FALSE;
        } else if( (instance->writeCount < topicQos->durabilityService.history_depth) || /*There is still room; add it*/
            (topicQos->durabilityService.history_depth == V_LENGTH_UNLIMITED) )
        {
            instance->messages = c_iterAppend(instance->messages, message);
            added = TRUE;
            checkCounters = TRUE;
        } else if(topicQos->orderby.kind == V_ORDERBY_RECEPTIONTIME) {/* V_ORDERBY_RECEPTIONTIME */
            if(!v_stateTest(v_nodeState(message), L_WRITE)){ /* Not a WRITE message; add it*/
                instance->messages = c_iterAppend(instance->messages, message);
                added = TRUE;
                checkCounters = TRUE;
            } else {
                /* This is a WRITE message; throw away last received write message,
                 * which is this one. Simply don't add it.
                 */

                /* Remove first sample */
                tmp = c_iterTakeFirst(instance->messages);
                c_free(tmp);
                tmp = NULL;

                /* Insert last sample */
                c_iterAppend(instance->messages, message);

                added = TRUE;
                checkCounters = TRUE;
            }
        } else { /* V_ORDERBY_SOURCETIME */
            if(!v_stateTest(v_nodeState(message), L_WRITE)){ /* Not a WRITE message; add it*/
                instance->messages = c_iterAppend(instance->messages, message);
                added = TRUE;
                checkCounters = TRUE;
            } else { /* This is a WRITE message; throw away write message with oldest write time*/
                assert(topicQos->durabilityService.history_depth == instance->writeCount);

                oldest = message;
                newList = c_iterNew(NULL);
                tmp = c_iterTakeFirst(instance->messages);

                while(tmp){
                    if(v_stateTest(v_nodeState(tmp), L_WRITE)){
                        if(v_timeCompare(tmp->writeTime, oldest->writeTime) == C_LT){
                            oldest = tmp;
                        } else {
                            newList = c_iterAppend(newList, tmp);
                        }
                    } else {
                        newList = c_iterAppend(newList, tmp);
                    }
                    tmp = c_iterTakeFirst(instance->messages);
                }
                if(oldest != message){
                    newList = c_iterAppend(newList, message);
                    c_free(oldest);
                    added = TRUE;
                }
                c_iterFree(instance->messages);
                instance->messages = newList;
            }
        }
    }
    if(checkCounters){
        if(v_stateTest(v_nodeState(message), L_WRITE)){
            instance->writeCount++;

            if(v_stateTest(v_nodeState(message), L_DISPOSED)){
                instance->disposeCount++;
            }
        } else if(v_stateTest(v_nodeState(message), L_DISPOSED)){
            instance->disposeCount++;
        } else if(v_stateTest(v_nodeState(message), L_REGISTER)){
            /*Do nothing here*/
        } else if(v_stateTest(v_nodeState(message), L_UNREGISTER)){
            /*Do nothing here*/
        } else {
            OS_REPORT_1(OS_ERROR, "durability", 0,
                    "Found message with state %d.",
                    v_nodeState(message));
            assert(FALSE);
        }
    }
    return added;
}

static int
persistentInstanceCompare(
    persistentInstance i1,
    persistentInstance i2)
{
    int result;

    if(i1 && i2){
        if(i1->keyValue && i2->keyValue){
            result = strcmp(i1->keyValue, i2->keyValue);
        } else if(i1->keyValue){
            result = 1;
        } else {
            result = -1;
        }
    } else if(i1){
        result = 1;
    } else {
        result = -1;
    }
    return result;
}

static void
persistentInstanceFree(
    persistentInstance instance)
{
    v_message message;

    if(instance){
        message = (v_message)(c_iterTakeFirst(instance->messages));

        while(message){
            if(instance->newMessage){
                if(message != (instance->newMessage->message)){
                    c_free(message);
                }
            } else {
                c_free(message);
            }
            message = (v_message)(c_iterTakeFirst(instance->messages));
        }
        c_iterFree(instance->messages);

        if(instance->newMessage){
            c_free(instance->newMessage);
        }
        if(instance->keyValue){
            os_free(instance->keyValue);
        }
        os_free(instance);
    }
    return;
}

static persistentInstance
persistentInstanceFindKey(
    persistentInstance instance,
    c_char* key)
{
    persistentInstance result;

    assert(key);
    result = NULL;

    if(strcmp(instance->keyValue, key) == 0){
        result = instance;
    }
    return result;
}

static groupExpungeActions
groupExpungeActionsNew(
    const c_char* partition,
    const c_char* topic)
{
    groupExpungeActions g;

    assert(partition);
    assert(topic);

    g = groupExpungeActions(os_malloc(C_SIZEOF(groupExpungeActions)));
    g->partition = os_strdup(partition);
    g->topic = os_strdup(topic);
    g->instances = d_tableNew(persistentInstanceCompare, persistentInstanceFree);

    return g;
}

static void
groupExpungeActionsFree(
    groupExpungeActions gea)
{
    if(gea){
        if(gea->partition){
            os_free(gea->partition);
        }
        if(gea->topic){
            os_free(gea->topic);
        }
        if(gea->instances){
            d_tableFree(gea->instances);
        }
        os_free(gea);
    }
}

static int
groupExpungeActionsCompare(
    groupExpungeActions g1,
    groupExpungeActions g2)
{
    int result;

    if(g1 != g2){
        result = strcmp(g1->partition, g2->partition);

        if(result == 0){
            result = strcmp(g1->topic, g2->topic);
        }
    } else {
        result = 0;
    }
    return result;
}

static void
groupExpungeActionsAdd(
    const d_storeXML persistentStore,
    const v_groupAction action)
{
    groupExpungeActions gea, dummy;
    persistentInstance instance, found;
    v_topicQos topicQos;

    gea = groupExpungeActionsNew(v_partitionName(v_groupPartition(action->group)),
                                 v_topicName(v_groupTopic(action->group)));
    dummy = d_tableInsert(persistentStore->expungeActions, gea);

    if(dummy){ /*Group already exists*/
        groupExpungeActionsFree(gea);
        gea = dummy;
    }
    instance = persistentInstanceNew(action);
    found = d_tableInsert(gea->instances, instance);

    if(found){
        persistentInstanceFree(instance);
        topicQos = v_topicQosRef(v_groupTopic(action->group));

        if(topicQos->orderby.kind == V_ORDERBY_RECEPTIONTIME){
            c_free(found->newMessage);
            found->newMessage = c_keep(action);
        } else if(v_timeCompare(found->newMessage->message->writeTime,
                                action->message->writeTime) == C_LT){
            c_free(found->newMessage);
            found->newMessage = c_keep(action);
        }
    }
}

static FILE*
getStoreFile(
    d_storeXML persistentStore,
    const c_char* path,
    const c_char* mode)
{
    d_storeFile result = NULL;
    FILE* fdes = NULL;

    assert(path);
    assert(mode);

    if(persistentStore->sessionAlive == TRUE){
        persistentStore->dummyFile->path = (c_char*)path;
        result = d_tableFind(persistentStore->openedFiles,
                             persistentStore->dummyFile);

        if(result){
            if(result->mode){
                if(strcmp(result->mode, mode) != 0){
                    result = d_tableRemove(persistentStore->openedFiles,
                                           result);
                    d_storeFileFree(result);
                    result = NULL;
                }
            } else {
                result = d_tableRemove(persistentStore->openedFiles, result);
                d_storeFileFree(result);
                result = NULL;
            }
        }
        persistentStore->dummyFile->path = NULL;
    }

    if(result){
        fdes = result->fdes;
    } else {
        char * filename = os_fileNormalize(path);
        if(filename){
            fdes = fopen(filename, mode);
            os_free(filename);
            if(fdes && (persistentStore->sessionAlive == TRUE)){
                result = d_storeFileNew(path, fdes, mode);
                d_tableInsert(persistentStore->openedFiles, result);
            }
        }
    }
    return fdes;
}

static int
openFile(
    d_storeXML        persistentStore,
    v_group           group,
    FILE **           fdes)
{
    c_char *    topicName;
    c_char *    partitionName;
    c_char *    fileStorePath;
    c_char      testEnd[15];
    struct os_stat statBuf;
    os_result   status;
    c_long      len = 0;

    testEnd[0] = '\0';

    topicName = v_topicName(v_groupTopic(group));
    partitionName = v_partitionName(v_groupPartition(group));
    fileStorePath = getDataFileName(persistentStore,
                                    partitionName,
                                    topicName);
    status = os_stat(fileStorePath, &statBuf);

    if (status == os_resultSuccess) {
        *fdes = getStoreFile(persistentStore, fileStorePath, "r");
    } else {
        *fdes = NULL;
    }
    if (*fdes != NULL) {
        len = statBuf.stat_size;
        fseek(*fdes, len-10, SEEK_SET);
        testEnd[14] = '\0';
        fscanf(*fdes, "%s", testEnd);
        status = strncmp(testEnd, D_STORE_END_STRING_NO_NL, strlen(D_STORE_END_STRING_NO_NL));

        if  (status != 0) {
           OS_REPORT_4(OS_WARNING, D_CONTEXT, 0,
               "Data for group '%s.%s' on disk mutilated "
               "(expected=\"%s\", found=\"%s\"), some data might be lost.",
               partitionName, topicName,
               D_STORE_END_STRING_NO_NL, testEnd);

        }
        fseek(*fdes, 0, SEEK_SET);
    } else {
        len = -1;
        d_storeReport(d_store(persistentStore), D_LEVEL_FINE,
                RR_TOPIC_NOT_YET_KNOWN, topicName);
    }

    os_free(fileStorePath);

    return (len);
/* QAC EXPECT 5101,5103; Complexity and too many lines of code */
}

static FILE*
openPersistentFile(
    d_storeXML persistentStore,
    const c_char* partition,
    const c_char* topic,
    const c_char* flags)
{
    FILE* fdes;
    c_char* fileStorePath;

    fdes = NULL;
    fileStorePath = getDataFileName(persistentStore, partition, topic);
    fdes = getStoreFile(persistentStore, fileStorePath, flags);
    os_free(fileStorePath);

    return fdes;
}

static FILE*
openPersistentTempFile(
    const d_storeXML persistentStore,
    const c_char* partition,
    const c_char* topic,
    const c_char* flags)
{
    FILE* fdes;
    c_char* fileStorePath;

    fdes = NULL;
    fileStorePath = getTmpFileName(persistentStore, partition, topic);
    fdes = getStoreFile(persistentStore, fileStorePath, flags);
    os_free(fileStorePath);

    return fdes;
}

static void
closeFile(
    const d_storeXML persistentStore,
    FILE* fdes)
{
    if(persistentStore->sessionAlive == FALSE){
        fflush(fdes);
        os_fsync(fdes);
        fclose(fdes);
    } else {
        fflush(fdes);
    }
}

static c_bool
renameTempToActual(
    const d_storeXML persistentStore,
    const c_char* partition,
    const c_char* topic)
{
    c_char* fileStorePath;
    c_char* tmpStorePath;
    struct os_stat statBuf;
    os_result osResult;
    c_bool result;
    d_storeFile storeFile, found;

    result = FALSE;

    fileStorePath = getDataFileName(persistentStore, partition, topic);
    tmpStorePath  = getTmpFileName(persistentStore, partition, topic);

    osResult = os_stat(tmpStorePath, &statBuf);

    if(osResult == os_resultSuccess){
        storeFile = d_storeFileNew(fileStorePath, NULL, NULL);
        found = d_tableRemove(persistentStore->openedFiles, storeFile);

        if(found){
            if(found->fdes){
                d_storeFileFree(found);
            }
        }
        d_storeFileFree(storeFile);
        storeFile = d_storeFileNew(tmpStorePath, NULL, NULL);
        found = d_tableRemove(persistentStore->openedFiles, storeFile);

        if(found){
            if(found->fdes){
                d_storeFileFree(found);
            }
        }
        d_storeFileFree(storeFile);

        osResult = os_remove(fileStorePath);
        if(osResult == os_resultSuccess){
            osResult = os_rename(tmpStorePath, fileStorePath);

            if(osResult == os_resultSuccess){
                result = TRUE;
            }
        }
    }
    os_free(fileStorePath);
    os_free(tmpStorePath);

    return result;

}

static d_storeResult
setOptimizeTime(
    const d_storeXML persistentStore,
    const c_char* partition,
    const c_char* topic,
    const os_time optimizeTime)
{
    d_storeResult result;
    c_char* optfname;
    FILE* fdes;

    optfname = getOptimizeFileName(persistentStore, partition, topic);

    if(optfname){
        char * filename = os_fileNormalize(optfname);
        fdes = fopen(filename, "w");
        os_free(filename);

        if(fdes){
            fprintf(fdes, "%d", optimizeTime.tv_sec);
            os_fsync(fdes);
            fclose(fdes);
            result = D_STORE_RESULT_OK;
        } else {
            result = D_STORE_RESULT_IO_ERROR;
        }
        os_free(optfname);
    } else {
        result = D_STORE_RESULT_ERROR;
    }
    return result;
}

static c_bool
isOptimized(
    const d_storeXML persistentStore,
    const c_char* partition,
    const c_char* topic)
{
    c_bool result;
    c_char *optfname, *dataFileName;
    FILE* fdes;
    c_long seconds;
    struct os_stat statBuf;
    os_result statResult;

    dataFileName = getDataFileName(persistentStore, partition, topic);
    optfname = getOptimizeFileName(persistentStore, partition, topic);

    if(optfname && optfname){
        char * filename = os_fileNormalize(optfname);
        fdes = fopen(filename, "r");
        os_free(filename);

        if(fdes){
            fscanf(fdes, "%d", &seconds);
            fclose(fdes);
            statResult = os_stat(dataFileName, &statBuf);

            if((statResult == os_resultSuccess) && (statBuf.stat_mtime.tv_sec <= seconds)){
                result = TRUE;
            } else {
                result = FALSE;
            }
        } else {
            result = FALSE;
        }
        os_free(optfname);
        os_free(dataFileName);
    } else if(dataFileName){
        os_free(dataFileName);
        result = FALSE;
    } else if(optfname){
        os_free(optfname);
        result = FALSE;
    } else {
        result = FALSE;
    }
    return result;
}

static d_storeResult
deleteHistoricalData(
    const d_storeXML persistentStore,
    const v_groupAction msg)
{
    v_group group;
    FILE *fdes, *tmpfdes;
    c_char *data, *partition, *topic, *keys;
    c_type type;
    sd_serializer serializer;
    v_message perData;
    sd_serializedData serData;
    c_long status1;
    c_time deleteTime;
    c_bool renameSuccess;
    d_storeResult result;
    c_bool success;

    group      = msg->group;
    partition  = v_partitionName(v_groupPartition(group));
    topic      = v_topicName(v_groupTopic(group));
    deleteTime = msg->actionTime;
    fdes       = openPersistentFile(persistentStore, partition, topic, "r");
    tmpfdes    = openPersistentTempFile(persistentStore, partition, topic, "w");
    result     = D_STORE_RESULT_OK;

    if(fdes && tmpfdes) {
        type = v_topicMessageType(group->topic);
        data = persistentStore->dataBuffer;
        data[0] = '\0';
        keys = persistentStore->keyBuffer;
        keys[0] = '\0';

        if ((data == NULL) || (keys == NULL)) {
            d_storeReport(d_store(persistentStore),
                          D_LEVEL_SEVERE,
                          RR_COULD_NOT_READ_TOPIC" \n",
                          topic);
            OS_REPORT_1(OS_ERROR, STORE_READ_TOPIC_XML, 0,
                       RR_COULD_NOT_READ_TOPIC" \n", topic);
            result = D_STORE_RESULT_ERROR;
        } else {
            serializer = sd_serializerXMLNewTyped(type);
            data[0] = '\0';
            readLine(fdes, MAX_KEY_SIZE, data);
            status1 = strncmp(data, "<TOPIC>", 7);

            if (status1 != 0) {
                d_storeReport(d_store(persistentStore), D_LEVEL_SEVERE,
                RR_TOPIC_SHOULD_BEGIN_WITH_TOPIC_TAG, topic);
                OS_REPORT_1(OS_ERROR, STORE_READ_TOPIC_XML, 0,
                           RR_TOPIC_SHOULD_BEGIN_WITH_TOPIC_TAG, topic);
                result = D_STORE_RESULT_MUTILATED;
            } else {
                fprintf(tmpfdes, D_STORE_START_STRING);
                data[0] = '\0';
                /*read keys*/
                readLine(fdes, MAX_KEY_SIZE, keys);
                status1 = feof(fdes);

                if(strncmp(keys, D_STORE_END_STRING_NO_NL, strlen(D_STORE_END_STRING_NO_NL)) != 0){
                    while (status1 == 0) {
                        /*read data*/
                        readObject(fdes, MAX_MESSAGE_SIZE, data);
                        serData = sd_serializerFromString(serializer, data);
                        perData = sd_serializerDeserialize(serializer, serData);

                        if(v_timeCompare(perData->writeTime, deleteTime) == C_GT){
                            fprintf(tmpfdes, "%s\n", keys); /*write keys*/
                            fprintf(tmpfdes, "%s\n", data); /*write data*/
                        }
                        sd_serializedDataFree(serData);
                        keys[0] = '\0';
                        data[0] = '\0';
                        /*read keys*/
                        readLine(fdes, MAX_KEY_SIZE, keys);
                        status1 = feof(fdes);

                        if(status1 == 0){
                            if(strncmp(keys, D_STORE_END_STRING_NO_NL, strlen(D_STORE_END_STRING_NO_NL)) == 0){
                                status1 = -1;
                            }
                        }
                    }
                }
            }
            sd_serializerFree(serializer);
        }
        fprintf(tmpfdes, D_STORE_END_STRING);
        closeFile(persistentStore, fdes);
        closeFile(persistentStore, tmpfdes);
    } else {
        d_storeReport(d_store(persistentStore), D_LEVEL_FINE, "No data exists yet on disk for group '%s.%s' (2)\n", partition, topic);
        success = createDirectoryIfNecessary(persistentStore, partition);

        if(success){
            success = storeTopicMetadata(persistentStore, group);

            if(success){
                tmpfdes = openPersistentTempFile(persistentStore, partition, topic, "w");

                if(tmpfdes){
                    fprintf(tmpfdes, D_STORE_START_STRING);
                    fprintf(tmpfdes, D_STORE_END_STRING);
                    closeFile(persistentStore, tmpfdes);
                } else {
                    d_storeReport(d_store(persistentStore),
                          D_LEVEL_SEVERE, "Could not open file for writing.\n");
                    result = D_STORE_RESULT_IO_ERROR;
                }
            } else {
                d_storeReport(d_store(persistentStore),
                        D_LEVEL_SEVERE, "Could not store metadata to disk.\n");
                result = D_STORE_RESULT_IO_ERROR;
            }
        } else {
            d_storeReport(d_store(persistentStore),
                    D_LEVEL_SEVERE, "Could not create directory.\n");
            result = D_STORE_RESULT_IO_ERROR;
        }
    }
    if(result == D_STORE_RESULT_OK){
        renameSuccess = renameTempToActual(persistentStore, partition, topic);

        if(renameSuccess == TRUE){
            result = D_STORE_RESULT_OK;
        }
        /* empty the expunge list */
        d_tableFree(persistentStore->expungeActions);
        persistentStore->expungeActions = d_tableNew(groupExpungeActionsCompare, groupExpungeActionsFree);
    }
    return result;
}

static persistentInstance
persistentInstanceRead(
    const d_storeXML persistentStore,
    const v_groupAction msg)
{
    persistentInstance instance;
    v_group group;
    FILE *fdes, *tmpfdes;
    c_char *data, *partition, *topic;
    c_type type;
    sd_serializer serializer;
    v_message perData;
    sd_serializedData serData;
    c_long status1;
    persistentInstance pInstance;
    c_bool success;

    instance  = persistentInstanceNew(msg);
    group     = msg->group;
    partition = v_partitionName(v_groupPartition(group));
    topic     = v_topicName(v_groupTopic(group));

    fdes = openPersistentFile(persistentStore, partition, topic, "r");
    tmpfdes = openPersistentTempFile(persistentStore, partition, topic, "w");

    if(fdes && tmpfdes) {
        type = v_topicMessageType(group->topic);
        data = persistentStore->dataBuffer;
        data[0] = '\0';

        if (data == NULL) {
            d_storeReport(d_store(persistentStore),
                          D_LEVEL_SEVERE,
                          RR_COULD_NOT_READ_TOPIC" \n",
                          topic);
            OS_REPORT_1(OS_ERROR, STORE_READ_TOPIC_XML, 0,
                        RR_COULD_NOT_READ_TOPIC" \n", topic);
        } else {
            serializer = sd_serializerXMLNewTyped(type);

            if(serializer){
                data[0] = '\0';
                readLine(fdes, MAX_MESSAGE_SIZE, data);
                status1 = strncmp(data, "<TOPIC>", 7);

                if (status1 != 0) {
                    d_storeReport(d_store(persistentStore), D_LEVEL_SEVERE,
                    RR_TOPIC_SHOULD_BEGIN_WITH_TOPIC_TAG, topic);
                    OS_REPORT_1(OS_ERROR, STORE_READ_TOPIC_XML, 0,
                               RR_TOPIC_SHOULD_BEGIN_WITH_TOPIC_TAG, topic);
                } else {
                    fprintf(tmpfdes, D_STORE_START_STRING);
                    data[0] = '\0';
                    /*read keys*/
                    readLine(fdes, MAX_KEY_SIZE, data);
                    status1 = feof(fdes);

                    if(strncmp(data, D_STORE_END_STRING, strlen(D_STORE_END_STRING)) != 0){

                        while (status1 == 0) {
                            pInstance = persistentInstanceFindKey(instance, data);

                            if(pInstance){
                                readObject(fdes, MAX_MESSAGE_SIZE, data);
                                serData = sd_serializerFromString(serializer, data);
                                perData = sd_serializerDeserialize(serializer, serData);
                                pInstance->messages = c_iterAppend(pInstance->messages, perData);
                                sd_serializedDataFree(serData);

                                if(v_stateTest(v_nodeState(perData), L_WRITE)){
                                    pInstance->writeCount++;
                                } else if(v_stateTest(v_nodeState(perData), L_DISPOSED)){
                                    pInstance->disposeCount++;
                                } else if(v_stateTest(v_nodeState(perData), L_REGISTER)){
                                    pInstance->registerCount++;
                                } else if(v_stateTest(v_nodeState(perData), L_UNREGISTER)){
                                    pInstance->unregisterCount++;
                                } else {
                                    OS_REPORT_1(OS_ERROR, "durability", 0,
                                            "Found message with state %d.",
                                            v_nodeState(perData));
                                     assert(FALSE);
                                }
                            } else {
                                fprintf(tmpfdes, "%s\n", data); /*write keys*/
                                /*read data*/
                                readObject(fdes, MAX_MESSAGE_SIZE, data);
                                fprintf(tmpfdes, "%s\n", data); /*write data*/
                            }
                            data[0] = '\0';
                            /*read keys*/
                            readLine(fdes, MAX_KEY_SIZE, data);
                            status1 = feof(fdes);

                            if(status1 == 0){
                                if(strncmp(data, D_STORE_END_STRING, strlen(D_STORE_END_STRING)) == 0){
                                    status1 = -1;
                                }
                            }
                        }
                    }
                }
                sd_serializerFree(serializer);
            }
        }
        closeFile(persistentStore, fdes);
        closeFile(persistentStore, tmpfdes);
    } else {
        d_storeReport(d_store(persistentStore), D_LEVEL_FINE,
                      "No data exists yet on disk for group '%s.%s' (2)\n", partition, topic);
        success = createDirectoryIfNecessary(persistentStore, partition);

        if(success){
            success = storeTopicMetadata(persistentStore, group);

            if(success){
                tmpfdes = openPersistentTempFile(persistentStore, partition, topic, "w");

                if(tmpfdes){
                    fprintf(tmpfdes, D_STORE_START_STRING);
                    closeFile(persistentStore, tmpfdes);
                }
            }
        }
    }
    return instance;
}

static d_storeResult
persistentInstanceStore(
    const d_storeXML persistentStore,
    persistentInstance instance)
{
    FILE* tmpfdes;
    v_group group;
    c_type type;
    sd_serializer serializer;
    sd_serializedData serData;
    c_char *partition, *topic, *data;
    d_storeResult result;
    c_bool renameSuccess;
    v_message message;

    result    = D_STORE_RESULT_ERROR;
    group     = instance->newMessage->group;
    partition = v_partitionName(v_groupPartition(group));
    topic     = v_topicName(v_groupTopic(group));

    tmpfdes = openPersistentTempFile(persistentStore, partition, topic, "a");

    if(tmpfdes){
        type = v_topicMessageType(group->topic);
        serializer = sd_serializerXMLNewTyped(type);

        if(serializer){
            message = (v_message)c_iterTakeFirst(instance->messages);

            while(message){
                serData = sd_serializerSerialize(serializer, (c_object)message);
                data = sd_serializerToString(serializer, serData);

                if(data){
                    fprintf(tmpfdes, "%s\n%s\n", instance->keyValue, data);
                    sd_serializedDataFree(serData);
                    os_free(data);
                }
                if(message != instance->newMessage->message){
                    c_free(message);
                }
                message = (v_message)c_iterTakeFirst(instance->messages);
            }
            sd_serializerFree(serializer);
            fprintf(tmpfdes, D_STORE_END_STRING);
            closeFile(persistentStore, tmpfdes);
            renameSuccess = renameTempToActual(persistentStore, partition, topic);

            if(renameSuccess == TRUE){
                result = D_STORE_RESULT_OK;
            } else {
                result = D_STORE_RESULT_IO_ERROR;
            }
        } else {
            result = D_STORE_RESULT_OUT_OF_RESOURCES;
        }
    } else {
        result = D_STORE_RESULT_MUTILATED;
    }
    return result;
}

static d_storeResult
appendMessage(
    const d_storeXML persistentStore,
    const v_groupAction msg)
{
    FILE *fdes;
    c_char *data, *partition, *topic, *keys;
    sd_serializer serializer;
    sd_serializedData serData;
    d_storeResult result;
    c_bool success;
    int res;

    partition = v_partitionName(v_groupPartition(msg->group));
    topic     = v_topicName(v_groupTopic(msg->group));
    fdes      = openPersistentFile(persistentStore, partition, topic, "r+");

    if(!fdes){
        d_storeReport(d_store(persistentStore), D_LEVEL_FINE, "No data exists yet on disk for group '%s.%s' (2)\n", partition, topic);
        success = createDirectoryIfNecessary(persistentStore, partition);

        if(success){
            success = storeTopicMetadata(persistentStore, msg->group);

            if(success) {
                fdes = openPersistentFile(persistentStore, partition, topic, "w");

                if(fdes){
                    fprintf(fdes, D_STORE_START_STRING);
                    serializer = sd_serializerXMLNewTyped(c_getType(msg->message));

                    if(serializer){
                        serData = sd_serializerSerialize(serializer, (c_object)msg->message);

                        if(serData){
                            data = sd_serializerToString(serializer, serData);

                            if(data){
                                keys = determineInstance(msg);

                                if(keys){
                                    fprintf(fdes, "%s\n%s\n", keys, data);
                                    fprintf(fdes, D_STORE_END_STRING);
                                    os_free(keys);
                                    closeFile(persistentStore, fdes);
                                    result = D_STORE_RESULT_OK;
                                } else {
                                    result = D_STORE_RESULT_OUT_OF_RESOURCES;
                                }
                                os_free(data);
                            } else {
                                result = D_STORE_RESULT_OUT_OF_RESOURCES;
                            }
                            sd_serializedDataFree(serData);
                        } else {
                            result = D_STORE_RESULT_OUT_OF_RESOURCES;
                        }
                        sd_serializerFree(serializer);
                    } else {
                        result = D_STORE_RESULT_OUT_OF_RESOURCES;
                    }
                } else {
                    d_storeReport(d_store(persistentStore), D_LEVEL_SEVERE,
                        "Unable to open persistent file for writing for group '%s.%s' (2)\n",
                        partition, topic);
                    OS_REPORT_2(OS_ERROR, D_CONTEXT, 0,
                        "Unable to open persistent file for writing for group '%s.%s' (2)\n",
                        partition, topic);
                    result = D_STORE_RESULT_IO_ERROR;
                }
            } else {
                result = D_STORE_RESULT_IO_ERROR;
            }
        } else {
            result = D_STORE_RESULT_IO_ERROR;
        }
    } else {
        res = fseek(fdes, D_SEEK_LENGTH, SEEK_END);
        assert(res == 0);

        serializer = sd_serializerXMLNewTyped(c_getType(msg->message));

        if(serializer){
            serData = sd_serializerSerialize(serializer, (c_object)msg->message);

            if(serData){
                data = sd_serializerToString(serializer, serData);

                if(data){
                    keys = determineInstance(msg);

                    if(keys){
                        fprintf(fdes, "%s\n%s\n", keys, data);
                        fprintf(fdes, D_STORE_END_STRING);
                        os_free(keys);
                        closeFile(persistentStore, fdes);
                        result = D_STORE_RESULT_OK;
                    } else {
                        result = D_STORE_RESULT_OUT_OF_RESOURCES;
                    }
                    os_free(data);
                } else {
                    result = D_STORE_RESULT_OUT_OF_RESOURCES;
                }
                sd_serializedDataFree(serData);
            } else {
                result = D_STORE_RESULT_OUT_OF_RESOURCES;
            }
            sd_serializerFree(serializer);
        } else {
            result = D_STORE_RESULT_OUT_OF_RESOURCES;
        }
    }
    return result;
}

static d_storeResult
expungeMessage(
    const d_storeXML persistentStore,
    const v_groupAction msg,
    c_bool messageOnly)
{
    c_long i;
    persistentInstance instance;
    v_topicQos qos;
    d_storeResult result;
    v_message message, newMessage, old;
    c_iter toRemove;
    v_group group;
    c_bool found;

    instance = persistentInstanceRead(persistentStore, msg);

    if(instance){
        toRemove   = c_iterNew(NULL);
        newMessage = v_message(instance->newMessage->message);

        if(newMessage){
            if(messageOnly == TRUE){
                found = FALSE;

                for(i=0; i<c_iterLength(instance->messages) && (found == FALSE); i++){
                    message = v_message(c_iterObject(instance->messages, i));

                    if(v_timeCompare(message->writeTime, newMessage->writeTime) == C_EQ){
                        if( v_gidEqual(message->writerGID, newMessage->writerGID))
                        {
                            toRemove = c_iterInsert(toRemove, message);
                            found = TRUE;
                        }
                    }
                }
            } else {
                group     = v_group(instance->newMessage->group);
                qos       = v_topicQosRef(group->topic);

                if(qos->orderby.kind == V_ORDERBY_RECEPTIONTIME) {
                    for(i=0; i<c_iterLength(instance->messages); i++){
                        message = v_message(c_iterObject(instance->messages, i));
                        toRemove = c_iterInsert(toRemove, message);
                    }
                } else {
                    for(i=0; i<c_iterLength(instance->messages); i++){
                        message = v_message(c_iterObject(instance->messages, i));

                        if(v_timeCompare(message->writeTime, newMessage->writeTime) == C_LT){
                            toRemove = c_iterInsert(toRemove, message);
                        }
                    }
                }
            }
            old = v_message(c_iterTakeFirst(toRemove));

            while(old){
                c_iterTake(instance->messages, old);
                c_free(old);
                old = v_message(c_iterTakeFirst(toRemove));
            }
            result = persistentInstanceStore(persistentStore, instance);
            persistentInstanceFree(instance);
            c_iterFree(toRemove);
        } else {
            result = D_STORE_RESULT_OUT_OF_RESOURCES;
        }
    } else {
        result = D_STORE_RESULT_OUT_OF_RESOURCES;
    }
    return result;
}

static d_storeResult
expungeMessageOptimized(
    const d_storeXML persistentStore,
    const v_groupAction msg,
    c_bool messageOnly)
{
    d_storeResult result;

    if(persistentStore->sessionAlive == TRUE){
        groupExpungeActionsAdd(persistentStore, msg);
        result = D_STORE_RESULT_OK;
    } else {
        result = expungeMessage(persistentStore, msg, messageOnly);
    }
    return result;
}

static c_bool
processExpungeAction(
    groupExpungeActions g,
    d_storeXML persistentStore)
{
    FILE *fdes, *tmpfdes;
    c_char *data, *partition, *topic, *key;
    c_type type;
    sd_serializer serializer;
    v_message perData;
    sd_serializedData serData;
    c_long status1;
    int cleanupCount, totalCount;
    persistentInstance pInstance, instance;
    c_time deleteTime;

    partition = g->partition;
    topic     = g->topic;
    cleanupCount = 0;
    totalCount = 0;
    deleteTime.nanoseconds = 0;
    deleteTime.seconds = 0;

    fdes = openPersistentFile(persistentStore, partition, topic, "r");
    tmpfdes = openPersistentTempFile(persistentStore, partition, topic, "w");

    if(fdes && tmpfdes) {
        pInstance = d_tableFirst(g->instances);

        if(pInstance && pInstance->newMessage){
            type = c_getType(pInstance->newMessage->message);
            pInstance = NULL;

            data = persistentStore->dataBuffer;
            key  = persistentStore->keyBuffer;
            data[0] = '\0';
            key[0] = '\0';

            serializer = sd_serializerXMLNewTyped(type);

            if(serializer){
                readLine(fdes, MAX_MESSAGE_SIZE, data);
                status1 = strncmp(data, "<TOPIC>", 7);

                if (status1 != 0) {
                    d_storeReport(d_store(persistentStore), D_LEVEL_SEVERE,
                    RR_TOPIC_SHOULD_BEGIN_WITH_TOPIC_TAG, topic);
                    OS_REPORT_1(OS_ERROR, STORE_READ_TOPIC_XML, 0,
                               RR_TOPIC_SHOULD_BEGIN_WITH_TOPIC_TAG, topic);
                } else {
                    fprintf(tmpfdes, D_STORE_START_STRING);
                    data[0] = '\0';
                    key[0] = '\0';
                    /*read keys*/
                    readLine(fdes, MAX_KEY_SIZE, key);
                    status1 = feof(fdes);

                    if(strncmp(key, D_STORE_END_STRING, strlen(D_STORE_END_STRING)) != 0){
                        while (status1 == 0) {
                            instance = persistentInstanceDummyNew(key);
                            pInstance = d_tableFind(g->instances, instance);
                            persistentInstanceFree(instance);

                            if(pInstance){
                                readObject(fdes, MAX_MESSAGE_SIZE, data);
                                serData = sd_serializerFromString(serializer, data);
                                perData = sd_serializerDeserialize(serializer, serData);
                                sd_serializedDataFree(serData);

                                switch(pInstance->newMessage->kind){
                                    case V_GROUP_ACTION_CLEANUP_DELAY_EXPIRE:
                                        deleteTime = pInstance->newMessage->actionTime;
                                        break;
                                    case V_GROUP_ACTION_LIFESPAN_EXPIRE:
                                        deleteTime = pInstance->newMessage->message->writeTime;
                                        break;
                                    default:
                                        /*Message should not be in this list*/
                                        assert(FALSE);
                                        break;
                                }

                                if(v_timeCompare(perData->writeTime, deleteTime) == C_GT){
                                    /*write it*/
                                    fprintf(tmpfdes, "%s\n", key); /*write keys*/
                                    fprintf(tmpfdes, "%s\n", data); /*write data*/
                                } else {
                                    /*throw away*/
                                    cleanupCount++;
                                }
                                c_free(perData);
                                totalCount++;
                            } else {
                                fprintf(tmpfdes, "%s\n", key); /*write keys*/
                                /*read data*/
                                readObject(fdes, MAX_MESSAGE_SIZE, data);
                                fprintf(tmpfdes, "%s\n", data); /*write data*/
                            }

                            data[0] = '\0';
                            key[0] = '\0';
                            /*read keys*/
                            readLine(fdes, MAX_KEY_SIZE, key);
                            status1 = feof(fdes);

                            if(status1 == 0){
                                if(strncmp(key, D_STORE_END_STRING, strlen(D_STORE_END_STRING)) == 0){
                                    status1 = -1;
                                }
                            }
                        }
                    }
                }
                sd_serializerFree(serializer);
            } else {
                d_storeReport(d_store(persistentStore), D_LEVEL_SEVERE,
                            "Unable to allocate serializer\n");
            }
            fprintf(tmpfdes, D_STORE_END_STRING);
        } else {
            d_storeReport(d_store(persistentStore), D_LEVEL_SEVERE,
                    "Unable to get instance for group %s.%s' (2)\n", partition, topic);
        }
        closeFile(persistentStore, fdes);
        closeFile(persistentStore, tmpfdes);
        renameTempToActual(persistentStore, partition, topic);
    } else {
        d_storeReport(d_store(persistentStore), D_LEVEL_FINE, "No data exists yet on disk for group '%s.%s' (2)\n", partition, topic);
    }
    return TRUE;
}

static c_bool
processGroupExpungeActions(
    d_storeXML persistentStore)
{
    c_bool result;

    result = d_tableWalk(persistentStore->expungeActions, processExpungeAction, persistentStore);

    return result;
}

/* Helper struct for unregister walk function */
typedef struct unregisterFindData
{
    v_message writeMsg;
    v_message unregisterMsg;
} unregisterFindData;

/* Search for a writer in registration list */
static void
unregisterLookupWalk (
        void* o,
        c_iterActionArg userData)
{
    unregisterFindData* walkData;
    v_message message;

    message = v_message(o);
    walkData = (unregisterFindData*)userData;

    /* Only search if no unregister message has is found */
    if (!(walkData->unregisterMsg))
    {
        /* If message is a unregister message and has equal GID,
         * this is a valid unregister message for writeMessage */
        if (message &&
            (v_stateTest(v_nodeState(message), L_UNREGISTER)) &&
            (v_gidEqual(walkData->writeMsg->writerGID, message->writerGID))) {

            walkData->unregisterMsg = message;
        }
    }
}

/* Create unregister message from write message */
static v_message
createUnregisterMessage(
        v_group group,
        v_message message)
{
    c_array            messageKeyList;
    c_long                i, nrOfKeys;
    v_message            unregisterMessage;

    assert(!v_stateTest(v_nodeState(message), L_UNREGISTER));

    /* Create new message objec */
    unregisterMessage = v_topicMessageNew
                                (group->topic);

    /* Copy keyvalues to unregistermessage */
    messageKeyList = v_topicMessageKeyList(v_groupTopic(group));
    nrOfKeys = c_arraySize(messageKeyList);
    for (i=0;i<nrOfKeys;i++) {
        c_fieldAssign (messageKeyList[i],
                unregisterMessage,
                c_fieldValue(messageKeyList[i],message));
    }

    /* Set instance & writer GID */
    unregisterMessage->writerGID =
            message->writerGID;
    unregisterMessage->writerInstanceGID =
            message->writerInstanceGID;

    /* Copy messageQos */
    c_keep (message->qos);
    unregisterMessage->qos = message->qos;

    /* Set nodestate to unregister */
    v_nodeState(unregisterMessage) = L_UNREGISTER;

    return unregisterMessage;
}

/* PRE: The v_group exists */
static d_storeResult
d_storeXMLOptimizeGroup(
    d_storeXML        persistentStore,
    v_group           group,
    d_partition       partition,
    d_topic           topic,
    c_bool            inject,
    c_bool            optimize,
    c_ulong           dataVersion)
{
    FILE *             fdes, *tmpfdes;
    c_char *           data, *data2, *newKeyList;
    c_long             len;
    c_type             type;
    sd_serializer      serializer;
    v_message          message, perData;
    sd_serializedData  serData;
    c_long             status1, status2;
    c_bool             renameSuccess, added;
    d_storeResult      result;
    d_table            instances;
    persistentInstance dummy, instance;
    v_topicQos         topicQos;
    os_time            optimizeTime, oneSec;
    v_writeResult      wr;
    unregisterFindData walkData;
    c_bool             reported = FALSE;

    message = NULL;
    perData = NULL;
    result = D_STORE_RESULT_OK;
    tmpfdes = NULL;
    len = openFile(persistentStore, group, &fdes);

    /* Do not create temporary file for optimizing when no optimization is required */
    if (optimize) {
        tmpfdes = openPersistentTempFile(persistentStore, partition, topic, "w");
    }

    /* Store file exists and is not empty, if store needs optimization tmp file must exist */
    if ((len > 0) && fdes && (tmpfdes || !optimize)) {
        type = v_topicMessageType(group->topic);
        topicQos = v_topicQosRef(group->topic);
        data = (c_char *)d_malloc((os_uint32)(len + EXTRA_BACKSLS), "readData");
        if (data == NULL) {
            result = D_STORE_RESULT_OUT_OF_RESOURCES;
            d_storeReport(d_store(persistentStore), D_LEVEL_SEVERE, "No more resources\n");
            OS_REPORT(OS_ERROR, D_CONTEXT, 0, "No more resources available\n");

        } else {
            serializer = sd_serializerXMLNewTyped(type);

            if(serializer){
                data[0] = '\0';
                readLine(fdes, len, data);
                status1 = strncmp(data, "<TOPIC>", 7);

                if (status1 != 0) {
                    result = D_STORE_RESULT_MUTILATED;
                    d_storeReport(d_store(persistentStore), D_LEVEL_SEVERE,
                            RR_TOPIC_SHOULD_BEGIN_WITH_TOPIC_TAG, topic);
                    OS_REPORT_1(OS_ERROR, STORE_READ_TOPIC_XML, 0,
                               RR_TOPIC_SHOULD_BEGIN_WITH_TOPIC_TAG, topic);
                } else {
                    instances = d_tableNew(persistentInstanceCompare, persistentInstanceFree);
                    data[0] = '\0';
                    readLine(fdes, len, data); /*read keys*/
                    status1 = feof(fdes);

                    while (status1 == 0) {
                        if(dataVersion < 6){
                            /* convert key-values */
                            newKeyList = NULL;
                            (void)convertOldToNewKeyList(data, &newKeyList);
                            dummy = persistentInstanceDummyNew(newKeyList);
                            os_free(newKeyList);
                        } else {
                            dummy = persistentInstanceDummyNew(data);
                        }
                        instance = d_tableInsert(instances, dummy);

                        if(instance){
                            persistentInstanceFree(dummy);
                        } else {
                            instance = dummy;
                        }
                        data[0] = '\0';
                        readObject(fdes, len, data); /*read object*/
                        status2 = strncmp(data, "<object>", 8);

                        if (status2 == 0) {
                            serData = sd_serializerFromString(serializer, data);

                            if(serData){
                                perData = sd_serializerDeserializeValidated(serializer, serData);

                                if(perData){
                                    /* Dispose messages caused by dispose_all_data have no inline qos,
                                     * but make sure all the other messages do.
                                     */
                                    if (perData->qos != NULL ||
                                            (v_nodeState(perData) == L_DISPOSED && v_nodeState(perData) != L_WRITE)) {
                                        added = persistentInstanceInsert(instance,
                                                                         topicQos,
                                                                         perData);
                                        if(!added){
                                            c_free(perData);
                                        }
                                    } else {
                                        OS_REPORT_2(OS_ERROR, D_CONTEXT, 0,
                                            "Data for group '%s.%s' on disk is mutilated. "
                                            "Found valid data without QoS value.",
                                            partition, topic);
                                        assert(0);
                                        c_free(perData);
                                    }

                                } else if(!reported) {
                                    OS_REPORT_2(OS_ERROR, D_CONTEXT, 0,
                                            "Data for group '%s.%s' on disk is mutilated. "
                                            "Some data might be lost.",
                                            partition, topic);
                                    d_storeReport(d_store(persistentStore), D_LEVEL_SEVERE,
                                            "Data for group '%s.%s' on disk is mutilated. "
                                            "Some data might be lost.\n",
                                            partition, topic);
                                    reported = TRUE;
                                }
                                sd_serializedDataFree(serData);
                            }
                        }
                        data[0] = '\0';
                        readLine(fdes, len, data); /*read keys*/
                        status1 = feof(fdes);

                        /* In case the previous keylist was corrupt already,
                         * remove the instance from the list.
                        if(status2 != 0){
                            dummy = d_tableRemove(instances, instance);
                            assert(dummy == instance);
                            persistentInstanceFree(instance);
                        }
                         */
                    }
                    closeFile(persistentStore, fdes);

                    d_storeReport(d_store(persistentStore), D_LEVEL_FINEST,
                            "Optimizing: found %d instances for group '%s.%s'\n",
                            d_tableSize(instances),
                            partition, topic);

                    perData = NULL;

                    if (optimize) {
                        fprintf(tmpfdes, D_STORE_START_STRING);
                    }

                    if(serializer){
                        instance = d_tableTake(instances);

                        while(instance){
                            message = (v_message)c_iterTakeFirst(instance->messages);

                            while(message){

                                /* Inject data */
                                if(inject == TRUE){
                                    wr = v_groupWriteNoStream(group, message, NULL, V_NETWORKID_ANY);
                                    oneSec.tv_sec  = 1;
                                    oneSec.tv_nsec = 0;

                                    while(wr == V_WRITE_REJECTED){
                                        wr = v_groupWriteNoStream(group, message, NULL, V_NETWORKID_ANY);
                                        os_nanoSleep(oneSec);
                                    }

                                    if((wr != V_WRITE_SUCCESS) &&
                                       (wr != V_WRITE_REGISTERED) &&
                                       (wr != V_WRITE_UNREGISTERED)) {
                                        OS_REPORT_1(OS_ERROR, D_CONTEXT, 0,
                                            "Unable to write persistent data to group. (result: '%d')\n",
                                            wr);
                                        result = D_STORE_RESULT_ERROR;
                                    }else
                                    {
                                        /* If a sample is written or registered, add unregister action */
                                        if (!v_stateTest(v_nodeState(message), L_UNREGISTER)) {
                                            /* Prepare walkdata */
                                            walkData.writeMsg = message;
                                            walkData.unregisterMsg = NULL;

                                            /* Lookup unregister message */
                                            c_iterWalk (
                                                    instance->messages,
                                                    unregisterLookupWalk,
                                                    &walkData);

                                            /* Create unregister message if none found */
                                            if (!(walkData.unregisterMsg)) {
                                                walkData.unregisterMsg =
                                                        createUnregisterMessage (group, message);
                                                c_iterAppend (instance->messages, walkData.unregisterMsg);
                                            }

                                            /* Set valid sequence number */
                                            if (message->sequenceNumber >=
                                                    walkData.unregisterMsg->sequenceNumber) {
                                                walkData.unregisterMsg->sequenceNumber =
                                                        message->sequenceNumber + 1;
                                            }

                                            /* Set sample time (always) after write\register time */
                                            walkData.unregisterMsg->writeTime = v_timeGet();
#ifndef _NAT_
                                            walkData.unregisterMsg->allocTime = walkData.unregisterMsg->writeTime;
#endif
                                        }
                                    }
                                }

                                /* Write optimized data */
                                if (optimize) {
                                    serData = sd_serializerSerialize(serializer, (c_object)message);

                                    if(serData){
                                        data2 = sd_serializerToString(serializer, serData);

                                        if(data2){
                                            fprintf(tmpfdes, "%s\n%s\n", instance->keyValue, data2);
                                            os_free(data2);
                                        }
                                        sd_serializedDataFree(serData);
                                        c_free(message);
                                    }
                                }
                                message = (v_message)c_iterTakeFirst(instance->messages);
                            }

                            persistentInstanceFree(instance);
                            instance = d_tableTake(instances);
                        }
                        d_tableFree(instances);
                        sd_serializerFree(serializer);
                    } else {
                        result = D_STORE_RESULT_OUT_OF_RESOURCES;
                    }

                    if (optimize)
                    {
                        fprintf(tmpfdes, D_STORE_END_STRING);
                        closeFile(persistentStore, tmpfdes);
                        renameSuccess = renameTempToActual(persistentStore, partition, topic);

                        if(renameSuccess == TRUE && optimize){
                            optimizeTime = os_timeGet();
                            result = setOptimizeTime(persistentStore, partition, topic, optimizeTime);
                        } else {
                            result = D_STORE_RESULT_IO_ERROR;
                        }
                    }
                }
            } else {
                result = D_STORE_RESULT_OUT_OF_RESOURCES;
            }
        }
        d_free(data);
    } else if(len == -1) {
        d_storeReport(d_store(persistentStore), D_LEVEL_FINE, "No data exists yet on disk for group %s.%s\n", partition, topic);
        result = D_STORE_RESULT_OK;
    }
    return result;
}

/* PRE: The v_group exists */
static d_storeResult
d_storeXMLInjectTopicXML(
    d_storeXML        persistentStore,
    v_group           group,
    d_partition       partition,
    d_topic           topic )
{
    c_type            type;
    c_bool            check;
    d_storeResult     result;
    c_char*           keys;
    c_bool            optimized;
    c_ulong           dataVersion;

    optimized = isOptimized(persistentStore, partition, topic);

    result = getDataVersion(persistentStore, partition, topic, &dataVersion);

    if(result == D_STORE_RESULT_OK){
       /*if(optimized == FALSE){*/
        type = v_topicMessageType(group->topic);
        keys = v_topicKeyExpr(group->topic);

        if(!keys){
            check = metaDataIsCorrect(type, persistentStore, topic, partition,
                "", v_topicQosRef(group->topic));
        } else {
            check = metaDataIsCorrect(type, persistentStore, topic, partition,
                keys, v_topicQosRef(group->topic));
        }
        if (check == FALSE) {
            d_storeReport(d_store(persistentStore), D_LEVEL_SEVERE, RR_META_DATA_MISMATCH, topic);
            OS_REPORT_1(OS_ERROR, STORE_READ_TOPIC_XML, 0, RR_META_DATA_MISMATCH, topic);
            result = D_STORE_RESULT_METADATA_MISMATCH;
            /* metaData is not OK: should the maintainer remove topic and meta data ?? */
        } else {
            /* Inject XML and optimize if necessary */
            result = d_storeXMLOptimizeGroup(persistentStore, group, partition, topic, TRUE, !optimized, dataVersion);
        }
    } else {
        d_storeReport(d_store(persistentStore), D_LEVEL_SEVERE, "Unable to resolve persistent data version.");
        OS_REPORT(OS_ERROR, STORE_READ_TOPIC_XML, 0, "Unable to resolve persistent data version.");
    }

    return result;
/* QAC EXPECT 5102; too many local variables */
}

static void
d_storeXMLInitGroups(
    d_storeXML store)
{
    forAllDirectoryEntries(store, d_store(store)->config->persistentStoreDirectory,
                            NULL, processPartition, TRUE);
}

static d_storeResult
d_storeGetQualityXMLUnlocked(
    const d_groupList list,
    const d_nameSpace nameSpace,
    d_quality* quality)
{
    d_groupList groupList;
    c_bool isIn;

    quality->seconds = 0;
    quality->nanoseconds = 0;

    groupList = list;

    while(groupList) {
        isIn = d_nameSpaceIsIn(nameSpace, groupList->partition, groupList->topic);

        if(isIn == TRUE){
            if(groupList->quality.seconds > quality->seconds){
                quality->seconds = groupList->quality.seconds;
                quality->nanoseconds = groupList->quality.nanoseconds;
            } else if(groupList->quality.seconds == quality->seconds){
                if(groupList->quality.nanoseconds > quality->nanoseconds){
                    quality->seconds = groupList->quality.seconds;
                    quality->nanoseconds = groupList->quality.nanoseconds;
                }
            }
        }
        groupList = d_groupList(groupList->next);
    }
    return D_STORE_RESULT_OK;
}

static void
correctGroupQuality(
    d_nameSpace nameSpace,
    c_voidp userData)
{
    d_groupList list, next;
    c_bool isIn;
    d_quality quality;

    list = d_groupList(userData);

    next = list;

    while(next){
        isIn = d_nameSpaceIsIn(nameSpace, next->partition, next->topic);

        if(isIn == TRUE){
            d_storeGetQualityXMLUnlocked(list, nameSpace, &quality);

            if(quality.seconds > next->quality.seconds){
                next->quality.seconds = quality.seconds;
                next->quality.nanoseconds = quality.nanoseconds;
            } else if(quality.seconds == next->quality.seconds){
                if(quality.nanoseconds > next->quality.nanoseconds){
                    next->quality.seconds = quality.seconds;
                    next->quality.nanoseconds = quality.nanoseconds;
                }
            }
        }
        next = next->next;
    }
}

d_storeXML
d_storeNewXML(u_participant participant)
{
    d_storeXML storeXML;
    d_store store;
    os_mutexAttr mutexAttr;
    os_result resultInit;

    OS_UNUSED_ARG(participant);

    storeXML = d_storeXML(os_malloc(C_SIZEOF(d_storeXML)));
    store = d_store(storeXML);
    d_storeInit(store, d_storeDeinitXML);

    storeXML->diskStorePath         = NULL;
    storeXML->maxPathLen            = 0;
    storeXML->groups                = NULL;
    storeXML->opened                = FALSE;
    storeXML->sessionAlive          = FALSE;
    storeXML->openedFiles           = NULL;
    storeXML->dummyFile             = d_storeFileNew("", NULL, NULL);
    os_free(storeXML->dummyFile->path);
    storeXML->dummyFile->path       = NULL;
    storeXML->dataBuffer            = (c_char *)os_malloc((os_uint32)MAX_MESSAGE_SIZE);
    assert(storeXML->dataBuffer);
    storeXML->keyBuffer             = (c_char *)os_malloc((os_uint32)MAX_KEY_SIZE);
    assert(storeXML->keyBuffer);
    resultInit = os_mutexAttrInit(&mutexAttr);
    if(resultInit == os_resultSuccess)
    {
        mutexAttr.scopeAttr = OS_SCOPE_PRIVATE;
        resultInit = os_mutexInit(&(storeXML->mutex), &mutexAttr);
        if(resultInit != os_resultSuccess)
        {
            OS_REPORT(OS_ERROR, "durability", 0, "Failed to init mutex for XML store.");
        }
    } else
    {
        OS_REPORT(OS_ERROR, "durability", 0, "Failed to init mutex attributes for XML store.");
    }
    store->openFunc                 = d_storeOpenXML;
    store->closeFunc                = d_storeCloseXML;
    store->groupsReadFunc           = d_storeGroupsReadXML;
    store->groupListFreeFunc        = NULL;
    store->groupInjectFunc          = d_storeGroupInjectXML;
    store->groupStoreFunc           = d_storeGroupStoreXML;
    store->getQualityFunc           = d_storeGetQualityXML;
    store->backupFunc               = d_storeBackupXML;
    store->restoreBackupFunc        = d_storeRestoreBackupXML;
    store->actionStartFunc          = d_storeActionStartXML;
    store->actionStopFunc           = d_storeActionStopXML;
    store->messageStoreFunc         = d_storeMessageStoreXML;
    store->instanceDisposeFunc       = d_storeInstanceDisposeXML;
    store->instanceExpungeFunc      = d_storeInstanceExpungeXML;
    store->messageExpungeFunc       = d_storeMessageExpungeXML;
    store->deleteHistoricalDataFunc = d_storeDeleteHistoricalDataXML;
    store->messagesInjectFunc       = d_storeMessagesInjectXML;
    store->instanceRegisterFunc        = d_storeInstanceRegisterXML;
    store->createPersistentSnapshotFunc   = d_storeCreatePersistentSnapshotXML;
    store->instanceUnregisterFunc   = d_storeInstanceUnregisterXML;
    store->optimizeGroupFunc        = d_storeOptimizeGroupXML;
    store->nsIsCompleteFunc            = d_storeNsIsCompleteXML;
    store->nsMarkCompleteFunc        = d_storeNsMarkCompleteXML;

    return storeXML;
}

void
d_storeDeinitXML(
    d_object object)
{
    d_storeXML store;

    assert(d_objectIsValid(d_object(object), D_STORE));
    assert(d_store(object)->type == D_STORE_TYPE_XML);

    if(object){
        store = d_storeXML(object);
        d_storeFileFree(store->dummyFile);
        os_free(store->dataBuffer);
        os_free(store->keyBuffer);
        os_mutexDestroy (&(store->mutex));
    }
}

d_storeResult
d_storeFreeXML(
    d_storeXML store)
{
    d_storeResult result;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(d_store(store)->type == D_STORE_TYPE_XML);

    if(store) {
        d_lockLock(d_lock(store));

        if(store->opened == TRUE) {
            d_lockUnlock(d_lock(store));
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else {
            d_lockUnlock(d_lock(store));

            d_storeFree(d_store(store));
            result = D_STORE_RESULT_OK;
        }
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeOpenXML(
    d_store store)
{
    d_storeResult result;
    d_storeXML storeXML;
    c_char* resultDir;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(store->type == D_STORE_TYPE_XML);

    if(store != NULL){
        storeXML = d_storeXML(store);
        d_lockLock(d_lock(store));

        if(storeXML->opened == TRUE) {
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else if(store->config->persistentStoreDirectory == NULL ){
            result = D_STORE_RESULT_ILL_PARAM;
        } else {
            resultDir = d_storeDirNew(store, store->config->persistentStoreDirectory);

            if(resultDir){
                os_free(store->config->persistentStoreDirectory);
                store->config->persistentStoreDirectory = resultDir;
                d_storeReport(store, D_LEVEL_INFO,
                                "Persistent store directory '%s' openened.\n",
                                store->config->persistentStoreDirectory);
                storeXML->opened = TRUE;
                result = D_STORE_RESULT_OK;
            } else {
                d_storeReport(store, D_LEVEL_SEVERE,
                                "Persistent store directory '%s' could not be created.\n",
                                store->config->persistentStoreDirectory);
                result = D_STORE_RESULT_PRECONDITION_NOT_MET;
            }
        }
        if(result == D_STORE_RESULT_OK) {
            storeXML->maxPathLen    = (c_ulong)D_PERSISTENT_STORE_DIR_SIZE +
                                        (c_ulong)strlen(store->config->persistentStoreDirectory);

            storeXML->diskStorePath = (c_char *)os_malloc((os_uint32)
                                        ((c_long)strlen(store->config->persistentStoreDirectory) + 1));
            os_strncpy(storeXML->diskStorePath, store->config->persistentStoreDirectory,
                        (os_uint32)((c_long)strlen(store->config->persistentStoreDirectory)+1));
            d_storeXMLInitGroups(storeXML);

            /* Walk namespaces in administration */
            d_adminNameSpaceWalk (store->admin, correctGroupQuality, storeXML->groups);
        }

        d_lockUnlock(d_lock(store));
    } else {
        d_storeReport(store, D_LEVEL_SEVERE, "Supplied parameter(s) not valid.\n");
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeCloseXML(
    d_store store)
{
    d_storeResult result;
    d_groupList groupList, next;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(store->type == D_STORE_TYPE_XML);

    if(store != NULL){
        d_lockLock(d_lock(store));
        if(d_storeXML(store)->opened == FALSE) {
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else {
            if(d_storeXML(store)->sessionAlive == TRUE){
                actionStopUnlocked(d_storeXML(store));
            }
            d_storeXML(store)->opened = FALSE;
            os_free(d_storeXML(store)->diskStorePath);
            d_storeXML(store)->diskStorePath = NULL;

            groupList = d_storeXML(store)->groups;

            while(groupList){
                next = groupList->next;
                os_free(groupList->partition);
                os_free(groupList->topic);
                os_free(groupList);
                groupList = next;
            }
            result = D_STORE_RESULT_OK;
        }
        d_lockUnlock(d_lock(store));
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeActionStartXML(
    const d_store store)
{
    d_storeResult result;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(store->type == D_STORE_TYPE_XML);

    if(store != NULL){
        os_mutexLock(&(d_storeXML(store)->mutex));
        d_lockLock(d_lock(store));
        if(d_storeXML(store)->opened == FALSE) {
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else {
            d_storeXML(store)->sessionAlive = TRUE;

            d_storeXML(store)->openedFiles = d_tableNew(d_storeFileCompare, d_storeFileFree);
            d_storeXML(store)->expungeActions = d_tableNew(groupExpungeActionsCompare, groupExpungeActionsFree);
            result = D_STORE_RESULT_OK;
        }
        d_lockUnlock(d_lock(store));
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeActionStopXML(
    const d_store store)
{
    d_storeResult result;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(store->type == D_STORE_TYPE_XML);
    if(store != NULL){
        d_lockLock(d_lock(store));
        if(d_storeXML(store)->opened == FALSE) {
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else {
            if(d_storeXML(store)->sessionAlive == TRUE){
                result = actionStopUnlocked(d_storeXML(store));
                processGroupExpungeActions(d_storeXML(store));
                d_tableFree(d_storeXML(store)->expungeActions);
            } else {
                result = D_STORE_RESULT_OK;
            }
        }
        d_lockUnlock(d_lock(store));
        os_mutexUnlock(&(d_storeXML(store)->mutex));
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeGroupsReadXML(
    const d_store store,
    d_groupList *list)
{
    d_storeResult result;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(store->type == D_STORE_TYPE_XML);

    if(store != NULL){
        d_lockLock(d_lock(store));
        result = groupsReadXMLUnsafe(store, list);
        d_lockUnlock(d_lock(store));
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }

    return result;
}

d_storeResult
groupsReadXMLUnsafe(
    const d_store store,
    d_groupList *list)
{
    d_storeResult result;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(store->type == D_STORE_TYPE_XML);

    if(store)
    {
        if(d_storeXML(store)->opened == FALSE)
        {
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else if(list == NULL)
        {
            result = D_STORE_RESULT_ILL_PARAM;
        } else
        {
            *list = d_storeXML(store)->groups;
            result = D_STORE_RESULT_OK;
        }
    } else
    {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeGroupInjectXML(
    const d_store store,
    const c_char* partition,
    const c_char* topic,
    const u_participant participant,
    d_group *group)
{
    d_groupList groupList;
    d_storeResult result;
    d_topicMetadata metadata;
    u_topic utopic;
    u_group ugroup;
    u_partition upartition;
    v_partitionQos partitionQos;
    v_duration timeout;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(store->type == D_STORE_TYPE_XML);

    *group = NULL;
    result = D_STORE_RESULT_ERROR;

    if(store != NULL){
        d_lockLock(d_lock(store));

        if(d_storeXML(store)->opened == FALSE) {
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else if((participant == NULL) || (partition == NULL) || (topic == NULL) || (group == NULL)) {
            result = D_STORE_RESULT_ILL_PARAM;
        } else {
            groupList = getPartitionTopic(d_storeXML(store), partition, topic);

            if(groupList) {
                metadata = readTopicMetadata(d_storeXML(store), participant, partition, topic);

                if(metadata) {
                    utopic = u_topicNew(participant, metadata->name,
                                        metadata->typeName,
                                        metadata->keyList, metadata->qos);
                    if(utopic) {
                        d_storeReport(store, D_LEVEL_FINE, "Topic %s created.\n", metadata->name);
                        partitionQos = u_partitionQosNew(NULL);

                        if(partitionQos) {
                            d_storeReport(store, D_LEVEL_FINE, "PartitionQoS created.\n");
                            upartition = u_partitionNew(participant, partition, partitionQos);

                            if(upartition) {
                                d_storeReport(store, D_LEVEL_FINE, "Partition %s created.\n", partition);
                                timeout.seconds = 0;
                                timeout.nanoseconds = 0;

                                ugroup = u_groupNew(participant, partition, topic, timeout);

                                if(ugroup) {
                                    d_storeReport(store, D_LEVEL_INFO, "Group %s.%s created.\n", partition, metadata->name);
                                    *group = d_groupNew(partition, topic,
                                                        D_DURABILITY_PERSISTENT,
                                                        groupList->completeness,
                                                        groupList->quality);
                                    u_entityAction(u_entity(ugroup), setKernelGroup, *group);
                                    u_entityFree(u_entity(ugroup));
                                    result = D_STORE_RESULT_OK;
                                } else {
                                    d_storeReport(store, D_LEVEL_SEVERE, "Group %s.%s could NOT be created.\n", partition, metadata->name);
                                }
                                u_partitionFree(upartition);
                            }  else {
                                d_storeReport(store, D_LEVEL_SEVERE, "Partition %s could NOT be created.\n", partition);
                            }
                            u_partitionQosFree(partitionQos);
                        }  else {
                            d_storeReport(store, D_LEVEL_SEVERE, "PartitionQos could NOT be created.\n");
                        }
                        (void)u_topicFree(utopic);
                    } else {
                        d_storeReport(store, D_LEVEL_SEVERE,
                            "Topic '%s' with typeName '%s' and keyList '%s' could NOT be created.\n",
                            metadata->name, metadata->typeName, metadata->keyList);
                        result = D_STORE_RESULT_METADATA_MISMATCH;
                    }
                    os_free(metadata->typeName);
                    os_free(metadata->name);
                    os_free(metadata->keyList);
                    v_topicQosFree(metadata->qos);
                    c_free(metadata->type);
                    os_free(metadata);
                } else {
                    result = D_STORE_RESULT_MUTILATED;
                }
            }
        }
        d_lockUnlock(d_lock(store));
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeGetQualityXML(
    const d_store store,
    const d_nameSpace nameSpace,
    d_quality* quality)
{
    d_storeResult result;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(store->type == D_STORE_TYPE_XML);

    if(store != NULL){
        d_lockLock(d_lock(store));

        if(d_storeXML(store)->opened == FALSE) {
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else if((quality == NULL) || (nameSpace == NULL)) {
            result = D_STORE_RESULT_ILL_PARAM;
        } else {
            result = d_storeGetQualityXMLUnlocked(d_storeXML(store)->groups, nameSpace, quality);
        }
        d_lockUnlock(d_lock(store));
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeGroupStoreXML(
    const d_store store,
    const d_group dgroup)
{
    c_char *    topicName = NULL;
    c_char *    partitionName = NULL;
    os_time     curTime;
    c_bool      storeMetaSuccess;
    v_group     group;
    d_storeResult result;
    d_quality   quality;
    d_storeXML  persistentStore;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(store->type == D_STORE_TYPE_XML);

    if(store != NULL) {
        persistentStore = d_storeXML(store);
        d_lockLock(d_lock(persistentStore));

        if(persistentStore->opened == FALSE) {
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else if(dgroup == NULL) {
            result = D_STORE_RESULT_ILL_PARAM;
        } else {
            group = d_groupGetKernelGroup(dgroup);
            topicName     = v_topicName(v_groupTopic(group));
            partitionName = v_partitionName(v_groupPartition(group));
            d_storeReport(store, D_LEVEL_FINE,
                          "Storing group (partition.topic): %s.%s\n",
                          partitionName, topicName);

            createDirectoryIfNecessary(d_storeXML(store), partitionName);
            storeMetaSuccess = storeTopicMetadata(d_storeXML(store), group);

            if(storeMetaSuccess == TRUE) {
                curTime = os_timeGet();
                quality.seconds = curTime.tv_sec;
                quality.nanoseconds = curTime.tv_nsec;
                setPartitionTopicQuality(persistentStore,
                                         partitionName, topicName,
                                         quality, D_GROUP_INCOMPLETE);
                result = D_STORE_RESULT_OK;
            } else {
                d_storeReport(d_store(persistentStore),
                              D_LEVEL_SEVERE,
                              RR_COULD_NOT_WRITE_META_DATA,
                              topicName);
                OS_REPORT_1(OS_ERROR,
                            STORE_STORE_TOPIC_XML, 0,
                            RR_COULD_NOT_WRITE_META_DATA,
                            topicName);
                result = D_STORE_RESULT_PRECONDITION_NOT_MET;
            }
            c_free(group);
        }
        d_lockUnlock(d_lock(persistentStore));
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}


d_storeResult
d_storeBackupXML(
    const d_store store,
    const d_nameSpace nameSpace)
{
    d_storeResult result;
    d_storeXML    persistentStore;
    d_groupList   groupList;
    c_char*       fileStorePath;
    c_char*       backupStorePath;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(store->type == D_STORE_TYPE_XML);

    if(store != NULL) {
        persistentStore = d_storeXML(store);
        d_lockLock(d_lock(persistentStore));

        if(persistentStore->opened == FALSE) {
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else if(nameSpace == NULL) {
            result = D_STORE_RESULT_ILL_PARAM;
        } else {
            groupList = persistentStore->groups;

            while(groupList) {
                if(d_nameSpaceIsIn(nameSpace, groupList->partition, groupList->topic)) {
                    fileStorePath = getDataFileName(persistentStore, groupList->partition, groupList->topic);
                    backupStorePath = getBakFileName(persistentStore, groupList->partition, groupList->topic);

                    if (os_rename(fileStorePath, backupStorePath) == os_resultFail)
                    {
                        result = D_STORE_RESULT_IO_ERROR;
                    }

                    os_free(fileStorePath);
                    os_free(backupStorePath);
                }
                groupList = groupList->next;
            }
            result = D_STORE_RESULT_OK;
        }
        d_lockUnlock(d_lock(persistentStore));
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

/* Restore backed up files */
d_storeResult
d_storeRestoreBackupXML (
        const d_store store,
        const d_nameSpace nameSpace)
{
    d_storeResult result;
    d_storeXML    persistentStore;
    d_groupList   groupList;
    c_char*       fileStorePath;
    c_char*       backupStorePath;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(store->type == D_STORE_TYPE_XML);

    if(store != NULL) {
        persistentStore = d_storeXML(store);
        d_lockLock(d_lock(persistentStore));

        if(persistentStore->opened == FALSE) {
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else if(nameSpace == NULL) {
            result = D_STORE_RESULT_ILL_PARAM;
        } else {
            groupList = persistentStore->groups;

            result = D_STORE_RESULT_OK;

            while(groupList) {
                if(d_nameSpaceIsIn(nameSpace, groupList->partition, groupList->topic)) {
                    fileStorePath = getDataFileName(persistentStore, groupList->partition, groupList->topic);
                    backupStorePath = getBakFileName(persistentStore, groupList->partition, groupList->topic);

                    if (os_rename(backupStorePath, fileStorePath) == os_resultFail)
                    {
                        result = D_STORE_RESULT_IO_ERROR;
                    }

                    os_free(fileStorePath);
                    os_free(backupStorePath);
                }
                /* Reset quality */
                groupList->quality.seconds = 0;
                groupList->quality.nanoseconds = 0;
                groupList = groupList->next;
            }

            /* Re-determine quality of groups */
            d_storeXMLInitGroups(persistentStore);

            /* Walk namespaces in administration */
            d_adminNameSpaceWalk (store->admin, correctGroupQuality, persistentStore->groups);
        }
        d_lockUnlock(d_lock(persistentStore));
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeMessageStoreXML(
    const d_store store,
    const v_groupAction msg)
{
    d_storeResult    result;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(store->type == D_STORE_TYPE_XML);

    if(store != NULL) {
        d_lockLock(d_lock(store));

        if(d_storeXML(store)->opened == FALSE) {
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else if((msg == NULL) || (msg->group == NULL) || (msg->message == NULL)) {
            result = D_STORE_RESULT_ILL_PARAM;
        } else {
            result = appendMessage(d_storeXML(store), msg);
        }
        d_lockUnlock(d_lock(store));
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeInstanceDisposeXML(
    const d_store store,
    const v_groupAction msg)
{
    d_storeResult        result;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(store->type == D_STORE_TYPE_XML);

    if(store != NULL) {
        d_lockLock(d_lock(store));

        if(d_storeXML(store)->opened == FALSE) {
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else if((msg == NULL) || (msg->group == NULL) || (msg->message == NULL)) {
            result = D_STORE_RESULT_ILL_PARAM;
        } else {
            result = appendMessage(d_storeXML(store), msg);
        }
        d_lockUnlock(d_lock(store));
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeInstanceExpungeXML(
    const d_store store,
    const v_groupAction msg)
{
    d_storeResult        result;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(store->type == D_STORE_TYPE_XML);

    if(store != NULL) {
        d_lockLock(d_lock(store));

        if(d_storeXML(store)->opened == FALSE) {
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else if((msg == NULL) || (msg->group == NULL) || (msg->message == NULL)) {
            result = D_STORE_RESULT_ILL_PARAM;
        } else {
            result = expungeMessageOptimized(d_storeXML(store), msg, FALSE);
        }
        d_lockUnlock(d_lock(store));
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeMessageExpungeXML(
    const d_store store,
    const v_groupAction msg)
{
    d_storeResult        result;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(store->type == D_STORE_TYPE_XML);

    if(store != NULL) {
        d_lockLock(d_lock(store));

        if(d_storeXML(store)->opened == FALSE) {
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else if((msg == NULL) || (msg->group == NULL)) {
            result = D_STORE_RESULT_ILL_PARAM;
        } else {
            if(msg->kind == V_GROUP_ACTION_DELETE_DATA){
                result = deleteHistoricalData(d_storeXML(store), msg);
            } else {
                result = expungeMessageOptimized(d_storeXML(store), msg, TRUE);
            }
        }
        d_lockUnlock(d_lock(store));
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeDeleteHistoricalDataXML(
    const d_store store,
    const v_groupAction msg)
{
    d_storeResult        result;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(store->type == D_STORE_TYPE_XML);

    if(store != NULL) {
        d_lockLock(d_lock(store));

        if(d_storeXML(store)->opened == FALSE) {
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else if((msg == NULL) || (msg->group == NULL)) {
            result = D_STORE_RESULT_ILL_PARAM;
        } else {
            result = deleteHistoricalData(d_storeXML(store), msg);
        }
        d_lockUnlock(d_lock(store));
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeInstanceRegisterXML(
    const d_store store,
    const v_groupAction msg)
{
    d_storeResult    result;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(store->type == D_STORE_TYPE_XML);

    if(store != NULL) {
        d_lockLock(d_lock(store));

        if(d_storeXML(store)->opened == FALSE) {
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else if((msg == NULL) || (msg->group == NULL) || (msg->message == NULL)) {
            result = D_STORE_RESULT_ILL_PARAM;
        } else {
            result = appendMessage(d_storeXML(store), msg);
        }
        d_lockUnlock(d_lock(store));
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeCreatePersistentSnapshotXML(
    const d_store store,
    const c_char* partitionExpr,
    const c_char* topicExpr,
    const c_char* uri)
{
    d_storeResult result;
    c_bool match;
    os_char* fileStorePath;
    os_char* destStorePath;
    d_groupList listIter;
    c_ulong length;
    c_ulong i;
    d_nameSpace nameSpace;
    d_durabilityKind dkind;
    c_iter nameSpaces;

    assert(store);
    assert(topicExpr);
    assert(partitionExpr);
    assert(uri);
    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(store->type == D_STORE_TYPE_XML);

    if(store)
    {
        os_mutexLock(&(d_storeXML(store)->mutex));
        d_lockLock(d_lock(store));
        if(d_storeXML(store)->opened == FALSE)
        {
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else
        {
            result = groupsReadXMLUnsafe(store, &listIter);
            while(result == D_STORE_RESULT_OK && listIter)
            {
                match = d_patternMatch(listIter->partition, (c_string)partitionExpr);
                if(match)
                {
                    match = d_patternMatch(listIter->topic, (c_string)topicExpr);
                    if(match)
                    {
                        c_char* tmp;
                        tmp = d_storeDirNew(store, uri);
                        if(tmp)
                        {
                            os_free(tmp);
                            createDirectoryIfNecessaryForStoreDir(uri, listIter->partition);
                            /* Copy data file */
                            fileStorePath = getDataFileName(d_storeXML(store), listIter->partition, listIter->topic);
                            destStorePath = getDataFileNameBasedOnPath(uri, listIter->partition, listIter->topic);
                            result = d_storeCopyFile(fileStorePath, destStorePath);
                            os_free(fileStorePath);
                            os_free(destStorePath);
                            if(result == D_STORE_RESULT_OK)
                            {
                                /* Copy meta file */
                                fileStorePath = getMetaFileName(d_storeXML(store), listIter->partition, listIter->topic);
                                destStorePath = getMetaFileNameBasedOnPath(uri, listIter->partition, listIter->topic);
                                result = d_storeCopyFile(fileStorePath, destStorePath);
                                os_free(fileStorePath);
                                os_free(destStorePath);
                                /* Store the complete file for the namespace */
                                if(result == D_STORE_RESULT_OK)
                                {
                                    /* Collect namespaces */
                                    nameSpaces = d_adminNameSpaceCollect(store->admin);
                                    length = c_iterLength(nameSpaces);
                                    for(i = 0; i < length; i++)
                                    {
                                        nameSpace = d_nameSpace(c_iterObject(nameSpaces, i));
                                        dkind = d_nameSpaceGetDurabilityKind(nameSpace);
                                        if(dkind == D_DURABILITY_PERSISTENT || dkind == D_DURABILITY_ALL)
                                        {
                                            if(d_nameSpaceIsIn(nameSpace, listIter->partition, listIter->topic))
                                            {
                                                /* always mark the namespace as complete for a snapshot */
                                                result = d_storeNsMarkCompleteXMLBasedOnPath (
                                                    uri,
                                                    nameSpace,
                                                    TRUE);
                                            }
                                        }
                                    }

                                    /* Free namespace list */
                                    d_adminNameSpaceCollectFree(store->admin, nameSpaces);
                                }
                            }
                        } else
                        {
                            result = D_STORE_RESULT_IO_ERROR;
                        }
                    }
                }
                listIter = d_groupList(listIter->next);
            }
        }
        d_lockUnlock(d_lock(store));
        os_mutexUnlock(&(d_storeXML(store)->mutex));
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeInstanceUnregisterXML(
    const d_store store,
    const v_groupAction msg)
{
    d_storeResult    result;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(store->type == D_STORE_TYPE_XML);

    if(store != NULL) {
        d_lockLock(d_lock(store));

        if(d_storeXML(store)->opened == FALSE) {
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else if((msg == NULL) || (msg->group == NULL) || (msg->message == NULL)) {
            result = D_STORE_RESULT_ILL_PARAM;
        } else {
            result = appendMessage(d_storeXML(store), msg);
        }
        d_lockUnlock(d_lock(store));
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeMessagesInjectXML(
    const d_store store,
    const d_group group)
{
    d_storeResult result;
    v_group       vgroup;
    d_partition   partition;
    d_topic       topic;
    c_char        *fileStorePath;
    d_storeXML    persistentStore;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(store->type == D_STORE_TYPE_XML);

    if(store != NULL) {
        d_lockLock(d_lock(store));

        if(d_storeXML(store)->opened == FALSE) {
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else if(group == NULL) {
            result = D_STORE_RESULT_ILL_PARAM;
        } else {
            persistentStore = d_storeXML(store);
            vgroup    = d_groupGetKernelGroup(group);

            if(vgroup){
                topic     = v_topicName(v_groupTopic(vgroup));
                partition = v_partitionName(v_groupPartition(vgroup));

                result = d_storeXMLInjectTopicXML(persistentStore,
                                                  vgroup,
                                                  partition, topic);

                if(result != D_STORE_RESULT_OK){
                    OS_REPORT_3(OS_ERROR, D_CONTEXT, 0,
                            "Unable to insert persistent data from disk for group '%s.%s'. Reason: '%d'. Removing data for this group...",
                            partition, topic, result);
                    d_storeReport(store,
                            D_LEVEL_SEVERE,
                            "Unable to insert persistent data from disk for group '%s.%s'. Reason: '%d'. Removing data for this group...",
                          partition, topic, result);
                    fileStorePath = getDataFileName(persistentStore, partition, topic);
                    os_remove(fileStorePath);
                    os_free(fileStorePath);
                }

                c_free(vgroup);
            } else {
                result = D_STORE_RESULT_ILL_PARAM;
            }
        }
        d_lockUnlock(d_lock(store));
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeOptimizeGroupXML(
    const d_store store,
    const d_group group)
{
    d_storeResult result;
    v_group       vgroup;
    d_partition   partition;
    d_topic       topic;
    c_bool        isOptimal;
    c_ulong       dataVersion;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(store->type == D_STORE_TYPE_XML);

    if(store != NULL) {
        d_lockLock(d_lock(store));

        if(d_storeXML(store)->opened == FALSE) {
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else if(group == NULL) {
            result = D_STORE_RESULT_ILL_PARAM;
        } else {
            vgroup    = d_groupGetKernelGroup(group);

            if(vgroup){
                topic     = v_topicName(v_groupTopic(vgroup));
                partition = v_partitionName(v_groupPartition(vgroup));

                isOptimal = isOptimized(d_storeXML(store), partition, topic);

                result = getDataVersion(d_storeXML(store), partition, topic, &dataVersion);

                if(result == D_STORE_RESULT_OK){
                    if((isOptimal == FALSE) || (dataVersion < D_STORE_VERSION)){
                        result = d_storeXMLOptimizeGroup(d_storeXML(store),
                                                         vgroup,
                                                         partition, topic,
                                                         FALSE,
                                                         TRUE,
                                                         dataVersion);
                    } else {
                        result = D_STORE_RESULT_OK;
                    }
                }
                c_free(vgroup);
            } else {
                result = D_STORE_RESULT_ILL_PARAM;
            }
        }
        d_lockUnlock(d_lock(store));
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}


/* Return filename for namespace complete indicator file */
static
d_storeResult
d_storeNsCompleteFileName(
    const char* storeDir,
    const d_nameSpace nameSpace,
    char nameBuff[])
{
    d_storeResult result;

    if (storeDir && nameSpace)
    {
        nameBuff[0] = '\0';

        /* Copy path of persistent store to buffer */
        strcat (nameBuff, storeDir);

        /* Copy '/' character */
        strcat (nameBuff, os_fileSep());

        /* Copy namespace name */
        strcat (nameBuff, d_nameSpaceGetName(nameSpace));

        /* Copy postfix for completefile */
        strcat (nameBuff, "_complete");

        result = D_STORE_RESULT_OK;
    }else
    {
        result = D_STORE_RESULT_ILL_PARAM;
    }

    return result;
}

/* Check if namespace complete indicator file exists */
static
c_bool
d_storeNsCompleteFileExists (
    const char completeFile[])
{
    c_bool result;
    os_result osResult;
    struct os_stat buf;

    result = FALSE;

    osResult = os_stat (completeFile, &buf);
    if (osResult == os_resultSuccess)
    {
        result = TRUE;
    }

    return result;
}

/* Create or remove namespace completefile */
static
d_storeResult
d_storeNsMarkComplete_w_name (
    const char completeFile[],
    c_bool isComplete)
{
    d_storeResult result;
    c_bool completeFileExists;
    FILE* hFile;

    result = D_STORE_RESULT_OK;

    completeFileExists = d_storeNsCompleteFileExists (completeFile);

    /* Remove existing complete file if namespace is incomplete, otherwise create it */
    if (isComplete)
    {
        if (!completeFileExists)
        {
            hFile = fopen (completeFile, "w");
            if (!hFile)
            {
                result = D_STORE_RESULT_IO_ERROR;
            }else
            {
                fclose (hFile);
            }
        }
    }else
    {
        if (completeFileExists && (os_remove (completeFile) == os_resultFail))
        {
            result = D_STORE_RESULT_IO_ERROR;
        }
    }

    return result;
}

/* Check if namespace is complete */
d_storeResult
d_storeNsIsCompleteXML (
    const d_store store,
    const d_nameSpace nameSpace,
    c_bool* isComplete)
{
    char completeFile[OS_PATH_MAX];
    d_storeResult result;

    if(store)
    {
        d_lockLock(d_lock(store));

        if (isComplete)
        {
            *isComplete = FALSE;

            /* Get completeFile name */
            result = d_storeNsCompleteFileName (
                store->config->persistentStoreDirectory,
                nameSpace,
                completeFile);
            if (result == D_STORE_RESULT_OK)
            {
                *isComplete = d_storeNsCompleteFileExists (completeFile);
            }
        }else
        {
            result = D_STORE_RESULT_ILL_PARAM;
        }
        d_lockUnlock(d_lock(store));
    } else
    {
        result = D_STORE_RESULT_ILL_PARAM;
    }

    return result;
}

d_storeResult
d_storeNsMarkCompleteXMLBasedOnPath(
    const c_char* storeDir,
    const d_nameSpace nameSpace,
    c_bool isComplete)
{
    d_storeResult result;
    char completeFile[OS_PATH_MAX];

    /* Get completeFile name */
    result = d_storeNsCompleteFileName (storeDir, nameSpace, completeFile);
    if (result == D_STORE_RESULT_OK)
    {
        result = d_storeNsMarkComplete_w_name (completeFile, isComplete);
    }
    return result;
}

/* Mark namespace incomplete or complete */
d_storeResult
d_storeNsMarkCompleteXML (
    const d_store store,
    const d_nameSpace nameSpace,
    c_bool isComplete)
{
    d_storeResult result;

    if(store)
    {
        d_lockLock(d_lock(store));
        result = d_storeNsMarkCompleteXMLBasedOnPath(
            store->config->persistentStoreDirectory,
            nameSpace,
            isComplete);
        d_lockUnlock(d_lock(store));
    } else
    {
        result = D_STORE_RESULT_ILL_PARAM;
    }

    return result;
}

static d_storeResult
getDataVersion(
    const d_storeXML persistentStore,
    const c_char* partition,
    const c_char* topic,
    c_ulong* version)
{
    c_long status;
    FILE *fdes;
    c_char *data, *dataFileName;
    d_storeResult result;
    int dataVersionFound, len;

    dataFileName = getDataFileName(persistentStore, partition, topic);

    if(dataFileName){
        fdes = fopen(dataFileName, "r");

        if (fdes) {
            len = 4096;
            data = (c_char *)d_malloc((os_uint32)len, "readData");

            if (data == NULL) {
                result = D_STORE_RESULT_OUT_OF_RESOURCES;
                d_storeReport(d_store(persistentStore), D_LEVEL_SEVERE, "No more resources\n");
                OS_REPORT(OS_ERROR, D_CONTEXT, 0, "No more resources available\n");
                result = D_STORE_RESULT_OUT_OF_RESOURCES;
            } else {
                data[0] = '\0';
                readLine(fdes, len, data);
                status = strncmp(data, "<TOPIC>", 7);

                if (status != 0) {
                    d_storeReport(d_store(persistentStore), D_LEVEL_SEVERE,
                            RR_TOPIC_SHOULD_BEGIN_WITH_TOPIC_TAG, topic);
                    OS_REPORT_1(OS_ERROR, STORE_READ_TOPIC_XML, 0,
                               RR_TOPIC_SHOULD_BEGIN_WITH_TOPIC_TAG, topic);
                    result = D_STORE_RESULT_MUTILATED;
                } else {
                    dataVersionFound = sscanf(data, "<TOPIC><message version=\"%u\"", version);

                    if(dataVersionFound != 1){
                        *version = 0;
                    }
                    d_storeReport(d_store(persistentStore), D_LEVEL_INFO,
                                "Found store version: %u for group %s.%s\n",
                                version, partition, topic);
                    result = D_STORE_RESULT_OK;
                }
                d_free(data);
            }
            fclose(fdes);
        } else {
            result = D_STORE_RESULT_MUTILATED;
        }
        os_free(dataFileName);
    } else {
        result = D_STORE_RESULT_OUT_OF_RESOURCES;
    }
    return result;
}

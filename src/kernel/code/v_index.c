#include "v__index.h"
#include "os_heap.h"
#include "v__topic.h"
#include "v_state.h"
#include "v_time.h"
#include "v_event.h"
#include "v__dataReader.h"
#include "v_dataReaderEntry.h"
#include "v__dataReader.h"
#include "v_subscriber.h"
#include "v_reader.h"
#include "v_readerQos.h"
#include "v_status.h"
#include "c_stringSupport.h"
#include "v_entity.h"
#include "v_projection.h"
#include "v_dataView.h"
#include "v__dataReaderInstance.h"
#include "v__dataReaderSample.h"
#include "v_public.h"
#include "v_instance.h"
#include "v__statisticsInterface.h"

#define _EXTENT_
#ifdef _EXTENT_
#include "c_extent.h"
#endif

#include "os_report.h"
#include "os.h"

static c_type
sampleTypeNew(
    v_topic topic)
{
    c_metaObject o;
    c_type sampleType,foundType;
    c_base base;
    c_char *name;
    c_long length,sres;

    assert(C_TYPECHECK(topic,v_topic));

    base = c_getBase(topic);

    sampleType = c_type(c_metaDefine(c_metaObject(base),M_CLASS));
    c_class(sampleType)->extends = v_dataReaderSample_t(base);
    o = c_metaDeclare(c_metaObject(sampleType),"message",M_ATTRIBUTE);
    c_property(o)->type = c_keep(v_topicMessageType(topic));
    c_free(o);
    c_metaObject(sampleType)->definedIn = c_keep(base);
    c_metaFinalize(c_metaObject(sampleType));
    if (v_topicName(topic) != NULL) {
#define SAMPLE_FORMAT "v_indexSample<%s>"
#define SAMPLE_NAME   "v_indexSample<>"
        /* sizeof contains \0 */
        length = sizeof(SAMPLE_NAME) + strlen(v_topicName(topic));
        name = os_malloc(length);
        sres = snprintf(name,length,SAMPLE_FORMAT,v_topicName(topic));
        assert(sres == (length-1));
#undef SAMPLE_FORMAT
#undef SAMPLE_NAME
    } else {
        /* not supposed to happen anymore! */
        assert(FALSE);
        length = 21;
        name = os_malloc(length);
        sprintf(name,"v_indexSample<0x"PA_ADDRFMT">",(c_address)topic);
    }
    foundType = c_type(c_metaBind(c_metaObject(base),
                                  name,
                                  c_metaObject(sampleType)));
    os_free(name);
    c_free(sampleType);

    return foundType;
}

static c_type
createInstanceType (
    v_topic topic,
    c_char *keyExpr,
    c_array *keyListRef)
{
    c_metaObject o;
    c_type instanceType, baseType, foundType;
    c_type sampleType, keyType, keyInstanceType;
    c_base base;
    c_char *name;
    c_long length,sres;

    assert(C_TYPECHECK(topic,v_topic));

    foundType = NULL;
    if (keyExpr) {
        keyType = v_topicKeyTypeCreate(topic,keyExpr,keyListRef);
    } else {
        keyExpr = v_topicKeyExpr(topic);
        keyType = v_topicKeyType(topic);
        *keyListRef = c_keep(v_topicMessageKeyList(topic));
    }
    sampleType = sampleTypeNew(topic);
    if (sampleType) {
        base = c_getBase(topic);
        baseType = v_dataReaderInstance_t(base);
        instanceType = c_type(c_metaDefine(c_metaObject(base),M_CLASS));
        if (instanceType != NULL) {
            c_class(instanceType)->extends = c_keep(c_class(baseType));
            o = c_metaDeclare(c_metaObject(instanceType),
                              "sample",M_ATTRIBUTE);
            c_property(o)->type = c_keep(sampleType);
            c_free(o);
            o = c_metaDeclare(c_metaObject(instanceType),
                              "tail",M_ATTRIBUTE);
            c_property(o)->type = (c_type)c_metaResolveType(c_metaObject(base),
                                                    "c_voidp");
            assert(c_property(o)->type);
            c_free(o);
            c_metaFinalize(c_metaObject(instanceType));
#define INSTANCE_NAME   "v_indexInstance<v_indexSample<>>"
#define INSTANCE_FORMAT "v_indexInstance<v_indexSample<%s>>"
            /* The sizeof contains \0 */
            length = sizeof(INSTANCE_NAME) + strlen(v_topicName(topic));
            name = os_alloca(length);
            sres = snprintf(name,length,INSTANCE_FORMAT,v_topicName(topic));
            assert(sres == (length-1));
#undef INSTANCE_NAME
#undef INSTANCE_FORMAT
            foundType = c_type(c_metaBind(c_metaObject(base),
                                          name,
                                          c_metaObject(instanceType)));

            os_freea(name);

            if (keyType != NULL) {
                keyInstanceType = c_type(c_metaDefine(c_metaObject(base),M_CLASS));
                if (keyInstanceType) {
                    c_class(keyInstanceType)->extends = c_keep(c_class(instanceType));
                    o = c_metaDeclare(c_metaObject(keyInstanceType),
                                      "key",M_ATTRIBUTE);
                    c_property(o)->type = c_keep(keyType);
                    c_free(o);
                    c_metaFinalize(c_metaObject(keyInstanceType));
#define INSTANCE_NAME   "v_indexKeyInstance<v_indexSample<>,>"
#define INSTANCE_FORMAT "v_indexKeyInstance<v_indexSample<%s>,%s>"
                    /* The sizeof contains \0 */
                    length = sizeof(INSTANCE_NAME) +
                             strlen(v_topicName(topic)) +
                             strlen(keyExpr);
                    name = os_alloca(length);
                    sres = snprintf(name,
                                    length,
                                    INSTANCE_FORMAT,
                                    v_topicName(topic),
                                    keyExpr);
                    assert(sres == (length-1));
#undef INSTANCE_NAME
#undef INSTANCE_FORMAT
                    c_free(foundType); /* Will be overwritten, so free */
                    foundType = c_type(c_metaBind(c_metaObject(base),
                                                  name,
                                                  c_metaObject(keyInstanceType)));
                    os_freea(name);
                    c_free(keyInstanceType);
                }
                c_free(keyType);
            }
            c_free(instanceType);
            c_free(baseType);
        } else {
            foundType = baseType; /* transfer refCount to caller */
        }
        c_free(sampleType);
    }

    return foundType;
}

static c_bool
indexCompare(
    void *o,
    c_iterActionArg arg)
{
    v_index index = v_index(o);
    v_topic topic = v_topic(arg);
    v_topic currentTopic = NULL;

    if (index->entry) {
        currentTopic = v_dataReaderEntryTopic(index->entry);
    }
    return (currentTopic == topic);
}

void
v_indexInit(
    v_index index,
    c_type instanceType,
    c_array keyList,
    v_reader reader)
{
    v_kernel kernel;
    c_property keyProperty;
    c_structure keyStructure;
    c_char fieldName[16];
    c_char *keyExpr;
    c_long i,nrOfKeys,totalSize;

    assert(index != NULL);
    assert(C_TYPECHECK(index,v_index));
    assert(C_TYPECHECK(instanceType,c_type));

    keyProperty = c_property(c_metaResolve(c_metaObject(instanceType),"key"));
    if (keyProperty) {
        keyStructure = c_structure(keyProperty->type);
        nrOfKeys = c_arraySize(keyStructure->members);
        c_free(keyProperty);
    } else {
        nrOfKeys = 0;
    }

    if (nrOfKeys>0) {
        totalSize = nrOfKeys * strlen("key.field0,");
        if (nrOfKeys > 9) {
            totalSize += (nrOfKeys-9);
            if (nrOfKeys > 99) {
                totalSize += (nrOfKeys-99);
            }
        }
        keyExpr = (char *)os_alloca(totalSize);
        keyExpr[0] = 0;
        for (i=0;i<nrOfKeys;i++) {
            sprintf(fieldName,"key.field%d",i);
            strcat(keyExpr,fieldName);
            if (i<(nrOfKeys-1)) { strcat(keyExpr,","); }
        }
    } else {
        keyExpr = NULL;
    }

    kernel = v_objectKernel(index);
    index->reader = reader;
    index->sourceKeyList = c_keep(keyList);

    index->objects = c_tableNew(instanceType,keyExpr);
    index->notEmptyList = c_tableNew(instanceType,keyExpr);

    if(keyExpr){
        os_freea(keyExpr);
    }
#ifdef _EXTENT_
#define _COUNT_ (32)
    index->objectExtent = c_extentSyncNew(instanceType,_COUNT_,TRUE);
#endif
}

/*#define prefix "sample.message.userData."*/

v_index
v__indexNew(
    v_dataReader reader,
    q_expr _from,
    c_iter indexList,
    v_indexNewAction action,
    c_voidp arg)
{
    v_kernel kernel;
    v_index index;
    v_topic topic;
    c_type instanceType;
    c_iter list;
    c_char *keyExpr;
    c_array keyList;
    c_long nrOfTopics;

    assert(C_TYPECHECK(reader,v_dataReader));
    kernel = v_objectKernel(reader);

    if (q_isId(_from)) {
        list = v_resolveTopics(kernel,q_getId(_from));
        nrOfTopics = c_iterLength(list);
        if (nrOfTopics == 0) {
            OS_REPORT_1(OS_ERROR,
                        "_v_dataReaderNew", 0,
                        "Unknown topic %s",
                        q_getId(_from));
            c_iterFree(list);
            return NULL;
        }
        if (nrOfTopics > 1) {
            OS_REPORT_1(OS_ERROR,
                        "_v_dataReaderNew", 0,
                        "Multiple topic definitions of: %s",
                        q_getId(_from));
            topic = v_topic(c_iterTakeFirst(list));
            while (topic != NULL) {
                c_free(topic);
                topic = v_topic(c_iterTakeFirst(list));
            }
            c_iterFree(list);
            return NULL;
        }
        topic = c_iterTakeFirst(list);
        c_iterFree(list);
        index = v_index(c_iterReadAction(indexList, indexCompare, topic));
        if (index == NULL) {
            /* If the userKey is enabled then the instance type key field type
             * will be determined from the user key expression and topic.
             * Otherwise when no user key is specified the default Topic key
             * type will be used.
             */
            if (v_reader(reader)->qos->userKey.enable) {
                keyExpr = v_reader(reader)->qos->userKey.expression;
            } else {
                keyExpr = NULL;
            }
            instanceType = createInstanceType(topic,keyExpr,&keyList);
            index = v_index(v_objectNew(kernel,K_INDEX));
            v_indexInit(index, instanceType, keyList, v_reader(reader));
            c_free(keyList);
            c_free(instanceType);

            if (action != NULL) {
                action(index, topic, arg);
            }
            c_iterAppend(indexList, index);
        }
    } else {
        OS_REPORT(OS_ERROR,
                  "v_indexNew failed",0,
                  "illegal from clause specified");
        assert(FALSE);
        index = NULL;
    }
    return index;
}

v_index
v_indexNew(
    v_dataReader reader,
    q_expr expr,
    v_indexNewAction action,
    c_voidp arg)
{
    v_index index;
    c_iter indexList;
    void *ptr;

    assert(C_TYPECHECK(reader,v_dataReader));

    indexList = c_iterNew(NULL);
    index = v__indexNew(reader, expr, indexList, action, arg);

    /* Clean up iterator */
    do {
        ptr = c_iterTakeFirst(indexList);
    } while (ptr != NULL);
    c_iterFree(indexList);

    assert(C_TYPECHECK(index,v_index));
    return index;
}


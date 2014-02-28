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
#include "d_storeMMFKernel.h"
#include "d_group.h"
#include "d_misc.h"
#include "c_collection.h"
#include "c_iterator.h"
#include "os.h"
#include "os_report.h"

static d_topicInfo
d_storeMMFKernelGetTopicInfo(
    d_storeMMFKernel _this,
    const char* topic)
{
    d_topicInfo result;
    c_value keyValue;

    keyValue = c_stringValue((c_string)topic);
    result = c_tableFind(_this->topics, &keyValue); /* c_tableFind already performs keep. */

    return result;
}

typedef struct groupWalkArg {
    d_nameSpace matchingNameSpace;
    c_action    action;
    c_voidp     actionArg;
} groupWalkArg;

static c_bool
groupWalkMatchingAction(
    c_object o,
    c_voidp arg)
{
    d_groupInfo groupInfo;
    groupWalkArg* walkArg;
    c_bool result;

    groupInfo = d_groupInfo(o);
    walkArg = (groupWalkArg*) arg;

    if(d_nameSpaceIsIn(walkArg->matchingNameSpace, groupInfo->partition,
            groupInfo->topic->name)){
        result = walkArg->action(o, walkArg->actionArg);
    } else {
        result = TRUE;
    }
    return result;
}

static c_bool
d_storeMMFKernelWalkGroups(
    d_storeMMFKernel  _this,
    d_nameSpace       matchingNameSpace,
    c_action          action,
    c_voidp           actionArg)
{
    groupWalkArg walkArg;
    c_bool result;

    if (matchingNameSpace == NULL){
        result = c_walk(_this->groups, action, actionArg);
    } else {
        walkArg.matchingNameSpace = matchingNameSpace;
        walkArg.action = action;
        walkArg.actionArg = actionArg;
        result = c_walk(_this->groups, groupWalkMatchingAction, &walkArg);
    }
    return result;
}

static c_bool
getBestQuality(
    c_object o,
    c_voidp arg)
{
    d_quality* quality;
    d_quality current;
    c_bool result;

    assert(C_TYPECHECK(o,d_groupInfo));
    assert(arg);

    if(o && arg){
        quality = (d_quality*) arg;
        current = d_groupInfoQuality(o);

        if(c_timeCompare(current, *quality) == C_GT){
            quality->seconds = current.seconds;
            quality->nanoseconds = current.nanoseconds;
        }
        result = TRUE;
    } else {
        result = FALSE;
    }
    return result;
}

static c_bool
setQuality(
    c_object o,
    c_voidp arg)
{
    d_quality quality;
    c_bool result;

    assert(C_TYPECHECK(o,d_groupInfo));
    assert(arg);

    if(o && arg){
        quality = *((d_quality*) arg);
        d_groupInfoSetQuality(o, quality);
        result = TRUE;
    } else {
        result = FALSE;
    }
    return result;
}

d_storeMMFKernel
d_storeMMFKernelAttach (
    c_base base,
    const c_char *name)
{
    d_storeMMFKernel kernel = NULL;

    if (name == NULL) {
        OS_REPORT(OS_ERROR,
                  "d_storeMMFKernelAttach",0,
                  "Failed to lookup kernel, specified kernel name = <NULL>");
    } else {
        kernel = c_lookup(base,name);
        if (kernel == NULL) {
            OS_REPORT_1(OS_ERROR,
                        "d_storeMMFKernelAttach",0,
                        "Failed to lookup kernel '%s' in Database",
                        name);
        } else if (c_checkType(kernel,"d_storeMMFKernel") != kernel) {
            c_free(kernel);
            kernel = NULL;
            OS_REPORT_1(OS_ERROR,
                        "d_storeMMFKernelAttach",0,
                        "Object '%s' is apparently not of type 'd_storeMMFKernel'",
                        name);
        }
    }
    return kernel;
}

d_storeMMFKernel
d_storeMMFKernelNew (
    c_base base,
    const c_char *name)
{
    d_storeMMFKernel kernel, found;
    c_type type;
    c_bool success;

    /* load meta-data */
    success = loadkernelModule(base);
    assert(success);

    success = loaddurabilityModule2(base);
    assert(success);

    /* create storeKernel */
    type = c_resolve(base,"durabilityModule2::d_storeMMFKernel");
    kernel = (d_storeMMFKernel)c_new(type);
    c_free(type);

     if(kernel) {
        /* Init storeKernel members */
        /* groups is a table of d_groupInfo with topic+partition as key */
        type = c_resolve(base,"durabilityModule2::d_groupInfo");
        assert(type);
        kernel->groups = c_tableNew(type, "partition,topic.name");
        kernel->backup = c_tableNew(type, "partition,topic.name");
        c_free(type);

        type = c_resolve(base,"durabilityModule2::d_topicInfo");
        assert(type);
        kernel->topics = c_tableNew(type, "name");
        c_free(type);

        type = c_resolve(base,"durabilityModule2::d_nameSpaceInfo");
        assert(type);
        kernel->nameSpaces = c_tableNew(type, "name");
        c_free(type);

        /* bind storeKernel */
        found = ospl_c_bind(kernel, name);
        assert(found == kernel);
     } else {
         OS_REPORT(OS_ERROR,
                   "d_storeMMFKernelNew",0,
                   "Failed to allocate kernel.");
         found = NULL;
     }
     return found;
}

d_groupInfo
d_storeMMFKernelGetGroupInfo(
    d_storeMMFKernel _this,
    const char *partition,
    const char *topic )
{
    d_groupInfo result;
    c_value keyValues[2];

    keyValues[0] = c_stringValue((c_string)partition);
    keyValues[1] = c_stringValue((c_string)topic);
    result = c_tableFind(_this->groups, keyValues); /* c_tableFind already performs keep. */

    return result;
}

d_storeResult
d_storeMMFKernelAddGroupInfo(
    d_storeMMFKernel _this,
    const d_group group)
{
    d_partition partition;
    d_topic topic;
    d_topicInfo topicInfo;
    d_groupInfo groupInfo;
    v_group vgroup;
    d_storeResult result;

    partition = d_groupGetPartition(group);
    topic = d_groupGetTopic(group);
    topicInfo = d_storeMMFKernelGetTopicInfo(_this, topic);

    if(!topicInfo){
        vgroup = d_groupGetKernelGroup(group);
        topicInfo = d_topicInfoNew(_this, vgroup->topic);

        if(topicInfo){
            c_tableInsert(_this->topics, topicInfo);
        }
    }
    if(topicInfo){
        groupInfo = d_groupInfoNew(_this, topicInfo, group);

        if(groupInfo){
            c_tableInsert(_this->groups, groupInfo);
            c_free(groupInfo);
            result = D_STORE_RESULT_OK;
        } else {
            result = D_STORE_RESULT_ERROR;
        }
        c_free(topicInfo);
    } else {
        result = D_STORE_RESULT_OUT_OF_RESOURCES;
    }
    os_free(partition);
    os_free(topic);

    return result;
}

d_storeResult
d_storeMMFKernelMarkNameSpaceComplete(
    d_storeMMFKernel kernel,
    const d_nameSpace nameSpace,
    const c_bool isComplete)
{
    d_nameSpaceInfo nsInfo;
    c_type type;
    c_char* name;
    c_value keyValues[1];
    d_storeResult result;

    if(kernel && nameSpace){
        name = d_nameSpaceGetName(nameSpace);
        keyValues[0] = c_stringValue((c_string)name);
        nsInfo = c_tableFind(kernel->nameSpaces, keyValues);

        if(nsInfo){
            nsInfo->complete = isComplete;
            result = D_STORE_RESULT_OK;
        } else {
            type = c_resolve(c_getBase(kernel),
                    "durabilityModule2::d_nameSpaceInfo");
            nsInfo = (d_nameSpaceInfo)c_new(type);
            c_free(type);

            if (nsInfo) {
                nsInfo->name = c_stringNew(c_getBase(kernel), name);
                nsInfo->complete = isComplete;
                c_tableInsert(kernel->nameSpaces, nsInfo);
                c_free(nsInfo);
                result = D_STORE_RESULT_OK;
            } else {
                OS_REPORT(OS_ERROR,
                      "d_storeMMFKernelMarkNameSpaceComplete",0,
                      "Failed to allocate nameSpaceInfo.");
                result = D_STORE_RESULT_OUT_OF_RESOURCES;
            }
        }
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeMMFKernelIsNameSpaceComplete(
    d_storeMMFKernel kernel,
    const d_nameSpace nameSpace,
    c_bool* isComplete)
{
    d_nameSpaceInfo nsInfo;
    c_char* name;
    d_storeResult result;
    c_value keyValues[1];

    if(kernel && nameSpace){
        name = d_nameSpaceGetName(nameSpace);
        keyValues[0] = c_stringValue((c_string)name);
        nsInfo = c_tableFind(kernel->nameSpaces, keyValues);

        if(nsInfo){
            *isComplete = nsInfo->complete;
            result = D_STORE_RESULT_OK;
        } else {
            *isComplete = FALSE;
            result = D_STORE_RESULT_OK;
        }
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeMMFKernelGetQuality(
    d_storeMMFKernel kernel,
    const d_nameSpace nameSpace,
    d_quality* quality)
{
    d_storeResult result;
    c_bool success;

    if(kernel && nameSpace && quality){
        quality->seconds = 0;
        quality->nanoseconds = 0;
        success = d_storeMMFKernelWalkGroups(
                kernel, nameSpace, getBestQuality, quality);

        if(success){
            result = D_STORE_RESULT_OK;
        } else {
            result = D_STORE_RESULT_ERROR;
        }
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeMMFKernelUpdateQuality(
    d_storeMMFKernel kernel,
    d_quality quality)
{
    c_bool success;
    d_storeResult result;

    if(kernel){
        success = d_storeMMFKernelWalkGroups(
                    kernel, NULL, setQuality, &quality);
        if(success){
            result = D_STORE_RESULT_OK;
        } else {
            result = D_STORE_RESULT_ERROR;
        }
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeMMFKernelBackup(
    d_storeMMFKernel kernel,
    const d_store store,
    const d_nameSpace nameSpace)
{
    c_iter groups;
    d_groupInfo group;
    d_groupInfo backup, found;
    d_storeResult result;

    if(kernel && nameSpace){
        groups = ospl_c_select(kernel->groups, 0);
        group = d_groupInfo(c_iterTakeFirst(groups));
        result = D_STORE_RESULT_OK;

        while(group && (result == D_STORE_RESULT_OK)){
            if(d_nameSpaceIsIn(nameSpace, group->partition, group->topic->name)){
                result = d_groupInfoBackup(group, store, &backup);

                if(result == D_STORE_RESULT_OK){
                    found = d_groupInfo(c_tableInsert(kernel->backup, backup));

                    if(found != backup){
                        c_remove(kernel->backup, found, NULL, NULL);
                        c_free(found);
                        found = d_groupInfo(c_tableInsert(kernel->backup, backup));
                        assert(found == backup);

                        if(found != backup){
                            result = D_STORE_RESULT_ERROR;
                        }
                    }
                    c_free(backup);
                }
            }
            c_free(group);
            group = d_groupInfo(c_iterTakeFirst(groups));
        }
        c_iterFree(groups);
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeMMFKernelBackupRestore(
    d_storeMMFKernel kernel,
    const d_store store,
    const d_nameSpace nameSpace)
{
    c_iter groups;
    d_groupInfo group;
    d_groupInfo restore, found;
    d_storeResult result;

    OS_UNUSED_ARG(store);

    if(kernel && nameSpace){
        groups = ospl_c_select(kernel->backup, 0);
        group = d_groupInfo(c_iterTakeFirst(groups));
        result = D_STORE_RESULT_OK;

        while(group && (result == D_STORE_RESULT_OK)){
            if(d_nameSpaceIsIn(nameSpace, group->partition, group->topic->name)){
                restore = c_remove(kernel->backup, group, NULL, NULL);
                assert(restore);

                if(restore){
                    found = d_groupInfo(c_tableInsert(kernel->groups, restore));

                    if(found != restore){
                        c_remove(kernel->groups, found, NULL, NULL);
                        c_free(found);
                        found = d_groupInfo(c_tableInsert(kernel->groups, restore));
                        assert(found == restore);

                        if(found != restore){
                            result = D_STORE_RESULT_ERROR;
                        }
                    }
                } else {
                    result = D_STORE_RESULT_ERROR;
                }
            }
            c_free(group);
            group = d_groupInfo(c_iterTakeFirst(groups));
        }
        c_iterFree(groups);
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeMMFKernelDeleteNonMatchingGroups(
    d_storeMMFKernel _this,
    c_string partitionExpr,
    c_string topicExpr)
{
    d_storeResult result;
    d_groupInfo group, removed;
    c_iter groups;
    c_bool match;

    if(_this && partitionExpr && topicExpr){
        result = D_STORE_RESULT_OK;
        groups = ospl_c_select(_this->groups, 0);
        group = d_groupInfo(c_iterTakeFirst(groups));

        while(group){
            match = d_patternMatch(group->partition, partitionExpr);

            if(match){
                match = d_patternMatch(group->topic->name, topicExpr);
            }

            if(!match){
                removed = c_remove(_this->groups, group, NULL, NULL);
                assert(removed == group);

                if(removed != group){
                    result = D_STORE_RESULT_MUTILATED;
                }
                c_free(removed);
            }
            c_free(group);
            group = d_groupInfo(c_iterTakeFirst(groups));
        }
        c_iterFree(groups);
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

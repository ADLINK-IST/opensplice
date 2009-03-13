#include "u__entity.h"
#include "u_query.h"
#include "u_reader.h"
#include "u_user.h"
#include "v_query.h"
#include "v_entity.h"
#include "v_collection.h"
#include "v_dataReader.h"
#include "v_dataView.h"
#include "v_dataViewInstance.h"

#include "u__types.h"
#include "q_expr.h" 

#include "os_report.h"

u_result
u_queryClaim(
    u_query _this,
    v_query *query)
{
    u_result result = U_RESULT_OK;

    if ((_this != NULL) && (query != NULL)) {
        *query = v_query(u_entityClaim(u_entity(_this)));
        if (*query == NULL) {
            OS_REPORT_2(OS_WARNING, "u_queryClaim", 0,
                        "Claim Query failed. "
                        "<_this = 0x%x, query = 0x%x>.",
                         _this, query);
            result = U_RESULT_INTERNAL_ERROR;
        }
    } else {
        OS_REPORT_2(OS_ERROR,"u_queryClaim",0,
                    "Illegal parameter. "
                    "<_this = 0x%x, query = 0x%x>.",
                    _this, query);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_queryRelease(
    u_query _this)
{
    u_result result = U_RESULT_OK;

    if (_this != NULL) {
        result = u_entityRelease(u_entity(_this));
    } else {
        OS_REPORT_1(OS_ERROR,"u_queryRelease",0,
                    "Illegal parameter. <_this = 0x%x>.", _this);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_query
u_queryNew(
    u_reader source,
    const c_char *name,
    q_expr predicate,
    c_value params[])
{
    u_participant p;
    u_query _this = NULL;
    v_collection kc;
    v_query query;
    u_result result;
    q_expr copy;

    if (source != NULL) {
        copy = q_exprCopy(predicate);
        if (copy != NULL) {
            kc = v_collection(u_entityClaim(u_entity(source)));
            if (kc != NULL) {
                query = v_queryNew(kc,name,predicate,params);
                if (query != NULL) {
                    p = u_entityParticipant(u_entity(source));
                    _this = u_entityAlloc(p,u_query,query,TRUE);
                    if (_this != NULL) {
                        result = u_queryInit(_this);
                        if (result == U_RESULT_OK) {
                            _this->source = source;
                            if (name != NULL) {
                                _this->name = os_strdup(name);
                            } else {
                                _this->name = NULL;
                            }
                            _this->predicate = copy;
                        } else {
                            q_dispose(copy);
                            OS_REPORT(OS_ERROR, "u_queryNew", 0, 
                                      "Initialisation failed.");
                        }
                    } else {
                        q_dispose(copy);
                        OS_REPORT(OS_ERROR, "u_queryNew", 0, 
                                  "Create user proxy failed.");
                    }
                    c_free(query);
                } else {
                    q_dispose(copy);
                    OS_REPORT(OS_ERROR, "u_queryNew", 0, 
                              "Create kernel entity failed.");
                }
                u_entityRelease(u_entity(source));
            } else {
                q_dispose(copy);
                OS_REPORT(OS_WARNING, "u_queryNew", 0, 
                          "Claim Query source failed.");
            }
        } else {
            OS_REPORT(OS_ERROR, "u_queryNew", 0,
                      "Failed to copy Query predicate.");
        }
    } else {
        OS_REPORT(OS_ERROR,"u_queryNew",0,
                  "No Query source specified.");
    }
    return _this;
}

u_result
u_queryInit(
    u_query _this)
{
    u_result result;

    if (_this != NULL) {
        result = u_readerInit(u_reader(_this));
        _this->source = NULL;
        _this->name = NULL;
        _this->predicate = NULL;
        u_entity(_this)->flags |= U_ECREATE_INITIALISED;
    } else {
        OS_REPORT(OS_ERROR,"u_queryInit",0, "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_queryFree(
    u_query _this)
{
    u_result result;

    if (_this != NULL) {
        if (u_entity(_this)->flags & U_ECREATE_INITIALISED) {
            result = u_queryDeinit(_this);
            os_free(_this);
        } else {
            result = u_entityFree(u_entity(_this));
        }
    } else {
        OS_REPORT(OS_WARNING,"u_queryFree",0,
                  "The specified Query = NIL.");
        result = U_RESULT_OK;
    }
    return result;
}

u_result
u_queryDeinit(
    u_query _this)
{
    u_result result;

    if (_this != NULL) {
        _this->source = NULL;
        q_dispose(_this->predicate);
        os_free(_this->name);
        result = u_readerDeinit(u_reader(_this));
    } else {
        OS_REPORT(OS_ERROR,"u_queryDeinit",0, "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

C_STRUCT(readActionArg) {
    u_readerAction action;
    c_voidp arg;
    c_bool proceed;
};

C_CLASS(readActionArg);

static c_bool
readAction(
    c_object sample,
    c_voidp arg)
{
    readActionArg a = (readActionArg)arg;

    if (sample == NULL) {
        a->action((v_dataReaderSample)sample,a->arg);
        return a->proceed;
    }
    a->proceed = a->action((v_dataReaderSample)sample,a->arg);
    return a->proceed;
}

u_result
u_queryRead(
    u_query _this, 
    u_readerAction action,
    c_voidp actionArg)
{
    u_result result;
    v_query query;
    C_STRUCT(readActionArg) arg;
    
    result = u_queryClaim(_this,&query);
    
    if (result == U_RESULT_OK) {
        arg.action = action;
        arg.arg = actionArg;
        arg.proceed = TRUE;
        v_queryRead(query,readAction,&arg);
        u_queryRelease(_this);
    } else {
        OS_REPORT(OS_WARNING, "u_queryRead", 0,
                  "Could not claim query.");
    }
    return result;
}

u_result
u_queryTake(
    u_query _this, 
    u_readerAction action,
    c_voidp actionArg)
{
    u_result result;
    v_query query;
    C_STRUCT(readActionArg) arg;
    
    result = u_queryClaim(_this,&query);
    
    if (result == U_RESULT_OK) {
        arg.action = action;
        arg.arg = actionArg;
        arg.proceed = TRUE;
        v_queryTake(query,readAction,&arg);
        u_queryRelease(_this);
    } else {
        OS_REPORT(OS_WARNING, "u_queryTake", 0,
                  "Could not claim query.");
    }
    return result;
}

C_STRUCT(readListActionArg) {
    c_iter iter;
    c_ulong spaceLeft;
    u_readerCopyList copyAction;
    c_voidp copyArg;
    c_voidp result;
};

C_CLASS(readListActionArg);

static c_bool
readListAction(
    c_object sample,
    c_voidp arg)
{
    readListActionArg a = (readListActionArg)arg;

    if (a->spaceLeft == 0) {
        return FALSE;
    }
    if (sample == NULL) {
        a->result = a->copyAction(NULL,a->iter,a->copyArg);
        return FALSE;
    }
    if (!c_iterContains(a->iter,sample)) {
        a->iter = c_iterInsert(a->iter,c_keep(sample));
        a->spaceLeft--;
    }
    if (a->spaceLeft == 0) {
        a->result = a->copyAction(NULL,a->iter,a->copyArg);
        return FALSE;
    } else {
        return TRUE;
    }
}

void *
u_queryReadList(
    u_query _this,
    c_ulong max,
    u_readerCopyList copy,
    c_voidp copyArg)
{
    v_query query;
    c_iter list;
    void *result;
    u_result r;
    C_STRUCT(readListActionArg) arg;
    c_object object;

    if (copy == NULL) {
        return NULL;
    }
    r = u_queryClaim(_this,&query);
    if (r != U_RESULT_OK) {
        OS_REPORT(OS_WARNING, "u_queryReadList", 0,"Could not claim query.");
        return NULL;
    }
    arg.iter = NULL;
    if (max == 0) {
        arg.spaceLeft = 0x7fffffff;
    } else {
        arg.spaceLeft = max;
    }
    arg.copyAction = copy;
    arg.copyArg = copyArg;
    arg.result = NULL;
    v_queryRead(query,readListAction,&arg);
    list = arg.iter;
    result = arg.result;
    object = c_iterTakeFirst(list);
    while (object != NULL) {
        c_free(object);
        object = c_iterTakeFirst(list);
    }
    c_iterFree(list);
    u_queryRelease(_this);
    return result;
}

void *
u_queryTakeList(
    u_query _this,
    c_ulong max,
    u_readerCopyList copy,
    c_voidp copyArg)
{
    v_query query;
    c_iter list;
    void *result;
    u_result r;
    C_STRUCT(readListActionArg) arg;
    c_object object;

    if (copy == NULL) {
        return NULL;
    }
    r = u_queryClaim(_this,&query);
    if (r != U_RESULT_OK) {
        OS_REPORT(OS_WARNING, "u_queryTakeList", 0,"Could not claim query.");
        return NULL;
    }
    arg.iter = NULL;
    if (max == 0) {
        arg.spaceLeft = 0x7fffffff;
    } else {
        arg.spaceLeft = max;
    }
    arg.copyAction = copy;
    arg.copyArg = copyArg;
    arg.result = NULL;
    v_queryTake(query,readListAction,&arg);
    list = arg.iter;
    result = arg.result;
    object = c_iterTakeFirst(list);
    while (object != NULL) {
        c_free(object);
        object = c_iterTakeFirst(list);
    }
    c_iterFree(list);
    u_queryRelease(_this);
    return result;
}

c_bool
u_queryTest(
    u_query _this)
{
    v_query query;
    c_bool result;
    u_result r;

    r = u_queryClaim(_this,&query);
    if ((r == U_RESULT_OK) && (query != NULL)) {
        result = v_queryTest(query);
        u_queryRelease(_this);
    } else {
        OS_REPORT(OS_WARNING, "u_queryTest", 0,"Could not claim query.");
        result = FALSE;
    }
    return result;
}

u_result
u_querySet(
    u_query _this,
    c_value params[])
{
    q_expr predicate;
    v_query query;
    c_bool kr;
    u_result r;

    if (_this != NULL) {
        r = u_queryClaim(_this,&query);
        if ((r == U_RESULT_OK) && (query != NULL)) {
            kr = v_querySetParams(query, _this->predicate, params);
            if (!kr) {
                OS_REPORT(OS_ERROR, "u_querySet", 0,
                          "Could not set kernel query parameters.");
                r = U_RESULT_INTERNAL_ERROR;
            }
            u_queryRelease(_this);
        } else {
            OS_REPORT(OS_WARNING, "u_querySet", 0,
                      "Claim query failed.");
        }
    } else {
        OS_REPORT(OS_ERROR, "u_querySet", 0,
                  "No Query specified.");
        r = U_RESULT_INTERNAL_ERROR;
    }
    return r;
}

/***************************** read/take_(next)_instance **********************/

static c_bool
queryContainsInstance (
    v_query _this,
    v_dataReaderInstance i)
{
    c_bool result = FALSE;
    v_dataReader reader;
    v_dataView view;
    v_collection src;

    switch (v_objectKind(_this)) {
    case K_DATAREADERQUERY:
        src = v_querySource(_this);
        reader = v_dataReader(src);
        result = v_dataReaderContainsInstance(reader,i);
        c_free(src);
    break;
    case K_DATAVIEWQUERY:
        src = v_querySource(_this);
        view = v_dataView(src);
        assert(C_TYPECHECK(i, v_dataViewInstance));
        result = v_dataViewContainsInstance(view,v_dataViewInstance(i));
        c_free(src);
    break;
    default:
        assert(FALSE);
    }

    return result;
}

u_result
u_queryReadInstance(
    u_query _this,
    u_instanceHandle handle,
    u_readerAction action,
    c_voidp actionArg)
{
    v_query  query;
    u_result result;
    v_dataReaderInstance instance;
     
    result = u_queryClaim(_this,&query);
    if ((result == U_RESULT_OK) && (query != NULL)) {
        result = u_instanceHandleClaim(handle, &instance);
        if (result == U_RESULT_OK) {
            assert(instance != NULL);
            if (queryContainsInstance(query,instance)) {
                v_queryReadInstance(query,instance,action,actionArg);
            } else {
                result = U_RESULT_PRECONDITION_NOT_MET;
            }
            u_instanceHandleRelease(handle);
        }
        u_queryRelease(_this);
    } else {
        OS_REPORT(OS_WARNING, "u_queryReadInstance", 0,
                  "Could not claim query.");
    }
    return result;
}

u_result
u_queryTakeInstance(
    u_query _this,
    u_instanceHandle handle,
    u_readerAction action,
    c_voidp actionArg)
{
    v_query  query;
    u_result result;
    v_dataReaderInstance instance;
     
    result = u_queryClaim(_this,&query);
    if ((result == U_RESULT_OK) && (query != NULL)) {
        result = u_instanceHandleClaim(handle, &instance);
        if (result == U_RESULT_OK) {
            assert(instance != NULL);
            if (queryContainsInstance(query,instance)) {
                v_queryTakeInstance(query,instance,action,actionArg);
            } else {
                result = U_RESULT_PRECONDITION_NOT_MET;
            }
            u_instanceHandleRelease(handle);
        }

        u_queryRelease(_this);
    } else {
        OS_REPORT(OS_WARNING, "u_queryTakeInstance", 0,
                  "Could not claim query.");
    }
   
    return result;
}

u_result
u_queryReadNextInstance(
    u_query _this,
    u_instanceHandle handle,
    u_readerAction action,
    c_voidp actionArg)
{
    v_query  query;
    u_result result;
    v_dataReaderInstance instance;
     
    result = u_queryClaim(_this,&query);
    if ((result == U_RESULT_OK) && (query != NULL)) {
        if ( u_instanceHandleIsNil(handle) ) {
            v_queryReadNextInstance(query, NULL, action, actionArg);
        } else {
            result = u_instanceHandleClaim(handle, &instance);
            if (result == U_RESULT_OK) {
                if (queryContainsInstance(query,instance)) {
                    v_queryReadNextInstance(query, instance, action, actionArg);
                    result = U_RESULT_OK;
                } else {
                    result = U_RESULT_PRECONDITION_NOT_MET;
                }
                u_instanceHandleRelease(handle);
            }
        }
        u_queryRelease(_this);
    } else {
        OS_REPORT(OS_WARNING, "u_queryReadNextInstance", 0,
                  "Could not claim query.");
    }
   
    return result;
}

u_result
u_queryTakeNextInstance(
    u_query _this,
    u_instanceHandle handle,
    u_readerAction action,
    c_voidp actionArg)
{
    v_query  query;
    u_result result;
    v_dataReaderInstance instance;
     
    result = u_queryClaim(_this,&query);
    if ((result == U_RESULT_OK) && (query != NULL)) {
        if ( u_instanceHandleIsNil(handle) ) {
            v_queryTakeNextInstance(query, NULL, action, actionArg);
        } else {
            result = u_instanceHandleClaim(handle, &instance);
            if (result == U_RESULT_OK) {
                if (queryContainsInstance(query,instance)) {
                    v_queryTakeNextInstance(query, instance, action, actionArg);
                    result = U_RESULT_OK;
                } else {
                    result = U_RESULT_PRECONDITION_NOT_MET;
                }
                u_instanceHandleRelease(handle);
            }
        }
        u_queryRelease(_this);
    } else {
        OS_REPORT(OS_WARNING, "u_queryTakeNextInstance", 0,
                  "Could not claim query.");
    }
   
    return result;
}

u_reader
u_querySource (
    u_query _this)
{
    assert(_this != NULL);
    assert(u_entityKind(u_entity(_this)) == U_QUERY);

    return _this->source;
}


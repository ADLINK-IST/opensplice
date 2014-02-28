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
#include <assert.h>
#include "qp_qosProvider.h"

#include "os.h"
#include "os_report.h"

#include "c_metabase.h"
#include "c_base.h"
#include "c_typebase.h"
#include "c_field.h"
#include "c_collection.h"

#include "u_user.h"
#include "u_types.h"

#include "cf_config.h"
#include "cfg_parser.h"

#include "ut_collection.h"

/* Included load functions; the load-functions are generated with
 * idlpp -m SPLLOAD. */
#include "dds_builtinTopicsSplLoad.c"
#include "dds_dcps_builtintopicsSplLoad.c"
#include "dds_namedQosTypesSplLoad.c"

/* Default QoS's in database-format (sequences and strings are NULL!) */
#include "qp_defaultQos.h"

#define DDS_TAG                 "dds"
#define PROFILE_TAG             "qos_profile"
/* 'DDS for lightweigth CCM v1.1' mandates "participant_qos"; the provided
 * example uses "domainparticipant_qos". We follow the specification. */
#define DPQOS_TAG               "domainparticipant_qos"
#define TQOS_TAG                "topic_qos"
#define PUBQOS_TAG              "publisher_qos"
#define SUBQOS_TAG              "subscriber_qos"
#define DWQOS_TAG               "datawriter_qos"
#define DRQOS_TAG               "datareader_qos"

#if 0
static int level;
#define QP_TRACE(t) t
#else
#define QP_TRACE(t)
#endif

C_CLASS(qp_entityAttr);
C_STRUCT (qp_entityAttr) {
    c_object                    defaultQosTmplt;
    c_type                      qosSampleType;
    qp_copyOut                  copyOut;
    ut_table                    qosTable; /* (char *, c_object)  */
};

C_STRUCT(qp_qosProvider) {
    c_base                      baseAddr;
    c_char *                    defaultProfile;
    cf_element                  rootElement;
    C_STRUCT (qp_entityAttr)    dpQosAttr;
    C_STRUCT (qp_entityAttr)    tQosAttr;
    C_STRUCT (qp_entityAttr)    pubQosAttr;
    C_STRUCT (qp_entityAttr)    subQosAttr;
    C_STRUCT (qp_entityAttr)    dwQosAttr;
    C_STRUCT (qp_entityAttr)    drQosAttr;
};

typedef enum {
    QP_SCOPE_NONE,
    QP_SCOPE_DDS,
    QP_SCOPE_PROFILE,
    QP_SCOPE_QOSPOLICY
} qp_parseScope;

C_CLASS(qp_parseContext);
C_STRUCT(qp_parseContext) {
    qp_qosProvider              qosProvider;
    c_field                     currentField;
    c_type                      currentFieldType;
    c_object                    qosSample;
    ut_table                    qosTable;
    qp_parseScope               scope;
    const char *                name;
};

typedef struct {
    const char *                name;
    const char *                value;
} ConstantValuePair;

static const ConstantValuePair ApiConstants[] = {
        { "LENGTH_UNLIMITED",       "-1"         },
        { "DURATION_INFINITE_SEC",  "2147483647" },
        { "DURATION_INFINITE_NSEC", "2147483647" }
};

/**************************************************************
 * Private functions
 **************************************************************/

static const char *
substituteConstants(
        const char *xmlValue)
    __attribute__((nonnull))
    __attribute__((pure));

static qp_result
processElement(
        cf_element element,
        C_STRUCT(qp_parseContext) ctx)
    __attribute__((nonnull));

static qp_result
processContainedElements(
        c_iter elements,
        C_STRUCT(qp_parseContext) ctx);

static qp_result
processElementData(
        cf_data data,
        C_STRUCT(qp_parseContext) ctx)
    __attribute__((nonnull));

static qp_result
processAttribute(
        cf_attribute attribute,
        qp_parseContext ctx)
    __attribute__((nonnull));

static qp_result
processContainedAttributes(
        c_iter attributes,
        qp_parseContext ctx)
    __attribute__((nonnull(2)));

static qp_result
prepareQosSample(
        const char *qosName,
        qp_entityAttr attr,
        qp_parseContext ctx)
    __attribute__((nonnull));

static void
unloadEntityQosAttributes(
        qp_entityAttr attr)
    __attribute__((nonnull));


static void
unloadQosProviderQosAttributes(
        qp_qosProvider _this)
    __attribute__((nonnull));

/* loadQosSampleType: load qosSample type into the database. */
#undef c_metaObject /* Undef casting macro to avoid ambiguity with return type of function pointer. */
typedef c_metaObject (*qp_fnLoadQosSampleType)(c_base base);

typedef void(*qp_shallowClone)(void *from, void *to);

static qp_result
loadEntityQosAttributes(
        qp_qosProvider _this,
        qp_fnLoadQosSampleType loadQosSample,
        const C_STRUCT(qp_qosInputAttr) *inputAttr,
        qp_shallowClone shallowClone,
        c_voidp defaultQos,
        qp_entityAttr outputAttr)
    __attribute__((nonnull));

static qp_result
loadQosProviderQosAttributes(
        qp_qosProvider _this,
        const C_STRUCT(qp_qosProviderInputAttr) *attr)
    __attribute__((nonnull));

/* Table routines */
static os_equality
qp_qosCompareByName (
        void *o1,
        void *o2,
        void *unused)
    __attribute__((nonnull(1,2)))
    __attribute__((pure));

static void
qp_qosTablefreeKey (
        void *o,
        void *unused)
    __attribute__((nonnull(1)));

static void
qp_qosTablefreeData (
        void *o,
        void *unused)
    __attribute__((nonnull(1)));

static c_object
qp_qosTableLookup(
        ut_table t,
        const c_char *defaultProfile,
        const c_char *id)
    __attribute__((nonnull(1,2)));

static os_equality
qp_qosCompareByName (
        void *o1,
        void *o2,
        void *unused)
{
    int result;

    OS_UNUSED_ARG(unused);

    assert(o1);
    assert(o2);

    result = strcmp((const char *)o1, (const char *)o2);
    if(result < 0){
        return C_LT;
    } else if (result > 0) {
        return C_GT;
    } else {
        return C_EQ;
    }
}

static void
qp_qosTablefreeKey (
        void *o,
        void *unused)
{
    assert(o);

    OS_UNUSED_ARG(unused);

    os_free(o);
}

static void
qp_qosTablefreeData (
        void *o,
        void *unused)
{
    assert(o);

    OS_UNUSED_ARG(unused);

    c_free(o);
}

static c_object
qp_qosTableLookup(
        ut_table t,
        const c_char *defaultProfile,
        const c_char *id)
{
    c_object qos = NULL;
    const c_char * normalized = NULL;
    const c_char * normalizedDefaultProfile;

    assert(t);
    assert(defaultProfile);

    normalizedDefaultProfile = strncmp("::", defaultProfile, strlen("::")) == 0 ? defaultProfile + strlen("::") : defaultProfile;

    if(id) {
        normalized = strncmp("::", id, strlen("::")) == 0 ? id + strlen("::") : id;
        if(id != normalized){
            /* Fully-qualified name */
            qos = ut_get(ut_collection(t), (void *)normalized);
        } else {
            c_char fullname[256];
            c_char *fn = fullname;
            os_size_t len;

            if((len = snprintf(fn, sizeof(fullname), "%s::%s", normalizedDefaultProfile, id)) >= sizeof(fullname)) {
                if((fn = os_malloc(len + 1)) != NULL) {
                    (void) snprintf(fn, len + 1, "%s::%s", normalizedDefaultProfile, id);
                }
            }
            if(fn){
                qos = ut_get(ut_collection(t), (void *)fn);

                if(fn != fullname) {
                    os_free(fn);
                }
            }
        }
    } else {
        /* Empty qosName in default profile */
        qos = ut_get(ut_collection(t), (void *)normalizedDefaultProfile);
    }

    return qos;
}

static const char *
substituteConstants(
        const char *xmlValue)
{
    size_t i;

    assert(xmlValue);

    for (i = 0; i < sizeof(ApiConstants)/sizeof(ConstantValuePair); i++) {
        if (strcmp(xmlValue, ApiConstants[i].name) == 0) return ApiConstants[i].value;
    }
    return xmlValue;
}

static qp_result
prepareQosSample(
        const char *qosName,
        qp_entityAttr attr,
        qp_parseContext ctx)
{
    qp_result result;
    const c_type qosSampleType = attr->qosSampleType;
    assert(qosSampleType);

    assert(qosName);
    assert(ctx);

    ctx->scope = QP_SCOPE_QOSPOLICY;
    ctx->qosSample = c_new(qosSampleType);
    if (ctx->qosSample == NULL){
        result = QP_RESULT_OUT_OF_MEMORY;
        goto err_new_qosSample;
    }
    c_cloneIn(qosSampleType, attr->defaultQosTmplt, &ctx->qosSample);
    ctx->currentField = c_fieldNew(qosSampleType, qosName);
    if (ctx->currentField == NULL){
        result = QP_RESULT_OUT_OF_MEMORY;
        goto err_fieldNew;
    }
    ctx->currentFieldType = c_fieldType(ctx->currentField);
    assert(ctx->currentFieldType);

    ctx->qosTable = attr->qosTable;
    return QP_RESULT_OK;

/* Error handling */
err_fieldNew:
    c_free(ctx->qosSample);
    ctx->qosSample = NULL;
err_new_qosSample:
    return result;
}


static qp_result
processAttribute(
        cf_attribute attribute,
        qp_parseContext ctx)
{
    qp_result result = QP_RESULT_OK;
    const c_char *name = cf_nodeGetName((cf_node)attribute);
    c_value value = cf_attributeValue(attribute);

    assert(value.kind == V_STRING);
    switch (ctx->scope)
    {
    case QP_SCOPE_PROFILE:
        if (strcmp(name, "name") == 0) {
            ctx->name = value.is.String;
        } else if (strcmp(name, "base_name") == 0){
            /* TODO: process base_name */
            OS_REPORT_1(
                    OS_INFO,
                    "qp_qosProvider::processAttribute",
                    0,
                    "Attribute (\"%s\") not yet supported for a <qos_profile> element...",
                    name);
        } else if (strcmp(name, "topic_filter") != 0){ /* topic_filter has no meaning here */
            result = QP_RESULT_UNKNOWN_ARGUMENT;
            OS_REPORT_1(
                    OS_API_INFO,
                    "qp_qosProvider::processAttribute",
                    0,
                    "Unknown attribute (\"%s\") for a <qos_profile> element...",
                    name);
        }
        break;
    case QP_SCOPE_QOSPOLICY:
        if (strcmp(name, "name") == 0) {
            ctx->name = value.is.String;
        } else if (strcmp(name, "base_name") == 0){
            /* TODO: process base_name */
            OS_REPORT_1(
                    OS_INFO,
                    "qp_qosProvider::processAttribute",
                    0,
                    "Attribute (\"%s\") not yet supported for a qos element...",
                    name);
        } else {
            result = QP_RESULT_UNKNOWN_ARGUMENT;
            OS_REPORT_1(
                    OS_API_INFO,
                    "qp_qosProvider::processAttribute",
                    0,
                    "Unknown attribute (\"%s\") for a <(" DPQOS_TAG "|" TQOS_TAG "|" PUBQOS_TAG "|" SUBQOS_TAG "|" DWQOS_TAG "|" DRQOS_TAG ")_qos> element...",
                    name);
        }
        break;
    default:
        result = QP_RESULT_UNKNOWN_ARGUMENT;
        OS_REPORT_1(
                OS_API_INFO,
                "qp_qosProvider::processAttribute",
                0,
                "Element specifies an unknown attribute (\"%s\")...",
                name);
    }

    return result;
}

static qp_result
processContainedAttributes(
        c_iter attributes,
        qp_parseContext ctx)
{
    qp_result result = QP_RESULT_OK;
    cf_attribute attribute;

    assert(ctx);

    while ((attribute = (cf_attribute) c_iterTakeFirst(attributes)) != NULL && result == QP_RESULT_OK)
    {
        result = processAttribute(attribute, ctx);
    }
    c_iterFree(attributes);
    return result;
}

static qp_result
processElementData(
        cf_data data,
        C_STRUCT(qp_parseContext) ctx)
{
    qp_result result;
    char first;

    assert(data);

    first = cf_dataValue(data).is.String[0];

    if(strcmp(cf_node(data)->name, "#text") == 0 &&
       (first == '\0'  || first == '\n' || first == '<' || first == ' '))
    {
        /* Skip comments */
        result = QP_RESULT_OK;
    } else {
        c_value xmlValue = cf_dataValue(data);
        c_value qosValue = c_fieldValue(ctx.currentField, ctx.qosSample);
        const char *actualValue = substituteConstants(xmlValue.is.String);

        if (c_imageValue(actualValue, &qosValue, ctx.currentFieldType)) {
            c_fieldAssign(ctx.currentField, ctx.qosSample, qosValue);
            result = QP_RESULT_OK;
        } else {
            result = QP_RESULT_ILLEGAL_VALUE;
            OS_REPORT_2(
                    OS_API_INFO,
                    "qp_qosProvider::processElementData",
                    0,
                    "Illegal value (\"%s\") for qosPolicy field \"%s\"...",
                    xmlValue.is.String,
                    c_fieldName(ctx.currentField));
        }
    }

    return result;
}

static qp_result
processContainedElements(
        c_iter elements,
        C_STRUCT(qp_parseContext) ctx)
{
    qp_result result = QP_RESULT_OK;
    cf_node node;

    while ((node = (cf_node) c_iterTakeFirst(elements)) != NULL && result == QP_RESULT_OK)
    {
        cf_kind kind = cf_nodeKind(node);
        switch(kind) {
        case CF_ELEMENT:
            result = processElement(cf_element(node), ctx);
            break;
        case CF_DATA:
            result = processElementData(cf_data(node), ctx);
            break;
        default:
            assert(0);
            break;
        }
    }
    /* Elements of iter elements don't have to be freed, so no worries that iter
     * is potentially not empty here. */
    c_iterFree(elements);
    return result;
}

static qp_result
processElement(
        cf_element element,
        C_STRUCT(qp_parseContext) ctx)
{
    qp_result result = QP_RESULT_OK;
    const c_char *name = cf_nodeGetName((cf_node)element);
    assert(name);

    assert(ctx.qosProvider->defaultProfile);

    /* Enforce the opening <dds> tag. */
    switch(ctx.scope)
    {
    case QP_SCOPE_NONE:
        if (strcmp(name, DDS_TAG) == 0) {
            ctx.scope = QP_SCOPE_DDS;
            QP_TRACE(printf("%*sBEGIN " DDS_TAG "\n", level++ * 2, ""));
            result = processContainedElements(cf_elementGetChilds(element), ctx);
            QP_TRACE(printf("%*sEND " DDS_TAG "\n", --level * 2, ""));
        } else {
            QP_TRACE(printf("ERROR: Unrecognized top-level element ('%s'), expected top-level element '%s'\n", name, DDS_TAG));
            OS_REPORT_2(
                    OS_API_INFO,
                    "qp_qosProvider::processElement",
                    0,
                    "Unrecognized top-level element (\"%s\"), expected top-level element \"%s\"...",
                    name,
                    DDS_TAG);
            return QP_RESULT_UNEXPECTED_ELEMENT;
        }
        break;
    case QP_SCOPE_DDS:
        if (strcmp(name, PROFILE_TAG) == 0) {
            ctx.scope = QP_SCOPE_PROFILE;
            /* Fetch the (required according to Annex C of the
             * 'DDS for Lightweight CCM, v1.1' specification) name of the
             * current profile and store it in the current ctx. */
            if((result = processContainedAttributes(cf_elementGetAttributes(element), &ctx)) != QP_RESULT_OK){
                return result;
            }
            if(!ctx.name){
                QP_TRACE(printf("ERROR: Element <" PROFILE_TAG "> has no 'name' attribute, which is mandatory.\n"));
                OS_REPORT(
                        OS_API_INFO,
                        "qp_qosProvider::processElement",
                        0,
                        "Element <" PROFILE_TAG "> has no 'name' attribute, which is mandatory...");
                return QP_RESULT_PARSE_ERROR;
            }
            QP_TRACE(printf("%*sBEGIN " PROFILE_TAG " '%s'\n", level++ * 2, "", ctx.name));
            result = processContainedElements(cf_elementGetChilds(element), ctx);
            QP_TRACE(printf("%*sEND " PROFILE_TAG " '%s'\n", --level * 2, "", ctx.name));
            break; /* Only break when new scope has been determined. */
        }
        /* Fall-through intentional !! */
    case QP_SCOPE_PROFILE:
        if (strcmp(name, DPQOS_TAG) == 0) {
            result = prepareQosSample(DPQOS_TAG, &ctx.qosProvider->dpQosAttr, &ctx);
        } else if (strcmp(name, TQOS_TAG) == 0) {
            result = prepareQosSample(TQOS_TAG, &ctx.qosProvider->tQosAttr, &ctx);
        } else if (strcmp(name, PUBQOS_TAG) == 0) {
            result = prepareQosSample(PUBQOS_TAG, &ctx.qosProvider->pubQosAttr, &ctx);
        } else if (strcmp(name, SUBQOS_TAG) == 0) {
            result = prepareQosSample(SUBQOS_TAG, &ctx.qosProvider->subQosAttr, &ctx);
        } else if (strcmp(name, DWQOS_TAG) == 0) {
            result = prepareQosSample(DWQOS_TAG, &ctx.qosProvider->dwQosAttr, &ctx);
        } else if (strcmp(name, DRQOS_TAG) == 0) {
            result = prepareQosSample(DRQOS_TAG, &ctx.qosProvider->drQosAttr, &ctx);
        } else {
            return QP_RESULT_UNEXPECTED_ELEMENT;
        }

        if(result == QP_RESULT_OK){
            const char * profileName = ctx.name;
            ctx.name = NULL;

            /* Fetch the (required according to Annex C of the
             * 'DDS for Lightweight CCM, v1.1' specification, but optional
             * according to the rest of the document) name of the
             * current qos and store it in the current ctx.
             * In this case we ignore the schema and follow the rest of the
             * specification (including Annex D). */
            if((result = processContainedAttributes(cf_elementGetAttributes(element), &ctx)) != QP_RESULT_OK){
                return result;
            }
            QP_TRACE(printf("%*sBEGIN %s (profile: '%s', name: '%s')\n",
                    level++ * 2, "",
                    name,
                    profileName ? profileName : "(null)",
                    ctx.name ? ctx.name : "(null)"));
            if((result = processContainedElements(cf_elementGetChilds(element), ctx)) != QP_RESULT_OK){
                return result;
            }

            {
                char * qosName;
                os_size_t len = 0;
                os_int32 tableRes;

                if(profileName){
                    len += strlen(profileName);
                } else {
                    len += strlen(ctx.qosProvider->defaultProfile);
                }
                if(ctx.name) {
                    len += strlen(ctx.name);
                    len += 2;
                }

                if((qosName = os_malloc(len + 1)) == NULL){
                    OS_REPORT_1(OS_API_INFO,
                            "processElement",
                            0,
                            "Out of memory. Memory claim of %u bytes denied.",
                            len + 1);
                    return QP_RESULT_OUT_OF_MEMORY; /* TODO: resources-cleanup */
                }

                if(profileName){
                    sprintf(qosName, "%s%s%s", profileName, ctx.name ? "::" : "", ctx.name ? ctx.name : "");
                } else {
                    if(*ctx.qosProvider->defaultProfile == '\0'){
                        sprintf(qosName, "%s", ctx.name ? ctx.name : "");
                    } else {
                        sprintf(qosName, "%s%s%s", ctx.qosProvider->defaultProfile, ctx.name ? "::" : "", ctx.name ? ctx.name : "");
                    }
                }

                if((tableRes = ut_tableInsert(ctx.qosTable, qosName, ctx.qosSample /* transfer ref */)) == 0){
                    OS_REPORT_1(OS_INFO,
                            "qp_qosProvider::processElement",
                            0,
                            "Identification for QoS '%s' is not unique...", /* TODO: report filename as well? */
                            qosName);
                    QP_TRACE(printf("%*sEND %s (%p), not stored under '%s'\n", --level * 2, "", name, (void *)ctx.qosSample, qosName));
                    c_free(ctx.qosSample);
                    os_free(qosName);
                } else {
                    QP_TRACE(printf("%*sEND %s (%p), stored under '%s'\n", --level * 2, "", name, (void *)ctx.qosSample, qosName));
                }
            }
            c_free(ctx.currentField);
            c_free(ctx.currentFieldType);
        }
        break;
    case QP_SCOPE_QOSPOLICY:
    {
        c_type at;
        c_field unscopedField = c_fieldNew(ctx.currentFieldType, name);

        if (unscopedField) {
            QP_TRACE(printf("%*sBEGIN field '%s'\n", level++ * 2, "", name));
            ctx.currentFieldType = c_fieldType(unscopedField);
            if (ctx.currentField) {
                ctx.currentField = c_fieldConcat(ctx.currentField, unscopedField);
                c_free(unscopedField);
            } else {
                ctx.currentField = unscopedField;
            }

            at =  c_typeActualType(ctx.currentFieldType);
            if(c_baseObjectKind(at) == M_COLLECTION && c_collectionTypeKind(at) == C_SEQUENCE) {
                if(c_baseObjectKind(c_collectionTypeSubType(at)) == M_COLLECTION && c_collectionTypeKind(c_collectionTypeSubType(at)) == C_STRING){
                QP_TRACE(printf("%*sBEGIN SEQUENCE<%s> field '%s'\n", level++ * 2, "", c_metaName(c_collectionTypeSubType(at)), name));
                    {
                        c_iter elements = cf_elementGetChilds(element);
                        qp_result result = QP_RESULT_OK;
                        cf_node node;
                        c_string *strings = NULL;
                        c_ulong used, len;

                        len = used = 0;

                        while ((node = (cf_node) c_iterTakeFirst(elements)) != NULL && result == QP_RESULT_OK)
                        {
                            cf_kind kind = cf_nodeKind(node);
                            if(kind == CF_ELEMENT && strcmp("element", cf_nodeGetName(node)) == 0){
                                c_iter stringElement = cf_elementGetChilds(cf_element(node));
                                cf_node stringNode;

                                while ((stringNode = (cf_node) c_iterTakeFirst(stringElement)) != NULL && result == QP_RESULT_OK) {
                                    if(cf_nodeKind(stringNode) == CF_DATA){
                                        if(used == len) {
                                            c_string *newStrings;

                                            len += 16;
                                            if((newStrings = os_realloc(strings, sizeof(*newStrings) * len)) == NULL) {
                                                result = QP_RESULT_OUT_OF_MEMORY;
                                                os_free(strings);
                                                used = 0;
                                            } else {
                                                strings = newStrings;
                                            }
                                        }

                                        if(result == QP_RESULT_OK){
                                            strings[used++] = cf_dataValue((cf_data(stringNode))).is.String;
                                        }
                                    }
                                }
                                c_iterFree(stringElement);
                            } else {
                                /* TODO: report malformed XML */
                                result = QP_RESULT_PARSE_ERROR;
                            }
                        }
                        c_iterFree(elements);

                        if(used > 0) {
                            c_base base = c_getBase(at);
                            c_ulong i;
                            c_string *seq = (c_string*)c_newBaseArrayObject(c_collectionType(at), used);

                            for(i = 0; i < used; i++){
                                seq[i] = c_stringNew(base, strings[used - i - 1]);
                                QP_TRACE(printf("%*s'%s'\n", (level+1) * 2, "", seq[i]));
                            }
                            os_free(strings);

                            c_fieldAssign(ctx.currentField, ctx.qosSample, c_objectValue(seq));
                            c_free(seq);
                        }
                    }
                    QP_TRACE(printf("%*sEND SEQUENCE<%s> field '%s'\n", --level * 2, "", c_metaName(c_collectionTypeSubType(at)), name));
                } else {
                    QP_TRACE(printf("%*sSKIPPING SEQUENCE<%s> field '%s'\n", level * 2, "", c_metaName(c_collectionTypeSubType(at)), name));
                }
            } else {
                if((result = processContainedElements(cf_elementGetChilds(element), ctx)) != QP_RESULT_OK){
                    return result;
                }
            }
            QP_TRACE(printf("%*sEND field '%s'\n", --level * 2, "", name));
            c_free(ctx.currentField);
        } else {
            QP_TRACE(printf("ERROR: Unrecognized element '%s' inside qosPolicy field '%s'\n", name, c_fieldName(ctx.currentField)));
            result = QP_RESULT_UNKNOWN_ELEMENT;
            OS_REPORT_2(
                    OS_API_INFO,
                    "qp_qosProvider::processElement",
                    0,
                    "Unrecognized element (\"%s\") inside qosPolicy field \"%s\"...",
                    name,
                    c_fieldName(ctx.currentField));
        }
        break;
    }
    default:
        assert(0);
    }

    return result;
}

static qp_result
checkQosProviderAttrIsSane (
    const C_STRUCT(qp_qosProviderInputAttr) *attr)
{
    if(!attr) goto err_attr;
    if(!attr->participantQos.copyOut) goto err_attr;
    if(!attr->topicQos.copyOut) goto err_attr;
    if(!attr->publisherQos.copyOut) goto err_attr;
    if(!attr->dataWriterQos.copyOut) goto err_attr;
    if(!attr->subscriberQos.copyOut) goto err_attr;
    if(!attr->dataReaderQos.copyOut) goto err_attr;

    return QP_RESULT_OK;
err_attr:
    return QP_RESULT_ILL_PARAM;
}

static void
unloadEntityQosAttributes(
        qp_entityAttr attr)
{
    assert(attr);

    c_free(attr->defaultQosTmplt);
    c_free(attr->qosSampleType);
    ut_tableFree(attr->qosTable, qp_qosTablefreeKey, NULL, qp_qosTablefreeData, NULL);
}

static void
unloadQosProviderQosAttributes(
        qp_qosProvider _this)
{
    unloadEntityQosAttributes(&_this->drQosAttr);
    unloadEntityQosAttributes(&_this->dwQosAttr);
    unloadEntityQosAttributes(&_this->subQosAttr);
    unloadEntityQosAttributes(&_this->pubQosAttr);
    unloadEntityQosAttributes(&_this->tQosAttr);
    unloadEntityQosAttributes(&_this->dpQosAttr);
}

static qp_result
loadEntityQosAttributes(
        qp_qosProvider _this,
        qp_fnLoadQosSampleType loadQosSample,
        const C_STRUCT(qp_qosInputAttr) *inputAttr,
        qp_shallowClone shallowClone,
        c_voidp defaultQos,
        qp_entityAttr outputAttr)
{
    assert(_this);
    assert(loadQosSample);
    assert(inputAttr);
    assert(shallowClone);
    assert(defaultQos);
    assert(outputAttr);

    if((outputAttr->qosTable = (ut_table)ut_tableNew(qp_qosCompareByName, NULL)) == NULL){
        OS_REPORT(OS_API_INFO, "loadEntityQosAttributes", 0, "Out of memory. Failed to allocate storage for QoS.");
        goto err_table;
    }
    if((outputAttr->qosSampleType = c_type(loadQosSample(_this->baseAddr))) == NULL){
        OS_REPORT(OS_API_INFO, "loadEntityQosAttributes", 0, "Out of memory. Failed to allocate QoS sample-type.");
        goto err_sampleType;
    }
    if((outputAttr->defaultQosTmplt = c_new(outputAttr->qosSampleType)) == NULL){
        OS_REPORT(OS_API_INFO, "loadEntityQosAttributes", 0, "Out of memory. Failed to allocate QoS template.");
        goto err_qosTmplt;
    }

    shallowClone(defaultQos, outputAttr->defaultQosTmplt);

    outputAttr->copyOut = inputAttr->copyOut;
    return QP_RESULT_OK;

/* Error handling */
err_qosTmplt:
    c_free(outputAttr->qosSampleType);
err_sampleType:
    /* Guaranteed empty, so no free-functions needed */
    ut_tableFree(outputAttr->qosTable, NULL, NULL, NULL, NULL);
err_table:
    return QP_RESULT_OUT_OF_MEMORY;
}

static void
qp_NamedDomainParticipantQos__shallowClone(
        void *_from,
        void *_to)
{
    struct _DDS_NamedDomainParticipantQos *from = (struct _DDS_NamedDomainParticipantQos *)_from;
    struct _DDS_NamedDomainParticipantQos *to = (struct _DDS_NamedDomainParticipantQos *)_to;

    *to = *from;
}

static void
qp_NamedTopicQos__shallowClone(
        void *_from,
        void *_to)
{
    struct _DDS_NamedTopicQos *from = (struct _DDS_NamedTopicQos *)_from;
    struct _DDS_NamedTopicQos *to = (struct _DDS_NamedTopicQos *)_to;

    *to = *from;
}

static void
qp_NamedPublisherQos__shallowClone(
        void *_from,
        void *_to)
{
    struct _DDS_NamedPublisherQos *from = (struct _DDS_NamedPublisherQos *)_from;
    struct _DDS_NamedPublisherQos *to = (struct _DDS_NamedPublisherQos *)_to;

    *to = *from;
}

static void
qp_NamedSubscriberQos__shallowClone(
        void *_from,
        void *_to)
{
    struct _DDS_NamedSubscriberQos *from = (struct _DDS_NamedSubscriberQos *)_from;
    struct _DDS_NamedSubscriberQos *to = (struct _DDS_NamedSubscriberQos *)_to;

    *to = *from;
}

static void
qp_NamedDataWriterQos__shallowClone(
        void *_from,
        void *_to)
{
    struct _DDS_NamedDataWriterQos *from = (struct _DDS_NamedDataWriterQos *)_from;
    struct _DDS_NamedDataWriterQos *to = (struct _DDS_NamedDataWriterQos *)_to;

    *to = *from;
}

static void
qp_NamedDataReaderQos__shallowClone(
        void *_from,
        void *_to)
{
    struct _DDS_NamedDataReaderQos *from = (struct _DDS_NamedDataReaderQos *)_from;
    struct _DDS_NamedDataReaderQos *to = (struct _DDS_NamedDataReaderQos *)_to;

    *to = *from;
}

static qp_result
loadQosProviderQosAttributes(
        qp_qosProvider _this,
        const C_STRUCT(qp_qosProviderInputAttr) *attr)
{
    qp_result result;
    c_voidp qos;

    assert(_this);
    assert(checkQosProviderAttrIsSane (attr) == QP_RESULT_OK);

    qos = (c_voidp)&qp_NamedDomainParticipantQos_default; /* Discard const */
    result = loadEntityQosAttributes(_this, __DDS_NamedDomainParticipantQos__load, &attr->participantQos, qp_NamedDomainParticipantQos__shallowClone, qos, &_this->dpQosAttr);
    if (result == QP_RESULT_OK) {
        qos = (c_voidp)&qp_NamedTopicQos_default; /* Discard const */
        result = loadEntityQosAttributes(_this, __DDS_NamedTopicQos__load, &attr->topicQos, qp_NamedTopicQos__shallowClone, qos, &_this->tQosAttr);
    }
    if (result == QP_RESULT_OK) {
        qos = (c_voidp)&qp_NamedPublisherQos_default; /* Discard const */
        result = loadEntityQosAttributes(_this, __DDS_NamedPublisherQos__load, &attr->publisherQos, qp_NamedPublisherQos__shallowClone, qos, &_this->pubQosAttr);
    }
    if (result == QP_RESULT_OK) {
        qos = (c_voidp)&qp_NamedSubscriberQos_default; /* Discard const */
        result = loadEntityQosAttributes(_this, __DDS_NamedSubscriberQos__load, &attr->subscriberQos, qp_NamedSubscriberQos__shallowClone, qos, &_this->subQosAttr);
    }
    if (result == QP_RESULT_OK) {
        qos = (c_voidp)&qp_NamedDataWriterQos_default; /* Discard const */
        result = loadEntityQosAttributes(_this, __DDS_NamedDataWriterQos__load, &attr->dataWriterQos, qp_NamedDataWriterQos__shallowClone, qos, &_this->dwQosAttr);
    }
    if (result == QP_RESULT_OK) {
        qos = (c_voidp)&qp_NamedDataReaderQos_default; /* Discard const */
        result = loadEntityQosAttributes(_this, __DDS_NamedDataReaderQos__load, &attr->dataReaderQos, qp_NamedDataReaderQos__shallowClone, qos, &_this->drQosAttr);
    }
    if (result != QP_RESULT_OK) {
        unloadQosProviderQosAttributes(_this);
    }

    return result;
}



/**************************************************************
 * constructor/destructor
 **************************************************************/

qp_qosProvider
qp_qosProviderNew(
    const c_char *uri,
    const c_char *profile,
    const C_STRUCT(qp_qosProviderInputAttr) *attr)
{
    cfgprs_status s;
    qp_qosProvider _this;
    const c_char *normalizedProfile = "";

    if(uri == NULL){
        OS_REPORT(OS_API_INFO, "qp_qosProviderNew", 0, "Illegal parameter; parameter 'uri' may not be NULL.");
        goto err_illegalParameter;
    }
    if(checkQosProviderAttrIsSane (attr) != QP_RESULT_OK){
        OS_REPORT(OS_API_INFO, "qp_qosProviderNew", 0, "Illegal parameter; parameter 'attr' may not be NULL and all fields must be initialized.");
        goto err_illegalParameter;
    }

    /* First make sure the user layer is initialized to allow Thread-specific
     * memory for error reporting. */
    if (u_userInitialise() != U_RESULT_OK) {
        OS_REPORT(OS_API_INFO, "qp_qosProviderNew", 0, "Error. Initialisation of user-layer failed.");
        goto err_userInitialize;
    }
    if((_this = os_malloc (C_SIZEOF(qp_qosProvider))) == NULL){
        OS_REPORT_1(OS_API_INFO, "qp_qosProviderNew", 0, "Out of memory. Memory claim of %u bytes denied.", C_SIZEOF(qp_qosProvider));
        goto err_malloc;
    }
    memset(_this, 0, C_SIZEOF(qp_qosProvider));

    if(profile){
        normalizedProfile = strncmp("::", profile, strlen("::")) == 0 ? profile + strlen("::") : profile;
    }
    _this->defaultProfile = os_strdup(normalizedProfile);
    if(!_this->defaultProfile){
        OS_REPORT_1(OS_API_INFO, "qp_qosProviderNew", 0, "Out of memory. Memory claim of %u bytes denied.", strlen(normalizedProfile) + 1);
        goto err_strdup;
    }
    if((s = cfg_parse_ospl(uri, &_this->rootElement)) != CFGPRS_OK){
        assert((s == CFGPRS_NO_INPUT) || (s == CFGPRS_ERROR));
        if (s == CFGPRS_NO_INPUT) {
            OS_REPORT_1(OS_API_INFO, "qp_qosProviderNew", 0, "Could not open XML file referred to by '%s'.", uri);
        } else {
            OS_REPORT_1(OS_API_INFO, "qp_qosProviderNew", 0, "The XML file referred to by '%s' does not parse correctly.", uri);
        }
        goto err_cfg_parse;
    }

    /* Create a heap database */
    if((_this->baseAddr = c_create("QOSProvider", NULL, 0, 0)) == NULL){
        OS_REPORT(OS_API_INFO, "qp_qosProviderNew", 0, "Out of memory. Failed to allocate heap database.");
        goto err_c_create;
    }

    if (loadQosProviderQosAttributes(_this, attr) != QP_RESULT_OK){
        OS_REPORT(OS_API_INFO, "qp_qosProviderNew", 0, "Out of memory. Failed to load QosProvider QosAttributes in heap database.");
        goto err_loadAttributes;
    }

    /* Parse the tree */
    {
        C_STRUCT(qp_parseContext) ctx;

        ctx.qosProvider = _this;
        ctx.currentField = NULL;
        ctx.currentFieldType = NULL;
        ctx.qosSample = NULL;
        ctx.scope = QP_SCOPE_NONE;
        ctx.name = NULL;
        ctx.qosTable = NULL;
        if (processElement(_this->rootElement, ctx) != QP_RESULT_OK) {
            /* ErrorReport already generated during recursive parse tree walk. */
            goto err_process;
        }
    }

    return _this;

/* Error handling */
err_process:
    unloadQosProviderQosAttributes(_this);
err_loadAttributes:
    c_destroy(_this->baseAddr);
err_c_create:
    if (_this->rootElement) {
        cf_elementFree(_this->rootElement);
    }
err_cfg_parse:
    os_free(_this->defaultProfile);
err_strdup:
    os_free(_this);
err_malloc:
    /* No undo for u_userInitialise */
err_userInitialize:
err_illegalParameter:
    return NULL;
}

void
qp_qosProviderFree(
    qp_qosProvider _this)
{
    if (_this) {
        unloadQosProviderQosAttributes(_this);
        if (_this->baseAddr) {
            c_destroy(_this->baseAddr);
        }
        if (_this->rootElement) {
            cf_elementFree(_this->rootElement);
        }
        os_free(_this->defaultProfile);
        os_free (_this);
    }
}

 /**************************************************************
  * Public functions
  **************************************************************/

qp_result
qp_qosProviderGetParticipantQos(
     qp_qosProvider _this,
     const c_char *id,
     c_voidp qos)
{
    c_object q;

    assert(_this);
    assert(_this->defaultProfile);
    assert(qos);

    if((q = qp_qosTableLookup(_this->dpQosAttr.qosTable, _this->defaultProfile, id)) != NULL){
        _this->dpQosAttr.copyOut(q, qos);
        return QP_RESULT_OK;
    } else {
        return QP_RESULT_NO_DATA;
    }
}

qp_result
qp_qosProviderGetParticipantQosType(
        qp_qosProvider _this,
        c_type *type)
{
    assert(_this);
    assert(type);

    *type = c_keep(_this->dpQosAttr.qosSampleType);

    return *type ? QP_RESULT_OK : QP_RESULT_NO_DATA;
}

qp_result
qp_qosProviderGetTopicQos(
     qp_qosProvider _this,
     const c_char *id,
     c_voidp qos)
{
    c_object q;

    assert(_this);
    assert(_this->defaultProfile);
    assert(qos);

    if((q = qp_qosTableLookup(_this->tQosAttr.qosTable, _this->defaultProfile, id)) != NULL){
        _this->tQosAttr.copyOut(q, qos);
        return QP_RESULT_OK;
    } else {
        return QP_RESULT_NO_DATA;
    }
}

qp_result
qp_qosProviderGetTopicQosType(
        qp_qosProvider _this,
        c_type *type)
{
    assert(_this);
    assert(type);

    *type = c_keep(_this->tQosAttr.qosSampleType);

    return *type ? QP_RESULT_OK : QP_RESULT_NO_DATA;
}

qp_result
qp_qosProviderGetPublisherQos(
     qp_qosProvider _this,
     const c_char *id,
     c_voidp qos)
{
    c_object q;

    assert(_this);
    assert(_this->defaultProfile);
    assert(qos);

    if((q = qp_qosTableLookup(_this->pubQosAttr.qosTable, _this->defaultProfile, id)) != NULL){
        _this->pubQosAttr.copyOut(q, qos);
        return QP_RESULT_OK;
    } else {
        return QP_RESULT_NO_DATA;
    }
}

qp_result
qp_qosProviderGetPublisherQosType(
        qp_qosProvider _this,
        c_type *type)
{
    assert(_this);
    assert(type);

    *type = c_keep(_this->pubQosAttr.qosSampleType);

    return *type ? QP_RESULT_OK : QP_RESULT_NO_DATA;
}

qp_result
qp_qosProviderGetDataWriterQos(
     qp_qosProvider _this,
     const c_char *id,
     c_voidp qos)
{
    c_object q;

    assert(_this);
    assert(_this->defaultProfile);
    assert(qos);

    if((q = qp_qosTableLookup(_this->dwQosAttr.qosTable, _this->defaultProfile, id)) != NULL){
        _this->dwQosAttr.copyOut(q, qos);
        return QP_RESULT_OK;
    } else {
        return QP_RESULT_NO_DATA;
    }
}

qp_result
qp_qosProviderGetDataWriterQosType(
        qp_qosProvider _this,
        c_type *type)
{
    assert(_this);
    assert(type);

    *type = c_keep(_this->dwQosAttr.qosSampleType);

    return *type ? QP_RESULT_OK : QP_RESULT_NO_DATA;
}

qp_result
qp_qosProviderGetSubscriberQos(
     qp_qosProvider _this,
     const c_char *id,
     c_voidp qos)
{
    c_object q;

    assert(_this);
    assert(_this->defaultProfile);
    assert(qos);

    if((q = qp_qosTableLookup(_this->subQosAttr.qosTable, _this->defaultProfile, id)) != NULL){
        _this->subQosAttr.copyOut(q, qos);
        return QP_RESULT_OK;
    } else {
        return QP_RESULT_NO_DATA;
    }
}

qp_result
qp_qosProviderGetSubscriberQosType(
        qp_qosProvider _this,
        c_type *type)
{
    assert(_this);
    assert(type);

    *type = c_keep(_this->subQosAttr.qosSampleType);

    return *type ? QP_RESULT_OK : QP_RESULT_NO_DATA;
}

qp_result
qp_qosProviderGetDataReaderQos(
     qp_qosProvider _this,
     const c_char *id,
     c_voidp qos)
{
    c_object q;

    assert(_this);
    assert(_this->defaultProfile);
    assert(qos);

    if((q = qp_qosTableLookup(_this->drQosAttr.qosTable, _this->defaultProfile, id)) != NULL){
        _this->drQosAttr.copyOut(q, qos);
        return QP_RESULT_OK;
    } else {
        return QP_RESULT_NO_DATA;
    }
}

qp_result
qp_qosProviderGetDataReaderQosType(
        qp_qosProvider _this,
        c_type *type)
{
    assert(_this);
    assert(type);

    *type = c_keep(_this->drQosAttr.qosSampleType);

    return *type ? QP_RESULT_OK : QP_RESULT_NO_DATA;
}

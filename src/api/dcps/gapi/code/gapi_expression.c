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
#include "v_dataViewInstance.h"
#include "v_dataReaderInstance.h"
#include "v_dataReader.h"
#include "v_dataView.h"
#include "u_query.h"

#include "v_state.h"
#include "v_topic.h"
#include "c_metabase.h"
#include "q_helper.h"

#include "gapi_expression.h"
#include "gapi_kernel.h"
#include "gapi_vector.h"
#include "gapi_dataReaderView.h"

#include "os_stdlib.h"
#include "os_abstract.h"
#include "os_report.h"

static gapi_boolean
buildParameterInfo (
    gapi_expression e);

static gapi_boolean
resolveFieldType (
    gapi_expression e);

static gapi_boolean
validParameterUsage (
    gapi_expression e);

static gapi_boolean
validQueryParameters (
    gapi_expression e,
    const gapi_stringSeq  *plist,
    c_value         *args);

static void
printParamRelation (
    gapi_vector info);

void
gapi_parserInit (
    void);
void
gapi_parserDeinit (
    void);

static gapi_long
getMaxParameterNumber (
    q_expr    expr,
    gapi_long max);


typedef enum {
    PARAM_KIND_CONST,
    PARAM_KIND_VAR,
    PARAM_KIND_FIELD
} ParamKind;

typedef enum {
    PARAM_TYPE_UNDEFINED,
    PARAM_TYPE_BOOLEAN,
    PARAM_TYPE_CHAR,
    PARAM_TYPE_OCTET,
    PARAM_TYPE_SHORT,
    PARAM_TYPE_USHORT,
    PARAM_TYPE_LONG,
    PARAM_TYPE_ULONG,
    PARAM_TYPE_LONGLONG,
    PARAM_TYPE_ULONGLONG,
    PARAM_TYPE_FLOAT,
    PARAM_TYPE_DOUBLE,
    PARAM_TYPE_STRING,
    PARAM_TYPE_ENUM
} ParamType;

typedef struct ParamDesc {
    ParamKind kind;
    ParamType ptype;
    q_expr    expr;
    c_type    ftype;
    c_long    pnum;
} ParamDesc;

typedef struct RelationInfo {
    ParamDesc p1;
    ParamDesc p2;
} RelationInfo;

#define RELATION_INFO_AT(v,i) ((RelationInfo *) gapi_vectorAt(v, i))


C_STRUCT(gapi_expression) {
    q_expr      expr;
    gapi_long   maxParmNum;
    gapi_vector pinfo;
    c_type      type;
};

gapi_expression
gapi_expressionNew (
    gapi_char *queryString)
{
    gapi_expression e;

    e = (gapi_expression)os_malloc(C_SIZEOF(gapi_expression));
    if ( e ) {
        e->pinfo = NULL;
        if ( queryString ) {
            e->expr = gapi_parseExpression(queryString);
            if ( e->expr ) {
                e->maxParmNum = getMaxParameterNumber(e->expr, -1);
                if ( buildParameterInfo(e) ) {
                    if ( !validParameterUsage(e) ) {
                        gapi_expressionFree(e);
                        e = NULL;
                    }
                } else {
                    gapi_expressionFree(e);
                    e = NULL;
                }
            } else {
                os_free(e);
                e = NULL;
            }
        } else {
            e->expr       = NULL;
            e->maxParmNum = -1;
        }
    } else {
        OS_REPORT(OS_ERROR,
                  "SQL expression parser", 0,
                  "Memory allocation failed");
    }

    return e;
}

void
gapi_expressionFree (
    gapi_expression e)
{
    if ( e->expr ) {
        q_dispose(e->expr);
    }
    if ( e->pinfo ) {
        gapi_vectorFree(e->pinfo);
    }
    os_free(e);
}

void
gapi_expressionInitParser (
    void)
{
    gapi_parserInit();
}

void
gapi_expressionDeinitParser (
    void)
{
    gapi_parserDeinit();
}

static gapi_boolean
createAndValidateParameters (
    gapi_expression e,
    const gapi_stringSeq *parms,
    c_value **args)
{
    gapi_boolean valid = FALSE;
    gapi_long max,size;
    gapi_unsigned_long i;

    assert(e);

    *args = NULL;
    max = e->maxParmNum;

    if ( max >= 0 ) {
        if ( parms ) {
            if ( parms->_length >= (gapi_unsigned_long)(max + 1) ) {
                size = parms->_length * sizeof(c_value);
                *args = (c_value *)os_malloc(size);
                if ( *args ) {
                    for (i = 0; i < parms->_length; i++) {
                        (*args)[i] = c_undefinedValue();
                    }
                    valid = validQueryParameters(e, parms, *args);
                    if ( !valid ) {
                        OS_REPORT(OS_ERROR,
                                  "SQL expression validate parameters", 0,
                                  "Parameters not valid");
                        os_free(*args);
                    }
                }
            } else {
                OS_REPORT(OS_ERROR,
                          "SQL expression validate parameters", 0,
                          "Wrong number of parameters");
            }
        } else {
            OS_REPORT(OS_ERROR,
                      "SQL expression validate parameters", 0,
                      "no parameters specified");
        }
    } else {
        valid = TRUE;
    }

    return valid;
}

static void
getReaderType (
    v_entity e,
    void *arg)
{
    v_dataReader r;
    v_topic topic;
    c_type t;
    c_type *result = (c_type *)arg;
    c_field f;

    if ( v_objectKind(e) == K_DATAVIEW) {
        v_dataView v = v_dataView(e);
        r  = v->reader;
    } else {
        r = v_dataReader(e);
    }

    topic = v_dataReaderGetTopic(r);
    t = v_topicMessageType(topic);
    f = c_fieldNew(t, "userData");

    if ( f ) {
        *result = c_fieldType(f);
        c_free(f);
    }
}

u_query
gapi_expressionCreateQuery (
    gapi_expression expression,
    u_reader        reader,
    const c_char   *queryName,
    gapi_stringSeq *parameters)
{
    c_value *args  = NULL;
    u_query  query = NULL;
    c_type   type  = NULL;
    gapi_boolean valid = TRUE;

    assert(expression);
    assert(reader);

    if ( expression->pinfo ) {

        u_entityWriteAction(u_entity(reader), getReaderType, &type);

        if ( type ) {
            expression->type = type;
            valid = resolveFieldType(expression);
        }
    }

    if ( valid && expression->expr ) {
        if ( createAndValidateParameters(expression, parameters, &args) ) {
            query = u_queryNew(reader, queryName, expression->expr, args);
            if ( !query ) {
                OS_REPORT(OS_ERROR,
                          "Creation of query", 0,
                          "Creation of query failed");
            }
            os_free(args);
            q_dispose(expression->expr);
            expression->expr = NULL;
        }
    }

    return query;
}

gapi_returnCode_t
gapi_expressionSetQueryArgs (
    gapi_expression expression,
    u_query         query,
    const gapi_stringSeq *parameters)
{
    gapi_returnCode_t result = GAPI_RETCODE_BAD_PARAMETER;
    c_value *args  = NULL;
    u_result uResult;

    assert(expression);
    assert(query);
    assert(parameters);

    if ( createAndValidateParameters(expression, parameters, &args) ) {
        uResult = u_querySet(query, args);
        if ( uResult != U_RESULT_OK ) {
            OS_REPORT(OS_ERROR,
                      "Set parameters on query", 0,
                      "Set parameters failed");
        }
        result = kernelResultToApiResult(uResult);
        os_free(args);
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }

    return result;
}


struct conditionArg {
    const c_char    *query;
    gapi_expression  expr;
};


static void
actionQueryCondition(
    v_entity e,
    c_voidp arg)
{
    struct conditionArg *ca = (struct conditionArg *)arg;
    q_expr expr;

    ca->expr = gapi_expressionNew(NULL);

    if ( ca->expr ) {
        expr = gapi_parseExpression(ca->query);
        if ( expr ) {
            ca->expr->expr = expr;
            ca->expr->maxParmNum = getMaxParameterNumber(expr, -1);
            if ( buildParameterInfo(ca->expr) ) {
                if ( !validParameterUsage(ca->expr) ) {
                    gapi_expressionFree(ca->expr);
                    ca->expr = NULL;
                }
            } else {
                gapi_expressionFree(ca->expr);
                ca->expr = NULL;
            }
        } else {
            gapi_expressionFree(ca->expr);
            ca->expr = NULL;
        }
    }
}

gapi_expression
gapi_createQueryExpression (
    u_entity entity,
    const c_char *query)
{
    struct conditionArg ca;

    ca.query = query;
    ca.expr  = NULL;

    u_entityAction(entity, actionQueryCondition, &ca);

    return ca.expr;
}

c_value
gapi_stringValue (
    const char *s)
{
    return c_stringValue((c_string)s);
}

static gapi_long
getMaxParameterNumber (
    q_expr expr,
    gapi_long max)
{
    c_longlong pn = 0;
    q_list list;

    switch ( q_getKind(expr) ) {
    case T_VAR:
        pn = q_getVar(expr);
        if ( pn > max ) {
            max = (gapi_long)pn;
        }
    break;
    case T_FNC:
        list = q_getLst(expr, 0);
        while ( list ) {
            max = getMaxParameterNumber(q_element(list), max);
            list = q_next(list);
        }
    break;
    default:
    break;
    }

    return max;
}


static void
fillExprParam (
    q_expr     e,
    ParamDesc *p)
{
    switch ( q_getKind(e) ) {
    case T_CHR:
        p->kind  = PARAM_KIND_CONST;
        p->expr  = e;
        p->ptype = PARAM_TYPE_CHAR;
    break;
    case T_INT:
        p->kind  = PARAM_KIND_CONST;
        p->expr  = e;
        p->ptype = PARAM_TYPE_LONG;
    break;
    case T_DBL:
        p->kind  = PARAM_KIND_CONST;
        p->expr  = e;
        p->ptype = PARAM_TYPE_DOUBLE;
    break;
    case T_STR:
        p->kind  = PARAM_KIND_CONST;
        p->expr  = e;
        p->ptype = PARAM_TYPE_STRING;
    break;
    case T_VAR:
        p->kind  = PARAM_KIND_VAR;
        p->expr  = e;
        p->ptype = PARAM_TYPE_UNDEFINED;
        p->pnum  = (c_long) q_getVar(e);
    break;
    case T_ID:
        p->kind  = PARAM_KIND_FIELD;
        p->expr  = e;
        p->ptype = PARAM_TYPE_UNDEFINED;
    break;
    case T_FNC:
        p->kind  = PARAM_KIND_FIELD;
        p->ptype = PARAM_TYPE_UNDEFINED;
        if ( q_getTag(e) == Q_EXPR_PROPERTY ) {
            p->expr  = e;
        } else {
            assert(0);
        }
    break;
    default:
        assert(0);
    break;
    }
}

static void
createParameterInfo (
    q_expr expr,
    gapi_vector info)
{
    q_list list;
    q_expr e1;
    q_expr e2;
    gapi_unsigned_long len;
    RelationInfo *rel;

    if ( q_getKind(expr) == T_FNC ) {
        switch ( q_getTag(expr) ) {
        case Q_EXPR_EQ:
        case Q_EXPR_NE:
        case Q_EXPR_LE:
        case Q_EXPR_LT:
        case Q_EXPR_GE:
        case Q_EXPR_GT:
        case Q_EXPR_LIKE:
            len = gapi_vectorGetLength(info);
            e1 = q_getPar(expr,0);
            e2 = q_getPar(expr,1);
            rel = (RelationInfo *) gapi_vectorAt(info, len);
            fillExprParam(e1, &rel->p1);
            fillExprParam(e2, &rel->p2);
        break;
        default:
            list = q_getLst(expr, 0);
            while ( list ) {
                createParameterInfo(q_element(list), info);
                list = q_next(list);
            }
        break;
        }
    }
}

static void
printParamRelation (
    gapi_vector info);


static ParamType
determineParamType (
    c_type t)
{
    ParamType pt = PARAM_TYPE_UNDEFINED;
    c_type    actual;

    actual = c_typeActualType(t);

    switch ( c_baseObject(actual)->kind ) {
    case M_PRIMITIVE:
        switch ( c_primitive(actual)->kind ) {
        case P_BOOLEAN:
            pt = PARAM_TYPE_BOOLEAN;
        break;
        case P_CHAR:
            pt = PARAM_TYPE_CHAR;
        break;
        case P_OCTET:
            pt = PARAM_TYPE_OCTET;
        break;
        case P_SHORT:
            pt = PARAM_TYPE_SHORT;
        break;
        case P_USHORT:
            pt = PARAM_TYPE_USHORT;
        break;
        case P_LONG:
            pt = PARAM_TYPE_LONG;
        break;
        case P_ULONG:
            pt = PARAM_TYPE_ULONG;
        break;
        case P_LONGLONG:
            pt = PARAM_TYPE_LONGLONG;
        break;
        case P_ULONGLONG:
            pt = PARAM_TYPE_ULONGLONG;
        break;
        case P_FLOAT:
            pt = PARAM_TYPE_FLOAT;
        break;
        case P_DOUBLE:
            pt = PARAM_TYPE_DOUBLE;
        break;
        default:
        break;
        }
    break;
    case M_COLLECTION:
        if ( c_collectionType(actual)->kind == C_STRING ) {
            pt = PARAM_TYPE_STRING;
        }
    break;
    case M_ENUMERATION:
        pt = PARAM_TYPE_ENUM;
    break;
    default:
    break;
    }

    return pt;
}

static gapi_boolean
validEnumValue (
    c_enumeration e,
    const c_char *value)
{
    gapi_boolean found = FALSE;
    gapi_long i;

    for ( i = 0; !found && (i < c_arraySize(e->elements)); i++ ) {
        c_metaObject  c = c_metaObject(e->elements[i]);
        c_char       *n = c_metaName(c);

        if ( strcmp(n, value) == 0 ) {
            found = TRUE;
        }
        c_free(n);
    }

    return found;
}


static gapi_boolean
isNatural (
    ParamType t)
{
    gapi_boolean result = FALSE;

    switch ( t ) {
    case PARAM_TYPE_SHORT:
    case PARAM_TYPE_USHORT:
    case PARAM_TYPE_LONG:
    case PARAM_TYPE_ULONG:
    case PARAM_TYPE_LONGLONG:
    case PARAM_TYPE_ULONGLONG:
        result = TRUE;
    break;
    default:
        result = FALSE;
    break;
    }

    return result;
}

static gapi_boolean
isUnsigned (
    ParamType t)
{
    gapi_boolean result = FALSE;

    switch ( t ) {
    case PARAM_TYPE_USHORT:
    case PARAM_TYPE_ULONG:
    case PARAM_TYPE_ULONGLONG:
        result = TRUE;
    break;
    default:
        result = FALSE;
    break;
    }

    return result;
}

static gapi_boolean
isFloat (
    ParamType t)
{
    gapi_boolean result = FALSE;

    switch ( t ) {
    case PARAM_TYPE_FLOAT:
    case PARAM_TYPE_DOUBLE:
        result = TRUE;
    break;
    default:
        result = FALSE;
    break;
    }

    return result;
}

static gapi_boolean
isNumber (
    ParamType t)
{
    gapi_boolean result = FALSE;

    if ( isNatural(t) || isFloat(t) ) {
        result = TRUE;
    }

    return result;
}

static gapi_boolean
splitEnumFullname (
    const c_char *fullname,
    c_char **enumName,
    c_char **enumValue)
{
    gapi_boolean result = FALSE;
    c_char *ptr;
    c_long size;

    ptr = strstr(fullname, "::");
    if ( ptr ) {
        size = (c_long) (ptr - fullname);
        *enumName = (c_char *) os_malloc(size + 1);
        os_strncpy(*enumName, fullname, size);
        (*enumName)[size] = '\0';
        *enumValue = ptr + 2;
        result = TRUE;
    }

    return result;
}


static gapi_boolean
checkFieldVersusField (
    ParamDesc *p1,
    ParamDesc *p2)
{
    gapi_boolean valid = FALSE;

    if ( p1->ptype == p2->ptype ) {
        if ( p1->ptype != PARAM_TYPE_UNDEFINED ) {
            valid = TRUE;
        }
    } else {
        if ( isNumber(p1->ptype) && isNumber(p2->ptype) ) {
            if ( isNatural(p1->ptype) && isNatural(p2->ptype) ) {
                valid = TRUE;
                if ( (isUnsigned(p1->ptype) && !isUnsigned(p2->ptype)) ||
                     (!isUnsigned(p1->ptype) && isUnsigned(p2->ptype))  ) {
                    OS_REPORT(OS_WARNING,
                              "SQL expression parser", 0,
                              "Compare signed with unsigned");
                }
            } else if ( isFloat(p1->ptype) && isFloat(p2->ptype) ) {
                valid = TRUE;
            } else if ( isFloat(p1->ptype) ) {
                valid = TRUE;
                OS_REPORT(OS_WARNING,
                      "SQL expression parser", 0,
                      "Compare float with integer type");
            } else {
                valid = TRUE;
            }
        } else if ( isNumber(p1->ptype) ) {
            valid = FALSE;
            OS_REPORT(OS_ERROR,
                      "SQL expression parser", 0,
                      "Compare number with not number");
        } else if ( isNumber(p2->ptype) ) {
            valid = FALSE;
            OS_REPORT(OS_ERROR,
                      "SQL expression parser", 0,
                      "Compare number with not number");
        } else {
            valid = FALSE;
            OS_REPORT(OS_ERROR,
                      "SQL expression parser", 0,
                      "Compare incompatible types");
        }
    }

    return valid;

}


static gapi_boolean
checkFieldVersusConst (
    ParamDesc *p1,
    ParamDesc *p2)
{
    gapi_boolean valid = FALSE;

    if ( (p1->ptype != PARAM_TYPE_CHAR) && (p1->ptype == p2->ptype) ) {
         valid = TRUE;
    } else {
        switch ( p1->ptype ) {
        case PARAM_TYPE_OCTET: valid = isNumber(p2->ptype); break;
        case PARAM_TYPE_SHORT: valid = isNumber(p2->ptype); break;
        case PARAM_TYPE_USHORT: valid = isNumber(p2->ptype); break;
        case PARAM_TYPE_LONG: valid = isNumber(p2->ptype); break;
        case PARAM_TYPE_ULONG: valid = isNumber(p2->ptype); break;
        case PARAM_TYPE_LONGLONG: valid = isNumber(p2->ptype); break;
        case PARAM_TYPE_ULONGLONG: valid = isNumber(p2->ptype); break;
        case PARAM_TYPE_FLOAT: valid = isNumber(p2->ptype); break;
        case PARAM_TYPE_DOUBLE: valid = isNumber(p2->ptype); break;
        case PARAM_TYPE_BOOLEAN:
            if ( p2->ptype == PARAM_TYPE_CHAR ) {
                c_char n = q_getChr(p2->expr);
                if ( (n == 0) || (n == 1) ) {
                    valid = TRUE;
                }
            }
        break;
        case PARAM_TYPE_CHAR:
            if ( p2->ptype == PARAM_TYPE_STRING ) {
                c_char *sval = q_getStr(p2->expr);
                if ( sval && (strlen(sval) < 2) ) {
                    valid = TRUE;
                }
            }
        break;
        case PARAM_TYPE_ENUM:
            if ( p2->ptype == PARAM_TYPE_STRING ) {
                c_char *enumValue = q_getStr(p2->expr);
                c_type actualType = c_typeActualType(p1->ftype);

                if ( validEnumValue(c_enumeration(actualType), enumValue) ) {
                    valid = TRUE;
                }
            }
            break;
            default:
                valid = FALSE;
        }
    }

    return valid;
}


static c_field
getFieldFromExpr (
    c_type t,
    q_expr e)
{
    c_field f    = NULL;
    c_char *name = NULL;
    gapi_boolean alloc = FALSE;

    if ( q_isId(e) ) {
        name = q_getId(e);
    } else if ( q_isFnc(e, Q_EXPR_PROPERTY) ) {
        name = q_propertyName(e);
        alloc = TRUE;
    } else {
        name = NULL;
    }

    if ( name ) {
        f = c_fieldNew(t, name);
        if ( alloc ) {
            os_free(name);
        }
    }

    return f;
}



static gapi_boolean
resolveFieldType (
    gapi_expression e)
{
    gapi_boolean valid = TRUE;
    gapi_unsigned_long i;
    c_field f = NULL;
    RelationInfo *rinfo = NULL;

    if ( e->pinfo ) {
        for ( i = 0; valid && (i < gapi_vectorGetLength(e->pinfo)); i++ ) {
            rinfo = RELATION_INFO_AT(e->pinfo, i);

            if ( rinfo->p1.kind == PARAM_KIND_FIELD ) {
                f = getFieldFromExpr(e->type, rinfo->p1.expr);

                if ( f ) {
                    rinfo->p1.ftype = c_fieldType(f);
                    rinfo->p1.ptype = determineParamType(rinfo->p1.ftype);
                } else {
                    valid = FALSE;
                }
                c_free(f);
            }

            if ( rinfo->p2.kind == PARAM_KIND_FIELD ) {
                f = getFieldFromExpr(e->type, rinfo->p2.expr);

                if ( f ) {
                    rinfo->p2.ftype = c_fieldType(f);
                    rinfo->p2.ptype = determineParamType(rinfo->p2.ftype);
                } else {
                    valid = FALSE;
                }
                c_free(f);
            }

            if ( rinfo->p1.kind == PARAM_KIND_VAR ) {
                rinfo->p1.ptype = rinfo->p2.ptype;
                rinfo->p1.ftype = rinfo->p2.ftype;
            } else if ( rinfo->p2.kind == PARAM_KIND_VAR ) {
                rinfo->p2.ptype = rinfo->p1.ptype;
                rinfo->p2.ftype = rinfo->p1.ftype;
            } else {
                /* nothing to do */
            }

            if ( rinfo->p1.kind == PARAM_KIND_FIELD ) {
                if ( rinfo->p2.kind == PARAM_KIND_FIELD ) {
                    valid = checkFieldVersusField(&rinfo->p1, &rinfo->p2);
                } else if ( rinfo->p2.kind == PARAM_KIND_CONST ) {
                    valid = checkFieldVersusConst(&rinfo->p1, &rinfo->p2);
                } else {
                    /* nothing to check */
                }
            } else if ( rinfo->p1.kind == PARAM_KIND_CONST ) {
                if ( rinfo->p2.kind == PARAM_KIND_FIELD ) {
                    valid = checkFieldVersusConst(&rinfo->p2, &rinfo->p1);
                }
            } else {
                /* nothing to check */
            }
        }
    }

    return valid;
}


static gapi_boolean
buildParameterInfo (
    gapi_expression e)
{
    gapi_boolean result = FALSE;
    gapi_vector v;

    v = gapi_vectorNew(0, 8, sizeof(RelationInfo));
    if ( v ) {
        createParameterInfo(e->expr, v);
        if (gapi_vectorGetLength(v) > 0) {
            e->pinfo = v;
        } else {
            gapi_vectorFree(v);
        }
        result = TRUE;
    } else {
        OS_REPORT(OS_ERROR,
                  "SQL expression parser", 0,
                  "Memory allocation failed");
        result = FALSE;
    }
    return result;
}

static gapi_boolean
stringIsNumber (
    const c_char *s)
{
    gapi_boolean isNumber = FALSE;
    gapi_long_long lvalue;
    gapi_double    dvalue;

    if ( s ) {
        if ( gapi_stringToLongLong(s, &lvalue) ) {
            isNumber = TRUE;
        } else if ( sscanf(s, "%lf", &dvalue) ) {
            isNumber = TRUE;
        } else {
            isNumber = FALSE;
        }
    }
    return isNumber;
}



static gapi_boolean
stringIsBoolean (
    const c_char *s)
{
    gapi_boolean valid = FALSE;

    if ( s ) {
        if ( (strcmp(s, "true") == 0) || (strcmp(s, "false") == 0) ||
             (strcmp(s, "TRUE") == 0) || (strcmp(s, "FALSE") == 0) ) {
            valid = TRUE;
        }
    }
    return valid;
}

static gapi_boolean
setAndValidateEnumValue (
    c_type        t,
    const c_char *s,
    c_value      *arg)
{
    gapi_boolean valid = FALSE;
    c_type actualType = c_typeActualType(t);
    if ( validEnumValue(c_enumeration(actualType), s) ) {
        valid = TRUE;
        *arg = gapi_stringValue(s);
    }

    return valid;
}

#define stringIsOctet     stringIsNumber
#define stringIsShort     stringIsNumber
#define stringIsUShort    stringIsNumber
#define stringIsLong      stringIsNumber
#define stringIsULong     stringIsNumber
#define stringIsLongLong  stringIsNumber
#define stringIsULongLong stringIsNumber
#define stringIsFloat     stringIsNumber
#define stringIsDouble    stringIsNumber

static gapi_boolean
parameterIsValid (
    ParamDesc *parm,
    gapi_char *value,
    c_value   *arg)
{
    gapi_boolean valid = FALSE;

    switch ( parm->ptype ) {
    case PARAM_TYPE_OCTET:     valid = stringIsOctet(value);     break;
    case PARAM_TYPE_SHORT:     valid = stringIsShort(value);     break;
    case PARAM_TYPE_USHORT:    valid = stringIsUShort(value);    break;
    case PARAM_TYPE_LONG:      valid = stringIsLong(value);      break;
    case PARAM_TYPE_ULONG:     valid = stringIsULong(value);     break;
    case PARAM_TYPE_LONGLONG:  valid = stringIsLongLong(value);  break;
    case PARAM_TYPE_ULONGLONG: valid = stringIsULongLong(value); break;
    case PARAM_TYPE_FLOAT:     valid = stringIsFloat(value);     break;
    case PARAM_TYPE_DOUBLE:    valid = stringIsDouble(value);    break;
    case PARAM_TYPE_BOOLEAN:
        valid = stringIsBoolean(value);
    break;
    case PARAM_TYPE_CHAR:
        if ( value && (strlen(value) < 2) ) {
            valid = TRUE;
        }
    break;
    case PARAM_TYPE_STRING:
        if ( value ) {
            valid = TRUE;
        }
    break;
    case PARAM_TYPE_ENUM:
        valid = setAndValidateEnumValue(parm->ftype, value, arg);
    break;
    default:
    break;
    }
    if ( valid && (parm->ptype != PARAM_TYPE_ENUM) ) {
        *arg = gapi_stringValue(value);
    }
    return valid;
}


static gapi_boolean
validParameterUsage (
    gapi_expression e)
{
    return TRUE;
}

static gapi_boolean
validQueryParameters (
    gapi_expression e,
    const gapi_stringSeq *plist,
    c_value *args)
{
    gapi_boolean valid = TRUE;
    gapi_unsigned_long i, length;
    gapi_unsigned_long n;

    if ( e->pinfo ) {
        length = gapi_vectorGetLength(e->pinfo);
        for ( i = 0; valid && (i < length); i++ ) {
            RelationInfo *rinfo;

            rinfo = RELATION_INFO_AT(e->pinfo, i);

            if ( rinfo->p1.kind == PARAM_KIND_VAR ) {
                n = (gapi_unsigned_long) rinfo->p1.pnum;
                if ( n < plist->_length ) {
                    valid = parameterIsValid(&rinfo->p1,
                                             plist->_buffer[n],
                                             &args[n]);
                    if ( !valid ) {
                        OS_REPORT_1(OS_ERROR,
                                    "SQL expression parameter check", 0,
                                    "parameter %d invalid type", n);
                    }
                } else {
                    valid = FALSE;
                }
            }

            if ( valid ) {
                if ( rinfo->p2.kind == PARAM_KIND_VAR ) {
                    n = (gapi_unsigned_long) rinfo->p2.pnum;
                    if ( n < plist->_length ) {
                        valid = parameterIsValid(&rinfo->p2,
                                                 plist->_buffer[n],
                                                 &args[n]);
                        if ( !valid ) {
                            OS_REPORT_1(OS_ERROR,
                                        "SQL expression parameter check", 0,
                                        "parameter %d invalid type", n);
                        }
                    } else {
                        valid = FALSE;
                    }
                }
            }
        }
    }
    return valid;
}

static void
testField (
    q_expr e)
{
    if ( q_getKind(e) == T_ID ) {
        printf("FIELD %s\n", q_getId(e));
    } else {
        printf("PROPERTY\n");
    }
}

static void
printParameterInfo (
    ParamDesc *p)
{
    switch ( p->kind ) {
    case PARAM_KIND_CONST:
        printf("const ");
    break;
    case PARAM_KIND_VAR:
        printf("var   ");
    break;
    case PARAM_KIND_FIELD:
        printf("field ");
    break;
    }

    switch ( p->ptype ) {
    case PARAM_TYPE_BOOLEAN:   printf("boolean");   break;
    case PARAM_TYPE_CHAR:      printf("char");      break;
    case PARAM_TYPE_OCTET:     printf("octet");     break;
    case PARAM_TYPE_SHORT:     printf("short");     break;
    case PARAM_TYPE_USHORT:    printf("ushort");    break;
    case PARAM_TYPE_LONG:      printf("long");      break;
    case PARAM_TYPE_ULONG:     printf("ulong");     break;
    case PARAM_TYPE_LONGLONG:  printf("longlong");  break;
    case PARAM_TYPE_ULONGLONG: printf("ulonglong"); break;
    case PARAM_TYPE_FLOAT:     printf("float");     break;
    case PARAM_TYPE_DOUBLE:    printf("double");    break;
    case PARAM_TYPE_STRING:    printf("string");    break;
    default: break;
    }
}

static void
printParamRelation (
    gapi_vector info)
{
    gapi_unsigned_long i,length;

    length = gapi_vectorGetLength(info);
    for ( i = 0; i < length; i++ ) {
        printf("<");
        printParameterInfo(&(RELATION_INFO_AT(info,i)->p1));
        printf(",");
        printParameterInfo(&(RELATION_INFO_AT(info,i)->p2));
        printf(">\n");
    }
}




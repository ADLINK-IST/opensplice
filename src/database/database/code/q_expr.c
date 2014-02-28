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
#include "c_iterator.h"
#include "q_expr.h"
#include "q_helper.h"
#include "q__parser.h"
#include "c_field.h"
#include "c__field.h"
#include "c_collection.h"
#include "os_report.h"
#include "c_stringSupport.h"
C_CLASS(q_func);

C_STRUCT(q_func) {
    q_tag tag;
    q_list params;
};

C_STRUCT(q_expr) {
    c_char* text;
    c_ulong instanceState;
    c_ulong sampleState;
    c_ulong viewState;

    enum q_kind kind;
    union {
        c_char *string;
        c_char character;
        c_longlong integer;
        c_double real;
        c_type type;
        q_func function;
    } info;
};

C_STRUCT(q_list) {
    q_expr expr;
    q_list next;
};

const c_char *
q_tagImage(
    q_tag tag)
{
#define _IMAGE_(t) #t
#define _CASE_(t) case Q_EXPR_##t: return _IMAGE_(Q_EXPR_##t)
    switch (tag) {
    _CASE_(DONTCARE); _CASE_(ERROR);
    _CASE_(PROGRAM); _CASE_(IMPORT); _CASE_(DEFINE); _CASE_(UNDEFINE);
    _CASE_(BIND); _CASE_(SELECT); _CASE_(SELECTDISTINCT); _CASE_(PARAMS);
    _CASE_(PROJECTION); _CASE_(FROM); _CASE_(WHERE); _CASE_(GROUP);
    _CASE_(HAVING); _CASE_(ORDER); _CASE_(DESC); _CASE_(CAST);
    _CASE_(OR); _CASE_(ORELSE); _CASE_(AND); _CASE_(FORALL);
    _CASE_(EXISTS); _CASE_(ANDTHEN); _CASE_(LIKE); _CASE_(EQ);
    _CASE_(NE); _CASE_(LT); _CASE_(LE); _CASE_(GT);
    _CASE_(GE); _CASE_(SOME); _CASE_(ANY); _CASE_(ALL);
    _CASE_(PLUS); _CASE_(SUB); _CASE_(UNION); _CASE_(EXCEPT);
    _CASE_(CONCAT); _CASE_(MUL); _CASE_(DIV); _CASE_(MOD);
    _CASE_(INTERSECT); _CASE_(IN); _CASE_(ABS); _CASE_(NOT);
    _CASE_(INDEX); _CASE_(PROPERTY); _CASE_(LIST); _CASE_(RANGE_);
    _CASE_(FUNCTION); _CASE_(LISTTOSET); _CASE_(ELEMENT); _CASE_(DISTINCT);
    _CASE_(FLATTEN); _CASE_(FIRST); _CASE_(LAST); _CASE_(UNIQUE);
    _CASE_(SUM); _CASE_(MIN); _CASE_(MAX); _CASE_(AVG);
    _CASE_(COUNT); _CASE_(ISUNDEF); _CASE_(ISDEF); _CASE_(CLASS);
    _CASE_(STRUCT); _CASE_(ARRAY); _CASE_(SET); _CASE_(BAG);
    _CASE_(DATE); _CASE_(TIME); _CASE_(TIMESTAMP); _CASE_(VARIABLE);
    _CASE_(SCOPEDNAME); _CASE_(JOIN); _CASE_(KEY);
    _CASE_(CALLBACK);
    }
#undef _CASE_
#undef _IMAGE_
    return NULL;
}

void
q_print(
    q_expr e,
    c_long i)
{
    char llstr[36];
    q_list l;
    c_long n,p;
    c_string name;
    c_string metaName;

    if (e == NULL) return;
    switch(e->kind) {
    case T_VAR:
    case T_INT:
	llstr[35] = '\0';
        printf("%s",os_lltostr(e->info.integer, &llstr[35]));
    break;
    case T_DBL:
        printf("%f",e->info.real);
    break;
    case T_CHR:
        printf("\'%c\'",e->info.character);
    break;
    case T_STR:
        printf("%s",e->info.string);
    break;
    case T_ID:
        printf("%s",e->info.string);
    break;
    case T_FNC:
        if (e->info.function->tag == Q_EXPR_CALLBACK) {
            name = (c_char *)q_tagImage(e->info.function->tag);
            metaName = c_metaName(c_metaObject(q_getTyp(q_getPar(e,0))));
            if (metaName != NULL) {
                printf("%s(<%s>,0x" PA_ADDRFMT,name,metaName,(c_address)q_getPar(e,1));
                c_free(metaName);
            } else {
                printf("%s(<anonomous type>,0x" PA_ADDRFMT,name,(c_address)q_getPar(e,1));
            }
            p = i+strlen(name)+1;
            printf(",\n");
            for (n=0;n<p;n++) printf(" ");
            q_print(q_getPar(e,2),p);
            printf(")");
        } else {
            name = (c_char *)q_tagImage(e->info.function->tag);
            printf("%s(",name);
            p = i+strlen(name)+1;
            l = e->info.function->params;
            if (l != NULL) {
                q_print(l->expr,p);
                l = l->next;
            }
            while (l != NULL) {
                printf(",\n");
                for (n=0;n<p;n++) printf(" ");
                q_print(l->expr,p);
                l = l->next;
            }
            printf(")");
        }
    break;
    case T_TYP:
        name = c_metaName(c_metaObject(e->info.type));
        if (name == NULL) {
            printf("<unnamed type>");
        } else {
            printf("%s",name);
        }
        c_free(name);
    break;
    case T_ERR:
    break;
    }
}

static q_list
listMalloc()
{
    return (q_list)os_malloc(C_SIZEOF(q_list));
}

static q_expr q_exprMalloc()
{
    q_expr expr;

    expr = (q_expr)os_malloc(C_SIZEOF(q_expr));
    if(expr){
        expr->text = NULL;
        expr->instanceState = 0;
        expr->sampleState = 0;
        expr->viewState = 0;
    }
    return expr;
}

static c_char *
q_stringMalloc(
    const c_char *string)
{
    return string ? os_strdup(string) : NULL;
}

static void
q_listDispose(
    q_list list)
{
    q_list ptr;
    while((ptr = list) != NULL) {
        list = ptr->next;
        q_dispose(ptr->expr);
        os_free(ptr);
    }
}

static q_func
funcMalloc(
    q_tag tag,
    q_list params)
{
    q_func func = (q_func)os_malloc(C_SIZEOF(q_func));
    if(func){
        func->tag = tag;
        func->params = params;
    }
    return func;
}

q_expr
q_newInt(
    c_longlong value)
{
    q_expr expr = q_exprMalloc();
    if(expr){
        expr->kind = T_INT;
        expr->info.integer = value;
    }
    return expr;
}

q_expr
q_newDbl(
    c_double value)
{
    q_expr expr = q_exprMalloc();
    if(expr){
        expr->kind = T_DBL;
        expr->info.real = value;
    }
    return expr;
}

q_expr
q_newChr(
    c_char value)
{
    q_expr expr = q_exprMalloc();
    if(expr){
        expr->kind = T_CHR;
        expr->info.character = value;
    }
    return expr;
}

q_expr
q_newStr(
    c_char *string)
{
    q_expr expr = q_exprMalloc();
    if(expr){
        expr->kind = T_STR;
        expr->info.string = q_stringMalloc(string);
    }
    return expr;
}

q_expr
q_newId(
    c_char *string)
{
    q_expr expr = q_exprMalloc();
    if(expr){
        expr->kind = T_ID;
        expr->info.string = q_stringMalloc(string);
    }
    return expr;
}

q_expr
q_newVar(c_longlong id)
{
    q_expr expr = q_exprMalloc();
    if(expr){
        expr->kind = T_VAR;
        expr->info.integer = id;
    }
    return expr;
}

q_expr
q_newTyp(
    c_type type)
{
    q_expr expr = q_exprMalloc();
    if(expr){
        expr->kind = T_TYP;
        expr->info.type = c_keep(type);
    }
    return expr;
}

q_expr
q_newFnc(
    q_tag tag,
     q_list params)
{
    q_expr expr;

/* ==Do=some=optimizing=============================== */

/* ================================================== */
    expr = q_exprMalloc();
    if(expr){
        expr->kind = T_FNC;
        expr->info.function = funcMalloc(tag, params);
    }
    return expr;
}

c_bool
q_isInt(
    q_expr expr)
{
    if (expr == NULL) {
        return FALSE;
    }
    return (expr->kind == T_INT);
}

c_bool
q_isDbl(
    q_expr expr)
{
    if (expr == NULL) {
        return FALSE;
    }
    return (expr->kind == T_DBL);
}

c_bool
q_isChr(
    q_expr expr)
{
    if (expr == NULL) {
        return FALSE;
    }
    return (expr->kind == T_CHR);
}

c_bool
q_isStr(
    q_expr expr)
{
    if (expr == NULL) {
        return FALSE;
    }
    return (expr->kind == T_STR);
}

c_bool
q_isId(
    q_expr expr)
{
    if (expr == NULL) {
        return FALSE;
    }
    return (expr->kind == T_ID);
}

c_bool
q_isVar(
    q_expr expr)
{
    if (expr == NULL) {
        return FALSE;
    }
    return (expr->kind == T_VAR);
}

c_bool
q_isTyp(
    q_expr expr)
{
    if (expr == NULL) {
        return FALSE;
    }
    return (expr->kind == T_TYP);
}

c_bool
q_isFnc(
    q_expr expr,
    q_tag tag)
{
    if (expr == NULL) {
        return FALSE;
    }
    if (expr->kind != T_FNC) {
        return FALSE;
    }
    if (tag == Q_EXPR_DONTCARE) {
        return TRUE;
    }
    return (expr->info.function->tag == tag);
}

c_longlong
q_getInt(
    q_expr expr)
{
    assert(expr->kind == T_INT);
    return expr->info.integer;
}

c_double
q_getDbl(
    q_expr expr)
{
    assert(expr->kind == T_DBL);
    return expr->info.real;
}

c_char
q_getChr(
    q_expr expr)
{
    assert(expr->kind == T_CHR);
    return expr->info.character;
}

c_char *
q_getStr(
    q_expr expr)
{
    assert(expr->kind == T_STR);
    return expr->info.string;
}

c_char *
q_getId(
    q_expr expr)
{
    assert(expr->kind == T_ID);
    return expr->info.string;
}

c_longlong
q_getVar(
    q_expr expr)
{
    assert(expr->kind == T_VAR);
    return expr->info.integer;
}

c_type
q_getTyp(
    q_expr expr)
{
    assert(expr->kind == T_TYP);
    return expr->info.type;
}

q_expr
q_getPar(
    q_expr expr,
    c_long index)
{
    q_list ptr;
    c_long i;

    assert(expr->kind == T_FNC);
    ptr = expr->info.function->params;
    i=0;
    for (i=0; (ptr!=NULL)&&(i!=index); i++) {
        ptr=ptr->next;
    }
    if (ptr == NULL) {
        return NULL;
    }
    return ptr->expr;
}

q_expr
q_takePar(
    q_expr expr,
    c_long index)
{
    q_list *ptr,found;
    q_expr e;
    c_long i;

    assert(expr->kind == T_FNC);

    ptr = &expr->info.function->params;
    i=0;
    for (i=0; (*ptr!=NULL)&&(i!=index); i++) {
        ptr=&(*ptr)->next;
    }
    if (*ptr == NULL) {
        e = NULL;
    } else {
        found = *ptr;
        *ptr = (*ptr)->next;
        e = found->expr;
        os_free(found);
    }
    return e;
}

q_list
q_listCopy(
    q_list l)
{
    q_list n = NULL;
    while (l != NULL) {
        n = q_append(n,q_exprCopy(l->expr));
        l = l->next;
    }
    return n;
}

q_expr
q_exprCopy(
    q_expr e)
{
    q_list n = NULL;
    q_expr copy;

    if (e == NULL) {
        return NULL;
    }
    switch (q_getKind(e)) {
    case T_FNC:
        if (e->info.function->tag == Q_EXPR_CALLBACK) {

            /* The first parameter specifies the result type of the callback function. */
            /* The second parameter is not of type q_expr but is a function pointer. */
            /* The function pointer is the address of the callback function.         */

            /* increment ref count of internal c_type because it is being copied */
            c_keep (q_getTyp(q_getPar(e,0)));
            n = q_append(n,q_getPar(e,0));
            n = q_append(n,q_getPar(e,1));
            n = q_append(n,q_exprCopy(q_getPar(e,2)));
            copy = q_newFnc(q_getTag(e),n);
            q_exprSetText(copy, e->text);
            q_exprSetInstanceState(copy, e->instanceState);
            q_exprSetSampleState(copy, e->sampleState);
            q_exprSetViewState(copy, e->viewState);
            return copy;
        } else {
            copy = q_newFnc(q_getTag(e),q_listCopy(q_getLst(e,0)));
            q_exprSetText(copy, e->text);
            q_exprSetInstanceState(copy, e->instanceState);
            q_exprSetSampleState(copy, e->sampleState);
            q_exprSetViewState(copy, e->viewState);
            return copy;
        }
    case T_TYP:
        copy = q_newTyp(q_getTyp(e));
        q_exprSetText(copy, e->text);
        q_exprSetInstanceState(copy, e->instanceState);
        q_exprSetSampleState(copy, e->sampleState);
        q_exprSetViewState(copy, e->viewState);
        return copy;
    case T_VAR:
        copy = q_newVar(q_getVar(e));
        q_exprSetText(copy, e->text);
        q_exprSetInstanceState(copy, e->instanceState);
        q_exprSetSampleState(copy, e->sampleState);
        q_exprSetViewState(copy, e->viewState);
        return copy;
    case T_INT:
        copy = q_newInt(q_getInt(e));
        q_exprSetText(copy, e->text);
        q_exprSetInstanceState(copy, e->instanceState);
        q_exprSetSampleState(copy, e->sampleState);
        q_exprSetViewState(copy, e->viewState);
        return copy;
    case T_DBL:
        copy = q_newDbl(q_getDbl(e));
        q_exprSetText(copy, e->text);
        q_exprSetInstanceState(copy, e->instanceState);
        q_exprSetSampleState(copy, e->sampleState);
        q_exprSetViewState(copy, e->viewState);
        return copy;
    case T_CHR:
        copy = q_newChr(q_getChr(e));
        q_exprSetText(copy, e->text);
        q_exprSetInstanceState(copy, e->instanceState);
        q_exprSetSampleState(copy, e->sampleState);
        q_exprSetViewState(copy, e->viewState);
        return copy;
    case T_STR:
        copy = q_newStr(q_getStr(e));
        q_exprSetText(copy, e->text);
        q_exprSetInstanceState(copy, e->instanceState);
        q_exprSetSampleState(copy, e->sampleState);
        q_exprSetViewState(copy, e->viewState);
        return copy;
    case T_ID:
        copy = q_newId(q_getId(e));
        q_exprSetText(copy, e->text);
        q_exprSetInstanceState(copy, e->instanceState);
        q_exprSetSampleState(copy, e->sampleState);
        q_exprSetViewState(copy, e->viewState);
        return copy;
    default:
        assert(FALSE);
    break;
    }
    return NULL;
}

q_expr
q_takeField(
    q_expr *e,
    c_string name)
{
    q_expr *l,*r,p,q,x;
    c_long i;
    c_char qn[1024];
    q_tag tag;

    if (e == NULL) return NULL;
    if (*e == NULL) return NULL;
    switch (q_getKind(*e)) {
    case T_FNC:
        tag = q_getTag(*e);
        switch (tag) {
        case Q_EXPR_PROGRAM:
            return q_takeField(&(*e)->info.function->params->expr,name);
        case Q_EXPR_OR:
            /* This function should never be used upon an OR expression.
               However if that functionality is required then this function should be redesigned.
            */
            assert(FALSE);
        break;
        case Q_EXPR_AND:
            l = &(*e)->info.function->params->expr;
            r = &(*e)->info.function->params->next->expr;
            p = q_takeField(l,name);
            q = q_takeField(r,name);
            if ((*l == NULL) && (*r == NULL)) {
                q_dispose(*e); *e = NULL;
            } else if ((*l == NULL) && (*r != NULL)) {
                x = *r; *r = NULL; q_dispose(*e); *e = x;
            } else if ((*l != NULL) && (*r == NULL)) {
                x = *l; *l = NULL; q_dispose(*e); *e = x;
            }
            if (p == NULL) {
                if (q == NULL) return NULL;
                return q;
            } else {
                if (q == NULL) {
                    return p;
                }
                return q_newFnc(tag,q_insert(q_insert(NULL,p),q));
            }
        case Q_EXPR_EQ:
        case Q_EXPR_NE:
        case Q_EXPR_LT:
        case Q_EXPR_LE:
        case Q_EXPR_GT:
        case Q_EXPR_GE:
        case Q_EXPR_LIKE:
            l = &(*e)->info.function->params->expr;
            r = &(*e)->info.function->params->next->expr;
            p = *e;
            if (q_takeField(l,name) != NULL) { *e = NULL; return p; }
            if (q_takeField(r,name) != NULL) { *e = NULL; return p; }
            return NULL;
        case Q_EXPR_NOT:
            p = *e;
            if (q_takeField(&(*e)->info.function->params->expr,name) != NULL) {
                *e = NULL;
                return p;
            }
        break;
        case Q_EXPR_PROPERTY:
            i=0; qn[0]=0;
            while ((p = q_getPar(*e,i)) != NULL) {
                if (i!=0) os_strcat(qn,".");
                os_strcat(qn,q_getId(p));
                i++;
            }
            if (strcmp(qn,name) == 0) return *e;
            return NULL;
        case Q_EXPR_CALLBACK:
            return q_exprCopy(*e);
        default:
            assert(FALSE);
        break;
        }
    break;
    case T_TYP:
    case T_VAR:
    case T_INT:
    case T_DBL:
    case T_CHR:
    case T_STR:
    case T_ERR:
    break;
    case T_ID:
        if (strcmp(q_getId(*e),name) == 0) return *e;
        return NULL;
    default:
        assert(FALSE);
    break;
    }
    return NULL;
}

q_expr
q_takeKey(
    q_expr *e,
    c_array keyList)
{
    q_expr *l,*r,p,q,x;
    c_long i,len;
    c_char qn[1024];
    q_tag tag;

    if (e == NULL) return NULL;
    if (*e == NULL) return NULL;
    len = c_arraySize(keyList);
    if (len == 0) {
        return NULL;
    }
    switch (q_getKind(*e)) {
    case T_FNC:
        tag = q_getTag(*e);
        switch (tag) {
        case Q_EXPR_AND:
            l = &(*e)->info.function->params->expr;
            r = &(*e)->info.function->params->next->expr;
            p = q_takeKey(l,keyList);
            q = q_takeKey(r,keyList);
            if ((*l == NULL) && (*r == NULL)) {
                q_dispose(*e); *e = NULL;
            } else if ((*l == NULL) && (*r != NULL)) {
                x = *r; *r = NULL; q_dispose(*e); *e = x;
            } else if ((*l != NULL) && (*r == NULL)) {
                x = *l; *l = NULL; q_dispose(*e); *e = x;
            }
            if (p == NULL) {
                if (q == NULL) return NULL;
                return q;
            } else {
                if (q == NULL) {
                    return p;
                }
                return q_newFnc(tag,q_insert(q_insert(NULL,p),q));
            }
        case Q_EXPR_EQ:
        case Q_EXPR_NE:
        case Q_EXPR_LT:
        case Q_EXPR_LE:
        case Q_EXPR_GT:
        case Q_EXPR_GE:
        case Q_EXPR_LIKE:
            l = &(*e)->info.function->params->expr;
            r = &(*e)->info.function->params->next->expr;

            if (q_takeKey(l,keyList) != NULL) {
                if (q_takeKey(r,keyList) != NULL) {
                    p = *e; *e = NULL;
                    return p;
                }
            }
            return NULL;
        case Q_EXPR_PROPERTY:
            i=0; qn[0]=0;
            while ((p = q_getPar(*e,i)) != NULL) {
                if (i!=0) os_strcat(qn,".");
                os_strcat(qn,q_getId(p));
                i++;
            }
            for (i=0; i<len; i++) {
                if (strcmp(qn,c_fieldName(keyList[i])) == 0) {
                    return *e;
                }
            }
            return NULL;
        case Q_EXPR_CALLBACK:
            return NULL;
        default:
            assert(FALSE);
        break;
        }
    break;
    case T_VAR:
    case T_INT:
    case T_DBL:
    case T_CHR:
    case T_STR:
        return *e;
    case T_TYP:
    case T_ERR:
    break;
    case T_ID:
        for (i=0; i<len; i++) {
            if (strcmp(qn,c_fieldName(keyList[i])) == 0) {
                return *e;
            }
        }
        return NULL;
    default:
        assert(FALSE);
    break;
    }
    return NULL;
}

/**
 * The following method like many other implement a generic expr walk with specific functionality.
 */
void
q_prefixFieldNames (
    q_expr *e,
    c_char *prefix)
{
    q_tag tag;
    q_expr p;
    q_list list;

    if (e == NULL) return;
    if (*e == NULL) return;
    if (prefix == NULL) return;
    if (strlen(prefix) == 0) return;
    switch (q_getKind(*e)) {
    case T_FNC:
        tag = q_getTag(*e);
        switch (tag) {
        case Q_EXPR_AND:
        case Q_EXPR_OR:
        case Q_EXPR_EQ:
        case Q_EXPR_NE:
        case Q_EXPR_LT:
        case Q_EXPR_LE:
        case Q_EXPR_GT:
        case Q_EXPR_GE:
        case Q_EXPR_LIKE:
            q_prefixFieldNames(&(*e)->info.function->params->expr,prefix);
            q_prefixFieldNames(&(*e)->info.function->params->next->expr,prefix);
            return;
        case Q_EXPR_NOT:
            q_prefixFieldNames(&(*e)->info.function->params->expr,prefix);
        break;
        case Q_EXPR_PROPERTY:
            list = q_listCopy(q_getLst(*e,0));
            list = q_insert(list,q_newId(prefix));
            p = q_newFnc(Q_EXPR_PROPERTY,list);
            q_swapExpr(*e,p);
            q_dispose(p);
            return;
        case Q_EXPR_CALLBACK:
            return;
        case Q_EXPR_PROGRAM:
            q_prefixFieldNames(&(*e)->info.function->params->expr,prefix);
        break;
        default:
            assert(FALSE);
        break;
        }
    break;
    case T_VAR:
    case T_INT:
    case T_DBL:
    case T_CHR:
    case T_STR:
    case T_TYP:
    case T_ERR:
    break;
    case T_ID:
        list = q_insert(NULL,q_newId(q_getId(*e)));
        list = q_insert(list,q_newId(prefix));
        p = q_newFnc(Q_EXPR_PROPERTY,list);
        q_swapExpr(*e,p);
        q_dispose(p);
        return;
    default:
        assert(FALSE);
    break;
    }
}

q_expr
q_takeTerm(
    q_expr *e)
{
    q_expr or,left;

    or = *e;
    if (or == NULL) {
        return NULL;
    }
    if ((q_getKind(or) != T_FNC) || (q_getTag(or) != Q_EXPR_OR)) {
        *e = NULL;
         return or;
    }
    left = q_takePar(or,0);
    *e = q_takePar(or,0);

    q_listDispose(or->info.function->params);
    os_free(or->info.function);
    os_free(or);
    return left;
}

void
q_insertPar(
    q_expr expr,
    q_expr par)
{
    assert(expr->kind == T_FNC);
    expr->info.function->params = q_insert(expr->info.function->params,par);
}

void
q_addPar(
    q_expr expr,
    q_expr par)
{
    assert(expr->kind == T_FNC);

    expr->info.function->params = q_append(expr->info.function->params, par);
}

void
q_swapExpr(
    q_expr oldExpr,
    q_expr newExpr)
{
    C_STRUCT(q_expr) e;

    memcpy(&e,oldExpr,C_SIZEOF(q_expr));
    memcpy(oldExpr,newExpr,C_SIZEOF(q_expr));
    memcpy(newExpr,&e,C_SIZEOF(q_expr));
}

q_expr
q_swapPar(
    q_expr expr,
    c_long index,
    q_expr par)
{
    q_list ptr;
    c_long i;
    q_expr old;

    assert(expr->kind == T_FNC);
    ptr = expr->info.function->params;
    for (i=0; (ptr!=NULL)&&(i!=index); i++) ptr=ptr->next;
    if (ptr == NULL) return par;
    old = ptr->expr;
    ptr->expr = par;
    return old;
}

c_long
q_getLen(
    q_expr expr)
{
    q_list ptr;
    c_long i;
    assert(expr->kind == T_FNC);
    ptr = expr->info.function->params;
    for (i=0; ptr!=NULL; i++) ptr=ptr->next;
    return i;
}

q_list
q_getLst(
    q_expr expr,
    c_long index)
{
    q_list ptr;
    c_long i;
    assert(expr->kind == T_FNC);
    ptr = expr->info.function->params;
    for (i=0; (ptr!=NULL)&&(i!=index); i++) ptr=ptr->next;
    return ptr;
}

q_kind
q_getKind(
    q_expr expr)
{
    return expr->kind;
}

q_tag
q_getTag(
    q_expr expr)
{
    if (expr->kind != T_FNC) return Q_EXPR_ERROR;
    return expr->info.function->tag;
}

q_tag
q_setTag(
    q_expr expr,
    q_tag tag)
{
    q_tag old;
    assert(expr->kind == T_FNC);
    old = expr->info.function->tag;
    expr->info.function->tag = tag;
    return old;
}

void
q_dispose(
    q_expr expr)
{
    c_type t;

    if (expr == NULL) return;
    switch (expr->kind) {
    case T_ID:
    case T_STR:
        os_free(expr->info.string);
    break;
    case T_TYP:
    break;
    case T_FNC:
        if (expr->info.function->tag == Q_EXPR_CALLBACK) {

            /* The first parameter is not of type q_expr but is of type c_type.      */
            /* It specifies the result type of the callback function.                */
            t = c_type(q_getTyp(q_swapPar(expr,0,NULL)));
            c_free(t);
            q_swapPar(expr,1,NULL);
            q_dispose((q_expr)q_swapPar(expr,2,NULL));
        }
        q_listDispose(expr->info.function->params);
        os_free(expr->info.function);
    break;
    default:
    break;
    }
    if(expr->text) os_free(expr->text);
    os_free(expr);
}

q_list
q_append(
    q_list list,
    q_expr expr)
{
    q_list *ptr;
    assert(expr != NULL);
    if (list == NULL) {
        list = listMalloc();
        list->next = NULL;
        list->expr = expr;
    } else {
        for (ptr = &list->next; *ptr != NULL; ptr = &(*ptr)->next);
        *ptr = listMalloc();
        (*ptr)->next = NULL;
        (*ptr)->expr = expr;
    }
    return list;
}

q_list
q_insert(
    q_list list,
    q_expr expr)
{
    q_list head;
    assert(expr != NULL);
    head = listMalloc();
    head->next = list;
    head->expr = expr;
    return head;
}

q_list
q_next(
    q_list list)
{
    if (list == NULL) return NULL;
    return list->next;
}

q_expr
q_element(
    q_list list)
{
    assert(list != NULL);
    return list->expr;
}

c_bool
q_isComparison(
    q_expr expr)
{
    assert(expr != NULL);
    assert(expr->kind == T_FNC);
    if (expr->kind != T_FNC) return FALSE;
    switch(expr->info.function->tag) {
    case Q_EXPR_EQ:
    case Q_EXPR_NE:
    case Q_EXPR_GT:
    case Q_EXPR_LT:
    case Q_EXPR_GE:
    case Q_EXPR_LE:
        return TRUE;
    default:
        return FALSE;
    }
}

c_bool
q_isAggregate(
    q_expr expr)
{
    assert(expr != NULL);
    assert(expr->kind == T_FNC);
    if (expr->kind != T_FNC) return FALSE;
    switch(expr->info.function->tag) {
    case Q_EXPR_COUNT:
    case Q_EXPR_MIN:
    case Q_EXPR_MAX:
    case Q_EXPR_AVG:
    case Q_EXPR_SUM:
        return TRUE;
    default:
        return FALSE;
    }
}

void
q_promote(
    q_expr e)
{
    q_expr leftTerm, rightTerm;
    q_expr newTerm;

    assert(e != NULL);
    assert(e->kind == T_FNC);
    assert(e->info.function->tag == Q_EXPR_AND);

    leftTerm = q_leftPar(e);
    rightTerm = q_rightPar(e);
    q_disjunctify(leftTerm);
    q_disjunctify(rightTerm);
    if (q_isFnc(leftTerm,Q_EXPR_OR)) {
        /* e = (A or B) and C ====> e = A and C or B and C */
        newTerm = F2(Q_EXPR_AND,q_rightPar(leftTerm),q_exprCopy(rightTerm));
        /* e = (A or B) and C; newTerm = B and C */
q_promote(newTerm);
        q_swapRight(leftTerm,q_swapRight(e,newTerm));
        /* e = (A or C) and (B and C); */
        q_setTag(leftTerm,Q_EXPR_AND);
        /* e = (A and C) and (B and C); */
q_promote(leftTerm);
        q_setTag(e,Q_EXPR_OR);
        /* e = (A and C) or (B and C); */
        if (q_isFnc(rightTerm,Q_EXPR_OR)) {
            /* e = (A and (C or D)) or (B and (C or D)); */
            q_promote(leftTerm);
            /* e = (A and C) or (A and D)) or (B and (C or D)); */
            q_promote(newTerm);
            /* e = ((A and C) or (A and D)) or ((B and C) or (A and D)); */
        }
    } else {
    	if (q_isFnc(rightTerm,Q_EXPR_OR)) {
            /* e = A and (B or C) ====> e = A and B or A and C */
            newTerm = F2(Q_EXPR_AND,q_exprCopy(leftTerm),q_leftPar(rightTerm));
            /* e = A and (B or C); newTerm = A and B */
            q_swapLeft(rightTerm,q_swapLeft(e,newTerm));
            /* e = (A and B) and (A or C); */
            q_setTag(rightTerm,Q_EXPR_AND);
            /* e = (A and B) and (A and C); */
            q_setTag(e,Q_EXPR_OR);
            /* e = (A and B) or (A and C); */
    	}
    }
}

void
q_disjunctify(
    q_expr e)
{
    q_expr leftTerm, rightTerm;
    c_long len,i;
    q_expr newTerm, notTerm;

    if (e == NULL) return;
    if (e->kind == T_FNC) {
        switch (e->info.function->tag) {
        case Q_EXPR_AND:
#if 1
            q_promote(e);
#else
            leftTerm = q_leftPar(e);
            rightTerm = q_rightPar(e);
            q_disjunctify(leftTerm);
            q_disjunctify(rightTerm);
            if (q_isFnc(leftTerm,Q_EXPR_OR)) {
                newTerm = F2(Q_EXPR_AND,q_rightPar(leftTerm),q_exprCopy(rightTerm));
                q_swapRight(leftTerm,q_swapRight(e,newTerm));
                q_setTag(leftTerm,Q_EXPR_AND);
                q_setTag(e,Q_EXPR_OR);
                q_disjunctify(e);
            }
            if (q_isFnc(rightTerm,Q_EXPR_OR)) {
                newTerm = F2(Q_EXPR_AND,q_rightPar(rightTerm),q_exprCopy(leftTerm));
                q_swapRight(rightTerm,q_swapLeft(e,newTerm));
                q_setTag(rightTerm,Q_EXPR_AND);
                q_setTag(e,Q_EXPR_OR);
                q_disjunctify(e);
            }
#endif
        break;
        case Q_EXPR_OR:
            leftTerm = q_leftPar(e);
            rightTerm = q_rightPar(e);
            q_disjunctify(leftTerm);
            q_disjunctify(rightTerm);
        break;
        case Q_EXPR_NOT:
            notTerm = q_getPar(e,0);
            if (notTerm->kind == T_FNC) {
                switch (notTerm->info.function->tag) {
                case Q_EXPR_NOT:
                    q_swapExpr(e,q_takePar(notTerm,0));
                    q_dispose(notTerm);
                    q_disjunctify(e);
                break;
                case Q_EXPR_OR:
                case Q_EXPR_AND:
                    /* e = not (A and/or B) */
                    notTerm = q_takePar(e,0);
                    /* e = not; notTerm = (A and/or B); */
                    newTerm = F1(Q_EXPR_NOT,q_exprCopy(q_getPar(notTerm,0)));
                    /* newTerm = not A */
#if 1
                    q_disjunctify(newTerm);
#endif
                    q_swapPar(notTerm,0,newTerm);
                    /* notTerm = (not A) and/or B */
                    newTerm = F1(Q_EXPR_NOT,q_exprCopy(q_getPar(notTerm,1)));
                    /* newTerm = not B */
#if 1
                    q_disjunctify(newTerm);
#endif
                    q_swapPar(notTerm,1,newTerm);
                    /* notTerm = (not A) and/or (not B) */
                    if (notTerm->info.function->tag == Q_EXPR_OR) {
                        notTerm->info.function->tag = Q_EXPR_AND;
                    } else {
                        notTerm->info.function->tag = Q_EXPR_OR;
                    }
                    /* notTerm = (not A) or/and (not B) */
                    q_swapExpr(e,notTerm);
                    /* e = (not A) or/and (not B) */
                    q_dispose(notTerm);
#if 0
                    q_disjunctify(e);
#endif
                break;
#define _CASE_(l,n) case l: notTerm = q_takePar(e,0); \
                            q_swapExpr(e,notTerm); \
                            e->info.function->tag = n; \
                            q_dispose(notTerm); \
                            q_disjunctify(e); \
                    break
                _CASE_(Q_EXPR_EQ,Q_EXPR_NE);
                _CASE_(Q_EXPR_NE,Q_EXPR_EQ);
                _CASE_(Q_EXPR_LT,Q_EXPR_GE);
                _CASE_(Q_EXPR_LE,Q_EXPR_GT);
                _CASE_(Q_EXPR_GT,Q_EXPR_LE);
                _CASE_(Q_EXPR_GE,Q_EXPR_LT);
#undef _CASE_
                default:
                    /* let it be */
                break;
                }
            }
        break;
        case Q_EXPR_CALLBACK:
            q_disjunctify(q_getPar(e,2));
        break;
        default:
            /* let it be */
            len = q_getLen(e);
            for (i=0;i<len;i++) {
                q_disjunctify(q_getPar(e,i));
            }
        break;
        }
    }
}

static q_expr
q_deNot(
    q_expr e)
{
    q_expr r;

    if (e == NULL) {
        return NULL;
    }
    if (e->kind == T_FNC) {
        switch (e->info.function->tag) {
        case Q_EXPR_LIKE:                                    r = e; break;
        case Q_EXPR_EQ:   e->info.function->tag = Q_EXPR_NE; r = e; break;
        case Q_EXPR_NE:   e->info.function->tag = Q_EXPR_EQ; r = e; break;
        case Q_EXPR_LT:   e->info.function->tag = Q_EXPR_GE; r = e; break;
        case Q_EXPR_LE:   e->info.function->tag = Q_EXPR_GT; r = e; break;
        case Q_EXPR_GT:   e->info.function->tag = Q_EXPR_LE; r = e; break;
        case Q_EXPR_GE:   e->info.function->tag = Q_EXPR_LT; r = e; break;
        case Q_EXPR_NOT:
            r = q_takePar(e,0);
            q_dispose(e);
        break;
        case Q_EXPR_AND:
            r = F2(Q_EXPR_OR,q_removeNots(q_takePar(e,0)),q_removeNots(q_takePar(e,1)));
            q_dispose(e);
        break;
        case Q_EXPR_OR:
            r = F2(Q_EXPR_AND,q_removeNots(q_takePar(e,0)),q_removeNots(q_takePar(e,1)));
            q_dispose(e);
        break;
        default:
            assert(FALSE);
            r = e;
        break;
        }
    } else {
        r = e;
    }
    return r;
}

q_expr
q_removeNots(
    q_expr e)
{
    q_expr r,p;
    c_long len,i;

    if (e == NULL) {
        return NULL;
    }
    if (e->kind == T_FNC) {
        switch (e->info.function->tag) {
        case Q_EXPR_NOT:
            r = q_removeNots(q_deNot(q_takePar(e,0)));
        break;
        default:
            len = q_getLen(e);
            for (i=0;i<len;i++) {
                p = q_removeNots(q_getPar(e,i));
                q_swapPar(e,i,p);
            }
            r = e;
        }
    } else {
        r = e;
    }
    return r;
}

c_char *
q_propertyName(
    q_expr e)
{
    q_expr p;
    c_char *name;
    c_long len,i;

    if (q_isFnc(e,Q_EXPR_PROPERTY)) {
        i=0; len = 0;
        while ((p = q_getPar(e,i)) != NULL) {
            len = len + 1 + strlen(q_getId(p));
            i++;
        }
        name = (c_char *)os_malloc(len);
        i=0; name[0]=0;
        while ((p = q_getPar(e,i)) != NULL) {
            if (i!=0) os_strcat(name,".");
            os_strcat(name,q_getId(p));
            i++;
        }
    } else {
        assert(FALSE);
        name = NULL;
    }
    return name;
}

static c_equality
compareVar(
    q_expr e1,
    q_expr e2)
{
    assert(q_isVar(e1));
    assert(q_isVar(e2));
    if (e1->info.integer == e2->info.integer) {
        return C_EQ;
    } else {
        return C_NE;
    }
}

static void
q_countVarWalk(
    q_expr e,
    c_iter list)
{
    q_list l;
    q_expr found;

    if (e != NULL) {
        switch(e->kind) {
        case T_VAR:
            found = c_iterResolve(list,(c_iterResolveCompare)compareVar,e);
            if (found == NULL) {
                list = c_iterInsert(list,e);
            }
        break;
        case T_FNC:
            if (e->info.function->tag == Q_EXPR_CALLBACK) {
                q_countVarWalk(q_getPar(e,2),list);
            } else {
                l = e->info.function->params;
                while (l != NULL) {
                    q_countVarWalk(l->expr,list);
                    l = l->next;
                }
            }
        break;
        default:
        break;
        }
    }
}

c_long
q_countVar(
    q_expr e)
{
    c_long nrOfVar;
    c_iter list;

    list = c_iterNew(NULL);
    q_countVarWalk(e,list);
    nrOfVar = c_iterLength(list);
    c_iterFree(list);
    return nrOfVar;
}

void
q_exprSetText(
    q_expr expr,
    const c_char* text)
{
    if(expr && text){
        expr->text = q_stringMalloc(text);
    } else if(expr){
        expr->text = NULL;
    }
}

c_char*
q_exprGetText(
    q_expr expr)
{
    if(expr){
        return q_stringMalloc(expr->text);
    }
    return NULL;
}

void
q_exprSetInstanceState(
    q_expr expr,
    c_ulong state)
{
    if(expr){
        expr->instanceState = state;
    }
}

void
q_exprSetSampleState(
    q_expr expr,
    c_ulong state)
{
    if(expr){
        expr->sampleState = state;
    }
}

void
q_exprSetViewState(
    q_expr expr,
    c_ulong state)
{
    if(expr){
        expr->viewState = state;
    }
}

c_ulong
q_exprGetInstanceState(
    q_expr expr)
{
    if(expr){
        return expr->instanceState;
    }
    return 0;
}

c_ulong
q_exprGetSampleState(
    q_expr expr)
{
    if(expr){
        return expr->sampleState;
    }
    return 0;
}

c_ulong
q_exprGetViewState(
    q_expr expr)
{
    if(expr){
        return expr->viewState;
    }
    return 0;
}

c_iter
deOr(
    q_expr e,
    c_iter list)
{
    c_iter results;

    if (q_getTag(e) == Q_EXPR_OR) {
        results = deOr(q_takePar(e,0),deOr(q_takePar(e,0),list));
        q_dispose(e);
    } else {
    		results = c_iterInsert(list,e);
    }
    return results;
}

void
translate(
    q_expr expr,
    c_array sourceKeyList, /* c_array<c_field> */
    c_array indexKeyList)  /* c_array<c_field> */
{
    assert(expr);
    assert(sourceKeyList);
    assert(indexKeyList);

    if(q_getKind(expr) == T_FNC){
        if(q_isFnc(expr, Q_EXPR_PROPERTY))
        {
            /* first get the string representation of the id's in this expr */
            c_field f;
            c_long i, index = -1, size = 0;
            c_char *name;

            name = q_propertyName(expr);
            if(name)
            {
                /* Now find the matching key in the sourceKeyList */
                size = c_arraySize(sourceKeyList);

                assert(size == c_arraySize(indexKeyList));

                if(size == c_arraySize(indexKeyList)){

                    for(i=0; i<size; i++)
                    {
                        f = (c_field)(sourceKeyList[i]);
                        if(strcmp(c_fieldName(f), name) == 0)
                        {
                            index = i;
                            break;
                        }
                    }

                    assert(index >= 0);

                    if(index >= 0)
                    {
                        /* now replace the Q_EXPR_PROPERTY id's by the indexKeyList ones */
                        q_expr e;
                        c_char *fieldNameStr;
                        c_char *str;
                        c_iter ids;

                        f = (c_field)(indexKeyList[index]);
                        fieldNameStr = c_fieldName(f);


                        /* clear current list */
                        e = q_takePar(expr, 0);
                        while(e){
                            q_dispose(e);
                            e = q_takePar(expr, 0);
                        }

                        ids = c_splitString(fieldNameStr, ".");
                        if(ids){
                            str = (c_char*)c_iterTakeFirst(ids);
                            while(str){
                                e = q_newId(str);
                                q_addPar(expr, e);
                                os_free(str);
                                str = (c_char*)c_iterTakeFirst(ids);
                            }
                            c_iterFree(ids);
                        }
                    } else {
                        OS_REPORT_1(OS_WARNING,"v_dataReaderQuery_translate failed", 0,
                                        "Cannot find key '%s' in key list.", name);
                    }
                } else {
                    OS_REPORT_2(OS_ERROR,"v_dataReaderQuery_translate failed", 0,
                                       "sizes of indexKeyList (size %d) and sourceKeyList (size %d) do not match.", c_arraySize(indexKeyList), size);
                }
                os_free(name);
            }
        } else if (!q_isFnc(expr, Q_EXPR_CALLBACK))
        {
            q_list l = q_getLst(expr, 0);
            while(l)
            {
                translate(q_element(l), sourceKeyList, indexKeyList);
                l = q_next(l);
            }
        }
    }
}

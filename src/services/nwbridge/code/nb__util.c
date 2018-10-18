/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#include "nb__util.h"
#include "nb__log.h"

#include "u_cfNode.h"
#include "v_public.h"

#include "c_stringSupport.h"

os_equality
nb_compareByName(
    void *o1,
    void *o2,
    void *arg)
{
    os_equality result;
    int equal;

    OS_UNUSED_ARG(arg);
    assert(o1);
    assert(o2);

    equal = strcmp((os_char*)o1, (os_char*)o2);

    if (equal > 0) {
        result = OS_GT;
    } else if (equal < 0) {
        result = OS_LT;
    } else {
        result = OS_EQ;
    }

    return result;
}

/* This compile-time constraint assures that the cast from c_equality to
 * os_equality is allowed. */
struct os_equality_equals_c_equality_constraint {
    char require_value_OS_LT_eq_value_C_LT [OS_LT == (os_equality)C_LT];
    char require_value_OS_EQ_eq_value_C_EQ [OS_EQ == (os_equality)C_EQ];
    char require_value_OS_GT_eq_value_C_GT [OS_GT == (os_equality)C_GT];
    char require_value_OS_NE_eq_value_C_NE [OS_NE == (os_equality)C_NE];
    char non_empty_dummy_last_member[1];
};

os_equality
nb_compareByGid(
    void *o1,
    void *o2,
    void *arg)
{
    OS_UNUSED_ARG(arg);
    assert(o1);
    assert(o2);

    /* v_gidCompare does consider the serial, which isn't the meaning of the
     * gid per se. */
    return (os_equality)v_gidCompare(*(v_gid *)o1, *(v_gid *)o2);;
}

c_bool
nb_match(
        const char * const * partitions,    /* partExpr */
        c_ulong partitionsLen,
        const char *topicName,              /* Absolute topicName */
        const char * const * includes,      /* NULL-terminated partExpr.topExpr */
        const char * const * excludes)      /* NULL-terminated partExpr.topExpr */
{
    c_ulong i, j;
    const char *topExpr;
    const char *partTopExpr; /* partExpr.topExpr */
    char *partExpr;

    assert(partitions);
    assert(topicName);
    assert(includes);
    assert(excludes);

    j = 0;
    while((partTopExpr = excludes[j++]) != NULL){
        topExpr = strrchr(partTopExpr, '.');
        assert(topExpr); /* partTopExpr MUST have a dot */

        if(!c_stringMatchesExpression(topicName, topExpr + 1)) continue;

        partExpr = os_strndup(partTopExpr, (os_size_t)(topExpr - partTopExpr));
        for(i = 0; i < partitionsLen; i++){
            if(c_stringMatchesExpression(partitions[i], partExpr)){
                NB_TRACE(("'%s.%s' excluded since it matches exclude expression '%s%s'\n",
                        partitions[i], topicName,
                        partExpr, topExpr
                ));
                os_free(partExpr);
                goto exclude_matched;
            }
        }
        os_free(partExpr);
    }

    j = 0;
    while((partTopExpr = includes[j++]) != NULL){
        topExpr = strrchr(partTopExpr, '.');
        assert(topExpr); /* partTopExpr MUST have a dot */

        if(!c_stringMatchesExpression(topicName, topExpr + 1)) continue;

        partExpr = os_strndup(partTopExpr, (os_size_t)(topExpr - partTopExpr));
        for(i = 0; i < partitionsLen; i++){
            if(c_stringMatchesExpression(partitions[i], partExpr)){
                NB_TRACE(("'%s.%s' included since it matches include expression '%s%s'\n",
                        partitions[i], topicName,
                        partExpr, topExpr
                ));
                os_free(partExpr);
                return TRUE;
            }
        }
        os_free(partExpr);
    }

    NB_TRACE(("Topic '%s' didn't match any include or exclude expression, so it is excluded\n", topicName));

exclude_matched:
    return FALSE;
}

void
nb_cfNodeIterFree(
    c_iter iter)
{
    u_cfNode node;
    assert(iter);
    while ((node = c_iterTakeFirst(iter))) {
        u_cfNodeFree(node);
    }
    c_iterFree(iter);
}

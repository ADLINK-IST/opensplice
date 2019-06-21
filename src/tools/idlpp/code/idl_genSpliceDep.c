/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
/*
   This module generates IDL file dependencies based upon
   the IDL input file base names.
*/

#include "idl_program.h"
#include "idl_scope.h"
#include "idl_dependencies.h"
#include "idl_genSpliceDep.h"
#include "idl_fileMap.h"
#include "os_heap.h"
#include "os_string.h"
#include "os_stdlib.h"

static char *
idl_stripIncludePath(
    idl_typeUser typeUser,
    void *userData)
{
    struct SpliceDepUserData *info = userData;
    char *inclName = NULL;

    if (info && info->keepIncludePaths) {
        c_type ctype = idl_typeSpecDef(idl_typeSpec(typeUser));
        const c_char *name = idl_fileMapResolveInclude (idl_fileMapDefGet(), c_baseObject(ctype));
        if (name && (*name != '\0')) {
            inclName = os_str_rtrim(name, ".idl");
        }
    }

    if (!inclName) {
        inclName = idl_scopeBasename(idl_typeUserScope(typeUser));
    }
    return inclName;
}

static char *
idl_getScopeFilename(
    idl_scope scope,
    void *userData)
{
    struct SpliceDepUserData *info = userData;
    char *fileName = NULL;

    if (info && info->keepIncludePaths) {
        char *sn = idl_scopeFilename(scope);
        fileName = os_str_rtrim(sn, ".idl");
        os_free(sn);
    }

    if (!fileName) {
        fileName = idl_scopeBasename(scope);
    }

    return fileName;
}


/* fileOpen callback

   return idl_explore to state that the rest of the file needs to be processed
*/
static idl_action
idl_fileOpen(
    idl_scope scope,
    const char *name,
    void *userData)
{
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(userData);

    return idl_explore;
}

/* moduleOpen callback

   return idl_explore to state that the rest of the module needs to be processed
*/
static idl_action
idl_moduleOpen(
    idl_scope scope,
    const char *name,
    void *userData)
{
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(userData);

    return idl_explore;
}

/* structureOpen callback

   return idl_explore to state that the rest of the structure needs to be processed
*/
static idl_action
idl_structureOpen(
    idl_scope scope,
    const char *name,
    idl_typeStruct structSpec,
    void *userData)
{
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(structSpec);
    OS_UNUSED_ARG(userData);

    return idl_explore;
}

/* structureMemberOpenClose callback

   Generate dependencies for the following IDL construct:
        struct <structure-name> {
   =>       <structure-member-1>;
   =>       ...              ...
   =>       <structure-member-n>;
        };
   For each structure member determine if the members (actual) type is
   defined within the same file as the structure or in another file.
   If it is defined in another file, add a dependency to the dependency
   list.
*/
static void
idl_structureMemberOpenClose(
    idl_scope scope,
    const char *name,
    idl_typeSpec typeSpec,
    void *userData)
{
    idl_typeSpec actualType;
    char *sn;
    char *fn;

    OS_UNUSED_ARG(name);

    sn = idl_getScopeFilename(scope, userData);
    /* QAC EXPECT 3416; No unexpected side effects here */
    if (idl_typeSpecType(typeSpec) == idl_tarray) {
        actualType = idl_typeArrayActual(idl_typeArray(typeSpec));
        /* QAC EXPECT 3416; No unexpected side effects here */
        if (idl_typeSpecType(actualType) != idl_tbasic) {
            fn = idl_stripIncludePath(idl_typeUser(actualType), userData);
            /* QAC EXPECT 3416, 5007; No unexpected side effects here, will not use wrapper */
            if (strlen(fn) && (strcmp(sn, fn) != 0)) {
                /* referenced type is in different scope */
                idl_depAdd(idl_depDefGet(), fn);
            }
            os_free(fn);
        }
        /* QAC EXPECT 3416; No unexpected side effects here */
    } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
        actualType = idl_typeSeqActual(idl_typeSeq(typeSpec));
        /* QAC EXPECT 3416; No unexpected side effects here */
        if (idl_typeSpecType(actualType) != idl_tbasic) {
            fn = idl_stripIncludePath(idl_typeUser(actualType), userData);
            /* QAC EXPECT 3416, 5007; No unexpected side effects here, will not use wrapper */
            if (strlen(fn) && (strcmp (sn, fn) != 0)) {
                /* referenced type is in different scope */
                idl_depAdd(idl_depDefGet(), fn);
            }
            os_free(fn);
        }
        /* QAC EXPECT 3416; No unexpected side effects here */
    } else if (idl_typeSpecType(typeSpec) != idl_tbasic) {
        fn = idl_stripIncludePath(idl_typeUser(typeSpec), userData);
        /* QAC EXPECT 3416, 5007; No unexpected side effects here, will not use wrapper */
        if (strlen(fn) && (strcmp(sn, fn) != 0)) {
            /* referenced type is in different scope */
            idl_depAdd(idl_depDefGet(), fn);
        }
        os_free(fn);
    } else {
        /* Do nothing, only to prevent dangling else-ifs QAC messages */
    }
    os_free(sn);
}



/* typedefOpenClose callback

   Generate dependencies for the following IDL construct:
   =>   typedef <type-name> <name>;
   For <type-name> determine if it is specified in the same file as
   this typedefinition self. If it is defined in another file, add
   a dependency to the dependency list.
*/
static void
idl_typedefOpenClose(
    idl_scope scope,
    const char *name,
    idl_typeDef defSpec,
    void *userData)
{
    idl_typeSpec arrayActual;
    idl_typeSpec seqActual;
    char *sn;
    char *fn;

    OS_UNUSED_ARG(name);

    sn = idl_getScopeFilename(scope, userData);
    /* QAC EXPECT 3416; No unexpected side effects here */
    if (idl_typeSpecType(idl_typeDefRefered(defSpec)) != idl_tbasic) {
        /* if the refered type is not basic */
        /* QAC EXPECT 3416; No unexpected side effects here */
        if (idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_tarray) {
            /* if it is an array */
            arrayActual = idl_typeArrayActual(idl_typeArray(idl_typeDefRefered(defSpec)));
            /* QAC EXPECT 3416; No unexpected side effects here */
            if (idl_typeSpecType(idl_typeSpec(arrayActual)) != idl_tbasic) {
                /* if the arrays actual type is not basic */
                fn = idl_stripIncludePath(idl_typeUser(arrayActual), userData);
                /* QAC EXPECT 3416, 5007; No unexpected side effects here, will not use wrapper */
                if (strlen (fn) && (strcmp(sn, fn) != 0)) {
                    /* referenced type is in different scope */
                    idl_depAdd(idl_depDefGet(), fn);
                }
                os_free(fn);
            }
           /* QAC EXPECT 3416; No unexpected side effects here */
        } else if (idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_tseq) {
            /* if it is an sequence */
            seqActual = idl_typeSeqActual(idl_typeSeq(idl_typeDefRefered(defSpec)));
            /* QAC EXPECT 3416; No unexpected side effects here */
            if (idl_typeSpecType(idl_typeSpec(seqActual)) != idl_tbasic) {
                /* if the sequence actual type is not basic */
                fn = idl_stripIncludePath(idl_typeUser(seqActual), userData);
                /* QAC EXPECT 3416, 5007; No unexpected side effects here, will not use wrapper */
                if (strlen (fn) && (strcmp(sn, fn) != 0)) {
                    /* referenced type is in different scope */
                    idl_depAdd(idl_depDefGet(), fn);
                }
                os_free(fn);
            }
        } else {
            /* the type is a structure or an union */
            fn = idl_stripIncludePath(idl_typeUser(idl_typeDefRefered(defSpec)), userData);
            /* QAC EXPECT 3416, 5007; No unexpected side effects here, will not use wrapper */
            if (strlen (fn) && (strcmp(sn, fn) != 0)) {
                /* referenced type is in different scope */
                idl_depAdd(idl_depDefGet(), fn);
            }
            os_free(fn);
        }
    }
    os_free(sn);
}

/* constantOpenClose callback

   Generate dependencies for the following IDL construct:
   =>   const <type-name> <name>;
   For <type-name> determine if it is specified in the same file as
   this const definition self. If it is defined in another file, add
   a dependency to the dependency list.
*/
static void
idl_constantOpenClose(
    idl_scope scope,
    idl_constSpec constantSpec,
    void *userData)
{
    idl_typeSpec typeSpec;
    OS_UNUSED_ARG(userData);

    typeSpec = idl_constSpecTypeGet(constantSpec);
    if (idl_typeSpecType(typeSpec) != idl_tbasic) {
        char *sn = idl_getScopeFilename(scope, userData);
        char *fn = idl_stripIncludePath(idl_typeUser(typeSpec), userData);

        if (strlen (fn) && (strcmp(sn, fn) != 0)) {
            /* referenced type is in different scope */
            idl_depAdd(idl_depDefGet(), fn);
        }
        os_free(sn);
        os_free(fn);
    }

}

/* unionOpen callback

   Generate dependencies for the following IDL construct:
   =>   union <union-name> switch(<switch-type>) {
            case label1.1; .. case label1.n;
                <union-case-1>;
            case label2.1; .. case label2.n;
                ...        ...
            case labeln.1; .. case labeln.n;
                <union-case-n>;
            default:
                <union-case-m>;
        };
   For <switch-type> determine if it is specified in the same file as
   this union self. If it is defined in another file, add a dependency
   to the dependency list.
*/
static idl_action
idl_unionOpen (
    idl_scope scope,
    const char *name,
    idl_typeUnion unionSpec,
    void *userData)
{
    OS_UNUSED_ARG(name);

    /* because the switch type is of integral type,
       if it is not basic type, it can only be an enumeration
       or a typedefed basic type or enumeration
    */
    /* QAC EXPECT 3416; No unexpected side effects here */
    if ((idl_typeSpecType(idl_typeUnionSwitchKind(unionSpec)) != idl_tbasic)) { 
        /* QAC EXPECT 5007; will not use wrapper */
        char *sn = idl_getScopeFilename(scope, userData);
        char *fn = idl_stripIncludePath(idl_typeUser(idl_typeUnionSwitchKind(unionSpec)), userData);

        if (strlen (fn) && (strcmp(sn, fn) != 0)) {
            /* referenced type is in different scope */
            idl_depAdd(idl_depDefGet(), fn);
        }
        os_free(sn);
        os_free(fn);
    }
    return idl_explore;
}

/* unionCaseOpenClose callback
                                                                                                                          
   Generate dependencies for the following IDL construct:
        union <union-name> switch(<switch-type>) {
            case label1; .. case labeln;
            case label1.1; .. case label1.n;
   =>           <union-case-1>;
            case label2.1; .. case label2.n;
   =>           ...        ...
            case labeln.1; .. case labeln.n;
   =>           <union-case-n>;
            default:
   =>           <union-case-m>;
        };
   For each union case determine if the case (actual) type is
   defined within the same file as the union or in another file.
   If it is defined in another file, add a dependency to the dependency
   list.
*/
static void
idl_unionCaseOpenClose (
    idl_scope scope,
    const char *name,
    idl_typeSpec typeSpec,
    void *userData)
{
    idl_typeSpec actualType;
    char *sn;
    char *fn;
    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(userData);

    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(userData);

    sn = idl_getScopeFilename(scope, userData);
    /* QAC EXPECT 3416; No unexpected side effects here */
    if (idl_typeSpecType(typeSpec) == idl_tarray) {
        actualType = idl_typeArrayActual(idl_typeArray(typeSpec));
        /* QAC EXPECT 3416; No unexpected side effects here */
        if (idl_typeSpecType(actualType) != idl_tbasic) {
            fn = idl_stripIncludePath(idl_typeUser(actualType), userData);

            /* QAC EXPECT 3416, 5007; No unexpected side effects here, will not use wrapper */
            if (strlen (fn) && (strcmp(sn, fn) != 0)) {
                /* referenced type is in different scope */
                idl_depAdd(idl_depDefGet(), fn);
            }
            os_free(fn);
        }
        /* QAC EXPECT 3416; No unexpected side effects here */
    } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
        actualType = idl_typeSeqActual(idl_typeSeq(typeSpec));
        /* QAC EXPECT 3416; No unexpected side effects here */
        if (idl_typeSpecType(actualType) != idl_tbasic) {
            fn = idl_stripIncludePath(idl_typeUser(actualType), userData);

            /* QAC EXPECT 3416, 5007; No unexpected side effects here, will not use wrapper */
            if (strlen (fn) && (strcmp(sn, fn) != 0)) {
                /* referenced type is in different scope */
                idl_depAdd(idl_depDefGet(), fn);
            }
            os_free(fn);
        }
        /* QAC EXPECT 3416; No unexpected side effects here */
    } else if (idl_typeSpecType(typeSpec) != idl_tbasic) { 
        fn = idl_stripIncludePath(idl_typeUser(typeSpec), userData);

        /* QAC EXPECT 3416, 5007; No unexpected side effects here, will not use wrapper */
        if (strlen (fn) && (strcmp(sn, fn) != 0)) {
            /* referenced type is in different scope */
            idl_depAdd(idl_depDefGet(), fn);
        }
        os_free(fn);
    } else {
        /* Do nothing, only to prevent dangling else-ifs QAC messages */
    }
    os_free(sn);
}

/* QAC EXPECT 5007; Bypass qactools bug */
/* Standard control structure to specify that inline
   type definitions are to be processed prior to the
   type itself in contrast with inline.
*/
static idl_programControl idl_genSpliceDepControl = {
        idl_prior
    };

/* programControl returns the locally specified
   program control settings
*/
static idl_programControl *
idl_getControl(
    void *userData)
{
    OS_UNUSED_ARG(userData);
    return &idl_genSpliceDepControl;
}

/* idl_genSpliceDep specifies the local
   callback routines
*/
static struct idl_program idl_genSpliceDep;

/* genSpliceDepProgram returns the local
   table of callback routines.
*/
idl_program
idl_genSpliceDepProgram(
    void *userData)
{
    /* The following control is required to allow idl_walk  */
    /* to analyse anonymous types within structs and unions */
    /* A special program for that would be more clear but   */
    /* also slower                                          */
    idl_genSpliceDep.idl_getControl                     = idl_getControl;
    idl_genSpliceDep.fileOpen                           = idl_fileOpen;
    idl_genSpliceDep.fileClose                          = NULL;
    idl_genSpliceDep.moduleOpen                         = idl_moduleOpen;
    idl_genSpliceDep.moduleClose                        = NULL;
    idl_genSpliceDep.structureOpen                      = idl_structureOpen;
    idl_genSpliceDep.structureClose                     = NULL;
    idl_genSpliceDep.structureMemberOpenClose           = idl_structureMemberOpenClose;
    idl_genSpliceDep.enumerationOpen                    = NULL;
    idl_genSpliceDep.enumerationClose                   = NULL;
    idl_genSpliceDep.enumerationElementOpenClose        = NULL;
    idl_genSpliceDep.unionOpen                          = idl_unionOpen;
    idl_genSpliceDep.unionClose                         = NULL;
    idl_genSpliceDep.unionCaseOpenClose                 = idl_unionCaseOpenClose;
    idl_genSpliceDep.unionLabelsOpenClose               = NULL;
    idl_genSpliceDep.unionLabelOpenClose                = NULL;
    idl_genSpliceDep.typedefOpenClose                   = idl_typedefOpenClose;
    idl_genSpliceDep.boundedStringOpenClose             = NULL;
    idl_genSpliceDep.sequenceOpenClose                  = NULL;
    idl_genSpliceDep.constantOpenClose                  = idl_constantOpenClose;
    idl_genSpliceDep.artificialDefaultLabelOpenClose    = NULL;
    idl_genSpliceDep.userData                           = userData;

    return &idl_genSpliceDep;
}

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
/*
   This module generates IDL file dependencies based upon
   the IDL input file base names.
*/

#include "idl_program.h"
#include "idl_scope.h"
#include "idl_dependencies.h"
#include "idl_genSpliceDep.h"

/* fileOpen callback

   return idl_explore to state that the rest of the file needs to be processed
*/
static idl_action
idl_fileOpen(
    idl_scope scope,
    const char *name,
    void *userData)
{
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
    char *bn;

    /* QAC EXPECT 3416; No unexpected side effects here */
    if (idl_typeSpecType(typeSpec) == idl_tarray) {
        actualType = idl_typeArrayActual(idl_typeArray(typeSpec));
        /* QAC EXPECT 3416; No unexpected side effects here */
        if (idl_typeSpecType(actualType) != idl_tbasic) {
            bn = idl_scopeBasename(idl_typeUserScope(idl_typeUser(actualType)));
            /* QAC EXPECT 3416, 5007; No unexpected side effects here, will not use wrapper */
            if (strlen(bn) && (strcmp(idl_scopeBasename(scope), bn) != 0)) {
                /* referenced type is in different scope */
                idl_depAdd(idl_depDefGet(), bn);
            }
        }
        /* QAC EXPECT 3416; No unexpected side effects here */
    } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
        actualType = idl_typeSeqActual(idl_typeSeq(typeSpec));
        /* QAC EXPECT 3416; No unexpected side effects here */
        if (idl_typeSpecType(actualType) != idl_tbasic) {
            bn = idl_scopeBasename(idl_typeUserScope(idl_typeUser(actualType)));
            /* QAC EXPECT 3416, 5007; No unexpected side effects here, will not use wrapper */
            if (strlen(bn) && (strcmp (idl_scopeBasename(scope), bn) != 0)) {
                /* referenced type is in different scope */
                idl_depAdd(idl_depDefGet(), bn);
            }
        }
        /* QAC EXPECT 3416; No unexpected side effects here */
    } else if (idl_typeSpecType(typeSpec) != idl_tbasic) {
        bn = idl_scopeBasename(idl_typeUserScope(idl_typeUser(typeSpec)));
        /* QAC EXPECT 3416, 5007; No unexpected side effects here, will not use wrapper */
        if (strlen(bn) && (strcmp(idl_scopeBasename(scope), bn) != 0)) {
            /* referenced type is in different scope */
            idl_depAdd(idl_depDefGet(), bn);
        }
    } else {
        /* Do nothing, only to prevent dangling else-ifs QAC messages */
    }
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
    char *bn;

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
                bn = idl_scopeBasename(idl_typeUserScope(idl_typeUser(arrayActual)));
                /* QAC EXPECT 3416, 5007; No unexpected side effects here, will not use wrapper */
                if (strlen (bn) && (strcmp(idl_scopeBasename(scope), bn) != 0)) {
                    /* referenced type is in different scope */
                    idl_depAdd(idl_depDefGet(), bn);
                }
	       }
	       /* QAC EXPECT 3416; No unexpected side effects here */
        } else if (idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_tseq) {
            /* if it is an sequence */
            seqActual = idl_typeSeqActual(idl_typeSeq(idl_typeDefRefered(defSpec)));
            /* QAC EXPECT 3416; No unexpected side effects here */
            if (idl_typeSpecType(idl_typeSpec(seqActual)) != idl_tbasic) {
                /* if the sequence actual type is not basic */
                bn = idl_scopeBasename(idl_typeUserScope(idl_typeUser(seqActual)));
                /* QAC EXPECT 3416, 5007; No unexpected side effects here, will not use wrapper */
                if (strlen (bn) && (strcmp(idl_scopeBasename(scope), bn) != 0)) {
                    /* referenced type is in different scope */
                    idl_depAdd(idl_depDefGet(), bn);
                }
            }
        } else {
            /* the type is a structure or an union */
            bn = idl_scopeBasename(idl_typeUserScope(idl_typeUser(idl_typeDefRefered(defSpec))));
            /* QAC EXPECT 3416, 5007; No unexpected side effects here, will not use wrapper */
            if (strlen (bn) && (strcmp(idl_scopeBasename(scope), bn) != 0)) {
                /* referenced type is in different scope */
                idl_depAdd(idl_depDefGet(), bn);
            }
        }
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
    char *bn;

    /* because the switch type is of integral type,
       if it is not basic type, it can only be an enumeration
       or a typedefed basic type or enumeration
    */
    /* QAC EXPECT 3416; No unexpected side effects here */
    if ((idl_typeSpecType(idl_typeUnionSwitchKind(unionSpec)) != idl_tbasic)) { 
        /* QAC EXPECT 5007; will not use wrapper */
        bn = idl_scopeBasename(idl_typeUserScope(idl_typeUser(idl_typeUnionSwitchKind(unionSpec))));
        if (strlen (bn) && (strcmp(idl_scopeBasename(scope), bn) != 0)) {
            /* referenced type is in different scope */
            idl_depAdd(idl_depDefGet(), bn);
        }
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
    char *bn;

    /* QAC EXPECT 3416; No unexpected side effects here */
    if (idl_typeSpecType(typeSpec) == idl_tarray) {
        actualType = idl_typeArrayActual(idl_typeArray(typeSpec));
        /* QAC EXPECT 3416; No unexpected side effects here */
        if (idl_typeSpecType(actualType) != idl_tbasic) {
            bn = idl_scopeBasename(idl_typeUserScope(idl_typeUser(actualType)));
            /* QAC EXPECT 3416, 5007; No unexpected side effects here, will not use wrapper */
            if (strlen (bn) && (strcmp(idl_scopeBasename(scope), bn) != 0)) {
                /* referenced type is in different scope */
                idl_depAdd(idl_depDefGet(), bn);
            }
        }
        /* QAC EXPECT 3416; No unexpected side effects here */
    } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
        actualType = idl_typeSeqActual(idl_typeSeq(typeSpec));
        /* QAC EXPECT 3416; No unexpected side effects here */
        if (idl_typeSpecType(actualType) != idl_tbasic) {
            bn = idl_scopeBasename(idl_typeUserScope(idl_typeUser(actualType)));
            /* QAC EXPECT 3416, 5007; No unexpected side effects here, will not use wrapper */
            if (strlen (bn) && (strcmp(idl_scopeBasename(scope), bn) != 0)) {
                /* referenced type is in different scope */
                idl_depAdd(idl_depDefGet(), bn);
            }
        }
        /* QAC EXPECT 3416; No unexpected side effects here */
    } else if (idl_typeSpecType(typeSpec) != idl_tbasic) { 
        bn = idl_scopeBasename(idl_typeUserScope(idl_typeUser(typeSpec)));
        /* QAC EXPECT 3416, 5007; No unexpected side effects here, will not use wrapper */
        if (strlen (bn) && (strcmp(idl_scopeBasename(scope), bn) != 0)) {
            /* referenced type is in different scope */
            idl_depAdd(idl_depDefGet(), bn);
        }
    } else {
        /* Do nothing, only to prevent dangling else-ifs QAC messages */
    }
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
    return &idl_genSpliceDepControl;
}

/* idl_genSpliceDep specifies the local
   callback routines
*/
static struct idl_program
idl_genSpliceDep = {
        /* The following control is required to allow idl_walk	*/
	/* to analyse anonymous types within structs and unions */
	/* A special program for that would be more clear but   */
	/* also slower                                          */
    idl_getControl,
    idl_fileOpen,
    NULL, /* idl_fileClose */
    idl_moduleOpen,
    NULL, /* idl_moduleClose */
    idl_structureOpen,
    NULL, /* idl_structureClose */
    idl_structureMemberOpenClose,
    NULL, /* idl_enumerationOpen */
    NULL, /* idl_enumerationClose */
    NULL, /* idl_enumerationElementOpenClose */
    idl_unionOpen,
    NULL, /* idl_unionClose */
    idl_unionCaseOpenClose,
    NULL, /* idl_unionLabelsOpenClose */
    NULL, /* idl_unionLabelOpenClose */
    idl_typedefOpenClose,
    NULL, /* idl_boundedStringOpenClose */
    NULL, /* idl_sequenceOpenClose */
    NULL, /* idl_constantOpenClose */
    NULL, /* idl_artificialDefaultLabelOpenClose */
    NULL  /* userData */
};

/* genSpliceDepProgram returns the local
   table of callback routines.
*/
idl_program
idl_genSpliceDepProgram(
    void)
{
    return &idl_genSpliceDep;
}

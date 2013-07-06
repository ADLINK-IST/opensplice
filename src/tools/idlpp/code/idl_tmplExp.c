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
#include "c_iterator.h"
#include <ctype.h>
#include "os_heap.h"
#include "os_stdlib.h"

#include "idl_tmplExp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define NMSIZE		(100)

/****************************************************************
 * Stream Exp Macro Attributes Class Implementation             *
 ****************************************************************/
C_STRUCT(idl_macroAttrib) {
    c_char	startToken;
    c_char	openToken;
    c_char	closeToken;
};

idl_macroAttrib
idl_macroAttribNew (
    c_char startToken,
    c_char openToken,
    c_char closeToken)
{
    /* QAC EXPECT 5007; will not use wrapper */
    idl_macroAttrib macroAttrib = os_malloc ((size_t)C_SIZEOF(idl_macroAttrib));

    macroAttrib->startToken = startToken;
    macroAttrib->openToken = openToken;
    macroAttrib->closeToken = closeToken;

    return macroAttrib;
}

void
idl_macroAttribFree (
    const idl_macroAttrib macroAttrib)
{
    /* QAC EXPECT 5007; will not use wrapper */
    os_free (macroAttrib);
}

/****************************************************************
 * Stream Exp Macro Class Implementation                        *
 ****************************************************************/
C_STRUCT(idl_macro) {
    c_char *name;
    c_char *value;
};

idl_macro
idl_macroNew (
    const c_char *name,
    const c_char *value)
{
    /* QAC EXPECT 5007; will not use wrapper */
    idl_macro macro = os_malloc((size_t)C_SIZEOF(idl_macro));

    macro->name = os_strdup (name);
    macro->value = os_strdup (value);
    return (macro);
}

void
idl_macroFree (
    const idl_macro macro)
{
    /* QAC EXPECT 5007; will not use wrapper */
    os_free (macro->name);
    /* QAC EXPECT 5007; will not use wrapper */
    os_free (macro->value);
    /* QAC EXPECT 5007; will not use wrapper */
    os_free (macro);
}

c_char
*idl_macroName (
    /* QAC EXPECT 3673; Can not be solved here */
    const idl_macro macro)
{
    return macro->name;
}

c_char
*idl_macroValue (
    /* QAC EXPECT 3673; Can not be solved here */
    const idl_macro macro)
{
    return macro->value;
}

/****************************************************************
 * Stream Exp Macro Set Class Implementation                    *
 ****************************************************************/
C_STRUCT(idl_macroSet) {
    c_iter macroSet;
};

static c_equality
idl_macroNameMatch(
    /* QAC EXPECT 3673; Can not be solved here */
    void *_macro,
    void *_name)
{
    idl_macro macro;
    c_char *name;
    c_equality result = C_NE;

    macro = _macro;
    name = _name;

    /* QAC EXPECT 3416; No unexpected side effect here */
    if (strcmp (macro->name, name) == 0) {
        result = C_EQ;
    }
    return result;
}

idl_macroSet
idl_macroSetNew (
    void)
{
    idl_macroSet macroSet = os_malloc ((size_t)C_SIZEOF(idl_macroSet));

    macroSet->macroSet = c_iterNew (0);
    return macroSet;
}

void
idl_macroSetFree (
    const idl_macroSet macroSet)
{
    idl_macroSetClear (macroSet);
    c_iterFree (macroSet->macroSet);
    /* QAC EXPECT 5007; will not use wrapper */
    os_free (macroSet);
}

void
idl_macroSetAdd (
    /* QAC EXPECT 3673; Can not be solved here */
    const idl_macroSet macroSet,
    const idl_macro macro)
{
    idl_macro old_macro;

    old_macro = c_iterResolve (macroSet->macroSet, idl_macroNameMatch, macro->name);
    if (old_macro) {
        c_iterTake (macroSet->macroSet, old_macro);
    }
    c_iterInsert (macroSet->macroSet, macro);
}

void
idl_macroSetRemove (
    /* QAC EXPECT 3673; Can not be solved here */
    const idl_macroSet macroSet,
    const idl_macro macro)
{
    c_iterTake (macroSet->macroSet, macro);
}

void
idl_macroSetClear (
    /* QAC EXPECT 3673; Can not be solved here */
    const idl_macroSet macroSet)
{
    idl_macro macro;

    macro = c_iterTakeFirst (macroSet->macroSet);
    while (macro != NULL) {
        idl_macroFree (macro);
        macro = c_iterTakeFirst (macroSet->macroSet);
    }
}

idl_macro
idl_macroSetGet (
    /* QAC EXPECT 3673; Can not be solved here */
    const idl_macroSet macroSet,
    const c_char *name)
{
    idl_macro macro;

    macro = c_iterResolve(macroSet->macroSet,
                idl_macroNameMatch, (c_iterResolveCompareArg)name);
    return macro;
}

/****************************************************************
 * Stream Exp Generic Stream Class Implementation               *
 ****************************************************************/
C_STRUCT(idl_stream) {
    c_char	*stream;
    c_long	length;
    c_long	curpos;
};

idl_stream
idl_streamInit (
    const idl_stream stream,
    const c_char *stream_val)
{
    stream->stream = os_strdup(stream_val);
    stream->length = strlen (stream_val);
    stream->curpos = 0;
    return stream;
}

void
idl_streamExit (
    /* QAC EXPECT 3673; Can not be solved here */
    const idl_stream stream)
{
    /* QAC EXPECT 5007; will not use wrapper */
    os_free (stream->stream);
}

c_char *
idl_streamGet (
    /* QAC EXPECT 3673; Can not be solved here */
    const idl_stream stream)
{
    return stream->stream;
}

c_char *
idl_streamCurGet (
    /* QAC EXPECT 3673; Can not be solved here */
    const idl_stream stream)
{
    return &stream->stream[stream->curpos];
}

c_long
idl_streamLength (
    const idl_stream stream)
{
    return stream->length;
}

/****************************************************************
 * Stream Exp Input Stream Class Implementation                 *
 ****************************************************************/
C_STRUCT(idl_streamIn) {
    C_EXTENDS(idl_stream);
    idl_macroAttrib macroAttrib;
    idl_macroSet macroSet;
};

idl_streamIn
idl_streamInNew (
    const c_char *stream_val,
    const idl_macroAttrib macroAttrib)
{
    idl_streamIn	stream = os_malloc ((size_t)C_SIZEOF(idl_streamIn));

    assert (stream_val);
    assert (macroAttrib);
    idl_streamInit (idl_stream(stream), stream_val);
    stream->macroAttrib = macroAttrib;
    return stream;
}

void
idl_streamInFree (
    const idl_streamIn stream)
{
    idl_streamExit (idl_stream(stream));
    /* QAC EXPECT 5007; will not use wrapper */
    os_free (stream);
}

c_char
idl_streamInCur (
    /* QAC EXPECT 3673; Can not be solved here */
    const idl_streamIn stream)
{
    idl_stream str = idl_stream(stream);

    return str->stream[str->curpos];
}

c_char
idl_streamInRel (
    /* QAC EXPECT 3673; Can not be solved here */
    const idl_streamIn stream,
    c_long offset)
{
    c_long abspos;
    idl_stream str = idl_stream(stream);

    abspos = str->curpos + offset;
    if (abspos >= str->length) {
        abspos = str->length -1;
    }
    if (abspos < 0) {
        abspos = 0;
    }
    return str->stream[abspos];
}

void
idl_streamInWind (
    /* QAC EXPECT 3673; Can not be solved here */
    const idl_streamIn stream)
{
    idl_stream str = idl_stream(stream);

    if (str->curpos < str->length) {
        str->curpos++;
    }
}

c_char
idl_streamInWindCur (
    /* QAC EXPECT 3673; Can not be solved here */
    const idl_streamIn stream)
{
    idl_stream str = idl_stream(stream);

    if (str->curpos < str->length) {
        str->curpos++;
    }
    return str->stream[str->curpos];
}

/****************************************************************
 * Stream Expander Output Stream Class Implementation           *
 ****************************************************************/
C_STRUCT(idl_streamOut) {
    C_EXTENDS(idl_stream);
    c_long max_length;
};

idl_streamOut
idl_streamOutNew (
    c_long max_length)
{
    idl_streamOut stream = os_malloc ((size_t)C_SIZEOF(idl_streamOut));

    idl_streamInit (idl_stream(stream), "");
    stream->max_length = max_length;
    return stream;
}

void
idl_streamOutFree (
    const idl_streamOut stream)
{
    idl_streamExit (idl_stream(stream));
    /* QAC EXPECT 5007; will not use wrapper */
    os_free (stream);
}

c_long
idl_streamOutPut (
    /* QAC EXPECT 3673; Can not be solved here */
    const idl_streamOut stream,
    c_char character)
{
    idl_stream str = idl_stream(stream);

    if (stream->max_length == 0) {
        if (((str->curpos) % 100) == 0) {
            str->stream = os_realloc (str->stream, (size_t)(str->curpos + 101));
	    }
        str->stream [str->curpos] = character;
        str->curpos++;
        str->stream [str->curpos] = '\0';
        str->length++;
    } else {
        if (str->length < stream->max_length) {
            str->stream[str->curpos] = character;
            str->curpos++;
            str->stream [str->curpos] = '\0';
            str->length++;
        }
    }
    return str->curpos;
}

/****************************************************************
 * File Output Stream Class Implementation                      *
 ****************************************************************/
C_STRUCT(idl_fileOut) {
    FILE *file;
};

static c_char* idl_outputdir = NULL;

c_char*
idl_dirOutCur(
    void)
{
    return idl_outputdir;
}

void
idl_dirOurFree(
    void)
{
    if(idl_outputdir){
        os_free(idl_outputdir);
    }
    return;
}

c_bool
idl_dirOutNew(
    const c_char *name)
{
    c_bool result;
    os_result status;
    char dirName[OS_PATH_MAX];
    struct os_stat statBuf;
    unsigned int i;

    memset(dirName, 0, OS_PATH_MAX);

    if (name) {
        result = TRUE;

        for (i=0; i<strlen(name) && result; i++) {
            if ((name[i] == OS_FILESEPCHAR) && (i != 0)) {
                status = os_stat(dirName, &statBuf);

                if (status != os_resultSuccess) {
                    os_mkdir(dirName, S_IRWXU | S_IRWXG | S_IRWXO);
                    status = os_stat(dirName, &statBuf);
                }
                if (!OS_ISDIR (statBuf.stat_mode)) {
#ifdef WIN32
                    if ((strlen(dirName) == 2) && (dirName[1] == ':')) {
                        /*This is a device like for instance: 'C:'*/
                    } else {
                        printf ("'%s' is not a directory\n", dirName);
                        result = FALSE;
                        idl_outputdir = NULL;
                    }
#else
                    printf ("'%s' is not a directory\n", dirName);
                    result = FALSE;
                    idl_outputdir = NULL;
#endif
                }
            }
            dirName[i] = name[i];
        }
        if (result) {
            if (dirName[i-1] != OS_FILESEPCHAR) {
                status = os_stat(dirName, &statBuf);

                if (status != os_resultSuccess) {
                    os_mkdir(dirName, S_IRWXU | S_IRWXG | S_IRWXO);
                    status = os_stat(dirName, &statBuf);
                }
                idl_outputdir = os_strdup(name);

                if (!OS_ISDIR (statBuf.stat_mode)) {
#ifdef WIN32
                    if ((strlen(dirName) == 2) && (dirName[1] == ':')) {
                        /*This is a device like for instance: 'C:'. Check if it exists...*/
                        dirName[2] = OS_FILESEPCHAR;
                        status = os_stat(dirName, &statBuf);

                        if (status == os_resultFail) {
                            printf("'%s' is not available", dirName);
                            result = FALSE;
                            idl_outputdir = NULL;
                        }
                    } else {
                        printf("'%s' is not a directory.\n", idl_outputdir);
                        result = FALSE;
                        idl_outputdir = NULL;
                    }
#else
                    printf ("'%s' is not a directory\n", dirName);
                    result = FALSE;
                    idl_outputdir = NULL;
#endif
                }
            } else {
                idl_outputdir = (c_char*)os_malloc(strlen(name));
                snprintf(idl_outputdir, strlen(name), "%s", name);
                idl_outputdir[strlen(name) - 1] = '\0';
            }
        }
    } else {
        result = FALSE;
        idl_outputdir = NULL;
    }

    if (result) {
        status = os_access(idl_outputdir, 2); /*Check whether dir is writable*/

        if(status != os_resultSuccess){
#ifdef WIN32
            if ((strlen(dirName) == 2) && (dirName[1] == ':')) {
                /*This is a device like for instance: 'C:'. Check if it exists...*/
                dirName[2] = OS_FILESEPCHAR;
                status = os_stat(dirName, &statBuf);

                if (status == os_resultFail) {
                    printf("'%s' cannot be found", dirName);
                    result = FALSE;
                    idl_outputdir = NULL;
                }
            } else {
                printf("Specified output directory '%s' is not writable.\n", idl_outputdir);
                result = FALSE;
                idl_outputdir = NULL;
            }
#else
            printf("Specified output directory '%s' is not writable.\n", idl_outputdir);
            result = FALSE;
            idl_outputdir = NULL;
#endif
        }
    }

    return result;
}

idl_fileOut
idl_fileOutNew (
    const c_char *name,
    const c_char *mode)
{
    idl_fileOut stream = os_malloc ((size_t)C_SIZEOF(idl_fileOut));
    c_char *fname;

    if(idl_outputdir){
        fname = os_malloc(strlen(idl_outputdir) + strlen(os_fileSep()) + strlen(name) + 1);
        os_sprintf(fname, "%s%s%s", idl_outputdir, os_fileSep(), name);
    } else {
        fname = os_strdup(name);
    }
    stream->file = fopen (fname, mode);

    if (stream->file == NULL)
    {
        /* QAC EXPECT 5007; will not use wrapper */
        os_free (stream);
        stream = NULL;
    }
    os_free(fname);
    return stream;
}

void
idl_fileOutFree (
    const idl_fileOut stream)
{
    fclose (stream->file);
    /* QAC EXPECT 5007; will not use wrapper */
    os_free (stream);
}

c_long
idl_fileOutPut (
    const idl_fileOut stream,
    c_char character)
{
    c_long result;

    result = putc (character, stream->file);
    return result;
}

void
idl_fileOutPrintf (
    /* QAC EXPECT 3673; Can not be solved here */
    const idl_fileOut stream,
    const c_char *format,
    ...)
{
    va_list args;

    if(stream)
    {
        va_start (args, format);
        vfprintf (stream->file, format, args);
        va_end (args);
    }
}

static idl_fileOut idl_fileCurrentOut = NULL;

void
idl_fileSetCur (
    const idl_fileOut fileOut)
{
    idl_fileCurrentOut = fileOut;
}

idl_fileOut
idl_fileCur (
    void)
{
    return idl_fileCurrentOut;
}

/****************************************************************
 * Stream Exp Tmpl Exp Class Implementation                     *
 ****************************************************************/
C_STRUCT(idl_tmplExp) {
    idl_macroSet macroSet;
};

static void
idl_tmplExpInsertMacro (
    const idl_tmplExp tmplExp,
    const c_char *name,
    const idl_streamOut so,
    const idl_macroAttrib macroAttrib);

static const c_char *
idl_tmplExpMacroValue (
    const idl_tmplExp tmplExp,
    const c_char *name);

static void
idl_tmplExpDeleteMacro (
    const idl_tmplExp tmplExp,
    const c_char *name);

static void
idl_tmplExpInsertText (
    const idl_streamOut so,
    const c_char *text);

static int
idl_tmplExpGetMacroSingleArg (
    const idl_tmplExp tmplExp,
    const idl_streamIn si,
    c_char *arg);

static int
idl_tmplExpGetMacroDoubleArg (
    const idl_tmplExp tmplExp,
    const idl_streamIn si,
    c_char *arg1,
    const c_char *arg2);

static int
idl_tmplExpProcessMacro (
    const idl_tmplExp tmplExp,
    const idl_streamIn si,
    const idl_streamOut so);

idl_tmplExp
idl_tmplExpNew (
    const idl_macroSet macroSet)
{
    idl_tmplExp tmplExp = os_malloc ((size_t)C_SIZEOF(idl_tmplExp));

    tmplExp->macroSet = macroSet;
    return tmplExp;
}

void
idl_tmplExpFree (
    const idl_tmplExp tmplExp)
{
    /* QAC EXPECT 5007; will not use wrapper */
    os_free (tmplExp);
}

static void
idl_tmplExpInsertMacro (
    const idl_tmplExp tmplExp,
    const c_char *name,
    const idl_streamOut so,
    const idl_macroAttrib macroAttrib)
{
    int inserted = 0;
    idl_streamIn macroStream;
    idl_macro macro;

    macro = idl_macroSetGet (tmplExp->macroSet, name);
    if (macro) {
        macroStream = idl_streamInNew (idl_macroValue(macro), macroAttrib);

        /* QAC EXPECT 3416; No unexpected side effect here */
    	while (idl_streamInCur(macroStream) != '\0') {
            /* QAC EXPECT 3416; No unexpected side effect here */
            if (idl_streamInCur(macroStream) == macroAttrib->startToken) {
                idl_tmplExpProcessMacro (tmplExp, macroStream, so);
            } else {
                idl_streamOutPut(so, idl_streamInCur(macroStream));
                idl_streamInWind(macroStream);
            }
        }
        idl_streamOutPut(so, '\0');
        idl_streamInFree (macroStream);
        inserted = 1;
    }
    if (!inserted) {
        if (os_getenv(name)) {
            c_char *value;

            value = os_getenv (name);
            macroStream = idl_streamInNew (value, macroAttrib);

            /* QAC EXPECT 3416; No unexpected side effect here */
    	    while (idl_streamInCur(macroStream) != '\0') {
                /* QAC EXPECT 3416; No unexpected side effect here */
                if (idl_streamInCur(macroStream) == macroAttrib->startToken) {
                    idl_tmplExpProcessMacro (tmplExp, macroStream, so);
                } else {
                    idl_streamOutPut(so, idl_streamInCur(macroStream));
                    idl_streamInWind(macroStream);
                }
            }
            idl_streamOutPut(so, '\0');
            inserted = 1;
        }
    }
    if (!inserted) {
        fprintf (stderr, "insert_macro: Undefined macro '%s'\n", name);
    }
}

static const c_char *
idl_tmplExpMacroValue (
    /* QAC EXPECT 3673; Can not be solved here */
    const idl_tmplExp tmplExp,
    const c_char *name)
{
    idl_macro macro;

    macro = idl_macroSetGet (tmplExp->macroSet, name);
    if (macro) {
        return idl_macroValue (macro);
    }
    /* QAC EXPECT 3416; No unexpected side effect here */
    if (os_getenv (name)) {
        return (os_getenv (name));
    }
    fprintf (stderr, "macro_value: Undefined macro '%s'\n", name);
    return "";
}

static void
idl_tmplExpDeleteMacro (
    /* QAC EXPECT 3673; Can not be solved here */
    const idl_tmplExp tmplExp,
    const c_char *name)
{
    int deleted = 0;
    idl_macro macro;

    macro = idl_macroSetGet (tmplExp->macroSet, name);
    if (macro) {
        idl_macroSetRemove (tmplExp->macroSet, macro);
        deleted = 1;
    }
    if (!deleted) {
        fprintf (stderr, "delete_macro: Undefined macro '%s'\n", name);
    }
}

static void
idl_tmplExpInsertText (
    const idl_streamOut so,
    const c_char *text)
{
    unsigned int y;

    /* QAC EXPECT 3416; No unexpected side effect here */
    for (y = 0; y < strlen (text); y++) {
        idl_streamOutPut (so, text[y]);
    }
}

static int
idl_tmplExpGetMacroSingleArg (
    const idl_tmplExp tmplExp,
    const idl_streamIn si,
    c_char *arg)
{
    idl_streamOut argStream;

    argStream = idl_streamOutNew (0);
    /* QAC EXPECT 3416; No unexpected side effect here */
    while (idl_streamInCur(si) != '\n' && idl_streamInCur(si) != si->macroAttrib->closeToken) {
        /* QAC EXPECT 3416; No unexpected side effect here */
        if (idl_streamInCur(si) == si->macroAttrib->startToken) {
            idl_tmplExpProcessMacro (tmplExp, si, argStream);
        } else {
            idl_streamOutPut (argStream, idl_streamInCur(si));
            idl_streamInWind(si);
        }
    }
    idl_streamOutPut (argStream, '\0');
    os_strcpy (arg, idl_streamGet(idl_stream(argStream)));
    idl_streamOutFree (argStream);
    /* QAC EXPECT 3416; No unexpected side effect here */
    if (idl_streamInCur(si) == si->macroAttrib->closeToken) {
        idl_streamInWind(si);
        return 1;
    }
    return 0;
}

static int
idl_tmplExpGetMacroDoubleArg (
    const idl_tmplExp tmplExp,
    const idl_streamIn si,
    c_char *arg1,
    const c_char *arg2)
{
    idl_streamOut arg1Stream;
    idl_streamOut arg2Stream;
    int return_val;

    arg1Stream = idl_streamOutNew (0);
    /* QAC EXPECT 3416; No unexpected side effect here */
    while (idl_streamInCur(si) != '\n' && idl_streamInCur(si) != ',') {
        /* QAC EXPECT 3416; No unexpected side effect here */
        if (idl_streamInCur(si) == si->macroAttrib->startToken) {
    	    idl_tmplExpProcessMacro (tmplExp, si, arg1Stream);
        } else {
            idl_streamOutPut (arg1Stream, idl_streamInCur(si));
    	    idl_streamInWind(si);
        }
    }
    idl_streamOutPut (arg1Stream, '\0');
    os_strcpy (arg1, idl_streamGet(idl_stream(arg1Stream)));
    idl_streamOutFree (arg1Stream);
    /* QAC EXPECT 3416; No unexpected side effect here */
    if (idl_streamInCur(si) == ',') {
        arg2Stream = idl_streamOutNew (0);
        idl_streamInWind(si);
        /* QAC EXPECT 3416; No unexpected side effect here */
        while (idl_streamInCur(si) != '\n' && idl_streamInCur(si) != si->macroAttrib->closeToken) {
            /* QAC EXPECT 3416; No unexpected side effect here */
            if (idl_streamInCur(si) == si->macroAttrib->startToken) {
    	        idl_tmplExpProcessMacro (tmplExp, si, arg2Stream);
            } else {
                idl_streamOutPut (arg2Stream, idl_streamInCur(si));
    	        idl_streamInWind(si);
    	    }
        }
        /* QAC EXPECT 3416; No unexpected side effect here */
        if (idl_streamInCur(si) == si->macroAttrib->closeToken) {
    	    idl_streamInWind(si);
            idl_streamOutPut (arg2Stream, '\0');
            return_val = 2;
        } else {
        	return_val = 1;
        }
	    idl_streamOutFree (arg2Stream);
	    return return_val;
    }
    return_val = 0;
    return return_val;
}

static int
idl_tmplExpProcessMacro (
    const idl_tmplExp tmplExp,
    const idl_streamIn si,
    const idl_streamOut so)
{
    idl_macro macro;
    int result = 0;

    idl_streamInWind(si); /* skip macro start character */

    /* QAC EXPECT 3416; No unexpected side effect here */
    if ((strncmp (idl_streamCurGet(idl_stream(si)), "def", 3) == 0) &&
        ((int)idl_streamInRel (si, 3) == (int)si->macroAttrib->openToken)) {
        int br_count = 1;
        idl_streamOut defName;
        idl_streamOut defValue;

        idl_streamInWind(si);
        idl_streamInWind(si);
        idl_streamInWind(si);
        idl_streamInWind(si);
        defName = idl_streamOutNew (0);
        defValue = idl_streamOutNew (0);
        /* QAC EXPECT 3416; No unexpected side effect here */
        while (idl_streamInCur(si) != '\n' && idl_streamInCur(si) != '=') {
         /* QAC EXPECT 3416; No unexpected side effect here */
            if (idl_streamInCur(si) == si->macroAttrib->startToken) {
                result = idl_tmplExpProcessMacro (tmplExp, si, defName);
            } else {
                idl_streamOutPut (defName, idl_streamInCur(si));
                idl_streamInWind(si);
            }
        }
        idl_streamOutPut (defName, '\0');
        if (idl_streamInCur(si) == '=') {
        	idl_streamInWind(si);
            /* QAC EXPECT 3416; No unexpected side effect here */
            while (idl_streamInCur(si) != '\n' &&
                   (idl_streamInCur(si) != si->macroAttrib->closeToken || br_count)) {
                idl_streamOutPut (defValue, idl_streamInCur(si));
                idl_streamInWind(si);
                /* QAC EXPECT 3416; No unexpected side effect here */
                if (idl_streamInCur(si) == si->macroAttrib->openToken) {
                    br_count++;
                } else if (idl_streamInCur(si) == si->macroAttrib->closeToken) {
                    br_count--;
                }
            }
            /* QAC EXPECT 3416; No unexpected side effect here */
            if (idl_streamInCur(si) == si->macroAttrib->closeToken) {
                idl_streamInWind(si);
                idl_streamOutPut (defValue, '\0');
                macro = idl_macroNew (idl_streamGet (idl_stream(defName)),
                idl_streamGet (idl_stream(defValue)));
                idl_macroSetAdd (tmplExp->macroSet, macro);
            } else {
                fprintf (stderr, "def: Incomplete definition '%s' missing '%c'\n",
                    idl_streamGet (idl_stream(defName)), si->macroAttrib->closeToken);
                result = 1;
            }
        } else {
            fprintf (stderr, "def: Incomplete definition '%s' missing '='\n",
            idl_streamGet (idl_stream(defName)));
            result = 1;
        }
        idl_streamOutFree (defName);
        idl_streamOutFree (defValue);
	/* QAC EXPECT 3416; No unexpected side effect here */
    } else if ((strncmp (idl_streamCurGet(idl_stream(si)), "undef", 5) == 0) &&
               ((int)idl_streamInRel (si, 5) == (int)si->macroAttrib->openToken)) {
         c_char macro_name [NMSIZE];

         idl_streamInWind(si);
         idl_streamInWind(si);
         idl_streamInWind(si);
         idl_streamInWind(si);
         idl_streamInWind(si);
         idl_streamInWind(si);
	/* QAC EXPECT 3416; No unexpected side effect here */
         if (idl_tmplExpGetMacroSingleArg (tmplExp, si, macro_name)) {
             idl_tmplExpDeleteMacro (tmplExp, macro_name);
         } else {
             fprintf (stderr, "undef: Incomplete function 'undef' missing '%c'\n",
                 si->macroAttrib->closeToken);
                 result = 1;
         }
	/* QAC EXPECT 3416; No unexpected side effect here */
    } else if (idl_streamInCur(si) == si->macroAttrib->openToken) {
         c_char macro_name [NMSIZE];

         idl_streamInWind(si);
	/* QAC EXPECT 3416; No unexpected side effect here */
         if (idl_tmplExpGetMacroSingleArg (tmplExp, si, macro_name)) {
             idl_tmplExpInsertMacro (tmplExp, macro_name, so, si->macroAttrib);
         } else {
             fprintf (stderr, "%c%c: Incomplete function '%c' missing '%c'\n",
                 si->macroAttrib->startToken, si->macroAttrib->openToken,
                 si->macroAttrib->startToken, si->macroAttrib->closeToken);
                 result = 1;
         }
	/* QAC EXPECT 3416; No unexpected side effect here */
    } else if ((strncmp (idl_streamCurGet(idl_stream(si)), "sp", 2) == 0) &&
               ((int)idl_streamInRel (si, 2) == (int)si->macroAttrib->openToken)) {
        c_char spaceCount [NMSIZE];
        unsigned int ni = 0;
        unsigned int spc = 0;

        idl_streamInWind(si);
        idl_streamInWind(si);
        idl_streamInWind(si);
	/* QAC EXPECT 3416; No unexpected side effect here */
        if (idl_tmplExpGetMacroSingleArg (tmplExp, si, spaceCount)) {
            sscanf (spaceCount, "%u", &spc);
            for (ni = 0; ni < spc; ni++) {
                idl_streamOutPut (so, ' ');
            }
        } else {
            fprintf (stderr, "upper: Incomplete function 'sp' missing ')'\n");
            result = 1;
        }
/* QAC EXPECT 3416; No unexpected side effect here */
    } else if ((strncmp (idl_streamCurGet(idl_stream(si)), "upper", 5) == 0) &&
               ((int)idl_streamInRel (si, 5) == (int)si->macroAttrib->openToken)) {
        c_char string [NMSIZE];
        unsigned int ni = 0;

        idl_streamInWind(si);
        idl_streamInWind(si);
        idl_streamInWind(si);
        idl_streamInWind(si);
        idl_streamInWind(si);
        idl_streamInWind(si);
        /* QAC EXPECT 3416; No unexpected side effect here */
        if (idl_tmplExpGetMacroSingleArg (tmplExp, si, string)) {
            /* QAC EXPECT 3416; No unexpected side effect here */
            for (ni = 0; ni < strlen(string); ni++) {
                string[ni] = toupper (string[ni]);
            }
            idl_tmplExpInsertText (so, string);
        } else {
            fprintf (stderr, "upper: Incomplete function 'upper' missing ')'\n");
            result = 1;
        }
	/* QAC EXPECT 3416; No unexpected side effect here */
    } else if ((strncmp (idl_streamCurGet(idl_stream(si)), "lower", 5) == 0) &&
               ((int)idl_streamInRel (si, 5) == (int)si->macroAttrib->openToken)) {
        c_char string [NMSIZE];
        unsigned int ni = 0;

        idl_streamInWind(si);
        idl_streamInWind(si);
        idl_streamInWind(si);
        idl_streamInWind(si);
        idl_streamInWind(si);
        idl_streamInWind(si);
        idl_streamInWind(si);
	/* QAC EXPECT 3416; No unexpected side effect here */
        if (idl_tmplExpGetMacroSingleArg (tmplExp, si, string)) {
    /* QAC EXPECT 3416; No unexpected side effect here */
            for (ni = 0; ni < strlen(string); ni++) {
                string[ni] = (c_char)tolower (string[ni]);
            }
            idl_tmplExpInsertText (so, string);
        } else {
            fprintf (stderr, "lower: Incomplete function 'lower' missing '%c'\n",
                si->macroAttrib->closeToken);
            result = 1;
        }
	/* QAC EXPECT 3416; No unexpected side effect here */
    } else if ((strncmp (idl_streamCurGet(idl_stream(si)), "ltrim", 5) == 0) &&
               ((int)idl_streamInRel (si, 5) == (int)si->macroAttrib->openToken)) {
        c_char src [NMSIZE];
        c_char trim [NMSIZE];
        c_char res [NMSIZE];
        c_char *substr;

        idl_streamInWind(si);
        idl_streamInWind(si);
        idl_streamInWind(si);
        idl_streamInWind(si);
        idl_streamInWind(si);
        idl_streamInWind(si);
        switch (idl_tmplExpGetMacroDoubleArg (tmplExp, si, src, trim)) {
        case 0:
            fprintf (stderr, "ltrim: Incomplete function 'ltrim' missing ','\n");
            result = 1;
        break;
        case 1:
            fprintf (stderr, "ltrim: Incomplete function 'ltrim' missing '%c'\n",
                si->macroAttrib->closeToken);
            result = 1;
        break;
        case 2:
            substr = strstr (src, trim);
            if (substr == NULL) {
	/* QAC EXPECT 5007; will not use wrapper */
                os_strcpy (res, src);
            } else {
	/* QAC EXPECT 0488; Mind your own bissiness */
                substr += strlen(trim);
                os_strcpy (res, substr);
            }
            idl_tmplExpInsertText (so, res);
        break;
        default:
            printf ("Unexpected case\n");
        break;
    }
	/* QAC EXPECT 3416; No unexpected side effect here */
    } else if ((strncmp (idl_streamCurGet(idl_stream(si)), "rtrim", 5) == 0) &&
               ((int)idl_streamInRel (si, 5) == (int)si->macroAttrib->openToken)) {
        c_char src [NMSIZE];
        c_char trim [NMSIZE];
        c_char res [NMSIZE];
        c_char *substr;
        c_char *p_substr;

        idl_streamInWind(si);
        idl_streamInWind(si);
        idl_streamInWind(si);
        idl_streamInWind(si);
        idl_streamInWind(si);
        idl_streamInWind(si);
        switch (idl_tmplExpGetMacroDoubleArg (tmplExp, si, src, trim)) {
        case 0:
            fprintf (stderr, "rtrim: Incomplete function 'ltrim' missing ','\n");
            result = 1;
        break;
        case 1:
            fprintf (stderr, "rtrim: Incomplete function 'ltrim' missing '%c'\n",
                si->macroAttrib->closeToken);
            result = 1;
        break;
        case 2:
            substr = strstr (src, trim);
            p_substr = substr;
            while (substr) {
                substr++;
                substr = strstr (substr, trim);
                if (substr) {
                    p_substr = substr;
                }
            }
            os_strcpy (res, src);
            if (p_substr != NULL) {
                res[p_substr-src] = '\0';
            }
            idl_tmplExpInsertText (so, res);
        break;
        default:
            printf ("Unexpected case\n");
        break;
    }
	/* QAC EXPECT 3416; No unexpected side effect here */
    } else if ((strncmp (idl_streamCurGet(idl_stream(si)), "mul", 3) == 0) &&
               ((int)idl_streamInRel (si, 3) == (int)si->macroAttrib->openToken)) {
        c_char mul1 [NMSIZE];
        c_char mul2 [NMSIZE];
        c_char res [NMSIZE];
        signed int value1;
        signed int value2;

        idl_streamInWind(si);
        idl_streamInWind(si);
        idl_streamInWind(si);
        idl_streamInWind(si);
        switch (idl_tmplExpGetMacroDoubleArg (tmplExp, si, mul1, mul2)) {
        case 0:
            fprintf (stderr, "mul: Incomplete function 'mul' missing ','\n");
            result = 1;
        break;
        case 1:
            fprintf (stderr, "mul: Incomplete function 'mul' missing '%c'\n",
                si->macroAttrib->closeToken);
            result = 1;
        break;
        case 2:
            sscanf (mul1, "%d", &value1);
            sscanf (mul2, "%d", &value2);
            snprintf (res, (size_t)sizeof(res), "%d", value1 * value2);
            idl_tmplExpInsertText (so, res);
        break;
        default:
            printf ("Unexpected case\n");
        break;
    }
	/* QAC EXPECT 3416; No unexpected side effect here */
    } else if ((strncmp (idl_streamCurGet(idl_stream(si)), "div", 3) == 0) &&
               ((int)idl_streamInRel (si, 3) == (int)si->macroAttrib->openToken)) {
        c_char div1[NMSIZE];
        c_char div2[NMSIZE];
        c_char res[NMSIZE];
        signed int value1;
        signed int value2;

        idl_streamInWind(si);
        idl_streamInWind(si);
        idl_streamInWind(si);
        idl_streamInWind(si);
        switch (idl_tmplExpGetMacroDoubleArg (tmplExp, si, div1, div2)) {
        case 0:
            fprintf (stderr, "div: Incomplete function 'div' missing ','\n");
            result = 1;
        break;
        case 1:
            fprintf (stderr, "div: Incomplete function 'div' missing '%c'\n",
                si->macroAttrib->closeToken);
            result = 1;
        break;
        case 2:
            sscanf (div1, "%d", &value1);
            sscanf (div2, "%d", &value2);
            if (value2 != 0) {
                snprintf (res, (size_t)sizeof(res), "%d", value1 / value2);
                idl_tmplExpInsertText (so, res);
            } else {
                fprintf (stderr, "div: Divide by zero exception\n");
            }
        break;
        default:
            printf ("Unexpected case\n");
            break;
        }
	/* QAC EXPECT 3416; No unexpected side effect here */
    } else if ((strncmp (idl_streamCurGet(idl_stream(si)), "add", 3) == 0) &&
               ((int)idl_streamInRel (si, 3) == (int)si->macroAttrib->openToken)) {
        c_char add1[NMSIZE];
        c_char add2[NMSIZE];
        c_char res[NMSIZE];
        signed int value1;
        signed int value2;

        idl_streamInWind(si);
        idl_streamInWind(si);
        idl_streamInWind(si);
        idl_streamInWind(si);
        switch (idl_tmplExpGetMacroDoubleArg (tmplExp, si, add1, add2)) {
        case 0:
            fprintf (stderr, "add: Incomplete function 'add' missing ','\n");
            result = 1;
        break;
        case 1:
            fprintf (stderr, "add: Incomplete function 'add' missing '%c'\n",
                si->macroAttrib->closeToken);
            result = 1;
        break;
        case 2:
            sscanf (add1, "%d", &value1);
            sscanf (add2, "%d", &value2);
            snprintf (res, (size_t)sizeof(res), "%d", value1 + value2);
            idl_tmplExpInsertText (so, res);
        break;
        default:
            printf ("Unexpected case\n");
        break;
    }
	/* QAC EXPECT 3416; No unexpected side effect here */
    } else if ((strncmp (idl_streamCurGet(idl_stream(si)), "sub", 3) == 0) &&
               ((int)idl_streamInRel (si, 3) == (int)si->macroAttrib->openToken)) {
        c_char sub1[NMSIZE];
        c_char sub2[NMSIZE];
        c_char res[NMSIZE];
        signed int value1;
        signed int value2;

        idl_streamInWind(si);
        idl_streamInWind(si);
        idl_streamInWind(si);
        idl_streamInWind(si);
        switch (idl_tmplExpGetMacroDoubleArg (tmplExp, si, sub1, sub2)) {
        case 0:
            fprintf (stderr, "sub: Incomplete function 'sub' missing ','\n");
            result = 1;
        break;
        case 1:
            fprintf (stderr, "sub: Incomplete function 'sub' missing '%c'\n",
                si->macroAttrib->closeToken);
            result = 1;
        break;
        case 2:
            sscanf (sub1, "%d", &value1);
            sscanf (sub2, "%d", &value2);
            snprintf (res, (size_t)sizeof(res), "%d", value1 - value2);
            idl_tmplExpInsertText (so, res);
        break;
        default:
            printf ("Unexpected case\n");
        break;
    }
	/* QAC EXPECT 3416; No unexpected side effect here */
    } else if ((strncmp (idl_streamCurGet(idl_stream(si)), "inc", 3) == 0) &&
               ((int)idl_streamInRel (si, 3) == (int)si->macroAttrib->openToken)) {
        c_char macro_name[NMSIZE];
        c_char res[NMSIZE];
        signed int val;

        idl_streamInWind(si);
        idl_streamInWind(si);
        idl_streamInWind(si);
        idl_streamInWind(si);
	/* QAC EXPECT 3416; No unexpected side effect here */
        if (idl_tmplExpGetMacroSingleArg (tmplExp, si, macro_name)) {
            sscanf (idl_tmplExpMacroValue(tmplExp, macro_name), "%d", &val);
            snprintf (res, (size_t)sizeof(res), "%d", val+1);
            idl_tmplExpInsertText (so, res);
            macro = idl_macroNew (macro_name, res);
            idl_macroSetAdd (tmplExp->macroSet, macro);
        } else {
            fprintf (stderr, "inc: Incomplete function 'inc' missing '%c'\n",
                si->macroAttrib->closeToken);
            result = 1;
        }
/* QAC EXPECT 3416; No unexpected side effect here */
    } else if ((strncmp (idl_streamCurGet(idl_stream(si)), "dec", 3)) == 0 &&
               ((int)idl_streamInRel (si, 3) == (int)si->macroAttrib->openToken)) {
        c_char macro_name [NMSIZE];
        c_char res [NMSIZE];
        signed int val;

        idl_streamInWind(si);
        idl_streamInWind(si);
        idl_streamInWind(si);
        idl_streamInWind(si);
	/* QAC EXPECT 3416; No unexpected side effect here */
        if (idl_tmplExpGetMacroSingleArg (tmplExp, si, macro_name)) {
            sscanf (idl_tmplExpMacroValue(tmplExp, macro_name), "%d", &val);
            snprintf (res, (size_t)sizeof(res), "%d", val-1);
            idl_tmplExpInsertText (so, res);
            macro = idl_macroNew (macro_name, res);
            idl_macroSetAdd (tmplExp->macroSet, macro);
        } else {
            fprintf (stderr, "dec: Incomplete function 'dec' missing '%c'\n",
                si->macroAttrib->closeToken);
            result = 1;
        }
	/* QAC EXPECT 3416; No unexpected side effect here */
    } else if ((strncmp (idl_streamCurGet(idl_stream(si)), "hex", 3) == 0) &&
               ((int)idl_streamInRel (si, 3) == (int)si->macroAttrib->openToken)) {
        c_char arg [NMSIZE];
        c_char res [NMSIZE];
        signed int val;

        idl_streamInWind(si);
        idl_streamInWind(si);
        idl_streamInWind(si);
        idl_streamInWind(si);
/* QAC EXPECT 3416; No unexpected side effect here */
        if (idl_tmplExpGetMacroSingleArg (tmplExp, si, arg)) {
            sscanf (arg, "%d", &val);
            snprintf (res, (size_t)sizeof(res), "%x", val);
            idl_tmplExpInsertText (so, res);
        } else {
            fprintf (stderr, "hex: Incomplete function 'hex' missing '%c'\n",
                si->macroAttrib->closeToken);
            result = 1;
        }
/* QAC EXPECT 3416; No unexpected side effect here */
    } else if ((strncmp (idl_streamCurGet(idl_stream(si)), "oct", 3) == 0) &&
               ((int)idl_streamInRel (si, 3) == (int)si->macroAttrib->openToken)) {
        c_char arg [NMSIZE];
        c_char res [NMSIZE];
        signed int val;

        idl_streamInWind(si);
        idl_streamInWind(si);
        idl_streamInWind(si);
        idl_streamInWind(si);
	/* QAC EXPECT 3416; No unexpected side effect here */
        if (idl_tmplExpGetMacroSingleArg (tmplExp, si, arg)) {
            sscanf (arg, "%d", &val);
            snprintf (res, (size_t)sizeof(res), "%o", val);
            idl_tmplExpInsertText (so, res);
        } else {
            fprintf (stderr, "oct: Incomplete function 'oct' missing '%c'\n",
                si->macroAttrib->closeToken);
            result = 1;
        }
    } else {
        idl_streamInWind(si);
        fprintf (stderr, "Unknown macro\n");
    }
    return result;
    /* QAC EXPECT 5101, 5103; real complexity is limited per case, number of lines per case is limited */
}

void
idl_tmplExpProcessTmpl (
    const idl_tmplExp tmplExp,
    const idl_streamIn si,
    const idl_fileOut fo)
{
    idl_streamOut so;
    int processResult = 0;

    /* QAC EXPECT 3416; No unexpected side effects here */
    while (((int)idl_streamInCur(si) != (int)'\0') && (processResult == 0)) {
	/* QAC EXPECT 3416; No unexpected side effects here */
        if ((int)idl_streamInCur(si) == (int)si->macroAttrib->startToken) {
            so = idl_streamOutNew (0);
            processResult = idl_tmplExpProcessMacro (tmplExp, si, so);
            idl_fileOutPrintf (fo, idl_streamGet (idl_stream(so)));
            idl_streamOutFree (so);
        } else {
            idl_fileOutPut(fo, idl_streamInCur(si));
            idl_streamInWind(si);
        }
    }
}

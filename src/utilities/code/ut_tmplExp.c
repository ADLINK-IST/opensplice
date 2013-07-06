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
#include "os_iterator.h"
#include "os_heap.h"
#include "os_stdlib.h"
#include "ut_tmplExp.h"

#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#define NMSIZE		(100)

/****************************************************************
 * Stream Exp Macro Attributes Class Implementation             *
 ****************************************************************/
OS_STRUCT(ut_macroAttrib) {
    os_char	startToken;
    os_char	openToken;
    os_char	closeToken;
};

ut_macroAttrib
ut_macroAttribNew(
    os_char startToken,
    os_char openToken,
    os_char closeToken)
{    
    ut_macroAttrib macroAttrib = os_malloc((size_t)OS_SIZEOF(ut_macroAttrib));

    macroAttrib->startToken = startToken;
    macroAttrib->openToken = openToken;
    macroAttrib->closeToken = closeToken;

    return macroAttrib;
}

void
ut_macroAttribFree(
    const ut_macroAttrib macroAttrib)
{    
    os_free(macroAttrib);
}

/****************************************************************
 * Stream Exp Macro Class Implementation                        *
 ****************************************************************/
OS_STRUCT(ut_macro) {
    os_char *name;
    os_char *value;
};

ut_macro
ut_macroNew (
    const os_char *name,
    const os_char *value)
{    
    ut_macro macro = os_malloc((size_t)OS_SIZEOF(ut_macro));

    macro->name = os_strdup(name);
    macro->value = os_strdup(value);
    return macro;
}

void
ut_macroFree (
    const ut_macro macro)
{   
    os_free(macro->name);   
    os_free(macro->value);
    os_free(macro);
}

os_char *
ut_macroName(
    const ut_macro macro)
{
    return macro->name;
}

os_char *
ut_macroValue (
    const ut_macro macro)
{
    return macro->value;
}

/****************************************************************
 * Stream Exp Macro Set Class Implementation                    *
 ****************************************************************/
OS_STRUCT(ut_macroSet) {
    os_iter macroSet;
};

static os_equality
ut_macroNameMatch(
    const ut_macro macro,
    const os_char *name)
{
    os_equality result = OS_NE;

    if (strcmp(macro->name, name) == 0) {
        result = OS_EQ;
    }
    return result;
}

ut_macroSet
ut_macroSetNew(
    void)
{
    ut_macroSet macroSet = os_malloc((size_t)OS_SIZEOF(ut_macroSet));

    macroSet->macroSet = os_iterNew(NULL);
    return macroSet;
}

void
ut_macroSetFree (
    const ut_macroSet macroSet)
{
    ut_macroSetClear(macroSet);
    os_iterFree(macroSet->macroSet);
    os_free(macroSet);
}

void
ut_macroSetAdd(
    const ut_macroSet macroSet,
    const ut_macro macro)
{
    ut_macro old_macro;

    old_macro = os_iterResolve(macroSet->macroSet, ut_macroNameMatch, macro->name);
    if (old_macro) {
        os_iterTake(macroSet->macroSet, old_macro);
    }
    os_iterInsert(macroSet->macroSet, macro);
}

void
ut_macroSetRemove(
    const ut_macroSet macroSet,
    const ut_macro macro)
{
    os_iterTake(macroSet->macroSet, macro);
}

void
ut_macroSetClear(
    const ut_macroSet macroSet)
{
    ut_macro macro;

    macro = os_iterTakeFirst(macroSet->macroSet);
    while (macro != NULL) {
        ut_macroFree(macro);
        macro = os_iterTakeFirst(macroSet->macroSet);
    }
}

ut_macro
ut_macroSetGet(
    const ut_macroSet macroSet,
    const os_char *name)
{
    ut_macro macro;

    macro = os_iterResolve(macroSet->macroSet, ut_macroNameMatch, (void *)name);
    return macro;
}

/****************************************************************
 * Stream Exp Generic Stream Class Implementation               *
 ****************************************************************/
OS_STRUCT(ut_stream) {
    os_char  *stream;
    os_uint32 length;
    os_uint32 curpos;
};

ut_stream
ut_streamInit(
    const ut_stream stream,
    const os_char *stream_val)
{
    stream->stream = os_strdup(stream_val);
    stream->length = strlen(stream_val);
    stream->curpos = 0;
    return stream;
}

void
ut_streamExit(
    const ut_stream stream)
{
    os_free(stream->stream);
}

os_char *
ut_streamGet(
    const ut_stream stream)
{
    return stream->stream;
}

os_char *
ut_streamCurGet(
    const ut_stream stream)
{
    return &stream->stream[stream->curpos];
}

os_int32
ut_streamLength(
    const ut_stream stream)
{
    return stream->length;
}

/****************************************************************
 * Stream Exp Input Stream Class Implementation                 *
 ****************************************************************/
OS_STRUCT(ut_streamIn) {
    OS_EXTENDS(ut_stream);
    ut_macroAttrib macroAttrib;
    ut_macroSet macroSet;
};

ut_streamIn
ut_streamInNew (
    const os_char *stream_val,
    const ut_macroAttrib macroAttrib)
{
    ut_streamIn	stream;

    assert(stream_val);
    assert(macroAttrib);
    stream = os_malloc((size_t)OS_SIZEOF(ut_streamIn));
    ut_streamInit(ut_stream(stream), stream_val);
    stream->macroAttrib = macroAttrib;
    return stream;
}

void
ut_streamInFree(
    const ut_streamIn stream)
{
    ut_streamExit(ut_stream(stream));
    os_free(stream);
}

os_char
ut_streamInCur(
    const ut_streamIn stream)
{
    ut_stream str = ut_stream(stream);

    return str->stream[str->curpos];
}

os_char
ut_streamInRel(
    const ut_streamIn stream,
    os_uint32 offset)
{
    os_uint32 abspos;
    ut_stream str = ut_stream(stream);

    abspos = str->curpos + offset;
    if (abspos >= str->length) {
        abspos = str->length -1;
    }
    return str->stream[abspos];
}

void
ut_streamInWind(
    const ut_streamIn stream)
{
    ut_stream str = ut_stream(stream);

    if (str->curpos < str->length) {
        str->curpos++;
    }
}

os_char
ut_streamInWindCur(
    const ut_streamIn stream)
{
    ut_stream str = ut_stream(stream);

    if (str->curpos < str->length) {
        str->curpos++;
    }
    return str->stream[str->curpos];
}

/****************************************************************
 * Stream Expander Output Stream Class Implementation           *
 ****************************************************************/
OS_STRUCT(ut_streamOut) {
    OS_EXTENDS(ut_stream);
    os_uint32 max_length;
};

ut_streamOut
ut_streamOutNew (
    os_uint32 max_length)
{
    ut_streamOut stream;

    stream = os_malloc((size_t)OS_SIZEOF(ut_streamOut));
    ut_streamInit(ut_stream(stream), "");
    stream->max_length = max_length;
    return stream;
}

void
ut_streamOutFree(
    const ut_streamOut stream)
{
    ut_streamExit(ut_stream(stream));
    os_free(stream);
}

os_uint32
ut_streamOutPut(
    const ut_streamOut stream,
    os_char character)
{
    ut_stream str = ut_stream(stream);

    if (stream->max_length == 0) {
        if (((str->curpos) % 100) == 0) {
            str->stream = os_realloc(str->stream, (size_t)(str->curpos + 101));
	    }
        str->stream[str->curpos] = character;
        str->curpos++;
        str->stream[str->curpos] = '\0';
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

os_char *
ut_streamOutGetAndClear(
    const ut_streamOut stream)
{
    ut_stream str = ut_stream(stream);
    os_char *result;

    result = str->stream;
    ut_streamInit(ut_stream(stream), "");

    return result;
}

/****************************************************************
 * File Output Stream Class Implementation                      *
 ****************************************************************/
OS_STRUCT(ut_fileOut) {
    FILE *file;
};

static os_char *ut_outputdir = NULL;

os_char *
ut_dirOutCur(void)
{
    return ut_outputdir;
}

void
ut_dirOutFree(void)
{
    if (ut_outputdir) {
        os_free(ut_outputdir);
    }
    return;
}

os_int32
ut_dirOutNew(
    const os_char *name)
{
    os_int32 result;
    os_result status;
    char dirName[OS_PATH_MAX];
    struct os_stat statBuf;
    os_uint32 i;

    memset(dirName, 0, OS_PATH_MAX);

    if (name) {
        result = 1;

        for (i = 0; i < strlen(name) && result; i++) {
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
                        printf("'%s' is not a directory\n", dirName);
                        result = 0;
                        ut_outputdir = NULL;
                    }
#else
                    printf("'%s' is not a directory\n", dirName);
                    result = 0;
                    ut_outputdir = NULL;
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
                ut_outputdir = os_strdup(name);

                if (!OS_ISDIR(statBuf.stat_mode)) {
#ifdef WIN32
                    if ((strlen(dirName) == 2) && (dirName[1] == ':')) {
                        /*This is a device like for instance: 'C:'. Check if it exists...*/
                        dirName[2] = OS_FILESEPCHAR;
                        status = os_stat(dirName, &statBuf);

                        if (status == os_resultFail) {
                            printf("'%s' is not available", dirName);
                            result = 0;
                            ut_outputdir = NULL;
                        }
                    } else {
                        printf("'%s' is not a directory.\n", ut_outputdir);
                        result = 0;
                        ut_outputdir = NULL;
                    }
#else
                    printf("'%s' is not a directory\n", dirName);
                    result = 0;
                    ut_outputdir = NULL;
#endif
                }
            } else {
                ut_outputdir = (os_char *)os_malloc(strlen(name)+1);
                snprintf(ut_outputdir, strlen(name), "%s", name);
            }
        }
    } else {
        result = 0;
        ut_outputdir = NULL;
    }

    if (result) {
        status = os_access(ut_outputdir, 2); /* Check whether dir is writable */

        if (status != os_resultSuccess) {
#ifdef WIN32
            if ((strlen(dirName) == 2) && (dirName[1] == ':')) {
                /*This is a device like for instance: 'C:'. Check if it exists...*/
                dirName[2] = OS_FILESEPCHAR;
                status = os_stat(dirName, &statBuf);

                if (status == os_resultFail) {
                    printf("'%s' cannot be found", dirName);
                    result = 0;
                    ut_outputdir = NULL;
                }
            } else {
                printf("Specified output directory '%s' is not writable.\n", ut_outputdir);
                result = 0;
                ut_outputdir = NULL;
            }
#else
            printf("Specified output directory '%s' is not writable.\n", ut_outputdir);
            result = 0;
            ut_outputdir = NULL;
#endif
        }
    }

    return result;
}

ut_fileOut
ut_fileOutNew(
    const os_char *name,
    const os_char *mode)
{
    ut_fileOut stream;
    os_char *fname;
    os_char * filename;
    
    stream = os_malloc((size_t)OS_SIZEOF(ut_fileOut));
    if (ut_outputdir) {
        fname = os_malloc(strlen(ut_outputdir) + strlen(os_fileSep()) + strlen(name) + 1);
        os_sprintf(fname, "%s%s%s", ut_outputdir, os_fileSep(), name);
    } else {
        fname = os_strdup(name);
    }
    filename = os_fileNormalize(fname); 
    stream->file = fopen(filename, mode);
    os_free(fname);
    os_free(filename);

    if (stream->file == NULL) {
        os_free(stream);
        stream = NULL;
    }
    return stream;
}

void
ut_fileOutFree(
    const ut_fileOut stream)
{
    fclose(stream->file);
    os_free(stream);
}

os_int32
ut_fileOutPut(
    const ut_fileOut stream,
    os_char character)
{
    os_int32 result;

    result = putc(character, stream->file);
    return result;
}

void
ut_fileOutPrintf(
    const ut_fileOut stream,
    const os_char *format,
    ...)
{
    va_list args;

    va_start(args, format);
    vfprintf(stream->file, format, args);
    va_end(args);
}

static ut_fileOut ut_fileCurrentOut = NULL;

void
ut_fileSetCur(
    const ut_fileOut fileOut)
{
    ut_fileCurrentOut = fileOut;
}

ut_fileOut
ut_fileCur(void)
{
    return ut_fileCurrentOut;
}

/****************************************************************
 * Stream Exp Tmpl Exp Class Implementation                     *
 ****************************************************************/
OS_STRUCT(ut_tmplExp) {
    ut_macroSet macroSet;
};

static void
ut_tmplExpInsertMacro (
    const ut_tmplExp tmplExp,
    const os_char *name,
    const ut_streamOut so,
    const ut_macroAttrib macroAttrib);

static const os_char *
ut_tmplExpMacroValue (
    const ut_tmplExp tmplExp,
    const os_char *name);

static void
ut_tmplExpDeleteMacro (
    const ut_tmplExp tmplExp,
    const os_char *name);

static void
ut_tmplExpInsertText (
    const ut_streamOut so,
    const os_char *text);

static os_int32
ut_tmplExpGetMacroSingleArg (
    const ut_tmplExp tmplExp,
    const ut_streamIn si,
    os_char *arg);

static os_int32
ut_tmplExpGetMacroDoubleArg (
    const ut_tmplExp tmplExp,
    const ut_streamIn si,
    os_char *arg1,
    const os_char *arg2);

static os_int32
ut_tmplExpProcessMacro (
    const ut_tmplExp tmplExp,
    const ut_streamIn si,
    const ut_streamOut so);

ut_tmplExp
ut_tmplExpNew (
    const ut_macroSet macroSet)
{
    ut_tmplExp tmplExp = os_malloc((size_t)OS_SIZEOF(ut_tmplExp));

    tmplExp->macroSet = macroSet;
    return tmplExp;
}

void
ut_tmplExpFree(
    const ut_tmplExp tmplExp)
{
    os_free(tmplExp);
}

static void
ut_tmplExpInsertMacro(
    const ut_tmplExp tmplExp,
    const os_char *name,
    const ut_streamOut so,
    const ut_macroAttrib macroAttrib)
{
    int inserted = 0;
    ut_streamIn macroStream;
    ut_macro macro;

    macro = ut_macroSetGet(tmplExp->macroSet, name);
    if (macro) {
        macroStream = ut_streamInNew(ut_macroValue(macro), macroAttrib);

    	while (ut_streamInCur(macroStream) != '\0') {
            if (ut_streamInCur(macroStream) == macroAttrib->startToken) {
                ut_tmplExpProcessMacro(tmplExp, macroStream, so);
            } else {
                ut_streamOutPut(so, ut_streamInCur(macroStream));
                ut_streamInWind(macroStream);
            }
        }
        ut_streamOutPut(so, '\0');
        ut_streamInFree(macroStream);
        inserted = 1;
    }
    if (!inserted) {
        const os_char *value;
        value = os_getenv((os_char *)name);
        if (value) {
            macroStream = ut_streamInNew(value, macroAttrib);

    	    while (ut_streamInCur(macroStream) != '\0') {
                if (ut_streamInCur(macroStream) == macroAttrib->startToken) {
                    ut_tmplExpProcessMacro(tmplExp, macroStream, so);
                } else {
                    ut_streamOutPut(so, ut_streamInCur(macroStream));
                    ut_streamInWind(macroStream);
                }
            }
            ut_streamOutPut(so, '\0');
            inserted = 1;
        }
    }
    if (!inserted) {
        fprintf(stderr, "insert_macro: Undefined macro '%s'\n", name);
    }
}

static const os_char *
ut_tmplExpMacroValue (
    const ut_tmplExp tmplExp,
    const os_char *name)
{
    ut_macro macro;
    const char *value;

    macro = ut_macroSetGet(tmplExp->macroSet, name);
    if (macro) {
        return ut_macroValue(macro);
    }
    value = os_getenv((os_char*)name);
    if (value) {
        return value;
    }
    fprintf(stderr, "macro_value: Undefined macro '%s'\n", name);
    return "";
}

static void
ut_tmplExpDeleteMacro (
    const ut_tmplExp tmplExp,
    const os_char *name)
{
    int deleted = 0;
    ut_macro macro;

    macro = ut_macroSetGet(tmplExp->macroSet, name);
    if (macro) {
        ut_macroSetRemove(tmplExp->macroSet, macro);
        deleted = 1;
    }
    if (!deleted) {
        fprintf(stderr, "delete_macro: Undefined macro '%s'\n", name);
    }
}

static void
ut_tmplExpInsertText (
    const ut_streamOut so,
    const os_char *text)
{
    os_uint32 y;

    for (y = 0; y < strlen(text); y++) {
        ut_streamOutPut(so, text[y]);
    }
}

static os_int32
ut_tmplExpGetMacroSingleArg (
    const ut_tmplExp tmplExp,
    const ut_streamIn si,
    os_char *arg)
{
    ut_streamOut argStream;

    argStream = ut_streamOutNew(0);

    while ((ut_streamInCur(si) != '\n') && (ut_streamInCur(si) != si->macroAttrib->closeToken)) {
        if (ut_streamInCur(si) == si->macroAttrib->startToken) {
            ut_tmplExpProcessMacro(tmplExp, si, argStream);
        } else {
            ut_streamOutPut(argStream, ut_streamInCur(si));
            ut_streamInWind(si);
        }
    }
    ut_streamOutPut(argStream, '\0');
    os_strcpy(arg, ut_streamGet(ut_stream(argStream)));
    ut_streamOutFree(argStream);

    if (ut_streamInCur(si) == si->macroAttrib->closeToken) {
        ut_streamInWind(si);
        return 1;
    }
    return 0;
}

static os_int32
ut_tmplExpGetMacroDoubleArg (
    const ut_tmplExp tmplExp,
    const ut_streamIn si,
    os_char *arg1,
    const os_char *arg2)
{
    ut_streamOut arg1Stream;
    ut_streamOut arg2Stream;
    int return_val;

    arg1Stream = ut_streamOutNew(0);

    while ((ut_streamInCur(si) != '\n') && (ut_streamInCur(si) != ',')) {
        if (ut_streamInCur(si) == si->macroAttrib->startToken) {
    	    ut_tmplExpProcessMacro(tmplExp, si, arg1Stream);
        } else {
            ut_streamOutPut(arg1Stream, ut_streamInCur(si));
    	    ut_streamInWind(si);
        }
    }
    ut_streamOutPut(arg1Stream, '\0');
    os_strcpy(arg1, ut_streamGet(ut_stream(arg1Stream)));
    ut_streamOutFree(arg1Stream);

    if (ut_streamInCur(si) == ',') {
        arg2Stream = ut_streamOutNew(0);
        ut_streamInWind(si);

        while ((ut_streamInCur(si) != '\n') && (ut_streamInCur(si) != si->macroAttrib->closeToken)) {
            if (ut_streamInCur(si) == si->macroAttrib->startToken) {
    	        ut_tmplExpProcessMacro(tmplExp, si, arg2Stream);
            } else {
                ut_streamOutPut(arg2Stream, ut_streamInCur(si));
    	        ut_streamInWind(si);
    	    }
        }

        if (ut_streamInCur(si) == si->macroAttrib->closeToken) {
    	    ut_streamInWind(si);
            ut_streamOutPut(arg2Stream, '\0');
            return_val = 2;
        } else {
        	return_val = 1;
        }
	    ut_streamOutFree(arg2Stream);
	    return return_val;
    }
    return_val = 0;
    return return_val;
}

static os_int32
ut_tmplExpProcessMacro (
    const ut_tmplExp tmplExp,
    const ut_streamIn si,
    const ut_streamOut so)
{
    ut_macro macro;
    os_int32 result = 0;

    ut_streamInWind(si); /* skip macro start character */


    if ((strncmp(ut_streamCurGet(ut_stream(si)), "def", 3) == 0) &&
        (ut_streamInRel(si, 3) == si->macroAttrib->openToken)) {
        os_int32 br_count = 1;
        ut_streamOut defName;
        ut_streamOut defValue;

        ut_streamInWind(si);
        ut_streamInWind(si);
        ut_streamInWind(si);
        ut_streamInWind(si);
        defName = ut_streamOutNew(0);
        defValue = ut_streamOutNew(0);

        while ((ut_streamInCur(si) != '\n') && (ut_streamInCur(si) != '=')) {
            if (ut_streamInCur(si) == si->macroAttrib->startToken) {
                result = ut_tmplExpProcessMacro(tmplExp, si, defName);
            } else {
                ut_streamOutPut(defName, ut_streamInCur(si));
                ut_streamInWind(si);
            }
        }
        ut_streamOutPut(defName, '\0');
        if (ut_streamInCur(si) == '=') {
        	ut_streamInWind(si);

            while ((ut_streamInCur(si) != '\n') &&
                   (ut_streamInCur(si) != si->macroAttrib->closeToken || br_count)) {
                ut_streamOutPut(defValue, ut_streamInCur(si));
                ut_streamInWind(si);

                if (ut_streamInCur(si) == si->macroAttrib->openToken) {
                    br_count++;
                } else {
                    if (ut_streamInCur(si) == si->macroAttrib->closeToken) {
                        br_count--;
                    }
                }
            }

            if (ut_streamInCur(si) == si->macroAttrib->closeToken) {
                ut_streamInWind(si);
                ut_streamOutPut(defValue, '\0');
                macro = ut_macroNew(ut_streamGet (ut_stream(defName)),
                ut_streamGet(ut_stream(defValue)));
                ut_macroSetAdd(tmplExp->macroSet, macro);
            } else {
                fprintf(stderr, "def: Incomplete definition '%s' missing '%c'\n",
                    ut_streamGet(ut_stream(defName)), si->macroAttrib->closeToken);
                result = 1;
            }
        } else {
            fprintf(stderr, "def: Incomplete definition '%s' missing '='\n",
            ut_streamGet(ut_stream(defName)));
            result = 1;
        }
        ut_streamOutFree(defName);
        ut_streamOutFree(defValue);
    } else if ((strncmp(ut_streamCurGet(ut_stream(si)), "undef", 5) == 0) &&
               (ut_streamInRel(si, 5) == si->macroAttrib->openToken)) {
         os_char macro_name[NMSIZE];

         ut_streamInWind(si);
         ut_streamInWind(si);
         ut_streamInWind(si);
         ut_streamInWind(si);
         ut_streamInWind(si);
         ut_streamInWind(si);

         if (ut_tmplExpGetMacroSingleArg(tmplExp, si, macro_name)) {
             ut_tmplExpDeleteMacro(tmplExp, macro_name);
         } else {
             fprintf(stderr, "undef: Incomplete function 'undef' missing '%c'\n",
                 si->macroAttrib->closeToken);
                 result = 1;
         }
    } else if (ut_streamInCur(si) == si->macroAttrib->openToken) {
         os_char macro_name[NMSIZE];

         ut_streamInWind(si);

         if (ut_tmplExpGetMacroSingleArg(tmplExp, si, macro_name)) {
             ut_tmplExpInsertMacro(tmplExp, macro_name, so, si->macroAttrib);
         } else {
             fprintf(stderr, "%c%c: Incomplete function '%c' missing '%c'\n",
                 si->macroAttrib->startToken, si->macroAttrib->openToken,
                 si->macroAttrib->startToken, si->macroAttrib->closeToken);
                 result = 1;
         }
    } else if ((strncmp(ut_streamCurGet(ut_stream(si)), "sp", 2) == 0) &&
               (ut_streamInRel(si, 2) == si->macroAttrib->openToken)) {
        os_char spaceCount[NMSIZE];
        unsigned int ni = 0;
        unsigned int spc = 0;

        ut_streamInWind(si);
        ut_streamInWind(si);
        ut_streamInWind(si);

        if (ut_tmplExpGetMacroSingleArg(tmplExp, si, spaceCount)) {
            sscanf(spaceCount, "%d", (int*)&spc);
            for (ni = 0; ni < spc; ni++) {
                ut_streamOutPut(so, ' ');
            }
        } else {
            fprintf(stderr, "upper: Incomplete function 'sp' missing ')'\n");
            result = 1;
        }

    } else if ((strncmp(ut_streamCurGet(ut_stream(si)), "upper", 5) == 0) &&
               (ut_streamInRel(si, 5) == si->macroAttrib->openToken)) {
        os_char string[NMSIZE];
        unsigned int ni = 0;

        ut_streamInWind(si);
        ut_streamInWind(si);
        ut_streamInWind(si);
        ut_streamInWind(si);
        ut_streamInWind(si);
        ut_streamInWind(si);

        if (ut_tmplExpGetMacroSingleArg(tmplExp, si, string)) {
            for (ni = 0; ni < strlen(string); ni++) {
                string[ni] = toupper(string[ni]);
            }
            ut_tmplExpInsertText(so, string);
        } else {
            fprintf(stderr, "upper: Incomplete function 'upper' missing ')'\n");
            result = 1;
        }
    } else if ((strncmp(ut_streamCurGet(ut_stream(si)), "lower", 5) == 0) &&
               (ut_streamInRel(si, 5) == si->macroAttrib->openToken)) {
        os_char string[NMSIZE];
        unsigned int ni = 0;

        ut_streamInWind(si);
        ut_streamInWind(si);
        ut_streamInWind(si);
        ut_streamInWind(si);
        ut_streamInWind(si);
        ut_streamInWind(si);
        ut_streamInWind(si);

        if (ut_tmplExpGetMacroSingleArg(tmplExp, si, string)) {
            for (ni = 0; ni < strlen(string); ni++) {
                string[ni] = (os_char)tolower(string[ni]);
            }
            ut_tmplExpInsertText(so, string);
        } else {
            fprintf(stderr, "lower: Incomplete function 'lower' missing '%c'\n",
                si->macroAttrib->closeToken);
            result = 1;
        }
    } else if ((strncmp(ut_streamCurGet(ut_stream(si)), "ltrim", 5) == 0) &&
               (ut_streamInRel(si, 5) == si->macroAttrib->openToken)) {
        os_char src[NMSIZE];
        os_char trim[NMSIZE];
        os_char res[NMSIZE];
        os_char *substr;

        ut_streamInWind(si);
        ut_streamInWind(si);
        ut_streamInWind(si);
        ut_streamInWind(si);
        ut_streamInWind(si);
        ut_streamInWind(si);
        switch (ut_tmplExpGetMacroDoubleArg(tmplExp, si, src, trim)) {
        case 0:
            fprintf(stderr, "ltrim: Incomplete function 'ltrim' missing ','\n");
            result = 1;
        break;
        case 1:
            fprintf(stderr, "ltrim: Incomplete function 'ltrim' missing '%c'\n",
                si->macroAttrib->closeToken);
            result = 1;
        break;
        case 2:
            substr = strstr(src, trim);
            if (substr == NULL) {
                os_strcpy(res, src);
            } else {
                substr += strlen(trim);
                os_strcpy(res, substr);
            }
            ut_tmplExpInsertText(so, res);
        break;
        default:
            printf("Unexpected case\n");
        break;
        }
    } else if ((strncmp(ut_streamCurGet(ut_stream(si)), "rtrim", 5) == 0) &&
               (ut_streamInRel(si, 5) == si->macroAttrib->openToken)) {
        os_char src[NMSIZE];
        os_char trim[NMSIZE];
        os_char res[NMSIZE];
        os_char *substr;
        os_char *p_substr;

        ut_streamInWind(si);
        ut_streamInWind(si);
        ut_streamInWind(si);
        ut_streamInWind(si);
        ut_streamInWind(si);
        ut_streamInWind(si);
        switch (ut_tmplExpGetMacroDoubleArg(tmplExp, si, src, trim)) {
        case 0:
            fprintf(stderr, "rtrim: Incomplete function 'ltrim' missing ','\n");
            result = 1;
        break;
        case 1:
            fprintf(stderr, "rtrim: Incomplete function 'ltrim' missing '%c'\n",
                si->macroAttrib->closeToken);
            result = 1;
        break;
        case 2:
            substr = strstr(src, trim);
            p_substr = substr;
            while (substr) {
                substr++;
                substr = strstr(substr, trim);
                if (substr) {
                    p_substr = substr;
                }
            }
            os_strcpy(res, src);
            if (p_substr != NULL) {
                res[p_substr-src] = '\0';
            }
            ut_tmplExpInsertText(so, res);
        break;
        default:
            printf("Unexpected case\n");
        break;
        }
    } else if ((strncmp(ut_streamCurGet(ut_stream(si)), "mul", 3) == 0) &&
               (ut_streamInRel(si, 3) == si->macroAttrib->openToken)) {
        os_char mul1[NMSIZE];
        os_char mul2[NMSIZE];
        os_char res[NMSIZE];
        signed int value1;
        signed int value2;

        ut_streamInWind(si);
        ut_streamInWind(si);
        ut_streamInWind(si);
        ut_streamInWind(si);
        switch (ut_tmplExpGetMacroDoubleArg(tmplExp, si, mul1, mul2)) {
        case 0:
            fprintf(stderr, "mul: Incomplete function 'mul' missing ','\n");
            result = 1;
        break;
        case 1:
            fprintf(stderr, "mul: Incomplete function 'mul' missing '%c'\n",
                si->macroAttrib->closeToken);
            result = 1;
        break;
        case 2:
            sscanf(mul1, "%d", &value1);
            sscanf(mul2, "%d", &value2);
            snprintf(res, (size_t)sizeof(res), "%d", value1 * value2);
            ut_tmplExpInsertText(so, res);
        break;
        default:
            printf("Unexpected case\n");
        break;
        }
    } else if ((strncmp(ut_streamCurGet(ut_stream(si)), "div", 3) == 0) &&
               (ut_streamInRel(si, 3) == si->macroAttrib->openToken)) {
        os_char div1[NMSIZE];
        os_char div2[NMSIZE];
        os_char res[NMSIZE];
        signed int value1;
        signed int value2;

        ut_streamInWind(si);
        ut_streamInWind(si);
        ut_streamInWind(si);
        ut_streamInWind(si);
        switch (ut_tmplExpGetMacroDoubleArg(tmplExp, si, div1, div2)) {
        case 0:
            fprintf(stderr, "div: Incomplete function 'div' missing ','\n");
            result = 1;
        break;
        case 1:
            fprintf(stderr, "div: Incomplete function 'div' missing '%c'\n",
                si->macroAttrib->closeToken);
            result = 1;
        break;
        case 2:
            sscanf(div1, "%d", &value1);
            sscanf(div2, "%d", &value2);
            if (value2 != 0) {
                snprintf(res, (size_t)sizeof(res), "%d", value1 / value2);
                ut_tmplExpInsertText(so, res);
            } else {
                fprintf(stderr, "div: Divide by zero exception\n");
            }
        break;
        default:
            printf("Unexpected case\n");
            break;
        }
    } else if ((strncmp(ut_streamCurGet(ut_stream(si)), "add", 3) == 0) &&
               (ut_streamInRel(si, 3) == si->macroAttrib->openToken)) {
        os_char add1[NMSIZE];
        os_char add2[NMSIZE];
        os_char res[NMSIZE];
        signed int value1;
        signed int value2;

        ut_streamInWind(si);
        ut_streamInWind(si);
        ut_streamInWind(si);
        ut_streamInWind(si);
        switch (ut_tmplExpGetMacroDoubleArg(tmplExp, si, add1, add2)) {
        case 0:
            fprintf(stderr, "add: Incomplete function 'add' missing ','\n");
            result = 1;
        break;
        case 1:
            fprintf(stderr, "add: Incomplete function 'add' missing '%c'\n",
                si->macroAttrib->closeToken);
            result = 1;
        break;
        case 2:
            sscanf(add1, "%d", &value1);
            sscanf(add2, "%d", &value2);
            snprintf(res, (size_t)sizeof(res), "%d", value1 + value2);
            ut_tmplExpInsertText(so, res);
        break;
        default:
            printf("Unexpected case\n");
        break;
        }
    } else if ((strncmp(ut_streamCurGet(ut_stream(si)), "sub", 3) == 0) &&
               (ut_streamInRel(si, 3) == si->macroAttrib->openToken)) {
        os_char sub1[NMSIZE];
        os_char sub2[NMSIZE];
        os_char res[NMSIZE];
        signed int value1;
        signed int value2;

        ut_streamInWind(si);
        ut_streamInWind(si);
        ut_streamInWind(si);
        ut_streamInWind(si);
        switch (ut_tmplExpGetMacroDoubleArg(tmplExp, si, sub1, sub2)) {
        case 0:
            fprintf(stderr, "sub: Incomplete function 'sub' missing ','\n");
            result = 1;
        break;
        case 1:
            fprintf(stderr, "sub: Incomplete function 'sub' missing '%c'\n",
                si->macroAttrib->closeToken);
            result = 1;
        break;
        case 2:
            sscanf(sub1, "%d", &value1);
            sscanf(sub2, "%d", &value2);
            snprintf(res, (size_t)sizeof(res), "%d", value1 - value2);
            ut_tmplExpInsertText(so, res);
        break;
        default:
            printf("Unexpected case\n");
        break;
        }
    } else if ((strncmp(ut_streamCurGet(ut_stream(si)), "inc", 3) == 0) &&
               (ut_streamInRel(si, 3) == si->macroAttrib->openToken)) {
        os_char macro_name[NMSIZE];
        os_char res[NMSIZE];
        signed int val;

        ut_streamInWind(si);
        ut_streamInWind(si);
        ut_streamInWind(si);
        ut_streamInWind(si);

        if (ut_tmplExpGetMacroSingleArg(tmplExp, si, macro_name)) {
            sscanf(ut_tmplExpMacroValue(tmplExp, macro_name), "%d", &val);
            snprintf(res, (size_t)sizeof(res), "%d", val+1);
            ut_tmplExpInsertText(so, res);
            macro = ut_macroNew(macro_name, res);
            ut_macroSetAdd(tmplExp->macroSet, macro);
        } else {
            fprintf(stderr, "inc: Incomplete function 'inc' missing '%c'\n",
                si->macroAttrib->closeToken);
            result = 1;
        }
    } else if ((strncmp(ut_streamCurGet(ut_stream(si)), "dec", 3)) == 0 &&
               (ut_streamInRel(si, 3) == si->macroAttrib->openToken)) {
        os_char macro_name[NMSIZE];
        os_char res[NMSIZE];
        signed int val;

        ut_streamInWind(si);
        ut_streamInWind(si);
        ut_streamInWind(si);
        ut_streamInWind(si);

        if (ut_tmplExpGetMacroSingleArg(tmplExp, si, macro_name)) {
            sscanf(ut_tmplExpMacroValue(tmplExp, macro_name), "%d", &val);
            snprintf(res, (size_t)sizeof(res), "%d", val-1);
            ut_tmplExpInsertText(so, res);
            macro = ut_macroNew(macro_name, res);
            ut_macroSetAdd(tmplExp->macroSet, macro);
        } else {
            fprintf(stderr, "dec: Incomplete function 'dec' missing '%c'\n",
                si->macroAttrib->closeToken);
            result = 1;
        }
    } else if ((strncmp(ut_streamCurGet(ut_stream(si)), "hex", 3) == 0) &&
               (ut_streamInRel(si, 3) == si->macroAttrib->openToken)) {
        os_char arg[NMSIZE];
        os_char res[NMSIZE];
        signed int val;

        ut_streamInWind(si);
        ut_streamInWind(si);
        ut_streamInWind(si);
        ut_streamInWind(si);

        if (ut_tmplExpGetMacroSingleArg(tmplExp, si, arg)) {
            sscanf(arg, "%d", &val);
            snprintf(res, (size_t)sizeof(res), "%x", val);
            ut_tmplExpInsertText(so, res);
        } else {
            fprintf(stderr, "hex: Incomplete function 'hex' missing '%c'\n",
                si->macroAttrib->closeToken);
            result = 1;
        }
    } else if ((strncmp(ut_streamCurGet(ut_stream(si)), "oct", 3) == 0) &&
               (ut_streamInRel(si, 3) == si->macroAttrib->openToken)) {
        os_char arg[NMSIZE];
        os_char res[NMSIZE];
        signed int val;

        ut_streamInWind(si);
        ut_streamInWind(si);
        ut_streamInWind(si);
        ut_streamInWind(si);

        if (ut_tmplExpGetMacroSingleArg(tmplExp, si, arg)) {
            sscanf(arg, "%d", &val);
            snprintf(res, (size_t)sizeof(res), "%o", val);
            ut_tmplExpInsertText(so, res);
        } else {
            fprintf(stderr, "oct: Incomplete function 'oct' missing '%c'\n",
                si->macroAttrib->closeToken);
            result = 1;
        }
    } else {
        ut_streamInWind(si);
        fprintf(stderr, "Unknown macro\n");
    }
    return result;
}

void
ut_tmplExpProcessTmpl(
    const ut_tmplExp tmplExp,
    const ut_streamIn si,
    const ut_fileOut fo)
{
    ut_streamOut so;
    int processResult = 0;

    while ((ut_streamInCur(si) != '\0') && (processResult == 0)) {
        if (ut_streamInCur(si) == si->macroAttrib->startToken) {
            so = ut_streamOutNew(0);
            processResult = ut_tmplExpProcessMacro(tmplExp, si, so);
            ut_fileOutPrintf(fo, ut_streamGet(ut_stream(so)));
            ut_streamOutFree(so);
        } else {
            ut_fileOutPut(fo, ut_streamInCur(si));
            ut_streamInWind(si);
        }
    }
}

int
ut_tmplExpProcessTmplToStream (
    const ut_tmplExp tmplExp,
    const ut_streamIn si,
    const ut_streamOut so)
{
    int processResult = 0;

    while ((ut_streamInCur(si) != '\0') && (processResult == 0)) {
        if (ut_streamInCur(si) == si->macroAttrib->startToken) {
            processResult = ut_tmplExpProcessMacro(tmplExp, si, so);
        } else {
            ut_streamOutPut(so, ut_streamInCur(si));
            ut_streamInWind(si);
        }
    }

    return processResult;
}

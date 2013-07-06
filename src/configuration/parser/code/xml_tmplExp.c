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

#include "xml_tmplExp.h"

#include "os_stdlib.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "os.h"

#define NMSIZE		(100)

/****************************************************************
 * Stream Exp Macro Attributes Class Implementation             *
 ****************************************************************/
C_STRUCT(xml_macroAttrib) {
    c_char	startToken;
    c_char	openToken;
    c_char	closeToken;
};

xml_macroAttrib
xml_macroAttribNew (
    c_char startToken,
    c_char openToken,
    c_char closeToken)
{
    xml_macroAttrib macroAttrib = os_malloc ((size_t)C_SIZEOF(xml_macroAttrib));

    macroAttrib->startToken = startToken;
    macroAttrib->openToken = openToken;
    macroAttrib->closeToken = closeToken;

    return macroAttrib;
}

void
xml_macroAttribFree (
    const xml_macroAttrib macroAttrib)
{
    os_free (macroAttrib);
}

/****************************************************************
 * Stream Exp Macro Class Implementation                        *
 ****************************************************************/
C_STRUCT(xml_macro) {
    c_char *name;
    c_char *value;
};

xml_macro
xml_macroNew (
    const c_char *name,
    const c_char *value)
{
    xml_macro macro = os_malloc((size_t)C_SIZEOF(xml_macro));

    macro->name = os_malloc(strlen(name) + 1);
    os_strcpy(macro->name, name);

    macro->value = os_malloc(strlen(value) + 1);
    os_strcpy(macro->value, value);

    return (macro);
}

void
xml_macroFree (
    const xml_macro macro)
{
    os_free (macro->name);
    os_free (macro->value);
    os_free (macro);
}

c_char
*xml_macroName (
    const xml_macro macro)
{
    return macro->name;
}

c_char
*xml_macroValue (
    const xml_macro macro)
{
    return macro->value;
}

/****************************************************************
 * Stream Exp Macro Set Class Implementation                    *
 ****************************************************************/
C_STRUCT(xml_macroSet) {
    c_iter macroSet;
};

static c_equality
xml_macroNameMatch(
    void *macro_voidp,
    void *name_voidp)
{
    const xml_macro macro = (const xml_macro)macro_voidp;
    const c_char *name = (const c_char *)name_voidp;
    c_equality result = C_NE;

    if (strcmp (macro->name, name) == 0) {
	result = C_EQ;
    }
    return result;
}

xml_macroSet
xml_macroSetNew (
    void)
{
    xml_macroSet macroSet = os_malloc ((size_t)C_SIZEOF(xml_macroSet));

    macroSet->macroSet = c_iterNew (0);
    return macroSet;
}

void
xml_macroSetFree (
    const xml_macroSet macroSet)
{
    xml_macroSetClear (macroSet);
    c_iterFree (macroSet->macroSet);
    os_free (macroSet);
}

void
xml_macroSetAdd (
    const xml_macroSet macroSet,
    const xml_macro macro)
{
    xml_macro old_macro;

    old_macro = c_iterResolve (macroSet->macroSet, xml_macroNameMatch, macro->name);
    if (old_macro) {
	c_iterTake (macroSet->macroSet, old_macro);
    }
    c_iterInsert (macroSet->macroSet, macro);
}

void
xml_macroSetRemove (
    const xml_macroSet macroSet,
    const xml_macro macro)
{
    c_iterTake (macroSet->macroSet, macro);
}

void
xml_macroSetClear (
    const xml_macroSet macroSet)
{
    xml_macro macro;

    macro = c_iterTakeFirst (macroSet->macroSet);
    while (macro != NULL) {
	xml_macroFree (macro);
	macro = c_iterTakeFirst (macroSet->macroSet);
    }
}

xml_macro
xml_macroSetGet (
    const xml_macroSet macroSet,
    const c_char *name)
{
    xml_macro macro;

    macro = c_iterResolve (macroSet->macroSet, xml_macroNameMatch, (c_iterResolveCompareArg)name);
    return macro;
}

/****************************************************************
 * Stream Exp Generic Stream Class Implementation               *
 ****************************************************************/
C_STRUCT(xml_stream) {
    c_char	*stream;
    c_long	length;
    c_long	curpos;
};

xml_stream
xml_streamInit (
    const xml_stream stream,
    const c_char *stream_val)
{

    stream->length = strlen (stream_val);
    stream->stream = os_malloc(stream->length + 1);
    os_strcpy(stream->stream, stream_val);
    stream->curpos = 0;
    return stream;
}

void
xml_streamExit (
    const xml_stream stream)
{
    os_free (stream->stream);
}

c_char *
xml_streamGet (
    const xml_stream stream)
{
    return stream->stream;
}

c_char *
xml_streamCurGet (
    const xml_stream stream)
{
    return &stream->stream[stream->curpos];
}

c_long
xml_streamLength (
    const xml_stream stream)
{
    return stream->length;
}

/****************************************************************
 * Stream Exp Input Stream Class Implementation                 *
 ****************************************************************/
C_STRUCT(xml_streamIn) {
    C_EXTENDS(xml_stream);
    xml_macroAttrib macroAttrib;
    xml_macroSet macroSet;
};

xml_streamIn
xml_streamInNew (
    const c_char *stream_val,
    const xml_macroAttrib macroAttrib)
{
    xml_streamIn	stream = os_malloc ((size_t)C_SIZEOF(xml_streamIn));

    assert (stream_val);
    assert (macroAttrib);
    xml_streamInit (xml_stream(stream), stream_val);
    stream->macroAttrib = macroAttrib;
    return stream;
}

void
xml_streamInFree (
    const xml_streamIn stream)
{
    xml_streamExit (xml_stream(stream));
    os_free (stream);
}

c_char
xml_streamInCur (
    const xml_streamIn stream)
{
    xml_stream str = xml_stream(stream);

    return str->stream[str->curpos];
}

c_char
xml_streamInRel (
    const xml_streamIn stream,
    c_long offset)
{
    c_long abspos;
    xml_stream str = xml_stream(stream);

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
xml_streamInWind (
    const xml_streamIn stream)
{
    xml_stream str = xml_stream(stream);

    if (str->curpos < str->length) {
	str->curpos++;
    }
}

c_char
xml_streamInWindCur (
    const xml_streamIn stream)
{
    xml_stream str = xml_stream(stream);

    if (str->curpos < str->length) {
	str->curpos++;
    }
    return str->stream[str->curpos];
}

/****************************************************************
 * Stream Expander Output Stream Class Implementation           *
 ****************************************************************/
C_STRUCT(xml_streamOut) {
    C_EXTENDS(xml_stream);
    c_long max_length;
};

xml_streamOut
xml_streamOutNew (
    c_long max_length)
{
    xml_streamOut stream = os_malloc ((size_t)C_SIZEOF(xml_streamOut));

    xml_streamInit (xml_stream(stream), "");
    stream->max_length = max_length;
    return stream;
}

void
xml_streamOutFree (
    const xml_streamOut stream)
{
    xml_streamExit (xml_stream(stream));
    os_free (stream);
}

c_long
xml_streamOutPut (
    const xml_streamOut stream,
    c_char character)
{
    xml_stream str = xml_stream(stream);
    c_char *new;

    if (stream->max_length == 0) {
	if (((str->curpos) % 100) == 0) {
            new = os_malloc((size_t)(str->curpos + 101));
            memcpy(new, str->stream, str->curpos);
            os_free(str->stream);
	    str->stream = new;
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

c_char *
xml_streamOutGetAndClear(
    const xml_streamOut stream)
{
    xml_stream str = xml_stream(stream);
    c_char *result;

    result = str->stream;
    xml_streamInit (xml_stream(stream), "");

    return result;
}

/****************************************************************
 * File Output Stream Class Implementation                      *
 ****************************************************************/
C_STRUCT(xml_fileOut) {
    FILE *file;
};

xml_fileOut
xml_fileOutNew (
    const c_char *name,
    const c_char *mode)
{
    char * filename;
    xml_fileOut stream = os_malloc ((size_t)C_SIZEOF(xml_fileOut));

    filename = os_fileNormalize(name);
    stream->file = fopen (filename, mode);
    os_free(filename);

    if (stream->file == NULL) {
       os_free (stream);
       stream = NULL;
    }
    return stream;
}

void
xml_fileOutFree (
    const xml_fileOut stream)
{
    fclose (stream->file);
    os_free (stream);
}

c_long
xml_fileOutPut (
    const xml_fileOut stream,
    c_char character)
{
    c_long result;

    result = putc (character, stream->file);
    return result;
}

void
xml_fileOutPrintf (
    const xml_fileOut stream,
    const c_char *format,
    ...)
{
    va_list args;

    va_start (args, format);
    vfprintf (stream->file, format, args);
    va_end (args);
}

static xml_fileOut xml_fileCurrentOut = NULL;

void
xml_fileSetCur (
    const xml_fileOut fileOut)
{
    xml_fileCurrentOut = fileOut;
}

xml_fileOut
xml_fileCur (
    void)
{
    return xml_fileCurrentOut;
}

/****************************************************************
 * Stream Exp Tmpl Exp Class Implementation                     *
 ****************************************************************/
C_STRUCT(xml_tmplExp) {
    xml_macroSet macroSet;
};

static void
xml_tmplExpInsertMacro (
    const xml_tmplExp tmplExp,
    const c_char *name,
    const xml_streamOut so,
    const xml_macroAttrib macroAttrib);

static const c_char *
xml_tmplExpMacroValue (
    const xml_tmplExp tmplExp,
    const c_char *name);

static void
xml_tmplExpDeleteMacro (
    const xml_tmplExp tmplExp,
    const c_char *name);

static void
xml_tmplExpInsertText (
    const xml_streamOut so,
    const c_char *text);

static int
xml_tmplExpGetMacroSingleArg (
    const xml_tmplExp tmplExp,
    const xml_streamIn si,
    c_char *arg);

static int
xml_tmplExpGetMacroDoubleArg (
    const xml_tmplExp tmplExp,
    const xml_streamIn si,
    c_char *arg1,
    const c_char *arg2);

static int
xml_tmplExpProcessMacro (
    const xml_tmplExp tmplExp,
    const xml_streamIn si,
    const xml_streamOut so);

xml_tmplExp
xml_tmplExpNew (
    const xml_macroSet macroSet)
{
    xml_tmplExp tmplExp = os_malloc ((size_t)C_SIZEOF(xml_tmplExp));

    tmplExp->macroSet = macroSet;
    return tmplExp;
}

void
xml_tmplExpFree (
    const xml_tmplExp tmplExp)
{
    os_free (tmplExp);
}

static void
xml_tmplExpInsertMacro (
    const xml_tmplExp tmplExp,
    const c_char *name,
    const xml_streamOut so,
    const xml_macroAttrib macroAttrib)
{
    int inserted = 0;
    xml_streamIn macroStream;
    xml_macro macro;

    macro = xml_macroSetGet (tmplExp->macroSet, name);
    if (macro) {
	macroStream = xml_streamInNew (xml_macroValue(macro), macroAttrib);

    	while (xml_streamInCur(macroStream) != '\0') {
	    if (xml_streamInCur(macroStream) == macroAttrib->startToken) {
		xml_tmplExpProcessMacro (tmplExp, macroStream, so);
	    } else {
		xml_streamOutPut(so, xml_streamInCur(macroStream));
		xml_streamInWind(macroStream);
	    }
	}
	xml_streamOutPut(so, '\0');
	xml_streamInFree (macroStream);
	inserted = 1;
    }
    if (!inserted) {
        if (os_getenv ((char*)name)) {
	    c_char *value;

	    value = os_getenv ((char*)name);
	    macroStream = xml_streamInNew (value, macroAttrib);

    	    while (xml_streamInCur(macroStream) != '\0') {
		if (xml_streamInCur(macroStream) == macroAttrib->startToken) {
		    xml_tmplExpProcessMacro (tmplExp, macroStream, so);
		} else {
		    xml_streamOutPut(so, xml_streamInCur(macroStream));
		    xml_streamInWind(macroStream);
		}
	    }
            xml_streamInFree(macroStream);

	    inserted = 1;
        }
    }
    if (!inserted) {
	fprintf (stderr, "insert_macro: Undefined macro '%s'\n", name);
    }
}

static const c_char *
xml_tmplExpMacroValue (
    const xml_tmplExp tmplExp,
    const c_char *name)
{
    xml_macro macro;

    macro = xml_macroSetGet (tmplExp->macroSet, name);
    if (macro) {
        return xml_macroValue (macro);
    }

    if (os_getenv ((char*)name)) {
	return (os_getenv ((char*)name));
    }
    fprintf (stderr, "macro_value: Undefined macro '%s'\n", name);
    return "";
}

static void
xml_tmplExpDeleteMacro (
    const xml_tmplExp tmplExp,
    const c_char *name)
{
    int deleted = 0;
    xml_macro macro;

    macro = xml_macroSetGet (tmplExp->macroSet, name);
    if (macro) {
        xml_macroSetRemove (tmplExp->macroSet, macro);
	deleted = 1;
    }
    if (!deleted) {
	fprintf (stderr, "delete_macro: Undefined macro '%s'\n", name);
    }
}

static void
xml_tmplExpInsertText (
    const xml_streamOut so,
    const c_char *text)
{
    unsigned int y;

    for (y = 0; y < strlen (text); y++) {
	xml_streamOutPut (so, text[y]);
    }
}

static int
xml_tmplExpGetMacroSingleArg (
    const xml_tmplExp tmplExp,
    const xml_streamIn si,
    c_char *arg)
{
    xml_streamOut argStream;

    argStream = xml_streamOutNew (0);

    while (xml_streamInCur(si) != '\n' && xml_streamInCur(si) != si->macroAttrib->closeToken) {
        if (xml_streamInCur(si) == si->macroAttrib->startToken) {
	    xml_tmplExpProcessMacro (tmplExp, si, argStream);
	} else {
	    xml_streamOutPut (argStream, xml_streamInCur(si));
	    xml_streamInWind(si);
	}
    }
    xml_streamOutPut (argStream, '\0');
    os_strcpy (arg, xml_streamGet(xml_stream(argStream)));
    xml_streamOutFree (argStream);

    if (xml_streamInCur(si) == si->macroAttrib->closeToken) {
        xml_streamInWind(si);
	return 1;
    }
    return 0;
}

static int
xml_tmplExpGetMacroDoubleArg (
    const xml_tmplExp tmplExp,
    const xml_streamIn si,
    c_char *arg1,
    const c_char *arg2)
{
    xml_streamOut arg1Stream;
    xml_streamOut arg2Stream;
    int return_val;

    arg1Stream = xml_streamOutNew (0);

    while (xml_streamInCur(si) != '\n' && xml_streamInCur(si) != ',') {
        if (xml_streamInCur(si) == si->macroAttrib->startToken) {
    	    xml_tmplExpProcessMacro (tmplExp, si, arg1Stream);
        } else {
	    xml_streamOutPut (arg1Stream, xml_streamInCur(si));
    	    xml_streamInWind(si);
        }
    }
    xml_streamOutPut (arg1Stream, '\0');
    os_strcpy (arg1, xml_streamGet(xml_stream(arg1Stream)));
    xml_streamOutFree (arg1Stream);

    if (xml_streamInCur(si) == ',') {
        arg2Stream = xml_streamOutNew (0);
        xml_streamInWind(si);

        while (xml_streamInCur(si) != '\n' && xml_streamInCur(si) != si->macroAttrib->closeToken) {
            if (xml_streamInCur(si) == si->macroAttrib->startToken) {
    	        xml_tmplExpProcessMacro (tmplExp, si, arg2Stream);
            } else {
	        xml_streamOutPut (arg2Stream, xml_streamInCur(si));
    	        xml_streamInWind(si);
    	    }
        }

        if (xml_streamInCur(si) == si->macroAttrib->closeToken) {
    	    xml_streamInWind(si);
	    xml_streamOutPut (arg2Stream, '\0');
	    return_val = 2;
	} else {
	    return_val = 1;
	}
	xml_streamOutFree (arg2Stream);
	return return_val;
    }
    return_val = 0;
    return return_val;
}

static int
xml_tmplExpProcessMacro (
    const xml_tmplExp tmplExp,
    const xml_streamIn si,
    const xml_streamOut so)
{
    xml_macro macro;
    int result = 0;

    xml_streamInWind(si); /* skip macro start character */


    if ((strncmp (xml_streamCurGet(xml_stream(si)), "def", 3) == 0) &&
	((int)xml_streamInRel (si, 3) == (int)si->macroAttrib->openToken)) {
	int	br_count = 1;
	xml_streamOut defName;
	xml_streamOut defValue;

	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);
	defName = xml_streamOutNew (0);
	defValue = xml_streamOutNew (0);
 
	while (xml_streamInCur(si) != '\n' && xml_streamInCur(si) != '=') {
	    if (xml_streamInCur(si) == si->macroAttrib->startToken) {
		result = xml_tmplExpProcessMacro (tmplExp, si, defName);
	    } else {
	        xml_streamOutPut (defName, xml_streamInCur(si));
		xml_streamInWind(si);
	    }
	}
	xml_streamOutPut (defName, '\0');
	if (xml_streamInCur(si) == '=') {
	    xml_streamInWind(si);

	    while (xml_streamInCur(si) != '\n' &&
		(xml_streamInCur(si) != si->macroAttrib->closeToken || br_count)) {
	        xml_streamOutPut (defValue, xml_streamInCur(si));
		xml_streamInWind(si);

		if (xml_streamInCur(si) == si->macroAttrib->openToken) {
		    br_count++;
		} else if (xml_streamInCur(si) == si->macroAttrib->closeToken) {
		    br_count--;
		}
	    }

	    if (xml_streamInCur(si) == si->macroAttrib->closeToken) {
		xml_streamInWind(si);
	        xml_streamOutPut (defValue, '\0');
		macro = xml_macroNew (xml_streamGet (xml_stream(defName)),
		    xml_streamGet (xml_stream(defValue)));
		xml_macroSetAdd (tmplExp->macroSet, macro);
	    } else {
	        fprintf (stderr, "def: Incomplete definition '%s' missing '%c'\n",
		    xml_streamGet (xml_stream(defName)), si->macroAttrib->closeToken);
	        result = 1;
	    }
	} else {
	    fprintf (stderr, "def: Incomplete definition '%s' missing '='\n",
		xml_streamGet (xml_stream(defName)));
	    result = 1;
	}
	xml_streamOutFree (defName);
	xml_streamOutFree (defValue);

    } else if ((strncmp (xml_streamCurGet(xml_stream(si)), "undef", 5) == 0) &&
	((int)xml_streamInRel (si, 5) == (int)si->macroAttrib->openToken)) {
	c_char	macro_name [NMSIZE];

	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);

	if (xml_tmplExpGetMacroSingleArg (tmplExp, si, macro_name)) {
	    xml_tmplExpDeleteMacro (tmplExp, macro_name);
	} else {
	    fprintf (stderr, "undef: Incomplete function 'undef' missing '%c'\n",
		si->macroAttrib->closeToken);
	    result = 1;
	}
    } else if (xml_streamInCur(si) == si->macroAttrib->openToken) {
	c_char	macro_name [NMSIZE];

	xml_streamInWind(si);

	if (xml_tmplExpGetMacroSingleArg (tmplExp, si, macro_name)) {
	    xml_tmplExpInsertMacro (tmplExp, macro_name, so, si->macroAttrib);
	} else {
	    fprintf (stderr, "%c%c: Incomplete function '%c' missing '%c'\n",
		si->macroAttrib->startToken, si->macroAttrib->openToken,
		si->macroAttrib->startToken, si->macroAttrib->closeToken);
	    result = 1;
	}
    } else if ((strncmp (xml_streamCurGet(xml_stream(si)), "sp", 2) == 0) &&
	((int)xml_streamInRel (si, 2) == (int)si->macroAttrib->openToken)) {
	c_char	spaceCount [NMSIZE];
	unsigned int	ni = 0;
	unsigned int	spc = 0;

	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);

	if (xml_tmplExpGetMacroSingleArg (tmplExp, si, spaceCount)) {
	    sscanf (spaceCount, "%x", &spc);
	    for (ni = 0; ni < spc; ni++) {
		xml_streamOutPut (so, ' ');
	    }
	} else {
	    fprintf (stderr, "upper: Incomplete function 'sp' missing ')'\n");
	    result = 1;
	}
    } else if ((strncmp (xml_streamCurGet(xml_stream(si)), "upper", 5) == 0) &&
	((int)xml_streamInRel (si, 5) == (int)si->macroAttrib->openToken)) {
	c_char	string [NMSIZE];
	unsigned int	ni = 0;

	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);

	if (xml_tmplExpGetMacroSingleArg (tmplExp, si, string)) {
	    for (ni = 0; ni < strlen(string); ni++) {
		string[ni] = toupper (string[ni]);
	    }
	    xml_tmplExpInsertText (so, string);
	} else {
	    fprintf (stderr, "upper: Incomplete function 'upper' missing ')'\n");
	    result = 1;
	}
    } else if ((strncmp (xml_streamCurGet(xml_stream(si)), "lower", 5) == 0) &&
	((int)xml_streamInRel (si, 5) == (int)si->macroAttrib->openToken)) {
	c_char	string [NMSIZE];
	unsigned int	ni = 0;

	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);

	if (xml_tmplExpGetMacroSingleArg (tmplExp, si, string)) {
	    for (ni = 0; ni < strlen(string); ni++) {
		string[ni] = (c_char)tolower (string[ni]);
	    }
	    xml_tmplExpInsertText (so, string);
	} else {
	    fprintf (stderr, "lower: Incomplete function 'lower' missing '%c'\n",
		si->macroAttrib->closeToken);
	    result = 1;
	}
    } else if ((strncmp (xml_streamCurGet(xml_stream(si)), "ltrim", 5) == 0) &&
	((int)xml_streamInRel (si, 5) == (int)si->macroAttrib->openToken)) {
	c_char	src [NMSIZE];
	c_char	trim [NMSIZE];
	c_char	res [NMSIZE];
	c_char	*substr;

	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);
	switch (xml_tmplExpGetMacroDoubleArg (tmplExp, si, src, trim)) {
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
		os_strcpy (res, src);
	    } else {
		substr += strlen(trim);
		os_strcpy (res, substr);
	    }
	    xml_tmplExpInsertText (so, res);
	    break;
	default:
	    printf ("Unexpected case\n");
	    break;
	}
    } else if ((strncmp (xml_streamCurGet(xml_stream(si)), "rtrim", 5) == 0) &&
	((int)xml_streamInRel (si, 5) == (int)si->macroAttrib->openToken)) {
	c_char	src [NMSIZE];
	c_char	trim [NMSIZE];
	c_char	res [NMSIZE];
	c_char	*substr;
	c_char	*p_substr;

	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);
	switch (xml_tmplExpGetMacroDoubleArg (tmplExp, si, src, trim)) {
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
	    xml_tmplExpInsertText (so, res);
	    break;
	default:
	    printf ("Unexpected case\n");
	    break;
	}
    } else if ((strncmp (xml_streamCurGet(xml_stream(si)), "mul", 3) == 0) &&
	((int)xml_streamInRel (si, 3) == (int)si->macroAttrib->openToken)) {
	c_char	mul1 [NMSIZE];
	c_char	mul2 [NMSIZE];
	c_char	res [NMSIZE];
	signed int	value1;
	signed int	value2;

	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);
	switch (xml_tmplExpGetMacroDoubleArg (tmplExp, si, mul1, mul2)) {
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
	    xml_tmplExpInsertText (so, res);
	    break;
	default:
	    printf ("Unexpected case\n");
	    break;
	}
    } else if ((strncmp (xml_streamCurGet(xml_stream(si)), "div", 3) == 0) &&
	((int)xml_streamInRel (si, 3) == (int)si->macroAttrib->openToken)) {
	c_char	div1 [NMSIZE];
	c_char	div2 [NMSIZE];
	c_char	res [NMSIZE];
	signed int	value1;
	signed int	value2;

	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);
	switch (xml_tmplExpGetMacroDoubleArg (tmplExp, si, div1, div2)) {
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
	        xml_tmplExpInsertText (so, res);
	    } else {
	        fprintf (stderr, "div: Divide by zero exception\n");
	    }
	    break;
	default:
	    printf ("Unexpected case\n");
	    break;
	}
    } else if ((strncmp (xml_streamCurGet(xml_stream(si)), "add", 3) == 0) &&
	((int)xml_streamInRel (si, 3) == (int)si->macroAttrib->openToken)) {
	c_char	add1 [NMSIZE];
	c_char	add2 [NMSIZE];
	c_char	res [NMSIZE];
	signed int	value1;
	signed int	value2;

	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);
	switch (xml_tmplExpGetMacroDoubleArg (tmplExp, si, add1, add2)) {
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
	    xml_tmplExpInsertText (so, res);
	    break;
	default:
	    printf ("Unexpected case\n");
	    break;
	}
    } else if ((strncmp (xml_streamCurGet(xml_stream(si)), "sub", 3) == 0) &&
	((int)xml_streamInRel (si, 3) == (int)si->macroAttrib->openToken)) {
	c_char	sub1 [NMSIZE];
	c_char	sub2 [NMSIZE];
	c_char	res [NMSIZE];
	signed int	value1;
	signed int	value2;

	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);
	switch (xml_tmplExpGetMacroDoubleArg (tmplExp, si, sub1, sub2)) {
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
	    xml_tmplExpInsertText (so, res);
	    break;
	default:
	    printf ("Unexpected case\n");
	    break;
	}
    } else if ((strncmp (xml_streamCurGet(xml_stream(si)), "inc", 3) == 0) &&
	((int)xml_streamInRel (si, 3) == (int)si->macroAttrib->openToken)) {
	c_char	macro_name [NMSIZE];
	c_char	res [NMSIZE];
	signed int	val;

	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);

	if (xml_tmplExpGetMacroSingleArg (tmplExp, si, macro_name)) {
	    sscanf (xml_tmplExpMacroValue(tmplExp, macro_name), "%d", &val);
	    snprintf (res, (size_t)sizeof(res), "%d", val+1);
	    xml_tmplExpInsertText (so, res);
	    macro = xml_macroNew (macro_name, res);
	    xml_macroSetAdd (tmplExp->macroSet, macro);
	} else {
	    fprintf (stderr, "inc: Incomplete function 'inc' missing '%c'\n",
		si->macroAttrib->closeToken);
	    result = 1;
	}
    } else if ((strncmp (xml_streamCurGet(xml_stream(si)), "dec", 3)) == 0 &&
	((int)xml_streamInRel (si, 3) == (int)si->macroAttrib->openToken)) {
	c_char	macro_name [NMSIZE];
	c_char	res [NMSIZE];
	signed int	val;

	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);

	if (xml_tmplExpGetMacroSingleArg (tmplExp, si, macro_name)) {
	    sscanf (xml_tmplExpMacroValue(tmplExp, macro_name), "%d", &val);
	    snprintf (res, (size_t)sizeof(res), "%d", val-1);
	    xml_tmplExpInsertText (so, res);
	    macro = xml_macroNew (macro_name, res);
	    xml_macroSetAdd (tmplExp->macroSet, macro);
	} else {
	    fprintf (stderr, "dec: Incomplete function 'dec' missing '%c'\n",
		si->macroAttrib->closeToken);
	    result = 1;
	}
    } else if ((strncmp (xml_streamCurGet(xml_stream(si)), "hex", 3) == 0) &&
	((int)xml_streamInRel (si, 3) == (int)si->macroAttrib->openToken)) {
	c_char	arg [NMSIZE];
	c_char	res [NMSIZE];
	signed int	val;

	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);

	if (xml_tmplExpGetMacroSingleArg (tmplExp, si, arg)) {
	    sscanf (arg, "%d", &val);
	    snprintf (res, (size_t)sizeof(res), "%x", val);
	    xml_tmplExpInsertText (so, res);
	} else {
	    fprintf (stderr, "hex: Incomplete function 'hex' missing '%c'\n",
		si->macroAttrib->closeToken);
	    result = 1;
	}
    } else if ((strncmp (xml_streamCurGet(xml_stream(si)), "oct", 3) == 0) &&
	((int)xml_streamInRel (si, 3) == (int)si->macroAttrib->openToken)) {
	c_char	arg [NMSIZE];
	c_char	res [NMSIZE];
	signed int	val;

	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);
	xml_streamInWind(si);

	if (xml_tmplExpGetMacroSingleArg (tmplExp, si, arg)) {
	    sscanf (arg, "%d", &val);
	    snprintf (res, (size_t)sizeof(res), "%o", val);
	    xml_tmplExpInsertText (so, res);
	} else {
	    fprintf (stderr, "oct: Incomplete function 'oct' missing '%c'\n",
		si->macroAttrib->closeToken);
	    result = 1;
	}
    } else {
	xml_streamInWind(si);
	fprintf (stderr, "Unknown macro\n");
    }
    return result;
}

void
xml_tmplExpProcessTmpl (
    const xml_tmplExp tmplExp,
    const xml_streamIn si,
    const xml_fileOut fo)
{
    xml_streamOut so;
    int processResult = 0;


    while (((int)xml_streamInCur(si) != (int)'\0') && (processResult == 0)) {
	if ((int)xml_streamInCur(si) == (int)si->macroAttrib->startToken) {
	    so = xml_streamOutNew (0);
	    processResult = xml_tmplExpProcessMacro (tmplExp, si, so);
	    xml_fileOutPrintf (fo, xml_streamGet (xml_stream(so)));
	    xml_streamOutFree (so);
	} else {
	    xml_fileOutPut(fo, xml_streamInCur(si));
	    xml_streamInWind(si);
	}
    }
}

int
xml_tmplExpProcessTmplToStream (
    const xml_tmplExp tmplExp,
    const xml_streamIn si,
    const xml_streamOut so)
{
    int processResult = 0;

 
    while (((int)xml_streamInCur(si) != (int)'\0') && (processResult == 0)) {
	if ((int)xml_streamInCur(si) == (int)si->macroAttrib->startToken) {
	    processResult = xml_tmplExpProcessMacro (tmplExp, si, so);
	} else {
            xml_streamOutPut(so, xml_streamInCur(si));
	    xml_streamInWind(si);
	}
    }

    return processResult;
}

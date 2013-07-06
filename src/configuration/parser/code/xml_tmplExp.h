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

/****************************************************************
 * Interface definition for stream expander                     *
 ****************************************************************/

/** \file xml_tmplExp.h
 *  \brief Expand input stream (tmpl) to output stream
 */

#ifndef XML_TMPLEXP_H
#define XML_TMPLEXP_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "c_typebase.h"

/****************************************************************
 * Stream Exp Macro Attributes Class                            *
 ****************************************************************/
C_CLASS(xml_macroAttrib);

#define xml_macroAttrib(o)	((xml_macroAttrib)(o))

xml_macroAttrib xml_macroAttribNew (c_char startToken, c_char openToken, c_char closeToken);

void xml_macroAttribFree (const xml_macroAttrib macroAttrib);

/****************************************************************
 * Stream Exp Macro Class                                       *
 ****************************************************************/
C_CLASS(xml_macro);

#define xml_macro(o)	((xml_macro)(o))

xml_macro xml_macroNew (const c_char *name, const c_char *value);

void xml_macroFree (const xml_macro macro);

c_char *xml_macroName (const xml_macro macro);

c_char *xml_macroValue (const xml_macro macro);

/****************************************************************
 * Stream Exp Macro Set Class                                   *
 ****************************************************************/
C_CLASS(xml_macroSet);

#define xml_macroSet(o)	((xml_macroSet)(o))

xml_macroSet xml_macroSetNew (void);

void xml_macroSetFree (const xml_macroSet macroSet);

void xml_macroSetAdd (const xml_macroSet macroSet, const xml_macro macro);

void xml_macroSetRemove (const xml_macroSet macroSet, const xml_macro macro);

void xml_macroSetClear (const xml_macroSet macroSet);

xml_macro xml_macroSetGet (const xml_macroSet macroSet, const c_char *name);

/****************************************************************
 * Stream Exp Generic Stream Class                              *
 ****************************************************************/
C_CLASS(xml_stream);

#define xml_stream(o)	((xml_stream)(o))

/* Constructor */
xml_stream xml_streamInit (const xml_stream stream, const c_char *stream_val);

/* Destructor */
void xml_streamExit (const xml_stream stream);

/* Get pointer to complete character stream */
c_char *xml_streamGet (const xml_stream stream);

/* Get pointer to complete character stream at current position */
c_char *xml_streamCurGet (const xml_stream stream);

/* Get stream length */
c_long xml_streamLength (const xml_stream stream);

/****************************************************************
 * Stream Exp Input Stream Class                                *
 ****************************************************************/
C_CLASS(xml_streamIn);

#define xml_streamIn(o)	((xml_streamIn)(o))

/* Constructor */
xml_streamIn xml_streamInNew (const c_char *stream_val, const xml_macroAttrib macroAttrib);

/* Destructor */
void xml_streamInFree (const xml_streamIn stream);

/* Get character under stream pointer */
c_char xml_streamInCur (const xml_streamIn stream);

/* Get character relative to current stream pointer */
c_char xml_streamInRel (const xml_streamIn stream, c_long offset);

/* Increment stream pointer */
void xml_streamInWind (const xml_streamIn stream);

/* Increment stream pointer and get character */
c_char xml_streamInWindCur (const xml_streamIn stream);

/****************************************************************
 * Stream Expander Output Stream Class                          *
 ****************************************************************/
C_CLASS(xml_streamOut);

#define xml_streamOut(o)	((xml_streamOut)(o))

/* Constructor, max_length 0 identifies extendable stream */
xml_streamOut xml_streamOutNew (c_long max_length);

/* Destructor */
void xml_streamOutFree (const xml_streamOut stream);

/* Put character at current position and increment pointer and length */
c_long xml_streamOutPut (const xml_streamOut stream, c_char character);
c_char *
xml_streamOutGetAndClear(
    const xml_streamOut stream);

/****************************************************************
 * File Output Stream Class                                     *
 ****************************************************************/
C_CLASS(xml_fileOut);

#define xml_fileOut(o)	((xml_fileOut)(o))

/* Constructor stream */
xml_fileOut xml_fileOutNew (const c_char *fileName, const c_char *mode);

/* Destructor */
void xml_fileOutFree (const xml_fileOut stream);

/* Put character at current position and increment pointer and length */
c_long xml_fileOutPut (const xml_fileOut stream, c_char character);

/* Print formatted to output stream */
void xml_fileOutPrintf (const xml_fileOut stream, const c_char *format, ...);

void xml_fileSetCur (const xml_fileOut fileOut);

xml_fileOut xml_fileCur (void);

/****************************************************************
 * Stream Exp Tmpl Exp Class                                    *
 ****************************************************************/
C_CLASS(xml_tmplExp);

#define xml_tmplExp(o)	((xml_tmplExp)(o))

/* Constructor */
xml_tmplExp xml_tmplExpNew (const xml_macroSet macroSet);

/* Destructor */
void xml_tmplExpFree (const xml_tmplExp tmplExp);

/* Expand the input stream si into output stream so */
void xml_tmplExpProcessTmpl (const xml_tmplExp tmplExp, const xml_streamIn si, const xml_fileOut fo);

int
xml_tmplExpProcessTmplToStream (
    const xml_tmplExp tmplExp,
    const xml_streamIn si,
    const xml_streamOut so);

#if defined (__cplusplus)
}
#endif

#endif /* XML_TMPLEXP_H */

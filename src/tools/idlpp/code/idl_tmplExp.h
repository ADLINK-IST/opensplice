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

/** \file idl_tmplExp.h
 *  \brief Expand input stream (tmpl) to output stream
 */

#ifndef IDL_TMPLEXP_H
#define IDL_TMPLEXP_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "c_typebase.h"

/****************************************************************
 * Stream Exp Macro Attributes Class                            *
 ****************************************************************/
C_CLASS(idl_macroAttrib);

#define idl_macroAttrib(o)	((idl_macroAttrib)(o))

idl_macroAttrib idl_macroAttribNew (c_char startToken, c_char openToken, c_char closeToken);

void idl_macroAttribFree (const idl_macroAttrib macroAttrib);

/****************************************************************
 * Stream Exp Macro Class                                       *
 ****************************************************************/
C_CLASS(idl_macro);

#define idl_macro(o)	((idl_macro)(o))

idl_macro idl_macroNew (const c_char *name, const c_char *value);

void idl_macroFree (const idl_macro macro);

c_char *idl_macroName (const idl_macro macro);

c_char *idl_macroValue (const idl_macro macro);

/****************************************************************
 * Stream Exp Macro Set Class                                   *
 ****************************************************************/
C_CLASS(idl_macroSet);

#define idl_macroSet(o)	((idl_macroSet)(o))

idl_macroSet idl_macroSetNew (void);

void idl_macroSetFree (const idl_macroSet macroSet);

void idl_macroSetAdd (const idl_macroSet macroSet, const idl_macro macro);

void idl_macroSetRemove (const idl_macroSet macroSet, const idl_macro macro);

void idl_macroSetClear (const idl_macroSet macroSet);

idl_macro idl_macroSetGet (const idl_macroSet macroSet, const c_char *name);

/****************************************************************
 * Stream Exp Generic Stream Class                              *
 ****************************************************************/
C_CLASS(idl_stream);

#define idl_stream(o)	((idl_stream)(o))

/* Constructor */
idl_stream idl_streamInit (const idl_stream stream, const c_char *stream_val);

/* Destructor */
void idl_streamExit (const idl_stream stream);

/* Get pointer to complete character stream */
c_char *idl_streamGet (const idl_stream stream);

/* Get pointer to complete character stream at current position */
c_char *idl_streamCurGet (const idl_stream stream);

/* Get stream length */
c_long idl_streamLength (const idl_stream stream);

/****************************************************************
 * Stream Exp Input Stream Class                                *
 ****************************************************************/
C_CLASS(idl_streamIn);

#define idl_streamIn(o)	((idl_streamIn)(o))

/* Constructor */
idl_streamIn idl_streamInNew (const c_char *stream_val, const idl_macroAttrib macroAttrib);

/* Destructor */
void idl_streamInFree (const idl_streamIn stream);

/* Get character under stream pointer */
c_char idl_streamInCur (const idl_streamIn stream);

/* Get character relative to current stream pointer */
c_char idl_streamInRel (const idl_streamIn stream, c_long offset);

/* Increment stream pointer */
void idl_streamInWind (const idl_streamIn stream);

/* Increment stream pointer and get character */
c_char idl_streamInWindCur (const idl_streamIn stream);

/****************************************************************
 * Stream Expander Output Stream Class                          *
 ****************************************************************/
C_CLASS(idl_streamOut);

#define idl_streamOut(o)	((idl_streamOut)(o))

/* Constructor, max_length 0 identifies extendable stream */
idl_streamOut idl_streamOutNew (c_long max_length);

/* Destructor */
void idl_streamOutFree (const idl_streamOut stream);

/* Put character at current position and increment pointer and length */
c_long idl_streamOutPut (const idl_streamOut stream, c_char character);

/****************************************************************
 * File Output Stream Class                                     *
 ****************************************************************/
C_CLASS(idl_fileOut);

#define idl_fileOut(o)	((idl_fileOut)(o))

c_bool      idl_dirOutNew   (const c_char *name);

c_char*     idl_dirOutCur   (void);

void        idl_dirOurFree  (void);

/* Constructor stream */
idl_fileOut idl_fileOutNew (const c_char *fileName, const c_char *mode);

/* Destructor */
void idl_fileOutFree (const idl_fileOut stream);

/* Put character at current position and increment pointer and length */
c_long idl_fileOutPut (const idl_fileOut stream, c_char character);

/* Print formatted to output stream */
void idl_fileOutPrintf (const idl_fileOut stream, const c_char *format, ...);

void idl_fileSetCur (const idl_fileOut fileOut);

idl_fileOut idl_fileCur (void);

/****************************************************************
 * Stream Exp Tmpl Exp Class                                    *
 ****************************************************************/
C_CLASS(idl_tmplExp);

#define idl_tmplExp(o)	((idl_tmplExp)(o))

/* Constructor */
idl_tmplExp idl_tmplExpNew (const idl_macroSet macroSet);

/* Destructor */
void idl_tmplExpFree (const idl_tmplExp tmplExp);

/* Expand the input stream si into output stream so */
void idl_tmplExpProcessTmpl (const idl_tmplExp tmplExp, const idl_streamIn si, const idl_fileOut fo);

#if defined (__cplusplus)
}
#endif

#endif /* IDL_TMPLEXP_H */

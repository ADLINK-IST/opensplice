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

/** \file ut_tmplExp.h
 *  \brief Expand input stream (tmpl) to output stream
 */

#ifndef UT_TMPLEXP_H
#define UT_TMPLEXP_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "os_defs.h"
#include "os_classbase.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/****************************************************************
 * Stream Exp Macro Attributes Class                            *
 ****************************************************************/
OS_CLASS(ut_macroAttrib);

#define ut_macroAttrib(o)	((ut_macroAttrib)(o))

OS_API ut_macroAttrib
ut_macroAttribNew(
    os_char startToken,
    os_char openToken,
    os_char closeToken);

OS_API void
ut_macroAttribFree(
    const ut_macroAttrib macroAttrib);

/****************************************************************
 * Stream Exp Macro Class                                       *
 ****************************************************************/
OS_CLASS(ut_macro);

#define ut_macro(o)	((ut_macro)(o))

OS_API ut_macro
ut_macroNew(
    const os_char *name,
    const os_char *value);

OS_API void
ut_macroFree(
    const ut_macro macro);

OS_API os_char *
ut_macroName(
    const ut_macro macro);

OS_API os_char *
ut_macroValue(
    const ut_macro macro);

/****************************************************************
 * Stream Exp Macro Set Class                                   *
 ****************************************************************/
OS_CLASS(ut_macroSet);

#define ut_macroSet(o)	((ut_macroSet)(o))

OS_API ut_macroSet
ut_macroSetNew(void);

OS_API void
ut_macroSetFree(
    const ut_macroSet macroSet);

OS_API void
ut_macroSetAdd(
    const ut_macroSet macroSet,
    const ut_macro macro);

OS_API void
ut_macroSetRemove(
    const ut_macroSet macroSet,
    const ut_macro macro);

OS_API void
ut_macroSetClear(
    const ut_macroSet macroSet);

OS_API ut_macro
ut_macroSetGet(
    const ut_macroSet macroSet,
    const os_char *name);

/****************************************************************
 * Stream Exp Generic Stream Class                              *
 ****************************************************************/
OS_CLASS(ut_stream);

#define ut_stream(o)	((ut_stream)(o))

/* Constructor */
OS_API ut_stream
ut_streamInit(
    const ut_stream stream,
    const os_char *stream_val);

/* Destructor */
OS_API void
ut_streamExit(
    const ut_stream stream);

/* Get pointer to complete character stream */
OS_API os_char *
ut_streamGet(
    const ut_stream stream);

/* Get pointer to complete character stream at current position */
OS_API os_char *
ut_streamCurGet(
    const ut_stream stream);

/* Get stream length */
OS_API os_int32
ut_streamLength(
    const ut_stream stream);

/****************************************************************
 * Stream Exp Input Stream Class                                *
 ****************************************************************/
OS_CLASS(ut_streamIn);

#define ut_streamIn(o)	((ut_streamIn)(o))

/* Constructor */
OS_API ut_streamIn
ut_streamInNew(
    const os_char *stream_val,
    const ut_macroAttrib macroAttrib);

/* Destructor */
OS_API void
ut_streamInFree(
    const ut_streamIn stream);

/* Get character under stream pointer */
OS_API os_char
ut_streamInCur(
    const ut_streamIn stream);

/* Get character relative to current stream pointer */
OS_API os_char
ut_streamInRel(
    const ut_streamIn stream,
    os_uint32 offset);

/* Increment stream pointer */
OS_API void
ut_streamInWind(
    const ut_streamIn stream);

/* Increment stream pointer and get character */
OS_API os_char
ut_streamInWindCur(
    const ut_streamIn stream);

/****************************************************************
 * Stream Expander Output Stream Class                          *
 ****************************************************************/
OS_CLASS(ut_streamOut);

#define ut_streamOut(o)	((ut_streamOut)(o))

/* Constructor, max_length 0 identifies extendable stream */
OS_API ut_streamOut
ut_streamOutNew(
    os_uint32 max_length);

/* Destructor */
OS_API void
ut_streamOutFree(
    const ut_streamOut stream);

/* Put character at current position and increment pointer and length */
OS_API os_uint32
ut_streamOutPut(
    const ut_streamOut stream,
    os_char character);

OS_API os_char *
ut_streamOutGetAndClear(
    const ut_streamOut stream);

/****************************************************************
 * File Output Stream Class                                     *
 ****************************************************************/
OS_CLASS(ut_fileOut);

#define ut_fileOut(o)	((ut_fileOut)(o))

OS_API os_int32
ut_dirOutNew(
    const os_char *name);

OS_API os_char *
ut_dirOutCur(void);

OS_API void
ut_dirOutFree(void);

/* Constructor stream */
OS_API ut_fileOut
ut_fileOutNew(
    const os_char *fileName,
    const os_char *mode);

/* Destructor */
OS_API void
ut_fileOutFree(
    const ut_fileOut stream);

/* Put character at current position and increment pointer and length */
OS_API os_int32
ut_fileOutPut(
    const ut_fileOut stream,
    os_char character);

/* Print formatted to output stream */
OS_API void
ut_fileOutPrintf(
    const ut_fileOut stream,
    const os_char *format,
    ...);

OS_API void
ut_fileSetCur(
    const ut_fileOut fileOut);

OS_API ut_fileOut
ut_fileCur(void);

/****************************************************************
 * Stream Exp Tmpl Exp Class                                    *
 ****************************************************************/
OS_CLASS(ut_tmplExp);

#define ut_tmplExp(o)	((ut_tmplExp)(o))

/* Constructor */
OS_API ut_tmplExp
ut_tmplExpNew(
    const ut_macroSet macroSet);

/* Destructor */
OS_API void
ut_tmplExpFree(
    const ut_tmplExp tmplExp);

/* Expand the input stream si into output stream so */
OS_API void
ut_tmplExpProcessTmpl(
    const ut_tmplExp tmplExp,
    const ut_streamIn si,
    const ut_fileOut fo);

OS_API int
ut_tmplExpProcessTmplToStream (
    const ut_tmplExp tmplExp,
    const ut_streamIn si,
    const ut_streamOut so);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* UT_TMPLEXP_H */

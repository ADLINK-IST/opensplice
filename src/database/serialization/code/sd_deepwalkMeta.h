/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
/** \file services/serialization/code/sd_deepwalkMeta.h
 *  \brief Prototypes for the deepwalkMeta functionality, to be used by 
 *         \b serializer descendants which need extended meta information.
 */

#ifndef SD_DEEPWALKMETA_H
#define SD_DEEPWALKMETA_H

#include "c_typebase.h"
#include "c_metabase.h"

/* A helper class for storing context information concerning the metaDeepwalk */

C_CLASS(sd_errorInfo);


typedef void   (*sd_deepwalkMetaFunc)(const c_char *name, c_type type, 
                                      c_object *object, void *arg,
                                      sd_errorInfo *errorInfo, void *userData);

typedef c_bool (*sd_deepwalkMetaHook)(const char *name, c_baseObject propOrMem,
                                      c_object *object, void *arg,
                                      void *userData);


sd_errorInfo           sd_errorInfoNew(
                           c_ulong errorNumber,
                           const c_char *name,
                           const c_char *message,
                           c_char *location);

void                   sd_errorInfoFree(
                           sd_errorInfo errorInfo);

c_char *               sd_errorInfoGetName(
                           sd_errorInfo errorInfo);                           
                           
void                   sd_errorInfoSetName(
                           sd_errorInfo errorInfo,
                           const c_char *name);
                           
c_bool                 sd_errorInfoGet(
                           sd_errorInfo errorInfo,
                           c_ulong *errorNumber,
                           c_char **name,
                           c_char **message,
                           c_char **location);
                           
C_CLASS(sd_deepwalkMetaContext);

sd_deepwalkMetaContext sd_deepwalkMetaContextNew(
                           const sd_deepwalkMetaFunc actionPre,
                           const sd_deepwalkMetaFunc actionPost,
                           const sd_deepwalkMetaHook beforeAction,
                           void *actionArg,
                           c_bool doValidation,
                           void *userData);
                           
void                   sd_deepwalkMetaContextFree(
                           sd_deepwalkMetaContext context);
                           
c_bool                 sd_deepwalkMetaContextGetErrorInfo(
                           sd_deepwalkMetaContext context,
                           c_ulong *errorNumber,
                           c_char **name,
                           c_char **message,
                           c_char **location);


/* The function itself */

void                   sd_deepwalkMeta(
                           c_type type,
                           const c_char *name,
                           c_object *objectPtr,
                           sd_deepwalkMetaContext context);


#endif  /* SD_DEEPWALKMETA_H */

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
/** \file services/serialization/code/sd__deepwalkMeta.h
 *  \brief Prototypes for the deepwalkMeta functionality, to be used by
 *         \b serializer descendants which need extended meta information.
 */

#ifndef SD__DEEPWALKMETA_H
#define SD__DEEPWALKMETA_H

#include "c_typebase.h"
#include "c_metabase.h"
#include "sd_deepwalkMeta.h"

/* A helper class for storing context information concerning the metaDeepwalk */


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


#endif  /* SD__DEEPWALKMETA_H */

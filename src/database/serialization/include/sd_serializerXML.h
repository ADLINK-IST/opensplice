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
/** \file services/serialization/include/sd_serializerXML.h
 *  \brief Declaration of the \b serializerXML class.
 */

#ifndef SD_SERIALIZERXML_H
#define SD_SERIALIZERXML_H

#include "sd_serializer.h"
#include "c_base.h"
#include "os_if.h"

#ifdef OSPL_BUILD_SER
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

OS_API sd_serializer sd_serializerXMLNew(c_base base);
OS_API sd_serializer sd_serializerXMLNewTyped(c_type type);

#undef OS_API

#endif /* SD_SERIALIZERXML_H */

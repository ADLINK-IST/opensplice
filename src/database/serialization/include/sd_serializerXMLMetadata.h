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
/** \file database/serialization/include/sd_serializerXMLMetadata.h
 *  \brief Declaration of the \b serializerXMLMetadata class.
 */

#ifndef SD_SERIALIZERXMLMETADATA_H
#define SD_SERIALIZERXMLMETADATA_H

#include "sd_serializer.h"
#include "c_base.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

OS_API sd_serializer sd_serializerXMLMetadataNew(c_base base);

#undef OS_API

#endif /* SD_SERIALIZERXMLMETADATA_H */

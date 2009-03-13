/** \file database/serialization/include/sd_serializerXMLMetadata.h
 *  \brief Declaration of the \b serializerXMLMetadata class.
 */

#ifndef SD_SERIALIZERXMLMETADATA_H
#define SD_SERIALIZERXMLMETADATA_H

#include "sd_serializer.h"
#include "c_base.h"
#include "os_if.h"

#ifdef OSPL_BUILD_SER
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

OS_API sd_serializer sd_serializerXMLMetadataNew(c_base base);

#undef OS_API

#endif /* SD_SERIALIZERXMLMETADATA_H */

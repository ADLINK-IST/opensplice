#ifndef SD__PRINTXMLTYPEINFO_H
#define SD__PRINTXMLTYPEINFO_H

#include "sd__contextItem.h"

#define SPLICE_METADATA_TAG  "MetaData"

c_long
sd_printXmlTypeinfoLength (
    sd_contextItem item,
    c_bool escapeQuote);

void
sd_printXmlTypeinfo (
    sd_contextItem item,
    char *buffer,
    c_bool escapeQuote
    );

#endif /* SD__PRINTXMLTYPEINFO_H */

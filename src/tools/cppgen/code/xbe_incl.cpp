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
#include "xbe_incl.h"

String_map DDSRealIncludeFiles::includes(idlc_hash_str);

DDS_StdString DDSRealIncludeFiles::clientHeader;
DDS_StdString DDSRealIncludeFiles::serverHeader;

UTL_IncludeFiles *UTL_IncludeFiles::realImplementation = new DDSRealIncludeFiles;

void DDSRealIncludeFiles::reallyAddIncludeFile(char *header, char *path)
{
   includes[(DDS_StdString)header] = (DDS_StdString)path;
}

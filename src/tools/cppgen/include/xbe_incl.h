#ifndef _XBE_INCL_HH_
#define _XBE_INCL_HH_

#include "utl_incl.h"
#include "xbe_globals.h"
#include "StdString.h"

class DDSRealIncludeFiles : public UTL_IncludeFiles
{
public:

   virtual ~DDSRealIncludeFiles()
   {}

   static String_map includes;
   static DDS_StdString clientHeader;
   static DDS_StdString serverHeader;

protected:

   virtual void reallyAddIncludeFile (char *header, char *path);
};

#endif

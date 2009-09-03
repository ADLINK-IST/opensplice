#ifndef _DDS_STD_H_
#define _DDS_STD_H_

#include "sacpp_DDS_DCPS.h"
#include "StdList.h"
#include "StdString.h"

class DDS_StdString;

class Std
{
public:

   typedef DDS_StdString String;
   typedef StdList List;

   static DDS::ULong hash_str (const char* str);
};

#endif

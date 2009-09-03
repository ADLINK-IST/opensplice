#ifndef XBE_GENLIST_HH_INCLUDED
#define XBE_GENLIST_HH_INCLUDED

#include <idl.h>
#include "xbe_codegen.h"
#include "xbe_cppscope.h"

class be_GeneratorList
{
   public:
      void Add(be_CodeGenerator* codegen);
      void GenerateGlobal (be_ClientHeader& source);

   private:
      DDSVector<be_CodeGenerator*> m_list;
};

extern be_GeneratorList g_generatorList;

#endif // XBE_GENLIST_HH_INCLUDED

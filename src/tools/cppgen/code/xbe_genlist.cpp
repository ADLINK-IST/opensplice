#include "xbe_genlist.h"

be_GeneratorList g_generatorList;

void
be_GeneratorList::Add(be_CodeGenerator* codegen)
{
   m_list.push_back (codegen);
}

void
be_GeneratorList::GenerateGlobal (be_ClientHeader& source)
{
   for (unsigned int i = 0; i < m_list.size(); i++)
   {
      m_list[i]->GenerateGlobal (source);
   }
}

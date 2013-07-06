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

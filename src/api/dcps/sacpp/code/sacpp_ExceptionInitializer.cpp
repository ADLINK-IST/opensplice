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
#include "sacpp_DDS_DCPS.h"
#include "sacpp_ExceptionInitializer.h"
#include "sacpp_if.h"

DDS::ExceptionInitializer *DDS::ExceptionInitializer::m_head = 0;

DDS::ExceptionInitializer::ExceptionInitializer 
   (const char *name, Factory fact)
:
   m_name (name),
   m_fact (fact)
{
   if (lookup (name) == 0)
   {
      m_next = m_head;
      m_head = this;
   }
}

DDS::ExceptionInitializer::Factory DDS::ExceptionInitializer::lookup (const char *name)
{
   for (ExceptionInitializer *node = m_head; node != 0; node = node->m_next)
   {
      if (DDS::string_cmp (node->m_name, name) == 0)
      {
         return node->m_fact;
      }
   }
   return 0;
}

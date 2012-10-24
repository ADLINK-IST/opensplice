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
#include "os_abstract.h"
#include "sacpp_ValueBase.h"

DDS::ValueBase* DDS::ValueBase::_downcast(DDS::ValueBase* v)
{
   return v;
}

DDS::ValueBase::ValueBase()
{
}

DDS::ValueBase::ValueBase(const DDS::ValueBase&)
{
}

DDS::ValueBase::~ValueBase()
{
}

void DDS::add_ref(DDS::ValueBase* vb)
{
   if (vb)
   {
      vb->_add_ref();
   }
}

void DDS::remove_ref(DDS::ValueBase* vb)
{
   if (vb)
   {
      vb->_remove_ref();
   }
}

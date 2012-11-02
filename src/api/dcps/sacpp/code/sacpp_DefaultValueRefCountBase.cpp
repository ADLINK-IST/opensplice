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
#include "sacpp_DefaultValueRefCountBase.h"

DDS::DefaultValueRefCountBase::DefaultValueRefCountBase ()
   : m_count (1)
{
}

DDS::ValueBase* DDS::DefaultValueRefCountBase::_add_ref()
{
    m_count++;
    return this;
}

void DDS::DefaultValueRefCountBase::_remove_ref()
{
    if (--m_count == 0)
    {
       delete this;
    }
}

DDS::ULong DDS::DefaultValueRefCountBase::_refcount_value()
{
   return m_count;
}

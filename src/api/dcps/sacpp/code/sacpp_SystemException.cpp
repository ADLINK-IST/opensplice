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
#include "sacpp_SystemException.h"

DDS::SystemException::SystemException() :
   m_minor (0),
   m_status (DDS::COMPLETED_NO)
{
}

DDS::SystemException::SystemException (DDS::ULong minor, DDS::CompletionStatus status) :
   m_minor (minor),
   m_status (status)
{
}

DDS::SystemException * DDS::SystemException::_downcast 
   (DDS::Exception * e)
{
   return e ? e->_as_system() : 0;
}

const DDS::SystemException * DDS::SystemException::_downcast 
   (const DDS::Exception * e)
{
   return _downcast ((DDS::Exception*) e);
}

DDS::ULong DDS::SystemException::minor () const
{
   return m_minor;
}

void DDS::SystemException::minor (DDS::ULong minor)
{
   m_minor = minor;
}

DDS::CompletionStatus DDS::SystemException::completed () const
{
   return m_status;
}

void DDS::SystemException::completed (DDS::CompletionStatus status)
{
   m_status = status;
}

DDS::SystemException *DDS::SystemException::_as_system()
{
   return this;
}
